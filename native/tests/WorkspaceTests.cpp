#include "workspace/WorkspaceWindow.h"
#include "workspace/ProjectCommands.h"
#include <tp_data/members/NumberMember.h>
#include <tp_data/Collection.h>
#include <QtNodes/internal/UndoCommands.hpp>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QUndoStack>
#include <QtTest/QtTest>

using namespace smartflow;

namespace {
QtNodes::NodeId findNode(WorkspaceWindow& window, const char* type)
{
    for(const auto id : window.canvas().allNodeIds())
        if(window.project().step(window.canvas().projectId(id))->delegateName() == type) return id;
    return QtNodes::InvalidNodeId;
}

double output(WorkspaceWindow& window, QtNodes::NodeId id)
{
    const auto& step = window.execution().result()->steps.at(window.canvas().projectId(id));
    return dynamic_cast<const tp_data::DoubleMember*>(step.output->members().front().get())->data;
}
}

class WorkspaceTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void parameterValidationAndUnknownMetadata()
    {
        GraphProject project(numericDelegates());
        auto* step = project.create("smartflow.numeric.number@1");
        tp_pipeline::Parameter unknown;
        unknown.name = "future-parameter";
        unknown.type = "future-type@1";
        unknown.value = std::string("retained");
        step->setParamerter(unknown);
        auto parameter = step->parameter("value");
        const auto revision = project.revision();
        parameter.value = std::string("invalid numeric value");
        QVERIFY(!project.setParameter(step->id(), parameter));
        parameter.value = 1000001.0;
        QVERIFY(!project.setParameter(step->id(), parameter));
        QCOMPARE(project.revision(), revision);
        parameter.value = 12.0;
        QUndoStack undo;
        undo.push(new SetParameterCommand(project, step->id(), parameter));
        QCOMPARE(step->parameterValue<double>("value"), 12.0);
        undo.undo();
        QCOMPARE(step->parameterValue<double>("value"), 1.0);
        const auto id = step->id();
        const auto snapshot = project.capture(id);
        project.remove(id);
        QVERIFY(project.restore(snapshot));
        QCOMPARE(project.capture(id), snapshot);
        QCOMPARE(project.step(id)->parameterValue<std::string>("future-parameter"), std::string("retained"));
    }

    void inspectorEditUndoAndRedo()
    {
        WorkspaceWindow window;
        window.show();
        const auto source = findNode(window, "smartflow.numeric.number@1");
        const auto target = findNode(window, "smartflow.numeric.add@1");
        QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(window.execution().result()->succeeded());
        QCOMPARE(output(window, target), 42.0);
        window.selectNode(source);
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        auto* spin = window.findChild<QDoubleSpinBox*>("parameter_value");
        auto* apply = window.findChild<QPushButton*>("applyParameter");
        QVERIFY(spin);
        QVERIFY(apply);
        spin->setValue(9.0);
        QTest::mouseClick(apply, Qt::LeftButton);
        QVERIFY(!window.execution().result());
        QTRY_VERIFY(window.execution().result().has_value());
        QCOMPARE(output(window, target), 10.0);
        window.scene().undoStack().undo();
        QTRY_VERIFY(window.execution().result().has_value());
        QCOMPARE(output(window, target), 42.0);
        window.scene().undoStack().redo();
        QTRY_VERIFY(window.execution().result().has_value());
        QCOMPARE(output(window, target), 10.0);
    }

    void deleteUndoPreservesIdentityParametersAndConnections()
    {
        WorkspaceWindow window;
        window.execution().setLive(false);
        const auto source = findNode(window, "smartflow.numeric.number@1");
        const auto target = findNode(window, "smartflow.numeric.add@1");
        const auto stable = window.canvas().projectId(source);
        const auto snapshot = window.project().capture(stable);
        const QtNodes::ConnectionId link{source, 0, target, 0};
        window.selectNode(source);
        auto& undo = window.scene().undoStack();
        undo.push(new QtNodes::DeleteCommand(&window.scene()));
        QVERIFY(!window.project().step(stable));
        QVERIFY(!window.project().step(window.canvas().projectId(target))->inputMapping()[0].dataName.isValid());
        undo.undo();
        QCOMPARE(window.canvas().projectId(source), stable);
        QCOMPARE(window.project().capture(stable), snapshot);
        QVERIFY(window.canvas().connectionExists(link));
        window.execution().run();
        QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(window.execution().result()->succeeded());
        QCOMPARE(output(window, target), 42.0);
        undo.redo();
        QVERIFY(!window.project().step(stable));
        undo.undo();
        QCOMPARE(window.project().capture(stable), snapshot);
    }

    void createDisconnectAndWorkspaceUndo()
    {
        WorkspaceWindow window;
        window.execution().setLive(false);
        const auto source = findNode(window, "smartflow.numeric.number@1");
        const auto target = findNode(window, "smartflow.numeric.add@1");
        auto& undo = window.scene().undoStack();
        undo.push(new QtNodes::DisconnectCommand(&window.scene(), {source, 0, target, 0}));
        window.execution().run();
        QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(!window.execution().result()->succeeded());
        undo.undo();
        window.execution().run();
        QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(window.execution().result()->succeeded());
        const auto revision = window.project().revision();
        window.selectNode(source);
        const auto position = window.canvas().nodeData(source, QtNodes::NodeRole::Position).value<QPointF>();
        undo.push(new QtNodes::MoveNodeCommand(&window.scene(), QPointF(25, 50)));
        QCOMPARE(window.project().revision(), revision);
        QVERIFY(window.execution().result().has_value());
        undo.undo();
        QCOMPARE(window.canvas().nodeData(source, QtNodes::NodeRole::Position).value<QPointF>(), position);
        undo.push(new QtNodes::CreateCommand(&window.scene(), "smartflow.numeric.number@1", {500, 200}));
        QCOMPARE(window.project().graph().steps().size(), size_t(3));
        undo.undo();
        QCOMPARE(window.project().graph().steps().size(), size_t(2));
        undo.redo();
        QCOMPARE(window.project().graph().steps().size(), size_t(3));
    }

    void staleCompletionCancellationAndManualRun()
    {
        WorkspaceWindow window;
        window.execution().setLive(false);
        const auto source = findNode(window, "smartflow.numeric.number@1");
        const auto target = findNode(window, "smartflow.numeric.add@1");
        // The constructor submitted a run; completion cannot publish until the
        // event loop polls it. Edit first to invalidate even a completed future.
        auto parameter = window.project().step(window.canvas().projectId(source))->parameter("value");
        parameter.value = 7.0;
        window.project().setParameter(window.canvas().projectId(source), parameter);
        QTRY_VERIFY(!window.execution().busy());
        QVERIFY(!window.execution().result());
        QVERIFY(window.execution().status().contains("Outdated"));
        window.execution().run();
        window.execution().cancel();
        QTRY_VERIFY(!window.execution().busy());
        QVERIFY(!window.execution().result());
        QCOMPARE(window.execution().status(), QString("Cancelled"));
        window.execution().run();
        QTRY_VERIFY(window.execution().result().has_value());
        QCOMPARE(output(window, target), 8.0);
        window.execution().setLive(true);
        for(int value = 10; value < 30; ++value) {
            parameter.value = double(value);
            window.project().setParameter(window.canvas().projectId(source), parameter);
        }
        QTRY_VERIFY(window.execution().result().has_value());
        QCOMPARE(output(window, target), 30.0);
    }
};
QTEST_MAIN(WorkspaceTests)
#include "WorkspaceTests.moc"
