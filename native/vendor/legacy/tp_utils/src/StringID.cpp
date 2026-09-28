#include "tp_utils/StringID.h"
#include "tp_utils/MutexUtils.h"
#include "tp_utils/CountStackTrace.h" // IWYU pragma: keep

#include "lib_stduuid/uuid.h"

#include <iostream>
#include <unordered_map>
#include <shared_mutex>

#define TP_SHARED_MUTEX_READ_LOCKER(m)  std::shared_lock<std::shared_mutex> TP_CONCAT(locker, __LINE__)(m); TP_UNUSED(TP_CONCAT(locker, __LINE__))
#define TP_SHARED_MUTEX_WRITE_LOCKER(m) std::unique_lock<std::shared_mutex> TP_CONCAT(locker, __LINE__)(m); TP_UNUSED(TP_CONCAT(locker, __LINE__))


namespace tp_utils
{

namespace
{
//##################################################################################################
struct StringHash_lt
{
  std::string string;
  size_t hash;
};

//##################################################################################################
auto hashStringHash_lt = [](const StringHash_lt& k)
{
  return k.hash;
};

//##################################################################################################
auto eqStringHash_lt = [](const StringHash_lt& a, const StringHash_lt& b)
{
  return a.hash==b.hash && a.string==b.string;
};
}

//##################################################################################################
struct StringID::StaticData
{
  std::shared_mutex mutex{TPM};
  std::unordered_map<StringHash_lt, SharedData*, decltype(hashStringHash_lt), decltype(eqStringHash_lt)> allKeys{128, hashStringHash_lt, eqStringHash_lt};
};

//##################################################################################################
struct StringID::SharedData
{
  TPMutex mutex{TPM};

  StringHash_lt hash;

  int referenceCount{0};

