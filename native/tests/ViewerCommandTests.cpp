#include "workspace/ViewerCommandAdapter.h"
#include "workspace/WorkspaceWindow.h"
#include <DataExtension.h>
#include <QTemporaryDir>
#include <QtTest/QtTest>
#include <limits>

using namespace smartflow;
class Viewer final : public OutputViewer {
public:
    void present(std::shared_ptr<const tp_data::Collection>) override {}
};
class ViewerCommandTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void atomicNumericCommandsAndLifetime()
    {
        Viewer viewer;
        auto document=std::make_unique<GraphProject>(numericDelegates(),numericPresentations());
        document->commands().createNode("first","smartflow.numeric.number@1");
        document->commands().createNode("second","smartflow.numeric.number@1");
        document->commands().undoStack().clear(); attachViewerCommands(viewer,*document);
        const auto before=document->retained(); const auto revision=document->revision();
        QVERIFY(viewer.commands.parameter("first","value"));
        ViewerEditRequest edit{revision,"Edit numbers",{{"first","value",20},{"second","value",30}}};
        QVERIFY(viewer.commands.apply(edit).isEmpty()); QCOMPARE(document->commands().undoStack().count(),1);
        QCOMPARE(document->selectedGraph()["nodes"][0]["parameters"]["value"].get<double>(),20.0);
        QVERIFY(!viewer.commands.apply(edit).isEmpty());
        document->commands().undoStack().undo(); QVERIFY(document->retained()==before);
        const auto checkRejected=[&](std::vector<ViewerParameterEdit> parameters) {
            const auto state=document->retained(); const auto count=document->commands().undoStack().count();
            QVERIFY(!viewer.commands.apply({document->revision(),"Invalid",parameters}).isEmpty());
            QVERIFY(document->retained()==state); QCOMPARE(document->commands().undoStack().count(),count);
            QVERIFY(document->commands().undoStack().canRedo());
        };
        checkRejected({{"first","value",5},{"absent","value",6}});
        checkRejected({{"first","value",5},{"first","value",6}});
        checkRejected({{"first","value",std::numeric_limits<double>::quiet_NaN()}});
        checkRejected({{"first","value",1e100}});
        document.reset(); QVERIFY(!viewer.commands.parameter("first","value"));
        QVERIFY(!viewer.commands.apply(edit).isEmpty());
    }
    void dataComponentScopeAndOpaqueRetention()
    {
        WorkspaceWindow window(data::dataConfiguration()); window.openProject(SMARTFLOW_COMPONENT_EXAMPLE);
        window.execution().setLive(false);
        auto retained=window.project().retained(); retained["project"]["future"]={{"untouched",true}};
        window.loadDocument(retained);
        const auto& node=window.project().selectedGraph()["nodes"][1];
        const auto body=node["component"];
        const auto expanded=project::GraphComponent(body).expandBody("summary-tool",node["parameters"]);
        const auto control=expanded.bindings.controls["minimum"];
        const auto compiled=QString::fromStdString(control["nodeId"].get<std::string>());
        const auto name=QString::fromStdString(control["parameter"].get<std::string>());
        QString scope="summary-tool"; Viewer viewer; attachViewerCommands(viewer,window.project(),[&] { return scope; });
        QVERIFY(viewer.commands.parameter(compiled,name));
        QVERIFY(viewer.commands.apply({window.project().revision(),"Filter",{{compiled,name,20}}}).isEmpty());
        QCOMPARE(window.project().step("summary-tool")->parameterValue<double>("minimum"),20.0);
        QVERIFY(window.project().selectedGraph()["nodes"][1]["component"]==body);
        QVERIFY(window.project().retained()["project"]["future"]["untouched"].get<bool>());
        scope="another-tool"; QVERIFY(!viewer.commands.parameter(compiled,name));
        QVERIFY(!viewer.commands.apply({window.project().revision(),"No",{{compiled,name,30}}}).isEmpty());
        scope.clear(); QVERIFY(!viewer.commands.parameter(compiled,name));
        QTemporaryDir folder; window.saveProject(folder.filePath("data.smartflow"));
        WorkspaceWindow reopened(data::dataConfiguration()); reopened.openProject(folder.filePath("data.smartflow"));
        QCOMPARE(reopened.project().step("summary-tool")->parameterValue<double>("minimum"),20.0);
        QVERIFY(reopened.project().retained()["project"]["future"]["untouched"].get<bool>());
    }
};
QTEST_MAIN(ViewerCommandTests)
#include "ViewerCommandTests.moc"
