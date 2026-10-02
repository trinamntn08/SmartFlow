#include "PipelineExecution.h"
#include "ExecutionData.h"

#include <tp_pipeline/PipelineManager.h>
#include <tp_pipeline/StepDelegate.h>
#include <tp_utils/Progress.h>
#include <tp_data/Collection.h>
#include <algorithm>
#include <queue>
#include <unordered_set>

namespace smartflow {
namespace {
using namespace tp_pipeline;
using tp_utils::StringID;

std::vector<std::string> validate(const PipelineDetails& graph, const StepDelegateMap& delegates,
    const std::vector<ResultGroup>& groups)
{
    std::vector<std::string> errors;
    struct Producer { StepDetails* step; StringID type; };
    std::unordered_map<StringID, Producer> producers;
    std::unordered_set<StringID> ids;
    std::unordered_map<StepDetails*, size_t> indegree;
    std::unordered_map<StepDetails*, std::vector<StepDetails*>> consumers;

    auto mappingsValid = [&](StepDetails* step, const std::vector<PortMapping>& mappings,
                             const std::vector<PortDetails>& ports) {
        std::unordered_set<StringID> names;
        if(mappings.size() != ports.size())
            errors.push_back(step->id().toString() + ": port mapping count mismatch");
        for(const auto& mapping : mappings) {
            const auto port = std::find_if(ports.begin(), ports.end(), [&](const auto& p) {
                return p.name == mapping.portName && p.type == mapping.portType;
            });
            if(port == ports.end() || !names.insert(mapping.portName).second)
                errors.push_back(step->id().toString() + ": invalid or duplicate port mapping");
        }
    };

    for(auto* step : graph.steps()) {
        indegree[step] = 0;
        if(!step->id().isValid() || !ids.insert(step->id()).second)
            errors.push_back("Invalid or duplicate step identity");
        const auto* delegate = delegates.stepDelegate(step->delegateName());
        if(!delegate) {
            errors.push_back(step->id().toString() + ": unavailable delegate " + step->delegateName().toString());
            continue;
        }
        if(step->noExec() || !step->overrideOutputs().empty() || !step->fallbackOperationNames().empty())
            errors.push_back(step->id().toString() + ": legacy overrides/fallbacks are unsupported");
        mappingsValid(step, step->inputMapping(), delegate->inPorts());
        mappingsValid(step, step->outputMapping(), delegate->outPorts());
        for(const auto& mapping : step->outputMapping()) {
            if(!mapping.dataName.isValid() ||
               !producers.emplace(mapping.dataName, Producer{step, mapping.portType}).second)
                errors.push_back(step->id().toString() + ": invalid or duplicate output identity");
        }
    }
    for(auto* step : graph.steps()) {
        for(const auto& mapping : step->inputMapping()) {
            const auto producer = producers.find(mapping.dataName);
            if(producer == producers.end()) {
                errors.push_back(step->id().toString() + ": missing input producer for " + mapping.portName.toString());
                continue;
            }
            if(producer->second.type != mapping.portType)
                errors.push_back(step->id().toString() + ": incompatible connection types");
            ++indegree[step];
            consumers[producer->second.step].push_back(step);
        }
    }
    std::queue<StepDetails*> ready;
    for(const auto& item : indegree)
        if(item.second == 0) ready.push(item.first);
    size_t visited = 0;
    while(!ready.empty()) {
        auto* step = ready.front();
        ready.pop();
        ++visited;
        for(auto* consumer : consumers[step])
            if(--indegree[consumer] == 0) ready.push(consumer);
    }
    if(visited != graph.steps().size()) errors.push_back("Cycle detected");
    std::unordered_map<StringID,StringID> owners;
    std::unordered_set<StringID> groupIds;
    for(const auto& group : groups) {
        if(!group.nodeId.isValid() || ids.count(group.nodeId) || !groupIds.insert(group.nodeId).second || group.steps.empty())
            errors.push_back("Invalid component result identity");
        for(const auto& id : group.steps)
            if(!ids.count(id) || !owners.emplace(id,group.nodeId).second) errors.push_back("Invalid or overlapping component result group");
        for(const auto& output : group.outputs) {
            const auto producer=producers.find(output);
            if(producer==producers.end() || std::find(group.steps.begin(),group.steps.end(),producer->second.step->id())==group.steps.end())
                errors.push_back("Invalid exposed component output identity");
        }
    }
    if(!groups.empty() && errors.empty()) {
        std::unordered_map<StringID,size_t> counts;
        std::unordered_map<StringID,std::vector<StringID>> next;
        auto owner=[&](StepDetails* step) { const auto found=owners.find(step->id()); return found==owners.end() ? step->id() : found->second; };
        for(auto* step : graph.steps()) counts[owner(step)]=0;
        for(auto* step : graph.steps()) for(const auto& input : step->inputMapping()) {
            const auto from=owner(producers.at(input.dataName).step), to=owner(step);
            if(from!=to) { ++counts[to]; next[from].push_back(to); }
        }
        std::queue<StringID> available;
        for(const auto& [id,count] : counts) if(!count) available.push(id);
        size_t completed=0;
        while(!available.empty()) {
            const auto id=available.front(); available.pop(); ++completed;
            for(const auto& to : next[id]) if(!--counts[to]) available.push(to);
        }
        if(completed!=counts.size()) errors.push_back("Component dependency cycle detected");
    }
    return errors;
}

void bindComponentDependencies(PipelineManager& manager, const PipelineDetails& graph,
    const std::vector<ResultGroup>& groups)
{
    // Public mutable contexts let the adapter add dependency barriers without
    // changing vendor code or inventing data ports. No output escapes a failed
    // component, and its body waits for all exposed inputs before starting.
    for(const auto& group : groups) {
        std::unordered_set<StepContext*> members, inputs;
        for(auto* step : graph.steps())
            if(std::find(group.steps.begin(),group.steps.end(),step->id())!=group.steps.end()) members.insert(manager.stepContext(step));
        for(auto* member : members) for(auto* parent : member->dependsOn) if(!members.count(parent)) inputs.insert(parent);
        for(auto* member : members) member->dependsOn.insert(inputs.begin(),inputs.end());
        for(auto* step : graph.steps()) {
            auto* context=manager.stepContext(step);
            if(members.count(context)) continue;
            if(std::any_of(context->dependsOn.begin(),context->dependsOn.end(),[&](auto* parent) { return members.count(parent); }))
                context->dependsOn.insert(members.begin(),members.end());
        }
    }
}

ExecutionResult execute(PipelineDetails& graph, const StepDelegateMap& delegates,
                        const tp_data::CollectionFactory& factory,
                        const std::atomic_bool& cancellation, tp_task_queue::Task& task,
                        const std::vector<ResultGroup>& groups)
{
    ExecutionResult result;
    auto cancelled = [&] { return cancellation.load() || task.shouldFinish(); };
    if(cancelled()) { result.cancelled = true; return result; }
    result.diagnostics = validate(graph, delegates, groups);
    if(!result.diagnostics.empty()) return result;

    // Do not fix up parameters or prune connections during execution.
    PipelineManager manager(&graph, &delegates, &factory, false);
    tp_utils::Progress progress(std::function<bool()>([&] { return !cancelled(); }), "Run graph");
    progress.setPrintToConsole(false);
    tp_utils::ParrallelProgress parallel(&progress);
    manager.startExecution();
    bindComponentDependencies(manager,graph,groups);
    while(auto* context = manager.takeNextAvailableStep(&parallel)) {
        if(cancelled()) break;
        // The copied parallel progress child reads only the parent's cached
        // stop flag. Refresh its cancellation callback whenever a delegate polls,
        // including polls from nested progress children.
        context->progress->pollCallbacks.addCallback([&progress] { progress.poll(); });
        StepResult stepResult;
        const bool upstreamFailed = std::any_of(context->dependsOn.begin(), context->dependsOn.end(),
                                               [](auto* parent) { return !parent->runOk; });
        if(upstreamFailed) {
            stepResult.error = "Upstream step failed";
        } else {
            try {
                context->stepInput = cloneExecutionData(*context->stepInput, factory);
                context->runOk = context->stepDelegate->executeStep(context);
                const auto& output = context->stepOutput->output();
                for(const auto& mapping : context->stepDetails->outputMapping()) {
                    const auto& member = output->member(mapping.dataName);
                    if(!member || member->type() != mapping.portType) {
                        context->progress->addError("Missing or incompatible output: " + mapping.portName.toString());
                        context->runOk = false;
                    }
                }
                stepResult.error = context->progress->compileErrors();
                context->runOk = context->runOk && stepResult.error.empty();
            } catch(const std::exception& error) {
                context->runOk = false;
                stepResult.error = error.what();
            } catch(...) {
                context->runOk = false;
                stepResult.error = "Unknown delegate exception";
            }
            stepResult.state = context->runOk ? StepState::Succeeded : StepState::Failed;
            if(context->runOk) {
                try { stepResult.output = cloneExecutionData(*context->stepOutput->output(), factory); }
                catch(const std::exception& error) {
                    context->runOk = false;
                    stepResult.state = StepState::Failed;
                    stepResult.error = error.what();
                }
            }
            else if(stepResult.error.empty()) stepResult.error = "Delegate returned failure";
        }
        manager.returnCompletedStep(context);
        result.steps.emplace(context->stepDetails->id(), std::move(stepResult));
    }
    if(cancelled()) {
        result.cancelled = true;
        result.steps.clear(); // Never publish partial output from a cancelled run.
    } else if(result.steps.size() != graph.steps().size()) {
        result.diagnostics.push_back("Execution stopped before all steps completed");
    }
    return result;
}
} // namespace

namespace {
void groupResults(ExecutionResult& result, const std::vector<ResultGroup>& groups)
{
    if(result.cancelled || !result.diagnostics.empty()) return;
    for(const auto& group : groups) {
        StepResult combined;
        combined.state=StepState::Succeeded;
        auto output=std::make_shared<tp_data::Collection>();
        for(const auto& id : group.steps) {
            const auto found=result.steps.find(id);
            if(found==result.steps.end() || found->second.state!=StepState::Succeeded) {
                if(found==result.steps.end() || found->second.state==StepState::Failed) combined.state=StepState::Failed;
                else if(combined.state!=StepState::Failed) combined.state=StepState::Skipped;
                combined.error+=id.toString()+": "+(found==result.steps.end() ? "Missing component step result" : found->second.error)+"\n";
            }
        }
        if(combined.state==StepState::Succeeded) for(const auto& name : group.outputs) {
            std::shared_ptr<tp_data::AbstractMember> member;
            for(const auto& id : group.steps) {
                const auto& source=result.steps.at(id).output;
                if(source && source->member(name)) { member=source->member(name); break; }
            }
            if(!member) { combined.state=StepState::Failed; combined.error="Missing exposed component output"; break; }
            if(!output->member(name)) output->addMember(member);
        }
        if(combined.state==StepState::Succeeded) combined.output=std::move(output);
        for(const auto& id : group.steps) result.steps.erase(id);
        result.steps.emplace(group.nodeId,std::move(combined));
    }
}
}

bool ExecutionResult::succeeded() const
{
    return !cancelled && diagnostics.empty() &&
        std::all_of(steps.begin(), steps.end(), [](const auto& entry) {
            return entry.second.state == StepState::Succeeded;
        });
}

ExecutionHandle PipelineExecution::submit(const tp_pipeline::PipelineDetails& graph,
    std::shared_ptr<const tp_pipeline::StepDelegateMap> delegates,
    std::shared_ptr<const tp_data::CollectionFactory> factory, std::vector<ResultGroup> groups)
{
    // Snapshot on the owner thread before the worker can see the graph.
    auto snapshot = std::make_shared<tp_pipeline::PipelineDetails>(graph);
    auto cancellation = std::make_shared<std::atomic_bool>(false);
    auto promise = std::make_shared<std::promise<ExecutionResult>>();
    auto future = promise->get_future();
    queue.addTask(new tp_task_queue::Task("Run graph",
        [snapshot, delegates, factory, cancellation, promise, groups=std::move(groups)](tp_task_queue::Task& task) {
            try {
                if(!delegates || !factory) throw std::invalid_argument("Missing execution registry or factory");
                auto result=execute(*snapshot, *delegates, *factory, *cancellation, task,groups);
                groupResults(result,groups);
                promise->set_value(std::move(result));
            } catch(...) {
                promise->set_exception(std::current_exception());
            }
            return tp_task_queue::RunAgain::No;
        }));
    return {std::move(cancellation), std::move(future)};
}
} // namespace smartflow
