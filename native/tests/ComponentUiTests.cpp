#include "workspace/WorkspaceWindow.h"
#include "workspace/ComponentDialogs.h"
#include <DataExtension.h>
#include <tp_data/Collection.h>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFontDatabase>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTemporaryDir>
#include <QTimer>
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
    void initTestCase()
    {
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
