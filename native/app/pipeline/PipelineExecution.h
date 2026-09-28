#pragma once

#include <tp_pipeline/PipelineDetails.h>
#include <tp_pipeline/StepDelegateMap.h>
#include <tp_data/CollectionFactory.h>
#include <tp_task_queue/TaskQueue.h>
#include <atomic>
#include <future>
#include <memory>
#include <unordered_map>

namespace smartflow {

enum class StepState { Succeeded, Failed, Skipped };

struct StepResult {
    StepState state = StepState::Skipped;
    std::shared_ptr<const tp_data::Collection> output;
    std::string error;
};

struct ExecutionResult {
    bool cancelled = false;
    std::vector<std::string> diagnostics;
    std::unordered_map<tp_utils::StringID, StepResult> steps;
    bool succeeded() const;
};

struct ExecutionHandle {
    std::shared_ptr<std::atomic_bool> cancellation;
    std::future<ExecutionResult> result;
    void cancel() const { cancellation->store(true); }
};

// Migration adapter, not the persisted SmartFlow model or public extension API.
// Submit from the graph's owning thread. Registry/factory must remain immutable
// after submission. Delegates execute serially on the queue's worker thread.
class PipelineExecution {
public:
    ExecutionHandle submit(
        const tp_pipeline::PipelineDetails& graph,
        std::shared_ptr<const tp_pipeline::StepDelegateMap> delegates,
        std::shared_ptr<const tp_data::CollectionFactory> factory);

private:
    tp_task_queue::TaskQueue queue{"SmartFlow pipeline", 1};
};

} // namespace smartflow
