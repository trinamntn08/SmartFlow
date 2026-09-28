#ifndef tp_utils_stack_trace_UnsupportedStackTrace_h
#define tp_utils_stack_trace_UnsupportedStackTrace_h

#include "tp_utils/detail/stack_trace/Common.h" // IWYU pragma: keep

#ifdef UNSUPPORTED_STACKTRACK
namespace tp_utils
{

//##################################################################################################
void TP_UTILS_EXPORT printStackTrace()
{

}

//##################################################################################################
std::vector<std::string> stackTraceFrames()
{
  return {};
}

}

#endif

#endif
