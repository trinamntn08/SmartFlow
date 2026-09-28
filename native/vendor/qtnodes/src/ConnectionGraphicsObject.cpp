#include "QtNodes/internal/ConnectionGraphicsObject.hpp"

#include "QtNodes/internal/AbstractConnectionPainter.hpp"
#include "QtNodes/internal/AbstractGraphModel.hpp"
#include "QtNodes/internal/AbstractNodeGeometry.hpp"
#include "QtNodes/internal/BasicGraphicsScene.hpp"
#include "QtNodes/internal/ConnectionIdUtils.hpp"
#include "QtNodes/internal/ConnectionState.hpp"
#include "QtNodes/internal/ConnectionStyle.hpp"
#include "QtNodes/internal/DataFlowGraphModel.hpp"
#include "QtNodes/internal/NodeConnectionInteraction.hpp"
#include "QtNodes/internal/NodeGraphicsObject.hpp"
#include "QtNodes/internal/StyleCollection.hpp"
#include "QtNodes/internal/locateNode.hpp"

#include <QtWidgets/QGraphicsBlurEffect>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QStyleOptionGraphicsItem>

#include <QtCore/QDebug>

#include <stdexcept>

namespace QtNodes {

ConnectionGraphicsObject::ConnectionGraphicsObject(BasicGraphicsScene &scene,
                                                   ConnectionId const connectionId)
    : _connectionShape(ConnectionShape::PolyCurve)
    , _connectionId(connectionId)
    , _graphModel(scene.graphModel())
    , _connectionState(*this)
    , _out{0, 0}
    , _in{0, 0}
{
    scene.addItem(this);

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    setAcceptHoverEvents(true);

    //addGraphicsEffect();

    setZValue(-1.0);

    initializePosition();
}

void ConnectionGraphicsObject::initializePosition()
{
    // This function is only called when the ConnectionGraphicsObject
    // is newly created. At this moment both end coordinates are (0, 0)
    // in Connection G.O. coordinates. The position of the whole
    // Connection G. O. in scene coordinate system is also (0, 0).
    // By moving the whole object to the Node Port position
    // we position both connection ends correctly.

    if (_connectionState.requiredPort() != PortType::None) {
        PortType attachedPort = oppositePort(_connectionState.requiredPort());

        PortIndex portIndex = getPortIndex(attachedPort, _connectionId);
        NodeId nodeId = getNodeId(attachedPort, _connectionId);

        NodeGraphicsObject *ngo = nodeScene()->nodeGraphicsObject(nodeId);

        if (ngo) {
            QTransform nodeSceneTransform = ngo->sceneTransform();

            AbstractNodeGeometry &geometry = nodeScene()->nodeGeometry();

            QPointF pos = geometry.portScenePosition(nodeId,
                                                     attachedPort,
                                                     portIndex,
                                                     nodeSceneTransform);

            this->setPos(pos);
        }
    }
    if (auto* dfModel = dynamic_cast<DataFlowGraphModel*>(&_graphModel))
    {
      auto storedAnchors = dfModel->connectionAnchors(_connectionId);
      if(!storedAnchors.empty())
      {
        setAnchors(storedAnchors);
      }
    }

    move();
}

AbstractGraphModel &ConnectionGraphicsObject::graphModel() const
{
    return _graphModel;
}

BasicGraphicsScene *ConnectionGraphicsObject::nodeScene() const
{
    return dynamic_cast<BasicGraphicsScene *>(scene());
}

ConnectionId const &ConnectionGraphicsObject::connectionId() const
{
    return _connectionId;
}

QRectF ConnectionGraphicsObject::boundingRect() const
{
  QRectF rect = QRectF(_out, _in).normalized();

  for(auto const &point : _anchors)
  {
    QRectF pRect(point, point);
    pRect.adjust(-40.0, -40.0, 40.0, 40.0);
    rect = rect.united(pRect);
  }

  if(_anchors.empty())
  {
    auto [c1, c2] = pointsC1C2();
    QPainterPath p;
    p.moveTo(_out);
    p.cubicTo(c1, c2, _in);

    rect = rect.united(p.boundingRect());
  }

  auto const &connectionStyle = StyleCollection::connectionStyle();
  float const diam = connectionStyle.pointDiameter();
  float const offset = std::max(diam, 20.0f);

  rect.adjust(qreal(-offset), qreal(-offset), qreal(offset), qreal(offset));

  return rect;
}

QPainterPath ConnectionGraphicsObject::shape() const
{
#ifdef DEBUG_DRAWING

    //QPainterPath path;

    //path.addRect(boundingRect());
    //return path;

#else
    return nodeScene()->connectionPainter().getPainterStroke(*this);
#endif
}

void ConnectionGraphicsObject::setConnectionShape(ConnectionShape style)
{
  if(_connectionShape != style)
  {
    _connectionShape = style;
    prepareGeometryChange();
    update();
  }
}

ConnectionShape ConnectionGraphicsObject::connectionShape() const
{
  return _connectionShape;
}

std::vector<QPointF> const& ConnectionGraphicsObject::anchors() const
{
  return _anchors;
}

void ConnectionGraphicsObject::setAnchors(const std::vector<QPointF>& anchors)
{
  _anchors = anchors;
  prepareGeometryChange();
  update();
}

void ConnectionGraphicsObject::clearAnchors()
{
  _anchors.clear();
  update();
}

QPointF const &ConnectionGraphicsObject::endPoint(PortType portType) const
{
    Q_ASSERT(portType != PortType::None);

    return (portType == PortType::Out ? _out : _in);
}

void ConnectionGraphicsObject::setEndPoint(PortType portType, QPointF const &point)
{
  if (portType == PortType::In)
      _in = point;
  else
      _out = point;
}

void ConnectionGraphicsObject::move()
{
  auto moveEnd = [this](ConnectionId cId, PortType portType) {
      NodeId nodeId = getNodeId(portType, cId);

      if (nodeId == InvalidNodeId)
          return;

      NodeGraphicsObject *ngo = nodeScene()->nodeGraphicsObject(nodeId);

      if (ngo) {
          AbstractNodeGeometry &geometry = nodeScene()->nodeGeometry();

          QPointF scenePos = geometry.portScenePosition(nodeId,
                                                        portType,
                                                        getPortIndex(portType, cId),
                                                        ngo->sceneTransform());

          QPointF connectionPos = sceneTransform().inverted().map(scenePos);

          setEndPoint(portType, connectionPos);
      }
  };

  moveEnd(_connectionId, PortType::Out);
  moveEnd(_connectionId, PortType::In);

  prepareGeometryChange();

  update();
}

ConnectionState const &ConnectionGraphicsObject::connectionState() const
{
  return _connectionState;
}

ConnectionState &ConnectionGraphicsObject::connectionState()
{
    return _connectionState;
}

void ConnectionGraphicsObject::paint(QPainter *painter,
                                     QStyleOptionGraphicsItem const *option,
                                     QWidget *)
{
    if (!scene())
        return;

    painter->setClipRect(option->exposedRect);

    nodeScene()->connectionPainter().paint(painter, *this);
}

void ConnectionGraphicsObject::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  if(_connectionState.requiredPort() != PortType::None)
  {
    QGraphicsObject::mousePressEvent(event);
    return;
  }

