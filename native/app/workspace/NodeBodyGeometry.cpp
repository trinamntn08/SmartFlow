#include "NodeBodyGeometry.h"
#include <QtNodes/internal/AbstractGraphModel.hpp>
#include <QWidget>
#include <algorithm>
#include <cmath>

namespace smartflow {
int NodeBodyGeometry::controlsTop(QtNodes::NodeId node) const
{
    const auto ports=std::max(_graphModel.nodeData(node,QtNodes::NodeRole::InPortCount).toUInt(),
                              _graphModel.nodeData(node,QtNodes::NodeRole::OutPortCount).toUInt());
    const auto headerBottom=captionRect(node).height()+10;
    if(!ports) return int(std::ceil(headerBottom))+6;
    const auto type=_graphModel.nodeData(node,QtNodes::NodeRole::InPortCount).toUInt()==ports
        ? QtNodes::PortType::In : QtNodes::PortType::Out;
    const auto halfRow=portPosition(node,type,0).y()-headerBottom;
    return int(std::ceil(portPosition(node,type,ports-1).y()+halfRow))+6;
}
void NodeBodyGeometry::recomputeSize(QtNodes::NodeId node) const
{
    DefaultHorizontalNodeGeometry::recomputeSize(node);
    auto* widget=_graphModel.nodeData(node,QtNodes::NodeRole::Widget).value<QWidget*>();
    if(!widget) return;
    const auto previous=size(node);
    const int width=std::max({184,int(std::ceil(captionRect(node).width()))+20,previous.width()-widget->width()});
    widget->setFixedWidth(width-20);
    _graphModel.setNodeData(node,QtNodes::NodeRole::Size,QSize(width,controlsTop(node)+widget->height()+10));
}
QPointF NodeBodyGeometry::widgetPosition(QtNodes::NodeId node) const
{
    return {10.0,double(controlsTop(node))};
}
}
