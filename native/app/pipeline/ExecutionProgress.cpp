#include "PipelineExecution.h"
#include <algorithm>
#include <cmath>

namespace smartflow {
namespace {
bool terminal(StepState state) {
    return state==StepState::Succeeded || state==StepState::Failed ||
           state==StepState::Skipped || state==StepState::Cancelled;
}
}
ExecutionProgress::ExecutionProgress(const tp_pipeline::PipelineDetails& graph, std::vector<ResultGroup> groups)
    : groups(std::move(groups))
{
    static std::atomic<uint64_t> next{1};
    value.runId=next.fetch_add(1);
    value.total=graph.steps().size();
    for(const auto* step : graph.steps()) value.steps.emplace(step->id(),StepProgress{});
}
void ExecutionProgress::state(const tp_utils::StringID& id, StepState state, std::string error)
{
    std::lock_guard<std::mutex> lock(mutex);
    auto& step=value.steps.at(id);
    if(terminal(step.state)) return;
    const double now=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-origin).count();
    if(state==StepState::Ready && !step.readyMs) step.readyMs=now;
    if(state==StepState::Running && !step.startedMs) { step.startedMs=now; if(!step.readyMs) step.readyMs=now; }
    if(terminal(state) && step.readyMs) step.endedMs=now;
    step.state=state; step.error=std::move(error);
    if(terminal(state)) { ++value.completed; step.fraction=1.0; }
    ++value.sequence;
}
void ExecutionProgress::fraction(const tp_utils::StringID& id, double fraction)
{
    if(!std::isfinite(fraction)) return;
    std::lock_guard<std::mutex> lock(mutex);
    auto& step=value.steps.at(id);
    if(terminal(step.state)) return;
    fraction=std::clamp(fraction,0.0,1.0);
    if(!step.fraction || fraction>*step.fraction) { step.fraction=fraction; ++value.sequence; }
}
void ExecutionProgress::finish(bool cancelled, std::vector<std::string> diagnostics)
{
    std::lock_guard<std::mutex> lock(mutex);
    value.elapsedMs=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-origin).count();
    value.cancelled=cancelled; value.finished=true; value.diagnostics=std::move(diagnostics);
    for(auto& entry : value.steps) if(!terminal(entry.second.state)) {
        entry.second.state=cancelled ? StepState::Cancelled : StepState::Skipped;
        if(entry.second.readyMs) entry.second.endedMs=value.elapsedMs;
        entry.second.fraction=1.0; ++value.completed;
    }
    ++value.sequence;
}
ExecutionProgressSnapshot ExecutionProgress::snapshot() const
{
    std::lock_guard<std::mutex> lock(mutex);
    auto output=value;
    if(!output.finished) output.elapsedMs=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-origin).count();
    output.nodes=output.steps;
    for(const auto& group : groups) {
        StepProgress combined;
        bool done=true, failed=false, cancelled=false, skipped=false, running=false, ready=false;
        double fraction=0;
        for(const auto& id : group.steps) {
            const auto found=output.steps.find(id);
            if(found==output.steps.end()) { done=false; continue; }
            const auto& step=found->second;
            if(step.readyMs && (!combined.readyMs || *step.readyMs<*combined.readyMs)) combined.readyMs=step.readyMs;
            if(step.startedMs && (!combined.startedMs || *step.startedMs<*combined.startedMs)) combined.startedMs=step.startedMs;
            if(step.endedMs && (!combined.endedMs || *step.endedMs>*combined.endedMs)) combined.endedMs=step.endedMs;
            done=done && terminal(step.state);
            failed=failed || step.state==StepState::Failed;
            cancelled=cancelled || step.state==StepState::Cancelled;
            skipped=skipped || step.state==StepState::Skipped;
            running=running || step.state==StepState::Running;
            ready=ready || step.state==StepState::Ready;
            fraction+=step.fraction.value_or(0);
            if(!step.error.empty()) combined.error+=id.toString()+": "+step.error+"\n";
            output.nodes.erase(id);
        }
        if(!done) combined.endedMs.reset();
        if(done) combined.state=failed ? StepState::Failed : cancelled ? StepState::Cancelled :
            skipped ? StepState::Skipped : StepState::Succeeded;
        else combined.state=running ? StepState::Running : ready ? StepState::Ready : StepState::Waiting;
        if(!group.steps.empty()) combined.fraction=fraction/double(group.steps.size());
        output.nodes[group.nodeId]=std::move(combined);
    }
    return output;
}
} // namespace smartflow
