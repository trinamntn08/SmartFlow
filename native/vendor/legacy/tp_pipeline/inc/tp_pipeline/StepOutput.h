#pragma once

#include "tp_pipeline/StepDetails.h"

#include <memory>

namespace tp_utils
{
class Progress;
}

namespace tp_data
{
class Collection;
class AbstractMember;
}

namespace tp_pipeline
{
class StepDelegate;

//##################################################################################################
class TP_PIPELINE_SHARED_EXPORT StepOutput
{
  TP_NONCOPYABLE(StepOutput);
  TP_DQ;
public:
  //################################################################################################
  StepOutput(const StepDelegate* stepDelegate, StepDetails* stepDetails);

  //################################################################################################
  ~StepOutput();

  //################################################################################################
  template<typename T>
  bool addMember(const tp_utils::StringID& portName, T* member, tp_utils::Progress* progress)
  {
    return addSharedMember(portName, std::shared_ptr<tp_data::AbstractMember>(member), progress);
  }

  //################################################################################################
  bool addMembers(std::vector<tp_pipeline::StepDetails::OverrideOutput>& outputs, tp_utils::Progress* progress);

  //################################################################################################
  bool addSharedMember(const tp_utils::StringID& portName, const std::shared_ptr<tp_data::AbstractMember>& member, tp_utils::Progress* progress);

  //################################################################################################
  const std::shared_ptr<tp_data::Collection>& output() const;
};

}
