#include <SceneExtension.h>
#include <SceneViewer.h>
#include "workspace/WorkspaceWindow.h"
#include "workspace/ComponentDialogs.h"
#include "project/DocumentSession.h"
#include <tp_math_utils/materials/OpenGLMaterial.h>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QUndoStack>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QDir>
#include <QJsonArray>
#include <QAction>
#include <QFontDatabase>
#include <QtNodes/GraphicsView>
#include <QtNodes/internal/NodeGraphicsObject.hpp>
#include <QtTest/QtTest>

using namespace smartflow;
using namespace smartflow::scene3d;
namespace {
QtNodes::NodeId node(WorkspaceWindow& window, const char* suffix)
{
    const auto type = QString("smartflow.scene-3d.%1@1").arg(suffix).toStdString();
    for(const auto id : window.canvas().allNodeIds())
        if(window.project().step(window.canvas().projectId(id))->delegateName().toString() == type) return id;
    return QtNodes::InvalidNodeId;
}
const SceneMember& scene(WorkspaceWindow& window, const char* suffix)
{
    const auto& output = window.execution().result()->steps.at(window.canvas().projectId(node(window,suffix))).output;
    return *dynamic_cast<const SceneMember*>(output->members().front().get());
}
void edit(WorkspaceWindow& window, const char* suffix, const char* name, double value)
{
    const auto id = window.canvas().projectId(node(window,suffix));
    auto p = window.project().step(id)->parameter(name);
    p.value = value;
    window.project().setParameter(id,p);
}
}

class SceneTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void sharedBrowserSceneExpectations()
    {
        const auto path=QFileInfo(QString::fromUtf8(SMARTFLOW_SCENE_EXAMPLE)).dir().filePath("../tests/fixtures/scene-results-v1.json");
        const auto fixture=project::jsonFile::read(path);
        WorkspaceWindow window(sceneConfiguration());
        QTRY_VERIFY(window.execution().result());
        window.execution().setLive(false);
        for(const auto& expected : fixture["cases"]) {
            edit(window,"cube","size",expected["size"].get<double>());
            edit(window,"transform","rotation Y",expected["rotation"].get<double>());
            const char* axes[]={"x","y","z"};
            const char* scales[]={"scale X","scale Y","scale Z"};
            for(int axis=0;axis<3;++axis) {
                edit(window,"transform",axes[axis],expected["position"][axis].get<double>());
                edit(window,"transform",scales[axis],expected["scale"][axis].get<double>());
            }
            window.execution().run(); QTRY_VERIFY(!window.execution().busy());
            QVERIFY(window.execution().result()); QVERIFY(window.execution().result()->succeeded());
            const auto actual=scene(window,"scene").objects.front().geometry.verts.front().vert;
            for(int axis=0;axis<3;++axis) QVERIFY(std::abs(actual[axis]-expected["firstVertex"][axis].get<double>())<1e-5);
        }
    }
    void parallelSceneMatchesSequential() {
        WorkspaceWindow window(sceneConfiguration());
        QTRY_VERIFY(window.execution().result());
        const auto previous=scene(window,"scene").objects.front().geometry.verts.front().vert;
        window.execution().setScheduling(ExecutionMode::Parallel,4);
        window.execution().run(); QTRY_VERIFY(window.execution().result());
        QVERIFY(window.execution().result()->succeeded());
        const auto actual=scene(window,"scene").objects.front().geometry.verts.front().vert;
        QCOMPARE(actual.x,previous.x); QCOMPARE(actual.y,previous.y); QCOMPARE(actual.z,previous.z);
    }
    void editedSceneCopyRetainsOriginalSnapshot()
    {
        WorkspaceWindow window(sceneConfiguration()); window.execution().setLive(false); window.show();
        std::vector<std::string> nodes; std::string cube, terminal;
        for(const auto& item : window.project().selectedGraph()["nodes"]) {
            nodes.push_back(item["id"]);
            if(item["typeId"]=="cube") cube=item["id"];
            if(item["typeId"]=="scene") terminal=item["id"];
        }
        const auto original=window.project().commands().extractComponent(nodes,"scene-original","Scene original",
            project::Document::array(),project::Document::array({{{"id","scene"},{"source",{{"nodeId",terminal},{"portId","out"}}}}}),
            project::Document::array({{{"id","size"},{"target",{{"nodeId",cube},{"parameter","size"}}}}}));
        window.project().commands().instantiateComponent(original,"original-instance",project::Document::object(),project::Document::object(),true);
        ComponentEditDialog editor(window.project(),window.configuration(),original);
        editor.workspace().project().commands().setParameter(cube,"size",4);
        auto copy=original.definition(); copy["id"]="scene-copy";
        copy["graph"]=editor.workspace().project().selectedGraph();
        editor.saveCopy(project::GraphComponent(copy));
        window.project().commands().instantiateComponent(project::GraphComponent(copy),"copy-instance",project::Document::object(),project::Document::object(),true);
        window.execution().run(); QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(window.execution().result()->succeeded());
        const auto result=window.execution().result();
        const auto originalOutput=result->steps.at("original-instance").output;
        const auto copyOutput=result->steps.at("copy-instance").output;
        const auto oldMax=dynamic_cast<const SceneMember*>(originalOutput->members().front().get())->objects.front().geometry.getMinMax().second.x;
        const auto newMax=dynamic_cast<const SceneMember*>(copyOutput->members().front().get())->objects.front().geometry.getMinMax().second.x;
        QVERIFY(newMax>oldMax);
        window.project().commands().replaceComponentInstance("original-instance",project::GraphComponent(copy));
        window.execution().run(); QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(window.execution().result()->succeeded());
        const auto updated=window.execution().result()->steps.at("original-instance").output;
        QCOMPARE(dynamic_cast<const SceneMember*>(updated->members().front().get())->objects.front().geometry.getMinMax().second.x,oldMax);
        auto bounded=copy; bounded["id"]="incompatible-bound";
        for(const auto& item : bounded["graph"]["nodes"]) if(item["typeId"]=="material")
            bounded["controls"][0]["target"]={{"nodeId",item["id"]},{"parameter","red"}};
        const auto preserved=window.project().retained();
        QVERIFY_EXCEPTION_THROWN(window.project().commands().replaceComponentInstance("original-instance",project::GraphComponent(bounded)),project::FileError);
        QVERIFY(window.project().retained()==preserved);
        QTemporaryDir directory; const auto path=directory.filePath("scene-copy.smartflow"); window.saveProject(path);
        WorkspaceWindow reopened(sceneConfiguration()); reopened.execution().setLive(false); reopened.openProject(path);
        QVERIFY(reopened.project().retained()["project"]["components"][1]==copy);
    }

    void initTestCase()
    {
        const auto path = qEnvironmentVariable("SMARTFLOW_TEST_FONT");
        if(!path.isEmpty()) {
            const auto id = QFontDatabase::addApplicationFont(path);
            QVERIFY(id >= 0);
            QApplication::setFont(QFont(QFontDatabase::applicationFontFamilies(id).first()));
        }
    }

    void collapsedSceneComponentMapsViewerControlsAndReopen()
    {
        WorkspaceWindow window(sceneConfiguration()); window.execution().setLive(false); window.show();
        std::vector<std::string> selection;
        std::string cube, terminal;
        for(const auto& item : window.project().selectedGraph()["nodes"]) {
            const auto id=item["id"].get<std::string>(); selection.push_back(id);
            if(item["typeId"]=="cube") cube=id;
            if(item["typeId"]=="scene") terminal=id;
        }
        const auto component=window.project().commands().extractComponent(selection,"scene-tool","Scene tool",
            project::Document::array(),project::Document::array({{{"id","scene"},{"source",{{"nodeId",terminal},{"portId","out"}}}}}),
            project::Document::array({{{"id","size"},{"target",{{"nodeId",cube},{"parameter","size"}}}}}));
        window.project().commands().instantiateComponent(component,"scene-instance",project::Document::object(),project::Document::object(),true);
        QtNodes::NodeId instance=QtNodes::InvalidNodeId;
        for(const auto id : window.canvas().allNodeIds()) if(window.canvas().projectId(id)=="scene-instance") instance=id;
        QVERIFY(instance!=QtNodes::InvalidNodeId);
        window.canvas().setNodeData(instance, QtNodes::NodeRole::Position, QPointF(300, 150));
        window.findChild<QtNodes::GraphicsView*>()->fitInView(window.scene().itemsBoundingRect(), Qt::KeepAspectRatio);
        window.selectNode(instance);
        window.findChild<QAction*>("pinOutput")->trigger();
        window.execution().run(); QTRY_VERIFY(window.execution().result().has_value());
        auto* viewer=dynamic_cast<SceneViewer*>(window.findChild<QWidget*>("sceneViewer"));
        QCOMPARE(viewer->objectCount(),size_t(1));
        const auto original=window.execution().result()->steps.at("scene-instance").output;
        const auto originalMax=dynamic_cast<const SceneMember*>(original->members().front().get())->objects.front().geometry.getMinMax().second.x;
        window.findChild<QDoubleSpinBox*>("parameter_size")->setValue(4);
        window.findChild<QPushButton*>("applyParameter")->click();
        window.execution().run(); QTRY_VERIFY(window.execution().result().has_value());
        const auto changed=window.execution().result()->steps.at("scene-instance").output;
        const auto changedMax=dynamic_cast<const SceneMember*>(changed->members().front().get())->objects.front().geometry.getMinMax().second.x;
        QVERIFY(changedMax>originalMax);
        window.scene().undoStack().undo();
        window.execution().run(); QTRY_VERIFY(window.execution().result().has_value());
        const auto undone=window.execution().result()->steps.at("scene-instance").output;
        QCOMPARE(dynamic_cast<const SceneMember*>(undone->members().front().get())->objects.front().geometry.getMinMax().second.x,originalMax);
        QCOMPARE(dynamic_cast<const SceneMember*>(original->members().front().get())->objects.front().geometry.getMinMax().second.x,originalMax);
        window.scene().undoStack().redo();
        window.execution().run(); QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(window.grab().save("component-collapsed-scene-smoke.png"));
        QTemporaryDir directory; const auto path=directory.filePath("collapsed-scene.smartflow"); window.saveProject(path);
        WorkspaceWindow reopened(sceneConfiguration()); reopened.execution().setLive(false); reopened.show(); reopened.openProject(path);
        reopened.execution().run(); QTRY_VERIFY(reopened.execution().result().has_value());
        QCOMPARE(dynamic_cast<SceneViewer*>(reopened.findChild<QWidget*>("sceneViewer"))->objectCount(),size_t(1));
        QCOMPARE(reopened.project().step("scene-instance")->parameterValue<double>("size"),4.0);
    }

    void shippedExampleRestoresSceneAndCamera()
    {
        WorkspaceWindow window(sceneConfiguration());
        window.show();
        window.openProject(SMARTFLOW_SCENE_EXAMPLE);
        QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(window.execution().result()->succeeded());
        QCOMPARE(scene(window,"cube").objects.front().geometry.getMinMax().second.x,1.5f);
        auto* viewer=dynamic_cast<SceneViewer*>(window.findChild<QWidget*>("sceneViewer"));
        QVERIFY(viewer);
        QCOMPARE(viewer->cameraAngles(),QPointF(60,15));
        QCOMPARE(viewer->selectedObject(),0);
        QCOMPARE(viewer->objectCount(),size_t(1));
        QVERIFY(!window.projectDirty());
        QTemporaryDir directory;
        const auto copy=directory.filePath("example-copy.smartflow");
        window.saveProject(copy);
        window.openProject(copy);
        QTRY_VERIFY(window.execution().result().has_value());
        QCOMPARE(viewer->cameraAngles(),QPointF(60,15));
        QVERIFY(window.grab().save("n5i-scene-example.png"));
    }

    void desktopSceneEditSaveAndReopenShowsResult()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const auto path=directory.filePath("desktop-scene.smartflow");
        WorkspaceWindow original(sceneConfiguration());
        original.show();
        QTRY_VERIFY(original.execution().result().has_value());
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        original.findChild<QDoubleSpinBox*>("parameter_size")->setValue(3);
        QTest::mouseClick(original.findChild<QPushButton*>("applyParameter"),Qt::LeftButton);
        QTRY_VERIFY(original.execution().result().has_value());
        QVERIFY(original.projectDirty());
        original.saveProject(path);
        QVERIFY(!original.projectDirty());
        WorkspaceWindow reopened(sceneConfiguration());
        reopened.show();
        reopened.openProject(path);
        QTRY_VERIFY(reopened.execution().result().has_value());
        QVERIFY(reopened.execution().result()->succeeded());
        QVERIFY(reopened.project().retained()==original.project().retained());
        QCOMPARE(scene(reopened,"cube").objects.front().geometry.getMinMax().second.x,1.5f);
        auto* viewer=dynamic_cast<SceneViewer*>(reopened.findChild<QWidget*>("sceneViewer"));
        QVERIFY(viewer);
        QCOMPARE(viewer->objectCount(),size_t(1));
        for(const auto id : reopened.canvas().allNodeIds())
            for(const auto originalId : original.canvas().allNodeIds())
                if(reopened.canvas().projectId(id)==original.canvas().projectId(originalId))
                    QCOMPARE(reopened.canvas().nodeData(id,QtNodes::NodeRole::Position).value<QPointF>(),
                             original.canvas().nodeData(originalId,QtNodes::NodeRole::Position).value<QPointF>());
        QVERIFY(!reopened.projectDirty());
    }

    void workspaceRoundTripPreservesOpaqueStateAndSemanticUndo()
    {
        QTemporaryDir directory;
        WorkspaceWindow original(sceneConfiguration());
        original.show();
        QTRY_VERIFY(original.execution().result().has_value());
        auto* viewer=dynamic_cast<SceneViewer*>(original.findChild<QWidget*>("sceneViewer"));
        const auto revision=original.project().revision();
        const auto cube=node(original,"cube");
        original.canvas().setNodeData(cube,QtNodes::NodeRole::Position,QPointF(-420,275));
        original.selectNode(cube);
        for(auto* action : original.findChildren<QAction*>())
            if(action->text()=="Pin output") action->trigger();
        original.selectNode(node(original,"transform"));
        auto* view=original.findChild<QtNodes::GraphicsView*>("graphCanvas");
        view->resetTransform(); view->scale(0.7,0.7); view->centerOn(200,100);
        viewer->restoreWorkspaceState({{"yaw",72},{"pitch",-12},{"span",9},
                                     {"target",QJsonArray{1,2,3}},{"selection",0}});
        const auto path=directory.filePath("workspace.smartflow");
        original.saveProject(path);
        QCOMPARE(original.project().revision(),revision);
        QCOMPARE(original.scene().undoStack().count(),0);
        auto file=project::read(path);
        auto& workspace=file["workspace"]["smartflow.native-editor@1"]["graph"];
        workspace["future"]={{"opaque",true}};
        workspace["positions"][original.canvas().projectId(cube).toString()]["future"]=42;
        workspace["viewers"]["smartflow.scene-3d.viewer@1"]["future"]="preserve";
        file["workspace"]["other-extension"]={{"data",17}};
        project::write(path,file);
        WorkspaceWindow reopened(sceneConfiguration());
        reopened.show();
        reopened.openProject(path);
        QTRY_VERIFY(reopened.execution().result().has_value());
        auto* restored=dynamic_cast<SceneViewer*>(reopened.findChild<QWidget*>("sceneViewer"));
        QCOMPARE(restored->workspaceState(),viewer->workspaceState());
        QCOMPARE(reopened.canvas().nodeData(node(reopened,"cube"),QtNodes::NodeRole::Position).value<QPointF>(),QPointF(-420,275));
        QCOMPARE(reopened.scene().selectedNodes().size(),size_t(1));
        QCOMPARE(reopened.canvas().projectId(reopened.scene().selectedNodes().front()),reopened.canvas().projectId(node(reopened,"transform")));
        QCOMPARE(reopened.findChild<QtNodes::GraphicsView*>("graphCanvas")->transform().m11(),0.7);
        QVERIFY(!reopened.projectDirty());
        QCOMPARE(reopened.project().retained()["workspace"]["smartflow.native-editor@1"]["graph"]["pinned"].get<std::string>(),
                 reopened.canvas().projectId(node(reopened,"cube")).toString());
        const auto camera=restored->workspaceState();
        edit(reopened,"cube","size",4);
        reopened.scene().undoStack().undo();
        QTRY_VERIFY(reopened.execution().result().has_value());
        QCOMPARE(restored->workspaceState(),camera);
        reopened.saveProject(path);
        const auto saved=project::read(path);
        const auto& result=saved["workspace"]["smartflow.native-editor@1"]["graph"];
        QVERIFY(result["future"]==workspace["future"]);
        QVERIFY(result["positions"][original.canvas().projectId(cube).toString()]["future"]==42);
        QVERIFY(result["viewers"]["smartflow.scene-3d.viewer@1"]["future"]=="preserve");
        QVERIFY(saved["workspace"]["other-extension"]==file["workspace"]["other-extension"]);
        const auto newRevision=reopened.project().revision();
        restored->frameScene();
        QVERIFY(reopened.projectDirty());
        QCOMPARE(reopened.project().revision(),newRevision);
    }

    void malformedViewerStateUsesDefaults()
    {
        SceneViewer viewer;
        viewer.restoreWorkspaceState({{"yaw","future"},{"pitch",9999},{"span",-1},
                                      {"target",QJsonArray{1e200,"bad",0}},{"selection",1e200}});
        QCOMPARE(viewer.cameraAngles(),QPointF(35,25));
        QCOMPARE(viewer.selectedObject(),-1);
        QCOMPARE(viewer.workspaceState().value("span").toDouble(),6.0);
        QCOMPARE(viewer.workspaceState().value("target").toArray(),QJsonArray({0,0,0}));
    }

    void persistedSceneGraphExecutesThroughExplicitIdentities()
    {
        const auto config=sceneConfiguration();
        auto file=project::create("scene-project");
        auto nodes=project::Document::array();
        GraphProject definitions(config.delegates,config.nodes);
        for(size_t i=0; i<config.preset.size(); ++i) {
            const auto& preset=config.preset[i];
            const auto entry=std::find_if(config.nodes.begin(),config.nodes.end(),[&](const auto& n) { return n.type==preset.type; });
            QVERIFY(entry!=config.nodes.end());
            auto* defaults=definitions.create(preset.type.toStdString());
            auto parameters=project::Document::object();
            for(const auto& [name,p] : defaults->parameters()) parameters[name.toString()]=std::get<double>(p.value);
            nodes.push_back({{"id",std::to_string(i)},{"packageId",entry->packageId.toStdString()},
                {"typeId",entry->typeId.toStdString()},{"version",entry->contractVersion},{"parameters",parameters}});
        }
        auto edges=project::Document::array();
        for(size_t i=0; i<config.connections.size(); ++i) {
            const auto& edge=config.connections[i];
            edges.push_back({{"id",std::to_string(i)},{"source",{{"nodeId",std::to_string(edge.source)},{"portId","out"}}},
                            {"target",{{"nodeId",std::to_string(edge.target)},{"portId","in"}}}});
        }
        file["project"]["graphs"]={{{"id","scene"},{"nodes",nodes},{"connections",edges}}};
        project::DocumentSession session(project::parse(project::serialize(file)),"scene",config.delegates,config.nodes);
        QVERIFY(session.diagnostics().empty());
        session.setParameter("0","size",3.0);
        PipelineExecution executor;
        auto handle=executor.submit(session.executableGraph(),config.delegates,config.factory);
        QVERIFY(handle.result.wait_for(std::chrono::seconds(10))==std::future_status::ready);
        const auto result=handle.result.get();
        QVERIFY(result.succeeded());
        const auto& output=result.steps.at("3").output;
        const auto* scene=dynamic_cast<const SceneMember*>(output->members().front().get());
        QVERIFY(scene);
        QCOMPARE(scene->objects.size(),size_t(1));
        QVERIFY(scene->objects.front().geometry.getMinMax().second.x > 1.5f);
    }

    void primitiveTransformMaterialAndSnapshotIsolation()
    {
        WorkspaceWindow window(sceneConfiguration());
        QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(window.execution().result()->succeeded());
        const auto& source = scene(window,"cube").objects.front().geometry;
        QCOMPARE(source.verts.size(), size_t(36));
        const auto bounds = source.getMinMax();
        QVERIFY(bounds.first == glm::vec3(-1));
        QVERIFY(bounds.second == glm::vec3(1));
        for(const auto& vertex : source.verts) QVERIFY(std::abs(glm::length(vertex.normal)-1) < 0.0001f);
        const auto oldResult = *window.execution().result();
        edit(window,"transform","rotation Y",0);
        edit(window,"transform","x",3);
        edit(window,"transform","scale X",2);
        edit(window,"transform","scale Y",3);
        edit(window,"transform","scale Z",4);
        QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(window.execution().result()->succeeded());
        const auto transformed = scene(window,"transform").objects.front().geometry.getMinMax();
        QVERIFY(transformed.first == glm::vec3(1,-3,-4));
        QVERIFY(transformed.second == glm::vec3(5,3,4));
        scene(window,"transform").objects.front().geometry.forEachTriangle(
            [](const auto& a, const auto& b, const auto& c, int, int, int) {
                const auto normal = glm::normalize(glm::cross(b.vert-a.vert,c.vert-a.vert));
                QVERIFY(glm::length(a.normal-normal) < 0.0001f);
            });
        QVERIFY(scene(window,"cube").objects.front().geometry.getMinMax().first == glm::vec3(-1));
        glm::vec3 color;
        scene(window,"scene").objects.front().geometry.material.viewOpenGL([&](const auto& m) { color=m.albedo; });
        QVERIFY(glm::length(color-glm::vec3(0.18f,0.58f,0.88f)) < 0.0001f);
        edit(window,"material","red",0.9);
        QTRY_VERIFY(window.execution().result().has_value());
        scene(window,"scene").objects.front().geometry.material.viewOpenGL([&](const auto& m) { color=m.albedo; });
        QVERIFY(std::abs(color.r-0.9f) < 0.0001f);
        const auto& old = oldResult.steps.at(window.canvas().projectId(node(window,"scene"))).output;
        dynamic_cast<const SceneMember*>(old->members().front().get())->objects.front().geometry.material.viewOpenGL(
            [&](const auto& m) { color=m.albedo; });
        QVERIFY(std::abs(color.r-0.18f) < 0.0001f);
    }

    void inspectorEditUpdatesPinnedViewerAndUndo()
    {
        WorkspaceWindow window(sceneConfiguration());
        window.show();
        QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(window.execution().result()->succeeded());
        auto* viewer = dynamic_cast<SceneViewer*>(window.findChild<QWidget*>("sceneViewer"));
        QVERIFY(viewer);
        QCOMPARE(viewer->objectCount(), size_t(1));
        const auto before = viewer->grab().toImage();
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        auto* spin = window.findChild<QDoubleSpinBox*>("parameter_size");
        QVERIFY(spin);
        spin->setValue(3);
        QTest::mouseClick(window.findChild<QPushButton*>("applyParameter"), Qt::LeftButton);
        QCOMPARE(viewer->objectCount(), size_t(0));
        QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(scene(window,"scene").objects.front().geometry.getMinMax().second.x > 1.5f);
        QVERIFY(viewer->grab().toImage() != before);
        window.scene().undoStack().undo();
        QTRY_VERIFY(window.execution().result().has_value());
        QCOMPARE(viewer->grab().toImage(), before);
        window.scene().undoStack().redo();
        QTRY_VERIFY(window.execution().result().has_value());
        QCOMPARE(scene(window,"cube").objects.front().geometry.getMinMax().second.x, 1.5f);
    }

    void cameraAndSelectionDoNotEditProject()
    {
        WorkspaceWindow window(sceneConfiguration());
        window.show();
        QTRY_VERIFY(window.execution().result().has_value());
        auto* viewer = dynamic_cast<SceneViewer*>(window.findChild<QWidget*>("sceneViewer"));
        QVERIFY(viewer);
        viewer->grab(); // Populate picking polygons through the paint path.
        const auto revision = window.project().revision();
        const auto initial = viewer->cameraAngles();
        const auto center = viewer->rect().center();
        QTest::mouseClick(viewer,Qt::LeftButton,Qt::NoModifier,center);
        QCOMPARE(viewer->selectedObject(),0);
        QTest::mousePress(viewer,Qt::LeftButton,Qt::NoModifier,center);
        QMouseEvent move(QEvent::MouseMove,QPointF(center+QPoint(35,20)),QPointF(center+QPoint(35,20)),
                         Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
        QApplication::sendEvent(viewer,&move);
        QTest::mouseRelease(viewer,Qt::LeftButton,Qt::NoModifier,center+QPoint(35,20));
        QVERIFY(viewer->cameraAngles() != initial);
        QVERIFY(window.projectDirty());
        QCOMPARE(window.project().revision(),revision);
        QCOMPARE(window.scene().undoStack().count(),0);
        const auto orbited = viewer->cameraAngles();
        edit(window,"cube","size",3);
        QTRY_VERIFY(window.execution().result().has_value());
        QCOMPARE(viewer->cameraAngles(),orbited);
        const auto afterEdit = window.project().revision();
        viewer->frameScene();
        QCOMPARE(window.project().revision(),afterEdit);
        QVERIFY(afterEdit > revision);
        QCOMPARE(window.scene().undoStack().count(),1);
    }

    void mergeAndInvalidInputFeedback()
    {
        auto config = sceneConfiguration();
        config.preset.push_back({"smartflow.scene-3d.merge@1", {750,0}, {}});
        config.connections.push_back({0,0,4,0});
        config.connections.push_back({2,0,4,1});
        WorkspaceWindow window(std::move(config));
        QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(window.execution().result()->succeeded());
        QCOMPARE(scene(window,"merge").objects.size(),size_t(2));
        const auto cubeNode = node(window,"cube");
        const auto transformNode = node(window,"transform");
        QVERIFY(window.canvas().deleteConnection({cubeNode,0,transformNode,0}));
        QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(!window.execution().result()->succeeded());
        auto* viewer = dynamic_cast<SceneViewer*>(window.findChild<QWidget*>("sceneViewer"));
        QCOMPARE(viewer->objectCount(),size_t(0));
        window.canvas().addConnection({cubeNode,0,transformNode,0});
        // Invalid persisted parameters are retained, diagnosed and never run.
        auto file=window.project().retained();
        const auto stable=window.canvas().projectId(cubeNode).toString();
        for(auto& item : file["project"]["graphs"][0]["nodes"])
            if(item["id"]==stable) item["parameters"]["size"]=-1.0;
        window.project().commands().replace(file,"graph");
        window.execution().run();
        QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(!window.execution().result()->succeeded());
        QVERIFY(window.execution().result()->steps.empty());
        QVERIFY(!window.execution().result()->diagnostics.empty());
    }

    void oversizedSceneIsRejected()
    {
        auto config = sceneConfiguration();
        size_t previous = 0;
        for(int i=0; i<7; ++i) {
            const auto next = config.preset.size();
            config.preset.push_back({"smartflow.scene-3d.merge@1", {double(i*100),200}, {}});
            config.connections.push_back({previous,0,next,0});
            config.connections.push_back({previous,0,next,1});
            previous = next;
        }
        WorkspaceWindow window(std::move(config));
        QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(!window.execution().result()->succeeded());
        bool foundLimit = false;
        for(const auto& [id, step] : window.execution().result()->steps)
            if(step.state == StepState::Failed && step.error.find("64 objects") != std::string::npos) foundLimit = true;
        QVERIFY(foundLimit);
    }
};
QTEST_MAIN(SceneTests)
#include "SceneTests.moc"
