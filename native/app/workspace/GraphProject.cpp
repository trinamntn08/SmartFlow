#include "GraphProject.h"
#include <tp_pipeline/StepDelegate.h>
#include <tp_data/members/NumberMember.h>

namespace smartflow {
using namespace tp_pipeline;
using tp_utils::StringID;

namespace {
class NumericDelegate final : public StepDelegate {
public:
    explicit NumericDelegate(bool add)
        : StepDelegate(add ? "smartflow.numeric.add@1" : "smartflow.numeric.number@1", {},
                       add ? std::vector<PortDetails>{{"in", tp_data::doubleSID()}} : std::vector<PortDetails>{},
                       {{"out", tp_data::doubleSID()}}) {}
    void fixupParameters(StepDetails* step, std::vector<StringID>& names) const override
    {
        Parameter p;
        p.name = "value";
        p.type = doubleSID();
        p.value = 1.0;
        p.min = -1000000.0;
        p.max = 1000000.0;
        p.step = 1.0;
        step->setParamerter(p);
        names.push_back(p.name);
    }
    bool executeStep(StepContext* context) const override
    {
        double value = context->stepDetails->parameterValue<double>("value");
        if(!inPorts().empty()) {
            auto* input = context->memberCast<tp_data::DoubleMember>("in");
            if(!input) return false;
            value += input->data;
        }
        auto output = std::make_shared<tp_data::DoubleMember>();
        output->data = value;
        return context->stepOutput->addSharedMember("out", output, context->progress);
    }
};
}

std::shared_ptr<StepDelegateMap> numericDelegates()
{
    auto registry = std::make_shared<StepDelegateMap>();
    registry->addStepDelegate(new NumericDelegate(false));
    registry->addStepDelegate(new NumericDelegate(true));
    return registry;
}

QString nodeTitle(const StringID& type)
{
    if(type == "smartflow.numeric.number@1") return "Number";
    if(type == "smartflow.numeric.add@1") return "Add";
    return QString::fromStdString(type.toString());
}

std::vector<NodePresentation> numericPresentations()
{
    return {{"smartflow.numeric.number@1","Number","Numeric","smartflow.numeric","number",1},
            {"smartflow.numeric.add@1","Add","Numeric","smartflow.numeric","add",1}};
}
GraphProject::GraphProject(std::shared_ptr<const StepDelegateMap> delegates,
                           std::vector<NodePresentation> presentations)
    : delegates(std::move(delegates)), presentations(std::move(presentations)),
      history(project::DocumentSession::empty(randomId().toString(),"graph",this->delegates,this->presentations).document(),
              "graph",this->delegates,this->presentations)
{
    QObject::connect(&history,&project::DocumentHistory::changed,this,[this] {
        refreshInspection();
        Q_EMIT changed();
    });
}
QString GraphProject::title(const StringID& type) const
{
    for(const auto& item : presentations)
        if(item.type.toStdString() == type.toString()) return item.title;
    return nodeTitle(type);
}
QString GraphProject::category(const StringID& type) const
{
    for(const auto& item : presentations)
        if(item.type.toStdString() == type.toString()) return item.category;
    return "Nodes";
}
void GraphProject::refreshInspection()
{
    inspection.clear();
    for(const auto& node : selectedGraph()["nodes"]) {
        const auto id=node["id"].get<std::string>();
        if(auto copy=history.session().inspectNode(id)) inspection.emplace(id,std::move(copy));
    }
}
StepDetails* GraphProject::step(const StringID& id) const
{
    const auto found=inspection.find(id.toString());
    return found==inspection.end() ? nullptr : found->second.get();
}
StepDetails* GraphProject::create(const StringID& type)
{
    const auto id=randomId();
    try { history.createNode(id.toString(),type.toString()); }
    catch(const project::FileError&) { return nullptr; }
    return step(id);
}
void GraphProject::remove(const StringID& id)
{
    history.removeNode(id.toString());
}
bool GraphProject::setParameter(const StringID& id, const Parameter& parameter)
{
    auto* item=step(id);
    if(!item) return false;
    const auto previous=item->parameter(parameter.name);
    if(!previous.name.isValid() || previous.type!=parameter.type ||
       previous.value.index()!=parameter.value.index() || previous.value==parameter.value) return false;
    project::Document value;
    if(const auto* number=std::get_if<double>(&parameter.value)) value=*number;
    else if(const auto* text=std::get_if<std::string>(&parameter.value)) value=*text;
    else if(const auto* boolean=std::get_if<bool>(&parameter.value)) value=*boolean;
    else return false;
    const auto nodeId=id.toString(), name=parameter.name.toString();
    try { history.setParameter(nodeId,name,value); }
    catch(const project::FileError&) { return false; }
    return true;
}
bool GraphProject::connectInput(const StringID& target, size_t input, const StringID& source, size_t output)
{
    auto* to=step(target);
    auto* from=step(source);
    if(!to || !from || input>=to->inputMapping().size() || output>=from->outputMapping().size()) return false;
    const auto targetId=target.toString(), sourceId=source.toString();
    const auto in=to->inputMapping()[input].portName.toString(), out=from->outputMapping()[output].portName.toString();
    try { history.connect(randomId().toString(),sourceId,out,targetId,in); }
    catch(const project::FileError&) { return false; }
    return true;
}
std::string GraphProject::connectionId(const StringID& target, size_t input) const
{
    const auto* to=step(target);
    if(!to || input>=to->inputMapping().size()) return {};
    for(const auto& edge : selectedGraph()["connections"])
        if(edge["target"]["nodeId"]==target.toString() &&
           edge["target"]["portId"]==to->inputMapping()[input].portName.toString())
            return edge["id"].get<std::string>();
    return {};
}
void GraphProject::disconnectInput(const StringID& target, size_t input)
{
    const auto id=connectionId(target,input);
    if(!id.empty()) history.disconnect(id);
}
} // namespace smartflow
