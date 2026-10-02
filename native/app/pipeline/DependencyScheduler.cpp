#include "DependencyScheduler.h"
#include "ExecutionData.h"
#include <NodeWork.h>
#include <tp_pipeline/StepDelegate.h>
#include <tp_utils/Progress.h>
#include <algorithm>
#include <condition_variable>
#include <deque>
#include <map>
#include <mutex>
#include <set>
#include <thread>

namespace smartflow {
namespace {
using namespace tp_pipeline;
using tp_utils::StringID;

struct Completion { size_t index; StepResult result; };
class NodeWorkers {
public:
    std::atomic_bool abort{false};
    explicit NodeWorkers(size_t count)
    {
        try {
            for(size_t i=0; i<count; ++i) threads.emplace_back([this] {
                for(;;) {
                    std::function<Completion()> work;
                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        available.wait(lock, [&] { return stopping || !jobs.empty(); });
                        if(stopping && jobs.empty()) return;
                        work=std::move(jobs.front()); jobs.pop_front();
                    }
                    auto completion=work();
                    {
                        std::lock_guard<std::mutex> lock(mutex);
                        completed.push_back(std::move(completion));
                    }
                    available.notify_all();
                }
            });
        } catch(...) { shutdown(); throw; }
    }
    ~NodeWorkers() { shutdown(); }
    void enqueue(std::function<Completion()> work)
    {
        { std::lock_guard<std::mutex> lock(mutex); jobs.push_back(std::move(work)); }
        available.notify_all();
    }
    std::deque<Completion> wait()
    {
        std::unique_lock<std::mutex> lock(mutex);
        available.wait_for(lock, std::chrono::milliseconds(10), [&] { return !completed.empty(); });
        std::deque<Completion> output;
        output.swap(completed);
        return output;
    }
private:
    void shutdown()
    {
        abort.store(true);
        { std::lock_guard<std::mutex> lock(mutex); stopping=true; }
        available.notify_all();
        for(auto& thread : threads) if(thread.joinable()) thread.join();
    }
    std::mutex mutex;
    std::condition_variable available;
    bool stopping=false;
    std::deque<std::function<Completion()>> jobs;
    std::deque<Completion> completed;
    std::vector<std::thread> threads;
};

// Process-local resources also cover windows/executors sharing a registry.
// Acquisition and release both occur on the coordinator thread, never a worker.
std::vector<std::shared_ptr<std::mutex>> resourceMutexes(const NodeExecutionPolicy& policy)
{
    static std::mutex registryMutex;
    static std::map<std::string, std::weak_ptr<std::mutex>> registry;
    std::set<std::string> keys;
    if(!policy.reentrant) keys.insert("unaudited-delegates");
    for(const auto& resource : policy.exclusiveResources) keys.insert("resource:"+resource);
    std::lock_guard<std::mutex> lock(registryMutex);
    std::vector<std::shared_ptr<std::mutex>> output;
    for(const auto& key : keys) {
        auto resource=registry[key].lock();
        if(!resource) { resource=std::make_shared<std::mutex>(); registry[key]=resource; }
        output.push_back(std::move(resource));
    }
    return output;
}

StepResult runNode(StepContext& context, const std::function<bool()>& cancelled,
    const std::shared_ptr<ExecutionProgress>& status, size_t threads)
{
    StepResult result;
    result.state=StepState::Failed;
    try {
        detail::NodeWorkContext budget{threads,cancelled};
        detail::NodeWorkScope scope(budget);
        tp_utils::Progress progress(cancelled ? std::function<bool()>([&] { return !cancelled(); }) :
            std::function<bool()>([] { return true; }), "Execute node");
        progress.setPrintToConsole(false);
        progress.changed.addCallback([&] { status->fraction(context.stepDetails->id(),progress.progress()); });
        context.progress=&progress;
        if(cancelled()) { result.error="Cancelled"; context.progress=nullptr; return result; }
        bool ok=context.stepDelegate->executeStep(&context);
        for(const auto& mapping : context.stepDetails->outputMapping()) {
            const auto& member=context.stepOutput->output()->member(mapping.dataName);
            if(!member || member->type()!=mapping.portType || member->name()!=mapping.dataName) {
                progress.addError("Missing or incompatible output: "+mapping.portName.toString());
                ok=false;
            }
        }
        result.error=progress.compileErrors();
        if(ok && result.error.empty()) {
            result.state=StepState::Succeeded;
            result.output=context.stepOutput->output();
        } else if(result.error.empty()) result.error="Delegate returned failure";
    } catch(const std::exception& error) { result.error=error.what(); }
      catch(...) { result.error="Unknown delegate exception"; }
    context.progress=nullptr;
    return result;
}
} // namespace