  if(event->button() == Qt::LeftButton)
  {
    QPointF clickPos = event->pos();
    _clickedPointIndex = -1;

    // A. Avoid to check near the starting and ending point
    double const deadZone = 20.0;
    double distOut = QLineF(clickPos, _out).length();
    double distIn  = QLineF(clickPos, _in).length();

    if(distOut < deadZone || distIn < deadZone)
    {
      QGraphicsObject::mousePressEvent(event);
      return;
    }

    // B. Check if existing anchor is clicked (Selection)
    double const pointRadius = 40.0;
    for(size_t i = 0; i < _anchors.size(); ++i)
    {
      if(QLineF(clickPos, _anchors[i]).length() <= pointRadius)
      {
        _clickedPointIndex = static_cast<int>(i);
        _isDragging = true;
        event->accept();
        return;
      }
    }

    // C. Check if the wire segment is clicked (Insertion)
    if(_anchors.empty())
    {
      QPainterPath path;
      path.moveTo(_out);

      auto [c1, c2] = pointsC1C2();
      path.cubicTo(c1, c2, _in);

      QPainterPathStroker stroker;
      stroker.setWidth(40.0);
      QPainterPath area = stroker.createStroke(path);

      if(area.contains(clickPos))
      {
        _anchors.push_back(clickPos);
        _clickedPointIndex = 0;
        _isDragging = true;
        update();
        event->accept();
        return;
      }
    }
    else
    {
      std::vector<QPointF> allPoints;
      allPoints.reserve(_anchors.size() + 2);
      allPoints.push_back(_out);
      allPoints.insert(allPoints.end(), _anchors.begin(), _anchors.end());
      allPoints.push_back(_in);

      auto const orientation = nodeScene()->orientation();

      for(size_t i = 0; i < allPoints.size() - 1; ++i)
      {
        QPointF pStart = allPoints[i];
        QPointF pEnd   = allPoints[i+1];

        QPainterPath segmentPath;
        segmentPath.moveTo(pStart);

        {
          double diff = (orientation == Qt::Horizontal)
                            ? pEnd.x() - pStart.x()
                            : pEnd.y() - pStart.y();

          double tangent = std::abs(diff) * 0.5;
          if (tangent < 50.0) tangent = 50.0;

          QPointF tangentVector = (orientation == Qt::Horizontal)
                                      ? QPointF(tangent, 0)
                                      : QPointF(0, tangent);

          QPointF c1 = pStart + tangentVector;
          QPointF c2 = pEnd   - tangentVector;

          segmentPath.cubicTo(c1, c2, pEnd);
        }

        QPainterPathStroker stroker;
        stroker.setWidth(40.0);
        QPainterPath area = stroker.createStroke(segmentPath);

        if(area.contains(clickPos))
        {
          auto it = _anchors.begin() + i;
          _anchors.insert(it, clickPos);
          _clickedPointIndex = static_cast<int>(i);
          _isDragging = true;
          update();
          event->accept();
          return;
        }
      }
    }
  }

