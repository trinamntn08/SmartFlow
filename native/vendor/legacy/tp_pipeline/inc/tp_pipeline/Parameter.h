#pragma once

#include "tp_pipeline/Globals.h"

#include "tp_utils/JSONUtils.h"

#include "base64.h"

#include "lib_platform/Format.h"

#include <variant>
#include <unordered_map>

namespace tp_pipeline
{

//##################################################################################################
typedef std::variant<
std::monostate,
bool,
int,
size_t,
float,
double,
std::string,
std::vector<std::string>
> Variant;

}

//##################################################################################################
template<typename T>
T tpGetVariantValue(const tp_pipeline::Variant& v, T defaultValue={})
{
  if(auto p = std::get_if<T>(&v))
    return *p;
  return defaultValue;
}

namespace tp_pipeline
{

//##################################################################################################
struct Parameter
{
  tp_utils::StringID name;

  /*!
  The data type of this parameter:
   - Int
   - Size       - size_t
   - Float
   - Double
   - String
   - Bool
   - Enum       - Put a string list into step
   - Directory
   - File
   - Named data
   - Binary data
  */
  tp_utils::StringID type;

  Variant value;
  Variant min;
  Variant max;
  Variant step;

  //! The description of what this parameter does
  std::string description;

  //! Enable mutation of value in genetic algorithm
  bool mutate{false};

  //! Depending on the mode that a step is in some parameters are not relevant
  bool enabled{true};

  //################################################################################################
  Parameter();

  //################################################################################################
  Parameter(const tp_utils::JSON& j);

  //################################################################################################
  Parameter(const tp_utils::JSON& j, const std::vector<std::string>& blobs);
  
  //################################################################################################
  tp_utils::JSON save() const;
  
  //################################################################################################
  void load(const tp_utils::JSON& j);

  //################################################################################################
  void saveBinary(tp_utils::JSON& j, const std::function<uint64_t(const std::string&)>& addBlob) const;

  //################################################################################################
  void loadBinary(const tp_utils::JSON& j, const std::vector<std::string>& blobs);

  //################################################################################################
  static tp_utils::JSON saveHash(const std::unordered_map<tp_utils::StringID, tp_pipeline::Parameter>& paramHash);

  //################################################################################################
  static std::unordered_map<tp_utils::StringID, tp_pipeline::Parameter> loadHash(const tp_utils::JSON& j);

  //################################################################################################
  template<typename T>
  void validateBounds(const T& initial=T())
  {
    if(value.index() == 0)
      value = initial;
    else
      value = tpBound(tpGetVariantValue<T>(min),
                      tpGetVariantValue<T>(value),
                      tpGetVariantValue<T>(max));
  }

  //################################################################################################
  void setEnum(const std::vector<std::string>& validValues)
  {
    type = enumSID();
    step = validValues;
    if(!tpContains(validValues, tpGetVariantValue<std::string>(value)) && !validValues.empty())
      value = validValues.at(0);
  }

  //################################################################################################
  void setEnum(const std::vector<std::string>& validValues, const std::string& defaultValue)
  {
    type = enumSID();
    step = validValues;
    if(!tpContains(validValues, tpGetVariantValue<std::string>(value)))
      value = defaultValue;
  }

  //################################################################################################
  void setBoolYesNo(bool defaultValue)
  {
    setEnum({"Yes", "No"}, defaultValue?"Yes":"No");
  }

  //################################################################################################
  template<typename T>
  void setInitial(const T& initial=T())
  {
    if(value.index() == 0)
      value = initial;
  }

