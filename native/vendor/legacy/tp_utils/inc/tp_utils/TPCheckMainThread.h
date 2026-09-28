#pragma once

#include "tp_utils/DebugUtils.h"
#include "tp_utils/StackTrace.h"

#include <thread>

//##################################################################################################
class TP_UTILS_EXPORT TPCheckMainThread
{
  std::thread::id m_mainThreadID{std::this_thread::get_id()};
public:

  //################################################################################################
  template<typename T>
  void operator()(T caller) const
  {
    if(m_mainThreadID != std::this_thread::get_id())
    {
      tpWarning() << caller << " called from wrong thread!";
      tpWarning() << "mainThreadID : " << m_mainThreadID <<std::endl;
      tpWarning() << "this_thread : "  << std::this_thread::get_id()<<std::endl;
      tp_utils::printStackTrace();
      assert(false);
      abort();
    }
  }

  //################################################################################################
  bool isMainThread() const
  {
    return m_mainThreadID == std::this_thread::get_id();
  }
};











