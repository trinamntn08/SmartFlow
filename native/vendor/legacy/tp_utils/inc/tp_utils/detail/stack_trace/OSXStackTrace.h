#ifndef tp_utils_stack_trace_OSXStackTrace_h
#define tp_utils_stack_trace_OSXStackTrace_h

#include "tp_utils/detail/stack_trace/Common.h" // IWYU pragma: keep

#ifdef OSX_STACKTRACE
#include <execinfo.h>
#include <stdio.h>

namespace tp_utils
{

//##################################################################################################
void printStackTrace()
{
  void* callstack[128];
  int i, frames = backtrace(callstack, 128);
  char** strs = backtrace_symbols(callstack, frames);
  for(i = 0; i < frames; ++i)
    tpWarning() << strs[i];
  free(strs);
}

//##################################################################################################
std::vector<std::string> stackTraceFrames()
{
  return {};
}

}

#endif

#endif
