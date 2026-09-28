#include "tp_pipeline/StepDelegate.h"
#include "tp_pipeline/StepDetails.h"
#include "tp_pipeline/StepContext.h"
#include "tp_pipeline/StepOutput.h"

namespace tp_pipeline
{

//##################################################################################################
std::vector<tp_pipeline::PortDetails> joinPorts(std::vector<tp_pipeline::PortDetails> ports,
                                                const std::vector<tp_pipeline::PortDetails>& additionalInPorts)
{
  ports.insert(ports.end(), additionalInPorts.begin(), additionalInPorts.end());
  return ports;
}

//##################################################################################################
bool delegatesInSameGroup(const StepDelegate* a, const StepDelegate* b)
{
  if(!a || !b)
    return false;

  for(const auto& group : a->groups())
    if(tpContains(b->groups(), group))
      return true;

  return false;
}

//##################################################################################################
struct StepDelegate::Private
{
  const tp_utils::StringID name;
  const std::vector<tp_utils::StringID> groups;

  const std::vector<PortDetails> inPorts;
  const std::vector<PortDetails> outPorts;

  //################################################################################################
  Private(const tp_utils::StringID& name_,
          const std::vector<tp_utils::StringID>& groups_,
          const std::vector<PortDetails>& inPorts_,
          const std::vector<PortDetails>& outPorts_):
    name(name_),
    groups(groups_),
    inPorts(inPorts_),
    outPorts(outPorts_)
  {

  }
};

//##################################################################################################
StepDelegate::StepDelegate(const tp_utils::StringID& name,
                                           const std::vector<tp_utils::StringID>& groups,
                                           const std::vector<PortDetails>& inPorts,
                                           const std::vector<PortDetails>& outPorts):
  d(new Private(name, groups, inPorts, outPorts))
{

}

//##################################################################################################
StepDelegate::~StepDelegate()
{
  delete d;
}

//##################################################################################################
const tp_utils::StringID& StepDelegate::name()const
{
  return d->name;
}

//##################################################################################################
const std::vector<tp_utils::StringID>& StepDelegate::groups()const
{
  return d->groups;
}

//##################################################################################################
const std::vector<PortDetails>& StepDelegate::inPorts() const
{
  return d->inPorts;
}

//##################################################################################################
const std::vector<PortDetails>& StepDelegate::outPorts() const
{
  return d->outPorts;
}

//##################################################################################################
void StepDelegate::fixupParameters(tp_pipeline::StepDetails* stepDetails, std::vector<tp_utils::StringID>& validParams) const
{
  TP_UNUSED(stepDetails);
  TP_UNUSED(validParams);
}

//##################################################################################################
std::string StepDelegate::description() const
{
  return {};
}

//##################################################################################################
std::string StepDelegate::url() const
{
  return {};
}

}
