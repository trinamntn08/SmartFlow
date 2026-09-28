#pragma once

#include "tp_pipeline/Globals.h"
#include "tp_pipeline/Parameter.h"
#include "tp_pipeline/ComplexObjectManager.h"

#include "tp_data/AbstractMember.h"

#include "tp_utils/RefCount.h"

#include <unordered_map>

namespace tp_pipeline
{
class PipelineDetails;

//##################################################################################################
struct TP_PIPELINE_SHARED_EXPORT PortMapping
{
  tp_utils::StringID portType;
  tp_utils::StringID portName; //!< The step delegate give each output a name (unique within that delegate)
  tp_utils::StringID dataName; //!< In the output collection we assign that data a unique name.

  std::vector<std::pair<float, float>> anchors; //! Stores the visual routing points for the connection between two nodes

  //################################################################################################
  void saveState(tp_utils::JSON& j) const;

  //################################################################################################
  void loadState(const tp_utils::JSON& j);

  //################################################################################################
  bool operator==(const PortMapping& other) const
  {
    return portType == other.portType &&
           portName == other.portName &&
           dataName == other.dataName &&
           anchors  == other.anchors;
  }

  //################################################################################################
  bool operator!=(const PortMapping& other) const
  {
    return !(*this == other);
  }
};

//##################################################################################################
//! The configuration for a single step in an image processing pipeline
/*!
An image processing pipeline is made up of multiple steps, the initial input is passed into the
first step then the output of each step is passed into the next step, finally the output of the last
step is returned as the result of the pipeline.

The StepDetails hold the configuration for one step, this includes the name of the delegate that
will be used to execute the step.
 */
class TP_PIPELINE_SHARED_EXPORT StepDetails
{
  friend class PipelineDetails;
  TP_REF_COUNT_OBJECTS("StepDetails");
  TP_DQ;
public:
  //################################################################################################
  /*!
  \param delegateName - The name of the delegate to use for executing this step.
  */
  StepDetails(const tp_utils::StringID& delegateName=tp_utils::StringID());

  //################################################################################################
  StepDetails(const StepDetails& other);

  //################################################################################################
  StepDetails& operator=(const StepDetails& other);

  //################################################################################################
  ~StepDetails();

  //################################################################################################
  //! Set the name of the delegate used to execute this step
  /*!
  This sets the name that will be used to select the delegate that will be used to execute this
  step.

  \param delegateName - The name of the delegate to use for executing this step.
  */
  void setDelegateName(const tp_utils::StringID& delegateName);

  //################################################################################################
  //! Returns the name of the delegate that will be used to execute the step
  const tp_utils::StringID& delegateName()const;

  //################################################################################################
  //! Ordered fallback operations to run if this step fails.
  std::vector<tp_utils::StringID> fallbackOperationNames();

  //################################################################################################
  //! Set ordered fallback operations.
  void setFallbackOperationNames(const std::vector<tp_utils::StringID>& fallbackOperationsNames);

  //################################################################################################
  const tp_utils::StringID& id() const;

  //################################################################################################
  const std::pair<double, double>& position() const;

  //################################################################################################
  void setPosition(const std::pair<double, double>& position);

  //################################################################################################
  //! Returns the pipeline that this step is part of or nullptr.
  PipelineDetails* parent()const;

  //################################################################################################
  void updateInputIds(const std::unordered_map<tp_utils::StringID, tp_utils::StringID>& idMap);

  //################################################################################################
  void generateNewOutputIds(std::unordered_map<tp_utils::StringID, tp_utils::StringID>& idMap);

  //################################################################################################
  //! Saves the state of a step
  void saveBinary(tp_utils::JSON& j, const std::function<uint64_t(const std::string&)>& addBlob) const noexcept;

  //################################################################################################
  //! loads the state of a step
  void loadBinary(const tp_utils::JSON& j, const std::vector<std::string>& blobs)noexcept;

  //################################################################################################
  const std::unordered_map<tp_utils::StringID, Parameter>& parameters();

  //################################################################################################
  void setParamerter(const Parameter& parameter);

  //################################################################################################
  void setActiveOperationName(const tp_utils::StringID& operationName);

  //################################################################################################
  const tp_utils::StringID& activeOperationName() const;

  //################################################################################################
  const std::unordered_map<tp_utils::StringID, Parameter>* parametersForOperation(const tp_utils::StringID& operationName) const;

  //################################################################################################
  const std::vector<tp_utils::StringID>* parameterOrderForOperation(const tp_utils::StringID& operationName) const;

