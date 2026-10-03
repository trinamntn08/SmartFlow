#pragma once
#include <QtNodes/internal/DefaultHorizontalNodeGeometry.hpp>

namespace smartflow {
// Sockets above controls keep node widths compact, as in familiar node editors.
class NodeBodyGeometry final : public QtNodes::DefaultHorizontalNodeGeometry {
public:
    using DefaultHorizontalNodeGeometry::DefaultHorizontalNodeGeometry;
    void recomputeSize(QtNodes::NodeId node) const override;
    QPointF widgetPosition(QtNodes::NodeId node) const override;
private:
    int controlsTop(QtNodes::NodeId node) const;
};
}
