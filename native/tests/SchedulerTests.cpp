#include "pipeline/PipelineExecution.h"
#include <tp_pipeline/StepDelegate.h>
#include <tp_data/members/NumberMember.h>
#include <tp_utils/Progress.h>
#include <QtTest/QtTest>
#include <condition_variable>
#include <mutex>
#include <thread>

using namespace smartflow;
using namespace tp_pipeline;
using namespace std::chrono_literals;
namespace {
struct Gate {
    std::mutex mutex;
    std::condition_variable changed;
    bool release=false;
    size_t active=0, maximum=0, starts=0;
    bool waitFor(size_t count) {
        std::unique_lock<std::mutex> lock(mutex);
        return changed.wait_for(lock,2s,[&] { return active>=count; });
    }
    void open() { { std::lock_guard<std::mutex> lock(mutex); release=true; } changed.notify_all(); }
};
class Work final : public StepDelegate {
public:
    Work(const char* name, size_t inputs, std::shared_ptr<Gate> gate={})
        : StepDelegate(name,{},ports(inputs),{{"out",tp_data::doubleSID()}}),gate(std::move(gate)) {}
    static std::vector<PortDetails> ports(size_t count) {
        std::vector<PortDetails> output;
        for(size_t i=0;i<count;++i) output.push_back({std::string("in")+std::to_string(i),tp_data::doubleSID()});
        return output;
    }
    bool executeStep(StepContext* context) const override {
        if(gate) {
            std::unique_lock<std::mutex> lock(gate->mutex);
            ++gate->active; ++gate->starts; gate->maximum=std::max(gate->maximum,gate->active);
            gate->changed.notify_all();
            const auto deadline=std::chrono::steady_clock::now()+4s;
            while(!gate->release && context->progress->poll() && std::chrono::steady_clock::now()<deadline)
                gate->changed.wait_for(lock,5ms);
            --gate->active;
            if(!gate->release) return false;
        }
        if(context->stepDetails->parameterValue<double>("fail")!=0) return false;
        double value=1;
        for(const auto& port : inPorts()) {
            auto* input=context->memberCast<tp_data::DoubleMember>(port.name);
            if(!input) return false;
            if(context->stepDetails->parameterValue<double>("mutate")!=0) input->data=100;
            value+=input->data;
        }
        auto output=std::make_shared<tp_data::DoubleMember>(); output->data=value;
        return context->stepOutput->addSharedMember("out",output,context->progress);
    }
private:
    std::shared_ptr<Gate> gate;
};
struct Graph {
    PipelineDetails graph;
    std::shared_ptr<StepDelegateMap> delegates=std::make_shared<StepDelegateMap>();
    std::shared_ptr<tp_data::CollectionFactory> factory=std::make_shared<tp_data::CollectionFactory>();
    Graph(std::shared_ptr<Gate> gate={}) {
        delegates->addStepDelegate(new Work("root",0));
        delegates->addStepDelegate(new Work("branch",1,gate));
        delegates->addStepDelegate(new Work("join",2));
        tp_data::createCollectionFactories(*factory); factory->finalize();
    }
    StepDetails* add(const char* id, const char* type, std::vector<const char*> sources={}) {
        auto* step=new StepDetails(type);
        tp_utils::JSON state; step->saveBinary(state,[](const std::string&) { return uint64_t(0); });
        state["id"]=id; step->loadBinary(state,{});
        step->setOutputMapping({{tp_data::doubleSID(),"out",std::string(id)+"-out",{}}});
        std::vector<PortMapping> inputs;
        for(size_t i=0;i<sources.size();++i)
            inputs.push_back({tp_data::doubleSID(),std::string("in")+std::to_string(i),std::string(sources[i])+"-out",{}});
        step->setInputMapping(inputs); graph.addStep(step); return step;
    }
    void parameter(StepDetails* step, const char* name) {
        Parameter p; p.name=name; p.type=doubleSID(); p.value=1.0; step->setParamerter(p);
    }
    ExecutionOptions options(size_t threads=2) {
        ExecutionOptions output; output.mode=ExecutionMode::Parallel; output.maxThreads=threads;
        for(const auto* name : {"root","branch","join"}) output.policies[name]={true,{}};
        return output;
    }
};
ExecutionResult get(ExecutionHandle& handle) {
    if(handle.result.wait_for(10s)!=std::future_status::ready) throw std::runtime_error("Scheduler timed out");
    return handle.result.get();
}
double number(const ExecutionResult& result,const char* id) {
    return result.steps.at(id).output->memberCast<tp_data::DoubleMember>(std::string(id)+"-out")->data;
}
}
class SchedulerTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void diamondModes_data() {
        QTest::addColumn<int>("threads"); QTest::addColumn<bool>("parallel");
        QTest::newRow("sequential")<<4<<false;
        QTest::newRow("parallel-one")<<1<<true;
        QTest::newRow("parallel-two")<<2<<true;
        QTest::newRow("parallel-four")<<4<<true;
    }
    void diamondModes() {
        QFETCH(int,threads); QFETCH(bool,parallel);
        Graph f; f.add("join","join",{"left","right"}); f.add("right","branch",{"root"});
        f.add("left","branch",{"root"}); f.add("root","root");
        PipelineExecution executor; auto options=f.options(threads);
        if(!parallel) options.mode=ExecutionMode::Sequential;
        auto handle=executor.submit(f.graph,f.delegates,f.factory,{},options);
        const auto result=get(handle); QVERIFY(result.succeeded()); QCOMPARE(result.steps.size(),size_t(4));
        QCOMPARE(number(result,"join"),5.0);
    }
    void independentBranchesOverlapWithinLimit() {
        auto gate=std::make_shared<Gate>(); Graph f(gate); f.add("root","root");
        for(const auto* id : {"a","b","c","d","e","f"}) f.add(id,"branch",{"root"});
        PipelineExecution executor; auto handle=executor.submit(f.graph,f.delegates,f.factory,{},f.options());
        const bool overlap=gate->waitFor(2); gate->open(); const auto result=get(handle);
        QVERIFY(overlap); QVERIFY(result.succeeded()); QCOMPARE(gate->maximum,size_t(2)); QCOMPARE(gate->starts,size_t(6));
    }
    void policiesSerialize_data() {
        QTest::addColumn<QString>("kind");
        for(const auto* kind : {"sequential","unknown","resource"}) QTest::newRow(kind)<<QString(kind);
    }
    void policiesSerialize() {
        QFETCH(QString,kind); auto gate=std::make_shared<Gate>(); Graph f(gate);
        f.add("root","root"); f.add("a","branch",{"root"}); f.add("b","branch",{"root"});
        auto options=f.options();
        if(kind=="sequential") options.mode=ExecutionMode::Sequential;
        if(kind=="unknown") options.policies.clear();
        if(kind=="resource") options.policies["branch"].exclusiveResources={"test-resource"};
        PipelineExecution executor; auto handle=executor.submit(f.graph,f.delegates,f.factory,{},options);
        const bool started=gate->waitFor(1); const bool overlapped=gate->waitFor(2);
        gate->open(); const auto result=get(handle);
        QVERIFY(started); QVERIFY(!overlapped); QVERIFY(result.succeeded()); QCOMPARE(gate->maximum,size_t(1));
    }
    void resourceLocksCoverSeparateExecutors() {
        auto gate=std::make_shared<Gate>(); Graph f(gate); f.add("root","root"); f.add("a","branch",{"root"});
        auto options=f.options(); options.policies["branch"].exclusiveResources={"shared-device"};
        PipelineExecution first,second;
        auto a=first.submit(f.graph,f.delegates,f.factory,{},options);
        const bool started=gate->waitFor(1);
        auto b=second.submit(f.graph,f.delegates,f.factory,{},options);
        const bool overlapped=gate->waitFor(2);
        gate->open(); QVERIFY(get(a).succeeded()); QVERIFY(get(b).succeeded());
        QVERIFY(started); QVERIFY(!overlapped); QCOMPARE(gate->maximum,size_t(1)); QCOMPARE(gate->starts,size_t(2));
    }
    void fanOutMutationDoesNotChangeSiblingOrProducer() {
        Graph f; f.add("join","join",{"left","right"});
        auto* left=f.add("left","branch",{"root"}); f.parameter(left,"mutate");
        f.add("right","branch",{"root"}); f.add("root","root");
        PipelineExecution executor; auto handle=executor.submit(f.graph,f.delegates,f.factory,{},f.options(4));
        const auto result=get(handle); QVERIFY(result.succeeded());
        QCOMPARE(number(result,"root"),1.0); QCOMPARE(number(result,"right"),2.0); QCOMPARE(number(result,"join"),104.0);
    }
    void multiplePortsFromOneProducerReleaseOnce() {
        Graph f; f.add("join","join",{"root","root"}); f.add("root","root");
        PipelineExecution executor; auto handle=executor.submit(f.graph,f.delegates,f.factory,{},f.options());
        const auto result=get(handle); QVERIFY(result.succeeded()); QCOMPARE(number(result,"join"),3.0);
    }
    void failureAndComponentBarriers() {
        Graph f; f.add("consumer","join",{"body","body"}); f.add("body","branch",{"root"});
        auto* hidden=f.add("hidden","root"); f.parameter(hidden,"fail");
        f.add("root","root"); f.add("independent","root");
        PipelineExecution executor; auto handle=executor.submit(f.graph,f.delegates,f.factory,
            {{"component",{"body","hidden"},{"body-out"}}},f.options(4));
        const auto result=get(handle); QVERIFY(!result.succeeded());
        QCOMPARE(result.steps.at("component").state,StepState::Failed);
        QCOMPARE(result.steps.at("consumer").state,StepState::Skipped);
        QCOMPARE(result.steps.at("independent").state,StepState::Succeeded);
    }
    void cancellationDrainsOverlappingWork() {
        auto gate=std::make_shared<Gate>(); Graph f(gate); f.add("root","root");
        f.add("a","branch",{"root"}); f.add("b","branch",{"root"}); f.add("c","branch",{"root"});
        PipelineExecution executor; auto handle=executor.submit(f.graph,f.delegates,f.factory,{},f.options());
        const bool started=gate->waitFor(2); handle.cancel(); const auto result=get(handle);
        QVERIFY(started); QVERIFY(result.cancelled); QVERIFY(result.steps.empty());
        QCOMPARE(gate->active,size_t(0)); QCOMPARE(gate->starts,size_t(2));
    }
    void adjacentComponentBarriersIgnoreGroupOrder() {
        Graph f;
        f.add("join","join",{"b","b"}); f.add("b","branch",{"a"});
        f.add("a","branch",{"root"}); f.add("spare-a","root"); f.add("spare-b","root"); f.add("root","root");
        PipelineExecution executor; auto handle=executor.submit(f.graph,f.delegates,f.factory,
            {{"B",{"b","spare-b"},{"b-out"}},{"A",{"a","spare-a"},{"a-out"}}},f.options(4));
        const auto result=get(handle); QVERIFY(result.succeeded()); QCOMPARE(number(result,"join"),7.0);
    }
    void optionsRejectInvalidLimits() {
        Graph f; PipelineExecution executor; auto options=f.options(0);
        QVERIFY_EXCEPTION_THROWN(executor.submit(f.graph,f.delegates,f.factory,{},options),std::invalid_argument);
        options.maxThreads=65;
        QVERIFY_EXCEPTION_THROWN(executor.submit(f.graph,f.delegates,f.factory,{},options),std::invalid_argument);
    }
    void emptyAndLargeGraphs() {
        Graph f; PipelineExecution executor;
        auto empty=executor.submit(f.graph,f.delegates,f.factory,{},f.options()); QVERIFY(get(empty).succeeded());
        for(size_t i=0;i<1000;++i) {
            const auto id=std::string("n")+std::to_string(i);
            f.add(id.c_str(),"root");
        }
        auto large=executor.submit(f.graph,f.delegates,f.factory,{},f.options(4));
        const auto result=get(large); QVERIFY(result.succeeded()); QCOMPARE(result.steps.size(),size_t(1000));
    }
};
QTEST_GUILESS_MAIN(SchedulerTests)
#include "SchedulerTests.moc"
