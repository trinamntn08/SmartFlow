#include "PipelineExecution.h"

#include <tp_pipeline/PipelineManager.h>
#include <tp_pipeline/StepDelegate.h>
#include <tp_utils/Progress.h>
#include <algorithm>
#include <queue>
#include <unordered_set>

namespace smartflow {
namespace {
using namespace tp_pipeline;
using tp_utils::StringID;

std::vector<std::string> validate(const PipelineDetails& graph, const StepDelegateMap& delegates)
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
    return errors;
}

ExecutionResult execute(PipelineDetails& graph, const StepDelegateMap& delegates,
                        const tp_data::CollectionFactory& factory,
                        const std::atomic_bool& cancellation, tp_task_queue::Task& task)
{
    ExecutionResult result;
    auto cancelled = [&] { return cancellation.load() || task.shouldFinish(); };
    if(cancelled()) { result.cancelled = true; return result; }
    result.diagnostics = validate(graph, delegates);
    if(!result.diagnostics.empty()) return result;

    // Do not fix up parameters or prune connections during execution.
    PipelineManager manager(&graph, &delegates, &factory, false);
    tp_utils::Progress progress(std::function<bool()>([&] { return !cancelled(); }), "Run graph");
    progress.setPrintToConsole(false);
    tp_utils::ParrallelProgress parallel(&progress);
    manager.startExecution();
    while(auto* context = manager.takeNextAvailableStep(&parallel)) {
        if(cancelled()) break;
        StepResult stepResult;
        const bool upstreamFailed = std::any_of(context->dependsOn.begin(), context->dependsOn.end(),
                                               [](auto* parent) { return !parent->runOk; });
        if(upstreamFailed) {
            stepResult.error = "Upstream step failed";
        } else {
            try {
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
            if(context->runOk) stepResult.output = context->stepOutput->output();
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

bool ExecutionResult::succeeded() const
{
    return !cancelled && diagnostics.empty() &&
        std::all_of(steps.begin(), steps.end(), [](const auto& entry) {
            return entry.second.state == StepState::Succeeded;
        });
}

ExecutionHandle PipelineExecution::submit(const tp_pipeline::PipelineDetails& graph,
    std::shared_ptr<const tp_pipeline::StepDelegateMap> delegates,
    std::shared_ptr<const tp_data::CollectionFactory> factory)
{
    // Snapshot on the owner thread before the worker can see the graph.
    auto snapshot = std::make_shared<tp_pipeline::PipelineDetails>(graph);
    auto cancellation = std::make_shared<std::atomic_bool>(false);
    auto promise = std::make_shared<std::promise<ExecutionResult>>();
    auto future = promise->get_future();
    queue.addTask(new tp_task_queue::Task("Run graph",
        [snapshot, delegates, factory, cancellation, promise](tp_task_queue::Task& task) {
            try {
                if(!delegates || !factory) throw std::invalid_argument("Missing execution registry or factory");
                promise->set_value(execute(*snapshot, *delegates, *factory, *cancellation, task));
            } catch(...) {
                promise->set_exception(std::current_exception());
            }
            return tp_task_queue::RunAgain::No;
        }));
    return {std::move(cancellation), std::move(future)};
}
} // namespace smartflow
