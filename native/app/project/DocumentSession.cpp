#include "DocumentSession.h"
#include <tp_pipeline/StepDelegate.h>
#include <cmath>
#include <map>
#include <set>

namespace smartflow::project {
namespace {
using namespace tp_pipeline;
using tp_utils::StringID;

const NodePresentation* registration(const Document& node, const std::vector<NodePresentation>& entries)
{
    for(const auto& entry : entries)
        if(node["packageId"] == entry.packageId.toStdString() &&
           node["typeId"] == entry.typeId.toStdString() && node["version"] == entry.contractVersion)
            return &entry;
    return nullptr;
}

bool decode(Parameter& parameter, const Document& value)
{
    if(std::holds_alternative<double>(parameter.value)) {
        if(!value.is_number()) return false;
        // Do not silently round an unknown large integer into a numeric control.
        if(value.is_number_integer() && (value.get<double>() > 9007199254740991.0 ||
                                        value.get<double>() < -9007199254740991.0)) return false;
        const auto number = value.get<double>();
        if(!std::isfinite(number) || number < tpGetVariantValue<double>(parameter.min,-INFINITY) ||
           number > tpGetVariantValue<double>(parameter.max,INFINITY)) return false;
        parameter.value=number;
        return true;
    }
    if(std::holds_alternative<std::string>(parameter.value) && value.is_string()) {
        parameter.value=value.get<std::string>();
        return true;
    }
    if(std::holds_alternative<bool>(parameter.value) && value.is_boolean()) {
        parameter.value=value.get<bool>();
        return true;
    }
    return false;
}

StepDetails prototype(const StepDelegate& delegate)
{
    StepDetails result(delegate.name());
    std::vector<StringID> order;
    // Ask a fresh, disposable definition instance for parameter metadata. Never
    // fix up a loaded node or merge these default values into the source DOM.
    delegate.fixupParameters(&result,order);
    result.setParametersOrder(order);
    return result;
}

Document encode(const Parameter& parameter)
{
    Document value;
    if(const auto* number=std::get_if<double>(&parameter.value)) value=*number;
    else if(const auto* text=std::get_if<std::string>(&parameter.value)) value=*text;
    else if(const auto* boolean=std::get_if<bool>(&parameter.value)) value=*boolean;
    else throw FileError("Unsupported default parameter codec: " + parameter.name.toString());
    auto checked=parameter;
    if(!decode(checked,value)) throw FileError("Invalid default parameter: " + parameter.name.toString());
    return value;
}

Document::const_iterator findId(const Document& items, const std::string& id)
{
    return std::find_if(items.begin(),items.end(),[&](const auto& item) { return item["id"]==id; });
}
}

DocumentSession DocumentSession::empty(const std::string& projectId, const std::string& graphId,
    std::shared_ptr<const tp_pipeline::StepDelegateMap> delegates, std::vector<NodePresentation> registrations)
{
    auto document=create(projectId);
    document["project"]["graphs"].push_back({{"id",graphId},{"nodes",Document::array()},
                                           {"connections",Document::array()}});
    return DocumentSession(std::move(document),graphId,std::move(delegates),std::move(registrations));
}

DocumentSession::DocumentSession(Document source, std::string graphId,
    std::shared_ptr<const tp_pipeline::StepDelegateMap> delegates,
    std::vector<NodePresentation> registrations)
    : source(std::move(source)), delegates(std::move(delegates)), registrations(std::move(registrations))
{
    validate(this->source);
    if(!this->delegates) throw FileError("Missing delegate registry");
    std::set<std::tuple<std::string,std::string,int>> identities;
    std::set<std::string> runtimeTypes;
    for(const auto& entry : this->registrations) {
        if(entry.packageId.trimmed().isEmpty() || entry.typeId.trimmed().isEmpty() || entry.contractVersion < 1 ||
           !this->delegates->stepDelegate(entry.type.toStdString()))
            throw FileError("Invalid native persistence registration");
        if(!identities.emplace(entry.packageId.toStdString(),entry.typeId.toStdString(),entry.contractVersion).second)
            throw FileError("Duplicate native persistence registration");
        if(!runtimeTypes.insert(entry.type.toStdString()).second)
            throw FileError("Ambiguous native runtime type registration");
    }
    const auto& graphs=this->source["project"]["graphs"];
    auto found=std::find_if(graphs.begin(),graphs.end(),[&](const auto& graph) { return graph["id"] == graphId; });
    if(found==graphs.end()) throw FileError("Requested graph does not exist: " + graphId);
    graphIndex=size_t(std::distance(graphs.begin(),found));
    compile();
}

void DocumentSession::compile()
{
    auto candidate=std::make_unique<tp_pipeline::PipelineDetails>();
    std::vector<std::string> problems;
    std::map<std::string,tp_pipeline::StepDetails*> steps;
    const auto& graph=source["project"]["graphs"][graphIndex];
    for(const auto& node : graph["nodes"]) {
        const auto id=node["id"].get<std::string>();
        const auto* entry=registration(node,registrations);
        if(!entry) { problems.push_back(id + ": unavailable node type or contract version"); continue; }
        const auto* delegate=delegates->stepDelegate(entry->type.toStdString());
        auto definition=prototype(*delegate);
        auto step=std::make_unique<tp_pipeline::StepDetails>();
        // StepDetails has no ID setter. Pass only this generated, validated
        // identity envelope to its legacy loader, never any raw file payload.
        step->loadBinary({{"id",id},{"delegateName",entry->type.toStdString()}},{});
        const auto& parameters=node["parameters"];
        for(const auto& [name, metadata] : definition.parameters()) {
            auto parameter=metadata;
            const auto found=parameters.find(name.toString());
            if(found==parameters.end() || !decode(parameter,*found)) {
                problems.push_back(id + ": missing, invalid or unsupported parameter " + name.toString());
                continue;
            }
            step->setParamerter(parameter);
        }
        for(auto it=parameters.begin(); it!=parameters.end(); ++it)
            if(!definition.parameter(it.key()).name.isValid())
                problems.push_back(id + ": unsupported parameter " + it.key());
        std::vector<tp_pipeline::PortMapping> inputs,outputs;
        for(const auto& port : delegate->inPorts()) inputs.push_back({port.type,port.name,{},{}});
        for(const auto& port : delegate->outPorts())
            outputs.push_back({port.type,port.name,tp_pipeline::randomId(),{}});
        step->setInputMapping(inputs);
        step->setOutputMapping(outputs);
        steps.emplace(id,step.get());
        candidate->addStep(step.release());
    }
    for(const auto& edge : graph["connections"]) {
        const auto label=edge["id"].get<std::string>();
        const auto from=steps.find(edge["source"]["nodeId"].get<std::string>());
        const auto to=steps.find(edge["target"]["nodeId"].get<std::string>());
        if(from==steps.end() || to==steps.end()) { problems.push_back(label + ": unavailable endpoint node"); continue; }
        auto inputs=to->second->inputMapping();
        const auto& outputs=from->second->outputMapping();
        const auto input=std::find_if(inputs.begin(),inputs.end(),[&](const auto& p) { return p.portName.toString()==edge["target"]["portId"]; });
        const auto output=std::find_if(outputs.begin(),outputs.end(),[&](const auto& p) { return p.portName.toString()==edge["source"]["portId"]; });
        if(input==inputs.end() || output==outputs.end()) { problems.push_back(label + ": unavailable endpoint port"); continue; }
        if(input->dataName.isValid()) { problems.push_back(label + ": multiple producers for input"); continue; }
        if(input->portType!=output->portType) { problems.push_back(label + ": incompatible port types"); continue; }
        input->dataName=output->dataName;
        to->second->setInputMapping(inputs);
    }
    for(const auto* step : candidate->steps())
        for(const auto& input : step->inputMapping())
            if(!input.dataName.isValid()) problems.push_back(step->id().toString() + ": missing required input " + input.portName.toString());
    compiled=std::move(candidate);
    issues=std::move(problems);
}

const tp_pipeline::PipelineDetails& DocumentSession::executableGraph() const
{
    if(!issues.empty()) throw FileError("Graph cannot execute: " + issues.front());
    return *compiled;
}

std::unique_ptr<tp_pipeline::StepDetails> DocumentSession::inspectNode(const std::string& id) const
{
    const auto* node=compiled->findStepFromStepId(id);
    return node ? std::make_unique<tp_pipeline::StepDetails>(*node) : nullptr;
}

void DocumentSession::setParameter(const std::string& nodeId, const std::string& name, const Document& value)
{
    auto& nodes=source["project"]["graphs"][graphIndex]["nodes"];
    const auto node=std::find_if(nodes.begin(),nodes.end(),[&](const auto& item) { return item["id"]==nodeId; });
    if(node==nodes.end()) throw FileError("Node does not exist: " + nodeId);
    const auto* entry=registration(*node,registrations);
    if(!entry) throw FileError("Cannot edit an unavailable node type");
    auto definition=prototype(*delegates->stepDelegate(entry->type.toStdString()));
    auto parameter=definition.parameter(name);
    if(!parameter.name.isValid() || !decode(parameter,value)) throw FileError("Unsupported parameter edit: " + name);
    auto replacement=source;
    replacement["project"]["graphs"][graphIndex]["nodes"][size_t(std::distance(nodes.begin(),node))]["parameters"][name]=value;
    replace(std::move(replacement));
}

void DocumentSession::replace(Document replacement)
{
    // Prepare a complete replacement projection first for a strong exception
    // guarantee. Failed edits never change either the retained file or graph.
    DocumentSession candidate(std::move(replacement),source["project"]["graphs"][graphIndex]["id"],delegates,registrations);
    source.swap(candidate.source);
    compiled.swap(candidate.compiled);
    issues.swap(candidate.issues);
}

void DocumentSession::createNode(const std::string& nodeId, const std::string& registeredType)
{
    const auto entry=std::find_if(registrations.begin(),registrations.end(),[&](const auto& item) {
        return item.type.toStdString()==registeredType;
    });
    if(entry==registrations.end()) throw FileError("Unavailable node type: " + registeredType);
    auto definition=prototype(*delegates->stepDelegate(registeredType));
    auto parameters=Document::object();
    for(const auto& [name,parameter] : definition.parameters()) parameters[name.toString()]=encode(parameter);
    auto replacement=source;
    replacement["project"]["graphs"][graphIndex]["nodes"].push_back({
        {"id",nodeId},{"packageId",entry->packageId.toStdString()},
        {"typeId",entry->typeId.toStdString()},{"version",entry->contractVersion},{"parameters",parameters}});
    replace(std::move(replacement));
}

void DocumentSession::removeNode(const std::string& nodeId)
{
    auto replacement=source;
    auto& graph=replacement["project"]["graphs"][graphIndex];
    auto& nodes=graph["nodes"];
    const auto node=findId(nodes,nodeId);
    if(node==nodes.cend()) throw FileError("Node does not exist: " + nodeId);
    nodes.erase(node);
    auto& edges=graph["connections"];
    for(auto edge=edges.begin(); edge!=edges.end(); ) {
        if((*edge)["source"]["nodeId"]==nodeId || (*edge)["target"]["nodeId"]==nodeId) edge=edges.erase(edge);
        else ++edge;
    }
    replace(std::move(replacement));
}

void DocumentSession::connect(const std::string& connectionId,
    const std::string& sourceNode, const std::string& sourcePort,
    const std::string& targetNode, const std::string& targetPort)
{
    const auto& graph=source["project"]["graphs"][graphIndex];
    const auto& nodes=graph["nodes"];
    const auto from=findId(nodes,sourceNode), to=findId(nodes,targetNode);
    if(from==nodes.cend() || to==nodes.cend()) throw FileError("Connection endpoint node does not exist");
    const auto* fromType=registration(*from,registrations);
    const auto* toType=registration(*to,registrations);
    if(!fromType || !toType) throw FileError("Cannot connect an unavailable node type");
    const auto& outputs=delegates->stepDelegate(fromType->type.toStdString())->outPorts();
    const auto& inputs=delegates->stepDelegate(toType->type.toStdString())->inPorts();
    const auto output=std::find_if(outputs.begin(),outputs.end(),[&](const auto& p) { return p.name.toString()==sourcePort; });
    const auto input=std::find_if(inputs.begin(),inputs.end(),[&](const auto& p) { return p.name.toString()==targetPort; });
    if(output==outputs.end() || input==inputs.end()) throw FileError("Connection endpoint port does not exist");
    if(output->type!=input->type) throw FileError("Incompatible connection port types");
    for(const auto& edge : graph["connections"])
        if(edge["target"]["nodeId"]==targetNode && edge["target"]["portId"]==targetPort)
            throw FileError("Connection input already has a producer");
    auto replacement=source;
    replacement["project"]["graphs"][graphIndex]["connections"].push_back({
        {"id",connectionId},{"source",{{"nodeId",sourceNode},{"portId",sourcePort}}},
        {"target",{{"nodeId",targetNode},{"portId",targetPort}}}});
    replace(std::move(replacement));
}

void DocumentSession::disconnect(const std::string& connectionId)
{
    auto replacement=source;
    auto& edges=replacement["project"]["graphs"][graphIndex]["connections"];
    const auto edge=findId(edges,connectionId);
    if(edge==edges.cend()) throw FileError("Connection does not exist: " + connectionId);
    edges.erase(edge);
    replace(std::move(replacement));
}

void DocumentSession::setWorkspaceField(const std::string& name, const Document& value)
{
    if(QString::fromStdString(name).trimmed().isEmpty()) throw FileError("Workspace field name must not be empty");
    auto replacement=source;
    replacement["workspace"][name]=value;
    validate(replacement);
    source.swap(replacement);
}

ComponentBindings DocumentSession::instantiateComponent(const GraphComponent& component, const std::string& instanceId,
    const Document& controls, const Document& inputSources)
{
    const auto graphId=selectedGraph()["id"].get<std::string>();
    auto expanded=component.instantiate(source,graphId,instanceId,controls,inputSources);
    DocumentSession candidate(std::move(expanded.document),graphId,delegates,registrations);
    // Existing unsupported content remains retained. Reject only new problems
    // introduced by this command, before changing the active document.
    const std::set<std::string> previous(issues.begin(),issues.end());
    for(const auto& issue : candidate.issues)
        if(!previous.count(issue)) throw FileError("Component cannot instantiate: "+issue);
    for(const auto& endpoint : expanded.bindings.outputs) {
        const auto id=endpoint["nodeId"].get<std::string>();
        const auto& nodes=candidate.selectedGraph()["nodes"];
        const auto found=findId(nodes,id);
        const auto* entry=found==nodes.end() ? nullptr : registration(*found,registrations);
        if(!entry) throw FileError("Component output node is unavailable");
        const auto& ports=delegates->stepDelegate(entry->type.toStdString())->outPorts();
        if(std::none_of(ports.begin(),ports.end(),[&](const auto& port) { return port.name.toString()==endpoint["portId"]; }))
            throw FileError("Component output port is unavailable");
    }
    source.swap(candidate.source); compiled.swap(candidate.compiled); issues.swap(candidate.issues);
    return expanded.bindings;
}
} // namespace smartflow::project
