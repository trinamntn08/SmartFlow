#include "workspace/WorkspaceWindow.h"
#include "workspace/PanelWorkspace.h"
#include <DataExtension.h>
#include <QAction>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFontDatabase>
#include <QPushButton>
#include <QSplitter>
#include <QTemporaryDir>
#include <QToolButton>
#include <QTreeWidget>
#include <QtNodes/GraphicsView>
#include <QtTest/QtTest>

using namespace smartflow;
using project::Document;

class PanelWorkspaceTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        const auto path=qEnvironmentVariable("SMARTFLOW_TEST_FONT");
        if(!path.isEmpty()) {
            const auto id=QFontDatabase::addApplicationFont(path); QVERIFY(id>=0);
            QApplication::setFont(QFont(QFontDatabase::applicationFontFamilies(id).first()));
        }
    }

    void chooseSplitCloseAndSaveReopenWithoutSemanticEdits()
    {
        WorkspaceWindow window(data::dataConfiguration()); window.execution().setLive(false); window.show();
        QTest::qWait(40);
        auto* panels=window.findChild<PanelWorkspace*>(); QVERIFY(panels);
        auto* graph=window.findChild<QtNodes::GraphicsView*>("graphCanvas");
        graph->resetTransform(); graph->scale(0.7,0.7);
        auto* minimum=window.findChild<QDoubleSpinBox*>("parameter_multiplier"); QVERIFY(minimum);
        const auto project=window.project().retained()["project"];
        const auto revision=window.project().revision();
        const auto history=window.scene().undoStack().count();
        auto* choice=panels->regionForPanel("library")->findChild<QComboBox*>("panelChoice");
        choice->setCurrentIndex(choice->findData("graph")); // Swap Graph with the Node library.
        QVERIFY(panels->regionForPanel("graph")->isAncestorOf(graph));
        QCOMPARE(graph->transform().m11(),0.7);
        QVERIFY(window.findChild<QDoubleSpinBox*>("parameter_multiplier")==minimum);
        auto* region=panels->regionForPanel("graph");
        region->findChild<QToolButton*>("splitRegionBelow")->click();
        int empty=0;
        for(auto* selector : panels->findChildren<QComboBox*>("panelChoice")) if(selector->isVisible() && selector->currentData().toString().isEmpty()) {
            ++empty; selector->setCurrentIndex(selector->findData("viewer")); break;
        }
        QCOMPARE(empty,1);
        QVERIFY(panels->regionForPanel("viewer"));
        panels->regionForPanel("results")->findChild<QToolButton*>("closePanelRegion")->click();
        QVERIFY(!panels->regionForPanel("results"));
        QVERIFY(window.findChild<QTreeWidget*>("executionResults")); // Hidden, not destroyed.
        QTest::qWait(40);
        QCOMPARE(window.project().revision(),revision); QCOMPARE(window.scene().undoStack().count(),history);
        QVERIFY(window.project().retained()["project"]==project); QVERIFY(window.projectDirty());
        auto* mainSplit=panels->findChild<QSplitter*>(); QVERIFY(mainSplit);
        mainSplit->setSizes({300,840}); QTest::qWait(30);
        QVERIFY(window.grab().save("selectable-panels-smoke.png"));
        QTemporaryDir directory; const auto path=directory.filePath("panels.smartflow");
        window.saveProject(path); const auto savedLayout=panels->saveLayout();
        WorkspaceWindow reopened(data::dataConfiguration()); reopened.execution().setLive(false); reopened.show(); reopened.openProject(path);
        auto* restored=reopened.findChild<PanelWorkspace*>(); QVERIFY(restored);
        // Splitter ratios are restored by Qt; metadata and leaf placement must match.
        const auto restoredLayout=restored->saveLayout();
        const auto originalSizes=savedLayout["root"]["sizes"], reopenedSizes=restoredLayout["root"]["sizes"];
        const auto originalRatio=originalSizes[0].get<double>()/(originalSizes[0].get<double>()+originalSizes[1].get<double>());
        const auto reopenedRatio=reopenedSizes[0].get<double>()/(reopenedSizes[0].get<double>()+reopenedSizes[1].get<double>());
        QVERIFY(std::abs(originalRatio-reopenedRatio)<0.04);
        std::function<QStringList(const Document&)> leaves=[&](const Document& tree) -> QStringList {
            if(tree.contains("panel")) return {QString::fromStdString(tree["panel"])};
            return leaves(tree["children"][0])+leaves(tree["children"][1]);
        };
        QCOMPARE(leaves(restoredLayout["root"]),leaves(savedLayout["root"]));
        QVERIFY(!restored->regionForPanel("results"));
        QVERIFY(reopened.project().retained()["project"]==project);
        reopened.execution().run(); QTRY_VERIFY(reopened.execution().result().has_value());
        QVERIFY(reopened.execution().result()->succeeded());
        reopened.findChild<QAction*>("resetWidgetLayout")->trigger();
        QVERIFY(restored->regionForPanel("results")); QVERIFY(restored->regionForPanel("graph"));
    }

    void unknownPanelsMetadataAndUnsupportedLayoutSurvive()
    {
        WorkspaceWindow window; window.execution().setLive(false); window.show(); QTest::qWait(30);
        auto* panels=window.findChild<PanelWorkspace*>(); auto layout=panels->saveLayout();
        layout["future"]={{"integer",uint64_t(18446744073709551615ULL)}};
        layout["root"]["futureRoot"]=true;
        layout["root"]["children"][0]["panel"]="future.extension.widget";
        layout["root"]["children"][0]["futureLeaf"]=7;
        QVERIFY(panels->restoreLayout(layout));
        QVERIFY(panels->regionForPanel("future.extension.widget"));
        const auto saved=panels->saveLayout();
        QVERIFY(saved["future"]==layout["future"]); QVERIFY(saved["root"]["futureRoot"]==true);
        QVERIFY(saved["root"]["children"][0]["futureLeaf"]==7);
        auto invalid=layout; invalid["root"]["children"][1]["children"][0]["panel"]="future.extension.widget";
        // Replace the entire subtree with a duplicate leaf.
        invalid["root"]["children"][1]["children"][0]={{"panel","future.extension.widget"}};
        QVERIFY(!panels->restoreLayout(invalid));
        QVERIFY(panels->saveLayout()==saved);
        QTemporaryDir directory; const auto path=directory.filePath("future.smartflow");
        auto source=window.project().retained();
        const auto id=window.project().selectedGraph()["id"].get<std::string>();
        const Document future={{"version",99},{"opaque","retain"}};
        source["workspace"]["smartflow.native-editor@1"][id]["panels"]=future;
        window.loadDocument(source); window.saveProject(path);
        QVERIFY(project::read(path)["workspace"]["smartflow.native-editor@1"][id]["panels"]==future);
        window.findChild<QAction*>("resetWidgetLayout")->trigger(); window.saveProject(path);
        QVERIFY(project::read(path)["workspace"]["smartflow.native-editor@1"][id]["panels"]["version"]==1);
    }
};

QTEST_MAIN(PanelWorkspaceTests)
#include "PanelWorkspaceTests.moc"
