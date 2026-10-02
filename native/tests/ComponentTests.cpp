#include "project/GraphComponent.h"
#include "project/DocumentHistory.h"
#include "pipeline/PipelineExecution.h"
#include <DataExtension.h>
#include <QTemporaryDir>
#include <QtTest/QtTest>

using namespace smartflow;
using namespace smartflow::project;
namespace {
Document definition()
{
    return {{"format","smartflow.graph-component"},{"schemaVersion",1},{"id","filtered-summary"},
        {"version",1},{"title","Filtered summary"},
        {"graph",{{"id","body"},{"nodes",{
            {{"id","filter"},{"packageId","smartflow.data"},{"typeId","filter"},{"version",1},
             {"parameters",{{"minimum",20}}},{"futureNode",{{"value",uint64_t(18446744073709551615ULL)}}}},
            {{"id","summary"},{"packageId","smartflow.data"},{"typeId","summary"},{"version",1},{"parameters",Document::object()}}}},
            {"connections",{{{"id","edge"},{"source",{{"nodeId","filter"},{"portId","out"},{"future",true}}},
                {"target",{{"nodeId","summary"},{"portId","in"}}},{"futureEdge",{1,2,3}}}}},
            {"futureGraph","preserve"}}},
        {"inputs",{{{"id","table"},{"target",{{"nodeId","filter"},{"portId","in"}}}}}},
        {"outputs",{{{"id","summary"},{"source",{{"nodeId","summary"},{"portId","out"}}}}}},
        {"controls",{{{"id","minimum"},{"target",{{"nodeId","filter"},{"parameter","minimum"}}}}}},
        {"futureDefinition",{{"opaque",true}}}};
}
Document source(const WorkspaceConfiguration& config)
{
    auto session=DocumentSession::empty("project","graph",config.delegates,config.nodes);
    session.createNode("sample","smartflow.data.sample@1");
    auto result=session.document();
    result["project"]["graphs"].push_back({{"id","other"},{"nodes",Document::array()},{"connections",Document::array()},{"future",true}});
    result["workspace"]={{"opaque",{{"camera",{1,2,3}}}}};
    result["futureEnvelope"]="keep";
    return result;
}
Document input()
{
    return {{"table",{{"nodeId","sample"},{"portId","out"}}}};
}
double total(const DocumentSession& session, const WorkspaceConfiguration& config, const ComponentBindings& bindings)
{
    PipelineExecution executor;
    auto handle=executor.submit(session.executableGraph(),config.delegates,config.factory);
    if(handle.result.wait_for(std::chrono::seconds(10))!=std::future_status::ready) throw FileError("Component test execution timeout");
    const auto result=handle.result.get();
    if(!result.succeeded()) throw FileError("Component graph execution failed");
    const auto id=bindings.outputs["summary"]["nodeId"].get<std::string>();
    const auto& output=result.steps.at(id).output;
    return dynamic_cast<const data::TableMember*>(output->members().front().get())->rows[1].value;
}
}

class ComponentTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void extractionCatalogUndoAndReopen()
    {
        const auto config=data::dataConfiguration();
        auto original=source(config);
        auto& graph=original["project"]["graphs"][0];
        const auto model=definition();
        for(const auto& node : model["graph"]["nodes"]) graph["nodes"].push_back(node);
        graph["connections"]=model["graph"]["connections"];
        graph["connections"].push_back({{"id","incoming"},{"source",{{"nodeId","sample"},{"portId","out"}}},
            {"target",{{"nodeId","filter"},{"portId","in"}}}});
        graph["futureGraph"]={{"large",uint64_t(18446744073709551615ULL)}};
        DocumentHistory history(original,"graph",config.delegates,config.nodes);
        const auto component=history.extractComponent({"filter","summary"},"extracted","Extracted summary",
            model["inputs"],model["outputs"],model["controls"]);
        QCOMPARE(history.undoStack().count(),1);
        QVERIFY(history.session().selectedGraph()==graph);
        QVERIFY(component.definition()["graph"]["nodes"]==model["graph"]["nodes"]);
        QVERIFY(component.definition()["graph"]["connections"]==model["graph"]["connections"]);
        QVERIFY(component.definition()["graph"]["futureGraph"]==graph["futureGraph"]);
        const auto cataloged=history.session().document();
        history.catalogComponent(component);
        QCOMPARE(history.undoStack().count(),1); // Identical definitions are a no-op.
        history.undoStack().undo();
        QVERIFY(history.session().document()==original);
        QVERIFY_EXCEPTION_THROWN(history.extractComponent({"filter","summary"},"bad","Bad",
            Document::array(),model["outputs"],model["controls"]),FileError);
        QVERIFY(history.undoStack().canRedo());
        history.setWorkspaceField("current",{{"zoom",3}});
        history.undoStack().redo();
        QVERIFY(history.session().document()["project"]==cataloged["project"]);
        QVERIFY(history.session().document()["workspace"]["current"]["zoom"]==3);
        QTemporaryDir directory;
        const auto path=directory.filePath("extracted.smartflow");
        write(path,history.session().document());
        DocumentSession reopened(read(path),"graph",config.delegates,config.nodes);
        const GraphComponent saved(reopened.document()["project"]["components"][0]);
        const auto instance=reopened.instantiateComponent(saved,"copy",{{"minimum",30}},input());
        QCOMPARE(total(reopened,config,instance),79.0);
    }

    void extractionRejectsIncompleteBoundariesAndCatalogConflicts()
    {
        const auto config=data::dataConfiguration();
        auto original=source(config);
        auto& graph=original["project"]["graphs"][0];
        const auto model=definition();
        for(const auto& node : model["graph"]["nodes"]) graph["nodes"].push_back(node);
        graph["connections"]=model["graph"]["connections"];
        auto extract=[&](std::vector<std::string> ids, Document outputs) {
            return GraphComponent::extract(original,"graph",ids,"filter-only","Filter",
                model["inputs"],outputs,model["controls"]);
        };
        QVERIFY_EXCEPTION_THROWN(extract({"filter"},Document::array()),FileError);
        const Document outputs=Document::array({{{"id","rows"},{"source",{{"nodeId","filter"},{"portId","out"}}}}});
        const auto component=extract({"filter"},outputs);
        QVERIFY_EXCEPTION_THROWN(extract({},outputs),FileError);
        QVERIFY_EXCEPTION_THROWN(extract({"filter","filter"},outputs),FileError);
        QVERIFY_EXCEPTION_THROWN(extract({"missing"},outputs),FileError);
        DocumentHistory history(original,"graph",config.delegates,config.nodes);
        const auto diagnostics=history.session().diagnostics();
        history.catalogComponent(component);
        const auto before=history.session().document();
        auto conflict=component.definition(); conflict["title"]="Conflict";
        QVERIFY_EXCEPTION_THROWN(history.catalogComponent(GraphComponent(conflict)),FileError);
        QVERIFY(history.session().document()==before);
        // Catalog storage preserves unavailable definitions without executing them.
        auto unknown=component.definition(); unknown["id"]="unknown";
        unknown["graph"]["nodes"][0]["packageId"]="future";
        history.catalogComponent(GraphComponent(unknown));
        QVERIFY(history.session().diagnostics()==diagnostics);
        QVERIFY(history.session().document()["project"]["components"][1]==unknown);
        auto malformed=original; malformed["project"]["components"]=Document::object();
        QVERIFY_EXCEPTION_THROWN(component.catalog(malformed),FileError);
        QVERIFY(original["project"].contains("components")==false);
    }

    void independentInstancesExposeControlsAndPersistOpaqueDefinition()
    {
        const auto config=data::dataConfiguration();
        const auto original=source(config);
        const GraphComponent component(definition());
        DocumentSession session(original,"graph",config.delegates,config.nodes);
        const auto first=session.instantiateComponent(component,"first",{{"minimum",30}},input());
        const auto second=session.instantiateComponent(component,"second",Document::object(),input());
        QVERIFY(first.outputs!=second.outputs);
        QCOMPARE(total(session,config,first),79.0);
        QCOMPARE(total(session,config,second),104.0);
        session.setParameter(first.controls["minimum"]["nodeId"],"minimum",45);
        QCOMPARE(total(session,config,first),48.0);
        QCOMPARE(total(session,config,second),104.0);
        QVERIFY(component.definition()==definition());
        const auto& saved=session.document();
        QCOMPARE(saved["project"]["components"].size(),size_t(1));
        QVERIFY(saved["project"]["components"][0]==definition());
        QVERIFY(saved["project"]["graphs"][1]==original["project"]["graphs"][1]);
        QVERIFY(saved["workspace"]==original["workspace"]);
        QVERIFY(saved["futureEnvelope"]==original["futureEnvelope"]);
        for(const auto& edge : saved["project"]["graphs"][0]["connections"])
            if(edge.contains("futureEdge")) {
                QVERIFY(edge["futureEdge"]==definition()["graph"]["connections"][0]["futureEdge"]);
                QVERIFY(edge["source"]["future"]==true);
            }
        const auto firstNode=first.controls["minimum"]["nodeId"];
        for(const auto& node : saved["project"]["graphs"][0]["nodes"])
            if(node["id"]==firstNode) QVERIFY(node["futureNode"]==definition()["graph"]["nodes"][0]["futureNode"]);
        QTemporaryDir directory;
        const auto path=directory.filePath("components.smartflow");
        write(path,saved);
        DocumentSession reopened(read(path),"graph",config.delegates,config.nodes);
        QCOMPARE(total(reopened,config,first),48.0);
        QCOMPARE(total(reopened,config,second),104.0);
        QVERIFY(reopened.document()==saved);
    }


    void atomicUndoRedoAndRejectedCommandPreserveRedoBranch()
    {
        const auto config=data::dataConfiguration();
        const auto original=source(config);
        const GraphComponent component(definition());
        DocumentHistory history(original,"graph",config.delegates,config.nodes);
        const auto bindings=history.instantiateComponent(component,"instance",{{"minimum",30}},input());
        const auto instantiated=history.session().document();
        QCOMPARE(history.undoStack().count(),1);
        QCOMPARE(total(history.session(),config,bindings),79.0);
        history.undoStack().undo();
        QVERIFY(history.session().document()==original);
        QVERIFY_EXCEPTION_THROWN(history.instantiateComponent(component,"invalid",{{"minimum",-1}},input()),FileError);
        QVERIFY(history.session().document()==original);
        QVERIFY(history.undoStack().canRedo());
        history.setWorkspaceField("new-view",{{"zoom",2}});
        history.undoStack().redo();
        QVERIFY(history.session().document()["project"]==instantiated["project"]);
        QVERIFY(history.session().document()["workspace"]["new-view"]["zoom"]==2);
        QCOMPARE(total(history.session(),config,bindings),79.0);
    }

    void invalidInterfaceAndCyclesAreRejected()
    {
        auto broken=definition();
        broken["inputs"][0]["target"]["nodeId"]="missing";
        QVERIFY_EXCEPTION_THROWN(GraphComponent{broken},FileError);
        broken=definition(); broken["controls"].push_back(broken["controls"][0]);
        QVERIFY_EXCEPTION_THROWN(GraphComponent{broken},FileError);
        broken=definition(); broken["outputs"][0]["source"]["nodeId"]="missing";
        QVERIFY_EXCEPTION_THROWN(GraphComponent{broken},FileError);
        broken=definition(); broken["controls"][0]["target"]["parameter"]="absent";
        QVERIFY_EXCEPTION_THROWN(GraphComponent{broken},FileError);
        broken=definition(); broken["inputs"][0]["target"]={{"nodeId","summary"},{"portId","in"}};
        QVERIFY_EXCEPTION_THROWN(GraphComponent{broken},FileError);
        broken=definition(); broken["graph"]["connections"].push_back({{"id","cycle"},
            {"source",{{"nodeId","summary"},{"portId","out"}}},{"target",{{"nodeId","filter"},{"portId","in"}}}});
        QVERIFY_EXCEPTION_THROWN(GraphComponent{broken},FileError);
        broken=definition(); broken["schemaVersion"]=2;
        QVERIFY_EXCEPTION_THROWN(GraphComponent{broken},FileError);
    }

    void bindingFailuresAndIdentityConflictsLeaveDocumentUnchanged()
    {
        const auto config=data::dataConfiguration();
        const GraphComponent component(definition());
        DocumentSession session(source(config),"graph",config.delegates,config.nodes);
        auto before=session.document();
        QVERIFY_EXCEPTION_THROWN(session.instantiateComponent(component,"one",Document::object(),Document::object()),FileError);
        QVERIFY_EXCEPTION_THROWN(session.instantiateComponent(component,"one",{{"private",1}},input()),FileError);
        QVERIFY_EXCEPTION_THROWN(session.instantiateComponent(component,"one",{{"minimum","bad"}},input()),FileError);
        auto extraInput=input(); extraInput["extra"]={{"nodeId","sample"},{"portId","out"}};
        QVERIFY_EXCEPTION_THROWN(session.instantiateComponent(component,"one",Document::object(),extraInput),FileError);
        QVERIFY_EXCEPTION_THROWN(session.instantiateComponent(component,"one",Document::object(),{{"table",{{"nodeId","missing"},{"portId","out"}}}}),FileError);
        QVERIFY_EXCEPTION_THROWN(session.instantiateComponent(component,"one",Document::object(),{{"table",{{"nodeId","sample"},{"portId","bad"}}}}),FileError);
        QVERIFY(session.document()==before);
        auto broken=definition(); broken["outputs"][0]["source"]["portId"]="bad";
        QVERIFY_EXCEPTION_THROWN(session.instantiateComponent(GraphComponent(broken),"one",Document::object(),input()),FileError);
        QVERIFY(session.document()==before);
        session.instantiateComponent(component,"one",Document::object(),input());
        before=session.document();
        QVERIFY_EXCEPTION_THROWN(session.instantiateComponent(component,"one",Document::object(),input()),FileError);
        broken=definition(); broken["title"]="Changed without version";
        QVERIFY_EXCEPTION_THROWN(session.instantiateComponent(GraphComponent(broken),"two",Document::object(),input()),FileError);
        QVERIFY(session.document()==before);
    }

    void unknownBodiesStayRetainedAndUnrelatedUnavailableNodesRemain()
    {
        const auto config=data::dataConfiguration();
        auto original=source(config);
        original["project"]["graphs"][0]["nodes"].push_back({{"id","unknown"},{"packageId","future"},{"typeId","opaque"},
            {"version",7},{"parameters",{{"payload",{1,2,3}}}}});
        DocumentSession session(original,"graph",config.delegates,config.nodes);
        const auto bindings=session.instantiateComponent(GraphComponent(definition()),"valid",Document::object(),input());
        QVERIFY(!session.diagnostics().empty());
        QVERIFY_EXCEPTION_THROWN(session.executableGraph(),FileError);
        QVERIFY(session.document()["project"]["graphs"][0]["nodes"][1]==original["project"]["graphs"][0]["nodes"][1]);
        auto unknown=definition(); unknown["graph"]["nodes"][0]["packageId"]="absent";
        original["project"]["components"]={unknown};
        const GraphComponent retained(unknown);
        QVERIFY(retained.definition()==unknown);
        QVERIFY(parse(serialize(original))==original);
        const auto before=session.document();
        QVERIFY_EXCEPTION_THROWN(session.instantiateComponent(retained,"unsupported",Document::object(),input()),FileError);
        QVERIFY(session.document()==before);
    }
};
QTEST_GUILESS_MAIN(ComponentTests)
#include "ComponentTests.moc"
