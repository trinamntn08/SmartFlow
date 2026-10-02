#include "ExecutionController.h"
#include <chrono>

namespace smartflow {
ExecutionController::ExecutionController(GraphProject& project,
    std::shared_ptr<const tp_data::CollectionFactory> factory, NodeExecutionPolicies policies)
    : project(project), factory(std::move(factory))
{
    executionOptions.policies=std::move(policies);
    debounce.setInterval(80);
    debounce.setSingleShot(true);
    completion.setInterval(15);
    connect(&project, &GraphProject::changed, this, &ExecutionController::invalidate);
    connect(&debounce, &QTimer::timeout, this, &ExecutionController::startRequested);
    connect(&completion, &QTimer::timeout, this, &ExecutionController::poll);
}

ExecutionController::~ExecutionController()
{
    debounce.stop();
    completion.stop();
    if(pending) pending->cancel();
}

void ExecutionController::invalidate()
{
    ++generation;
    published.reset();
    publishedProgress.reset();
    if(pending) pending->cancel();
    requested = live;
    message = live ? "Waiting for latest edit..." : "Outdated - run to update";
    if(live) debounce.start();
    Q_EMIT updated();
}

void ExecutionController::setLive(bool enabled)
{
    live = enabled;
    if(enabled) run();
    else { debounce.stop(); requested = false; }
}

void ExecutionController::setScheduling(ExecutionMode mode, size_t maxThreads)
{
    if(mode!=ExecutionMode::Sequential && mode!=ExecutionMode::Parallel) throw std::invalid_argument("Invalid execution mode");
    if(maxThreads<1 || maxThreads>64) throw std::invalid_argument("Execution thread budget must be between 1 and 64");
    if(executionOptions.mode==mode && executionOptions.maxThreads==maxThreads) return;
    executionOptions.mode=mode; executionOptions.maxThreads=maxThreads;
    invalidate();
}

void ExecutionController::run()
{
    ++generation;
    published.reset();
    publishedProgress.reset();
    requested = true;
    debounce.stop();
    if(pending) { pending->cancel(); message = "Waiting for previous run to stop..."; Q_EMIT updated(); }
    else startRequested();
}

void ExecutionController::cancel()
{
    requested = false;
    debounce.stop();
    if(pending) pending->cancel();
    published.reset();
    publishedProgress.reset();
    message = pending ? "Cancelling..." : "Cancelled";
    Q_EMIT updated();
}

void ExecutionController::startRequested()
{
    if(!requested || pending) return;
    requested = false;
    submittedRevision = project.revision();
    submittedGeneration = generation;
    if(!project.diagnostics().empty()) {
        published=ExecutionResult{};
        published->diagnostics=project.diagnostics();
        message="Graph has unsupported content or missing inputs";
        Q_EMIT updated();
        return;
    }
    try {
        pending = executor.submit(project.graph(), project.registry(), factory, project.resultGroups(),executionOptions);
        publishedProgress=pending->progress->snapshot();
        message = "Running...";
        completion.start();
    } catch(const std::exception& error) {
        message = "Execution error: " + QString::fromUtf8(error.what());
    }
    Q_EMIT updated();
}

void ExecutionController::poll()
{
    if(!pending) return;
    bool current = submittedRevision == project.revision() && submittedGeneration == generation;
    if(current) {
        auto snapshot=pending->progress->snapshot();
        if(!publishedProgress || snapshot.sequence!=publishedProgress->sequence || !snapshot.finished) {
            publishedProgress=std::move(snapshot);
            if(!pending->cancellation->load())
                message=QString("Running: %1/%2 steps finished").arg(qulonglong(publishedProgress->completed)).arg(qulonglong(publishedProgress->total));
            Q_EMIT updated();
        }
    }
    // A synchronous progress observer can edit the project or request a new run.
    // Recheck after emitting updated before publishing a ready final future.
    current = submittedRevision == project.revision() && submittedGeneration == generation;
    if(pending->result.wait_for(std::chrono::seconds(0)) != std::future_status::ready) return;
    try {
        auto value = pending->result.get();
        if(current) {
            publishedProgress=pending->progress->snapshot();
            if(pending->cancellation->load()) publishedProgress->cancelled=true;
        }
        if(current && !value.cancelled && !pending->cancellation->load()) {
            message = value.succeeded() ? "Complete" : "Graph has errors";
            published = std::move(value);
        } else if(current) message = "Cancelled";
    } catch(const std::exception& error) {
        if(current) message = "Execution error: " + QString::fromUtf8(error.what());
    }
    pending.reset();
    completion.stop();
    Q_EMIT updated();
    if(requested && !debounce.isActive()) startRequested();
}
} // namespace smartflow
