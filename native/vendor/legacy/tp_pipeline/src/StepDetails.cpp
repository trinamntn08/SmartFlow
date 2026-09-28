#include "tp_pipeline/StepDetails.h"
#include "tp_pipeline/PipelineDetails.h"

#include "tp_utils/JSONUtils.h"

#include <algorithm>

namespace tp_pipeline
{
//##################################################################################################
struct StepDetails::Private
{
  Q* q;
  PipelineDetails* parent;
  tp_utils::StringID delegateName;
  tp_utils::StringID id;

  std::unordered_map<tp_utils::StringID, Parameter> parameters;
  std::vector<tp_utils::StringID> validParams;
  std::vector<tp_utils::StringID> parametersOrder;

  //Used for fallback operation
  struct OperationParameterSet
  {
    std::unordered_map<tp_utils::StringID, Parameter> parameters;
    std::vector<tp_utils::StringID> order;
  };
  tp_utils::StringID activeParameterOperationName;
  std::unordered_map<tp_utils::StringID, OperationParameterSet> parametersForOperation;

  std::vector<PortMapping> inputMapping;
  std::vector<PortMapping> outputMapping;

  bool noExec{false};
  std::vector<StepDetails::OverrideOutput> overrideOutputs;

  std::pair<double, double> position{0.0, 0.0};

  //################################################################################################
  std::vector<tp_utils::StringID> sanitizeFallbackOperationNames(const tp_utils::StringID& primaryDelegateName,
                                                                 const std::vector<tp_utils::StringID>& names)
  {
    std::vector<tp_utils::StringID> result;
    result.reserve(names.size());

    for(const auto& name : names)
    {
      if(!name.isValid())
        continue;

      if(name == primaryDelegateName)
        continue;

      if(tpContains(result, name))
        continue;

      result.push_back(name);
    }

    return result;
  }

