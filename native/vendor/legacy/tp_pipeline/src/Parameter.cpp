#include "tp_pipeline/Parameter.h"

#include "tp_utils/JSONUtils.h"

namespace tp_pipeline
{

//##################################################################################################
Parameter::Parameter()
{

}

//##################################################################################################
Parameter::Parameter(const tp_utils::JSON& j)
{
  load(j);
}

//##################################################################################################
Parameter::Parameter(const tp_utils::JSON& j, const std::vector<std::string>& blobs)
{
  loadBinary(j, blobs);
}

//##################################################################################################
tp_utils::JSON Parameter::save()const
{
  tp_utils::JSON j;

  j["Name"]        = name.toString();
  j["Type"]        = type.toString();
  j["Value"]       = value           ;
  j["Min"]         = min             ;
  j["Max"]         = max             ;
  j["Step"]        = step            ;
  j["Mutate"]      = mutate          ;
  j["Description"] = description     ;

  return j;
}

//##################################################################################################
void Parameter::load(const tp_utils::JSON& j)
{
  name        = TPJSONString(j, "Name");
  type        = TPJSONString(j, "Type");
  mutate      = TPJSONBool  (j, "Mutate");
  description = TPJSONString(j, "Description");

  value = variantFromJSON(j, "Value");
  min   = variantFromJSON(j, "Min"  );
  max   = variantFromJSON(j, "Max"  );
  step  = variantFromJSON(j, "Step" );
}

//##################################################################################################
void Parameter::saveBinary(tp_utils::JSON& j, const std::function<uint64_t(const std::string&)>& addBlob) const
{
  j["Name"]        = name.toString();
  j["Type"]        = type.toString();
  j["Min"]         = min             ;
  j["Max"]         = max             ;
  j["Step"]        = step            ;
  j["Mutate"]      = mutate          ;
  j["Description"] = description     ;

  if(type == binaryDataSID())
  {
    j["Value"]["type"] = "size_t";
    j["Value"]["value"] = size_t(addBlob(tpGetVariantValue<std::string>(value)));
  }
  else
    j["Value"] = value;
}

//##################################################################################################
void Parameter::loadBinary(const tp_utils::JSON& j, const std::vector<std::string>& blobs)
{
  name        = TPJSONString(j, "Name"       );
  type        = TPJSONString(j, "Type"       );
  mutate      = TPJSONBool  (j, "Mutate"     );
  description = TPJSONString(j, "Description");

  value = variantFromJSON(j, "Value");
  min   = variantFromJSON(j, "Min"  );
  max   = variantFromJSON(j, "Max"  );
  step  = variantFromJSON(j, "Step" );

  if(type == binaryDataSID())
  {
    size_t index = tpGetVariantValue<size_t>(value);
    if(index<blobs.size())
      value = blobs.at(index);
  }
}

//##################################################################################################
tp_utils::JSON Parameter::saveHash(const std::unordered_map<tp_utils::StringID, tp_pipeline::Parameter>& paramHash)
{
  tp_utils::JSON j = tp_utils::JSON::array();

  for(const auto& v : paramHash)
    j.push_back(v.second.save());

  return j;
}

//##################################################################################################
std::unordered_map<tp_utils::StringID, tp_pipeline::Parameter> Parameter::loadHash(const tp_utils::JSON& j)
{
  std::unordered_map<tp_utils::StringID, tp_pipeline::Parameter> paramHash;
  if(j.is_array())
  {
    for(const auto& jj : j)
    {
      Parameter param(jj);
      if(param.name.isValid())
        paramHash[param.name] = param;
    }
  }

  return paramHash;
}

//##################################################################################################
void Parameter::setNamedData()
{
  type = tp_pipeline::namedDataSID();
  if(value.index() == 0)
    value = std::string("Output data");
}

}
