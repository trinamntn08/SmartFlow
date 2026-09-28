#include "lib_platform/RandomDevice.h"

#include <chrono>
#include <mutex>

namespace lib_platform
{

//##################################################################################################
RandomDevice::RandomDevice():
  m_seed(initSeq())
{
}

//##################################################################################################
RandomDevice::result_type RandomDevice::operator()()
{
  return std::random_device::operator()() ^ m_seed;
}

//##################################################################################################
RandomDevice::result_type RandomDevice::initMT()
{
  std::random_device rd;
  return rd() ^ result_type(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
}

//##################################################################################################
RandomDevice::result_type RandomDevice::initSeq()
{
  static std::mutex mutex;
  static std::mt19937 mt(initMT());
  std::lock_guard lg(mutex);
  return mt();
}

}
