#ifndef tp_utils_StaticState_h
#define tp_utils_StaticState_h

#include "tp_utils/MutexUtils.h" // IWYU pragma: keep
#include "tp_utils/StackTrace.h" // IWYU pragma: keep
#include "tp_utils/DebugUtils.h" // IWYU pragma: keep
#include "tp_utils/Garbage.h" // IWYU pragma: keep
#include "tp_utils/RefCount.h" // IWYU pragma: keep
#include "tp_utils/TimeUtils.h" // IWYU pragma: keep

namespace tp_utils
{

//##################################################################################################
class StaticState
{
public:
  //################################################################################################
  static std::shared_ptr<StaticState> instance();

  //################################################################################################
#ifdef TP_ENABLE_COUNT_STACK_TRACE
  struct CountStackTraceStats
  {
    TPMutex mutex{TPM};
    std::unordered_map<StackTrace, size_t> counts;
  } countStackTraceStats;
#endif

  //################################################################################################
#ifndef TP_NO_THREADS
  struct Garbage
  {
    TPMutex instanceMutex{TPM};
    tp_utils::Garbage* instance{nullptr};
  } garbage;
#endif

  //################################################################################################
#ifdef TP_REF_COUNT
  struct RefCountStats
  {
    std::mutex mutex;
    std::unordered_map<tp_utils::StringID, InstanceDetails> instances;
  } refCountStats;
#endif

  //################################################################################################
#ifdef TP_ENABLE_FUNCTION_TIME
  struct FunctionTimeStats
  {
    TPMutex mutex{TPM};
    std::unordered_map<std::string, FunctionTimeStatsDetails> stats;
  } functionTimeStats;
#endif

};

}




#endif
