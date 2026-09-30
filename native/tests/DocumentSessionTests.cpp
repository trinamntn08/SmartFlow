#include "project/DocumentSession.h"
#include "workspace/GraphProject.h"
#include "pipeline/PipelineExecution.h"
#include <tp_data/members/NumberMember.h>
#include <QTemporaryDir>
#include <QtTest/QtTest>
#include <chrono>

using namespace smartflow;
using namespace smartflow::project;
namespace {
std::vector<NodePresentation> registrations()
{
    return {{"smartflow.numeric.number@1","Number","Numeric","smartflow.numeric","number",1},
            {"smartflow.numeric.add@1","Add","Numeric","smartflow.numeric","add",1}};
}
Document fixture()
{
    auto file=create("project");
    file["project"]["packages"]={{{"id","smartflow.numeric"},{"version","1.0.0"}}};
    file["project"]["graphs"]={{{"id","graph"},
        {"nodes", {{{"id","source"},{"packageId","smartflow.numeric"},{"typeId","number"},
                    {"version",1},{"parameters",{{"value",41}}},{"futureNode",{{"keep",true}}}},
                   {{"id","target"},{"packageId","smartflow.numeric"},{"typeId","add"},
                    {"version",1},{"parameters",{{"value",1}}}}}},
        {"connections", {{{"id","edge"},{"source",{{"nodeId","source"},{"portId","out"}}},
                          {"target",{{"nodeId","target"},{"portId","in"}}},{"futureEdge","keep"}}}}}};
    file["project"]["graphs"].push_back({{"id","untouched"},{"nodes",Document::array()},
                                       {"connections",Document::array()},{"futureGraph",123}});
    file["workspace"]={{"viewers",{{"camera",{1,2,3}}}}};
    file["futureEnvelope"]={{"keep","all"}};
    return file;
}
double execute(const DocumentSession& session, const std::shared_ptr<tp_pipeline::StepDelegateMap>& registry)
{
    auto factory=std::make_shared<tp_data::CollectionFactory>();
    tp_data::createCollectionFactories(*factory);
    factory->finalize();
    PipelineExecution executor;
    auto handle=executor.submit(session.executableGraph(),registry,factory);
    if(handle.result.wait_for(std::chrono::seconds(10))!=std::future_status::ready) throw FileError("Test execution timeout");
    const auto result=handle.result.get();
    if(!result.succeeded()) throw FileError("Test graph failed");
    return dynamic_cast<const tp_data::DoubleMember*>(result.steps.at("target").output->members().front().get())->data;
}
}

class DocumentSessionTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void savedGraphExecutesAndEditsPreserveOtherContent()
    {
        const auto registry=numericDelegates();
        const auto original=fixture();
        DocumentSession session(original,"graph",registry,registrations());
        QVERIFY(session.diagnostics().empty());
        QVERIFY(session.document()==original);
        QCOMPARE(execute(session,registry),42.0);
        session.setParameter("source","value",9.0);
        auto expected=original;
        expected["project"]["graphs"][0]["nodes"][0]["parameters"]["value"]=9.0;
        QVERIFY(session.document()==expected);
        QCOMPARE(execute(session,registry),10.0);
        QTemporaryDir directory;
        const auto path=directory.filePath("project.smartflow");
        write(path,session.document());
        DocumentSession reopened(read(path),"graph",registry,registrations());
        QVERIFY(reopened.document()==expected);
        QCOMPARE(execute(reopened,registry),10.0);
    }
    void unknownContentAndIncompatibleConnectionsBlockExecutionWithoutLoss()
    {
        const std::vector<std::function<void(Document&)>> mutations={
            [](auto& d) { d["project"]["graphs"][0]["nodes"][0]["packageId"]="unavailable"; },
            [](auto& d) { d["project"]["graphs"][0]["nodes"][0]["version"]=99; },
            [](auto& d) { d["project"]["graphs"][0]["nodes"][0]["parameters"]["future"]={{"raw",{1,2,3}}}; },
            [](auto& d) { d["project"]["graphs"][0]["nodes"][0]["parameters"].erase("value"); },
            [](auto& d) { d["project"]["graphs"][0]["nodes"][0]["parameters"]["value"]="unhandled"; },
            [](auto& d) { d["project"]["graphs"][0]["connections"][0]["source"]["nodeId"]="absent"; },
            [](auto& d) { d["project"]["graphs"][0]["connections"][0]["source"]["portId"]="absent"; },
            [](auto& d) { auto edge=d["project"]["graphs"][0]["connections"][0]; edge["id"]="duplicate-input"; d["project"]["graphs"][0]["connections"].push_back(edge); }
        };
        for(const auto& mutation : mutations) {
            auto document=fixture(); mutation(document);
            DocumentSession session(document,"graph",numericDelegates(),registrations());
            QVERIFY(!session.diagnostics().empty());
            QVERIFY_EXCEPTION_THROWN(session.executableGraph(),FileError);
            QVERIFY(parse(serialize(session.document()))==document);
        }
    }
    void rejectedEditsAndReplacementLeaveOriginalIntact()
    {
        DocumentSession current(fixture(),"graph",numericDelegates(),registrations());
        const auto before=current.document();
        QVERIFY_EXCEPTION_THROWN(current.setParameter("source","value",1000001),FileError);
        QVERIFY_EXCEPTION_THROWN(current.setParameter("source","value","bad"),FileError);
        QVERIFY_EXCEPTION_THROWN(current.setParameter("source","unknown",1),FileError);
        QVERIFY_EXCEPTION_THROWN(current.setParameter("absent","value",1),FileError);
        auto invalid=fixture(); invalid["schemaVersion"]=99;
        QVERIFY_EXCEPTION_THROWN(DocumentSession(invalid,"graph",numericDelegates(),registrations()),FileError);
        QVERIFY(current.document()==before);
        QCOMPARE(execute(current,numericDelegates()),42.0);
    }
    void knownEditDoesNotEraseUnsupportedParameters()
    {
        auto document=fixture();
        document["project"]["graphs"][0]["nodes"][0]["parameters"]["opaque"]={{"future",true}};
        DocumentSession session(document,"graph",numericDelegates(),registrations());
        session.setParameter("source","value",25);
        document["project"]["graphs"][0]["nodes"][0]["parameters"]["value"]=25;
        QVERIFY(session.document()==document);
        QVERIFY(!session.diagnostics().empty());
        QVERIFY_EXCEPTION_THROWN(session.executableGraph(),FileError);
    }
    void ambiguousRegistrationAndMissingGraphAreRejected()
    {
        auto entries=registrations(); entries.push_back(entries.front());
        QVERIFY_EXCEPTION_THROWN(DocumentSession(fixture(),"graph",numericDelegates(),entries),FileError);
        QVERIFY_EXCEPTION_THROWN(DocumentSession(fixture(),"absent",numericDelegates(),registrations()),FileError);
        entries=registrations(); entries.front().packageId.clear();
        QVERIFY_EXCEPTION_THROWN(DocumentSession(fixture(),"graph",numericDelegates(),entries),FileError);
    }
};
QTEST_GUILESS_MAIN(DocumentSessionTests)
#include "DocumentSessionTests.moc"
