#ifndef tp_utils_stack_trace_GCCStackTrace_h
#define tp_utils_stack_trace_GCCStackTrace_h

#include "tp_utils/detail/stack_trace/Common.h" // IWYU pragma: keep

#include "tp_utils/FileUtils.h"

#ifdef GCC_STACKTRACE
//This allows us to print a stack trace
//This is gcc specific, and we may want to remove it from production code
#include <execinfo.h>
#include <cxxabi.h>
#include <ucontext.h>
#include <cstring>
#include <memory>

namespace tp_utils
{
//##################################################################################################
static bool demangle(const char* symbol, std::string& output, std::string& offset)
{
  std::cout << "SSS: " << symbol << std::endl;
  std::unique_ptr<char, decltype(&free)> symbol2(strdup(symbol), &free);

  // find parentheses and +address offset surrounding the mangled name:
  // ./module(function+0x15c) [0x8048a6d]
  char* begin_name = nullptr;
  char* end_name = nullptr;
  char* begin_offset = nullptr;
  char* end_offset = nullptr;
  char* p;
  for(p = symbol2.get(); *p; ++p)
  {
    if(*p == '(')
      begin_name = p;
    else if(*p == '+')
      begin_offset = p;
    else if((*p == ')') && begin_offset)
    {
      end_offset = p;
      break;
    }
  }

  // BCH 24 Dec 2014: backtrace_symbols() on OS X seems to return strings in a different, non-standard format.
  // Added this code in an attempt to parse that format.  No doubt it could be done more cleanly.  :->
  // Updated to correctly parse the symbols on iOS too.
  if(!begin_name)
  {
    begin_offset = nullptr;
    end_offset = nullptr;

    enum class ParseState
    {
      kInWhitespace1 = 1,
      kInLineNumber,
      kInWhitespace2,
      kInPackageName,
      kInWhitespace3,
      kInAddress,
      kInWhitespace4,
      kInFunction,
      kInWhitespace5,
      kInPlus,
      kInWhitespace6,
      kInOffset,
      kInOverrun
    };

    ParseState parse_state = ParseState::kInWhitespace1;
    for(p = symbol2.get(); *p; ++p)
    {
      switch(parse_state)
      {
        case ParseState::kInWhitespace1: if(!isspace(*p)) parse_state = ParseState::kInLineNumber;  break;
        case ParseState::kInLineNumber:  if(isspace(*p))  parse_state = ParseState::kInWhitespace2; break;
        case ParseState::kInWhitespace2: if(!isspace(*p)) parse_state = ParseState::kInPackageName; break;
        case ParseState::kInPackageName: if(isspace(*p))  parse_state = ParseState::kInWhitespace3; break;
        case ParseState::kInWhitespace3: if(!isspace(*p)) parse_state = ParseState::kInAddress;     break;
        case ParseState::kInAddress:     if(isspace(*p))  parse_state = ParseState::kInWhitespace4; break;
        case ParseState::kInWhitespace4:
        if(!isspace(*p))
        {
          parse_state = ParseState::kInFunction;
          begin_name = p - 1;
        }
        break;

        case ParseState::kInFunction:
        if(isspace(*p))
        {
          parse_state = ParseState::kInWhitespace5;
          end_name = p;
        }
        break;

        case ParseState::kInWhitespace5:
        if(!isspace(*p))
        {
          if(*p == '+')
            parse_state = ParseState::kInPlus;
          else
          {
            parse_state = ParseState::kInOffset;
            begin_offset = p - 1;
          }
        }
        break;

        case ParseState::kInPlus:        if(isspace(*p)) parse_state = ParseState::kInWhitespace6;  break;
        case ParseState::kInWhitespace6:
        if(!isspace(*p))
        {
          parse_state = ParseState::kInOffset;
          begin_offset = p - 1;
        }
        break;

        case ParseState::kInOffset:
        if(isspace(*p))
        {
          parse_state = ParseState::kInOverrun;
          end_offset = p;
        }
        break;

        case ParseState::kInOverrun:
        break;
      }
    }

    if((parse_state == ParseState::kInOffset) && !end_offset)
      end_offset = p;
  }

  if(begin_name && begin_offset && end_offset && (begin_name < begin_offset))
  {
    *begin_name++ = '\0';
    if(end_name)
      *end_name = '\0';

    *begin_offset++ = '\0';
    *end_offset = '\0';

    // mangled name is now in [begin_name, begin_offset) and caller
    // offset in [begin_offset, end_offset). now apply __cxa_demangle():

    int status = -1;
    std::unique_ptr<char, decltype(&free)> demangled(abi::__cxa_demangle(begin_name, nullptr, nullptr, &status), &free);
    if(demangled && !status)
    {
      output += demangled.get();
      output += " + ";
      output += begin_offset;
    }
    else
    {
      // demangling failed. Output function name as a C function with no arguments.
      output += begin_name;
      output += "() + ";
      output += begin_offset;
    }

    offset = begin_offset;
  }
  else
  {
    // couldn't parse the line? Just print the whole line.
    output += symbol;
  }

  return true;
}

//##################################################################################################
void printStackTrace()
{
  tpWarning() << "Start print stack trace.";

#ifdef TP_PRINT_ADDR2LINE
  printAddr2Line();
#endif

#ifdef TP_ADDR2LINE
  execAddr2Line();
#endif

#ifdef TP_EUADDR2LINE
  execEUAddr2Line();
#endif



  //Get the backtrace
  std::array<void*, MAX_LEVELS> array = tpMakeArray<void*, MAX_LEVELS>(nullptr);

  int startOffset = 1;   // don't include printStackTrace() in the output
#if defined(__mips)
  int size = backtraceMIPS(array, MAX_LEVELS);
  ucontext_t* context = (ucontext_t*)pcontext;
  if(context)
  {
    // When called from the CrashReporter signal handler, the first two frames on the stack are the
    // signal handler and the sigaction() address where the signal handler was called from. Insert
    // the address of the last caller before the signal was generated.
    array[0] = array[1];
    array[1] = array[2];
    array[2] = (void*)context->uc_mcontext.pc;
    startOffset = 0;
  }
#else
  int size = backtrace(array.data(), MAX_LEVELS);
#endif
  std::unique_ptr<char*, decltype(&free)> strings(backtrace_symbols(array.data(), size), &free);
  tpWarning() << "Stack frames: " << size - startOffset;
  for(int i = startOffset; i < size; i++)
  {
    const char* symbol = strings.get()[i];

    //Extract name and address
    std::string demangled;
    std::string offset;
    if(demangle(symbol, demangled, offset))
      tpWarning() << "Frame " << i << ": " << demangled;
    else
      tpWarning() << "Frame " << i << ": " << symbol;
  }
}

//##################################################################################################
std::vector<std::string> addr2Line()
{
  std::vector<std::string> results;

  //Get the backtrace
  std::array<void*, MAX_LEVELS> array = tpMakeArray<void*, MAX_LEVELS>(nullptr);

  int startOffset = 1;   // don't include printStackTrace() in the output
#if defined(__mips)
  int size = backtraceMIPS(array, MAX_LEVELS);
  ucontext_t* context = (ucontext_t*)pcontext;
  if(context)
  {
    // When called from the CrashReporter signal handler, the first two frames on the stack are the
    // signal handler and the sigaction() address where the signal handler was called from. Insert
    // the address of the last caller before the signal was generated.
    array[0] = array[1];
    array[1] = array[2];
    array[2] = (void*)context->uc_mcontext.pc;
    startOffset = 0;
  }
#else
  int size = backtrace(array.data(), MAX_LEVELS);
#endif
  std::unique_ptr<char*, decltype(&free)> strings(backtrace_symbols(array.data(), size), &free);
  for(int i = startOffset; i < size; i++)
  {
    const char* symbol = strings.get()[i];

    std::string demangled;
    std::string offset;
    demangle(symbol, demangled, offset);

    std::vector<std::string> partsA;
    tpSplit(partsA, symbol, '(', TPSplitBehavior::SkipEmptyParts);
    if(partsA.size()==2)
    {
      std::vector<std::string> partsB;
      tpSplit(partsB, partsA.back(), '[', TPSplitBehavior::SkipEmptyParts);
      if(!partsB.empty())
      {
        std::vector<std::string> partsC;
        tpSplit(partsC, partsB.back(), ']', TPSplitBehavior::SkipEmptyParts);

        if(!partsC.empty())
        {
          if(offset.empty())
          {
            std::string output;
            output += " echo '";
            output += symbol;
            output += "'";
            results.push_back(output);
          }
          else
          {
            std::string output;
            output += " addr2line ";
            output += offset;
            output += " -f -C -e ";
            output += partsA.front();
            results.push_back(output);
          }
        }
      }
    }
  }

  return results;
}

//##################################################################################################
void printAddr2Line()
{
  for(const auto& line : addr2Line())
    std::cout << line << std::endl;
}

//##################################################################################################
void execAddr2Line()
{
  auto lines = addr2Line();
  for(size_t l=0; l<lines.size(); l++)
  {
    const auto& line = lines.at(l);
    std::cerr << "Frame " << l << ":" << std::endl;
    std::cerr << line << std::endl;
    if(std::system(line.c_str()) != 0)
      break;
  }
}

//##################################################################################################
void execEUAddr2Line()
{
  //Get the backtrace
  std::array<void*, MAX_LEVELS> array = tpMakeArray<void*, MAX_LEVELS>(nullptr);

  int startOffset = 1;
  int size = backtrace(array.data(), MAX_LEVELS);
  std::unique_ptr<char*, decltype(&free)> strings(backtrace_symbols(array.data(), size), &free);

  for(int i=startOffset; i<size; i++)
  {
    std::cout << "=================================================================================" << std::endl;
    const char* symbol = strings.get()[i];

    std::string demangled;
    std::string offset;
    demangle(symbol, demangled, offset);

    std::string directory;

    std::vector<std::string> partsA;
    tpSplit(partsA, symbol, '(', TPSplitBehavior::SkipEmptyParts);
    if(partsA.size()==2)
       directory = tp_utils::directoryName(partsA.front());

    std::string command;

    {
      char syscom[1024];
      syscom[0] = '\0';
      snprintf(syscom, 1024, "eu-addr2line '%p'", array[i]);
      command = std::string(syscom);
    }

    //command += " --pid=" + std::to_string(getpid());

    if(!directory.empty())
      command += " --debuginfo-path=$(realpath " + directory + ")";

    if(system(command.c_str()) != 0)
      fprintf(stderr, "eu-addr2line failed\n");

    std::cout << "AAAAAAA: " << command << std::endl;
    std::cout << "=================================================================================" << std::endl;
  }


}

//##################################################################################################
std::vector<std::string> stackTraceFrames()
{
  //Get the backtrace
  std::array<void*, MAX_LEVELS> array = tpMakeArray<void*, MAX_LEVELS>(nullptr);

  int startOffset = 1;   // don't include printStackTrace() in the output
  int size = backtrace(array.data(), MAX_LEVELS);

  //Convert the backtrace to strings
  std::unique_ptr<char*, decltype(&free)> strings(backtrace_symbols(array.data(), size), &free);

  std::vector<std::string> results;
  for(int i = startOffset; i < size; i++)
    results.push_back(strings.get()[i]);

  return results;
}

}

#endif
#endif