  QGraphicsObject::mousePressEvent(event);
}

void ConnectionGraphicsObject::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
  if(_isDragging && _clickedPointIndex >= 0 && _clickedPointIndex < int(_anchors.size()))
  {
    QPointF newPos = event->pos();
    _anchors[_clickedPointIndex] = newPos;

    _anchorToRemove = -1;

    const float anchorRemovalDistance = 30.0f;
    auto isClose = [&](const QPointF& a, const QPointF& b)
    {
      return QLineF(a, b).length() <= qreal(anchorRemovalDistance);
    };

    auto checkNeighbor = [&](int neighborIndex)
    {
      if(neighborIndex >= 0 && neighborIndex < int(_anchors.size()))
        return isClose(newPos, _anchors[neighborIndex]);

      return false;
    };

    bool remove = false;
    if(_clickedPointIndex == 0)
      remove = isClose(newPos, _out);
    else
      remove = checkNeighbor(_clickedPointIndex - 1);

    if(!remove)
    {
      if(_clickedPointIndex == int(_anchors.size()) - 1)
        remove = isClose(newPos, _in);
      else
        remove = checkNeighbor(_clickedPointIndex + 1);
    }

    if(remove)
      _anchorToRemove = _clickedPointIndex;

    prepareGeometryChange();
    update();
    event->accept();
  }
  else
  {
    prepareGeometryChange();

    auto view = static_cast<QGraphicsView *>(event->widget());
    auto ngo = locateNodeAt(event->scenePos(), *nodeScene(), view->transform());
    if(ngo)
    {
      ngo->reactToConnection(this);

      _connectionState.setLastHoveredNode(ngo->nodeId());
    }
    else
    {
      _connectionState.resetLastHoveredNode();
    }

    auto requiredPort = _connectionState.requiredPort();

    if(requiredPort != PortType::None)
    {
      setEndPoint(requiredPort, event->pos());
    }

    update();

    event->accept();
  }
}

