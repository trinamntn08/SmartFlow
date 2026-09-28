#pragma once

#include "tp_utils/StringID.h"

#include "json.hpp"

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <optional>


#define TPJSON          tp_utils::getJSON
#define TPJSONString    tp_utils::getJSONString
#define TPJSONInt       tp_utils::getJSONNumber<int>
#define TPJSONSizeT     tp_utils::getJSONNumber<size_t>
#define TPJSONInt64T    tp_utils::getJSONNumber<int64_t>
#define TPJSONUint64T   tp_utils::getJSONNumber<uint64_t>
#define TPJSONUint16T   tp_utils::getJSONNumber<uint16_t>
#define TPJSONFloat     tp_utils::getJSONNumber<float>
#define TPJSONDouble    tp_utils::getJSONNumber<double>
#define TPJSONBool      tp_utils::getJSONBool
#define TPJSONList      tp_utils::getJSONStringList
#define TPJSONStringIDs tp_utils::getJSONStringIDs

namespace tp_utils
{

//##################################################################################################
typedef nlohmann::json JSON;

//##################################################################################################
[[nodiscard]] std::string TP_UTILS_EXPORT stringFromJSON(const JSON& json);

//##################################################################################################
[[nodiscard]] JSON TP_UTILS_EXPORT jsonFromString(const std::string& json);

//##################################################################################################
[[nodiscard]]std::shared_ptr<JSON> TP_UTILS_EXPORT jsonPtrFromString(const std::string& json);

//##################################################################################################
[[nodiscard]] const JSON& TP_UTILS_EXPORT getJSON(const JSON& j,
                                                            const std::string& key);

//##################################################################################################
[[nodiscard]] float TP_UTILS_EXPORT getJSONFloat(const JSON& j,
                                                 const std::string& key,
                                                 float defaultValue=JSON());

//##################################################################################################
template<typename T>
[[nodiscard]] T getJSONNumber(const JSON& j,
                              const std::string& key,
                              const T& defaultValue=T())
{
  if(const auto i = j.find(key); i != j.end() && i->is_number())
    return i->get<T>();

  return defaultValue;
}


//##################################################################################################
void TP_UTILS_EXPORT getJSONStringID(const JSON& j,
                                     const std::string& key,
                                     StringID& result);

//##################################################################################################
[[nodiscard]] std::string TP_UTILS_EXPORT getJSONString(const JSON& j,
                                                        const std::string& key,
                                                        const std::string& defaultValue=std::string());

//##################################################################################################
[[nodiscard]] bool TP_UTILS_EXPORT getJSONBool(const JSON& j,
                                               const std::string& key,
                                               const bool& defaultValue=bool());

//##################################################################################################
void TP_UTILS_EXPORT getJSONStringList(const JSON& j,
                                       const std::string& key,
                                       std::vector<std::string>& result);

//##################################################################################################
[[nodiscard]] std::vector<std::string> TP_UTILS_EXPORT getJSONStringList(const JSON& j,
                                                                         const std::string& key);

//##################################################################################################
[[nodiscard]] std::vector<StringID> TP_UTILS_EXPORT getJSONStringIDs(const JSON& j,
                                                                     const std::string& key);

//##################################################################################################
void TP_UTILS_EXPORT loadVectorOfStringIDsFromJSON(const JSON& j,
                                                   const std::string& key,
                                                   std::vector<StringID>& stringIDs);

//##################################################################################################
void loadVectorOfStringIDsFromJSON(const JSON& j,
                                   std::vector<StringID>& stringIDs);

//##################################################################################################
void TP_UTILS_EXPORT loadUnorderedSetOfStringIDsFromJSON(const JSON& j,
                                                   const std::string& key,
                                                   std::unordered_set<StringID>& stringIDs);

//##################################################################################################
void loadUnorderedSetOfStringIDsFromJSON(const JSON& j,
                                   std::unordered_set<StringID>& stringIDs);

//##################################################################################################
[[nodiscard]] JSON stringIDsToJSON(const std::vector<StringID>& stringIDs);

//##################################################################################################
void saveVectorOfStringIDsToJSON(JSON& j, const std::vector<StringID>& stringIDs);

//##################################################################################################
void saveUnorderedSetOfStringIDsToJSON(JSON& j, const std::unordered_set<StringID>& stringIDs);

//##################################################################################################
void saveVectorOfStringsToJSON(JSON& j, const std::vector<std::string>& strings);

//##################################################################################################
void saveMapOfStringIDAndStringToJSON(JSON& j, const std::unordered_map<StringID, std::string>& map);

//##################################################################################################
void saveMapOfStringIDAndStringIDToJSON(JSON& j, const std::unordered_map<StringID, StringID>& map);

//##################################################################################################
void saveMapOfStringIDAndStringIDToJSON(JSON& j, const std::unordered_map<StringID, StringID>& map);

//##################################################################################################
void saveMapOfStringIDAndFloatToJSON(JSON& j, const std::unordered_map<StringID, float>& map);

//##################################################################################################
void loadMapOfStringIDAndStringFromJSON(const JSON& j, std::unordered_map<StringID, std::string>& map);

//##################################################################################################
void loadMapOfStringIDAndStringIDFromJSON(const JSON& j, std::unordered_map<StringID, StringID>& map);

//##################################################################################################
void loadMapOfStringIDAndFloatFromJSON(const JSON& j, std::unordered_map<StringID, float>& map);

//##################################################################################################
void loadMapOfStringIDAndStringFromJSON(const JSON& j,
                                        const std::string& key,
                                        std::unordered_map<StringID, std::string>& map);

//##################################################################################################
void loadMapOfStringIDAndStringIDFromJSON(const JSON& j,
                                          const std::string& key,
                                          std::unordered_map<StringID, StringID>& map);

//##################################################################################################
void loadMapOfStringIDAndFloatFromJSON(const JSON& j,
                                       const std::string& key,
                                       std::unordered_map<StringID, float>& map);

//##################################################################################################
void loadVectorOfStringsFromJSON(const JSON& j, const std::string& key, std::vector<std::string>& vector);

//##################################################################################################
template<typename T, typename = std::enable_if_t<!std::is_pointer<T>::value>>
void loadObjectFromJSON(const JSON& j, const char* key, T& object)
{
  if(auto i=j.find(key); i!=j.end())
    object.loadState(*i);
  else
    object = T();
}

//##################################################################################################
template<typename T, typename... Args, typename = std::enable_if_t<!std::is_pointer<T>::value>>
void loadObjectFromJSONArgs(const JSON& j, const char* key, T& object, Args... args)
{
  if(auto i=j.find(key); i!=j.end())
    object.loadState(*i, args...);
  else
    object = T();
}

//##################################################################################################
template<typename T, typename = std::enable_if_t<std::is_pointer<T>::value>>
void loadObjectFromJSON(const JSON& j, const char* key, T object)
{
  if(auto i=j.find(key); i!=j.end())
    object->loadState(*i);
  else
    object->loadState({});
}


//##################################################################################################
template<typename T>
void loadOptionalObjectFromJSON(const JSON& j, const char* key, std::optional<T>& object)
{
  if(auto i=j.find(key); i!=j.end())
  {
    object = std::make_optional<T>();
    object->loadState(*i);
  }
  else
    object = T();
}

//##################################################################################################
template<typename T>
void saveOptionalObjectToJSON(JSON& j, const char* key, const std::optional<T>& object)
{
  if(object)
    object->saveState(j[key]);
}

//##################################################################################################
template<typename T>
void saveVectorOfValuesToJSON(JSON& j, const T& vector)
{
  j = JSON::array();
  j.get_ptr<JSON::array_t*>()->reserve(vector.size());
  for(const auto& i : vector)
    j.push_back(i);
}

//##################################################################################################
template<typename T>
void loadVectorOfNumbersFromJSON(const JSON& j, const std::string& key, T& vector)
{
  vector.clear();
  if(auto i=j.find(key); i!=j.end() && i->is_array())
  {
    vector.reserve(i->size());
    for(const auto& v : *i)
      if(v.is_number())
        vector.emplace_back(v.get<typename T::value_type>());
  }
}

//##################################################################################################
template<typename T>
void saveVectorOfObjectsToJSON(JSON& j, const T& vector)
{
  j = JSON::array();
  j.get_ptr<JSON::array_t*>()->reserve(vector.size());
  for(const auto& i : vector)
  {
    j.emplace_back();
    i.saveState(j.back());
  }
}

//##################################################################################################
template<typename T>
void loadVectorOfObjectsFromJSON(const JSON& j, T& vector)
{
  vector.clear();
  if(j.is_array())
  {
    vector.reserve(j.size());
    for(const auto& v : j)
      vector.emplace_back().loadState(v);
  }
}

//##################################################################################################
template<typename T, typename K>
void loadVectorOfObjectsFromJSON(const JSON& j, K key, T& vector)
{
  vector.clear();
  if(auto i=j.find(key); i!=j.end() && i->is_array())
  {
    vector.reserve(i->size());
    for(const auto& v : *i)
      vector.emplace_back().loadState(v);
  }
}

//##################################################################################################
template<typename T, typename... Args>
void loadVectorOfObjectsFromJSONArgs(const JSON& j, std::vector<T>& vector, Args... args)
{
  vector.clear();
  if(j.is_array())
  {
    vector.reserve(j.size());
    for(const auto& v : j)
      vector.emplace_back().loadState(v, args...);
  }
}

//##################################################################################################
template<typename T, typename K, typename... Args>
void loadVectorOfObjectsFromJSONArgs(const JSON& j, K key, std::vector<T>& vector, Args... args)
{
  vector.clear();
  if(auto i=j.find(key); i!=j.end() && i->is_array())
  {
    vector.reserve(i->size());
    for(const auto& v : *i)
      vector.emplace_back().loadState(v, args...);
  }
}

//##################################################################################################
template<typename T>
void saveMapOfObjectsToJSON(JSON& j, const T& map)
{
  j = JSON::object();
  for(const auto& i : map)
    i.second.saveState(j[i.first.toString()]);
}

//##################################################################################################
template<typename T>
void loadMapOfObjectsFromJSON(const JSON& j, T& map)
{
  map.clear();
  if(j.is_object())
  {
    map.reserve(j.size());
    for(auto p=j.begin(); p!=j.end(); ++p)
      map[p.key()].loadState(p.value());
  }
}

//##################################################################################################
template<typename T, typename K>
void loadMapOfObjectsFromJSON(const JSON& j, K key, T& map)
{
  map.clear();
  if(auto i=j.find(key); i!=j.end() && i->is_object())
  {
    map.reserve(i->size());
    for(auto p=i->begin(); p!=i->end(); ++p)
      map[p.key()].loadState(p.value());
  }
}

}
