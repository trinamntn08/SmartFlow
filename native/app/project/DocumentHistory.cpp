#include "DocumentHistory.h"
#include <QUndoCommand>

namespace smartflow::project {
namespace {
Document semanticDocument(const Document& source)
{
    auto result=source;
    result.erase("workspace");
    return result;
}
}

class DocumentHistory::Command final : public QUndoCommand {
public:
    Command(DocumentHistory& owner, QString label, Document before, Document after, quint64 group)
        : QUndoCommand(std::move(label)), owner(owner), before(std::move(before)), after(std::move(after)), group(group) {}
    void undo() override { owner.restore(before); }
    void redo() override { owner.restore(after); }
    int id() const override { return group ? 1 : -1; }
    bool mergeWith(const QUndoCommand* command) override {
        const auto* next=dynamic_cast<const Command*>(command);
        if(!next || &owner!=&next->owner || group!=next->group || after!=next->before) return false;
        after=next->after;
        setObsolete(before==after);
        return true;
    }
private:
    DocumentHistory& owner;
    Document before, after;
    quint64 group;
};

DocumentHistory::DocumentHistory(Document source, std::string graphId,
    std::shared_ptr<const tp_pipeline::StepDelegateMap> delegates,
    std::vector<NodePresentation> registrations)
    : delegates(std::move(delegates)), registrations(std::move(registrations)), graphId(std::move(graphId))
{
    current=prepare(std::move(source),this->graphId);
}

std::unique_ptr<DocumentSession> DocumentHistory::prepare(Document source, const std::string& graph) const
{
    return std::make_unique<DocumentSession>(std::move(source),graph,delegates,registrations);
}

void DocumentHistory::edit(const QString& label, const std::function<void(DocumentSession&)>& operation, quint64 group)
{
    auto candidate=prepare(current->document(),graphId);
    operation(*candidate); // Reject before QUndoStack can discard its redo branch.
    auto before=semanticDocument(current->document());
    auto after=semanticDocument(candidate->document());
    if(before==after) return;
    history.push(new Command(*this,label,std::move(before),std::move(after),group));
}

void DocumentHistory::restore(const Document& semanticSnapshot)
{
    auto source=semanticSnapshot;
    const auto& active=current->document();
    if(active.contains("workspace")) source["workspace"]=active["workspace"];
    auto candidate=prepare(std::move(source),graphId);
    current.swap(candidate);
    ++semanticRevision;
    Q_EMIT changed();
}

void DocumentHistory::replace(Document source, std::string graph)
{
    auto candidate=prepare(std::move(source),graph);
    current.swap(candidate);
    graphId.swap(graph);
    history.clear();
    ++semanticRevision;
    Q_EMIT changed();
    Q_EMIT workspaceChanged();
}

void DocumentHistory::createNode(const std::string& id, const std::string& type)
{
    edit("Create node",[&](auto& session) { session.createNode(id,type); });
}

void DocumentHistory::removeNode(const std::string& id)
{
    edit("Delete node",[&](auto& session) { session.removeNode(id); });
}

void DocumentHistory::setParameter(const std::string& id, const std::string& name, const Document& value, quint64 group)
{
    edit("Edit " + QString::fromStdString(name),[&](auto& session) { session.setParameter(id,name,value); },group);
}

void DocumentHistory::connect(const std::string& id, const std::string& source, const std::string& output,
                              const std::string& target, const std::string& input)
{
    edit("Connect nodes",[&](auto& session) { session.connect(id,source,output,target,input); });
}

void DocumentHistory::disconnect(const std::string& id)
{
    edit("Disconnect nodes",[&](auto& session) { session.disconnect(id); });
}

void DocumentHistory::setWorkspaceField(const std::string& name, const Document& value)
{
    const auto& source=current->document();
    if(source.contains("workspace") && source["workspace"].contains(name) && source["workspace"][name]==value) return;
    current->setWorkspaceField(name,value);
    Q_EMIT workspaceChanged();
}

void DocumentHistory::catalogComponent(const GraphComponent& component)
{
    edit("Catalog component",[&](auto& candidate) { candidate.catalogComponent(component); });
}

void DocumentHistory::removeCatalogComponent(size_t index)
{
    edit("Remove component from library",[&](auto& candidate) { candidate.removeCatalogComponent(index); });
}

void DocumentHistory::replaceComponentInstance(const std::string& nodeId, const GraphComponent& component)
{
    edit("Update component instance",[&](auto& candidate) { candidate.replaceComponentInstance(nodeId,component); });
}

GraphComponent DocumentHistory::extractComponent(const std::vector<std::string>& nodeIds,
    const std::string& id, const std::string& title, const Document& inputs,
    const Document& outputs, const Document& controls)
{
    auto component=GraphComponent::extract(current->document(),graphId,nodeIds,id,title,inputs,outputs,controls);
    catalogComponent(component);
    return component;
}

ComponentBindings DocumentHistory::instantiateComponent(const GraphComponent& component, const std::string& instanceId,
    const Document& controls, const Document& inputSources, bool collapsed)
{
    ComponentBindings result;
    edit("Instantiate component",[&](auto& candidate) {
        result=candidate.instantiateComponent(component,instanceId,controls,inputSources,collapsed);
    });
    return result;
}
} // namespace smartflow::project
