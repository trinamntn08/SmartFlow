#pragma once

#include "tp_pipeline/Globals.h"

#include "tp_utils/JSONUtils.h"

namespace tp_pipeline
{

//##################################################################################################
struct TP_PIPELINE_SHARED_EXPORT StepProfilingData
{
  std::vector<tp_utils::StringID> dependencies;
  std::vector<tp_utils::StringID> recursiveDependencies;

  uint64_t readyToRunAt{0};
  uint64_t runStartedAt{0};
  uint64_t runCompleteAt{0};

  uint64_t cacheKey{0};

  double fractionOfTotal{0.0};

  StepStatus status{StepStatus::NotRun};

  //################################################################################################
  uint64_t queueTime() const;

  //################################################################################################
  uint64_t runTime() const;

  //################################################################################################
  uint64_t totalTime() const;

  //################################################################################################
  void saveState(tp_utils::JSON& j) const;

  //################################################################################################
  void loadState(const tp_utils::JSON& j);
};

}
