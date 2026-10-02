#include "ProjectFile.h"
#include <QFile>
#include <QSaveFile>
#include <cmath>
#include <set>
#include <vector>

namespace smartflow::project {
namespace {
[[noreturn]] void fail(const std::string& message) { throw FileError(message); }

// Preflight avoids losing duplicate keys and integer overflow before the DOM
// exists. JSON floating-point numbers use finite IEEE doubles; integer tokens
// must fit signed/unsigned 64-bit storage instead of silently becoming doubles.
class Preflight final : public nlohmann::json_sax<Document> {
public:
    bool null() override { return true; }
    bool boolean(bool) override { return true; }
    bool number_integer(number_integer_t) override { return true; }
    bool number_unsigned(number_unsigned_t) override { return true; }
    bool number_float(number_float_t number, const string_t& token) override
    {
        if(!std::isfinite(number)) fail("JSON contains a nonfinite number");
        if(token.find_first_of(".eE") == std::string::npos)
            fail("Integer is outside the supported 64-bit range");
        return true;
    }
    bool string(string_t&) override { return true; }
    bool binary(binary_t&) override { fail("Binary JSON is unsupported"); }
    bool start_object(std::size_t) override { return enter(); }
    bool start_array(std::size_t) override { return enter(); }
    bool key(string_t& key) override
    {
        if(!keys.back().insert(key).second) fail("Duplicate JSON key: " + key);
        return true;
    }
    bool end_object() override { keys.pop_back(); return true; }
    bool end_array() override { keys.pop_back(); return true; }
    bool parse_error(std::size_t, const std::string&, const nlohmann::detail::exception& error) override
    { fail(std::string("Invalid JSON: ") + error.what()); }
private:
    bool enter()
    {
        if(keys.size() >= MaximumNesting) fail("JSON exceeds nesting limit");
        keys.emplace_back();
        return true;
    }
    std::vector<std::set<std::string>> keys;
};

const Document& field(const Document& value, const char* name)
{
    const auto found = value.find(name);
    if(found == value.end()) fail(std::string("Missing field: ") + name);
    return *found;
}
void object(const Document& value, const std::string& path)
{
    if(!value.is_object()) fail(path + ": expected object");
}
void name(const Document& value, const std::string& path)
{
    if(!value.is_string()) fail(path + ": expected nonempty string");
    const auto& text = value.get_ref<const std::string&>();
    if(QString::fromUtf8(text.data(), qsizetype(text.size())).trimmed().isEmpty())
        fail(path + ": expected nonempty string");
}
void identified(const Document& value, const std::string& path)
{
    if(!value.is_array()) fail(path + ": expected array");
    std::set<std::string> ids;
    for(const auto& item : value) {
        object(item, path);
        const auto& id = field(item,"id");
        name(id,path + ".id");
        if(!ids.insert(id.get<std::string>()).second) fail(path + ": duplicate id");
    }
}
void jsonValues(const Document& value, int depth = 0)
{
    if(value.is_discarded() || value.is_binary()) fail("JSON contains a non-JSON value");
    if(value.is_number_float() && !std::isfinite(value.get<double>())) fail("JSON contains a nonfinite number");
    if(value.is_structured()) {
        if(depth >= MaximumNesting) fail("JSON exceeds nesting limit");
        for(const auto& child : value) jsonValues(child,depth+1);
    }
}
std::string ioMessage(const QString& path, const QString& detail)
{
    return (path + ": " + detail).toStdString();
}
}

Document create(const std::string& projectId)
{
    Document result = {{"format","smartflow"}, {"schemaVersion",1},
        {"project", {{"id",projectId}, {"graphs",Document::array()},
                     {"packages",Document::array()}, {"assets",Document::array()}}},
        {"workspace",Document::object()}};
    validate(result);
    return result;
}

void validate(const Document& document)
{
    jsonValues(document);
    object(document,"file");
    if(field(document,"format") != "smartflow" || field(document,"schemaVersion") != 1)
        fail("Unsupported project format or schema version");
    const auto& project = field(document,"project");
    object(project,"project");
    object(field(document,"workspace"),"workspace");
    name(field(project,"id"),"project.id");
    const auto& packages = field(project,"packages");
    identified(packages,"packages");
    for(const auto& package : packages) name(field(package,"version"),"package.version");
    const auto& assets = field(project,"assets");
    identified(assets,"assets");
    for(const auto& asset : assets) name(field(asset,"uri"),"asset.uri");
    const auto& graphs = field(project,"graphs");
    identified(graphs,"graphs");
    for(const auto& graph : graphs) {
        const auto& nodes = field(graph,"nodes");
        identified(nodes,"nodes");
        for(const auto& node : nodes) {
            name(field(node,"packageId"),"node.packageId");
            name(field(node,"typeId"),"node.typeId");
            const auto& version = field(node,"version");
            if(!version.is_number()) fail("node.version: expected positive safe integer");
            const auto number = version.get<double>();
            if(number < 1 || number > 9007199254740991.0 || std::floor(number) != number)
                fail("node.version: expected positive safe integer");
            object(field(node,"parameters"),"node.parameters");
        }
        const auto& connections = field(graph,"connections");
        identified(connections,"connections");
        for(const auto& connection : connections)
            for(const auto* end : {"source","target"}) {
                const auto& endpoint = field(connection,end);
                object(endpoint,std::string("connection.") + end);
                name(field(endpoint,"nodeId"),"endpoint.nodeId");
                name(field(endpoint,"portId"),"endpoint.portId");
            }
    }
}

namespace jsonFile {
Document parse(const QByteArray& bytes)
{
    if(bytes.size() > MaximumFileBytes) fail("JSON exceeds 16 MiB file limit");
    try {
        Preflight preflight;
        Document::sax_parse(bytes.begin(),bytes.end(),&preflight);
        auto document = Document::parse(bytes.begin(),bytes.end());
        jsonValues(document);
        return document;
    } catch(const nlohmann::json::exception& error) {
        fail(std::string("Invalid JSON: ") + error.what());
    }
}

QByteArray serialize(const Document& document)
{
    jsonValues(document);
    try {
        const auto text = document.dump(2) + "\n";
        if(text.size() > size_t(MaximumFileBytes)) fail("JSON exceeds 16 MiB file limit");
        return QByteArray(text.data(),qsizetype(text.size()));
    } catch(const nlohmann::json::exception& error) {
        fail(std::string("Cannot serialize JSON: ") + error.what());
    }
}

Document read(const QString& path)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) fail(ioMessage(path,file.errorString()));
    const auto bytes = file.read(MaximumFileBytes+1);
    if(file.error() != QFileDevice::NoError) fail(ioMessage(path,file.errorString()));
    return jsonFile::parse(bytes);
}

void write(const QString& path, const Document& document)
{
    const auto bytes = jsonFile::serialize(document);
    QSaveFile file(path);
    file.setDirectWriteFallback(false);
    if(!file.open(QIODevice::WriteOnly)) fail(ioMessage(path,file.errorString()));
    if(file.write(bytes) != bytes.size()) {
        file.cancelWriting();
        fail(ioMessage(path,file.errorString()));
    }
    if(!file.commit()) fail(ioMessage(path,file.errorString()));
}
} // namespace jsonFile

Document parse(const QByteArray& bytes)
{
    auto document = jsonFile::parse(bytes);
    validate(document);
    return document;
}

QByteArray serialize(const Document& document)
{
    validate(document);
    return jsonFile::serialize(document);
}

Document read(const QString& path)
{
    auto document = jsonFile::read(path);
    validate(document);
    return document;
}

void write(const QString& path, const Document& document)
{
    validate(document);
    jsonFile::write(path, document);
}
} // namespace smartflow::project
