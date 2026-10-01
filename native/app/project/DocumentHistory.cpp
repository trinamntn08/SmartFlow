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
    Command(DocumentHistory& owner, QString label, Document before, Document after)
        : QUndoCommand(std::move(label)), owner(owner), before(std::move(before)), after(std::move(after)) {}
    void undo() override { owner.restore(before); }
    void redo() override { owner.restore(after); }
private:
    DocumentHistory& owner;
    Document before, after;
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

void DocumentHistory::edit(const QString& label, const std::function<void(DocumentSession&)>& operation)
{
    auto candidate=prepare(current->document(),graphId);
    operation(*candidate); // Reject before QUndoStack can discard its redo branch.
    auto before=semanticDocument(current->document());
    auto after=semanticDocument(candidate->document());
    if(before==after) return;
    history.push(new Command(*this,label,std::move(before),std::move(after)));
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

void DocumentHistory::setParameter(const std::string& id, const std::string& name, const Document& value)
{
    edit("Edit " + QString::fromStdString(name),[&](auto& session) { session.setParameter(id,name,value); });
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
} // namespace smartflow::project
