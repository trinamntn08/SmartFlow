#include "tp_utils/JSONUtils.h"

namespace tp_utils
{

//##################################################################################################
std::string stringFromJSON(const JSON& json)
{
  return nlohmann::to_string(json);
}

//##################################################################################################
JSON jsonFromString(const std::string& json)
{
  JSON j = JSON::parse(json, nullptr, /*allow exceptions*/false);
  if(j.is_discarded())
    return JSON();

  return j;
}

//##################################################################################################
std::shared_ptr<JSON> jsonPtrFromString(const std::string& json)
{
  auto j = std::make_shared<JSON>(JSON::parse(json, nullptr, /*allow exceptions*/false));
  if (j->is_discarded())
    return std::make_shared<JSON>();

  return j;
}

//##################################################################################################
const JSON& getJSON(const JSON& j,
                              const std::string& key)
{  
  if(auto i = j.find(key); i != j.end())
    return i.value();

  static const JSON defaultValue;
  return defaultValue;
}

//##################################################################################################
void TP_UTILS_EXPORT getJSONStringID(const JSON& j,
                                     const std::string& key,
                                     StringID& result)
{
  if(auto i=j.find(key); i!=j.end() && i->is_string())
    result = i->get<std::string>();
  else
    result = StringID();
}

//##################################################################################################
std::string getJSONString(const JSON& j,
                          const std::string& key,
                          const std::string& defaultValue)
{
  if(auto i = j.find(key); i!=j.end() && i->is_string())
    return i->get<std::string>();

  return defaultValue;
}

//##################################################################################################
bool getJSONBool(const JSON& j,
                 const std::string& key,
                 const bool& defaultValue)
{
  if(const auto i = j.find(key); i != j.end() && i->is_boolean())
    return i->get<bool>();

  return defaultValue;
}

//##################################################################################################
void TP_UTILS_EXPORT getJSONStringList(const JSON& j,
                                       const std::string& key,
                                       std::vector<std::string>& result)
{
  result.clear();
  if(auto i=j.find(key); i!=j.end() && i->is_array())
  {
    result.reserve(i->size());
    for(const JSON& s : *i)
      if(s.is_string())
        result.push_back(s);
  }
}

//##################################################################################################
std::vector<std::string> getJSONStringList(const JSON& j,
                                           const std::string& key)
{
  std::vector<std::string> result;
  getJSONStringList(j, key, result);
  return result;
}

//##################################################################################################
std::vector<StringID> getJSONStringIDs(const JSON& j,
                                       const std::string& key)
{
  std::vector<StringID> ids;
  if(auto i = j.find(key); i!=j.end() && i.value().is_array())
  {
    ids.reserve(i->size());
    for(const auto& jj : i.value())
    {
      if(jj.is_string())
      {
        std::string str = jj;
        ids.emplace_back(str);
      }
    }
  }

  return ids;
}

//##################################################################################################
void loadVectorOfStringIDsFromJSON(const JSON& j,
                                   const std::string& key,
                                   std::vector<StringID>& stringIDs)
{
  stringIDs.clear();
  if(const auto& i = j.find(key); i != j.end() && !i->empty())
  {
    if(i->is_array())
    {
      stringIDs.reserve(i->size());
      for(const auto& jj : *i)
        stringIDs.emplace_back(jj.get<std::string>());
    }
  }
}

//##################################################################################################
void loadVectorOfStringIDsFromJSON(const JSON& j,
                                   std::vector<StringID>& stringIDs)
{
  stringIDs.clear();
  if(j.is_array())
  {
    stringIDs.reserve(j.size());
    for(const auto& jj : j)
      stringIDs.emplace_back(jj.get<std::string>());
  }
}

//##################################################################################################
void TP_UTILS_EXPORT loadUnorderedSetOfStringIDsFromJSON(const JSON& j,
                                                         const std::string& key,
                                                         std::unordered_set<StringID>& stringIDs)
{
  stringIDs.clear();
  if(const auto& i = j.find(key); i != j.end() && !i->empty())
  {
    if(i->is_array())
    {
      stringIDs.reserve(i->size());
      for(const auto& jj : *i)
        stringIDs.insert(jj.get<std::string>());
    }
  }
}

//##################################################################################################
void loadUnorderedSetOfStringIDsFromJSON(const JSON& j,
                                         std::unordered_set<StringID>& stringIDs)
{
  stringIDs.clear();
  if(j.is_array())
  {
    stringIDs.reserve(j.size());
    for(const auto& jj : j)
      stringIDs.insert(jj.get<std::string>());
  }
}

//##################################################################################################
JSON stringIDsToJSON(const std::vector<StringID>& stringIDs)
{
  JSON j;
  saveVectorOfStringIDsToJSON(j, stringIDs);
  return j;
}

//##################################################################################################
void saveVectorOfStringIDsToJSON(JSON& j, const std::vector<StringID>& stringIDs)
{
  j=JSON::array();
  j.get_ptr<JSON::array_t*>()->reserve(stringIDs.size());
  for(const auto& stringID : stringIDs)
    j.emplace_back(stringID.toString());
}

//##################################################################################################
void saveUnorderedSetOfStringIDsToJSON(JSON& j, const std::unordered_set<StringID>& stringIDs)
{
  j=JSON::array();
  j.get_ptr<JSON::array_t*>()->reserve(stringIDs.size());
  for(const auto& stringID : stringIDs)
    j.emplace_back(stringID.toString());
}

//##################################################################################################
void saveVectorOfStringsToJSON(JSON& j, const std::vector<std::string>& strings)
{
  saveVectorOfValuesToJSON(j, strings);
}

//##################################################################################################
void saveMapOfStringIDAndStringToJSON(JSON& j, const std::unordered_map<StringID, std::string>& map)
{
  j = JSON::object();
  for(const auto& i : map)
    j[i.first.toString()] = i.second;
}

//##################################################################################################
void saveMapOfStringIDAndStringIDToJSON(JSON& j, const std::unordered_map<StringID, StringID>& map)
{
  j = JSON::object();
  for(const auto& i : map)
    j[i.first.toString()] = i.second.toString();
}

//##################################################################################################
void saveMapOfStringIDAndFloatToJSON(JSON& j, const std::unordered_map<StringID, float>& map)
{
  j = JSON::object();
  for(const auto& i : map)
    j[i.first.toString()] = i.second;
}

//##################################################################################################
void loadMapOfStringIDAndStringFromJSON(const JSON& j, std::unordered_map<StringID, std::string>& map)
{
  map.clear();
  if(j.is_object())
  {
    map.reserve(j.size());
    for(auto p=j.begin(); p!=j.end(); ++p)
      if(p->is_string())
        map[p.key()] = p->get<std::string>();
  }
}

//##################################################################################################
void loadMapOfStringIDAndStringIDFromJSON(const JSON& j, std::unordered_map<StringID, StringID>& map)
{
  map.clear();
  if(j.is_object())
  {
    map.reserve(j.size());
    for(auto p=j.begin(); p!=j.end(); ++p)
      if(p->is_string())
        map[p.key()] = p->get<std::string>();
  }
}

//##################################################################################################
void loadMapOfStringIDAndFloatFromJSON(const JSON& j, std::unordered_map<StringID, float>& map)
{
  map.clear();
  if(j.is_object())
  {
    map.reserve(j.size());
    for(auto p=j.begin(); p!=j.end(); ++p)
      if(p->is_number())
        map[p.key()] = p->get<float>();
  }
}

//##################################################################################################
void loadMapOfStringIDAndStringFromJSON(const JSON& j,
                                        const std::string& key,
                                        std::unordered_map<StringID, std::string>& map)
{
  map.clear();
  if(const auto& i = j.find(key); i != j.end() && !i->empty())
  {
    if(i->is_object())
    {
      map.reserve(i->size());
      for(auto p=i->begin(); p!=i->end(); ++p)
        if(p->is_string())
          map[p.key()] = p->get<std::string>();
    }
  }
}

//##################################################################################################
void loadMapOfStringIDAndStringIDFromJSON(const JSON& j,
                                          const std::string& key,
                                          std::unordered_map<StringID, StringID>& map)
{
  map.clear();
  if(const auto& i = j.find(key); i != j.end() && !i->empty())
  {
    if(i->is_object())
    {
      map.reserve(i->size());
      for(auto p=i->begin(); p!=i->end(); ++p)
        if(p->is_string())
          map[p.key()] = p->get<std::string>();
    }
  }
}

//##################################################################################################
void loadMapOfStringIDAndFloatFromJSON(const JSON& j,
                                       const std::string& key,
                                       std::unordered_map<StringID, float>& map)
{
  map.clear();
  if(const auto& i = j.find(key); i != j.end() && !i->empty())
  {
    if(i->is_object())
    {
      map.reserve(i->size());
      for(auto p=i->begin(); p!=i->end(); ++p)
        if(p->is_number())
          map[p.key()] = p->get<float>();
    }
  }
}

//##################################################################################################
void loadVectorOfStringsFromJSON(const JSON& j, const std::string& key, std::vector<std::string>& vector)
{
  vector.clear();
  if(auto i=j.find(key); i!=j.end() && i->is_array())
  {
    vector.reserve(i->size());
    for(const auto& v : *i)
      if(v.is_string())
        vector.emplace_back(v.get<std::string>());
  }
}

}