  //################################################################################################
  void setNamedData();
};

template <typename T>
T getParameterValue(const std::unordered_map<tp_utils::StringID, Parameter>& parameters,
                    const tp_utils::StringID& key,
                    const T& defaultValue=T())
{
  Parameter param = tpGetMapValue(parameters, key);
  return tpGetVariantValue<T>(param.value, defaultValue);
}

//##################################################################################################
inline std::string variantToString(const tp_pipeline::Variant& v)
{
  switch(v.index())
  {
  case 0: // std::monostate
    return "blank";

  case 1: // bool
    return tpGetVariantValue<bool>(v)?"true":"false";

  case 2: // int
    return std::to_string(tpGetVariantValue<int>(v));

  case 3: // size_t
    return std::to_string(tpGetVariantValue<size_t>(v));

  case 4: // float
    return lib_platform::toString(tpGetVariantValue<float>(v));

  case 5: // double
    return lib_platform::toString(tpGetVariantValue<double>(v));

  case 6: // std::string
    return tpGetVariantValue<std::string>(v);

  case 7: // std::vector<std::string>
  {
    std::string result;
    for(const auto& i : tpGetVariantValue<std::vector<std::string>>(v))
    {
      if(!result.empty())
        result.push_back(',');
      result += i;
    }
    return result;
  }
  }

  return "";
}

//##################################################################################################
inline Variant variantFromJSON(const tp_utils::JSON& j)
{
  Variant v;

  std::string type = TPJSONString(j, "type", "blank");

  if(type == "bool")
  {
    std::string value = TPJSONString(j, "value", "false");
    v = bool((value=="true")?true:false);
  }
  else if(type == "int")
  {
    v = TPJSONInt(j, "value", 0);
  }
  else if(type == "size_t")
  {
    v = TPJSONSizeT(j, "value", 0);
  }
  else if(type == "float")
  {
    std::string hex = TPJSONString(j, "bin", std::string());
    std::string str = tpFromHEX(hex);

    float value=0.0f;
    if(str.size() == 4)
      memcpy(&value, str.data(), 4);
    v = value;
  }
  else if(type == "double")
  {
    std::string hex = TPJSONString(j, "bin", std::string());
    std::string str = tpFromHEX(hex);

    double value=0.0;
    if(str.size() == 8)
      memcpy(&value, str.data(), 8);
    v = value;  \
  }
  else if(type == "string")
  {
    v = TPJSONString(j, "value", std::string());
  }
  else if(type == "binary")
  {
    v = base64_decode(TPJSONString(j, "value", std::string()));
  }
  else if(type == "list")
  {
    if(auto i = j.find("value"); i != j.end() && i->is_array())
    {
      std::vector<std::string> list;
      for(const tp_utils::JSON& j : *i)
        if(j.is_string())
          list.push_back(j);
      v = list;
    }
    else
      v = std::monostate();
  }
  else
  {
    v = std::monostate();
  }

  return v;
}

//##################################################################################################
inline Variant variantFromJSON(const tp_utils::JSON& j, const std::string& key)
{
  if(auto i = j.find(key); i != j.end())
    return variantFromJSON(i.value());

  return Variant();
}

}

namespace std
{

//##################################################################################################
inline void to_json(tp_utils::JSON& j, const tp_pipeline::Variant& v)
{
  switch(v.index())
  {
  case 0: // std::monostate
    j["type"]  = "blank";
    break;

  case 1: // bool
    j["type"]  = "bool";
    j["value"] = tpGetVariantValue<bool>(v)?"true":"false";
    break;

  case 2: // int
    j["type"]  =    "int";
    j["value"] = tpGetVariantValue<int>(v);
    break;

  case 3: // size_t
    j["type"]  =    "size_t";
    j["value"] = tpGetVariantValue<size_t>(v);
    break;

  case 4: // float
  {
    j["type"]  =  "float";
    float value = tpGetVariantValue<float>(v);
    std::string str;
    str.resize(4);
    memcpy(&str[0], &value, 4);
    j["bin"] = tpToHex(str);
    j["pretty"] = std::to_string(value);
    break;
  }

  case 5: // double
  {
    j["type"]  = "double";
    double value = tpGetVariantValue<double>(v);
    std::string str;
    str.resize(8);
    memcpy(&str[0], &value, 8);
    j["bin"] = tpToHex(str);
    j["pretty"] = std::to_string(value);
    break;
  }

  case 6: // std::string
  {
    const auto value = tpGetVariantValue<std::string>(v);
    bool binary=false;
    {
      const char* i = value.data();
      auto iMax = i+value.size();

      for(; i<iMax; i++)
      {
        if(((*i)<' ' || (*i)>'~'))
        {
          binary=true;
          break;
        }
      }
    }

    if(binary)
    {
      j["type"]   = "binary";
      j["value"]  = base64_encode(reinterpret_cast<unsigned char const*>(value.data()), value.size());
    }
    else
    {
      j["type"]   = "string";
      j["value"]  = value;
    }

    break;
  }

  case 7: // std::vector<std::string>
    j["type"]  =   "list";
    j["value"]  = tp_utils::JSON::array();
    for(const auto& i : tpGetVariantValue<std::vector<std::string>>(v))
      j["value"].push_back(i);
    break;
  }
}

}
