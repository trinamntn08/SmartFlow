#ifndef tp_utils_stack_trace_Common_h
#define tp_utils_stack_trace_Common_h

#include "tp_utils/DebugUtils.h" // IWYU pragma: keep
#include "tp_utils/FileUtils.h" // IWYU pragma: keep

#include <chrono>
#include <thread>
#include <iostream>

#if defined(TP_ANDROID)
#  define ANDROID_STACKTRACE

#elif defined(TP_OSX)
#  define OSX_STACKTRACE

#elif defined(EMSCRIPTEN)
#  define EMSCRIPTEN_STACKTRACE

#elif defined(TP_WIN32_MSVC)
#  define WIN23_MSVC_STACKTRACE

#elif defined(__GLIBC__) && (!defined(__UCLIBC__) || defined(__UCLIBC_HAS_BACKTRACE__))
#  define GCC_STACKTRACE

#else
#  define UNSUPPORTED_STACKTRACK

#endif

#define MAX_LEVELS 100
//#define MAX_LEVELS 20

#endif
