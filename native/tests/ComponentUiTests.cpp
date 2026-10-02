#include "workspace/WorkspaceWindow.h"
#include "workspace/ComponentDialogs.h"
#include "project/ComponentFile.h"
#include <DataExtension.h>
#include <tp_data/Collection.h>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFontDatabase>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTimer>
#include <QTableWidget>
#include <QPlainTextEdit>
#include <set>
#include <QtNodes/internal/NodeGraphicsObject.hpp>
#include <QtTest/QtTest>

using namespace smartflow;
using project::Document;
namespace {
std::string nodeId(WorkspaceWindow& window, const char* type)
{
    for(const auto& node : window.project().selectedGraph()["nodes"])
        if(node["typeId"]==type) return node["id"].get<std::string>();
    return {};
}
double total(WorkspaceWindow& window, const std::string& summary)
{
    for(const auto& node : window.project().selectedGraph()["nodes"]) {
        const auto id=node["id"].get<std::string>();
        if(node["typeId"]=="summary" && id==summary) {
            const auto& output=window.execution().result()->steps.at(id).output;
            return dynamic_cast<const data::TableMember*>(output->members().front().get())->rows[1].value;
        }
    }
    return -1;
}
}

class ComponentUiTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void updateDialogReviewsAndPreservesPinnedAliases()
    {
        WorkspaceWindow window(data::dataConfiguration()); window.execution().setLive(false); window.show();
        window.openProject(SMARTFLOW_COMPONENT_EXAMPLE);
        const auto before=window.project().retained();
        std::string instance;
        for(const auto& node : window.project().selectedGraph()["nodes"]) if(project::GraphComponent::isInstance(node)) instance=node["id"];
        auto replacement=before["project"]["components"][0]; replacement["id"]="updated-tool"; replacement["title"]="Updated tool";
        replacement["graph"]["nodes"].push_back({{"id","extra"},{"packageId","smartflow.data"},{"typeId","filter"},{"version",1},{"parameters",{{"minimum",40}}}});
        replacement["graph"]["connections"][0]["target"]["nodeId"]="extra";
        replacement["graph"]["connections"].push_back({{"id","extra-edge"},{"source",{{"nodeId","extra"},{"portId","out"}}},{"target",{{"nodeId","summary"},{"portId","in"}}}});
        window.project().commands().catalogComponent(project::GraphComponent(replacement));
        for(const auto id : window.canvas().allNodeIds()) if(window.canvas().projectId(id).toString()==instance) {
            window.scene().clearSelection(); window.scene().nodeGraphicsObject(id)->setSelected(true); window.selectNode(id);
        }
        QTest::qWait(40); // Flush the selected-node workspace capture before opening the modal.
        const auto unchanged=window.project().retained();
        QTimer::singleShot(0,&window,[&] {
            auto* dialog=dynamic_cast<ComponentUpdateDialog*>(QApplication::activeModalWidget()); QVERIFY(dialog);
            auto* choices=dialog->findChild<QComboBox*>("componentUpdateCatalog"); choices->setCurrentIndex(1);
            QVERIFY(dialog->findChild<QPlainTextEdit*>("componentUpdateComparison")->toPlainText().contains("minimum = 40"));
            QVERIFY(dialog->findChild<QPushButton*>("applyComponentUpdate")->isEnabled());
            QVERIFY(dialog->grab().save("component-update-smoke.png"));
            dialog->accept(); QCOMPARE(dialog->result(),int(QDialog::Accepted));
        });
        window.findChild<QAction*>("updateComponentInstance")->trigger();
        QCOMPARE(QString::fromStdString(window.project().retained()["workspace"].dump()),QString::fromStdString(unchanged["workspace"].dump()));
        window.execution().run(); QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(window.execution().result()->succeeded());
        QCOMPARE(window.findChild<QTableWidget*>("outputTable")->item(1,1)->text(),QString("48"));
        window.scene().undoStack().undo(); QVERIFY(window.project().retained()["project"]==unchanged["project"]);
        window.scene().undoStack().redo();
        ComponentUpdateDialog cancelled(window.project(),instance); cancelled.reject();
        const auto current=window.project().retained();
        ComponentUpdateDialog stale(window.project(),instance);
        stale.findChild<QComboBox*>("componentUpdateCatalog")->setCurrentIndex(0);
        window.project().commands().setParameter(instance,"minimum",35); stale.accept();
        QCOMPARE(stale.result(),int(QDialog::Rejected));
        QVERIFY(!stale.findChild<QLabel*>("componentUpdateError")->text().isEmpty());
        window.scene().undoStack().undo(); QVERIFY(window.project().retained()["project"]==current["project"]);
    }

    void editedCopyDraftInterfaceUndoAndReopen()
    {
        WorkspaceWindow window(data::dataConfiguration()); window.execution().setLive(false); window.show();
        window.openProject(SMARTFLOW_COMPONENT_EXAMPLE);
        auto initial=window.project().retained();
        auto definition=initial["project"]["components"][0];
        definition["futureDefinition"]={{"integer",uint64_t(18446744073709551615ULL)}};
        definition["controls"][0]["futureInterface"]=true;
        initial["project"]["components"][0]=definition;
        window.project().commands().replace(initial,"graph");
        ComponentEditDialog editor(window.project(),window.configuration(),project::GraphComponent(definition),&window);
        editor.show();
        const auto filter=definition["controls"][0]["target"]["nodeId"].get<std::string>();
        editor.workspace().project().commands().setParameter(filter,"minimum",40);
        QVERIFY(window.project().retained()["project"]==initial["project"]);
        editor.workspace().scene().undoStack().undo();
        QVERIFY(editor.workspace().project().selectedGraph()==definition["graph"]);
        editor.workspace().scene().undoStack().redo();
        QTest::qWait(40);
        QVERIFY(editor.grab().save("component-editor-smoke.png"));
        QTimer::singleShot(0,&editor,[&] {
            auto* author=dynamic_cast<ComponentAuthorDialog*>(QApplication::activeModalWidget()); QVERIFY(author);
            QCOMPARE(author->findChild<QLineEdit*>("controlsName0")->text(),QString("minimum"));
            author->findChild<QLineEdit*>("componentTitle")->setText("Edited filter");
            author->accept(); QCOMPARE(author->result(),int(QDialog::Accepted));
        });
        editor.findChild<QPushButton*>("saveComponentCopy")->click();
        QCOMPARE(editor.result(),int(QDialog::Accepted));
        const auto saved=window.project().retained();
        const auto copy=saved["project"]["components"][1];
        QVERIFY(copy["id"]!=definition["id"]); QVERIFY(copy["version"]==1);
        QVERIFY(copy["futureDefinition"]==definition["futureDefinition"]);
        QVERIFY(copy["controls"][0]["futureInterface"]==true);
        QVERIFY(copy["graph"]["nodes"][0]["parameters"]["minimum"]==40);
        QVERIFY(saved["project"]["graphs"]==initial["project"]["graphs"]);
        window.scene().undoStack().undo(); QVERIFY(window.project().retained()["project"]==initial["project"]);
        window.scene().undoStack().redo(); QVERIFY(window.project().retained()["project"]==saved["project"]);
        QTemporaryDir directory; const auto path=directory.filePath("edited.smartflow"); window.saveProject(path);
        WorkspaceWindow reopened(data::dataConfiguration()); reopened.execution().setLive(false); reopened.openProject(path);
        QVERIFY(reopened.project().retained()["project"]["components"][1]==copy);
        reopened.execution().run(); QTRY_VERIFY(reopened.execution().result().has_value());
        QVERIFY(reopened.execution().result()->succeeded());
        ComponentLibraryDialog library(reopened.project(),&reopened);
        library.findChild<QComboBox*>("componentCatalog")->setCurrentIndex(1);
        library.findChild<QComboBox*>("componentInput_table")->setCurrentIndex(1);
        library.accept(); QCOMPARE(library.result(),int(QDialog::Accepted));
        reopened.execution().run(); QTRY_VERIFY(reopened.execution().result().has_value());
        QVERIFY(reopened.execution().result()->succeeded());
        const auto id=reopened.project().selectedGraph()["nodes"].back()["id"].get<std::string>();
        const auto output=reopened.execution().result()->steps.at(id).output;
        QCOMPARE(dynamic_cast<const data::TableMember*>(output->members().back().get())->rows[1].value,48.0);
    }

    void editorCancelStaleAndInvalidDraftPreserveDestination()
    {
        WorkspaceWindow window(data::dataConfiguration()); window.execution().setLive(false);
        window.openProject(SMARTFLOW_COMPONENT_EXAMPLE);
        const auto initial=window.project().retained();
        const auto definition=initial["project"]["components"][0];
        ComponentEditDialog cancelled(window.project(),window.configuration(),project::GraphComponent(definition));
        cancelled.workspace().project().commands().removeNode(definition["graph"]["nodes"][0]["id"]);
        cancelled.reject(); QVERIFY(window.project().retained()==initial);
        ComponentEditDialog stale(window.project(),window.configuration(),project::GraphComponent(definition));
        auto copy=definition; copy["id"]="new-copy";
        window.project().commands().catalogComponent(project::GraphComponent(copy));
        const auto changed=window.project().retained();
        copy["id"]="another-copy";
        QVERIFY_EXCEPTION_THROWN(stale.saveCopy(project::GraphComponent(copy)),project::FileError);
        QVERIFY(window.project().retained()==changed);
        ComponentEditDialog invalid(window.project(),window.configuration(),project::GraphComponent(definition));
        copy["outputs"][0]["source"]["portId"]="missing-port";
        QVERIFY_EXCEPTION_THROWN(invalid.saveCopy(project::GraphComponent(copy)),project::FileError);
        QVERIFY(window.project().retained()==changed);
        copy=definition; copy["id"]="missing-boundary"; copy["inputs"]=Document::array();
        QVERIFY_EXCEPTION_THROWN(invalid.saveCopy(project::GraphComponent(copy)),project::FileError);
        QVERIFY(window.project().retained()==changed);
    }

    void libraryRemovalConfirmationUndoAndSnapshotReopen()
    {
        WorkspaceWindow window(data::dataConfiguration()); window.execution().setLive(false); window.show();
        window.openProject(SMARTFLOW_COMPONENT_EXAMPLE);
        auto source=window.project().retained();
        source["project"]["components"].push_back({{"future",uint64_t(18446744073709551615ULL)}});
        window.project().commands().replace(source,"graph");
        ComponentLibraryDialog library(window.project()); library.show();
        auto* choices=library.findChild<QComboBox*>("componentCatalog"); choices->setCurrentIndex(1);
        auto* remove=library.findChild<QPushButton*>("removeCatalogComponent");
        QVERIFY(remove->isEnabled());
        QVERIFY(!library.findChild<QPushButton*>("insertComponentConfirm")->isEnabled());
        auto answer=[&](QMessageBox::StandardButton response) {
            QTimer::singleShot(0,&library,[response] {
                auto* dialog=qobject_cast<QMessageBox*>(QApplication::activeModalWidget()); QVERIFY(dialog);
                QCOMPARE(dialog->defaultButton(),dialog->button(QMessageBox::No));
                dialog->button(response)->click();
            });
        };
        answer(QMessageBox::No); remove->click();
        QVERIFY(window.project().retained()==source); QCOMPARE(choices->count(),2);
        answer(QMessageBox::Yes); remove->click(); QCOMPARE(choices->count(),1);
        QVERIFY(window.project().retained()["project"]["graphs"]==source["project"]["graphs"]);
        QTest::qWait(30); // Let replacement binding widgets complete their layout.
        QVERIFY(library.findChild<QComboBox*>("componentInput_table")->isVisible());
        QVERIFY(library.grab().save("component-library-removal-smoke.png"));
        answer(QMessageBox::Yes); remove->click(); QCOMPARE(choices->count(),0);
        QVERIFY(!remove->isEnabled());
        QVERIFY(window.project().retained()["project"]["components"].empty());
        window.execution().run(); QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(window.execution().result()->succeeded());
        QCOMPARE(window.findChild<QTableWidget*>("outputTable")->item(1,1)->text(),QString("79"));
        window.scene().undoStack().undo(); window.scene().undoStack().undo();
        QVERIFY(window.project().retained()["project"]==source["project"]);
        window.scene().undoStack().redo(); window.scene().undoStack().redo();
        const auto removed=window.project().retained();
        QVERIFY_EXCEPTION_THROWN(library.removeSelected(),project::FileError); // Stale revision.
        QVERIFY(window.project().retained()==removed);
        QTemporaryDir directory; const auto path=directory.filePath("removed-library.smartflow"); window.saveProject(path);
        WorkspaceWindow reopened(data::dataConfiguration()); reopened.execution().setLive(false); reopened.show(); reopened.openProject(path);
        reopened.execution().run(); QTRY_VERIFY(reopened.execution().result().has_value());
        QCOMPARE(reopened.findChild<QTableWidget*>("outputTable")->item(1,1)->text(),QString("79"));
        QVERIFY(reopened.project().retained()["project"]["graphs"]==source["project"]["graphs"]);
        ComponentLibraryDialog empty(reopened.project());
        QVERIFY(!empty.findChild<QPushButton*>("removeCatalogComponent")->isEnabled());
        QVERIFY_EXCEPTION_THROWN(empty.removeSelected(),project::FileError);
    }

    void libraryFileExchangeUndoConflictsAndSnapshotIsolation()
    {
        WorkspaceWindow source(data::dataConfiguration()); source.execution().setLive(false); source.show();
        source.openProject(SMARTFLOW_COMPONENT_EXAMPLE);
        QTemporaryDir directory; const auto path=directory.filePath("tool.smartflow-component");
        ComponentLibraryDialog library(source.project());
        const auto before=source.project().retained();
        const auto historyIndex=source.scene().undoStack().index();
        library.exportFile(path);
        QVERIFY(source.project().retained()==before);
        QCOMPARE(source.scene().undoStack().index(),historyIndex);
        QVERIFY(!source.projectDirty());

        WorkspaceWindow destination(data::dataConfiguration()); destination.execution().setLive(false); destination.show();
        QTest::qWait(30);
        const auto empty=destination.project().retained();
        ComponentLibraryDialog imported(destination.project());
        QVERIFY(imported.findChild<QPushButton*>("importComponentFile")->isEnabled());
        QVERIFY(!imported.findChild<QPushButton*>("exportComponentFile")->isEnabled());
        imported.importFile(path);
        QCOMPARE(imported.findChild<QComboBox*>("componentCatalog")->count(),1);
        QVERIFY(destination.projectDirty());
        QVERIFY(destination.project().selectedGraph()==empty["project"]["graphs"][0]);
        const auto count=destination.scene().undoStack().count();
        imported.importFile(path); QCOMPARE(destination.scene().undoStack().count(),count);
        imported.findChild<QComboBox*>("componentInput_table")->setCurrentIndex(1);
        imported.accept(); QCOMPARE(imported.result(),int(QDialog::Accepted));
        const auto retained=destination.project().retained();
        const auto& node=retained["project"]["graphs"][0]["nodes"].back();
        QVERIFY(node["component"]==before["project"]["components"][0]);
        destination.execution().run(); QTRY_VERIFY(destination.execution().result().has_value());
        QVERIFY(destination.execution().result()->succeeded());
        destination.scene().undoStack().undo(); // Instance insertion.
        destination.scene().undoStack().undo(); // Catalog import.
        QVERIFY(destination.project().retained()["project"]==empty["project"]);
        QVERIFY(destination.scene().undoStack().canRedo());
        auto conflict=project::readComponent(path).definition(); conflict["title"]="Changed same identity";
        project::writeComponent(path,project::GraphComponent(conflict));
        destination.scene().undoStack().redo(); // Restore original catalog.
        ComponentLibraryDialog conflicting(destination.project());
        const auto preserved=destination.project().retained();
        QVERIFY_EXCEPTION_THROWN(conflicting.importFile(path),project::FileError);
        QVERIFY(destination.project().retained()==preserved);
        QVERIFY(destination.scene().undoStack().canRedo());
        destination.scene().undoStack().redo();
        QVERIFY(destination.project().retained()["project"]==retained["project"]);
        QVERIFY_EXCEPTION_THROWN(conflicting.exportFile(directory.filePath("stale.smartflow-component")),project::FileError);
        const auto projectPath=directory.filePath("destination.smartflow"); destination.saveProject(projectPath);
        WorkspaceWindow reopened(data::dataConfiguration()); reopened.execution().setLive(false); reopened.show(); reopened.openProject(projectPath);
        QVERIFY(reopened.project().retained()["project"]==retained["project"]);
        QVERIFY(library.grab().save("component-file-library-smoke.png"));
    }

    void libraryButtonsUseFileDialogsAndRetainUnavailableDefinitions()
    {
        WorkspaceWindow window(data::dataConfiguration()); window.execution().setLive(false); window.show();
        QTemporaryDir directory; const auto path=directory.filePath("unavailable.smartflow-component");
        auto definition=project::read(SMARTFLOW_COMPONENT_EXAMPLE)["project"]["components"][0];
        definition["graph"]["nodes"][0]["packageId"]="unavailable.package";
        project::writeComponent(path,project::GraphComponent(definition));
        ComponentLibraryDialog library(window.project()); library.show();
        auto choose=[&](const QString& file) {
            QTimer::singleShot(0,&library,[file] {
                auto* dialog=qobject_cast<QFileDialog*>(QApplication::activeModalWidget());
                QVERIFY(dialog); dialog->selectFile(file);
                QMetaObject::invokeMethod(dialog,"accept",Qt::DirectConnection);
            });
        };
        choose(path); library.findChild<QPushButton*>("importComponentFile")->click();
        QVERIFY(window.project().retained()["project"]["components"][0]==definition);
        QVERIFY(!library.findChild<QPushButton*>("insertComponentConfirm")->isEnabled());
        QVERIFY(library.findChild<QPushButton*>("exportComponentFile")->isEnabled());
        const auto copy=directory.filePath("exported.smartflow-component");
        const auto retained=window.project().retained();
        choose(copy); library.findChild<QPushButton*>("exportComponentFile")->click();
        QVERIFY(project::readComponent(copy).definition()==definition);
        QVERIFY(window.project().retained()==retained);
        QTimer::singleShot(0,&library,[] {
            auto* dialog=qobject_cast<QFileDialog*>(QApplication::activeModalWidget()); QVERIFY(dialog); dialog->reject();
        });
        library.findChild<QPushButton*>("importComponentFile")->click();
        QVERIFY(window.project().retained()==retained);
        QVERIFY_EXCEPTION_THROWN(library.importFile(directory.filePath("missing")),project::FileError);
        QVERIFY(window.project().retained()==retained);
        const auto invalid=directory.filePath("invalid.smartflow-component");
        QFile file(invalid); QVERIFY(file.open(QIODevice::WriteOnly)); file.write("{}"); file.close();
        choose(invalid); library.findChild<QPushButton*>("importComponentFile")->click();
        QVERIFY(!library.findChild<QLabel*>("componentError")->text().isEmpty());
        QVERIFY(window.project().retained()==retained);
    }

    void shippedCollapsedExampleRestoresSelectedOutputAndControl()
    {
        WorkspaceWindow window(data::dataConfiguration()); window.execution().setLive(false); window.show();
        window.openProject(SMARTFLOW_COMPONENT_EXAMPLE);
        QCOMPARE(window.canvas().allNodeIds().size(),size_t(2));
        QCOMPARE(window.findChild<QComboBox*>("outputPort")->currentText(),QString("summary"));
        window.execution().run(); QTRY_VERIFY(window.execution().result().has_value());
        QCOMPARE(window.findChild<QTableWidget*>("outputTable")->item(1,1)->text(),QString("79"));
        QVERIFY(window.grab().save("component-example-smoke.png"));
        const auto snapshot=window.project().retained()["project"]["graphs"][0]["nodes"][1]["component"];
        window.findChild<QDoubleSpinBox*>("parameter_minimum")->setValue(45);
        window.findChild<QPushButton*>("applyParameter")->click();
        window.execution().run(); QTRY_VERIFY(window.execution().result().has_value());
        QCOMPARE(window.findChild<QTableWidget*>("outputTable")->item(1,1)->text(),QString("48"));
        QVERIFY(window.project().retained()["project"]["graphs"][0]["nodes"][1]["component"]==snapshot);
        QCOMPARE(window.execution().result()->steps.size(),size_t(2));
    }

    void collapsedMenuCanvasInspectorViewerAndReopen()
    {
        WorkspaceWindow window(data::dataConfiguration());
        window.execution().setLive(false); window.show(); QTest::qWait(30);
        ComponentAuthorDialog author(window.project(),{nodeId(window,"filter"),nodeId(window,"summary")});
        author.findChild<QLineEdit*>("componentTitle")->setText("Summary tool");
        author.findChild<QLineEdit*>("inputsName0")->setText("table");
        author.findChild<QLineEdit*>("outputsName1")->setText("summary");
        author.findChild<QCheckBox*>("outputsExpose0")->setChecked(true);
        author.findChild<QLineEdit*>("outputsName0")->setText("rows");
        author.findChild<QCheckBox*>("controlsExpose0")->setChecked(true);
        author.findChild<QLineEdit*>("controlsName0")->setText("minimum");
        author.accept(); QCOMPARE(author.result(),int(QDialog::Accepted));
        bool inserted=false;
        QTimer::singleShot(0,&window,[&] {
            auto* dialog=dynamic_cast<ComponentLibraryDialog*>(QApplication::activeModalWidget());
            if(!dialog) return;
            dialog->findChild<QComboBox*>("componentInput_table")->setCurrentIndex(1);
            dialog->findChild<QDoubleSpinBox*>("componentControl_minimum")->setValue(30);
            dialog->findChild<QPushButton*>("insertComponentConfirm")->click();
            inserted=dialog->result()==QDialog::Accepted;
            if(!inserted) dialog->reject();
        });
        window.findChild<QAction*>("insertComponent")->trigger(); QVERIFY(inserted);
        QCOMPARE(window.canvas().allNodeIds().size(),size_t(4));
        QCOMPARE(window.scene().selectedNodes().size(),size_t(1));
        const auto canvasId=window.scene().selectedNodes().front();
        const auto instance=window.canvas().projectId(canvasId).toString();
        QCOMPARE(window.canvas().nodeData(canvasId,QtNodes::NodeRole::Caption).toString(),QString("Summary tool"));
        QCOMPARE(window.canvas().nodeData(canvasId,QtNodes::NodeRole::InPortCount).toUInt(),1u);
        QCOMPARE(window.canvas().nodeData(canvasId,QtNodes::NodeRole::OutPortCount).toUInt(),2u);
        window.findChild<QComboBox*>("outputPort")->setCurrentText("summary");
        window.findChild<QAction*>("pinOutput")->trigger();
        window.execution().run(); QTRY_VERIFY(window.execution().result().has_value());
        QCOMPARE(window.execution().result()->steps.size(),size_t(4));
        QCOMPARE(window.findChild<QTableWidget*>("outputTable")->item(1,1)->text(),QString("79"));
        window.findChild<QComboBox*>("outputPort")->setCurrentText("rows");
        window.findChild<QAction*>("pinOutput")->trigger();
        QCOMPARE(window.findChild<QTableWidget*>("outputTable")->rowCount(),2);
        window.findChild<QComboBox*>("outputPort")->setCurrentText("summary");
        window.findChild<QAction*>("pinOutput")->trigger();
        window.findChild<QDoubleSpinBox*>("parameter_minimum")->setValue(45);
        window.findChild<QPushButton*>("applyParameter")->click();
        window.execution().run(); QTRY_VERIFY(window.execution().result().has_value());
        QCOMPARE(window.findChild<QTableWidget*>("outputTable")->item(1,1)->text(),QString("48"));
        window.scene().undoStack().undo();
        QCOMPARE(window.project().step(instance)->parameterValue<double>("minimum"),30.0);
        window.scene().undoStack().redo();
        QCOMPARE(window.project().step(instance)->parameterValue<double>("minimum"),45.0);
        window.execution().run(); QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(window.grab().save("component-collapsed-smoke.png"));
        QTemporaryDir directory; const auto path=directory.filePath("collapsed.smartflow"); window.saveProject(path);
        const auto saved=window.project().retained();
        WorkspaceWindow reopened(data::dataConfiguration()); reopened.execution().setLive(false); reopened.show(); reopened.openProject(path);
        QVERIFY(reopened.project().retained()==saved);
        QCOMPARE(reopened.canvas().allNodeIds().size(),size_t(4));
        reopened.execution().run(); QTRY_VERIFY(reopened.execution().result().has_value());
        QCOMPARE(reopened.findChild<QTableWidget*>("outputTable")->item(1,1)->text(),QString("48"));
        reopened.scene().deleteSelected(); QCOMPARE(reopened.canvas().allNodeIds().size(),size_t(3));
        reopened.scene().undoStack().undo();
        QVERIFY(reopened.project().retained()["project"]==saved["project"]);
        QCOMPARE(reopened.canvas().allNodeIds().size(),size_t(4));
    }

    void initTestCase()
    {
        QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
        const auto path=qEnvironmentVariable("SMARTFLOW_TEST_FONT");
        if(path.isEmpty()) return;
        const auto id=QFontDatabase::addApplicationFont(path);
        QVERIFY(id>=0);
        QApplication::setFont(QFont(QFontDatabase::applicationFontFamilies(id).first(),10));
    }
    void menuAuthoringLibraryUndoAndReopen()
    {
        WorkspaceWindow window(data::dataConfiguration());
        window.execution().setLive(false); window.show();
        QTest::qWait(30);
        window.scene().clearSelection();
        for(const auto id : window.canvas().allNodeIds()) {
            const auto* step=window.project().step(window.canvas().projectId(id));
            if(step->delegateName()!="smartflow.data.sample@1") window.scene().nodeGraphicsObject(id)->setSelected(true);
        }
        const auto originalGraph=window.project().selectedGraph();
        bool authored=false, rendered=false;
        QTimer::singleShot(0,&window,[&] {
            auto* dialog=dynamic_cast<ComponentAuthorDialog*>(QApplication::activeModalWidget());
            if(!dialog) return;
            dialog->findChild<QLineEdit*>("componentTitle")->setText("Reusable summary");
            dialog->findChild<QLineEdit*>("inputsName0")->setText("table");
            dialog->findChild<QCheckBox*>("outputsExpose0")->setChecked(true);
            dialog->findChild<QLineEdit*>("outputsName0")->setText("rows");
            dialog->findChild<QLineEdit*>("outputsName1")->setText("summary");
            dialog->findChild<QCheckBox*>("controlsExpose0")->setChecked(true);
            dialog->findChild<QLineEdit*>("controlsName0")->setText("minimum");
            rendered=dialog->grab().save("component-author-smoke.png");
            dialog->findChild<QPushButton*>("createComponentConfirm")->click();
            authored=dialog->result()==QDialog::Accepted;
            if(!authored) dialog->reject();
        });
        window.findChild<QAction*>("createComponent")->trigger();
        QVERIFY(authored); QVERIFY(rendered);
        QVERIFY(window.project().selectedGraph()==originalGraph);
        QCOMPARE(window.project().retained()["project"]["components"].size(),size_t(1));
        const auto cataloged=window.project().retained()["project"];
        window.scene().undoStack().undo();
        QVERIFY(!window.project().retained()["project"].contains("components"));
        window.scene().undoStack().redo();
        QVERIFY(window.project().retained()["project"]==cataloged);
        auto insert=[&](double minimum) {
            std::set<std::string> previous;
            for(const auto& node : window.project().selectedGraph()["nodes"]) previous.insert(node["id"]);
            bool inserted=false;
            QTimer::singleShot(0,&window,[&] {
                auto* dialog=dynamic_cast<ComponentLibraryDialog*>(QApplication::activeModalWidget());
                if(!dialog) return;
                dialog->findChild<QCheckBox*>("componentCollapsed")->setChecked(false);
                auto* input=dialog->findChild<QComboBox*>("componentInput_table");
                const auto sample=nodeId(window,"sample");
                for(int i=1;i<input->count();++i)
                    if(Document::parse(input->itemData(i).toString().toStdString())["nodeId"]==sample) input->setCurrentIndex(i);
                dialog->findChild<QDoubleSpinBox*>("componentControl_minimum")->setValue(minimum);
                if(minimum==30) dialog->grab().save("component-library-smoke.png");
                dialog->findChild<QPushButton*>("insertComponentConfirm")->click();
                inserted=dialog->result()==QDialog::Accepted;
                if(!inserted) dialog->reject();
            });
            window.findChild<QAction*>("insertComponent")->trigger();
            if(inserted) for(const auto& node : window.project().selectedGraph()["nodes"])
                if(node["typeId"]=="summary" && !previous.count(node["id"])) return node["id"].get<std::string>();
            return std::string();
        };
        const auto first=insert(30), second=insert(20);
        QVERIFY(!first.empty()); QVERIFY(!second.empty());
        QCOMPARE(window.canvas().allNodeIds().size(),size_t(7));
        QCOMPARE(window.scene().selectedNodes().size(),size_t(2));
        window.execution().run(); QTRY_VERIFY(window.execution().result().has_value());
        QVERIFY(window.execution().result()->succeeded());
        QCOMPARE(total(window,first),79.0);
        QCOMPARE(total(window,second),104.0);
        const auto expanded=window.project().retained()["project"];
        window.scene().undoStack().undo(); QCOMPARE(window.canvas().allNodeIds().size(),size_t(5));
        const auto undone=window.project().retained();
        ComponentLibraryDialog incomplete(window.project());
        incomplete.accept();
        QVERIFY(incomplete.result()!=QDialog::Accepted);
        QVERIFY(!incomplete.findChild<QLabel*>("componentError")->text().isEmpty());
        QVERIFY(window.scene().undoStack().canRedo());
        QVERIFY(window.project().retained()==undone);
        window.scene().undoStack().redo(); QVERIFY(window.project().retained()["project"]==expanded);
        // Insertion puts copies beside the graph instead of covering existing nodes.
        for(const auto first : window.canvas().allNodeIds())
            for(const auto second : window.canvas().allNodeIds()) if(first!=second)
                QVERIFY(!window.scene().nodeGraphicsObject(first)->sceneBoundingRect().intersects(
                    window.scene().nodeGraphicsObject(second)->sceneBoundingRect()));
        QTemporaryDir directory; const auto path=directory.filePath("components.smartflow");
        window.saveProject(path);
        WorkspaceWindow reopened(data::dataConfiguration()); reopened.execution().setLive(false); reopened.show();
        reopened.openProject(path);
        QVERIFY(reopened.project().retained()["project"]==expanded);
        QCOMPARE(reopened.canvas().allNodeIds().size(),size_t(7));
        ComponentLibraryDialog library(reopened.project());
        library.findChild<QCheckBox*>("componentCollapsed")->setChecked(false);
        QCOMPARE(library.findChild<QComboBox*>("componentCatalog")->currentText(),QString("Reusable summary"));
        library.findChild<QComboBox*>("componentInput_table")->setCurrentIndex(1);
        library.accept(); QCOMPARE(library.result(),int(QDialog::Accepted));
        QCOMPARE(reopened.canvas().allNodeIds().size(),size_t(9));
        reopened.scene().undoStack().undo();
        QVERIFY(reopened.project().retained()["project"]==expanded);
        reopened.execution().run(); QTRY_VERIFY(reopened.execution().result().has_value());
        QCOMPARE(total(reopened,first),79.0);
        QCOMPARE(total(reopened,second),104.0);
        QVERIFY(reopened.grab().save("component-workspace-smoke.png"));
    }
    void validationCancellationAndUnavailableLibraryPreserveContent()
    {
        WorkspaceWindow window(data::dataConfiguration()); window.execution().setLive(false);
        const auto filter=nodeId(window,"filter"), summary=nodeId(window,"summary");
        const auto before=window.project().retained();
        ComponentAuthorDialog canceled(window.project(),{filter,summary}); canceled.reject();
        QVERIFY(window.project().retained()==before);
        ComponentAuthorDialog author(window.project(),{filter,summary});
        author.findChild<QLineEdit*>("componentTitle")->clear(); author.accept();
        QVERIFY(author.result()!=QDialog::Accepted);
        QVERIFY(!author.findChild<QLabel*>("componentError")->text().isEmpty());
        QVERIFY(window.project().retained()==before);
        author.findChild<QLineEdit*>("componentTitle")->setText("Summary"); author.accept();
        QCOMPARE(author.result(),int(QDialog::Accepted));
        const auto saved=window.project().retained();
        ComponentLibraryDialog missingBinding(window.project()); missingBinding.accept();
        QVERIFY(missingBinding.result()!=QDialog::Accepted);
        QVERIFY(!missingBinding.findChild<QLabel*>("componentError")->text().isEmpty());
        QVERIFY(window.project().retained()==saved);
        auto unknown=saved;
        unknown["project"]["components"][0]["graph"]["nodes"][0]["packageId"]="unavailable";
        unknown["project"]["components"].push_back({{"future","opaque"}});
        window.project().commands().replace(unknown,"graph");
        ComponentLibraryDialog unavailable(window.project());
        QVERIFY(!unavailable.findChild<QPushButton*>("insertComponentConfirm")->isEnabled());
        unavailable.accept(); QVERIFY(window.project().retained()==unknown);
        unavailable.findChild<QComboBox*>("componentCatalog")->setCurrentIndex(1);
        QVERIFY(!unavailable.findChild<QPushButton*>("insertComponentConfirm")->isEnabled());
        QVERIFY(window.project().retained()==unknown);
        auto malformed=unknown; malformed["project"]["components"]=Document::object();
        window.project().commands().replace(malformed,"graph");
        ComponentLibraryDialog unsupported(window.project());
        QVERIFY(!unsupported.findChild<QPushButton*>("insertComponentConfirm")->isEnabled());
        QVERIFY(window.project().retained()==malformed);
    }
    void compatibleInputsAndStaleDialogs()
    {
        WorkspaceConfiguration config;
        config.delegates=numericDelegates(); config.nodes=numericPresentations();
        config.factory=std::make_shared<tp_data::CollectionFactory>();
        tp_data::createCollectionFactories(*config.factory);
        data::contribute(config); config.factory->finalize();
        config.preset={{"smartflow.numeric.number@1",{0,0},{}},
            {"smartflow.data.sample@1",{240,0},{}},{"smartflow.data.filter@1",{480,0},{}},
            {"smartflow.data.summary@1",{720,0},{}}};
        config.connections={{1,0,2,0},{2,0,3,0}};
        WorkspaceWindow window(config); window.execution().setLive(false);
        ComponentAuthorDialog author(window.project(),{nodeId(window,"filter"),nodeId(window,"summary")});
        author.findChild<QLineEdit*>("inputsName0")->setText("table"); author.accept();
        QCOMPARE(author.result(),int(QDialog::Accepted));
        ComponentLibraryDialog library(window.project());
        auto* input=library.findChild<QComboBox*>("componentInput_table");
        QCOMPARE(input->count(),4); // Placeholder and three table outputs; no Number output.
        const auto numeric=nodeId(window,"number");
        for(int i=1;i<input->count();++i)
            QVERIFY(Document::parse(input->itemData(i).toString().toStdString())["nodeId"]!=numeric);
        input->setCurrentIndex(1);
        window.project().commands().setParameter(numeric,"value",17);
        const auto changed=window.project().retained();
        library.accept();
        QVERIFY(library.result()!=QDialog::Accepted);
        QVERIFY(library.findChild<QLabel*>("componentError")->text().contains("graph changed"));
        QVERIFY(window.project().retained()==changed);
        window.project().commands().removeNode(nodeId(window,"sample"));
        window.project().commands().removeNode(nodeId(window,"filter"));
        window.project().commands().removeNode(nodeId(window,"summary"));
        ComponentLibraryDialog noSource(window.project());
        QVERIFY(!noSource.findChild<QPushButton*>("insertComponentConfirm")->isEnabled());
        QCOMPARE(noSource.findChild<QComboBox*>("componentInput_table")->count(),1);
    }
};
QTEST_MAIN(ComponentUiTests)
#include "ComponentUiTests.moc"
