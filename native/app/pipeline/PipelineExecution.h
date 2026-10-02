#pragma once
#include <ExecutionPolicy.h>

#include <tp_pipeline/PipelineDetails.h>
#include <tp_pipeline/StepDelegateMap.h>
#include <tp_data/CollectionFactory.h>
#include <tp_task_queue/TaskQueue.h>
#include <atomic>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>

namespace smartflow {

enum class StepState { Waiting, Ready, Running, Succeeded, Failed, Skipped, Cancelled };

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

// Immutable mapping from private expanded steps back to one visible node.
// Output identities are shared without renaming domain members.
struct ResultGroup {
    tp_utils::StringID nodeId;
    std::vector<tp_utils::StringID> steps;
    std::vector<tp_utils::StringID> outputs;
};

struct StepProgress {
    StepState state = StepState::Waiting;
    std::optional<double> fraction;
    std::string error;
};
struct ExecutionProgressSnapshot {
    uint64_t runId = 0, sequence = 0;
    size_t total = 0, completed = 0;
    bool finished = false, cancelled = false;
    std::vector<std::string> diagnostics;
    // Counts use compiled steps; nodes aggregate components without double counting.
    std::unordered_map<tp_utils::StringID, StepProgress> steps, nodes;
};
class ExecutionProgress {
public:
    ExecutionProgress(const tp_pipeline::PipelineDetails& graph, std::vector<ResultGroup> groups);
    ExecutionProgressSnapshot snapshot() const;
    void state(const tp_utils::StringID& id, StepState state, std::string error = {});
    void fraction(const tp_utils::StringID& id, double fraction);
    void finish(bool cancelled, std::vector<std::string> diagnostics = {});
private:
    mutable std::mutex mutex;
    ExecutionProgressSnapshot value;
    std::vector<ResultGroup> groups;
};
struct ExecutionHandle {
    std::shared_ptr<std::atomic_bool> cancellation;
    std::future<ExecutionResult> result;
    std::shared_ptr<ExecutionProgress> progress;
    void cancel() const { cancellation->store(true); }
};

// Migration adapter, not the persisted SmartFlow model or public extension API.
// Submit from the graph's owning thread. Registry/factory must remain immutable
// after submission. The coordinator owns dependencies and result publication.
class PipelineExecution {
public:
    ExecutionHandle submit(
        const tp_pipeline::PipelineDetails& graph,
        std::shared_ptr<const tp_pipeline::StepDelegateMap> delegates,
        std::shared_ptr<const tp_data::CollectionFactory> factory,
        std::vector<ResultGroup> groups = {}, ExecutionOptions options = {});

private:
    tp_task_queue::TaskQueue queue{"SmartFlow pipeline", 1};
};

} // namespace smartflow
