#include "PipelineCanvas.h"
#include <tp_qt_pipeline_widgets/StepDelegateNodeDelegateModel.h>
#include <tp_pipeline/StepDelegate.h>
#include <QScopedValueRollback>
#include <set>

namespace smartflow {
namespace {
class CanvasNode final : public tp_qt_pipeline_widgets::StepDelegateNodeDelegateModel {
public:
    CanvasNode(const tp_pipeline::StepDelegate* definition, QString title)
        : StepDelegateNodeDelegateModel(definition), title(std::move(title)) {}
    CanvasNode(std::shared_ptr<const tp_pipeline::StepDelegate> definition, QString title)
        : StepDelegateNodeDelegateModel(definition.get()), title(std::move(title)), owner(std::move(definition)) {}
    QString caption() const override { return title; }
    // Routing points belong exclusively to the canvas workspace state.
    void setConnectionAnchors(QtNodes::PortType, QtNodes::PortIndex, const std::vector<QPointF>&) override {}
private:
    QString title;
    std::shared_ptr<const tp_pipeline::StepDelegate> owner;
};

class ComponentInterface final : public tp_pipeline::StepDelegate {
public:
    explicit ComponentInterface(const tp_pipeline::StepDetails& step)
        : StepDelegate(step.delegateName(),{},ports(step.inputMapping()),ports(step.outputMapping())) {}
    bool executeStep(tp_pipeline::StepContext*) const override { return false; } // Display metadata only.
private:
    static std::vector<tp_pipeline::PortDetails> ports(const std::vector<tp_pipeline::PortMapping>& mappings) {
        std::vector<tp_pipeline::PortDetails> result;
        for(const auto& port : mappings) result.push_back({port.portName,port.portType});
        return result;
    }
};

class MissingNode final : public QtNodes::NodeDelegateModel {
public:
    QString name() const override { return "smartflow.unavailable"; }
    QString caption() const override { return "Unavailable node"; }
    unsigned int nPorts(QtNodes::PortType) const override { return 0; }
    QtNodes::NodeDataType dataType(QtNodes::PortType,QtNodes::PortIndex) const override { return {}; }
    void setInData(std::shared_ptr<QtNodes::NodeData>,QtNodes::PortIndex) override {}
    std::shared_ptr<QtNodes::NodeData> outData(QtNodes::PortIndex) override { return {}; }
    QWidget* embeddedWidget() override { return nullptr; }
};

auto canvasRegistry(const GraphProject& project)
{
    auto registry = std::make_shared<QtNodes::NodeDelegateModelRegistry>();
    for(const auto& entry : project.registry()->stepDelegates()) {
        const auto* definition = entry.second;
        const auto title = project.title(definition->name());
        registry->registerModel<CanvasNode>([definition, title] { return std::make_unique<CanvasNode>(definition, title); }, project.category(definition->name()));
    }
    registry->registerModel<MissingNode>("Unavailable");
    return registry;
}
}

PipelineCanvas::PipelineCanvas(GraphProject& project)
    : DataFlowGraphModel(canvasRegistry(project)), project(project)
{
    QObject::connect(&project,&GraphProject::changed,this,[this] { synchronize(); });
    synchronize();
}

tp_utils::StringID PipelineCanvas::projectId(QtNodes::NodeId id) const
{
    auto found=bindings.find(id);
    return found==bindings.end() ? tp_utils::StringID() : found->second;
}

QVariant PipelineCanvas::nodeData(QtNodes::NodeId id, QtNodes::NodeRole role) const
{
    if(role==QtNodes::NodeRole::Caption && !project.step(projectId(id)))
        for(const auto& node : project.selectedGraph()["nodes"])
            if(node["id"]==projectId(id).toString())
                return "Unavailable: " + QString::fromStdString(node["packageId"].get<std::string>() + "/" + node["typeId"].get<std::string>());
    return DataFlowGraphModel::nodeData(id,role);
}

QtNodes::NodeId PipelineCanvas::addNode(QString type)
{
    auto* step=project.create(type.toStdString());
    if(!step) return QtNodes::InvalidNodeId;
    return identities.at(step->id().toString());
}

bool PipelineCanvas::deleteNode(QtNodes::NodeId id)
{
    if(syncing) return DataFlowGraphModel::deleteNode(id);
    if(!nodeExists(id)) return false;
    project.remove(projectId(id));
    return true;
}

bool PipelineCanvas::connectionPossible(QtNodes::ConnectionId connection) const
{
    if(!nodeExists(connection.outNodeId) || !nodeExists(connection.inNodeId)) return false;
    if(connection.outPortIndex>=nodeData(connection.outNodeId,QtNodes::NodeRole::OutPortCount).toUInt() ||
       connection.inPortIndex>=nodeData(connection.inNodeId,QtNodes::NodeRole::InPortCount).toUInt()) return false;
    // An unresolved retained edge still occupies its input, even when hidden.
    if(!project.connectionId(projectId(connection.inNodeId),connection.inPortIndex).empty()) return false;
    return DataFlowGraphModel::connectionPossible(connection);
}

void PipelineCanvas::addConnection(QtNodes::ConnectionId connection)
{
    if(syncing) { DataFlowGraphModel::addConnection(connection); return; }
    if(!connectionPossible(connection)) return;
    project.connectInput(projectId(connection.inNodeId),connection.inPortIndex,
                         projectId(connection.outNodeId),connection.outPortIndex);
}

bool PipelineCanvas::deleteConnection(QtNodes::ConnectionId connection)
{
    if(syncing) return DataFlowGraphModel::deleteConnection(connection);
    if(!connectionExists(connection)) return false;
    const auto id=edgeId(connection);
    if(id.empty()) return false;
    project.commands().disconnect(id);
    return true;
}

std::string PipelineCanvas::edgeId(QtNodes::ConnectionId connection) const
{
    const auto found=edgeBindings.find(connection);
    return found==edgeBindings.end() ? std::string() : found->second;
}

void PipelineCanvas::synchronize()
{
    QScopedValueRollback<bool> guard(syncing,true);
    for(const auto& node : project.selectedGraph()["nodes"]) if(project::GraphComponent::isInstance(node)) {
        const auto* step=project.step(node["id"].get<std::string>());
        if(!step) continue;
        auto definition=std::make_shared<ComponentInterface>(*step);
        const auto title=project.title(step->delegateName());
        dataModelRegistry()->registerModel<CanvasNode>([definition,title] {
            return std::make_unique<CanvasNode>(definition,title);
        },"Components");
    }
    std::set<std::string> wanted;
    for(const auto& node : project.selectedGraph()["nodes"]) wanted.insert(node["id"].get<std::string>());
    for(const auto id : allNodeIds()) {
        const auto stable=projectId(id).toString();
        positions[stable]=DataFlowGraphModel::nodeData(id,QtNodes::NodeRole::Position).value<QPointF>();
        const auto* step=project.step(stable);
        const auto type=step ? QString::fromStdString(step->delegateName().toString()) : "smartflow.unavailable";
        if(!wanted.count(stable) || DataFlowGraphModel::nodeData(id,QtNodes::NodeRole::Type).toString()!=type) {
            DataFlowGraphModel::deleteNode(id);
            bindings.erase(id);
        }
    }
    for(const auto& stable : wanted) {
        auto found=identities.find(stable);
        const auto id=found==identities.end() ? static_cast<QtNodes::AbstractGraphModel&>(*this).newNodeId() : found->second;
        identities[stable]=id;
        if(nodeExists(id)) {
            Q_EMIT nodeUpdated(id); // Recompute captions, including changed unavailable identities.
            continue;
        }
        const auto* step=project.step(stable);
        const auto type=step ? QString::fromStdString(step->delegateName().toString()) : "smartflow.unavailable";
        const auto ordinal=identities.size()-1;
        const auto pos=positions.try_emplace(stable,QPointF((ordinal%4)*300.0,(ordinal/4)*180.0)).first->second;
        bindings.emplace(id,stable);
        // Only generated display fields reach QtNodes; no raw project data.
        DataFlowGraphModel::loadNode({{"id",int(id)},{"internal-data",QJsonObject{{"model-name",type}}},
                                     {"position",QJsonObject{{"x",pos.x()},{"y",pos.y()}}}});
    }
    std::unordered_set<QtNodes::ConnectionId> desired;
    edgeBindings.clear();
    std::set<std::pair<std::string,std::string>> inputs;
    for(const auto& edge : project.selectedGraph()["connections"]) {
        const auto from=edge["source"]["nodeId"].get<std::string>(), to=edge["target"]["nodeId"].get<std::string>();
        const auto* source=project.step(from);
        const auto* target=project.step(to);
        if(!source || !target) continue;
        const auto& outputs=source->outputMapping();
        const auto& input=target->inputMapping();
        const auto out=std::find_if(outputs.begin(),outputs.end(),[&](const auto& p) { return p.portName.toString()==edge["source"]["portId"]; });
        const auto in=std::find_if(input.begin(),input.end(),[&](const auto& p) { return p.portName.toString()==edge["target"]["portId"]; });
        if(out==outputs.end() || in==input.end() || out->portType!=in->portType) continue;
        if(!inputs.emplace(to,in->portName.toString()).second) continue;
        const QtNodes::ConnectionId connection{identities.at(from),QtNodes::PortIndex(std::distance(outputs.begin(),out)),
                                               identities.at(to),QtNodes::PortIndex(std::distance(input.begin(),in))};
        desired.insert(connection);
        edgeBindings.emplace(connection,edge["id"].get<std::string>());
    }
    for(const auto id : allNodeIds())
        for(const auto edge : allConnectionIds(id))
            if(!desired.count(edge)) DataFlowGraphModel::deleteConnection(edge);
    for(const auto edge : desired)
        if(!connectionExists(edge)) DataFlowGraphModel::addConnection(edge);
}

void PipelineCanvas::resetLayout()
{
    {
        QScopedValueRollback<bool> guard(syncing,true);
        for(const auto id : allNodeIds()) DataFlowGraphModel::deleteNode(id);
        bindings.clear();
        identities.clear();
        positions.clear();
        edgeBindings.clear();
    }
    synchronize();
}
} // namespace smartflow
