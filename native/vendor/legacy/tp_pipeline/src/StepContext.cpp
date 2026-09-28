#include "tp_pipeline/StepContext.h"
#include "tp_pipeline/StepDetails.h"
#include "tp_pipeline/StepDelegate.h"

#include "tp_data/Collection.h"
#include "tp_data/CollectionFactory.h"

#include "tp_utils/Progress.h"

namespace tp_pipeline
{

//##################################################################################################
bool StepContext::dependenciesMet() const
{
  for(auto s : dependsOn)
    if(!s->runComplete)
      return false;

  return true;
}

//##################################################################################################
const tp_utils::StringID& StepContext::inputDataNameFromPortName(const tp_utils::StringID& portName) const
{
  static const tp_utils::StringID n;
  for(const auto& m : stepDetails->inputMapping())
    if(m.portName == portName)
      return m.dataName;
  return n;
}

//##################################################################################################
std::shared_ptr<tp_data::AbstractMember> StepContext::member(const tp_utils::StringID& portName) const
{
  auto name = inputDataNameFromPortName(portName);
  if(!name.isValid())
    return {};
  return stepInput->member(name);
}

//##################################################################################################
bool StepContext::copyIntoCollection(const tp_data::CollectionFactory* collectionFactory,
                                     tp_data::Collection& collection,
                                     tp_utils::Progress* progress) const
{
  if(!stepDelegate)
  {
    progress->addError("Delegate not set in copyIntoCollection.");
    return false;
  }

  bool ok=true;

  for(const auto& inPort : stepDelegate->inPorts())
  {
    auto member = this->member(inPort.name);
    if(!member)
    {
      progress->addError("Failed to load member: " + inPort.name.toString());
      ok=false;
      continue;
    }

    std::string error;
    auto newMember = collectionFactory->clone(error, *member);

    if(!error.empty())
    {
      progress->addError("Error cloning member: " + inPort.name.toString() + ", Error: " + error);
      ok=false;
      continue;
    }

    if(!newMember)
    {
      progress->addError("Failed to clone member: " + inPort.name.toString());
      ok=false;
      continue;
    }

    newMember->setName(inPort.name);
    collection.addMember(newMember);
  }

  return ok;
}

}
