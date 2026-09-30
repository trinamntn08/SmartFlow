#include "GraphProject.h"
#include <tp_pipeline/StepDelegate.h>
#include <tp_data/members/NumberMember.h>
#include <QJsonArray>
#include <QJsonDocument>
#include <cmath>

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

GraphProject::GraphProject(std::shared_ptr<const StepDelegateMap> delegates,
                           std::vector<NodePresentation> presentations)
    : delegates(std::move(delegates)), presentations(std::move(presentations)) {}

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

StepDetails* GraphProject::step(const StringID& id) const
{
    return document.findStepFromStepId(id);
}

void GraphProject::modified()
{
    ++currentRevision;
    Q_EMIT changed();
}

StepDetails* GraphProject::create(const StringID& type)
{
    const auto* definition = delegates->stepDelegate(type);
    if(!definition) return nullptr;
    auto item = std::make_unique<StepDetails>(type);
    std::vector<StringID> parameters;
    // Defaults are initialized only for a brand-new node, never during load/edit.
    definition->fixupParameters(item.get(), parameters);
    item->setParametersOrder(parameters);
    std::vector<PortMapping> inputs, outputs;
    for(const auto& port : definition->inPorts())
        inputs.push_back({port.type, port.name, {}, {}});
    for(const auto& port : definition->outPorts())
        outputs.push_back({port.type, port.name, randomId(), {}});
    item->setInputMapping(inputs);
    item->setOutputMapping(outputs);
    auto* result = item.release();
    document.addStep(result);
    modified();
    return result;
}

void GraphProject::remove(const StringID& id)
{
    if(auto* item = step(id)) { document.deleteStep(item); modified(); }
}

bool GraphProject::setParameter(const StringID& id, const Parameter& parameter)
{
    auto* item = step(id);
    if(!item) return false;
    const auto previous = item->parameter(parameter.name);
    if(!previous.name.isValid() || previous.type != parameter.type ||
       previous.value.index() != parameter.value.index() || previous.value == parameter.value)
        return false;
    if(const auto* value = std::get_if<double>(&parameter.value)) {
        if(!std::isfinite(*value) || *value < tpGetVariantValue<double>(previous.min, -INFINITY) ||
           *value > tpGetVariantValue<double>(previous.max, INFINITY)) return false;
    }
    // Retain metadata and unknown parameters; a control only changes its value.
    item->setParameterValue(parameter.name, parameter.value);
    modified();
    return true;
}

void GraphProject::connectInput(const StringID& target, size_t input, const StringID& source, size_t output)
{
    auto* to = step(target);
    auto* from = step(source);
    if(!to || !from || input >= to->inputMapping().size() || output >= from->outputMapping().size()) return;
    auto mapping = to->inputMapping();
    mapping[input].dataName = from->outputMapping()[output].dataName;
    to->setInputMapping(mapping);
    modified();
}

void GraphProject::disconnectInput(const StringID& target, size_t input)
{
    auto* to = step(target);
    if(!to || input >= to->inputMapping().size()) return;
    auto mapping = to->inputMapping();
    mapping[input].dataName = {};
    to->setInputMapping(mapping);
    modified();
}

QJsonObject GraphProject::capture(const StringID& id) const
{
    tp_utils::JSON data;
    QJsonArray blobs;
    step(id)->saveBinary(data, [&](const std::string& blob) {
        blobs.append(QString::fromLatin1(QByteArray::fromStdString(blob).toBase64()));
        return uint64_t(blobs.size() - 1);
    });
    return {{"step", QJsonDocument::fromJson(QByteArray::fromStdString(data.dump())).object()}, {"blobs", blobs}};
}

StepDetails* GraphProject::restore(const QJsonObject& snapshot)
{
    const auto data = tp_utils::jsonFromString(QJsonDocument(snapshot["step"].toObject()).toJson(QJsonDocument::Compact).toStdString());
    std::vector<std::string> blobs;
    for(const auto& value : snapshot["blobs"].toArray())
        blobs.push_back(QByteArray::fromBase64(value.toString().toLatin1()).toStdString());
    auto item = std::make_unique<StepDetails>();
    item->loadBinary(data, blobs);
    if(step(item->id()) || !delegates->stepDelegate(item->delegateName())) return nullptr;
    auto* result = item.release();
    document.addStep(result);
    modified();
    return result;
}
} // namespace smartflow
