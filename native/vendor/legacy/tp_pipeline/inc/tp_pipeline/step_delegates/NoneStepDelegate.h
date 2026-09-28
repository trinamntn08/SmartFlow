#pragma once

#include "tp_pipeline/StepDelegate.h"

namespace tp_pipeline
{

//##################################################################################################
class NoneStepDelegate: public tp_pipeline::StepDelegate
{
public:
  //################################################################################################
  NoneStepDelegate();

  //################################################################################################
  bool executeStep(StepContext* stepContext) const override;
};

}
