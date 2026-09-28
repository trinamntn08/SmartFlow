#ifndef tp_utils_stack_trace_Win32MSVCStackTrace_h
#define tp_utils_stack_trace_Win32MSVCStackTrace_h

#include "tp_utils/detail/stack_trace/Common.h" // IWYU pragma: keep
#include "tp_utils/StackTrace.h"

#ifdef TP_WIN32_MSVC

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <new.h>
#include <signal.h>
#include <exception>
#include <sys/stat.h>
#include <Psapi.h>
#include <rtcapi.h>
#include <shellapi.h>
//#include <DbgHelp.h>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "dbghelp.lib")

// Some versions of imagehlp.dll lack the proper packing directives themselves
// so we need to do it.
#pragma pack( push, before_imagehlp, 8 )
#include <ImageHlp.h>
#pragma pack( pop, before_imagehlp )

namespace tp_utils
{

//##################################################################################################
// This method creates minidump of the process
void createMiniDump(EXCEPTION_POINTERS* pExceptionPtrs)
{
  if(!pExceptionPtrs || !pExceptionPtrs->ContextRecord)
    return;

  HMODULE hDbgHelp = nullptr;
  HANDLE hFile = nullptr;
  MINIDUMP_EXCEPTION_INFORMATION mei;
  MINIDUMP_CALLBACK_INFORMATION mci;

  // Load dbghelp.dll
  hDbgHelp = LoadLibrary(_T("dbghelp.dll"));
  if(hDbgHelp==nullptr)
  {
    // Error - couldn't load dbghelp.dll
    return;
  }

  // Create the minidump file
  hFile = CreateFile(
        _T("crashdump.dmp"),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

  if(hFile==INVALID_HANDLE_VALUE)
  {
    // Couldn't create file
    return;
  }

  // Write minidump to the file
  mei.ThreadId = GetCurrentThreadId();
  mei.ExceptionPointers = pExceptionPtrs;
  mei.ClientPointers = FALSE;
  mci.CallbackRoutine = nullptr;
  mci.CallbackParam = nullptr;

  typedef BOOL (WINAPI *LPMINIDUMPWRITEDUMP)(
        HANDLE hProcess,
        DWORD ProcessId,
        HANDLE hFile,
        MINIDUMP_TYPE DumpType,
        CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
        CONST PMINIDUMP_USER_STREAM_INFORMATION UserEncoderParam,
        CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

  LPMINIDUMPWRITEDUMP pfnMiniDumpWriteDump = reinterpret_cast<LPMINIDUMPWRITEDUMP>(GetProcAddress(hDbgHelp, "MiniDumpWriteDump"));
  if(!pfnMiniDumpWriteDump)
  {
    // Bad MiniDumpWriteDump function
    return;
  }

  HANDLE hProcess = GetCurrentProcess();
  DWORD dwProcessId = GetCurrentProcessId();

  BOOL bWriteDump = pfnMiniDumpWriteDump(
        hProcess,
        dwProcessId,
        hFile,
        MiniDumpNormal,
        &mei,
        nullptr,
        &mci);

  if(!bWriteDump)
  {
    // Error writing dump.
    return;
  }

  // Close file
  CloseHandle(hFile);

  // Unload dbghelp.dll
  FreeLibrary(hDbgHelp);
}

//##################################################################################################
// The following code gets exception pointers using a workaround found in CRT code.
void getExceptionPointers(DWORD dwExceptionCode, EXCEPTION_POINTERS** ppExceptionPointers)
{
  // The following code was taken from VC++ 8.0 CRT (invarg.c: line 104)

  EXCEPTION_RECORD ExceptionRecord;
  CONTEXT ContextRecord;
  memset(&ContextRecord, 0, sizeof(CONTEXT));
  ContextRecord.ContextFlags = CONTEXT_FULL;

#ifdef _X86_

  __asm {
    mov dword ptr [ContextRecord.Eax], eax
        mov dword ptr [ContextRecord.Ecx], ecx
        mov dword ptr [ContextRecord.Edx], edx
        mov dword ptr [ContextRecord.Ebx], ebx
        mov dword ptr [ContextRecord.Esi], esi
        mov dword ptr [ContextRecord.Edi], edi
        mov word ptr [ContextRecord.SegSs], ss
        mov word ptr [ContextRecord.SegCs], cs
        mov word ptr [ContextRecord.SegDs], ds
        mov word ptr [ContextRecord.SegEs], es
        mov word ptr [ContextRecord.SegFs], fs
        mov word ptr [ContextRecord.SegGs], gs
        pushfd
        pop [ContextRecord.EFlags]
  }

  ContextRecord.ContextFlags = CONTEXT_CONTROL;
#pragma warning(push)
#pragma warning(disable:4311)
  ContextRecord.Eip = (ULONG)_ReturnAddress();
  ContextRecord.Esp = (ULONG)_AddressOfReturnAddress();
#pragma warning(pop)
  ContextRecord.Ebp = *((ULONG *)_AddressOfReturnAddress()-1);


#elif defined (_IA64_) || defined (_AMD64_)

  /* Need to fill up the Context in IA64 and AMD64. */
  RtlCaptureContext(&ContextRecord);

#else  /* defined (_IA64_) || defined (_AMD64_) */

  ZeroMemory(&ContextRecord, sizeof(ContextRecord));

#endif  /* defined (_IA64_) || defined (_AMD64_) */

  ZeroMemory(&ExceptionRecord, sizeof(EXCEPTION_RECORD));

  ExceptionRecord.ExceptionCode = dwExceptionCode;
  ExceptionRecord.ExceptionAddress = _ReturnAddress();

  ///

  EXCEPTION_RECORD* pExceptionRecord = new EXCEPTION_RECORD;
  memcpy(pExceptionRecord, &ExceptionRecord, sizeof(EXCEPTION_RECORD));
  CONTEXT* pContextRecord = new CONTEXT;
  memcpy(pContextRecord, &ContextRecord, sizeof(CONTEXT));

  *ppExceptionPointers = new EXCEPTION_POINTERS;
  (*ppExceptionPointers)->ExceptionRecord = pExceptionRecord;
  (*ppExceptionPointers)->ContextRecord = pContextRecord;
}

//##################################################################################################
[[noreturn]]void saveCrashReportAndExit()
{
  saveCrashReportAndExit(nullptr);
}

//##################################################################################################
[[noreturn]]void saveCrashReportAndExit(EXCEPTION_POINTERS* pExceptionPtrs)
{
  if(!pExceptionPtrs)
    getExceptionPointers(0, &pExceptionPtrs);

  printStackTrace(pExceptionPtrs);
  tp_utils::writeTextFile("crash.txt", formatStackTrace(pExceptionPtrs));
  createMiniDump(pExceptionPtrs);
  exit(1);
}

namespace
{

//##################################################################################################
struct module_data
{
  std::string image_name;
  std::string module_name;
  void *base_address;
  DWORD load_size;
};

//##################################################################################################
class symbol
{
  typedef IMAGEHLP_SYMBOL64 sym_type;
  sym_type *sym;
  static const int max_name_len = 1024;
public:
  //################################################################################################
  symbol(HANDLE process, DWORD64 address):
    sym(reinterpret_cast<sym_type*>(::operator new(sizeof(*sym) + max_name_len)))
  {
    memset(sym, '\0', sizeof(*sym) + max_name_len);
    sym->SizeOfStruct = sizeof(*sym);
    sym->MaxNameLength = max_name_len;
    DWORD64 displacement;

    if(!SymGetSymFromAddr64(process, address, &displacement, sym))
      std::cout << "Bad symbol." << std::endl;
  }

  //################################################################################################
  std::string undecorated_name()
  {
    std::vector<char> und_name(max_name_len);
    UnDecorateSymbolName(sym->Name, &und_name[0], max_name_len, UNDNAME_COMPLETE);
    return std::string(&und_name[0], strlen(&und_name[0]));
  }
};
}

//##################################################################################################
std::vector<std::string> TP_UTILS_EXPORT stackTraceFrames(EXCEPTION_POINTERS* pExceptionPtrs)
{
  std::vector<std::string> result;
  if(!pExceptionPtrs || !pExceptionPtrs->ContextRecord)
  {
    result.push_back("No stack trace available.");
    return result;
  }

  HANDLE process = GetCurrentProcess();
  SymInitialize(process, nullptr, TRUE);

  // StackWalk64() may modify context record passed to it, so we will
  // use a copy.
  CONTEXT context_record = *pExceptionPtrs->ContextRecord;
  // Initialize stack walking.
  STACKFRAME64 stackFrame;
  memset(&stackFrame, 0, sizeof(stackFrame));
#if defined(_WIN64)
  int machine_type = IMAGE_FILE_MACHINE_AMD64;
  stackFrame.AddrPC.Offset = context_record.Rip;
  stackFrame.AddrFrame.Offset = context_record.Rbp;
  stackFrame.AddrStack.Offset = context_record.Rsp;
#else
  int machine_type = IMAGE_FILE_MACHINE_I386;
  stackFrame.AddrPC.Offset = context_record.Eip;
  stackFrame.AddrFrame.Offset = context_record.Ebp;
  stackFrame.AddrStack.Offset = context_record.Esp;
#endif
  stackFrame.AddrPC.Mode = AddrModeFlat;
  stackFrame.AddrFrame.Mode = AddrModeFlat;
  stackFrame.AddrStack.Mode = AddrModeFlat;

  int frameNumber=0;
  DWORD offsetFromSymbol=0;
  while(StackWalk64(DWORD(machine_type),
                    GetCurrentProcess(),
                    GetCurrentThread(),
                    &stackFrame,
                    &context_record,
                    nullptr,
                    &SymFunctionTableAccess64,
                    &SymGetModuleBase64,
                    nullptr))
  {
    if(frameNumber>MAX_LEVELS)
      break;

    auto& resultLine = result.emplace_back();
    resultLine += tp_utils::fixedWidthKeepRight(std::to_string(frameNumber), 3, '0') + ' ';

    if(stackFrame.AddrPC.Offset!=0)
    {
      resultLine += symbol(process, stackFrame.AddrPC.Offset).undecorated_name();

      IMAGEHLP_LINE64 line;
      line.SizeOfStruct = sizeof(line);
      if(SymGetLineFromAddr64(process, stackFrame.AddrPC.Offset, &offsetFromSymbol, &line))
        resultLine += ' ' + std::string(line.FileName) + ':' + std::to_string(line.LineNumber);
    }
    else
      result.push_back("(No Symbols: PC == 0)");
    frameNumber++;
  }

  return result;
}

//##################################################################################################
std::vector<std::string> TP_UTILS_EXPORT stackTraceFrames()
{
  EXCEPTION_POINTERS* pExceptionPtrs = nullptr;
  getExceptionPointers(0, &pExceptionPtrs);
  return stackTraceFrames(pExceptionPtrs);
}

//##################################################################################################
std::string formatStackTrace(EXCEPTION_POINTERS* pExceptionPtrs)
{
  return formatStackTrace(stackTraceFrames(pExceptionPtrs));
}

//##################################################################################################
void printStackTrace()
{
  std::cout << formatStackTrace() << std::endl;
}

//##################################################################################################
void printStackTrace(EXCEPTION_POINTERS* pExceptionPtrs)
{
  std::cout << formatStackTrace(pExceptionPtrs) << std::endl;
}

}

#endif

#endif
