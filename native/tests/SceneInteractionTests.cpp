#include <SceneExtension.h>
#include <SceneViewer.h>
#include <interaction/ObjectInteractionController.h>
#include "workspace/WorkspaceWindow.h"
#include "workspace/UseWorkspace.h"
#include <QComboBox>
#include <QLabel>
#include <QTemporaryDir>
#include <QFontDatabase>
#include <QtTest/QtTest>

using namespace smartflow;
using namespace smartflow::scene3d;
namespace {
SceneViewer* viewer(WorkspaceWindow& window,bool use=false)
{ return dynamic_cast<SceneViewer*>(window.findChild<QWidget*>(use ? "useResultViewer" : "sceneViewer")); }
std::shared_ptr<const tp_data::Collection> sceneOutput(WorkspaceWindow& window)
{
    for(const auto& [id,step] : window.execution().result()->steps)
        if(step.output)
            for(const auto& member : step.output->members())
                if(const auto* scene=dynamic_cast<const SceneMember*>(member.get());scene && !scene->objects.empty() && scene->objects[0].transform)
                    return step.output;
    return {};
}
const SceneMember& scene(const std::shared_ptr<const tp_data::Collection>& output)
{ return *dynamic_cast<const SceneMember*>(output->members().front().get()); }
void configure(SceneViewer& viewer,InteractionMode mode)
{
    viewer.restoreWorkspaceState({{"yaw",0},{"pitch",0},{"view",1},{"span",8},{"selection",0}});
    viewer.findChild<QComboBox*>("sceneInteractionMode")->setCurrentIndex(int(mode));
}
QPoint center(SceneViewer& viewer)
{ return {viewer.width()/2,(90+viewer.height()-30)/2}; }
}
class SceneInteractionTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        const auto path=qEnvironmentVariable("SMARTFLOW_TEST_FONT"); if(path.isEmpty()) return;
        const auto id=QFontDatabase::addApplicationFont(path); QVERIFY(id>=0);
        QApplication::setFont(QFont(QFontDatabase::applicationFontFamilies(id).first(),10));
    }
    void sharedTransformPreviewAndAtomicScale()
    {
        auto config=sceneConfiguration();
        config.preset={{"smartflow.scene-3d.cube@1",{0,0},{}},{"smartflow.scene-3d.merge@1",{190,0},{}},
            {"smartflow.scene-3d.transform@1",{380,0},{}},{"smartflow.scene-3d.scene@1",{570,0},{}}};
        config.connections={{0,0,1,0},{0,0,1,1},{1,0,2,0},{2,0,3,0}};
        WorkspaceWindow window(config); window.show(); QTRY_VERIFY(window.execution().result());
        const auto original=sceneOutput(window); QVERIFY(original); QCOMPARE(scene(original).objects.size(),size_t(2));
        const auto firstVertex=scene(original).objects[0].geometry.verts[0].vert;
        auto* view=viewer(window); QVERIFY(view); ObjectInteractionController control;
        CameraController camera; camera.view=CameraView::Front; const QRectF area(0,0,400,400);
        const auto revision=window.project().revision(); window.scene().undoStack().clear();
        QVERIFY(control.begin(scene(original),0,InteractionMode::Move,0,{200,200},camera,area,view->commands));
        control.update({240,200},false); QVERIFY(control.preview()); QCOMPARE(window.project().revision(),revision);
        for(int i=0;i<2;++i) {
            const auto v=control.preview()->objects[i].geometry.verts[0].vert;
            QVERIFY(std::abs(v.x-firstVertex.x-0.6f)<1e-4f); QCOMPARE(v.y,firstVertex.y);
        }
        QCOMPARE(scene(original).objects[0].geometry.verts[0].vert,firstVertex);
        QVERIFY(control.finish(view->commands).isEmpty()); QCOMPARE(window.scene().undoStack().count(),1);
        QTRY_VERIFY(window.execution().result());
        const auto moved=sceneOutput(window); const auto source=scene(moved).objects[0].transform->node;
        QVERIFY(std::abs(window.project().step(source)->parameterValue<double>("x")-0.6)<1e-5);
        window.scene().undoStack().undo(); QTRY_VERIFY(window.execution().result());
        QCOMPARE(scene(sceneOutput(window)).objects[0].geometry.verts[0].vert,firstVertex);
        const auto beforeScale=sceneOutput(window);
        QVERIFY(control.begin(scene(beforeScale),0,InteractionMode::Scale,0,{200,200},camera,area,view->commands));
        control.update({1200,200},false); QVERIFY(control.finish(view->commands).isEmpty());
        for(const auto* name : {"scale X","scale Y","scale Z"}) QCOMPARE(window.project().step(source)->parameterValue<double>(name),10.0);
        QCOMPARE(window.scene().undoStack().count(),1); window.scene().undoStack().undo();
        for(const auto* name : {"scale X","scale Y","scale Z"}) QCOMPARE(window.project().step(source)->parameterValue<double>(name),1.0);
    }
    void cancelAndRejectStaleGestures()
    {
        WorkspaceWindow window(sceneConfiguration()); QTRY_VERIFY(window.execution().result());
        auto* view=viewer(window); const auto original=sceneOutput(window); const auto source=scene(original).objects[0].transform->node;
        ObjectInteractionController control; CameraController camera; camera.view=CameraView::Front;
        const auto before=window.project().retained()["project"]; const auto revision=window.project().revision();
        QVERIFY(control.begin(scene(original),0,InteractionMode::Rotate,0,{200,200},camera,{0,0,400,400},view->commands));
        control.update({255,200},true); control.cancel(); QVERIFY(!control.preview()); QCOMPARE(window.project().revision(),revision);
        QVERIFY(window.project().retained()["project"]==before);
        QVERIFY(control.begin(scene(original),0,InteractionMode::Move,0,{200,200},camera,{0,0,400,400},view->commands));
        control.update({240,200},false);
        window.project().commands().setParameter(source.toString(),"y",2);
        QVERIFY(!control.finish(view->commands).isEmpty()); QCOMPARE(window.project().step(source)->parameterValue<double>("x"),0.0);
        QVERIFY(!control.begin(scene(original),0,InteractionMode::Move,1,{200,200},camera,{0,0,400,400},view->commands));
        QVERIFY(control.message().contains("stale"));
        camera.view=CameraView::Right;
        QVERIFY(!control.begin(scene(original),0,InteractionMode::Move,0,{200,200},camera,{0,0,400,400},view->commands));
        QVERIFY(control.message().contains("camera"));
    }
    void desktopGesturesUndoCancelSaveReopen()
    {
        WorkspaceWindow window(sceneConfiguration()); window.show(); QTRY_VERIFY(window.execution().result());
        auto* view=viewer(window); QVERIFY(view); configure(*view,InteractionMode::Move);
        window.scene().undoStack().clear(); const auto original=sceneOutput(window); const auto source=scene(original).objects[0].transform->node;
        const auto revision=window.project().revision(); const auto point=center(*view);
        QTest::mousePress(view,Qt::LeftButton,Qt::NoModifier,point);
        QTest::mouseMove(view,point+QPoint(35,0)); QCOMPARE(window.project().revision(),revision);
        QVERIFY(view->grab().save("scene-move-preview.png"));
        QTest::mouseRelease(view,Qt::LeftButton,Qt::NoModifier,point+QPoint(35,0));
        QCOMPARE(window.scene().undoStack().count(),1); QTRY_VERIFY(window.execution().result());
        const auto x=window.project().step(source)->parameterValue<double>("x"); QVERIFY(x>0);
        const auto selectedId=view->workspaceState()["selectedObjectId"].toString(); QVERIFY(!selectedId.isEmpty());
        window.scene().undoStack().undo(); QTRY_VERIFY(window.execution().result());
        QCOMPARE(window.project().step(source)->parameterValue<double>("x"),0.0);
        window.scene().undoStack().redo(); QTRY_VERIFY(window.execution().result());
        configure(*view,InteractionMode::Rotate);
        const auto rotatePoint=center(*view)+QPoint(35,0); // Cube moved 35 screen pixels.
        const auto beforeCancel=window.project().revision();
        QTest::mousePress(view,Qt::LeftButton,Qt::NoModifier,rotatePoint); QTest::mouseMove(view,rotatePoint+QPoint(30,0));
        QTest::keyClick(view,Qt::Key_Escape); QTest::mouseRelease(view,Qt::LeftButton,Qt::NoModifier,rotatePoint+QPoint(30,0));
        QCOMPARE(window.project().revision(),beforeCancel);
        QTest::mousePress(view,Qt::LeftButton,Qt::NoModifier,rotatePoint); QTest::mouseRelease(view,Qt::LeftButton,Qt::NoModifier,rotatePoint+QPoint(30,0));
        QTRY_VERIFY(window.execution().result()); QCOMPARE(window.project().step(source)->parameterValue<double>("rotation Y"),40.0);
        QTemporaryDir folder; const auto path=folder.filePath("edited.smartflow"); window.saveProject(path);
        WorkspaceWindow reopened(sceneConfiguration()); reopened.show(); reopened.openProject(path); QTRY_VERIFY(reopened.execution().result());
        QCOMPARE(reopened.project().step(source)->parameterValue<double>("x"),x);
        QCOMPARE(reopened.project().step(source)->parameterValue<double>("rotation Y"),40.0);
        QCOMPARE(viewer(reopened)->workspaceState()["selectedObjectId"].toString(),selectedId);
        QVERIFY(window.grab().save("scene-interaction-build.png"));
    }
    void useExposedControlsAndImmutableComponent()
    {
        WorkspaceWindow window(sceneConfiguration()); window.show(); window.openProject(SMARTFLOW_SCENE_INTERACTION_EXAMPLE);
        QTRY_VERIFY(window.execution().result()); QVERIFY(window.useMode());
        auto* view=viewer(window,true); QVERIFY(view); configure(*view,InteractionMode::Scale);
        const auto body=window.project().selectedGraph()["nodes"][0]["component"];
        const auto point=center(*view); window.scene().undoStack().clear();
        QTest::mousePress(view,Qt::LeftButton,Qt::NoModifier,point); QTest::mouseRelease(view,Qt::LeftButton,Qt::NoModifier,point+QPoint(40,0));
        QCOMPARE(window.scene().undoStack().count(),1); QTRY_VERIFY(window.execution().result());
        const auto value=window.project().step("scene-interaction-tool")->parameterValue<double>("scale X"); QVERIFY(value>1);
        for(const auto* name : {"scale Y","scale Z"}) QCOMPARE(window.project().step("scene-interaction-tool")->parameterValue<double>(name),value);
        QVERIFY(window.project().selectedGraph()["nodes"][0]["component"]==body);
        QTemporaryDir folder; const auto path=folder.filePath("tool.smartflow"); window.saveProject(path);
        WorkspaceWindow reopened(sceneConfiguration()); reopened.show(); reopened.openProject(path); QTRY_VERIFY(reopened.execution().result());
        QVERIFY(reopened.useMode()); QCOMPARE(reopened.project().step("scene-interaction-tool")->parameterValue<double>("scale X"),value);
        window.scene().undoStack().undo(); QTRY_VERIFY(window.execution().result());
        QCOMPARE(window.project().step("scene-interaction-tool")->parameterValue<double>("scale X"),1.0);
        QVERIFY(window.grab().save("scene-interaction-use.png"));
        window.openProject(SMARTFLOW_SCENE_TOOL_EXAMPLE); QTRY_VERIFY(window.execution().result()); configure(*view,InteractionMode::Move);
        view->findChild<QComboBox*>("sceneMoveAxis")->setCurrentIndex(1);
        const auto oldRevision=window.project().revision(); const auto point2=center(*view);
        QTest::mousePress(view,Qt::LeftButton,Qt::NoModifier,point2);
        QTest::mouseRelease(view,Qt::LeftButton,Qt::NoModifier,point2+QPoint(0,-40));
        QCOMPARE(window.project().revision(),oldRevision);
        QVERIFY(view->findChild<QLabel*>("sceneInteractionStatus")->text().contains("Expose"));
    }
    void navigationAndProjectChangesCancelPreview()
    {
        WorkspaceWindow window(sceneConfiguration()); window.show(); QTRY_VERIFY(window.execution().result());
        auto* view=viewer(window); configure(*view,InteractionMode::Rotate);
        const auto source=scene(sceneOutput(window)).objects[0].transform->node; const auto point=center(*view);
        const auto revision=window.project().revision();
        QTest::mousePress(view,Qt::LeftButton,Qt::NoModifier,point); QTest::mouseMove(view,point+QPoint(30,0));
        QTest::mousePress(view,Qt::RightButton,Qt::NoModifier,point+QPoint(30,0));
        QTest::mouseRelease(view,Qt::RightButton,Qt::NoModifier,point+QPoint(30,0));
        QTest::mouseRelease(view,Qt::LeftButton,Qt::NoModifier,point+QPoint(30,0));
        QCOMPARE(window.project().revision(),revision);
        QCOMPARE(window.project().step(source)->parameterValue<double>("rotation Y"),25.0);
        QTest::mousePress(view,Qt::LeftButton,Qt::NoModifier,point); QTest::mouseMove(view,point+QPoint(30,0));
        window.project().commands().setParameter(source.toString(),"x",0.1);
        const auto changed=window.project().revision();
        QTest::mouseRelease(view,Qt::LeftButton,Qt::NoModifier,point+QPoint(30,0));
        QCOMPARE(window.project().revision(),changed);
        QCOMPARE(window.project().step(source)->parameterValue<double>("rotation Y"),25.0);
        QTRY_VERIFY(window.execution().result());
        QTest::mousePress(view,Qt::LeftButton,Qt::NoModifier,point); QTest::mouseMove(view,point+QPoint(30,0));
        window.setUseMode(true); window.setUseMode(false);
        QTest::mouseRelease(view,Qt::LeftButton,Qt::NoModifier,point+QPoint(30,0));
        QCOMPARE(window.project().revision(),changed);
    }
};
QTEST_MAIN(SceneInteractionTests)
#include "SceneInteractionTests.moc"
