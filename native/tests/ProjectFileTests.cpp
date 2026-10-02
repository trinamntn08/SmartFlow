#include "project/ProjectFile.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryDir>
#include <QtTest/QtTest>
#include <limits>

using namespace smartflow::project;

namespace {
Document fixture() { return read(QString::fromUtf8(SMARTFLOW_PROJECT_FIXTURE)); }
QByteArray contents(const QString& path)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) throw FileError(file.errorString().toStdString());
    return file.readAll();
}
}

class ProjectFileTests : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void sharedBrowserTransportAcceptance()
    {
        const auto path = QFileInfo(QString::fromUtf8(SMARTFLOW_PROJECT_FIXTURE)).dir().filePath("transport-v1.json");
        const auto cases = Document::parse(contents(path).toStdString());
        const auto envelope = [](const QByteArray& payload) {
            return QByteArray("{\"format\":\"smartflow\",\"schemaVersion\":1,\"project\":{\"id\":\"transport\",\"graphs\":[],\"packages\":[],\"assets\":[]},\"workspace\":{},\"opaque\":") + payload + "}";
        };
        for(const auto& entry : cases["cases"]) {
            const auto input = envelope(QByteArray::fromStdString(entry["json"].get<std::string>()));
            bool accepted = false;
            try { const auto document = parse(input); accepted = parse(serialize(document)) == document; }
            catch(const FileError&) {}
            QVERIFY2(accepted == entry["native"].get<bool>(),entry["name"].get<std::string>().c_str());
        }
        for(const auto& entry : cases["names"]) {
            const auto id = entry["id"].get<std::string>();
            bool accepted = false;
            try { accepted = parse(serialize(create(id)))["project"]["id"] == id; }
            catch(const FileError& error) { qInfo() << "Identity rejection:" << error.what(); }
            QVERIFY2(accepted == entry["accepted"].get<bool>(),entry.dump().c_str());
        }
        for(const auto& entry : cases["depths"]) {
            const auto count = entry["arrays"].get<int>();
            const auto input = envelope(QByteArray(count,'[') + "0" + QByteArray(count,']'));
            if(entry["accepted"].get<bool>()) QVERIFY(parse(serialize(parse(input))) == parse(input));
            else QVERIFY_EXCEPTION_THROWN(parse(input),FileError);
        }
        for(const auto& entry : cases["sizes"]) {
            auto input = envelope(QByteArray::fromStdString("\"é😀\""));
            input += QByteArray(entry["bytes"].get<int>() - input.size(),' ');
            if(entry["accepted"].get<bool>()) QVERIFY(parse(input).is_object());
            else QVERIFY_EXCEPTION_THROWN(parse(input),FileError);
            auto document = parse(envelope(QByteArray::fromStdString("\"é😀\"")));
            const auto padding = entry["bytes"].get<int>() - serialize(document).size();
            document["opaque"] = document["opaque"].get<std::string>() + std::string(size_t(padding),'x');
            if(entry["accepted"].get<bool>()) QCOMPARE(serialize(document).size(),entry["bytes"].get<int>());
            else QVERIFY_EXCEPTION_THROWN(serialize(document),FileError);
        }
        const auto input = envelope("null");
        QVERIFY(parse(QByteArray::fromHex("efbbbf") + input) == parse(input));
        QVERIFY_EXCEPTION_THROWN(parse(envelope(QByteArray("\"") + QByteArray::fromHex("c328") + "\"")),FileError);
    }

    void sharedFixtureAndUnknownContentSurviveFileRoundTrip()
    {
        const auto original = fixture();
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const auto path = directory.filePath(QString::fromUtf8("projet-é.smartflow"));
        write(path,original);
        auto loaded = read(path);
        QVERIFY(loaded == original);
        QVERIFY(loaded["project"]["graphs"][0]["connections"][0]["source"]["nodeId"] == "missing-node");
        loaded["project"]["graphs"][0]["nodes"][0]["parameters"]["value"] = 42.5;
        write(path,loaded);
        QVERIFY(read(path) == loaded);
        QVERIFY(original["project"]["graphs"][0]["nodes"][0]["parameters"]["value"] == 3.25);
        // Make the one intentional edit in the baseline; everything else must
        // survive, including all unknown fields and unresolved connections.
        auto expected = original;
        expected["project"]["graphs"][0]["nodes"][0]["parameters"]["value"] = 42.5;
        QVERIFY(read(path) == expected);
    }

    void emptyProjectNeedsNoDomainOrRegistry()
    {
        const auto empty = create("empty");
        QVERIFY(parse(serialize(empty)) == empty);
        QVERIFY(empty["project"]["graphs"].empty());
        QVERIFY(empty["project"]["packages"].empty());
        QVERIFY_EXCEPTION_THROWN(create(" \t"),FileError);
    }

    void malformedAndUnsupportedDocumentsAreRejected()
    {
        for(const auto& bytes : {QByteArray("{"), QByteArray("null"), QByteArray("[]"), QByteArray("{}"),
                                QByteArray("{\"duplicate\":1,\"duplicate\":2}"),
                                QByteArray("{\"number\":1e400}"),
                                QByteArray("{\"number\":18446744073709551616}")})
            QVERIFY_EXCEPTION_THROWN(parse(bytes),FileError);
        const std::vector<std::function<void(Document&)>> mutations = {
            [](auto& d) { d["schemaVersion"]=2; },
            [](auto& d) { d["workspace"]=Document::array(); },
            [](auto& d) { d["project"].erase("assets"); },
            [](auto& d) { d["project"]["graphs"].push_back(d["project"]["graphs"][0]); },
            [](auto& d) { d["project"]["graphs"][0]["nodes"].push_back(d["project"]["graphs"][0]["nodes"][0]); },
            [](auto& d) { d["project"]["graphs"][0]["nodes"][0]["version"]=0; },
            [](auto& d) { d["project"]["graphs"][0]["nodes"][0]["version"]=1.5; },
            [](auto& d) { d["project"]["graphs"][0]["nodes"][0]["version"]=9007199254740992ULL; },
            [](auto& d) { d["project"]["graphs"][0]["nodes"][0]["parameters"]=nullptr; },
            [](auto& d) { d["project"]["graphs"][0]["connections"][0]["source"]["portId"]=""; },
            [](auto& d) { d["project"]["packages"][0]["version"]=" "; },
            [](auto& d) { d["project"]["assets"][0]["uri"]=false; }
        };
        for(const auto& mutation : mutations) {
            auto document = fixture();
            mutation(document);
            QVERIFY_EXCEPTION_THROWN(serialize(document),FileError);
        }
    }

    void invalidSaveDoesNotDamageExistingFile()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const auto path = directory.filePath("project.smartflow");
        const auto original = fixture();
        write(path,original);
        const auto bytes = contents(path);
        auto invalid = original;
        invalid["schemaVersion"]=99;
        QVERIFY_EXCEPTION_THROWN(write(path,invalid),FileError);
        QCOMPARE(contents(path),bytes);
        invalid=original;
        invalid["futureEnvelope"]["bad"]=std::numeric_limits<double>::infinity();
        QVERIFY_EXCEPTION_THROWN(write(path,invalid),FileError);
        QCOMPARE(contents(path),bytes);
        QVERIFY_EXCEPTION_THROWN(write(directory.filePath("missing/project.smartflow"),original),FileError);
        QVERIFY(read(path) == original);
        QVERIFY_EXCEPTION_THROWN(read(directory.filePath("absent.smartflow")),FileError);
    }

    void finiteNumbersAndIntegerPrecisionArePreserved()
    {
        auto document = fixture();
        document["futureNumbers"] = {std::numeric_limits<int64_t>::min(),
                                     std::numeric_limits<uint64_t>::max(), 1.25e100};
        const auto reopened = parse(serialize(document));
        QVERIFY(reopened == document);
        QVERIFY(reopened["futureNumbers"][1].is_number_unsigned());
        QCOMPARE(reopened["futureNumbers"][1].get<uint64_t>(),std::numeric_limits<uint64_t>::max());
        document["futureNumbers"]=std::numeric_limits<double>::quiet_NaN();
        QVERIFY_EXCEPTION_THROWN(serialize(document),FileError);
    }

    void resourceLimitsRejectBeforeReplacement()
    {
        QTemporaryDir directory;
        const auto path=directory.filePath("project.smartflow");
        write(path,fixture());
        const auto before=contents(path);
        auto document=fixture();
        document["oversized"] = std::string(size_t(MaximumFileBytes),'x');
        QVERIFY_EXCEPTION_THROWN(write(path,document),FileError);
        QCOMPARE(contents(path),before);
        QVERIFY_EXCEPTION_THROWN(parse(QByteArray(MaximumFileBytes+1,' ')),FileError);
        QByteArray nested(MaximumNesting+1,'[');
        nested += '0';
        nested += QByteArray(MaximumNesting+1,']');
        QVERIFY_EXCEPTION_THROWN(parse(nested),FileError);
        auto branch=Document::array();
        for(int i=0; i<MaximumNesting; ++i) branch=Document::array({branch});
        document=fixture();
        document["deep"]=std::move(branch);
        QVERIFY_EXCEPTION_THROWN(write(path,document),FileError);
        QCOMPARE(contents(path),before);
    }
};
QTEST_GUILESS_MAIN(ProjectFileTests)
#include "ProjectFileTests.moc"
