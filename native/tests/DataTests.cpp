#include <DataExtension.h>
#include <TableViewer.h>
#include "workspace/WorkspaceWindow.h"
#include "project/ProjectFile.h"
#include <QTableWidget>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QTemporaryDir>
#include <QUndoStack>
#include <QFontDatabase>
#include <QFileInfo>
#include <QDir>
#include <QtTest/QtTest>

using namespace smartflow;
using namespace smartflow::data;
namespace {
QtNodes::NodeId node(WorkspaceWindow& window, const char* name)
{
    const auto type=std::string("smartflow.data.")+name+"@1";
    for(const auto id : window.canvas().allNodeIds())
        if(const auto* step=window.project().step(window.canvas().projectId(id)))
            if(step->delegateName().toString()==type) return id;
    return QtNodes::InvalidNodeId;
}
const TableMember& output(WorkspaceWindow& window, const char* name)
{
    const auto& value=window.execution().result()->steps.at(window.canvas().projectId(node(window,name))).output;
    return *dynamic_cast<const TableMember*>(value->members().front().get());
}
void minimum(WorkspaceWindow& window, double value)
{
    const auto id=window.canvas().projectId(node(window,"filter"));
    auto p=window.project().step(id)->parameter("minimum");
    p.value=value;
    window.project().setParameter(id,p);
}
}

class DataTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void sharedBrowserDataExpectations()
    {
        const auto path=QFileInfo(QString::fromUtf8(SMARTFLOW_DATA_EXAMPLE)).dir().filePath("../tests/fixtures/data-results-v1.json");
        const auto fixture=project::jsonFile::read(path);
        WorkspaceWindow window(dataConfiguration());
        QTRY_VERIFY(window.execution().result());
        window.execution().setLive(false);
        for(const auto& expected : fixture["cases"]) {
            const auto source=window.canvas().projectId(node(window,"sample"));
            auto parameter=window.project().step(source)->parameter("multiplier");
            parameter.value=expected["multiplier"].get<double>();
            window.project().setParameter(source,parameter);
            minimum(window,expected["minimum"].get<double>());
            window.execution().run();
            QTRY_VERIFY(!window.execution().busy());
            QVERIFY(window.execution().result());
            QVERIFY(window.execution().result()->succeeded());
            const auto& rows=output(window,"summary").rows;
            QCOMPARE(rows[0].value,expected["count"].get<double>());
            QCOMPARE(rows[1].value,expected["total"].get<double>());
            QVERIFY(std::abs(rows[2].value-expected["mean"].get<double>())<1e-10);
        }
    }
    void parallelDataMatchesSequential() {
        WorkspaceWindow window(dataConfiguration());
        QTRY_VERIFY(window.execution().result());
        const auto total=output(window,"summary").rows[1].value;
        window.execution().setScheduling(ExecutionMode::Parallel,4);
        window.execution().run(); QTRY_VERIFY(window.execution().result());
        QVERIFY(window.execution().result()->succeeded());
        QCOMPARE(output(window,"summary").rows[1].value,total);
    }
    void initTestCase()
    {
        const auto path=qEnvironmentVariable("SMARTFLOW_TEST_FONT");
        if(path.isEmpty()) return;
        const auto id=QFontDatabase::addApplicationFont(path);
        QVERIFY(id>=0);
        QApplication::setFont(QFont(QFontDatabase::applicationFontFamilies(id).first(),10));
    }

    void sampleFilterSummaryAndSnapshotIsolation()
    {
        WorkspaceWindow window(dataConfiguration());
        QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(window.execution().result()->succeeded());
        QCOMPARE(output(window,"sample").rows.size(),size_t(6));
        QCOMPARE(output(window,"filter").rows.size(),size_t(3));
        QCOMPARE(output(window,"summary").rows[0].value,3.0);
        QCOMPARE(output(window,"summary").rows[1].value,104.0);
        QVERIFY(std::abs(output(window,"summary").rows[2].value-104.0/3)<1e-10);
        const auto previous=*window.execution().result();
        minimum(window,30);
        QTRY_VERIFY(window.execution().result().has_value());
        QCOMPARE(output(window,"summary").rows[0].value,2.0);
        QCOMPARE(output(window,"summary").rows[1].value,79.0);
        const auto& old=previous.steps.at(window.canvas().projectId(node(window,"filter"))).output;
        QCOMPARE(dynamic_cast<const TableMember*>(old->members().front().get())->rows.size(),size_t(3));
        QCOMPARE(output(window,"sample").rows[1].value,25.0);
        minimum(window,1000);
        QTRY_VERIFY(window.execution().result().has_value());
        QCOMPARE(output(window,"filter").rows.size(),size_t(0));
        for(const auto& row : output(window,"summary").rows) QCOMPARE(row.value,0.0);
    }

    void shippedExampleRestoresDataAndViewer()
    {
        WorkspaceWindow window(dataConfiguration());
        window.show();
        window.openProject(SMARTFLOW_DATA_EXAMPLE);
        QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(window.execution().result()->succeeded());
        QCOMPARE(output(window,"summary").rows[1].value,79.0);
        auto* table=window.findChild<QWidget*>("tableViewer")->findChild<QTableWidget*>("outputTable");
        QCOMPARE(table->item(2,1)->text(),QString("39.5"));
        QVERIFY(!table->selectionModel()->selectedRows().isEmpty());
        QCOMPARE(table->selectionModel()->selectedRows().front().row(),1);
        QVERIFY(!window.projectDirty());
        QTemporaryDir directory;
        const auto copy=directory.filePath("example-copy.smartflow");
        window.saveProject(copy);
        window.openProject(copy);
        QTRY_VERIFY(window.execution().result().has_value());
        QCOMPARE(output(window,"summary").rows[1].value,79.0);
        QVERIFY(window.grab().save("n5i-data-example.png"));
    }

    void inspectorUndoSaveAndFreshWindowRestore()
    {
        QTemporaryDir directory;
        WorkspaceWindow original(dataConfiguration());
        original.show();
        QTRY_VERIFY(original.execution().result().has_value());
        original.selectNode(node(original,"filter"));
        QCoreApplication::sendPostedEvents(nullptr,QEvent::DeferredDelete);
        auto* spin=original.findChild<QDoubleSpinBox*>("parameter_minimum");
        QVERIFY(spin);
        spin->setValue(30);
        original.findChild<QPushButton*>("applyParameter")->click();
        QTRY_VERIFY(original.execution().result().has_value());
        QCOMPARE(output(original,"summary").rows[1].value,79.0);
        original.scene().undoStack().undo();
        QTRY_VERIFY(original.execution().result().has_value());
        QCOMPARE(output(original,"summary").rows[1].value,104.0);
        original.scene().undoStack().redo();
        QTRY_VERIFY(original.execution().result().has_value());
        auto* table=original.findChild<QWidget*>("tableViewer")->findChild<QTableWidget*>("outputTable");
        QCOMPARE(table->rowCount(),3);
        const auto revision=original.project().revision();
        table->selectRow(1);
        QCOMPARE(original.project().revision(),revision);
        original.canvas().setNodeData(node(original,"filter"),QtNodes::NodeRole::Position,QPointF(370,145));
        const auto path=directory.filePath("data.smartflow");
        original.saveProject(path);
        auto saved=project::read(path);
        saved["workspace"]["smartflow.native-editor@1"]["graph"]["viewers"]["smartflow.data.table-viewer@1"]["future"]="retain";
        project::write(path,saved);
        WorkspaceWindow reopened(dataConfiguration());
        reopened.show();
        reopened.openProject(path);
        QTRY_VERIFY(reopened.execution().result().has_value());
        QVERIFY(reopened.execution().result()->succeeded());
        QCOMPARE(output(reopened,"summary").rows[1].value,79.0);
        auto* restored=reopened.findChild<QWidget*>("tableViewer")->findChild<QTableWidget*>("outputTable");
        QCOMPARE(restored->rowCount(),3);
        QCOMPARE(restored->item(1,1)->text(),QString("79"));
        QCOMPARE(restored->selectionModel()->selectedRows().front().row(),1);
        QCOMPARE(reopened.canvas().nodeData(node(reopened,"filter"),QtNodes::NodeRole::Position).value<QPointF>(),QPointF(370,145));
        QVERIFY(!reopened.projectDirty());
        reopened.saveProject(path);
        QVERIFY(project::read(path)["workspace"]["smartflow.native-editor@1"]["graph"]["viewers"]["smartflow.data.table-viewer@1"]["future"]=="retain");
        QVERIFY(reopened.grab().save("n5h-data-reopened.png"));
    }

    void invalidParametersAndMissingPackageDoNotRunPartialGraphs()
    {
        WorkspaceWindow window(dataConfiguration());
        QTRY_VERIFY(window.execution().result().has_value());
        auto file=window.project().retained();
        for(auto& item : file["project"]["graphs"][0]["nodes"])
            if(item["typeId"]=="filter") item["parameters"]["minimum"]=-1;
        window.project().commands().replace(file,"graph");
        window.execution().run();
        QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(!window.execution().result()->succeeded());
        QVERIFY(window.execution().result()->steps.empty());
        QVERIFY(window.project().retained()["project"]==file["project"]);
        auto config=dataConfiguration();
        auto valid=project::DocumentSession::empty("data","graph",config.delegates,config.nodes);
        valid.createNode("source","smartflow.data.sample@1");
        auto retained=valid.document();
        retained["project"]["graphs"][0]["nodes"][0]["opaque"]={{"future",42}};
        project::DocumentSession absent(retained,"graph",std::make_shared<tp_pipeline::StepDelegateMap>(),{});
        QVERIFY(!absent.diagnostics().empty());
        QVERIFY(absent.document()==retained);
        QVERIFY_EXCEPTION_THROWN(absent.executableGraph(),project::FileError);
    }
};
QTEST_MAIN(DataTests)
#include "DataTests.moc"
