#include "tp_pipeline/StepDelegateMap.h"
#include "tp_pipeline/StepDelegate.h"

namespace tp_pipeline
{

//##################################################################################################
StepDelegateMap::StepDelegateMap() = default;

//##################################################################################################
StepDelegateMap::~StepDelegateMap()
{
  for(const auto& i : m_stepDelegates)
    delete i.second;
}

//##################################################################################################
void StepDelegateMap::addStepDelegate(tp_pipeline::StepDelegate* stepDelegate)
{
  auto i = m_stepDelegates.find(stepDelegate->name());
  if(i != m_stepDelegates.end())
  {
    delete i->second;
    m_stepDelegates.erase(i);
  }
  m_stepDelegates[stepDelegate->name()] = stepDelegate;
}

//##################################################################################################
const tp_pipeline::StepDelegate* StepDelegateMap::stepDelegate(const tp_utils::StringID& name) const
{
  return tpGetMapValue(m_stepDelegates, name, nullptr);
}

//##################################################################################################
std::vector<tp_utils::StringID> StepDelegateMap::stepDelegateNames() const
{
  std::vector<tp_utils::StringID> stepDelegateNames;
  for(const auto& i : m_stepDelegates)
    stepDelegateNames.push_back(i.first);
  return stepDelegateNames;
}

//##################################################################################################
const std::unordered_map<tp_utils::StringID, StepDelegate*>& StepDelegateMap::stepDelegates() const
{
  return m_stepDelegates;
}

}
