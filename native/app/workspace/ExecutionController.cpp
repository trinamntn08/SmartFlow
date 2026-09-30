#include "ExecutionController.h"
#include <chrono>

namespace smartflow {
ExecutionController::ExecutionController(GraphProject& project,
    std::shared_ptr<const tp_data::CollectionFactory> factory)
    : project(project), factory(std::move(factory))
{
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

void ExecutionController::run()
{
    ++generation;
    published.reset();
    requested = true;
    debounce.stop();
    if(pending) { pending->cancel(); message = "Waiting for previous run to stop..."; Q_EMIT updated(); }
    else startRequested();
}

void ExecutionController::cancel()
{
    ++generation;
    requested = false;
    debounce.stop();
    if(pending) pending->cancel();
    published.reset();
    message = "Cancelled";
    Q_EMIT updated();
}

void ExecutionController::startRequested()
{
    if(!requested || pending) return;
    requested = false;
    submittedRevision = project.revision();
    submittedGeneration = generation;
    try {
        pending = executor.submit(project.graph(), project.registry(), factory);
        message = "Running...";
        completion.start();
    } catch(const std::exception& error) {
        message = "Execution error: " + QString::fromUtf8(error.what());
    }
    Q_EMIT updated();
}

void ExecutionController::poll()
{
    if(!pending || pending->result.wait_for(std::chrono::seconds(0)) != std::future_status::ready) return;
    const bool current = submittedRevision == project.revision() && submittedGeneration == generation;
    try {
        auto value = pending->result.get();
        if(current && !value.cancelled) {
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
