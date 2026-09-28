#ifndef tp_utils_stack_trace_AndroidStackTrace_h
#define tp_utils_stack_trace_AndroidStackTrace_h

#include "tp_utils/detail/stack_trace/Common.h" // IWYU pragma: keep

#ifdef ANDROID_STACKTRACE
#include <unwind.h>
#include <dlfcn.h>
#include <cxxabi.h>

// Android version taken from:
// http://stackoverflow.com/questions/8115192/android-ndk-getting-the-backtrace/35585744#35585744
struct android_backtrace_state
{
  void** current;
  void** end;
};

//##################################################################################################
static _Unwind_Reason_Code android_unwind_callback(struct _Unwind_Context* context, void* arg)
{
  android_backtrace_state* state = (android_backtrace_state*)arg;
  uintptr_t pc = _Unwind_GetIP(context);
  if(pc)
  {
    if(state->current == state->end)
      return _URC_END_OF_STACK;
    else
      *state->current++ = reinterpret_cast<void*>(pc);
  }

  return _URC_NO_REASON;
}

//##################################################################################################
void printStackTrace()
{
  tpWarning() << "\nandroid stack dump";

  const int max = 100;
  void* buffer[max];

  android_backtrace_state state;
  state.current = buffer;
  state.end = buffer + max;

  _Unwind_Backtrace(android_unwind_callback, &state);

  char traceString[512];
  int count = (int)(state.current - buffer);
  for(int idx = 0; idx < count; idx++)
  {
    const void* addr = buffer[idx];
    const char* symbol = "";

    Dl_info info;
    if(dladdr(addr, &info) && info.dli_sname)
      symbol = info.dli_sname;

    int status = -1;
    char* demangled = __cxxabiv1::__cxa_demangle(symbol, 0, 0, &status);

    sprintf(traceString, "%03d: 0x%p %s", idx, addr, (demangled && !status) ? demangled : symbol);
    tpWarning() << traceString;

    if(!status)
      free(demangled);
  }

  tpWarning() << "android stack dump done\n";
}

//##################################################################################################
std::vector<std::string> stackTraceFrames()
{
  return {};
}


#endif

#endif
