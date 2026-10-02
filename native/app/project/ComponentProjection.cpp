#include "ComponentProjection.h"
#include <set>
#include <map>
#include <queue>

namespace smartflow::project {
Document expandComponents(const Document& graph, std::vector<ExpandedComponent>& instances,
    std::vector<std::string>& diagnostics)
{
    auto result=graph;
    result["nodes"]=Document::array();
    std::set<std::string> nodes, edges;
    for(const auto& node : graph["nodes"]) nodes.insert(node["id"].get<std::string>());
    for(const auto& edge : graph["connections"]) edges.insert(edge["id"].get<std::string>());
    for(const auto& node : graph["nodes"]) {
        if(!GraphComponent::isInstance(node)) { result["nodes"].push_back(node); continue; }
        const auto id=node["id"].get<std::string>();
        try {
            if(node["version"]!=1 || !node.contains("component")) throw FileError("Unsupported component instance contract");
            const GraphComponent definition(node["component"]);
            for(const auto& control : definition.definition()["controls"])
                if(!node["parameters"].contains(control["id"].get<std::string>())) throw FileError("Missing exposed component control");
            auto body=definition.expandBody(id,node["parameters"]);
            auto nextNodes=nodes, nextEdges=edges;
            for(const auto& child : body.graph["nodes"])
                if(!nextNodes.insert(child["id"].get<std::string>()).second) throw FileError("Component body node identity collision");
            for(const auto& edge : body.graph["connections"])
                if(!nextEdges.insert(edge["id"].get<std::string>()).second) throw FileError("Component body edge identity collision");
            nodes.swap(nextNodes); edges.swap(nextEdges);
            for(const auto& child : body.graph["nodes"]) result["nodes"].push_back(child);
            instances.push_back({node,std::move(body)});
        } catch(const std::exception& error) {
            diagnostics.push_back(id+": "+error.what());
            result["nodes"].push_back(node); // Unavailable placeholder, never repaired.
        }
    }
    for(auto& edge : result["connections"]) for(const auto* end : {"source","target"})
        for(const auto& instance : instances) if(edge[end]["nodeId"]==instance.node["id"]) {
            const auto& bindings=std::string(end)=="source" ? instance.body.bindings.outputs : instance.body.inputs;
            const auto port=edge[end]["portId"].get<std::string>();
            if(bindings.contains(port)) {
                // Preserve opaque endpoint fields in this disposable projection.
                edge[end]["nodeId"]=bindings[port]["nodeId"];
                edge[end]["portId"]=bindings[port]["portId"];
            }
        }
    for(const auto& instance : instances)
        for(const auto& edge : instance.body.graph["connections"]) result["connections"].push_back(edge);
    if(!instances.empty()) {
        // A component remains a single dependency unit, even if some of its
        // body branches are independent. Reject cycles at the visible boundary.
        std::map<std::string,size_t> indegree;
        std::map<std::string,std::vector<std::string>> consumers;
        for(const auto& node : graph["nodes"]) indegree[node["id"]]=0;
        for(const auto& edge : graph["connections"]) {
            const auto from=edge["source"]["nodeId"].get<std::string>(), to=edge["target"]["nodeId"].get<std::string>();
            if(indegree.count(from) && indegree.count(to)) { ++indegree[to]; consumers[from].push_back(to); }
        }
        std::queue<std::string> ready;
        for(const auto& [id,count] : indegree) if(!count) ready.push(id);
        size_t visited=0;
        while(!ready.empty()) {
            const auto id=ready.front(); ready.pop(); ++visited;
            for(const auto& to : consumers[id]) if(!--indegree[to]) ready.push(to);
        }
        if(visited!=indegree.size()) diagnostics.push_back("Component graph contains a cycle");
    }
    return result;
}
} // namespace smartflow::project
