#include "tp_pipeline/StepOutput.h"
#include "tp_pipeline/StepDetails.h"
#include "tp_pipeline/StepDelegate.h"

#include "tp_data/Collection.h"
#include "tp_data/AbstractMember.h"

#include "tp_utils/Progress.h"

namespace tp_pipeline
{

//##################################################################################################
struct StepOutput::Private
{
  const StepDelegate* stepDelegate;
  StepDetails* stepDetails;
  std::shared_ptr<tp_data::Collection> output{std::make_shared<tp_data::Collection>()};

  std::vector<std::shared_ptr<tp_data::AbstractMember>> unusedOutput;
  std::vector<tp_utils::StringID> populatedPorts;

  //################################################################################################
  Private(const StepDelegate* stepDelegate_, StepDetails* stepDetails_):
    stepDelegate(stepDelegate_),
    stepDetails(stepDetails_)
  {

  }
};

//##################################################################################################
StepOutput::StepOutput(const StepDelegate* stepDelegate, StepDetails* stepDetails):
  d(new Private(stepDelegate, stepDetails))
{

}

//##################################################################################################
StepOutput::~StepOutput()
{
  delete d;
}

//################################################################################################
bool StepOutput::addMembers(std::vector<tp_pipeline::StepDetails::OverrideOutput>& outputs, tp_utils::Progress* progress)
{
  bool ok=true;

  for(const auto& output : outputs)
    ok = addSharedMember(output.member->name(), output.member, progress) && ok;

  return ok;
}

//##################################################################################################
bool StepOutput::addSharedMember(const tp_utils::StringID& portName, const std::shared_ptr<tp_data::AbstractMember>& member, tp_utils::Progress* progress)
{
  bool added=false;
  TP_CLEANUP([&]
  {
    if(!added)
      d->unusedOutput.push_back(member);
  });

  const auto& outPorts = d->stepDelegate->outPorts();

  for(const auto& port : outPorts)
  {
    if(port.name == portName)
    {
      if(port.type != member->type())
      {
        progress->addError("Type of output data member does not match type expected by port.");
        progress->addError("Got: " + member->type().toString());
        progress->addError("Expected: " + port.type.toString());
        return false;
      }

      for(const auto& outputMapping : d->stepDetails->outputMapping())
      {
        if(outputMapping.portName == portName)
        {
          if(!outputMapping.dataName.isValid())
          {
            progress->addError("Invalid output mapping for port: " + portName.toString());
            return false;
          }

          if(d->output->member(outputMapping.dataName) != nullptr)
          {
            progress->addError("Output data already contains: " + outputMapping.dataName.toString());
            return false;
          }

          added=true;
          member->setName(outputMapping.dataName);
          d->output->addMember(member);
          return true;
        }
      }

      progress->addError("Failed to find output mapping for port: " + portName.toString());
      return false;
    }
  }

  progress->addError("Failed to find port: " + portName.toString());
  return false;
}

//##################################################################################################
const std::shared_ptr<tp_data::Collection>& StepOutput::output() const
{
  if(!d)
  {
    static const std::shared_ptr<tp_data::Collection> empty;
    return empty;
  }
  return d->output;
}

}
