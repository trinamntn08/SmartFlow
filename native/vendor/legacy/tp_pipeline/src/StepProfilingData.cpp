#include "tp_pipeline/StepProfilingData.h"

#include "tp_utils/JSONUtils.h"

namespace tp_pipeline
{
//##################################################################################################
uint64_t StepProfilingData::queueTime() const
{
  return runStartedAt - readyToRunAt;
}

//##################################################################################################
uint64_t StepProfilingData::runTime() const
{
  return runCompleteAt - runStartedAt;
}

//##################################################################################################
uint64_t StepProfilingData::totalTime() const
{
  return runCompleteAt - readyToRunAt;
}


//##################################################################################################
void StepProfilingData::saveState(tp_utils::JSON& j) const
{
  tp_utils::saveVectorOfStringIDsToJSON(j["dependencies"], dependencies);
  tp_utils::saveVectorOfStringIDsToJSON(j["recursiveDependencies"], recursiveDependencies);

  j["readyToRunAt"] = readyToRunAt;
  j["runStartedAt"] = runStartedAt;
  j["runCompleteAt"] = runCompleteAt;

  j["cacheKey"] = cacheKey;

  j["fractionOfTotal"] = fractionOfTotal;
  j["status"] = int(status);
}

//##################################################################################################
void StepProfilingData::loadState(const tp_utils::JSON& j)
{
  tp_utils::loadVectorOfStringIDsFromJSON(j, "dependencies", dependencies);
  tp_utils::loadVectorOfStringIDsFromJSON(j, "recursiveDependencies", recursiveDependencies);

  readyToRunAt = TPJSONUint64T(j, "readyToRunAt", 0);
  runStartedAt = TPJSONUint64T(j, "runStartedAt", 0);
  runCompleteAt = TPJSONUint64T(j, "runCompleteAt", 0);

  cacheKey = TPJSONUint64T(j, "cacheKey", 0);

  fractionOfTotal = TPJSONDouble(j, "fractionOfTotal", 0.0);
  status = StepStatus(TPJSONInt(j, "status", int(StepStatus::NotRun)));
}

}
