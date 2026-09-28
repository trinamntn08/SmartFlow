#pragma once

#include "tp_pipeline/Globals.h"
#include "tp_pipeline/StepDetails.h"
#include "tp_pipeline/StepContext.h"

namespace tp_utils
{
class ParrallelProgress;
}

namespace tp_data
{
class Collection;
class CollectionFactory;
}

namespace tp_pipeline
{
class StepDelegate;


//##################################################################################################
//! This uses delegates to manage and execute a pipeline.
class TP_PIPELINE_SHARED_EXPORT PipelineManager
{
  TP_DQ;
public:
  //################################################################################################
  PipelineManager(PipelineDetails* pipelineDetails,
                  const StepDelegateMap* stepDelegates,
                  const tp_data::CollectionFactory* collectionFactory,
                  bool fixupParameters=true);

  //################################################################################################
  ~PipelineManager();

  //################################################################################################
  PipelineDetails* pipelineDetails() const;

  //################################################################################################
  void setDebugRootDir(const std::string& debugRootDir);

  //################################################################################################
  const std::string& debugRootDir() const;

  //################################################################################################
  void startExecution(StepDetails* finalStep=nullptr);

  //################################################################################################
  StepContext* takeNextAvailableStep(tp_utils::ParrallelProgress* parrallelProgress);

  //################################################################################################
  void returnCompletedStep(StepContext* stepContext);

  //################################################################################################
  StepContext* stepContext(StepDetails* stepDetails) const;

  //################################################################################################
  const std::vector<StepContext>& stepContexts() const;

  //################################################################################################
  void printRunStats(tp_utils::Progress* progress) const;

  //################################################################################################
  uint64_t calculateCacheKey(const StepContext* stepContext) const;
};

}
