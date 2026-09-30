#include "PipelineCanvas.h"
#include <tp_qt_pipeline_widgets/StepDelegateNodeDelegateModel.h>
#include <tp_pipeline/StepDelegate.h>

namespace smartflow {
namespace {
class CanvasNode final : public tp_qt_pipeline_widgets::StepDelegateNodeDelegateModel {
public:
    explicit CanvasNode(const tp_pipeline::StepDelegate* definition)
        : StepDelegateNodeDelegateModel(definition) {}
    QString caption() const override { return nodeTitle(stepDelegate()->name()); }
    // Routing points belong exclusively to the canvas workspace state.
    void setConnectionAnchors(QtNodes::PortType, QtNodes::PortIndex, const std::vector<QPointF>&) override {}
};

auto canvasRegistry(const GraphProject& project)
{
    auto registry = std::make_shared<QtNodes::NodeDelegateModelRegistry>();
    for(const auto& entry : project.registry()->stepDelegates()) {
        const auto* definition = entry.second;
        registry->registerModel<CanvasNode>([definition] { return std::make_unique<CanvasNode>(definition); }, "Numeric");
    }
    return registry;
}
}

PipelineCanvas::PipelineCanvas(GraphProject& project)
    : DataFlowGraphModel(canvasRegistry(project)), project(project) {}

tp_utils::StringID PipelineCanvas::projectId(QtNodes::NodeId id) const
{
    auto found = bindings.find(id);
    return found == bindings.end() ? tp_utils::StringID() : found->second;
}

QtNodes::NodeId PipelineCanvas::addNode(QString type)
{
    auto* step = project.create(type.toStdString());
    if(!step) return QtNodes::InvalidNodeId;
    const auto id = DataFlowGraphModel::addNode(type);
    bindings.emplace(id, step->id());
    return id;
}

bool PipelineCanvas::deleteNode(QtNodes::NodeId id)
{
    if(!nodeExists(id)) return false;
    const auto stable = projectId(id);
    DataFlowGraphModel::deleteNode(id); // Calls this adapter's deleteConnection.
    bindings.erase(id);
    project.remove(stable);
    return true;
}

bool PipelineCanvas::connectionPossible(QtNodes::ConnectionId connection) const
{
    if(!nodeExists(connection.outNodeId) || !nodeExists(connection.inNodeId)) return false;
    if(connection.outPortIndex >= nodeData(connection.outNodeId, QtNodes::NodeRole::OutPortCount).toUInt() ||
       connection.inPortIndex >= nodeData(connection.inNodeId, QtNodes::NodeRole::InPortCount).toUInt()) return false;
    return DataFlowGraphModel::connectionPossible(connection);
}

void PipelineCanvas::addConnection(QtNodes::ConnectionId connection)
{
    if(!connectionPossible(connection)) return;
    DataFlowGraphModel::addConnection(connection);
    project.connectInput(projectId(connection.inNodeId), connection.inPortIndex,
                         projectId(connection.outNodeId), connection.outPortIndex);
}

bool PipelineCanvas::deleteConnection(QtNodes::ConnectionId connection)
{
    if(!DataFlowGraphModel::deleteConnection(connection)) return false;
    project.disconnectInput(projectId(connection.inNodeId), connection.inPortIndex);
    return true;
}

QJsonObject PipelineCanvas::saveNode(QtNodes::NodeId id) const
{
    auto result = DataFlowGraphModel::saveNode(id);
    result["project-undo-snapshot"] = project.capture(projectId(id));
    return result;
}

void PipelineCanvas::loadNode(const QJsonObject& snapshot)
{
    const auto id = QtNodes::NodeId(snapshot["id"].toInt());
    if(nodeExists(id)) return;
    // Trusted in-memory undo snapshots only. Clipboard/import is not exposed.
    auto* step = project.restore(snapshot["project-undo-snapshot"].toObject());
    if(!step) return;
    bindings.emplace(id, step->id());
    DataFlowGraphModel::loadNode(snapshot);
}
} // namespace smartflow