ExecutionResult executeScheduled(PipelineDetails& graph, const StepDelegateMap& delegates,
    const tp_data::CollectionFactory& factory, const std::vector<ResultGroup>& groups,
    const ExecutionOptions& options, const std::function<bool()>& cancelled,
    const std::shared_ptr<ExecutionProgress>& progress)
{
    ExecutionResult result;
    const size_t count=graph.steps().size();
    std::vector<StepContext> contexts(count);
    std::unordered_map<StringID,size_t> indices, producers;
    std::vector<std::vector<size_t>> owners(count);
    std::vector<NodeExecutionPolicy> policies(count);
    std::vector<std::vector<std::shared_ptr<std::mutex>>> resources(count);
    for(size_t i=0; i<count; ++i) {
        auto& context=contexts[i];
        context.stepDetails=graph.steps()[i];
        context.stepDelegate=delegates.stepDelegate(context.stepDetails->delegateName());
        context.stepOutput=std::make_shared<StepOutput>(context.stepDelegate,context.stepDetails);
        indices.emplace(context.stepDetails->id(),i);
        for(const auto& output : context.stepDetails->outputMapping()) producers.emplace(output.dataName,i);
        owners[i]={i};
        const auto policy=options.policies.find(context.stepDetails->delegateName());
        if(policy!=options.policies.end()) policies[i]=policy->second;
        resources[i]=resourceMutexes(policies[i]);
    }
    for(const auto& group : groups) {
        std::vector<size_t> members;
        for(const auto& id : group.steps) members.push_back(indices.at(id));
        for(const auto index : members) owners[index]=members;
    }
    std::vector<std::set<size_t>> parents(count);
    for(size_t to=0; to<count; ++to) for(const auto& input : contexts[to].stepDetails->inputMapping()) {
        const auto from=producers.at(input.dataName);
        parents[to].insert(from);
        if(owners[from]!=owners[to])
            for(const auto consumer : owners[to]) for(const auto producer : owners[from]) parents[consumer].insert(producer);
    }
    std::vector<size_t> remaining(count);
    std::vector<std::vector<size_t>> next(count);
    for(size_t i=0; i<count; ++i) {
        remaining[i]=parents[i].size();
        for(const auto parent : parents[i]) next[parent].push_back(i);
    }
    // Validate the actual barrier-expanded plan before launching any delegate.
    auto check=remaining;
    std::deque<size_t> checkReady;
    for(size_t i=0; i<count; ++i) if(!check[i]) checkReady.push_back(i);
    size_t visited=0;
    while(!checkReady.empty()) {
        const auto i=checkReady.front(); checkReady.pop_front(); ++visited;
        for(const auto child : next[i]) if(!--check[child]) checkReady.push_back(child);
    }
    if(visited!=count) { result.diagnostics.push_back("Component barrier dependency cycle detected"); return result; }
    if(!count) { result.cancelled=cancelled(); return result; }

    std::set<size_t> ready;
    for(size_t i=0; i<count; ++i) if(!remaining[i]) {
        ready.insert(i); progress->state(contexts[i].stepDetails->id(),StepState::Ready);
    }
    std::vector<StepResult> outputs(count);
    std::vector<std::vector<std::unique_lock<std::mutex>>> locks(count);
    size_t running=0, finished=0, reserved=0;
    std::vector<size_t> reservations(count);
    const size_t workers=options.mode==ExecutionMode::Sequential ? 1 : std::min(options.maxThreads,count);
    NodeWorkers pool(workers);
    auto stopping=[&] { return cancelled() || pool.abort.load(); };
    auto complete=[&](size_t index, StepResult value) {
        if(value.state==StepState::Succeeded) {
            try { value.output=cloneExecutionData(*value.output,factory); }
            catch(const std::exception& error) { value.state=StepState::Failed; value.output.reset(); value.error=error.what(); }
            catch(...) { value.state=StepState::Failed; value.output.reset(); value.error="Unknown output clone exception"; }
        }
        contexts[index].runOk=value.state==StepState::Succeeded;
        contexts[index].runComplete=true;
        progress->state(contexts[index].stepDetails->id(),value.state,value.error);
        outputs[index]=std::move(value);
        ++finished;
        for(const auto child : next[index]) if(!--remaining[child]) {
            ready.insert(child); progress->state(contexts[child].stepDetails->id(),StepState::Ready);
        }
    };
    while(finished<count) {
        for(auto it=ready.begin(); it!=ready.end() && running<workers && !stopping();) {
            const auto index=*it;
            if(std::any_of(parents[index].begin(),parents[index].end(),[&](size_t parent) {
                return outputs[parent].state!=StepState::Succeeded;
            })) {
                it=ready.erase(it);
                StepResult skipped; skipped.error="Upstream step failed";
                complete(index,std::move(skipped));
                continue;
            }
            const size_t demand=std::min(options.maxThreads,policies[index].threadLimit);
            if(reserved+demand>options.maxThreads) { ++it; continue; }
            std::vector<std::unique_lock<std::mutex>> acquired;
            bool locked=true;
            for(const auto& resource : resources[index]) {
                acquired.emplace_back(*resource,std::try_to_lock);
                if(!acquired.back().owns_lock()) { locked=false; break; }
            }
            if(!locked) { ++it; continue; }
            it=ready.erase(it);
            try {
                auto input=std::make_shared<tp_data::Collection>();
                for(const auto& mapping : contexts[index].stepDetails->inputMapping()) {
                    const auto& member=outputs[producers.at(mapping.dataName)].output->member(mapping.dataName);
                    if(!input->member(mapping.dataName)) input->addMember(member);
                }
                contexts[index].stepInput=cloneExecutionData(*input,factory);
            } catch(const std::exception& error) {
                StepResult failed; failed.state=StepState::Failed; failed.error=error.what();
                complete(index,std::move(failed)); continue;
            } catch(...) {
                StepResult failed; failed.state=StepState::Failed; failed.error="Unknown input clone exception";
                complete(index,std::move(failed)); continue;
            }
            if(stopping()) break;
            locks[index]=std::move(acquired);
            contexts[index].runStarted=true;
            progress->state(contexts[index].stepDetails->id(),StepState::Running);
            ++running;
            reservations[index]=demand; reserved+=demand;
            pool.enqueue([&,index,stopping,progress,demand] { return Completion{index,runNode(contexts[index],stopping,progress,demand)}; });
        }
        if(stopping() && !running) break;
        if(finished==count) break;
        for(auto& completion : pool.wait()) {
            --running;
            reserved-=reservations[completion.index];
            if(stopping()) {
                completion.result.state=StepState::Cancelled;
                completion.result.output.reset(); completion.result.error="Cancelled";
            }
            complete(completion.index,std::move(completion.result));
            locks[completion.index].clear();
        }
    }
    result.cancelled=stopping();
    if(!result.cancelled)
        for(size_t i=0; i<count; ++i) result.steps.emplace(contexts[i].stepDetails->id(),std::move(outputs[i]));
    return result;
}
} // namespace smartflow
