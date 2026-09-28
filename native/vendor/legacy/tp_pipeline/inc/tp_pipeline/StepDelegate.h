#pragma once

#include "tp_pipeline/Globals.h"
#include "tp_pipeline/StepContext.h"
#include "tp_pipeline/StepOutput.h"

namespace tp_data
{
class Collection;
}

namespace tp_pipeline
{
class StepDetails;
class StepDelegate;

//##################################################################################################
struct PortDetails
{
  tp_utils::StringID name;
  tp_utils::StringID type;

  // This was added
  PortDetails() = default;
  PortDetails(const tp_utils::StringID& name_, const tp_utils::StringID& type_)
      : name(name_), type(type_) {}
};

//##################################################################################################
std::vector<tp_pipeline::PortDetails> joinPorts(std::vector<tp_pipeline::PortDetails> ports,
                                                const std::vector<tp_pipeline::PortDetails>& additionalInPorts);

//##################################################################################################
//! Returns true if the delegates share at least one group.
TP_PIPELINE_SHARED_EXPORT bool delegatesInSameGroup(const StepDelegate* a, const StepDelegate* b);

//##################################################################################################
class TP_PIPELINE_SHARED_EXPORT StepDelegate
{
  TP_DQ;
public:
  //################################################################################################
  StepDelegate(const tp_utils::StringID& name,
               const std::vector<tp_utils::StringID>& groups,
               const std::vector<PortDetails>& inPorts,
               const std::vector<PortDetails>& outPorts);

  //################################################################################################
  virtual ~StepDelegate();

  //################################################################################################
  const tp_utils::StringID& name() const;

  //################################################################################################
  const std::vector<tp_utils::StringID>& groups() const;

  //################################################################################################
  const std::vector<PortDetails>& inPorts() const;

  //################################################################################################
  const std::vector<PortDetails>& outPorts() const;

  //################################################################################################
  //! Execute this step in the pipeline.
  /*!
  This method should be reimplemented to perform the work of the step. The stepContext contains the
  context for the step.

  \param stepContext the context of the step.
  */
  virtual bool executeStep(StepContext* stepContext) const=0;

  //################################################################################################
  //! This gets called to add, adapt, and validate step parameters
  /*!
  This should add any parameters to the step that are missing, remove any that are not needed, and
  alter any that are not valid. This will be called on the blan step to add the default set of
  parameters, it will then be called each time the step is altered to make sure that it has a valid
  set of parameters.
  \param stepDetails
  */
  //################################################################################################
  virtual void fixupParameters(tp_pipeline::StepDetails* stepDetails, std::vector<tp_utils::StringID>& validParams) const;

  //################################################################################################
  virtual std::string description() const;

  //################################################################################################
  virtual std::string url() const;

  //################################################################################################
  template<typename T>
  T* readOutput(const StepContext* stepContext,const tp_utils::StringID& dataName) const
  {
    return stepContext->stepOutput->output()->memberCast<T>(dataName);
  }
};

}
