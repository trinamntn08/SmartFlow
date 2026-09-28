#pragma once

#include "tp_pipeline/Globals.h"
#include "tp_pipeline/StepDetails.h"

#include "tp_utils/CallbackCollection.h"

namespace tp_pipeline
{

//##################################################################################################
class TP_PIPELINE_SHARED_EXPORT PipelineDetails
{
  TP_DQ;
public:
  //################################################################################################
  PipelineDetails();

  //################################################################################################
  PipelineDetails(const PipelineDetails& other);

  //################################################################################################
  //! Destroy the pipeline and all child steps
  /*!
  This will destroy the pipeline and delete all of its steps. All callbacks should have been removed
  before calling this. This will not call removed or changed callbacks.
  */
  ~PipelineDetails();  

  //################################################################################################
  PipelineDetails& operator=(const PipelineDetails& other);  

  //################################################################################################
  void printStats() const;

  //################################################################################################
  void appendPipeline(const PipelineDetails& other);

  //################################################################################################
  //! Returns the steps that form this pipeline
  /*!
  This returns the list of steps that form this pipeline in the order that they will be executed.

  \return The list of steps belonging to this pipeline.
  */
  const std::vector<StepDetails*>& steps()const;

  //################################################################################################
  StepDetails* findStepFromDelegateName(const tp_utils::StringID& delegateName) const;

  //################################################################################################
  StepDetails* findStepFromStepId(const tp_utils::StringID& stepId) const;

  //################################################################################################
  //! Adds a step to this pipeline
  /*!
  This pipeline will take ownership of the step. If the step is already part of this pipeline it
  will be removed and re-added to the end of this pipeline.

  \warning The step must not be part of another pipeline.

  \param step - A new step to append to the end of the pipeline.
  */
  void addStep(StepDetails* step);

  //################################################################################################
  //! Insert a step
  /*!
  This is the same as addStep however it allows you to specify an index to insert the step at. This
  will take ownership of the step.

  \param step - The step to insert.
  \param index - The index to insrt the step at.
  */
  void insertStep(StepDetails* step, size_t index);

  //################################################################################################
  //! Call this when the step changes
  /*!
  Call this each time the step changes, this will cause any cached state to be destroyed.

  \note This is usually called by the step itself.
  \note This will call the changed callbacks.

  \param step - The step that has changed.
  */
  void invalidateStep(StepDetails* step);

  //################################################################################################
  //! Delete a step that belongs to this pipeline
  /*!
  This will delete the step and call the remove callbacks.

  \param step - The step to delete, must be a child of this pipeline.
  */
  void deleteStep(StepDetails* step);

  //################################################################################################
  //! Clear inputs names that are not connected to valid
  void clearDanglingInputs();

  //################################################################################################
  //! Called when a step changes or is added.
  tp_utils::CallbackCollection<void(StepDetails*)> changedCallback;

  //################################################################################################
  //! Called just before a step is deleted.
  tp_utils::CallbackCollection<void(StepDetails*)> deletedCallback;

  //################################################################################################
  void saveBinary(std::string& data);

  //################################################################################################
  void loadBinary(std::string& error, const std::string& data);
};

}
