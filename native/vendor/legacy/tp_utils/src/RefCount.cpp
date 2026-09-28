#ifdef TP_REF_COUNT

#include "tp_utils/RefCount.h"

#include "tp_utils/StackTrace.h"
#include "tp_utils/DebugUtils.h"

#include "lib_platform/Polyfill.h"

#include <mutex>
#include <queue>

namespace tp_utils
{

namespace
{

//##################################################################################################
class StaticDetails_lt
{
public:
  std::mutex mutex;
  std::unordered_map<tp_utils::StaticStringID, InstanceDetails> instances;

  //################################################################################################
  std::vector<std::string> serialize()
  {
    std::vector<std::string> results;

    mutex.lock();

    std::string title = "Class";
    size_t maxLength = title.size();
    size_t maxDigits = 1;

    for(const auto& i : instances)
    {
      maxLength = tpMax(maxLength, i.first.sid.toString().size());
      maxDigits = tpMax(maxDigits, std::to_string(i.second.total).size());
    }

    {
      std::string count = std::string("#");
      std::string peak = std::string("##");
      std::string total = std::string("###");

      leftJustified(title, maxLength);
      rightJustified(count, maxDigits);
      rightJustified(peak, maxDigits);
      rightJustified(total, maxDigits);

      const std::string line = title + ' ' + count + ' ' + peak + ' ' + total;
      results.push_back(line);
    }

    for(const auto& i : instances)
    {
      title = i.first.sid.toString();
      std::string count = std::to_string(i.second.count);
      std::string peak = std::to_string(i.second.peak);
      std::string total = std::to_string(i.second.total);

      leftJustified(title, maxLength);
      rightJustified(count, maxDigits);
      rightJustified(peak, maxDigits);
      rightJustified(total, maxDigits);

      std::string line = title;
      line += ' ';
      line += count;
      line += ' ';
      line += peak;
      line += ' ';
      line += total;

      results.push_back(line);
    }

    mutex.unlock();

    return results;
  }

  //################################################################################################
  std::map<std::string, size_t> keyValueResults()
  {
    std::map<std::string, size_t> result;

    mutex.lock();
    for(const auto& i : instances)
    {
      auto title = i.first.sid.toString();
      result[title+"_count"] = i.second.count;
      result[title+"_peak"] = i.second.peak;
      result[title+"_total"] = i.second.total;
    }
    mutex.unlock();

    return result;
  }
};

//##################################################################################################
StaticDetails_lt& staticDetails()
{
  // Force order of static init. As StaticDetails_lt relies on the static data of StringID.
  static StaticStringID refCount("Ref count");
  static StaticDetails_lt staticDetails;
  return staticDetails;
}

}

//##################################################################################################
void RefCount::ref(const tp_utils::StaticStringID& type)
{
  StaticDetails_lt& sd(staticDetails());
  sd.mutex.lock();
  InstanceDetails& instanceDetails(sd.instances[type]);
  instanceDetails.count++;
  instanceDetails.peak = std::max(instanceDetails.peak, instanceDetails.count);
  instanceDetails.total++;
  sd.mutex.unlock();
}

//##################################################################################################
void RefCount::unref(const tp_utils::StaticStringID& type)
{
  StaticDetails_lt& sd(staticDetails());
  sd.mutex.lock();
  InstanceDetails& instanceDetails(sd.instances[type]);
  instanceDetails.count--;
  if(instanceDetails.count<0)
  {
    tpWarning() << "RefCount::unref, error! Type: " << type.sid.toString().data();
    printStackTrace();
    abort();
  }
  sd.mutex.unlock();
}

//##################################################################################################
void RefCount::lock()
{
  staticDetails().mutex.lock();
}

//##################################################################################################
void RefCount::unlock()
{
  staticDetails().mutex.unlock();
}

//##################################################################################################
const std::unordered_map<tp_utils::StaticStringID, InstanceDetails>& RefCount::instances()
{
  if(staticDetails().mutex.try_lock())
  {
    staticDetails().mutex.unlock();
    tpWarning() << "You must call RefCount::lock() before RefCount::instances()!";
  }

  return staticDetails().instances;
}

//##################################################################################################
std::vector<std::string> RefCount::serialize()
{
  return staticDetails().serialize();
}

//##################################################################################################
std::string RefCount::takeResults()
{
  std::string result;
  for(const auto& l : serialize())
    result += l + '\n';
  return result;
}

//##################################################################################################
std::map<std::string, size_t> RefCount::keyValueResults()
{
  return staticDetails().keyValueResults();
}

//##################################################################################################
template <typename T, int N>
struct FixedQueue : public std::queue<T> {
  void pop() { std::queue<T>::pop(); }

  void push(const T& value) {
    if (this->size() == N)
      pop();
    std::queue<T>::push(value);
  }

  void push_if_changed(const T& value) {
    if (!this->empty() && this->back() == value)
      return;
    push(value);
  }
};

//##################################################################################################
void refCountHistoryReport()
{
  static std::map<std::string, FixedQueue<size_t, 10>> refCountHistory;

  for (auto entry : tp_utils::RefCount::keyValueResults())
  {
    refCountHistory[entry.first].push_if_changed(entry.second);
    std::stringstream ss;
    ss << "  " << entry.first << ", history: {";
    auto copy = refCountHistory[entry.first];
    while (!copy.empty())
    {
      ss << " " << copy.front();
      copy.pop();
    }
    ss << " }";
    tpWarning() << ss.str();
  }
}

}
#else
extern int tp_utilsRefCount;
int tp_utilsRefCount{0};
#endif
