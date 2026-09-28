#ifndef tp_utils_stack_trace_EnscriptenStackTrace_h
#define tp_utils_stack_trace_EnscriptenStackTrace_h

#include "tp_utils/detail/stack_trace/Common.h" // IWYU pragma: keep

#ifdef EMSCRIPTEN_STACKTRACE
#include <emscripten.h>

namespace tp_utils
{

//##################################################################################################
void printStackTrace()
{
  emscripten_run_script("console.log((new Error()).stack);");
}

//##################################################################################################
std::vector<std::string> stackTraceFrames()
{
  return {emscripten_run_script_string("(new Error()).stack;")};
}

}

#endif

#endif
