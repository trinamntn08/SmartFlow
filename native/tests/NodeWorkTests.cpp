#include "pipeline/PipelineExecution.h"
#include <NodeWork.h>
#include <tp_pipeline/StepDelegate.h>
#include <tp_data/members/NumberMember.h>
#include <QtTest/QtTest>
#include <condition_variable>
#include <mutex>

using namespace smartflow;
using namespace tp_pipeline;
using namespace std::chrono_literals;
namespace {
struct Measure {
    std::mutex mutex;
    std::condition_variable changed;
    bool open=false;
    size_t active=0,maximum=0,invocations=0;
    bool waitFor(size_t count) {
        std::unique_lock<std::mutex> lock(mutex);
        return changed.wait_for(lock,2s,[&] { return active>=count; });
    }
    void release() { { std::lock_guard<std::mutex> lock(mutex); open=true; } changed.notify_all(); }
};
class InternalWork final : public StepDelegate {
public:
    InternalWork(const char* name, std::shared_ptr<Measure> measure={},bool fail=false)
        : StepDelegate(name,{},{},{{"out",tp_data::doubleSID()}}),measure(std::move(measure)),fail(fail) {}
    bool executeStep(StepContext* context) const override {
        if(measure) { std::lock_guard<std::mutex> lock(measure->mutex); ++measure->invocations; }
        std::vector<double> values(64);
        const bool completed=parallelFor(values.size(),[&](size_t index) {
            if(fail && index==0) throw std::runtime_error("Internal work failed");
            if(measure) {
                std::unique_lock<std::mutex> lock(measure->mutex);
                ++measure->active; measure->maximum=std::max(measure->maximum,measure->active); measure->changed.notify_all();
                const auto deadline=std::chrono::steady_clock::now()+5s;
                while(!measure->open && !nodeWorkCancelled() && std::chrono::steady_clock::now()<deadline)
                    measure->changed.wait_for(lock,2ms);
                --measure->active;
            }
            // Nested helpers must stay on this thread and within the reservation.
            const auto thread=std::this_thread::get_id();
            double value=0;
            parallelFor(3,[&](size_t) {
                if(std::this_thread::get_id()!=thread) throw std::runtime_error("Nested helper spawned work");
                value+=1;
            });
            values[index]=value;
        });
        if(!completed) return false;
        auto output=std::make_shared<tp_data::DoubleMember>();
        output->data=0;
        for(const auto value : values) output->data+=value;
        return context->stepOutput->addSharedMember("out",output,context->progress);
    }
private:
    std::shared_ptr<Measure> measure;
    bool fail;
};
struct Graph {
    PipelineDetails graph;
    std::shared_ptr<StepDelegateMap> delegates=std::make_shared<StepDelegateMap>();
    std::shared_ptr<tp_data::CollectionFactory> factory=std::make_shared<tp_data::CollectionFactory>();
    ExecutionOptions options;
    Graph(std::shared_ptr<Measure> measure={},bool fail=false) {
        delegates->addStepDelegate(new InternalWork("internal",measure,fail));
        delegates->addStepDelegate(new InternalWork("single",measure));
        tp_data::createCollectionFactories(*factory); factory->finalize();
        options.mode=ExecutionMode::Parallel; options.maxThreads=4;
        options.policies["internal"]={true,{},3}; options.policies["single"]={true,{},1};
    }
    StepDetails* add(const char* type) {
        auto* step=new StepDetails(type);
        step->setOutputMapping({{tp_data::doubleSID(),"out",step->id().toString()+"-out",{}}});
        graph.addStep(step); return step;
    }
};
ExecutionResult get(ExecutionHandle& handle) {
    if(handle.result.wait_for(10s)!=std::future_status::ready) throw std::runtime_error("Internal node work timed out");
    return handle.result.get();
}
}
class NodeWorkTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void mixedWorkSharesRunBudget() {
        auto measure=std::make_shared<Measure>(); Graph f(measure);
        f.add("internal"); f.add("internal"); f.add("single");
        PipelineExecution executor; auto handle=executor.submit(f.graph,f.delegates,f.factory,{},f.options);
        const bool full=measure->waitFor(4);
        size_t invocations; { std::lock_guard<std::mutex> lock(measure->mutex); invocations=measure->invocations; }
        measure->release(); const auto result=get(handle);
        QVERIFY(full); QCOMPARE(invocations,size_t(2)); QVERIFY(result.succeeded());
        QCOMPARE(measure->maximum,size_t(4)); QCOMPARE(measure->active,size_t(0));
        for(const auto& entry : result.steps)
            QCOMPARE(entry.second.output->members().size(),size_t(1));
    }
    void sequentialNodesMayUseReservedInternalThreads() {
        auto measure=std::make_shared<Measure>(); Graph f(measure);
        f.options.mode=ExecutionMode::Sequential; f.options.maxThreads=2;
        f.add("internal"); f.add("internal");
        PipelineExecution executor; auto handle=executor.submit(f.graph,f.delegates,f.factory,{},f.options);
        const bool full=measure->waitFor(2);
        size_t invocations; { std::lock_guard<std::mutex> lock(measure->mutex); invocations=measure->invocations; }
        measure->release(); QVERIFY(get(handle).succeeded());
        QVERIFY(full); QCOMPARE(invocations,size_t(1)); QCOMPARE(measure->maximum,size_t(2));
    }
    void internalExceptionJoinsBeforeFailurePublication() {
        Graph f({},true); const auto* step=f.add("internal");
        PipelineExecution executor; auto handle=executor.submit(f.graph,f.delegates,f.factory,{},f.options);
        const auto result=get(handle); QVERIFY(!result.succeeded());
        QCOMPARE(result.steps.at(step->id()).state,StepState::Failed);
        QVERIFY(result.steps.at(step->id()).error.find("Internal work failed")!=std::string::npos);
        QVERIFY(!result.steps.at(step->id()).output);
    }
    void cancellationJoinsInternalWorkers() {
        auto measure=std::make_shared<Measure>(); Graph f(measure); f.add("internal"); f.add("single");
        PipelineExecution executor; auto handle=executor.submit(f.graph,f.delegates,f.factory,{},f.options);
        const bool full=measure->waitFor(4); handle.cancel(); const auto result=get(handle);
        QVERIFY(full); QVERIFY(result.cancelled); QVERIFY(result.steps.empty()); QCOMPARE(measure->active,size_t(0));
    }
    void helperOutsideExecutionIsSerialAndExactOnce() {
        const auto owner=std::this_thread::get_id(); std::vector<int> visits(50);
        QVERIFY(parallelFor(visits.size(),[&](size_t i) {
            if(std::this_thread::get_id()!=owner) throw std::runtime_error("Unmanaged parallel work");
            ++visits[i];
        }));
        for(const auto value : visits) QCOMPARE(value,1);
        QCOMPARE(nodeThreadLimit(),size_t(1)); QVERIFY(parallelFor(0,[](size_t) {}));
    }
    void parallelInternalResultsMatchSerial() {
        Graph f; auto* step=f.add("internal"); PipelineExecution executor;
        f.options.maxThreads=1; auto first=executor.submit(f.graph,f.delegates,f.factory,{},f.options);
        const auto serial=get(first);
        f.options.maxThreads=4; auto second=executor.submit(f.graph,f.delegates,f.factory,{},f.options);
        const auto parallel=get(second); QVERIFY(serial.succeeded()); QVERIFY(parallel.succeeded());
        const auto name=step->outputMapping().front().dataName;
        QCOMPARE(serial.steps.at(step->id()).output->memberCast<tp_data::DoubleMember>(name)->data,192.0);
        QCOMPARE(parallel.steps.at(step->id()).output->memberCast<tp_data::DoubleMember>(name)->data,192.0);
    }
};
QTEST_GUILESS_MAIN(NodeWorkTests)
#include "NodeWorkTests.moc"
