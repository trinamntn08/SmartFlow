#include "QtNodes/internal/DefaultConnectionPainter.hpp"

#include "QtNodes/internal/AbstractGraphModel.hpp"
#include "QtNodes/internal/ConnectionGraphicsObject.hpp"
#include "QtNodes/internal/ConnectionState.hpp"
#include "QtNodes/internal/Definitions.hpp"
#include "QtNodes/internal/NodeData.hpp"
#include "QtNodes/internal/StyleCollection.hpp"

#include <QtGui/QIcon>


namespace QtNodes {

QPainterPath DefaultConnectionPainter::cubicPath(ConnectionGraphicsObject const &connection) const
{
  QPointF const &in = connection.endPoint(PortType::In);
  QPointF const &out = connection.endPoint(PortType::Out);

  auto const c1c2 = connection.pointsC1C2();

  QPainterPath cubic(out);

  cubic.cubicTo(c1c2.first, c1c2.second, in);

  return cubic;
}

QPainterPath DefaultConnectionPainter::polyCurvePath(ConnectionGraphicsObject const &connection) const
{
  QPointF const &in   = connection.endPoint(PortType::In);
  QPointF const &out  = connection.endPoint(PortType::Out);
  auto const &anchors = connection.anchors();

  QPainterPath path(out);

  // 1. Create a list of targets: Out -> [anchors] -> In
  std::vector<QPointF> targets;
  targets.reserve(anchors.size() + 1);

  for(const auto& p : anchors)
    targets.push_back(p);

  targets.push_back(in);

  // 2. Draw curves between each point
  QPointF currentStart = out;

  for(const auto& point : targets)
  {
    double xDiff = point.x() - currentStart.x();

    // Calculate control points for a nice horizontal "S" curve
    // "0.3" is the curvature strength.
    double tangentLength = std::abs(xDiff) * 0.3;

    QPointF c1 = currentStart + QPointF(tangentLength, 0);
    QPointF c2 = point - QPointF(tangentLength, 0);

    path.cubicTo(c1, c2, point);

    currentStart = point;
  }

  return path;
}

QPainterPath DefaultConnectionPainter::painterPath(ConnectionGraphicsObject const &connection) const
{
  switch(connection.connectionShape())
  {
    case ConnectionShape::PolyCurve:
      return polyCurvePath(connection);

    case ConnectionShape::Cubic:
    default:
      return cubicPath(connection);
  }
}

void DefaultConnectionPainter::drawSketchLine(QPainter *painter, ConnectionGraphicsObject const &cgo) const
{
    ConnectionState const &state = cgo.connectionState();

    if (state.requiresPort()) {
        auto const &connectionStyle = QtNodes::StyleCollection::connectionStyle();

        QPen pen;
        pen.setWidth(static_cast<int>(connectionStyle.constructionLineWidth()));
        pen.setColor(connectionStyle.constructionColor());
        pen.setStyle(Qt::DashLine);

        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        auto path = painterPath(cgo);

        // cubic spline
        painter->drawPath(path);
    }
}

void DefaultConnectionPainter::drawHoveredOrSelected(QPainter *painter, ConnectionGraphicsObject const &cgo) const
{
  bool const hovered = cgo.connectionState().hovered();
  bool const selected = cgo.isSelected();

  // drawn as a fat background
  if(hovered || selected)
  {
    auto const &connectionStyle = QtNodes::StyleCollection::connectionStyle();

    double const lineWidth = connectionStyle.lineWidth();

    QPen pen;
    pen.setWidth(static_cast<int>(2 * lineWidth));
    pen.setColor(selected ? connectionStyle.selectedHaloColor()
                           : connectionStyle.hoveredColor());

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    auto path = painterPath(cgo);
    painter->drawPath(path);

    if(cgo.connectionShape() == ConnectionShape::PolyCurve && !cgo.anchors().empty())
    {
      painter->setBrush(Qt::white);
      painter->setPen(QPen(Qt::black, 1.2));

      for(auto const &point : cgo.anchors())
      {
        painter->drawEllipse(point, 6.0, 6.0);
      }
    }
  }
}

void DefaultConnectionPainter::drawNormalLine(QPainter *painter, ConnectionGraphicsObject const &cgo) const
{
    ConnectionState const &state = cgo.connectionState();

    if (state.requiresPort())
        return;

    // colors

    auto const &connectionStyle = QtNodes::StyleCollection::connectionStyle();

    QColor normalColorOut = connectionStyle.normalColor();
    QColor normalColorIn = connectionStyle.normalColor();
    QColor selectedColor = connectionStyle.selectedColor();

    bool useGradientColor = false;

    AbstractGraphModel const &graphModel = cgo.graphModel();

    if (connectionStyle.useDataDefinedColors()) {
        using QtNodes::PortType;

        auto const cId = cgo.connectionId();

        auto dataTypeOut = graphModel
                               .portData(cId.outNodeId,
                                         PortType::Out,
                                         cId.outPortIndex,
                                         PortRole::DataType)
                               .value<NodeDataType>();

        auto dataTypeIn
            = graphModel.portData(cId.inNodeId, PortType::In, cId.inPortIndex, PortRole::DataType)
                  .value<NodeDataType>();

        useGradientColor = (dataTypeOut.id != dataTypeIn.id);

        normalColorOut = connectionStyle.normalColor(dataTypeOut.id);
        normalColorIn = connectionStyle.normalColor(dataTypeIn.id);
        selectedColor = normalColorOut.darker(200);
    }

    // geometry

    double const lineWidth = connectionStyle.lineWidth();

    // draw normal line
    QPen p;

    p.setWidth(lineWidth);

    bool const selected = cgo.isSelected();

    auto path = painterPath(cgo);
    if (useGradientColor) {
        painter->setBrush(Qt::NoBrush);

        QColor cOut = normalColorOut;
        if (selected)
            cOut = cOut.darker(200);
        p.setColor(cOut);
        painter->setPen(p);

        unsigned int constexpr segments = 60;

        for (unsigned int i = 0ul; i < segments; ++i) {
            double ratioPrev = double(i) / segments;
            double ratio = double(i + 1) / segments;

            if (i == segments / 2) {
                QColor cIn = normalColorIn;
                if (selected)
                    cIn = cIn.darker(200);

                p.setColor(cIn);
                painter->setPen(p);
            }
            painter->drawLine(path.pointAtPercent(ratioPrev), path.pointAtPercent(ratio));
        }

        {
            QIcon icon(":convert.png");

            QPixmap pixmap = icon.pixmap(QSize(22, 22));
            painter->drawPixmap(path.pointAtPercent(0.50)
                                    - QPoint(pixmap.width() / 2, pixmap.height() / 2),
                                pixmap);
        }
    } else {
        p.setColor(normalColorOut);

        if (selected) {
            p.setColor(selectedColor);
        }

        painter->setPen(p);
        painter->setBrush(Qt::NoBrush);

        painter->drawPath(path);
    }
}

void DefaultConnectionPainter::paint(QPainter *painter, ConnectionGraphicsObject const &cgo) const
{
    drawHoveredOrSelected(painter, cgo);

    drawSketchLine(painter, cgo);

    drawNormalLine(painter, cgo);

#ifdef NODE_DEBUG_DRAWING
    debugDrawing(painter, cgo);
#endif

    // draw end points
    auto const &connectionStyle = QtNodes::StyleCollection::connectionStyle();

    double const pointDiameter = connectionStyle.pointDiameter();

    painter->setPen(connectionStyle.constructionColor());
    painter->setBrush(connectionStyle.constructionColor());
    double const pointRadius = pointDiameter / 2.0;
    painter->drawEllipse(cgo.out(), pointRadius, pointRadius);
    painter->drawEllipse(cgo.in(), pointRadius, pointRadius);
}

QPainterPath DefaultConnectionPainter::getPainterStroke(ConnectionGraphicsObject const &connection) const
{
    auto path = painterPath(connection);

    QPointF const &out = connection.endPoint(PortType::Out);
    QPainterPath result(out);

    unsigned int constexpr segments = 20;

    for (auto i = 0ul; i < segments; ++i) {
        double ratio = double(i + 1) / segments;
        result.lineTo(path.pointAtPercent(ratio));
    }

    QPainterPathStroker stroker;
    stroker.setWidth(10.0);

    return stroker.createStroke(result);
}

#ifdef NODE_DEBUG_DRAWING
void DefaultConnectionPainter::debugDrawing(QPainter *painter, ConnectionGraphicsObject const &cgo)
{
    Q_UNUSED(painter);

    {
        QPointF const &in = cgo.endPoint(PortType::In);
        QPointF const &out = cgo.endPoint(PortType::Out);

        auto const points = cgo.pointsC1C2();

        painter->setPen(Qt::red);
        painter->setBrush(Qt::red);

        painter->drawLine(QLineF(out, points.first));
        painter->drawLine(QLineF(points.first, points.second));
        painter->drawLine(QLineF(points.second, in));
        painter->drawEllipse(points.first, 3, 3);
        painter->drawEllipse(points.second, 3, 3);

        painter->setBrush(Qt::NoBrush);
        painter->drawPath(cubicPath(cgo));
    }

    {
        painter->setPen(Qt::yellow);
        painter->drawRect(cgo.boundingRect());
    }
}
#endif

} // namespace QtNodes