  //################################################################################################
  void setParameterForOperation(const tp_utils::StringID& operationName, const Parameter& parameter);

  //################################################################################################
  void removeParameterForOperation(const tp_utils::StringID& operationName, const tp_utils::StringID& parameterName);

  //################################################################################################
  std::vector<tp_utils::StringID> orderedParameterNames() const;

  //################################################################################################
  std::vector<tp_utils::StringID> parameterNamesForOperation(const tp_utils::StringID& operationName) const;

  //################################################################################################
  StepDetails detailsForOperationParameters(const tp_utils::StringID& operationName) const;

  //################################################################################################
  void syncParametersForOperation(const tp_utils::StringID& operationName, StepDetails* sourceDetails);

  //################################################################################################
  void setParameterValue(const tp_utils::StringID& name, const Variant& value);

  //################################################################################################
  void removeParameter(const tp_utils::StringID& name);

  //################################################################################################
  Parameter parameter(const tp_utils::StringID& name)
  {
    const auto& p = parameters();

    if(const auto& operationName = activeOperationName(); operationName.isValid())
    {
      if(const auto* operationParameters = parametersForOperation(operationName))
      {
        const auto i = operationParameters->find(name);
        if(i != operationParameters->end())
        {
          auto parameter = i->second;
          parameter.name = name;
          return parameter;
        }
      }
    }

    const auto i = p.find(name);
    return (i == p.end())?Parameter():i->second;
  }

  //################################################################################################
  template<typename T>
  T parameterValue(const tp_utils::StringID& name, const T& defaultValue=T())
  {
    const auto p = parameter(name);
    if(!p.name.isValid())
      return defaultValue;

    return tpGetVariantValue(p.value, defaultValue);
  }

  //################################################################################################
  bool boolParameter(const tp_utils::StringID& name, bool defaultValue=false)
  {
    std::string value = parameterValue<std::string>(name);

    if(defaultValue)
    {
      if(value == "No")
        return false;

      if(value == "False")
        return false;
    }
    else
    {
      if(value == "Yes")
        return true;

      if(value == "True")
        return true;
    }

    return defaultValue;
  }

  //################################################################################################
  const std::vector<tp_utils::StringID>& parametersOrder()const;

  //################################################################################################
  //! Uses this to set the order of parameters
  void setParametersOrder(const std::vector<tp_utils::StringID>& parametersOrder);

  //################################################################################################
  //! This will remove invalid parameters
  void setValidParameters(const std::vector<tp_utils::StringID>& validParams);

  //################################################################################################
  ComplexObjectManager complexObjectManager;

  //################################################################################################
  //! Sets the mapping between a port on the step delegate and the name of the data in the collection.
  void setInputMapping(const std::vector<PortMapping>& inputMapping);

  //################################################################################################
  const std::vector<PortMapping>& inputMapping() const;

  //################################################################################################
  //! Sets the mapping between a port on the step delegate and the name of the data in the collection.
  void setOutputMapping(const std::vector<PortMapping>& outputMapping);

  //################################################################################################
  const std::vector<PortMapping>& outputMapping() const;

  //################################################################################################
  void invalidate();

  //------------------------------------------------------------------------------------------------
  // The following functions are used when we are running the pipeline rather than developing and
  // testing it. They allow us to override the inputs to the pipeline an not save the outputs to
  // disk instead whatever code is calling the pipeline grab the outputs directly.
  //
  // These settings do not get saved to the pipeline.
  //
  //------------------------------------------------------------------------------------------------

  //################################################################################################
  //! Special case for output steps to prevent them from running and saving.
  void setNoExec(bool noExec);

  //################################################################################################
  bool noExec() const;

  //################################################################################################
  struct OverrideOutput
  {
    std::shared_ptr<tp_data::AbstractMember> member;

    OverrideOutput() = default;

    OverrideOutput(tp_data::AbstractMember* member_):
      member(member_)
    {

    }

    OverrideOutput(const std::shared_ptr<tp_data::AbstractMember>& member_):
      member(member_)
    {

    }
  };

  //################################################################################################
  //! Used to override the output data of a node in a pipeline when running, not saved.
  void setOverrideOutputs(const std::vector<OverrideOutput>& overrideOutputs);

  //################################################################################################
  const std::vector<OverrideOutput>& overrideOutputs() const;

  //------------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------


private:
  //################################################################################################
  //Called by the pipeline
  void setParent(PipelineDetails* parent);
};

}
