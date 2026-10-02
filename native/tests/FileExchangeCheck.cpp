#include "workspace/WorkspaceWindow.h"
#include <DataExtension.h>
#include <SceneExtension.h>
#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QElapsedTimer>
#include <QThread>
#include <iostream>

// Test-only fresh-process adapter: exercise the actual native window load/save
// paths and executor, without adding test automation flags to the shipped app.
int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption(QCommandLineOption("domain","Workflow configuration","domain","data"));
    parser.addOption(QCommandLineOption("input","Project to open","path"));
    parser.addOption(QCommandLineOption("output","Project to save","path"));
    parser.addOption(QCommandLineOption("report","Execution report","path"));
    parser.addOption(QCommandLineOption("allow-failure","Retained unsupported content may block execution"));
    parser.process(app);
    try {
        smartflow::WorkspaceWindow window(parser.value("domain")=="scene" ?
            smartflow::scene3d::sceneConfiguration() : smartflow::data::dataConfiguration());
        window.execution().setLive(false);
        window.openProject(parser.value("input"));
        window.show();
        window.execution().run();
        QElapsedTimer timer; timer.start();
        while(window.execution().busy() && timer.elapsed()<15000) {
            app.processEvents(); QThread::msleep(5);
        }
        if(window.execution().busy()) throw std::runtime_error("Execution timed out");
        const auto& result=window.execution().result();
        if(!result) throw std::runtime_error("No execution result");
        if(!result->succeeded() && !parser.isSet("allow-failure")) throw std::runtime_error("Project did not execute successfully");
        smartflow::project::Document report={{"succeeded",result->succeeded()},{"outputs",smartflow::project::Document::object()}};
        for(const auto& [id,step] : result->steps) if(step.output)
            for(const auto& member : step.output->members()) {
                auto portName=member->name().toString();
                if(const auto* node=window.project().step(id))
                    for(const auto& mapping : node->outputMapping())
                        if(mapping.dataName==member->name()) portName=mapping.portName.toString();
                auto& output=report["outputs"][id.toString()][portName];
                if(const auto* table=dynamic_cast<const smartflow::data::TableMember*>(member.get())) {
                    output["rows"]=smartflow::project::Document::array();
                    for(const auto& row : table->rows) output["rows"].push_back({{"label",row.label},{"value",row.value}});
                }
                if(const auto* scene=dynamic_cast<const smartflow::scene3d::SceneMember*>(member.get())) {
                    output["objects"]=smartflow::project::Document::array();
                    for(const auto& object : scene->objects) {
                        const auto bounds=object.geometry.getMinMax();
                        output["objects"].push_back({{"minimum",{bounds.first.x,bounds.first.y,bounds.first.z}},
                            {"maximum",{bounds.second.x,bounds.second.y,bounds.second.z}}});
                    }
                }
            }
        window.saveProject(parser.value("output"));
        smartflow::project::jsonFile::write(parser.value("report"),report);
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<error.what()<<'\n'; return 1;
    }
}
