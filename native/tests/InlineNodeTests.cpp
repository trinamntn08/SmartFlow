#include "workspace/WorkspaceWindow.h"
#include "workspace/NodeParameterPanel.h"
#include <SceneExtension.h>
#include <DataExtension.h>
#include <SceneViewer.h>
#include <QtNodes/GraphicsView>
#include <QtNodes/internal/NodeGraphicsObject.hpp>
#include <QGraphicsProxyWidget>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QLayout>
#include <QTemporaryDir>
#include <QFontDatabase>
#include <tp_data/members/NumberMember.h>
#include <QtTest/QtTest>

using namespace smartflow;
namespace {
QtNodes::NodeId findNode(WorkspaceWindow& window,const char* type)
{
    for(const auto id : window.canvas().allNodeIds())
        if(auto* step=window.project().step(window.canvas().projectId(id));step && step->delegateName()==type) return id;
    return QtNodes::InvalidNodeId;
}
QWidget* body(WorkspaceWindow& window,QtNodes::NodeId node)
{ return window.canvas().nodeData(node,QtNodes::NodeRole::Widget).value<QWidget*>(); }
QDoubleSpinBox* field(WorkspaceWindow& window,QtNodes::NodeId node,const char* name)
{ return body(window,node)->findChild<QDoubleSpinBox*>("nodeParameter_"+QString::fromUtf8(name)); }
QPoint fieldPosition(WorkspaceWindow& window,QtNodes::NodeId node,const char* name)
{
    auto* root=body(window,node); auto* editor=field(window,node,name); auto* view=window.findChild<QtNodes::GraphicsView*>("graphCanvas");
    const auto scenePoint=root->graphicsProxyWidget()->mapToScene(editor->mapTo(root,editor->rect().center()));
    view->resetTransform(); view->centerOn(scenePoint);
    return view->mapFromScene(scenePoint);
}
double output(WorkspaceWindow& window)
{
    const auto id=window.canvas().projectId(findNode(window,"smartflow.numeric.add@1"));
    const auto& result=window.execution().result()->steps.at(id);
    return dynamic_cast<const tp_data::DoubleMember*>(result.output->members().front().get())->data;
}
}
class InlineNodeTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        const auto path=qEnvironmentVariable("SMARTFLOW_TEST_FONT"); if(path.isEmpty()) return;
        const auto id=QFontDatabase::addApplicationFont(path); QVERIFY(id>=0);
        QApplication::setFont(QFont(QFontDatabase::applicationFontFamilies(id).first(),10));
    }
    void canvasTypingExecutionUndoAndNoMovement()
    {
        WorkspaceWindow window; window.show(); QTRY_VERIFY(window.execution().result()); QCOMPARE(output(window),42.0);
        const auto number=findNode(window,"smartflow.numeric.number@1"); auto* editor=field(window,number,"value"); QVERIFY(editor);
        const auto original=window.canvas().nodeData(number,QtNodes::NodeRole::Position).value<QPointF>();
        const auto revision=window.project().revision(); const QPointer<QWidget> originalBody(body(window,number));
        auto* view=window.findChild<QtNodes::GraphicsView*>("graphCanvas");
        const auto point=fieldPosition(window,number,"value"); QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,point);
        QTest::keyClick(view->viewport(),Qt::Key_A,Qt::ControlModifier); QTest::keyClicks(view->viewport(),"12.5");
        QCOMPARE(window.project().revision(),revision); // Typing remains a draft.
        QTest::keyClick(view->viewport(),Qt::Key_Return);
        QCOMPARE(window.project().step(window.canvas().projectId(number))->parameterValue<double>("value"),12.5);
        QCOMPARE(window.scene().undoStack().count(),1); QTRY_VERIFY(window.execution().result()); QCOMPARE(output(window),13.5);
        QCOMPARE(window.canvas().nodeData(number,QtNodes::NodeRole::Position).value<QPointF>(),original);
        QCOMPARE(body(window,number),originalBody.data());
        window.scene().undoStack().undo(); QTRY_VERIFY(window.execution().result()); QCOMPARE(output(window),42.0); QCOMPARE(editor->value(),41.0);
        window.scene().undoStack().redo(); QTRY_VERIFY(window.execution().result()); QCOMPARE(editor->value(),12.5);
        const auto beforeCancel=window.project().revision();
        QTest::keyClick(editor,Qt::Key_A,Qt::ControlModifier); QTest::keyClicks(editor,"99"); QTest::keyClick(editor,Qt::Key_Escape);
        QCOMPARE(editor->value(),12.5); QCOMPARE(window.project().revision(),beforeCancel);
        QCOMPARE(editor->findChild<QLineEdit*>()->text(),QString("12.5"));
        QTest::keyClick(editor,Qt::Key_Up); QCOMPARE(window.scene().undoStack().count(),2);
        QCOMPARE(window.project().step(window.canvas().projectId(number))->parameterValue<double>("value"),13.5);
        QTest::mouseClick(view->viewport(),Qt::LeftButton,Qt::NoModifier,fieldPosition(window,number,"value"));
        QTest::keyClick(view->viewport(),Qt::Key_A,Qt::ControlModifier); QTest::keyClicks(view->viewport(),"14.5");
        QTest::mouseClick(window.findChild<QPushButton*>("runGraph"),Qt::LeftButton);
        QCOMPARE(window.project().step(window.canvas().projectId(number))->parameterValue<double>("value"),14.5);
        QCOMPARE(window.scene().undoStack().count(),3);
        QTemporaryDir folder; const auto path=folder.filePath("inline.smartflow"); window.saveProject(path);
        WorkspaceWindow reopened; reopened.show(); reopened.openProject(path); QTRY_VERIFY(reopened.execution().result());
        QCOMPARE(output(reopened),15.5); QCOMPARE(field(reopened,findNode(reopened,"smartflow.numeric.number@1"),"value")->value(),14.5);
        QVERIFY(body(window,number)->findChild<QLabel*>("nodeExecutionTime"));
        QVERIFY(window.grab().save("inline-numeric-smoke.png"));
    }
    void sceneParametersInspectorAndViewerShareValues()
    {
        WorkspaceWindow window(scene3d::sceneConfiguration()); window.show(); QTRY_VERIFY(window.execution().result());
        const auto transform=findNode(window,"smartflow.scene-3d.transform@1"); const auto id=window.canvas().projectId(transform);
        auto* panel=body(window,transform); QCOMPARE(panel->findChildren<QDoubleSpinBox*>().size(),7);
        const auto order=window.project().step(id)->orderedParameterNames();
        QCOMPARE(order[0].toString(),std::string("x")); QCOMPARE(order[1].toString(),std::string("y"));
        QCOMPARE(order[3].toString(),std::string("rotation Y"));
        field(window,transform,"x")->setValue(1.25); QTRY_VERIFY(window.execution().result());
        QCOMPARE(window.project().step(id)->parameterValue<double>("x"),1.25);
        window.selectNode(transform); QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        auto* inspector=window.findChild<QDoubleSpinBox*>("parameter_x"); QVERIFY(inspector); QCOMPARE(inspector->value(),1.25);
        auto* parameterLayout=inspector->parentWidget()->parentWidget()->layout();
        const auto editorIndex=parameterLayout->indexOf(inspector->parentWidget());
        QVERIFY(editorIndex>=0);
        auto* apply=qobject_cast<QPushButton*>(parameterLayout->itemAt(editorIndex+1)->widget());
        QVERIFY(apply); inspector->setValue(2.5); apply->click();
        QCOMPARE(field(window,transform,"x")->value(),2.5); QTRY_VERIFY(window.execution().result());
        auto* view=dynamic_cast<scene3d::SceneViewer*>(window.findChild<QWidget*>("sceneViewer")); QVERIFY(view);
        QVERIFY(view->commands.apply({window.project().revision(),"Viewer move",{{QString::fromStdString(id.toString()),"y",3}}}).isEmpty());
        QCOMPARE(field(window,transform,"y")->value(),3.0); QCOMPARE(body(window,transform),panel);
        window.scene().undoStack().undo(); QCOMPARE(field(window,transform,"y")->value(),0.0);
        // Geometry contains sockets and controls with no overlap.
        const auto& geometry=window.scene().nodeGeometry(); const auto widgetTop=geometry.widgetPosition(transform).y();
        QVERIFY(geometry.portPosition(transform,QtNodes::PortType::In,0).y()<widgetTop);
        QVERIFY(widgetTop+panel->height()<=geometry.size(transform).height());
        const auto before=window.project().revision(); field(window,transform,"scale X")->setValue(100);
        QCOMPARE(window.project().step(id)->parameterValue<double>("scale X"),10.0);
        QCOMPARE(window.project().revision(),before+1);
        QTRY_VERIFY(window.execution().result());
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        QVERIFY(window.grab().save("inline-scene-smoke.png"));
    }
    void dataComponentControlsAndOpaqueRoundTrip()
    {
        WorkspaceWindow window(data::dataConfiguration()); window.show(); window.openProject(SMARTFLOW_COMPONENT_EXAMPLE);
        window.setUseMode(false); QTRY_VERIFY(window.execution().result());
        QtNodes::NodeId component=QtNodes::InvalidNodeId;
        for(const auto candidate : window.canvas().allNodeIds()) if(window.canvas().projectId(candidate)=="summary-tool") component=candidate;
        QVERIFY(component!=QtNodes::InvalidNodeId); auto* editor=field(window,component,"minimum"); QVERIFY(editor);
        QCOMPARE(body(window,component)->findChildren<QDoubleSpinBox*>().size(),1); // Public facade only.
        auto retained=window.project().retained(); retained["project"]["future"]={{"untouched",true}};
        window.loadDocument(retained); window.setUseMode(false);
        for(const auto candidate : window.canvas().allNodeIds()) if(window.canvas().projectId(candidate)=="summary-tool") component=candidate;
        const auto definition=window.project().selectedGraph()["nodes"][1]["component"];
        editor=field(window,component,"minimum"); editor->setValue(20); QTRY_VERIFY(window.execution().result());
        QCOMPARE(window.project().step("summary-tool")->parameterValue<double>("minimum"),20.0);
        QVERIFY(window.project().selectedGraph()["nodes"][1]["component"]==definition);
        QTemporaryDir folder; const auto path=folder.filePath("component.smartflow"); window.saveProject(path);
        WorkspaceWindow reopened(data::dataConfiguration()); reopened.openProject(path); QTRY_VERIFY(reopened.execution().result());
        QCOMPARE(reopened.project().step("summary-tool")->parameterValue<double>("minimum"),20.0);
        QVERIFY(reopened.project().retained()["project"]["future"]["untouched"].get<bool>());
        window.scene().undoStack().undo(); QCOMPARE(editor->value(),30.0);
        QVERIFY(window.grab().save("inline-component-smoke.png"));
    }
    void externalEditsDiscardDraftsAndDeletionIsSafe()
    {
        WorkspaceWindow window; window.show(); window.execution().setLive(false);
        const auto number=findNode(window,"smartflow.numeric.number@1"); const auto id=window.canvas().projectId(number);
        auto* editor=field(window,number,"value"); const QPointer<QWidget> panel(body(window,number));
        QTest::keyClick(editor,Qt::Key_A,Qt::ControlModifier); QTest::keyClicks(editor,"999");
        window.project().commands().setParameter(id.toString(),"value",7);
        QTest::keyClick(editor,Qt::Key_Return); QCOMPARE(window.project().step(id)->parameterValue<double>("value"),7.0);
        QCOMPARE(window.scene().undoStack().count(),1);
        window.canvas().deleteNode(number); QVERIFY(panel.isNull()); QVERIFY(!window.project().step(id));
        window.scene().undoStack().undo(); QVERIFY(window.canvas().nodeExists(number)); QCOMPARE(field(window,number,"value")->value(),7.0);
        editor=field(window,number,"value");
        QTest::keyClick(editor,Qt::Key_A,Qt::ControlModifier); QTest::keyClicks(editor,"999");
        auto* text=editor->findChild<QLineEdit*>(); QCOMPARE(text->text(),QString("999"));
        window.canvas().showExecutionTimes(std::nullopt); QCOMPARE(text->text(),QString("999"));
        window.project().commands().setParameter(window.canvas().projectId(findNode(window,"smartflow.numeric.add@1")).toString(),"value",2);
        QCOMPARE(text->text(),QString("7")); QTest::keyClick(editor,Qt::Key_Return);
        QCOMPARE(window.project().step(id)->parameterValue<double>("value"),7.0);
        auto source=window.project().retained();
        for(auto& node : source["project"]["graphs"][0]["nodes"]) if(node["id"]==id.toString()) { node["packageId"]="future"; node["parameters"]["opaque"]={{"value",42}}; }
        window.loadDocument(source); QVERIFY(!window.project().step(id));
        for(const auto candidate : window.canvas().allNodeIds()) if(window.canvas().projectId(candidate)==id)
            QVERIFY(!body(window,candidate));
        QVERIFY(window.project().retained()["project"]==source["project"]);
    }
};
QTEST_MAIN(InlineNodeTests)
#include "InlineNodeTests.moc"
