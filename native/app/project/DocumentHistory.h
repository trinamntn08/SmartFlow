#pragma once
#include "DocumentSession.h"
#include <QObject>
#include <QUndoStack>
#include <functional>

namespace smartflow::project {
// Editor command boundary. History stores retained JSON, never Qt JSON or
// disposable StepDetails. A semantic undo preserves the current workspace.
class DocumentHistory : public QObject {
    Q_OBJECT
public:
    DocumentHistory(Document source, std::string graphId,
                    std::shared_ptr<const tp_pipeline::StepDelegateMap> delegates,
                    std::vector<NodePresentation> registrations);
    const DocumentSession& session() const { return *current; }
    QUndoStack& undoStack() { return history; }
    quint64 revision() const { return semanticRevision; }

    void createNode(const std::string& id, const std::string& type);
    void removeNode(const std::string& id);
    void setParameter(const std::string& id, const std::string& name, const Document& value);
    void connect(const std::string& id, const std::string& source, const std::string& output,
                 const std::string& target, const std::string& input);
    void disconnect(const std::string& id);
    // One atomic command for multi-selection edits on a disposable candidate.
    void edit(const QString& label, const std::function<void(DocumentSession&)>& operation);
    void setWorkspaceField(const std::string& name, const Document& value);
    // Prepare before replacing. Failed loads preserve the session and history;
    // successful replacement invalidates old commands and execution revisions.
    void replace(Document source, std::string graphId);
Q_SIGNALS:
    void changed();
    void workspaceChanged();
private:
    class Command;
    void restore(const Document& semanticSnapshot);
    std::unique_ptr<DocumentSession> prepare(Document source, const std::string& graph) const;
    std::shared_ptr<const tp_pipeline::StepDelegateMap> delegates;
    std::vector<NodePresentation> registrations;
    std::string graphId;
    std::unique_ptr<DocumentSession> current;
    QUndoStack history;
    quint64 semanticRevision = 0;
};
} // namespace smartflow::project
