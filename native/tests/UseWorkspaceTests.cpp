#include "workspace/WorkspaceWindow.h"
#include "workspace/UseWorkspace.h"
#include "workspace/PanelWorkspace.h"
#include <DataExtension.h>
#include <SceneExtension.h>
#include <SceneViewer.h>
#include <QtNodes/GraphicsView>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFontDatabase>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QUndoStack>
#include <QtTest/QtTest>

using namespace smartflow;
using project::Document;
namespace {
void flushEditors()
{
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
}
void apply(WorkspaceWindow& window, const QString& control, double value)
{
    flushEditors();
    window.findChild<QDoubleSpinBox*>("useParameter_" + control)->setValue(value);
    window.findChild<QPushButton*>("useApply_" + control)->click();
}
const data::TableMember& table(WorkspaceWindow& window, const char* port)
{
    const auto* step = window.project().step("summary-tool");
    const auto& output = window.execution().result()->steps.at("summary-tool").output;
    for(const auto& mapping : step->outputMapping())
        if(mapping.portName == port) return *dynamic_cast<const data::TableMember*>(output->member(mapping.dataName).get());
    throw std::runtime_error("Missing output");
}
Document panelStructure(Document state)
{
    state.erase("sizes");
    if(state.contains("children")) for(auto& child : state["children"]) child = panelStructure(child);
    return state;
}
}

class UseWorkspaceTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        const auto path = qEnvironmentVariable("SMARTFLOW_TEST_FONT");
        if(path.isEmpty()) return;
        const auto id = QFontDatabase::addApplicationFont(path);
        QVERIFY(id >= 0);
        QApplication::setFont(QFont(QFontDatabase::applicationFontFamilies(id).first(), 10));
    }
    void dataControlsShareHistoryExecutionAndSave()
    {
        WorkspaceWindow window(data::dataConfiguration());
        window.show(); window.openProject(SMARTFLOW_COMPONENT_EXAMPLE);
        QTRY_VERIFY(window.execution().result());
        const auto projectBefore = window.project().retained()["project"];
        const auto revision = window.project().revision();
        auto* mode = window.findChild<QComboBox*>("workspaceMode");
        mode->setCurrentIndex(1);
        QVERIFY(window.useMode());
        QVERIFY(window.findChild<UseWorkspace*>()->isVisible());
        QVERIFY(!window.findChild<PanelWorkspace*>()->isVisible());
        QVERIFY(window.project().retained()["project"] == projectBefore);
        QCOMPARE(window.project().revision(), revision);
        QVERIFY(!window.findChild<QDoubleSpinBox*>("useParameter_multiplier"));
        auto* outputs = window.findChild<QComboBox*>("useOutput");
        outputs->setCurrentIndex(outputs->findText("summary"));
        auto* use = window.findChild<UseWorkspace*>();
        auto* view = use->findChild<QTableWidget*>();
        QTRY_COMPARE(view->item(1, 1)->text(), QString("79"));
        apply(window, "minimum", 20);
        QTRY_VERIFY(window.execution().result());
        QCOMPARE(table(window, "summary").rows[1].value, 104.0);
        QTRY_COMPARE(view->item(1, 1)->text(), QString("104"));
        QVERIFY(window.projectDirty());
        window.scene().undoStack().undo();
        QTRY_VERIFY(window.execution().result());
        QCOMPARE(table(window, "summary").rows[1].value, 79.0);
        window.scene().undoStack().redo();
        QTRY_VERIFY(window.execution().result());
        QCOMPARE(table(window, "summary").rows[1].value, 104.0);
        outputs->setCurrentIndex(outputs->findText("rows"));
        QCOMPARE(view->rowCount(), 3);
        QTemporaryDir directory;
        const auto path = directory.filePath("tool.smartflow");
        window.saveProject(path);
        QVERIFY(!window.projectDirty());
        WorkspaceWindow reopened(data::dataConfiguration());
        reopened.show(); reopened.openProject(path);
        QVERIFY(reopened.useMode());
        QCOMPARE(reopened.findChild<QComboBox*>("useOutput")->currentText(), QString("rows"));
        QTRY_VERIFY(reopened.execution().result());
        QCOMPARE(table(reopened, "summary").rows[1].value, 104.0);
        QVERIFY(!reopened.projectDirty());
        QVERIFY(window.grab().save("use-data-smoke.png"));
    }
    void modesPreserveBuildLayoutSelectionAndNavigation()
    {
        WorkspaceWindow window(data::dataConfiguration());
        window.show(); window.openProject(SMARTFLOW_COMPONENT_EXAMPLE);
        QTRY_VERIFY(window.execution().result());
        auto* panels = window.findChild<PanelWorkspace*>();
        const auto layout = panels->saveLayout();
        const auto selected = window.scene().selectedNodes();
        auto* view = window.findChild<QtNodes::GraphicsView*>("graphCanvas");
        const auto transform = view->transform();
        window.setUseMode(true);
        const auto navigation = window.project().retained()["workspace"]["smartflow.native-editor@1"]["graph"]["navigation"];
        window.resize(1000, 680);
        QTemporaryDir directory;
        window.saveProject(directory.filePath("use.smartflow"));
        QVERIFY(window.project().retained()["workspace"]["smartflow.native-editor@1"]["graph"]["navigation"] == navigation);
        QVERIFY(window.project().retained()["workspace"]["smartflow.native-editor@1"]["graph"]["panels"] == layout);
        window.setUseMode(false);
        QVERIFY(panelStructure(panels->saveLayout()["root"]) == panelStructure(layout["root"]));
        QVERIFY(window.scene().selectedNodes() == selected);
        QCOMPARE(view->transform(), transform);
        QVERIFY(view->isVisible());
    }
    void explicitRunsClearOutdatedOutputsAndRejectStaleEditors()
    {
        WorkspaceWindow window(data::dataConfiguration());
        window.show(); window.openProject(SMARTFLOW_COMPONENT_EXAMPLE);
        window.setUseMode(true);
        QTRY_VERIFY(window.execution().result());
        window.execution().setLive(false);
        flushEditors();
        auto* staleSpin = window.findChild<QDoubleSpinBox*>("useParameter_minimum");
        auto* staleApply = window.findChild<QPushButton*>("useApply_minimum");
        staleSpin->setValue(99);
        window.project().commands().setParameter("summary-tool", "minimum", 20);
        const auto revision = window.project().revision();
        staleApply->click();
        QCOMPARE(window.project().revision(), revision);
        QCOMPARE(std::get<double>(window.project().step("summary-tool")->parameter("minimum").value), 20.0);
        QVERIFY(!window.execution().result());
        auto* view = window.findChild<UseWorkspace*>()->findChild<QTableWidget*>();
        QCOMPARE(view->rowCount(), 0);
        QVERIFY(window.findChild<QLabel*>("useStatus")->text().contains("Outdated"));
        window.findChild<QPushButton*>("runGraph")->click();
        QTRY_VERIFY(window.execution().result());
        QCOMPARE(table(window, "summary").rows[1].value, 104.0);
        window.findChild<QPushButton*>("runGraph")->click();
        window.findChild<QPushButton*>("cancelGraph")->click();
        QVERIFY(!window.execution().result());
        QCOMPARE(view->rowCount(), 0);
        QTRY_VERIFY(window.findChild<QLabel*>("useStatus")->text().contains("Cancelled"));
    }
    void unavailableEmptyAndFutureWorkspacesRetainContent()
    {
        WorkspaceWindow window(data::dataConfiguration());
        window.show(); window.setUseMode(true);
        QCOMPARE(window.findChild<QComboBox*>("useComponent")->count(), 0);
        QVERIFY(window.findChild<UseWorkspace*>()->findChildren<QDoubleSpinBox*>().empty());
        auto source = project::read(SMARTFLOW_COMPONENT_EXAMPLE);
        source["project"]["graphs"][0]["nodes"][1]["component"]["graph"]["nodes"][0]["packageId"] = "future";
        source["workspace"]["smartflow.native-use@1"]["graph"] = {
            {"mode", "use"}, {"future", {{"opaque", 42}}}, {"viewers", "future-shape"}};
        window.loadDocument(source);
        QVERIFY(window.useMode());
        QTRY_VERIFY(window.execution().result());
        QVERIFY(!window.execution().result()->succeeded());
        QVERIFY(window.findChild<QLabel*>("useStatus")->text().contains("unsupported", Qt::CaseInsensitive)
                || window.findChild<QLabel*>("useStatus")->text().contains("unavailable", Qt::CaseInsensitive));
        QTemporaryDir directory;
        const auto path = directory.filePath("unknown.smartflow");
        window.saveProject(path);
        auto saved = project::read(path);
        QVERIFY(saved["project"] == source["project"]);
        QCOMPARE(saved["workspace"]["smartflow.native-use@1"]["graph"]["future"]["opaque"].get<int>(), 42);
        QVERIFY(saved["workspace"]["smartflow.native-use@1"]["graph"]["viewers"] == "future-shape");
        source["workspace"]["smartflow.native-use@1"] = "unknown-namespace";
        window.loadDocument(source);
        window.saveProject(path);
        QVERIFY(project::read(path)["workspace"]["smartflow.native-use@1"] == "unknown-namespace");
    }
    void sceneToolControlsUpdateViewerAndKeepCameraSeparate()
    {
        WorkspaceWindow window(scene3d::sceneConfiguration());
        window.show(); window.openProject(SMARTFLOW_SCENE_TOOL_EXAMPLE);
        QVERIFY(window.useMode());
        QTRY_VERIFY(window.execution().result());
        QVERIFY(window.execution().result()->succeeded());
        auto* useViewer = dynamic_cast<scene3d::SceneViewer*>(window.findChild<QWidget*>("useResultViewer"));
        QVERIFY(useViewer);
        QCOMPARE(useViewer->objectCount(), size_t(1));
        const auto before = window.execution().result()->steps.at("scene-tool").output;
        const auto* sceneBefore = dynamic_cast<const scene3d::SceneMember*>(before->members().front().get());
        const auto vertex = sceneBefore->objects.front().geometry.verts.front().vert;
        apply(window, "size", 6);
        QTRY_VERIFY(window.execution().result());
        const auto* sceneAfter = dynamic_cast<const scene3d::SceneMember*>(window.execution().result()->steps.at("scene-tool").output->members().front().get());
        const auto actual = sceneAfter->objects.front().geometry.verts.front().vert;
        QVERIFY(std::abs(actual.x - 2 * vertex.x) < 1e-5);
        QVERIFY(std::abs(actual.y - 2 * vertex.y) < 1e-5);
        const auto revision = window.project().revision();
        const auto graph = window.project().retained()["project"];
        useViewer->restoreWorkspaceState({{"yaw", 80}, {"pitch", 10}});
        QTemporaryDir directory;
        const auto path = directory.filePath("scene-tool.smartflow");
        window.saveProject(path);
        QCOMPARE(window.project().revision(), revision);
        QVERIFY(window.project().retained()["project"] == graph);
        const auto& native = window.project().retained()["workspace"]["smartflow.native-editor@1"]["graph"];
        QCOMPARE(native["viewers"]["smartflow.scene-3d.viewer@1"]["yaw"].get<int>(), 60);
        WorkspaceWindow reopened(scene3d::sceneConfiguration());
        reopened.show(); reopened.openProject(path);
        auto* restored = dynamic_cast<scene3d::SceneViewer*>(reopened.findChild<QWidget*>("useResultViewer"));
        QVERIFY(restored);
        QCOMPARE(restored->cameraAngles(), QPointF(80, 10));
        window.scene().undoStack().undo();
        QTRY_VERIFY(window.execution().result());
        QCOMPARE(std::get<double>(window.project().step("scene-tool")->parameter("size").value), 3.0);
        QVERIFY(window.grab().save("use-scene-smoke.png"));
    }
};
QTEST_MAIN(UseWorkspaceTests)
#include "UseWorkspaceTests.moc"
