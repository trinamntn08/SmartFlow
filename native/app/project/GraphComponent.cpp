#include "GraphComponent.h"
#include <algorithm>
#include <map>
#include <queue>
#include <set>

namespace smartflow::project {
namespace {
const Document& field(const Document& value, const char* key)
{
    if(!value.is_object() || !value.contains(key)) throw FileError(std::string("Component missing field: ")+key);
    return value[key];
}
std::string name(const Document& value)
{
    if(!value.is_string() || QString::fromStdString(value.get<std::string>()).trimmed().isEmpty())
        throw FileError("Component identity must be a nonempty string");
    return value.get<std::string>();
}
std::string scoped(const std::string& instance, const std::string& kind, const std::string& local)
{
    // Length-prefix the instance so slashes/colons in user IDs cannot alias.
    return "component/"+kind+"/"+std::to_string(instance.size())+"/"+instance+"/"+local;
}
Document remapEndpoint(Document endpoint, const std::string& instance)
{
    endpoint["nodeId"]=scoped(instance,"node",endpoint["nodeId"].get<std::string>());
    return endpoint;
}
}

GraphComponent::GraphComponent(Document definition) : retained(std::move(definition))
{
    if(field(retained,"format")!="smartflow.graph-component" || field(retained,"schemaVersion")!=1 ||
       field(retained,"version")!=1) throw FileError("Unsupported graph component format or version");
    name(field(retained,"id")); name(field(retained,"title"));
    // Reuse the project codec's strict graph/JSON validation without teaching
    // that codec to interpret optional or unknown component catalogs.
    auto envelope=create("component-validation");
    envelope["project"]["graphs"]=Document::array({field(retained,"graph")});
    envelope["component"]=retained;
    validate(envelope);
    const auto& graph=retained["graph"];
    if(graph["nodes"].empty()) throw FileError("Component graph must contain nodes");
    std::map<std::string,const Document*> nodes;
    std::map<std::string,size_t> indegree;
    std::map<std::string,std::vector<std::string>> consumers;
    for(const auto& node : graph["nodes"]) {
        if(isInstance(node)) throw FileError("Nested graph components are unsupported");
        const auto id=node["id"].get<std::string>();
        nodes.emplace(id,&node); indegree[id]=0;
    }
    auto endpoint=[&](const Document& value, const char* portKey) {
        const auto node=name(field(value,"nodeId"));
        const auto port=name(field(value,portKey));
        if(!nodes.count(node)) throw FileError("Component endpoint node is missing: "+node);
        return std::make_pair(node,port);
    };
    std::set<std::pair<std::string,std::string>> occupied;
    for(const auto& edge : graph["connections"]) {
        const auto from=endpoint(edge["source"],"portId"), to=endpoint(edge["target"],"portId");
        if(!occupied.insert(to).second) throw FileError("Component input has multiple producers");
        ++indegree[to.first]; consumers[from.first].push_back(to.first);
    }
    std::queue<std::string> ready;
    for(const auto& [id,count] : indegree) if(!count) ready.push(id);
    size_t visited=0;
    while(!ready.empty()) {
        const auto id=ready.front(); ready.pop(); ++visited;
        for(const auto& consumer : consumers[id]) if(!--indegree[consumer]) ready.push(consumer);
    }
    if(visited!=nodes.size()) throw FileError("Component graph contains a cycle");
    for(const auto* section : {"inputs","outputs","controls"}) {
        const auto& entries=field(retained,section);
        if(!entries.is_array()) throw FileError("Component interface must be an array");
        std::set<std::string> identities;
        std::set<std::pair<std::string,std::string>> targets;
        for(const auto& entry : entries) {
            if(!identities.insert(name(field(entry,"id"))).second) throw FileError("Duplicate component interface ID");
            const bool control=std::string(section)=="controls";
            const bool output=std::string(section)=="outputs";
            const auto target=endpoint(field(entry,output ? "source" : "target"),control ? "parameter" : "portId");
            if(!output && !targets.insert(target).second) throw FileError("Duplicate exposed component target");
            if(std::string(section)=="inputs" && occupied.count(target))
                throw FileError("Exposed component input already has an internal producer");
            if(control && !(*nodes.at(target.first))["parameters"].contains(target.second))
                throw FileError("Exposed component parameter is missing");
        }
    }
}

GraphComponent GraphComponent::extract(const Document& source, const std::string& graphId,
    const std::vector<std::string>& nodeIds, const std::string& id, const std::string& title,
    const Document& inputs, const Document& outputs, const Document& controls)
{
    validate(source);
    const auto& graphs=source["project"]["graphs"];
    const auto found=std::find_if(graphs.begin(),graphs.end(),[&](const auto& graph) { return graph["id"]==graphId; });
    if(found==graphs.end()) throw FileError("Component source graph is missing");
    const std::set<std::string> selected(nodeIds.begin(),nodeIds.end());
    if(selected.empty() || selected.size()!=nodeIds.size()) throw FileError("Component selection must be nonempty and unique");
    auto body=*found;
    body["id"]="body";
    body["nodes"]=Document::array(); body["connections"]=Document::array();
    for(const auto& node : (*found)["nodes"])
        if(selected.count(node["id"].get<std::string>())) body["nodes"].push_back(node);
    if(body["nodes"].size()!=selected.size()) throw FileError("Component selection contains a missing node");
    for(const auto& edge : (*found)["connections"])
        if(selected.count(edge["source"]["nodeId"].get<std::string>()) &&
           selected.count(edge["target"]["nodeId"].get<std::string>())) body["connections"].push_back(edge);
    GraphComponent result({{"format","smartflow.graph-component"},{"schemaVersion",1},{"version",1},
        {"id",id},{"title",title},{"graph",body},{"inputs",inputs},{"outputs",outputs},{"controls",controls}});
    for(const auto& edge : (*found)["connections"]) {
        const bool from=selected.count(edge["source"]["nodeId"].get<std::string>());
        const bool to=selected.count(edge["target"]["nodeId"].get<std::string>());
        if(from==to) continue;
        const auto& endpoint=edge[to ? "target" : "source"];
        const auto& entries=to ? inputs : outputs;
        const char* key=to ? "target" : "source";
        if(std::none_of(entries.begin(),entries.end(),[&](const auto& entry) {
            return entry[key]["nodeId"]==endpoint["nodeId"] && entry[key]["portId"]==endpoint["portId"];
        })) throw FileError("Component boundary connection requires an exposed endpoint");
    }
    return result;
}

Document GraphComponent::catalog(const Document& source) const
{
    validate(source);
    auto result=source;
    auto& catalog=result["project"]["components"];
    if(catalog.is_null()) catalog=Document::array();
    if(!catalog.is_array()) throw FileError("Unsupported component catalog shape");
    bool cataloged=false;
    for(const auto& definition : catalog)
        if(definition.is_object() && definition.value("id",Document())==retained["id"] &&
           definition.value("version",Document())==retained["version"]) {
            if(definition!=retained) throw FileError("Component definition conflicts with saved identity/version");
            cataloged=true;
        }
    if(!cataloged) catalog.push_back(retained);
    validate(result);
    return result;
}

bool GraphComponent::isInstance(const Document& node)
{
    return node.is_object() && node.value("packageId",Document())=="smartflow.components" &&
        node.value("typeId",Document())=="instance";
}

ComponentExpansion GraphComponent::expandBody(const std::string& instanceId, const Document& controls) const
{
    name(instanceId);
    if(!controls.is_object()) throw FileError("Component controls must be an object");
    ComponentExpansion result{retained["graph"]};
    std::set<std::string> known;
    for(const auto& control : retained["controls"]) {
        const auto id=control["id"].get<std::string>(); known.insert(id);
        const auto& target=control["target"];
        for(auto& node : result.graph["nodes"])
            if(node["id"]==target["nodeId"] && controls.contains(id))
                node["parameters"][target["parameter"].get<std::string>()]=controls[id];
        result.bindings.controls[id]=remapEndpoint(target,instanceId);
    }
    for(auto entry=controls.begin(); entry!=controls.end(); ++entry)
        if(!known.count(entry.key())) throw FileError("Unknown exposed component control: "+entry.key());
    for(auto& node : result.graph["nodes"]) node["id"]=scoped(instanceId,"node",node["id"]);
    for(auto& edge : result.graph["connections"]) {
        edge["id"]=scoped(instanceId,"edge",edge["id"]);
        edge["source"]=remapEndpoint(edge["source"],instanceId);
        edge["target"]=remapEndpoint(edge["target"],instanceId);
    }
    for(const auto& input : retained["inputs"])
        result.inputs[input["id"].get<std::string>()]=remapEndpoint(input["target"],instanceId);
    for(const auto& output : retained["outputs"])
        result.bindings.outputs[output["id"].get<std::string>()]=remapEndpoint(output["source"],instanceId);
    return result;
}

ComponentInstantiation GraphComponent::instantiateCollapsed(const Document& source, const std::string& graphId,
    const std::string& instanceId, const Document& controls, const Document& inputSources) const
{
    const auto expanded=expandBody(instanceId,controls);
    if(!inputSources.is_object()) throw FileError("Component inputs must be an object");
    ComponentInstantiation result{catalog(source),{}};
    auto& graphs=result.document["project"]["graphs"];
    const auto graph=std::find_if(graphs.begin(),graphs.end(),[&](const auto& item) { return item["id"]==graphId; });
    if(graph==graphs.end()) throw FileError("Component destination graph is missing");
    if(std::any_of((*graph)["nodes"].begin(),(*graph)["nodes"].end(),[&](const auto& node) { return node["id"]==instanceId; }))
        throw FileError("Component instance node ID collision");
    auto parameters=Document::object();
    for(const auto& control : retained["controls"]) {
        const auto id=control["id"].get<std::string>();
        for(const auto& node : retained["graph"]["nodes"])
            if(node["id"]==control["target"]["nodeId"])
                parameters[id]=controls.contains(id) ? controls[id] : node["parameters"][control["target"]["parameter"].get<std::string>()];
        result.bindings.controls[id]={{"nodeId",instanceId},{"parameter",id}};
    }
    std::set<std::string> edges;
    for(const auto& edge : (*graph)["connections"]) edges.insert(edge["id"].get<std::string>());
    for(auto input=inputSources.begin(); input!=inputSources.end(); ++input)
        if(!expanded.inputs.contains(input.key())) throw FileError("Unknown exposed component input: "+input.key());
    for(auto input=expanded.inputs.begin(); input!=expanded.inputs.end(); ++input) {
        if(!inputSources.contains(input.key())) throw FileError("Missing exposed component input: "+input.key());
        const auto& endpoint=inputSources[input.key()];
        const auto from=name(field(endpoint,"nodeId")); name(field(endpoint,"portId"));
        if(std::none_of((*graph)["nodes"].begin(),(*graph)["nodes"].end(),[&](const auto& node) { return node["id"]==from; }))
            throw FileError("Component input source node is missing");
        const auto edgeId=scoped(instanceId,"input",input.key());
        if(!edges.insert(edgeId).second) throw FileError("Component instance edge ID collision");
        (*graph)["connections"].push_back({{"id",edgeId},{"source",endpoint},
            {"target",{{"nodeId",instanceId},{"portId",input.key()}}}});
    }
    (*graph)["nodes"].push_back({{"id",instanceId},{"packageId","smartflow.components"},
        {"typeId","instance"},{"version",1},{"parameters",parameters},{"component",retained}});
    for(const auto& output : retained["outputs"])
        result.bindings.outputs[output["id"].get<std::string>()]={{"nodeId",instanceId},{"portId",output["id"]}};
    validate(result.document);
    return result;
}

ComponentInstantiation GraphComponent::instantiate(const Document& source, const std::string& graphId,
    const std::string& instanceId, const Document& controls, const Document& inputSources) const
{
    name(instanceId); validate(source);
    if(!controls.is_object() || !inputSources.is_object()) throw FileError("Component bindings must be objects");
    ComponentInstantiation result{catalog(source),{}};
    auto& graphs=result.document["project"]["graphs"];
    const auto graph=std::find_if(graphs.begin(),graphs.end(),[&](const auto& item) { return item["id"]==graphId; });
    if(graph==graphs.end()) throw FileError("Component destination graph is missing");
    std::set<std::string> existingNodes, existingEdges;
    for(const auto& node : (*graph)["nodes"]) existingNodes.insert(node["id"].get<std::string>());
    for(const auto& edge : (*graph)["connections"]) existingEdges.insert(edge["id"].get<std::string>());
    auto body=retained["graph"];
    std::set<std::string> knownControls, knownInputs;
    for(const auto& control : retained["controls"]) {
        const auto id=control["id"].get<std::string>(); knownControls.insert(id);
        auto target=control["target"];
        for(auto& node : body["nodes"])
            if(node["id"]==target["nodeId"] && controls.contains(id))
                node["parameters"][target["parameter"].get<std::string>()]=controls[id];
        result.bindings.controls[id]=remapEndpoint(target,instanceId);
    }
    for(auto it=controls.begin(); it!=controls.end(); ++it)
        if(!knownControls.count(it.key())) throw FileError("Unknown exposed component control: "+it.key());
    for(auto& node : body["nodes"]) {
        node["id"]=scoped(instanceId,"node",node["id"].get<std::string>());
        if(!existingNodes.insert(node["id"].get<std::string>()).second) throw FileError("Component instance node ID collision");
        (*graph)["nodes"].push_back(node);
    }
    auto appendEdge=[&](Document edge) {
        if(!existingEdges.insert(edge["id"].get<std::string>()).second) throw FileError("Component instance edge ID collision");
        (*graph)["connections"].push_back(std::move(edge));
    };
    for(auto edge : body["connections"]) {
        edge["id"]=scoped(instanceId,"edge",edge["id"].get<std::string>());
        edge["source"]=remapEndpoint(edge["source"],instanceId);
        edge["target"]=remapEndpoint(edge["target"],instanceId);
        appendEdge(std::move(edge));
    }
    for(const auto& input : retained["inputs"]) {
        const auto id=input["id"].get<std::string>(); knownInputs.insert(id);
        if(!inputSources.contains(id)) throw FileError("Missing exposed component input: "+id);
        const auto& from=inputSources[id];
        const auto sourceNode=name(field(from,"nodeId")); name(field(from,"portId"));
        // External input bindings must refer to nodes in the original graph.
        const auto& originalGraphs=source["project"]["graphs"];
        const auto original=std::find_if(originalGraphs.begin(),originalGraphs.end(),[&](const auto& item) { return item["id"]==graphId; });
        if(std::none_of((*original)["nodes"].begin(),(*original)["nodes"].end(),[&](const auto& node) { return node["id"]==sourceNode; }))
            throw FileError("Component input source node is missing");
        appendEdge({{"id",scoped(instanceId,"input",id)},{"source",from},{"target",remapEndpoint(input["target"],instanceId)}});
    }
    for(auto it=inputSources.begin(); it!=inputSources.end(); ++it)
        if(!knownInputs.count(it.key())) throw FileError("Unknown exposed component input: "+it.key());
    for(const auto& output : retained["outputs"])
        result.bindings.outputs[output["id"].get<std::string>()]=remapEndpoint(output["source"],instanceId);
    validate(result.document);
    return result;
}
} // namespace smartflow::project