void ConnectionGraphicsObject::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  if(_isDragging)
  {
    _isDragging = false;
    if(_anchorToRemove != -1)
    {
      if(_anchorToRemove >= 0 && _anchorToRemove < int(_anchors.size()))
      {
        _anchors.erase(_anchors.begin() + _anchorToRemove);
        _graphModel.setConnectionAnchors(_connectionId, _anchors);
        _anchorToRemove = -1;
        _clickedPointIndex = -1;
        prepareGeometryChange();
        update();
      }
    }
    else
    {
      _graphModel.setConnectionAnchors(_connectionId, _anchors);
    }
  }
  else
  {
    QGraphicsObject::mouseReleaseEvent(event);
  }

  ungrabMouse();
  event->accept();

  auto view = static_cast<QGraphicsView *>(event->widget());

  Q_ASSERT(view);

  auto ngo = locateNodeAt(event->scenePos(), *nodeScene(), view->transform());

  bool wasConnected = false;

  if(ngo)
  {
      NodeConnectionInteraction interaction(*ngo, *this, *nodeScene());

      wasConnected = interaction.tryConnect();
  }

  // If connection attempt was unsuccessful
  if(!wasConnected)
  {
    nodeScene()->resetDraftConnection();
  }
}

void ConnectionGraphicsObject::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
  _connectionState.setHovered(true);
  update();
  emit nodeScene()->connectionHovered(connectionId(), event->screenPos());
  event->accept();
}

void ConnectionGraphicsObject::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
  _connectionState.setHovered(false);
  update();
  emit nodeScene()->connectionHoverLeft(connectionId());
  event->accept();
}

std::pair<QPointF, QPointF> ConnectionGraphicsObject::pointsC1C2() const
{
    switch (nodeScene()->orientation()) {
    case Qt::Horizontal:
        return pointsC1C2Horizontal();
        break;

    case Qt::Vertical:
        return pointsC1C2Vertical();
        break;
    }

    throw std::logic_error("Unreachable code after switch statement");
}

void ConnectionGraphicsObject::addGraphicsEffect()
{
  auto effect = new QGraphicsBlurEffect;
  effect->setBlurRadius(5);
  setGraphicsEffect(effect);
}

std::pair<QPointF, QPointF> ConnectionGraphicsObject::pointsC1C2Horizontal() const
{
  double const defaultOffset = 200;

  double xDistance = _in.x() - _out.x();

  double horizontalOffset = qMin(defaultOffset, std::abs(xDistance));

  double verticalOffset = 0;

  double ratioX = 0.5;

  if (xDistance <= 0) {
      double yDistance = _in.y() - _out.y() + 20;

      double vector = yDistance < 0 ? -1.0 : 1.0;

      verticalOffset = qMin(defaultOffset, std::abs(yDistance)) * vector;

      ratioX = 1.0;
  }

  horizontalOffset *= ratioX;

  QPointF c1(_out.x() + horizontalOffset, _out.y() + verticalOffset);

  QPointF c2(_in.x() - horizontalOffset, _in.y() - verticalOffset);

  return std::make_pair(c1, c2);
}

std::pair<QPointF, QPointF> ConnectionGraphicsObject::pointsC1C2Vertical() const
{
  double const defaultOffset = 200;

  double yDistance = _in.y() - _out.y();

  double verticalOffset = qMin(defaultOffset, std::abs(yDistance));

  double horizontalOffset = 0;

  double ratioY = 0.5;

  if (yDistance <= 0) {
      double xDistance = _in.x() - _out.x() + 20;

      double vector = xDistance < 0 ? -1.0 : 1.0;

      horizontalOffset = qMin(defaultOffset, std::abs(xDistance)) * vector;

      ratioY = 1.0;
  }

  verticalOffset *= ratioY;

  QPointF c1(_out.x() + horizontalOffset, _out.y() + verticalOffset);

  QPointF c2(_in.x() - horizontalOffset, _in.y() - verticalOffset);

  return std::make_pair(c1, c2);
}

} // namespace QtNodes
