#include "tp_pipeline/step_delegates/NoneStepDelegate.h"
#include "tp_pipeline/StepDetails.h"

namespace tp_pipeline
{

//##################################################################################################
NoneStepDelegate::NoneStepDelegate():
  StepDelegate(noneSID(), {}, {}, {})
{

}

//##################################################################################################
bool NoneStepDelegate::executeStep(StepContext* stepContext) const
{
  TP_UNUSED(stepContext);
  return false;
}

}
