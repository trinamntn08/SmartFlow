#include "workspace/WorkspaceWindow.h"
#include "workspace/GanttWidget.h"
#include <tp_pipeline/StepDelegate.h>
#include <tp_data/members/NumberMember.h>
#include <tp_utils/Progress.h>
#include <QComboBox>
#include <QSpinBox>
#include <QTreeWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QFontDatabase>
#include <QtTest/QtTest>
#include <thread>

using namespace smartflow;
using namespace tp_pipeline;
namespace {
struct Gate { std::atomic_bool open{false}; std::atomic<size_t> active{0}; };
class Delegate final : public StepDelegate {
public:
    Delegate(bool join,std::shared_ptr<Gate> gate)
        : StepDelegate(join ? "test.ui.join@1" : "test.ui.work@1",{},
            join ? std::vector<PortDetails>{{"first",tp_data::doubleSID()},{"second",tp_data::doubleSID()}} : std::vector<PortDetails>{},
            {{"out",tp_data::doubleSID()}}),gate(std::move(gate)),join(join) {}
    void fixupParameters(StepDetails* step,std::vector<tp_utils::StringID>& names) const override {
        Parameter p; p.name="value"; p.type=doubleSID(); p.value=2.0; p.min=0.0; p.max=100.0; p.step=1.0;
        step->setParamerter(p); names.push_back(p.name);
    }
    bool executeStep(StepContext* context) const override {
        double value=context->stepDetails->parameterValue<double>("value",2);
        if(!join) {
            ++gate->active;
            auto* child=context->progress->addChildStep("Processing",1.0f);
            child->setProgress(0.5f);
            const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(10);
            while(!gate->open && child->poll() && std::chrono::steady_clock::now()<deadline)
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            --gate->active;
            if(!gate->open) return false;
        } else {
            for(const auto& port : inPorts()) {
                const auto* input=context->memberCast<tp_data::DoubleMember>(port.name);
                if(!input) return false; value+=input->data;
            }
        }
        auto output=std::make_shared<tp_data::DoubleMember>(); output->data=value;
        return context->stepOutput->addSharedMember("out",output,context->progress);
    }
private:
    std::shared_ptr<Gate> gate;
    bool join;
};
WorkspaceConfiguration configuration(std::shared_ptr<Gate> gate) {
    WorkspaceConfiguration config;
    config.delegates=std::make_shared<StepDelegateMap>();
    config.delegates->addStepDelegate(new Delegate(false,gate)); config.delegates->addStepDelegate(new Delegate(true,gate));
    config.factory=std::make_shared<tp_data::CollectionFactory>();
    tp_data::createCollectionFactories(*config.factory); config.factory->finalize();
    config.nodes={{"test.ui.work@1","Work","Test","test.ui","work",1},{"test.ui.join@1","Join","Test","test.ui","join",1}};
    config.executionPolicies["test.ui.work@1"]={true,{}}; config.executionPolicies["test.ui.join@1"]={true,{}};
    config.preset={{"test.ui.work@1",{0,0},{}},{"test.ui.work@1",{0,180},{}},{"test.ui.join@1",{300,90},{}}};
    config.connections={{0,0,2,0},{1,0,2,1}};
    return config;
}
}
class ExecutionUiTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() {
        const auto path=qEnvironmentVariable("SMARTFLOW_TEST_FONT"); if(path.isEmpty()) return;
        const auto id=QFontDatabase::addApplicationFont(path); QVERIFY(id>=0);
        QApplication::setFont(QFont(QFontDatabase::applicationFontFamilies(id).first(),10));
    }
    void controlsAndLiveProgressOnQtThread() {
        auto gate=std::make_shared<Gate>(); WorkspaceWindow window(configuration(gate)); window.show();
        window.execution().setLive(false); window.execution().cancel(); QTRY_VERIFY(!window.execution().busy());
        auto* mode=window.findChild<QComboBox*>("executionMode");
        auto* threads=window.findChild<QSpinBox*>("executionThreads");
        auto* bar=window.findChild<QProgressBar*>("executionProgress");
        auto* tree=window.findChild<QTreeWidget*>("executionResults");
        QVERIFY(mode && threads && bar && tree);
        mode->setCurrentIndex(1); threads->setValue(2);
        QCOMPARE(window.execution().options().mode,ExecutionMode::Parallel);
        QCOMPARE(window.execution().options().maxThreads,size_t(2));
        bool uiThread=true;
        QObject::connect(&window.execution(),&ExecutionController::updated,&window,[&] {
            uiThread=uiThread && QThread::currentThread()==QApplication::instance()->thread();
        });
        window.findChild<QPushButton*>("runGraph")->click();
        QTRY_COMPARE(gate->active.load(),size_t(2));
        QTRY_VERIFY(window.execution().progress() && window.execution().progress()->sequence>=6 && tree->topLevelItemCount()==3);
        const auto snapshot=*window.execution().progress();
        QCOMPARE(snapshot.total,size_t(3)); QCOMPARE(snapshot.completed,size_t(0));
        size_t running=0,waiting=0;
        for(const auto& entry : snapshot.nodes) {
            if(entry.second.state==StepState::Running) { ++running; QCOMPARE(entry.second.fraction.value(),0.5); }
            if(entry.second.state==StepState::Waiting) ++waiting;
        }
        for(const auto& entry:snapshot.steps) if(entry.second.state==StepState::Running) {
            QVERIFY(entry.second.readyMs); QVERIFY(entry.second.startedMs); QVERIFY(!entry.second.endedMs);
            QVERIFY(*entry.second.readyMs<=*entry.second.startedMs); QVERIFY(*entry.second.startedMs<=snapshot.elapsedMs);
        }
        QCOMPARE(running,size_t(2)); QCOMPARE(waiting,size_t(1)); QCOMPARE(bar->value(),0);
        QVERIFY(window.grab().save("execution-parallel-smoke.png"));
        gate->open=true; QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(window.execution().result()->succeeded()); QVERIFY(uiThread);
        QCOMPARE(window.execution().progress()->completed,size_t(3)); QCOMPARE(bar->value(),3);
        const auto final=*window.execution().progress();
        for(const auto& entry:final.steps) {
            QVERIFY(entry.second.endedMs); QVERIFY(*entry.second.startedMs<=*entry.second.endedMs);
            QVERIFY(*entry.second.endedMs<=final.elapsedMs);
        }
        GanttWidget gantt; gantt.resize(1200,360); gantt.refresh(window.project(),final); gantt.show();
        auto* ganttTree=gantt.findChild<QTreeWidget*>("executionGantt"); QVERIFY(ganttTree);
        QCOMPARE(ganttTree->topLevelItemCount(),3);
        QString clicked;
        gantt.selected=[&](const std::string& id) { clicked=QString::fromStdString(id); };
        const auto* first=ganttTree->topLevelItem(0);
        QTest::mouseClick(ganttTree->viewport(),Qt::LeftButton,Qt::NoModifier,ganttTree->visualItemRect(first).center());
        QCOMPARE(clicked,first->data(0,Qt::UserRole).toString());
        QVERIFY(gantt.grab().save("execution-gantt-smoke.png"));
        // Frozen elapsed/timings must not advance after finish.
        QTest::qWait(20); QCOMPARE(window.execution().progress()->elapsedMs,final.elapsedMs);
        auto unsupported=window.project().retained();
        unsupported["project"]["graphs"][0]["nodes"][0]["typeId"]="future";
        window.loadDocument(std::move(unsupported));
        QVERIFY(!window.project().diagnostics().empty());
        gantt.refresh(window.project(),final); // stale snapshot must not request an unsupported projection
        QCOMPARE(ganttTree->topLevelItemCount(),0);
    }
    void editsRejectOldProgressAndModeChangesCancel() {
        auto gate=std::make_shared<Gate>(); WorkspaceWindow window(configuration(gate));
        window.execution().setLive(false); window.execution().cancel(); QTRY_VERIFY(!window.execution().busy());
        window.execution().setScheduling(ExecutionMode::Parallel,2); window.execution().run();
        QTRY_COMPARE(gate->active.load(),size_t(2));
        const auto old=window.execution().progress()->runId;
        const auto* step=window.project().graph().steps().front();
        window.project().commands().setParameter(step->id().toString(),"value",7.0);
        QTRY_VERIFY(!window.execution().busy());
        QVERIFY(!window.execution().result()); QVERIFY(!window.execution().progress());
        window.execution().run(); QTRY_COMPARE(gate->active.load(),size_t(2));
        QVERIFY(window.execution().progress()->runId!=old);
        window.execution().setScheduling(ExecutionMode::Sequential,1);
        QTRY_VERIFY(!window.execution().busy()); QVERIFY(!window.execution().progress());
        gate->open=true; window.execution().run(); QTRY_VERIFY(window.execution().result());
        QVERIFY(window.execution().result()->succeeded());
    }
    void progressObserverCanInvalidateACompletedFuture() {
        auto gate=std::make_shared<Gate>(); WorkspaceWindow window(configuration(gate));
        window.execution().setLive(false); window.execution().cancel(); QTRY_VERIFY(!window.execution().busy());
        window.execution().setScheduling(ExecutionMode::Parallel,2); window.execution().run();
        QTRY_COMPARE(gate->active.load(),size_t(2));
        bool edited=false;
        QObject::connect(&window.execution(),&ExecutionController::updated,&window,[&] {
            if(edited || !window.execution().progress() || !window.execution().progress()->finished) return;
            edited=true;
            const auto id=window.project().graph().steps().front()->id().toString();
            window.project().commands().setParameter(id,"value",9.0);
        });
        gate->open=true;
        QTRY_VERIFY(!window.execution().busy()); QVERIFY(edited);
        QVERIFY(!window.execution().result()); QVERIFY(!window.execution().progress());
    }
};
QTEST_MAIN(ExecutionUiTests)
#include "ExecutionUiTests.moc"