  //################################################################################################
  SharedData(const StringHash_lt& hash_):
    hash(hash_)
  {
  }
};

//##################################################################################################
StringID::StringID():
  sd(nullptr)
{

}

//##################################################################################################
StringID::StringID(const StringID& other):
  sd(other.sd)
{
  attach();
}

//##################################################################################################
StringID::StringID(StringID&& other) noexcept:
  sd(other.sd)
{
  other.sd = nullptr;
}

//##################################################################################################
StringID::StringID(const std::string& string):
  sd(nullptr)
{
  fromString(string);
}

//##################################################################################################
StringID::StringID(const char* string):
  sd(nullptr)
{
  fromString(string);
}

//##################################################################################################
StringID& StringID::operator=(const StringID& other)
{
  if(&other == this || other.sd == sd)
    return *this;

  detach();
  sd = other.sd;
  attach();

  return *this;
}

//##################################################################################################
StringID& StringID::operator=(StringID&& other) noexcept
{
  if(&other == this || other.sd == sd)
    return *this;

  detach();
  sd = other.sd;
  other.sd = nullptr;

  return *this;
}

//##################################################################################################
StringID& StringID::operator=(const char* string)
{
  detach();
  if(string)
    fromString(string);
  else
    fromString("");

  return *this;
}

//##################################################################################################
StringID& StringID::operator=(const std::string& string)
{
  detach();
  fromString(string);

  return *this;
}

//##################################################################################################
StringID::~StringID()
{
  detach();
}

//##################################################################################################
StringID StringID::fromWeak(WeakStringID weak)
{
  StringID stringID;
  stringID.sd = static_cast<SharedData*>(weak);
  stringID.attach();
  return stringID;
}

//##################################################################################################
std::vector<WeakStringID> StringID::toWeak(const std::vector<StringID>& ids)
{
  std::vector<WeakStringID> weak;
  weak.resize(ids.size());

  for(size_t i=0; i<ids.size(); i++)
    weak[i] = ids.at(i).weak();

  return weak;
}

//##################################################################################################
const std::string& StringID::toString() const
{
  static const std::string emptyString;
  if(!sd)
    return emptyString;

  return sd->hash.string;
}

//##################################################################################################
bool StringID::isValid() const
{
  return sd!=nullptr;
}

//##################################################################################################
void StringID::reset()
{
  detach();
}

//##################################################################################################
std::vector<std::string> StringID::toStringList(const std::vector<StringID>& stringIDs)
{
  std::vector<std::string> stringList;
  stringList.reserve(stringIDs.size());

  for(const StringID& stringID : stringIDs)
    stringList.emplace_back(stringID.toString());

  return stringList;
}

//##################################################################################################
std::vector<StringID> StringID::fromStringList(const std::vector<std::string>& stringIDs)
{
  std::vector<StringID> result;
  result.reserve(stringIDs.size());

  const std::string* s = stringIDs.data();
  const std::string* sMax = s + stringIDs.size();
  for(; s<sMax; s++)
    result.emplace_back(*s);

  return result;
}

//##################################################################################################
void StringID::fromString(const std::string& string)
{
  if(string.empty())
    return;

  StringHash_lt hash;
  hash.string = string;
  hash.hash = std::hash<std::string>()(hash.string);

  StaticData& staticData(StringID::staticData(hash.hash));

  // First, take the optimistic path (the hash already exists), where lock is cheaper
  {
    TP_SHARED_MUTEX_READ_LOCKER(staticData.mutex);
    sd = tpGetMapValue(staticData.allKeys, hash);
    if(sd)
    {
      sd->mutex.lock(TPM);
      sd->referenceCount++;
      sd->mutex.unlock(TPM);
      return;
    }
  }

  // In case of failure, take the expensive path
  {
    TP_SHARED_MUTEX_WRITE_LOCKER(staticData.mutex);
    sd = tpGetMapValue(staticData.allKeys, hash);
    if(!sd)
    {
      sd = new SharedData(hash);
      sd->referenceCount++;
      staticData.allKeys[hash] = sd;
    }
    else
    {
      sd->mutex.lock(TPM);
      sd->referenceCount++;
      sd->mutex.unlock(TPM);
    }
  }
}

//##################################################################################################
void StringID::attach()
{
  if(sd)
  {
    TP_MUTEX_LOCKER(sd->mutex);
    sd->referenceCount++;
  }
}

//##################################################################################################
void StringID::detach()
{
  if(sd)
  {
    //TP_FUNCTION_TIME("StringID::detach");

    //if(FunctionTimeStats::isMainThread())
    //  TP_COUNT_STACK_TRACE;

    silentDetachInternal();
  }
}

//##################################################################################################
void StringID::silentDetach()
{
  if(sd)
    silentDetachInternal();
}
//##################################################################################################
void StringID::silentDetachInternal()
{
  {
    TP_MUTEX_LOCKER(sd->mutex);
    if(sd->referenceCount>1)
    {
      sd->referenceCount--;
      sd = nullptr;
      return;
    }
  }

  StaticData& staticData(StringID::staticData(sd->hash.hash));
  TP_SHARED_MUTEX_WRITE_LOCKER(staticData.mutex);

  sd->mutex.lock(TPM);
  sd->referenceCount--;

  //Delete unused shared data
  if(!sd->referenceCount)
  {
    sd->mutex.unlock(TPM);
    staticData.allKeys.erase(sd->hash);
    delete sd;
  }
  else
    sd->mutex.unlock(TPM);

  sd = nullptr;
}

//##################################################################################################
StringID::StaticData& StringID::staticData(size_t hash)
{
  constexpr size_t n{256};
  constexpr size_t m{n-1};
  static StaticData staticData[n];
  return staticData[hash&m];
}

//##################################################################################################
bool lessThanStringID(const StringID& lhs, const StringID& rhs)
{
  return lhs.toString() < rhs.toString();
}

//##################################################################################################
std::string join(const std::vector<StringID>& ids)
{
  return join(ids, ",");
}

//##################################################################################################
std::string TP_UTILS_EXPORT join(const std::vector<StringID>& ids, const std::string& del)
{
  std::string result;
  for(const StringID& id : ids)
  {
    if(!result.empty())
      result += del;
    result += id.toString();
  }
  return result;
}

//##################################################################################################
std::string TP_UTILS_EXPORT join(const std::vector<std::string>& parts, const std::string& del)
{
  std::string result;
  for(const auto& part : parts)
  {
    if(!result.empty())
      result += del;
    result += part;
  }
  return result;
}

namespace
{
struct RNG
{
  lib_platform::RandomDevice rd;
  std::ranlux48_base generator{rd()};
};
}

//##################################################################################################
tp_utils::StringID makeUUID()
{
  static TPMutex mutex{TPM};
  TP_MUTEX_LOCKER(mutex);
  static RNG rng;
  uuids::basic_uuid_random_generator<std::ranlux48_base> gen(&rng.generator);
  const uuids::uuid id = gen();

  tp_utils::StringID sid = uuids::to_string(id);
  return sid;
}

}
//##################################################################################################
void tpSplitSIDs(std::vector<tp_utils::StringID>& result, const std::string& input)
{
  std::string text = input;
  tp_utils::replace(text, "\n", " ");
  tp_utils::replace(text, "\r", " ");
  tp_utils::replace(text, "\t", " ");
  tp_utils::replace(text, ",", " ");
  tp_utils::replace(text, ";", " ");
  tp_utils::replace(text, "\"", " ");
  tp_utils::replace(text, "[", " ");
  tp_utils::replace(text, "]", " ");
  tp_utils::replace(text, "(", " ");
  tp_utils::replace(text, ")", " ");
  tp_utils::replace(text, "{", " ");
  tp_utils::replace(text, "}", " ");

  std::vector<std::string> parts;
  tpSplit(parts, text, ' ', TPSplitBehavior::SkipEmptyParts);

  result.reserve(parts.size());
  for(const auto& part : parts)
    if(tp_utils::StringID id = tpTrim(part); id.isValid())
      result.emplace_back(std::move(id));
}
