#include "tp_utils/StackTrace.h"

#include "tp_utils/detail/stack_trace/Common.h"

#ifdef GCC_STACKTRACE
#  include "tp_utils/detail/stack_trace/GCCStackTrace.h"
#  include "tp_utils/detail/stack_trace/StackTrace_GCC.h"

#elif defined(ANDROID_STACKTRACE)
#  include "tp_utils/detail/stack_trace/AndroidStackTrace.h"
#  include "tp_utils/detail/stack_trace/StackTrace_BasicImpl.h"

#elif defined(EMSCRIPTEN_STACKTRACE)
#  include "tp_utils/detail/stack_trace/EmscriptenStackTrace.h"
#  include "tp_utils/detail/stack_trace/StackTrace_BasicImpl.h"

#elif defined(OSX_STACKTRACE)
#  include "tp_utils/detail/stack_trace/OSXStackTrace.h"
#  include "tp_utils/detail/stack_trace/StackTrace_BasicImpl.h"

#elif defined(WIN23_MSVC_STACKTRACE)
#  include "tp_utils/detail/stack_trace/Win32MSVCStackTrace.h"
#  include "tp_utils/detail/stack_trace/StackTrace_BasicImpl.h"

#else
#  include "tp_utils/detail/stack_trace/UnsupportedStackTrace.h"
#  include "tp_utils/detail/stack_trace/StackTrace_BasicImpl.h"

#endif

namespace tp_utils
{

//##################################################################################################
std::string formatStackTrace()
{
  return formatStackTrace(stackTraceFrames());
}

//##################################################################################################
std::string TP_UTILS_EXPORT formatStackTrace(const std::vector<std::string>& frames)
{
  std::string results = std::string("Stack frames: ") + std::to_string(frames.size()) + '\n';
  for(size_t i=0; i<frames.size(); i++)
    results.append(std::string("Frame ") + std::to_string(i) +": " + frames.at(i) + '\n');
  return results;
}

//##################################################################################################
void saveCrashReport()
{
  printStackTrace();
  tp_utils::writeTextFile("crash.txt", formatStackTrace());
}

//##################################################################################################
#ifndef TP_WIN32_MSVC
[[noreturn]]void saveCrashReportAndExit()
{
  saveCrashReport();
  exit(1);
}
#endif

}
