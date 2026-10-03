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
    return int(std::ceil(captionRect(node).height()))+10+int(ports)*(QFontMetrics(QFont()).height()+10)+6;
}
void NodeBodyGeometry::recomputeSize(QtNodes::NodeId node) const
{
    DefaultHorizontalNodeGeometry::recomputeSize(node);
    const auto* widget=_graphModel.nodeData(node,QtNodes::NodeRole::Widget).value<QWidget*>();
    if(!widget) return;
    const auto previous=size(node);
    const int width=std::max({widget->width()+20,int(std::ceil(captionRect(node).width()))+20,previous.width()-widget->width()});
    _graphModel.setNodeData(node,QtNodes::NodeRole::Size,QSize(width,controlsTop(node)+widget->height()+10));
}
QPointF NodeBodyGeometry::widgetPosition(QtNodes::NodeId node) const
{
    const auto* widget=_graphModel.nodeData(node,QtNodes::NodeRole::Widget).value<QWidget*>();
    return {(size(node).width()-(widget ? widget->width() : 0))/2.0,double(controlsTop(node))};
}
}
