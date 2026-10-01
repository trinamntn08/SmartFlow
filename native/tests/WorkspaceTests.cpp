#include "workspace/WorkspaceWindow.h"
#include <tp_data/members/NumberMember.h>
#include <tp_data/Collection.h>
#include <QtNodes/internal/UndoCommands.hpp>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QUndoStack>
#include <QTemporaryDir>
#include <QFontDatabase>
#include <QtNodes/internal/NodeGraphicsObject.hpp>
#include <QtNodes/GraphicsView>
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
    void fileActionsRoundTripAndFailures()
    {
        WorkspaceWindow window;
        window.execution().setLive(false);
        QTemporaryDir directory;
        const auto path=directory.filePath("saved.smartflow");
        QVERIFY(!window.projectDirty());
        const auto source=findNode(window,"smartflow.numeric.number@1");
        window.selectNode(source);
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        window.findChild<QDoubleSpinBox*>("parameter_value")->setValue(23);
        window.findChild<QPushButton*>("applyParameter")->click();
        QVERIFY(window.projectDirty());
        window.saveProject(path);
        QVERIFY(!window.projectDirty());
        const auto saved=window.project().retained();
        window.scene().undoStack().undo();
        QVERIFY(window.projectDirty());
        window.scene().undoStack().redo();
        QVERIFY(!window.projectDirty());
        window.project().commands().setWorkspaceField("opaque",{{"future",42}});
        QVERIFY(window.projectDirty());
        const auto before=window.project().retained();
        const auto revision=window.project().revision();
        const auto history=window.scene().undoStack().count();
        QVERIFY_EXCEPTION_THROWN(window.saveProject(directory.path()),project::FileError);
        QVERIFY_EXCEPTION_THROWN(window.openProject(directory.filePath("missing")),project::FileError);
        auto empty=project::create("empty");
        project::write(directory.filePath("empty.smartflow"),empty);
        QVERIFY_EXCEPTION_THROWN(window.openProject(directory.filePath("empty.smartflow")),project::FileError);
        QVERIFY(window.project().retained()==before);
        QCOMPARE(window.project().revision(),revision);
        QCOMPARE(window.scene().undoStack().count(),history);
        QCOMPARE(window.projectPath(),path);
        QVERIFY(window.projectDirty());
        QVERIFY(project::read(path)==saved);
        window.openProject(path);
        QVERIFY(window.project().retained()==saved);
        QVERIFY(!window.projectDirty());
        QCOMPARE(window.scene().undoStack().count(),0);
        QCOMPARE(window.scene().selectedNodes().size(),size_t(1));
        window.execution().run();
        QTRY_VERIFY(window.execution().result().has_value());
        QCOMPARE(output(window,findNode(window,"smartflow.numeric.add@1")),24.0);
        auto unknown=saved;
        unknown["project"]["graphs"][0]["nodes"][0]["packageId"]="future";
        unknown["workspace"]={{"opaque",{{"integer",uint64_t(18446744073709551615ULL)}}}};
        project::write(path,unknown);
        window.openProject(path);
        const auto copy=directory.filePath("copy.smartflow");
        window.saveProject(copy);
        const auto copied=project::read(copy);
        QVERIFY(copied["project"]==unknown["project"]);
        QVERIFY(copied["workspace"]["opaque"]==unknown["workspace"]["opaque"]);
        QVERIFY(!window.project().diagnostics().empty());
    }

    void initTestCase()
    {
        // Optional offscreen visual QA font; no machine-specific path is bundled.
        const auto path=qEnvironmentVariable("SMARTFLOW_TEST_FONT");
        if(path.isEmpty()) return;
        const auto id=QFontDatabase::addApplicationFont(path);
        QVERIFY(id>=0);
        const auto families=QFontDatabase::applicationFontFamilies(id);
        QVERIFY(!families.isEmpty());
        QApplication::setFont(QFont(families.front(),10));
    }

    void hiddenEdgesOccupyInputsAndVisibleDeletionUsesExactIdentity()
    {
        WorkspaceWindow window;
        window.execution().setLive(false);
        const auto source=findNode(window,"smartflow.numeric.number@1");
        const auto target=findNode(window,"smartflow.numeric.add@1");
        const QtNodes::ConnectionId connection{source,0,target,0};
        auto file=window.project().retained();
        auto& edges=file["project"]["graphs"][0]["connections"];
        auto hidden=edges[0];
        hidden["id"]="hidden";
        hidden["source"]["nodeId"]="unavailable";
        edges.insert(edges.begin(),hidden);
        window.project().commands().replace(file,"graph");
        QVERIFY(window.canvas().connectionExists(connection));
        window.scene().disconnectNodes(connection);
        QVERIFY(window.project().selectedGraph()["connections"]==project::Document::array({hidden}));
        QVERIFY(!window.canvas().connectionPossible(connection));
        window.scene().undoStack().undo();
        QVERIFY(window.project().retained()==file);
        QVERIFY(window.canvas().connectionExists(connection));
    }

    void retainedEditorRoundTripUnknownNodesAndAtomicSelectionUndo()
    {
        WorkspaceWindow window;
        window.execution().setLive(false);
        const auto source=findNode(window,"smartflow.numeric.number@1");
        const auto target=findNode(window,"smartflow.numeric.add@1");
        const auto stable=window.canvas().projectId(source).toString();
        auto file=window.project().retained();
        auto& graph=file["project"]["graphs"][0];
        graph["connections"][0]["opaque"]=uint64_t(18446744073709551615ULL);
        graph["nodes"].push_back({{"id","missing"},{"packageId","uninstalled"},{"typeId","custom"},
            {"version",77},{"parameters",{{"integer",uint64_t(18446744073709551615ULL)}}}});
        graph["connections"].push_back({{"id","hidden"},{"source",{{"nodeId",stable},{"portId","future"}}},
            {"target",{{"nodeId","missing"},{"portId","future"}}},{"opaque",{1,2,3}}});
        window.project().commands().replace(file,"graph");
        QCOMPARE(window.canvas().allNodeIds().size(),size_t(3));
        QtNodes::NodeId missing=QtNodes::InvalidNodeId;
        for(const auto id : window.canvas().allNodeIds())
            if(window.canvas().projectId(id)=="missing") missing=id;
        QVERIFY(missing!=QtNodes::InvalidNodeId);
        QVERIFY(window.canvas().nodeData(missing,QtNodes::NodeRole::Caption).toString().contains("uninstalled/custom"));
        window.execution().run();
        QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(!window.execution().result()->succeeded());
        QVERIFY(window.execution().result()->steps.empty());
        window.scene().disconnectNodes({source,0,target,0});
        window.scene().undoStack().undo();
        QVERIFY(window.project().retained()["project"]==file["project"]);
        window.selectNode(source);
        window.scene().nodeGraphicsObject(missing)->setSelected(true);
        window.findChild<QtNodes::GraphicsView*>("graphCanvas")->onDeleteSelectedObjects();
        QCOMPARE(window.scene().undoStack().count(),1);
        QCOMPARE(window.canvas().allNodeIds().size(),size_t(1));
        window.scene().undoStack().undo();
        QVERIFY(window.project().retained()["project"]==file["project"]);
        QCOMPARE(window.canvas().projectId(source).toString(),stable);
        QVERIFY(window.canvas().nodeExists(missing));
        QVERIFY(window.canvas().connectionExists({source,0,target,0}));
        QTemporaryDir directory;
        const auto path=directory.filePath("editor.smartflow");
        project::write(path,window.project().retained());
        window.project().commands().replace(project::read(path),"graph");
        QVERIFY(window.project().retained()["project"]==file["project"]);
        QCOMPARE(window.scene().undoStack().count(),0);
        QCOMPARE(window.canvas().allNodeIds().size(),size_t(3));
        window.show();
        window.selectNode(missing);
        QCoreApplication::processEvents();
        QVERIFY(window.grab().save("n5e-unavailable-smoke.png"));
    }

    void replacementInvalidatesPendingExecutionAndInspectionCannotMutateGraph()
    {
        WorkspaceWindow window;
        window.execution().setLive(false);
        const auto source=findNode(window,"smartflow.numeric.number@1");
        const auto target=findNode(window,"smartflow.numeric.add@1");
        window.project().step(window.canvas().projectId(source))->setParameterValue("value",999.0);
        window.execution().run();
        QTRY_VERIFY(window.execution().result().has_value());
        QCOMPARE(output(window,target),42.0);
        auto replacement=window.project().retained();
        for(auto& node : replacement["project"]["graphs"][0]["nodes"])
            if(node["id"]==window.canvas().projectId(source).toString()) node["parameters"]["value"]=9;
        window.execution().run();
        window.project().commands().replace(replacement,"graph");
        QTRY_VERIFY(!window.execution().busy());
        QVERIFY(!window.execution().result());
        window.execution().run();
        QTRY_VERIFY(window.execution().result().has_value());
        QCOMPARE(output(window,target),10.0);
        const auto revision=window.project().revision();
        auto invalid=replacement; invalid["schemaVersion"]=99;
        QVERIFY_EXCEPTION_THROWN(window.project().commands().replace(invalid,"graph"),project::FileError);
        QCOMPARE(window.project().revision(),revision);
        QCOMPARE(output(window,target),10.0);
    }

    void parameterValidationAndUnknownMetadata()
    {
        GraphProject project(numericDelegates(),numericPresentations());
        const auto id=project.create("smartflow.numeric.number@1")->id();
        auto file=project.retained();
        file["project"]["graphs"][0]["nodes"][0]["parameters"]["future-parameter"]={
            {"integer",uint64_t(18446744073709551615ULL)}};
        project.commands().replace(file,"graph");
        auto parameter=project.step(id)->parameter("value");
        const auto revision=project.revision();
        parameter.value=std::string("invalid numeric value");
        QVERIFY(!project.setParameter(id,parameter));
        parameter.value=1000001.0;
        QVERIFY(!project.setParameter(id,parameter));
        QCOMPARE(project.revision(),revision);
        parameter.value=12.0;
        QVERIFY(project.setParameter(id,parameter));
        QCOMPARE(project.step(id)->parameterValue<double>("value"),12.0);
        project.commands().undoStack().undo();
        QVERIFY(project.retained()==file);
        project.remove(id);
        QVERIFY(!project.step(id));
        project.commands().undoStack().undo();
        QVERIFY(project.retained()==file);
        QVERIFY_EXCEPTION_THROWN(project.graph(),project::FileError);
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
        const auto snapshot = window.project().retained();
        const QtNodes::ConnectionId link{source, 0, target, 0};
        window.selectNode(source);
        auto& undo = window.scene().undoStack();
        window.scene().deleteSelected();
        QVERIFY(!window.project().step(stable));
        QVERIFY(!window.project().step(window.canvas().projectId(target))->inputMapping()[0].dataName.isValid());
        undo.undo();
        QCOMPARE(window.canvas().projectId(source), stable);
        QVERIFY(window.project().retained()["project"]==snapshot["project"]);
        QVERIFY(window.canvas().connectionExists(link));
        window.execution().run();
        QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(window.execution().result()->succeeded());
        QCOMPARE(output(window, target), 42.0);
        undo.redo();
        QVERIFY(!window.project().step(stable));
        undo.undo();
        QVERIFY(window.project().retained()["project"]==snapshot["project"]);
    }

    void createDisconnectAndWorkspaceUndo()
    {
        WorkspaceWindow window;
        window.execution().setLive(false);
        const auto source = findNode(window, "smartflow.numeric.number@1");
        const auto target = findNode(window, "smartflow.numeric.add@1");
        auto& undo = window.scene().undoStack();
        window.scene().disconnectNodes({source, 0, target, 0});
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
        window.scene().createNode("smartflow.numeric.number@1", {500, 200});
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
