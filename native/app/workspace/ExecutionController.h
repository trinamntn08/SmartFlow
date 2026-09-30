#pragma once
#include "GraphProject.h"
#include "pipeline/PipelineExecution.h"
#include <QTimer>
#include <optional>

namespace smartflow {
// At most one submitted run. Edits cancel active work and coalesce the next
// request. Completed results must match both the project revision and request.
class ExecutionController : public QObject {
    Q_OBJECT
public:
    ExecutionController(GraphProject& project, std::shared_ptr<const tp_data::CollectionFactory> factory);
    ~ExecutionController() override;
    void run();
    void cancel();
    void setLive(bool enabled);
    bool busy() const { return pending.has_value(); }
    const std::optional<ExecutionResult>& result() const { return published; }
    QString status() const { return message; }
Q_SIGNALS:
    void updated();
private:
    void invalidate();
    void startRequested();
    void poll();
    GraphProject& project;
    std::shared_ptr<const tp_data::CollectionFactory> factory;
    PipelineExecution executor;
    QTimer debounce;
    QTimer completion;
    std::optional<ExecutionHandle> pending;
    std::optional<ExecutionResult> published;
    quint64 submittedRevision = 0;
    quint64 generation = 0;
    quint64 submittedGeneration = 0;
    bool live = true;
    bool requested = false;
    QString message = "Ready";
};
} // namespace smartflow
