#include <SceneExtension.h>
#include <controllers/CameraController.h>
#include <model/SceneSnapshot.h>
#include <rendering/SceneRenderer.h>
#include "workspace/WorkspaceWindow.h"
#include <tp_math_utils/Intersection.h>
#include <tp_math_utils/Plane.h>
#include <QtTest/QtTest>
#include <limits>

using namespace smartflow;
using namespace smartflow::scene3d;
class SceneControllerTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void planeIntersectionGuards()
    {
        glm::vec3 hit;
        const tp_math_utils::Plane plane({0,0,0},{0,1,0});
        QVERIFY(tp_math_utils::rayPlaneIntersection({{2,1,3},{2,-1,3}},plane,hit));
        QCOMPARE(hit,glm::vec3(2,0,3));
        QVERIFY(!tp_math_utils::rayPlaneIntersection({{2,0,3},{3,0,3}},plane,hit));
        QVERIFY(!tp_math_utils::rayPlaneIntersection({{2,1,3},{3,1,3}},plane,hit));
        QVERIFY(!tp_math_utils::rayPlaneIntersection({{0,1,0},{0,1,0}},plane,hit));
        QVERIFY(!tp_math_utils::rayPlaneIntersection({{0,1,0},{0,0,0}},tp_math_utils::Plane({0,0,0},{0,0,0}),hit));
        QVERIFY(!tp_math_utils::rayPlaneIntersection({{0,1,0},{0,0,0}},tp_math_utils::Plane({0,INFINITY,0},{0,1,0}),hit));
    }
    void cameraAnchoringAndAxes()
    {
        CameraController camera;
        const QRectF viewport(0,0,800,400); const QPointF cursor(580,120);
        const auto before=camera.pointOnPlane(cursor,viewport,camera.target,camera.forward()); QVERIFY(before);
        camera.zoom(120,cursor,viewport);
        const auto after=camera.pointOnPlane(cursor,viewport,{0,0,0},camera.forward()); QVERIFY(after);
        QVERIFY(glm::length(*after-*before)<1e-4f);
        const auto p=camera.project({2,1,0},viewport); camera.pan({40,-20},viewport);
        QVERIFY(QLineF(camera.project({2,1,0},viewport),p+QPointF(40,-20)).length()<1e-3);
        for(const auto view : {CameraView::Front,CameraView::Right,CameraView::Top}) {
            camera.view=view;
            QCOMPARE(glm::dot(camera.right(),camera.forward()),0.0f);
            QVERIFY(std::abs(glm::length(camera.up())-1)<1e-6f);
        }
        camera.restore({{"span",-1},{"pitch",90},{"view",1.5}});
        QCOMPARE(camera.span,6.0f); QCOMPARE(camera.pitch,25.0f); QCOMPARE(camera.view,CameraView::Orbit);
    }
    void stableSelectionAndBranchPicking()
    {
        auto config=sceneConfiguration();
        config.preset={{"smartflow.scene-3d.cube@1",{0,0},{}},{"smartflow.scene-3d.merge@1",{190,0},{}}};
        config.connections={{0,0,1,0},{0,0,1,1}};
        WorkspaceWindow window(config); QTRY_VERIFY(window.execution().result());
        std::shared_ptr<const tp_data::Collection> output;
        for(const auto& [id,step] : window.execution().result()->steps)
            if(step.output && step.output->members().size()) {
                const auto* s=dynamic_cast<const SceneMember*>(step.output->members()[0].get());
                if(s && s->objects.size()==2) output=step.output;
            }
        QVERIFY(output); SceneSnapshot snapshot; snapshot.present(output); QCOMPARE(snapshot.scene()->objects.size(),size_t(2));
        QVERIFY(snapshot.scene()->objects[0].id!=snapshot.scene()->objects[1].id);
        snapshot.select(1); const auto selected=snapshot.selectedId(); snapshot.present({}); QCOMPARE(snapshot.selectedIndex(),-1);
        auto member=std::make_shared<SceneMember>(); member->objects=snapshot.scene() ? snapshot.scene()->objects :
            dynamic_cast<const SceneMember*>(output->members()[0].get())->objects;
        std::swap(member->objects[0],member->objects[1]);
        auto next=std::make_shared<tp_data::Collection>(); next->addMember(member); snapshot.present(next);
        QCOMPARE(snapshot.selectedIndex(),0); QCOMPARE(snapshot.selectedId(),selected);
        CameraController camera; camera.view=CameraView::Front;
        member->objects[0].geometry.transform(glm::translate(glm::mat4(1),glm::vec3(0,0,4)));
        SceneRenderer renderer; QCOMPARE(renderer.pick({200,200},{0,0,400,400},camera,*member),0);
        QCOMPARE(renderer.pick({0,0},{0,0,400,400},camera,*member),-1);
        member->objects.erase(member->objects.begin()); snapshot.present(next); QCOMPARE(snapshot.selectedIndex(),-1);
    }
};
QTEST_MAIN(SceneControllerTests)
#include "SceneControllerTests.moc"
