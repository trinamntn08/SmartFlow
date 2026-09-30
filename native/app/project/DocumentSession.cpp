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
}

DocumentSession::DocumentSession(Document source, std::string graphId,
    std::shared_ptr<const tp_pipeline::StepDelegateMap> delegates,
    std::vector<NodePresentation> registrations)
    : source(std::move(source)), delegates(std::move(delegates)), registrations(std::move(registrations))
{
    validate(this->source);
    if(!this->delegates) throw FileError("Missing delegate registry");
    std::set<std::tuple<std::string,std::string,int>> identities;
    for(const auto& entry : this->registrations) {
        if(entry.packageId.trimmed().isEmpty() || entry.typeId.trimmed().isEmpty() || entry.contractVersion < 1 ||
           !this->delegates->stepDelegate(entry.type.toStdString()))
            throw FileError("Invalid native persistence registration");
        if(!identities.emplace(entry.packageId.toStdString(),entry.typeId.toStdString(),entry.contractVersion).second)
            throw FileError("Duplicate native persistence registration");
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
    // Prepare a complete replacement projection first for a strong exception
    // guarantee. Failed edits never change either the retained file or graph.
    DocumentSession candidate(std::move(replacement),source["project"]["graphs"][graphIndex]["id"],delegates,registrations);
    source.swap(candidate.source);
    compiled.swap(candidate.compiled);
    issues.swap(candidate.issues);
}
} // namespace smartflow::project
