#ifndef tp_utils_CountStackTrace_h
#define tp_utils_CountStackTrace_h

#include "tp_utils/Globals.h"

namespace tp_utils
{

#ifdef TP_ENABLE_COUNT_STACK_TRACE

//##################################################################################################
struct TP_UTILS_EXPORT CountStackTrace
{
  //################################################################################################
  static void init();

  //################################################################################################
  static void count();

  //################################################################################################
  static void print();
};

#define TP_COUNT_STACK_TRACE tp_utils::CountStackTrace::count()

#else
#define TP_COUNT_STACK_TRACE do{}while(false)
#endif

}

#endif