  //################################################################################################
  Private(Q* q_, const tp_utils::StringID& delegateName_):
    q(q_),
    parent(nullptr),
    delegateName(delegateName_),
    id(tp_utils::makeUUID())
  {

  }
};

//##################################################################################################
void PortMapping::saveState(tp_utils::JSON& j) const
{
  j["portType"] = portType.toString();
  j["portName"] = portName.toString();
  j["dataName"] = dataName.toString();
  if(!anchors.empty())
  {
    auto& jAnchors = j["anchors"];
    jAnchors = tp_utils::JSON::array();
    jAnchors.get_ptr<tp_utils::JSON::array_t*>()->reserve(anchors.size());

    for(const auto& point : anchors)
    {
      jAnchors.push_back({point.first, point.second});
    }
  }
}

//##################################################################################################
void PortMapping::loadState(const tp_utils::JSON& j)
{
  portType = TPJSONString(j, "portType");
  portName = TPJSONString(j, "portName");
  dataName = TPJSONString(j, "dataName");

  anchors.clear();
  if(auto i = j.find("anchors"); i != j.end() && i->is_array())
  {
    anchors.reserve(i->size());
    for(const auto& point : *i)
    {
      if(point.is_array() && point.size() >= 2)
      {
        anchors.emplace_back(point[0].get<float>(), point[1].get<float>());
      }
    }
  }
}

//##################################################################################################
StepDetails::StepDetails(const tp_utils::StringID& delegateName):
  d(new Private(this, delegateName))
{

}

//##################################################################################################
StepDetails::StepDetails(const StepDetails& other):
  d(new Private(this, other.d->delegateName)),
  complexObjectManager(other.complexObjectManager)
{
  d->id                          = other.d->id;
  d->parameters                  = other.d->parameters;
  d->parametersOrder             = other.d->parametersOrder;
  d->activeParameterOperationName       = other.d->activeParameterOperationName;
  d->parametersForOperation      = other.d->parametersForOperation;
  d->inputMapping                = other.d->inputMapping;
  d->outputMapping               = other.d->outputMapping;

  d->position                    = other.d->position;
  d->noExec                      = other.d->noExec;
  d->overrideOutputs             = other.d->overrideOutputs;
}

//##################################################################################################
StepDetails& StepDetails::operator=(const StepDetails& other)
{
  if(&other == this)
    return *this;

  d->delegateName               = other.d->delegateName;
  d->id                         = other.d->id;
  d->parameters                 = other.d->parameters;
  d->parametersOrder            = other.d->parametersOrder;
  d->activeParameterOperationName      = other.d->activeParameterOperationName;
  d->parametersForOperation     = other.d->parametersForOperation;
  d->inputMapping               = other.d->inputMapping;
  d->outputMapping              = other.d->outputMapping;

  d->position                   = other.d->position;
  d->noExec                     = other.d->noExec;
  d->overrideOutputs            = other.d->overrideOutputs;

  complexObjectManager          = other.complexObjectManager;

  return *this;
}

//##################################################################################################
StepDetails::~StepDetails()
{
  delete d;
}

//##################################################################################################
void StepDetails::setDelegateName(const tp_utils::StringID& delegateName)
{
  d->delegateName = delegateName;
  complexObjectManager.clearComplexObjects();
  d->parameters.clear();
  d->parametersOrder.clear();
  d->activeParameterOperationName = {};
  d->parametersForOperation.clear();
  invalidate();
}

//##################################################################################################
const tp_utils::StringID& StepDetails::delegateName()const
{
  return d->delegateName;
}

//##################################################################################################
std::vector<tp_utils::StringID> StepDetails::fallbackOperationNames()
{
  std::vector<tp_utils::StringID> names;
  for(const auto& name : parameterValue<std::vector<std::string>>(fallbackOperationsSID()))
    names.emplace_back(name);

  return  d->sanitizeFallbackOperationNames(d->delegateName, names);
}

//##################################################################################################
void StepDetails::setFallbackOperationNames(const std::vector<tp_utils::StringID>& fallbackOperationsNames)
{
  const auto sanitized = d->sanitizeFallbackOperationNames(d->delegateName, fallbackOperationsNames);

  std::vector<std::string> serialized;
  serialized.reserve(sanitized.size());
  for(const auto& name : sanitized)
    serialized.push_back(name.toString());

  if(parameterValue<std::vector<std::string>>(fallbackOperationsSID()) == serialized)
    return;

  if(serialized.empty())
  {
    removeParameter(fallbackOperationsSID());
  }
  else
  {
    auto p = parameter(fallbackOperationsSID());
    p.name = fallbackOperationsSID();
    p.type = stringSID();
    p.value = serialized;
    setParamerter(p);
  }
}

//##################################################################################################
const tp_utils::StringID& StepDetails::id() const
{
  return d->id;
}

//##################################################################################################
const std::pair<double, double>& StepDetails::position() const
{
  return d->position;
}

//##################################################################################################
void StepDetails::setPosition(const std::pair<double, double>& position)
{
  if(d->position == position)
    return;

  d->position = position;
  invalidate();
}

//##################################################################################################
PipelineDetails* StepDetails::parent()const
{
  return d->parent;
}

//##################################################################################################
void StepDetails::saveBinary(tp_utils::JSON& j, const std::function<uint64_t(const std::string&)>& addBlob) const noexcept
{
  j["delegateName"] = d->delegateName.toString();
  j["id"] = d->id.toString();

  {
    auto& jj = j["parameters"];
    jj = tp_utils::JSON::array();
    jj.get_ptr<tp_utils::JSON::array_t*>()->reserve(d->parameters.size());
    for(const auto& i : d->parameters)
    {
      jj.emplace_back();
      i.second.saveBinary(jj.back(), addBlob);
    }
  }

  if(!d->parametersForOperation.empty())
  {
    auto& jj = j["parametersForOperation"];
    jj = tp_utils::JSON::array();
    jj.get_ptr<tp_utils::JSON::array_t*>()->reserve(d->parametersForOperation.size());

    for(const auto& operationParameters : d->parametersForOperation)
    {
      auto& jOperation = jj.emplace_back();
      jOperation["operationName"] = operationParameters.first.toString();

      auto& jParameters = jOperation["parameters"];
      jParameters = tp_utils::JSON::array();
      jParameters.get_ptr<tp_utils::JSON::array_t*>()->reserve(operationParameters.second.parameters.size());

      for(const auto& parameter : operationParameters.second.parameters)
      {
        auto& jParameter = jParameters.emplace_back();
        parameter.second.saveBinary(jParameter, addBlob);
      }

      if(!operationParameters.second.order.empty())
        tp_utils::saveVectorOfStringIDsToJSON(jOperation["parameterOrder"], operationParameters.second.order);
    }
  }

  tp_utils::saveVectorOfStringIDsToJSON(j["parametersOrder"], d->parametersOrder);

  if(!complexObjectManager.isEmpty())
    complexObjectManager.saveBinary(j["complexObjectManager"], addBlob);

  tp_utils::saveVectorOfObjectsToJSON(j["inputMapping"], d->inputMapping);
  tp_utils::saveVectorOfObjectsToJSON(j["outputMapping"], d->outputMapping);

  j["position"] = tp_utils::JSON::array({d->position.first, d->position.second});
}

//##################################################################################################
void StepDetails::loadBinary(const tp_utils::JSON& j, const std::vector<std::string>& blobs) noexcept
{
  d->delegateName = TPJSONString(j, "delegateName", "None");
  d->activeParameterOperationName = {};

  d->id = TPJSONString(j, "id");
  if(!d->id.isValid())
    d->id = tp_utils::makeUUID();

  d->parameters.clear();
  d->parametersForOperation.clear();

  if(auto i = j.find("parameters"); i != j.end() && i->is_array())
  {
    for(const auto& jj : *i)
    {
      Parameter parameter(jj, blobs);
      if(parameter.name.isValid())
        d->parameters[parameter.name] = parameter;
    }
  }

  auto parseOperationParameters = [&](const tp_utils::JSON& jOperation)
  {
    tp_utils::StringID operationName = TPJSONString(jOperation, "operationName");

    if(!operationName.isValid())
      return;

    auto& operationParameters = d->parametersForOperation[operationName];
    operationParameters.parameters.clear();
    operationParameters.order.clear();

    if(auto p = jOperation.find("parameters"); p != jOperation.end() && p->is_array())
    {
      for(const auto& jParameter : *p)
      {
        Parameter parameter(jParameter, blobs);
        if(parameter.name.isValid())
          operationParameters.parameters[parameter.name] = parameter;
      }
    }

    tp_utils::loadVectorOfStringIDsFromJSON(jOperation, "parameterOrder", operationParameters.order);
  };

  if(auto i = j.find("parametersForOperation"); i != j.end() && i->is_array())
  {
    for(const auto& jOperation : *i)
      parseOperationParameters(jOperation);
  }

  tp_utils::loadVectorOfStringIDsFromJSON(j, "parametersOrder", d->parametersOrder);

  if(auto i=j.find("complexObjectManager"); i!=j.end())
    complexObjectManager.loadBinary(*i, blobs);
  else
    complexObjectManager.clearComplexObjects();

  tp_utils::loadVectorOfObjectsFromJSON(j, "inputMapping", d->inputMapping);
  tp_utils::loadVectorOfObjectsFromJSON(j, "outputMapping", d->outputMapping);

  for(auto& m : d->outputMapping)
    if(!m.dataName.isValid())
      m.dataName = randomId();

  if(auto i = j.find("position");
     i != j.end() &&
     i->is_array() &&
     i->size() == 2 &&
     i->at(0).is_number()&&
     i->at(1).is_number())
    d->position = {i->at(0).get<double>(), i->at(1).get<double>()};
  else
    d->position = {0.0, 0.0};

  invalidate();
}

//##################################################################################################
const std::unordered_map<tp_utils::StringID, Parameter>& StepDetails::parameters()
{
  return d->parameters;
}

//##################################################################################################
void StepDetails::setParamerter(const Parameter& parameter)
{
  if(const auto& operationName = d->activeParameterOperationName; operationName.isValid() && parameter.name != fallbackOperationsSID())
  {
    setParameterForOperation(operationName, parameter);
    return;
  }

  d->parameters[parameter.name] = parameter;
  d->validParams.clear();
  invalidate();
}

//##################################################################################################
void StepDetails::setActiveOperationName(const tp_utils::StringID& operationName)
{
  d->activeParameterOperationName = operationName;
}

//##################################################################################################
const tp_utils::StringID& StepDetails::activeOperationName() const
{
  return d->activeParameterOperationName;
}

//##################################################################################################
const std::unordered_map<tp_utils::StringID, Parameter>* StepDetails::parametersForOperation(const tp_utils::StringID& operationName) const
{
  auto i = d->parametersForOperation.find(operationName);
  return (i == d->parametersForOperation.end()) ? nullptr : &i->second.parameters;
}

//##################################################################################################
const std::vector<tp_utils::StringID>* StepDetails::parameterOrderForOperation(const tp_utils::StringID& operationName) const
{
  auto i = d->parametersForOperation.find(operationName);
  return (i == d->parametersForOperation.end()) ? nullptr : &i->second.order;
}

//##################################################################################################
void StepDetails::setParameterForOperation(const tp_utils::StringID& operationName, const Parameter& parameter)
{
  if(!operationName.isValid() || !parameter.name.isValid())
    return;

  auto& operationParameters = d->parametersForOperation[operationName];
  operationParameters.parameters[parameter.name] = parameter;

  if(!tpContains(operationParameters.order, parameter.name))
    operationParameters.order.push_back(parameter.name);

  invalidate();
}

//##################################################################################################
void StepDetails::removeParameterForOperation(const tp_utils::StringID& operationName, const tp_utils::StringID& parameterName)
{
  auto delegateIt = d->parametersForOperation.find(operationName);
  if(delegateIt == d->parametersForOperation.end())
    return;

  auto& operationParameters = delegateIt->second;
  auto parameterIt = operationParameters.parameters.find(parameterName);
  if(parameterIt == operationParameters.parameters.end())
    return;

  operationParameters.parameters.erase(parameterIt);

  auto i = std::find(operationParameters.order.begin(), operationParameters.order.end(), parameterName);
  if(i != operationParameters.order.end())
    operationParameters.order.erase(i);

  if(operationParameters.parameters.empty())
    d->parametersForOperation.erase(delegateIt);

  invalidate();
}

//##################################################################################################
std::vector<tp_utils::StringID> StepDetails::orderedParameterNames() const
{
  std::vector<tp_utils::StringID> names = d->parametersOrder;
  names.reserve(names.size() + d->parameters.size());

  for(const auto& i : d->parameters)
  {
    if(!tpContains(names, i.first))
      names.push_back(i.first);
  }

  return names;
}

//##################################################################################################
std::vector<tp_utils::StringID> StepDetails::parameterNamesForOperation(const tp_utils::StringID& operationName) const
{
  std::vector<tp_utils::StringID> names;
  const auto* operationParameters = parametersForOperation(operationName);
  if(!operationParameters)
    return names;

  names.reserve(operationParameters->size());
  for(const auto& i : *operationParameters)
    names.push_back(i.first);

  return names;
}

//##################################################################################################
StepDetails StepDetails::detailsForOperationParameters(const tp_utils::StringID& operationName) const
{
  StepDetails operationDetails(operationName);

  const auto* operationParameters = parametersForOperation(operationName);
  if(!operationParameters)
    return operationDetails;

  const auto* order = parameterOrderForOperation(operationName);
  if(order)
  {
    for(const auto& name : *order)
    {
      auto i = operationParameters->find(name);
      if(i == operationParameters->end())
        continue;

      operationDetails.setParamerter(i->second);
    }
  }

  for(const auto& i : *operationParameters)
  {
    if(order && tpContains(*order, i.first))
      continue;

    operationDetails.setParamerter(i.second);
  }

  return operationDetails;
}

//##################################################################################################
void StepDetails::syncParametersForOperation(const tp_utils::StringID& operationName, StepDetails* sourceDetails)
{
  if(!operationName.isValid() || !sourceDetails)
    return;

  const auto existingNames = parameterNamesForOperation(operationName);
  std::vector<tp_utils::StringID> producedNames;

  for(const auto& name : sourceDetails->orderedParameterNames())
  {
    if(!name.isValid() || name == fallbackOperationsSID())
      continue;

    auto parameter = sourceDetails->parameter(name);
    if(!parameter.name.isValid())
      continue;

    parameter.name = name;
    producedNames.push_back(parameter.name);
    setParameterForOperation(operationName, parameter);
  }

  for(const auto& name : existingNames)
  {
    if(!tpContains(producedNames, name))
      removeParameterForOperation(operationName, name);
  }
}

//##################################################################################################
void StepDetails::setParameterValue(const tp_utils::StringID& name, const Variant& value)
{
  if(const auto& scope = d->activeParameterOperationName; scope.isValid() && name != fallbackOperationsSID())
  {
    auto& operationParameters = d->parametersForOperation[scope];
    auto& parameter = operationParameters.parameters[name];
    parameter.name = name;
    parameter.value = value;

    if(!tpContains(operationParameters.order, name))
      operationParameters.order.push_back(name);

    invalidate();
    return;
  }

  d->parameters[name].value = value;
  invalidate();
}

//##################################################################################################
void StepDetails::removeParameter(const tp_utils::StringID& name)
{
  if(const auto& scope = d->activeParameterOperationName; scope.isValid() && name != fallbackOperationsSID())
  {
    removeParameterForOperation(scope, name);
    return;
  }

  auto i = d->parameters.find(name);
  if(i!=d->parameters.end())
    d->parameters.erase(i);
  d->validParams.clear();
  invalidate();
}

//##################################################################################################
const std::vector<tp_utils::StringID>& StepDetails::parametersOrder()const
{
  return d->parametersOrder;
}

//##################################################################################################
void StepDetails::setParametersOrder(const std::vector<tp_utils::StringID>& parametersOrder)
{
  if(d->parametersOrder != parametersOrder)
  {
    d->parametersOrder = parametersOrder;
    invalidate();
  }
}

//##################################################################################################
void StepDetails::setValidParameters(const std::vector<tp_utils::StringID>& validParams)
{
  if(d->validParams != validParams)
  {
    for (auto i = d->parameters.begin(); i != d->parameters.end();)
    {
      if(i->first != fallbackOperationsSID() && !tpContains(validParams, i->first))
        i = d->parameters.erase(i);
      else
        ++i;
    }
  }
}

//##################################################################################################
void StepDetails::setInputMapping(const std::vector<PortMapping>& inputMapping)
{
  if(d->inputMapping == inputMapping)
    return;

  d->inputMapping = inputMapping;
  invalidate();
}

//##################################################################################################
const std::vector<PortMapping>& StepDetails::inputMapping() const
{
  return d->inputMapping;
}

//##################################################################################################
void StepDetails::setOutputMapping(const std::vector<PortMapping>& outputMapping)
{
  if(d->outputMapping == outputMapping)
    return;

  d->outputMapping = outputMapping;
  invalidate();
}

//##################################################################################################
const std::vector<PortMapping>& StepDetails::outputMapping() const
{
  return d->outputMapping;
}

//##################################################################################################
void StepDetails::invalidate()
{
  if(d->parent)
    d->parent->invalidateStep(this);
}

//##################################################################################################
void StepDetails::setNoExec(bool noExec)
{
  d->noExec = noExec;
  invalidate();
}

//##################################################################################################
bool StepDetails::noExec() const
{
  return d->noExec;
}

//##################################################################################################
void StepDetails::setOverrideOutputs(const std::vector<StepDetails::OverrideOutput>& overrideOutputs)
{
  d->overrideOutputs = overrideOutputs;
  invalidate();
}

//##################################################################################################
const std::vector<StepDetails::OverrideOutput>& StepDetails::overrideOutputs() const
{
  return d->overrideOutputs;
}

//##################################################################################################
void StepDetails::setParent(PipelineDetails* parent)
{
  d->parent = parent;
}

//##################################################################################################
void StepDetails::updateInputIds(const std::unordered_map<tp_utils::StringID, tp_utils::StringID>& idMap)
{
  bool changed = false;
  for(auto& port : d->inputMapping)
  {
    if(port.dataName.isValid())
    {
      auto i = idMap.find(port.dataName);
      if(i != idMap.end())
      {
        port.dataName = i->second;
        changed = true;
      }
    }
  }

  if(changed)
    invalidate();
}

//##################################################################################################
void StepDetails::generateNewOutputIds(std::unordered_map<tp_utils::StringID, tp_utils::StringID>& idMap)
{
  bool changed = false;
  for(auto& port : d->outputMapping)
  {
    if(port.dataName.isValid())
    {
      tp_utils::StringID newId = tp_utils::makeUUID();
      idMap[port.dataName] = newId;
      port.dataName = newId;
      changed = true;
    }
  }

  if(changed)
    invalidate();
}

}
