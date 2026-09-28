#include "lib_platform/SetThreadName.h"

namespace lib_platform
{
#ifdef TP_WIN32 //=================================================================================
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#include <Windows.h>
#include <Processthreadsapi.h>

const DWORD MS_VC_EXCEPTION=0x406D1388;

//##################################################################################################
#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
  DWORD dwType; // Must be 0x1000.
  LPCSTR szName; // Pointer to name (in user addr space).
  DWORD dwThreadID; // Thread ID (-1=caller thread).
  DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

//##################################################################################################
void setThreadName(uint32_t dwThreadID, const char* threadName)
{
  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = threadName;
  info.dwThreadID = dwThreadID;
  info.dwFlags = 0;

  __try
  {
    RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), reinterpret_cast<ULONG_PTR*>(&info));
  }
  __except(EXCEPTION_EXECUTE_HANDLER)
  {
  }
}

//##################################################################################################
void setThreadName(const std::string& threadName)
{
  setThreadName(GetCurrentThreadId(), threadName.c_str());
}

//##################################################################################################
void setThreadName(std::thread& thread, const std::string& threadName)
{
  setThreadName(GetThreadId(static_cast<HANDLE>(thread.native_handle())), threadName.c_str());
}

#elif defined(__GCC__) || defined(__GNUC__)
#include <windows.h>
#include <processthreadsapi.h>
#include <winnt.h>
#include <winternl.h>

#include <pshpack8.h>

typedef struct
{
  DWORD dwType;
  LPCSTR szName;
  DWORD dwThreadID;
  DWORD dwFlags;
} THREADNAME_INFO;
#include <poppack.h>

//##################################################################################################
static EXCEPTION_DISPOSITION NTAPI ignore_handler(EXCEPTION_RECORD*, void*, CONTEXT*, void*)
{
  return ExceptionContinueExecution;
}

//##################################################################################################
void setThreadName(uint32_t dwThreadID, const char* threadName)
{
  static const DWORD MS_VC_EXCEPTION = 0x406D1388;

  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = threadName;
  info.dwThreadID = dwThreadID;
  info.dwFlags = 0;

  // Push an exception handler to ignore all following exceptions
  NT_TIB *tib = reinterpret_cast<NT_TIB*>(NtCurrentTeb());
  EXCEPTION_REGISTRATION_RECORD rec;
  rec.Next = tib->ExceptionList;
  rec.Handler = ignore_handler;
  tib->ExceptionList = &rec;

  // Visual Studio and compatible debuggers receive thread names from the
  // program through a specially crafted exception
  RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), reinterpret_cast<ULONG_PTR*>(&info));

  // Pop exception handler
  tib->ExceptionList = tib->ExceptionList->Next;
}

//##################################################################################################
void setThreadName(const std::string& threadName)
{
  setThreadName(GetCurrentThreadId(), threadName.c_str());
}

//##################################################################################################
void setThreadName(std::thread& thread, const std::string& threadName)
{
  setThreadName(thread.native_handle(), threadName.c_str());
}

#else

//##################################################################################################
void setThreadName(const std::string&)
{

}

//##################################################################################################
void setThreadName(std::thread&, const std::string&)
{

}

#endif

#elif defined(TP_IOS) //===========================================================================

#include <pthread.h>

//##################################################################################################
void setThreadName(const std::string& threadName)
{
  std::string n = threadName;
  if(n.size()>15)
    n.resize(15);
  pthread_setname_np(n.c_str());
}

//##################################################################################################
void setThreadName(std::thread& thread, const std::string& threadName)
{
  (void)(thread);
  (void)(threadName);
}

#elif defined(TP_OSX) //===========================================================================

#include <pthread.h>

//##################################################################################################
void setThreadName(const std::string& threadName)
{
  std::string n = threadName;
  if(n.size()>15)
    n.resize(15);
  pthread_setname_np(n.c_str());
}

//##################################################################################################
void setThreadName(std::thread& thread, const std::string& threadName)
{
  (void)(thread);
  (void)(threadName);
}
#elif defined(TP_EMSCRIPTEN) //====================================================================

//##################################################################################################
void setThreadName(const std::string&)
{

}

//##################################################################################################
void setThreadName(std::thread&, const std::string&)
{

}

#else //============================================================================================

#include <sys/prctl.h>

//##################################################################################################
void setThreadName(const std::string& threadName)
{
  std::string n = threadName;
  if(n.size()>15)
    n.resize(15);
  prctl(PR_SET_NAME, n.c_str(), 0, 0, 0);
}

//##################################################################################################
void setThreadName(std::thread& thread, const std::string& threadName)
{
  std::string n = threadName;
  if(n.size()>15)
    n.resize(15);
  pthread_setname_np(thread.native_handle(), n.c_str());
}
#endif

}
