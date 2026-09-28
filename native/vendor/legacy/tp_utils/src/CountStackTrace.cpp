#include "tp_utils/CountStackTrace.h"
#include "tp_utils/detail/StaticState.h"// IWYU pragma: keep

#include <map>

namespace tp_utils
{
#ifdef TP_ENABLE_COUNT_STACK_TRACE

namespace
{
//##################################################################################################
StaticState::CountStackTraceStats& countStackTraceStats()
{
  auto i=StaticState::instance();
  return i->countStackTraceStats;
}
}

//##################################################################################################
void CountStackTrace::init()
{
  auto& s = countStackTraceStats();
  TP_MUTEX_LOCKER(s.mutex);
}

//##################################################################################################
void CountStackTrace::count()
{
  auto& s = countStackTraceStats();
  TP_MUTEX_LOCKER(s.mutex);
  s.counts[StackTrace()]++;
}

//##################################################################################################
void CountStackTrace::print()
{
  std::vector<std::pair<std::string, size_t>> counts;

  {
    auto& s = countStackTraceStats();
    TP_MUTEX_LOCKER(s.mutex);
    counts.reserve(s.counts.size());

    for(const auto& p : s.counts)
      counts.emplace_back(formatStackTrace(p.first.frames()), p.second);
  }

  std::sort(counts.begin(), counts.end(), [](const auto& a, const auto& b)
  {
    return a.second < b.second;
  });

  for(const auto& count : counts)
  {
    tpWarning() << "------------------------------------------------------------------------------";
    tpWarning() << "Count: " << count.second << '\n' << count.first;
    tpWarning() << "------------------------------------------------------------------------------";
  }
}
#endif

}
