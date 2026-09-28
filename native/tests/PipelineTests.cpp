#include "pipeline/PipelineExecution.h"
#include <tp_pipeline/PipelineManager.h>
#include <tp_pipeline/StepDelegate.h>
#include <tp_data/members/NumberMember.h>
#include <tp_data/members/StringMember.h>
#include <tp_utils/Progress.h>
#include <QtTest/QtTest>
#include <chrono>
#include <thread>

using namespace tp_pipeline;
using namespace smartflow;
using namespace std::chrono_literals;

namespace {
void addParameter(StepDetails* step, const char* name, const Variant& value)
{
    Parameter parameter;
    parameter.name = name;
    parameter.type = std::holds_alternative<double>(value) ? doubleSID() : stringSID();
    parameter.value = value;
    step->setParamerter(parameter);
}

struct Probe {
    std::promise<void> entered;
    std::shared_future<void> release;
    std::thread::id worker;
};

class NumberDelegate final : public StepDelegate {
public:
    explicit NumberDelegate(bool input = false, std::shared_ptr<Probe> probe = {})
        : StepDelegate(input ? "test.add@1" : "test.number@1", {},
                       input ? std::vector<PortDetails>{{"in", tp_data::doubleSID()}} : std::vector<PortDetails>{},
                       {{"out", tp_data::doubleSID()}}), probe(std::move(probe)) {}

    bool executeStep(StepContext* context) const override
    {
        if(probe) {
            probe->worker = std::this_thread::get_id();
            probe->entered.set_value();
            if(probe->release.valid()) {
                if(probe->release.wait_for(5s) != std::future_status::ready)
                    throw std::runtime_error("Test release timed out");
            } else {
                const auto deadline = std::chrono::steady_clock::now() + 5s;
                while(context->progress->poll() && std::chrono::steady_clock::now() < deadline)
                    std::this_thread::yield();
            }
        }
        const auto mode = context->stepDetails->parameterValue<std::string>("mode");
        if(mode == "throw") throw std::runtime_error("Deliberate failure");
        if(mode == "false") return false;
        if(mode == "missing-output") return true;
        if(mode == "wrong-type") {
            auto value = std::make_shared<tp_data::StringMember>();
            return context->stepOutput->addSharedMember("out", value, context->progress);
        }
        auto output = std::make_shared<tp_data::DoubleMember>();
        output->data = context->stepDetails->parameterValue<double>("value", 1.0);
        if(!inPorts().empty()) {
            auto* input = context->memberCast<tp_data::DoubleMember>("in");
            if(!input) return false;
            output->data += input->data;
        }
        return context->stepOutput->addSharedMember("out", output, context->progress);
    }
private:
    std::shared_ptr<Probe> probe;
};

struct Fixture {
    PipelineDetails graph;
    std::shared_ptr<StepDelegateMap> delegates = std::make_shared<StepDelegateMap>();
    std::shared_ptr<tp_data::CollectionFactory> factory = std::make_shared<tp_data::CollectionFactory>();
    StepDetails* source;
    StepDetails* target;

    explicit Fixture(std::shared_ptr<Probe> probe = {})
    {
        delegates->addStepDelegate(new NumberDelegate(false, std::move(probe)));
        delegates->addStepDelegate(new NumberDelegate(true));
        tp_data::createCollectionFactories(*factory);
        factory->finalize();
        source = new StepDetails("test.number@1");
        target = new StepDetails("test.add@1");
        source->setOutputMapping({{tp_data::doubleSID(), "out", "source-data", {}}});
        target->setInputMapping({{tp_data::doubleSID(), "in", "source-data", {}}});
        target->setOutputMapping({{tp_data::doubleSID(), "out", "target-data", {}}});
        addParameter(source, "value", 41.0);
        addParameter(source, "mode", std::string());
        // Consumers deliberately precede their producers in stored order.
        graph.addStep(target);
        graph.addStep(source);
    }

    ExecutionResult run(PipelineExecution& executor)
    {
        auto handle = executor.submit(graph, delegates, factory);
        if(handle.result.wait_for(10s) != std::future_status::ready)
            throw std::runtime_error("Pipeline timed out");
        return handle.result.get();
    }
};

double value(const ExecutionResult& result, StepDetails* step, const char* output)
{
    return result.steps.at(step->id()).output->memberCast<tp_data::DoubleMember>(output)->data;
}
} // namespace

class PipelineTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void executesDependenciesAndRecomputes()
    {
        Fixture f;
        PipelineExecution executor;
        auto first = f.run(executor);
        QVERIFY(first.succeeded());
        QCOMPARE(value(first, f.target, "target-data"), 42.0);
        f.source->setParameterValue("value", 5.0);
        auto second = f.run(executor);
        QVERIFY(second.succeeded());
        QCOMPARE(value(second, f.target, "target-data"), 6.0);
        QCOMPARE(value(first, f.target, "target-data"), 42.0);
    }

    void invalidGraph_data()
    {
        QTest::addColumn<QString>("kind");
        for(const auto* kind : {"missing-delegate", "missing-producer", "invalid-port", "wrong-type",
                               "duplicate-output", "duplicate-node", "missing-input", "cycle", "self-cycle", "override"})
            QTest::newRow(kind) << QString(kind);
    }

    void invalidGraph()
    {
        QFETCH(QString, kind);
        Fixture f;
        if(kind == "missing-delegate") f.source->setDelegateName("unavailable.package.node@1");
        if(kind == "missing-producer") f.target->setInputMapping({{tp_data::doubleSID(), "in", "unknown", {}}});
        if(kind == "invalid-port") f.target->setInputMapping({{tp_data::doubleSID(), "bad-port", "source-data", {}}});
        if(kind == "wrong-type") f.target->setInputMapping({{tp_data::stringSID(), "in", "source-data", {}}});
        if(kind == "duplicate-output") f.target->setOutputMapping(f.source->outputMapping());
        if(kind == "duplicate-node") f.graph.addStep(new StepDetails(*f.source));
        if(kind == "missing-input") f.target->setInputMapping({});
        if(kind == "cycle") {
            f.source->setDelegateName("test.add@1");
            f.source->setInputMapping({{tp_data::doubleSID(), "in", "target-data", {}}});
        }
        if(kind == "self-cycle") f.target->setInputMapping({{tp_data::doubleSID(), "in", "target-data", {}}});
        if(kind == "override") f.source->setNoExec(true);
        const auto before = f.target->inputMapping();
        PipelineExecution executor;
        const auto result = f.run(executor);
        QVERIFY(!result.succeeded());
        QVERIFY(!result.diagnostics.empty());
        QVERIFY(result.steps.empty());
        QCOMPARE(f.target->inputMapping().size(), before.size());
        if(!before.empty()) QCOMPARE(f.target->inputMapping()[0].dataName.toString(), before[0].dataName.toString());
    }

    void failureSkipsConsumers_data()
    {
        QTest::addColumn<QString>("mode");
        for(const auto* mode : {"false", "throw", "missing-output", "wrong-type"})
            QTest::newRow(mode) << QString(mode);
    }

    void failureSkipsConsumers()
    {
        QFETCH(QString, mode);
        Fixture f;
        f.source->setParameterValue("mode", mode.toStdString());
        PipelineExecution executor;
        const auto result = f.run(executor);
        QVERIFY(!result.succeeded());
        QCOMPARE(result.steps.at(f.source->id()).state, StepState::Failed);
        QCOMPARE(result.steps.at(f.target->id()).state, StepState::Skipped);
        QVERIFY(!result.steps.at(f.source->id()).output);
        QVERIFY(!result.steps.at(f.target->id()).output);
        QVERIFY(!result.steps.at(f.source->id()).error.empty());
    }

    void snapshotAndWorkerIsolation()
    {
        auto probe = std::make_shared<Probe>();
        std::promise<void> release;
        probe->release = release.get_future().share();
        auto entered = probe->entered.get_future();
        Fixture f(probe);
        PipelineExecution executor;
        auto handle = executor.submit(f.graph, f.delegates, f.factory);
        QVERIFY(entered.wait_for(5s) == std::future_status::ready);
        QVERIFY(probe->worker != std::this_thread::get_id());
        f.source->setParameterValue("value", 100.0);
        release.set_value();
        QVERIFY(handle.result.wait_for(5s) == std::future_status::ready);
        const auto result = handle.result.get();
        QVERIFY(result.succeeded());
        QCOMPARE(value(result, f.target, "target-data"), 42.0);
        QCOMPARE(f.source->parameterValue<double>("value"), 100.0);
    }

    void cancellationDiscardsOutputs()
    {
        auto probe = std::make_shared<Probe>();
        auto entered = probe->entered.get_future();
        Fixture f(probe);
        PipelineExecution executor;
        auto handle = executor.submit(f.graph, f.delegates, f.factory);
        QVERIFY(entered.wait_for(5s) == std::future_status::ready);
        handle.cancel();
        QVERIFY(handle.result.wait_for(5s) == std::future_status::ready);
        const auto result = handle.result.get();
        QVERIFY(result.cancelled);
        QVERIFY(!result.succeeded());
        QVERIFY(result.steps.empty());
    }

    void queuedCancellationDoesNotExecute()
    {
        auto probe = std::make_shared<Probe>();
        std::promise<void> release;
        probe->release = release.get_future().share();
        auto entered = probe->entered.get_future();
        Fixture f(probe);
        PipelineExecution executor;
        auto first = executor.submit(f.graph, f.delegates, f.factory);
        QVERIFY(entered.wait_for(5s) == std::future_status::ready);
        auto second = executor.submit(f.graph, f.delegates, f.factory);
        second.cancel();
        release.set_value();
        QVERIFY(first.result.wait_for(5s) == std::future_status::ready);
        QVERIFY(first.result.get().succeeded());
        QVERIFY(second.result.wait_for(5s) == std::future_status::ready);
        const auto result = second.result.get();
        QVERIFY(result.cancelled);
        QVERIFY(result.steps.empty());
    }

    void independentBranchSurvivesFailure()
    {
        Fixture f;
        f.source->setParameterValue("mode", std::string("throw"));
        auto* independent = new StepDetails("test.number@1");
        independent->setOutputMapping({{tp_data::doubleSID(), "out", "independent-data", {}}});
        f.graph.addStep(independent);
        PipelineExecution executor;
        const auto result = f.run(executor);
        QVERIFY(!result.succeeded());
        QCOMPARE(result.steps.at(independent->id()).state, StepState::Succeeded);
        QCOMPARE(value(result, independent, "independent-data"), 1.0);
    }

    void noFixupSurvivesRestartAndEdit()
    {
        Fixture f;
        addParameter(f.source, "unknown.extension.parameter", std::string("keep me"));
        f.target->setInputMapping({{tp_data::doubleSID(), "in", "unknown", {}}});
        PipelineManager manager(&f.graph, f.delegates.get(), f.factory.get(), false);
        manager.startExecution();
        f.source->setParameterValue("value", 7.0);
        manager.startExecution();
        QCOMPARE(f.target->inputMapping()[0].dataName.toString(), std::string("unknown"));
        QCOMPARE(f.source->parameterValue<std::string>("unknown.extension.parameter"), std::string("keep me"));
    }
};

QTEST_GUILESS_MAIN(PipelineTests)
#include "PipelineTests.moc"
