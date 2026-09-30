#include "tp_math_utils/materials/ExternalMaterial.h"

namespace tp_math_utils
{

//##################################################################################################
ExternalMaterialPayload::~ExternalMaterialPayload() = default;

//##################################################################################################
void ExternalMaterialPayload::saveState(tp_utils::JSON& j) const
{
  TP_UNUSED(j);
}

//##################################################################################################
void ExternalMaterialPayload::loadState(const tp_utils::JSON& j)
{
  TP_UNUSED(j);
}

//##################################################################################################
ExternalMaterial::ExternalMaterial() = default;

//##################################################################################################
ExternalMaterial::ExternalMaterial(const ExternalMaterial& other)
{
  jPayload = tp_utils::JSON::object();
  if(other.externalMaterialPayload)
    other.externalMaterialPayload->saveState(jPayload);
  else
    jPayload = other.jPayload;

  assetType = other.assetType;
  subPath = other.subPath;
}

//##################################################################################################
ExternalMaterial::ExternalMaterial(ExternalMaterial&& other) noexcept
{
  externalMaterialPayload = std::move(other.externalMaterialPayload);
  jPayload.swap(other.jPayload);

  assetType = std::move(other.assetType);
  subPath = std::move(other.subPath);
}

//##################################################################################################
ExternalMaterial& ExternalMaterial::operator=(const ExternalMaterial& other)
{
  if(&other != this)
  {
    externalMaterialPayload = {};
    jPayload = tp_utils::JSON::object();
    if(other.externalMaterialPayload)
      other.externalMaterialPayload->saveState(jPayload);
    else
      jPayload = other.jPayload;

    assetType = other.assetType;
    subPath = other.subPath;
  }
  return *this;
}

//##################################################################################################
ExternalMaterial& ExternalMaterial::operator=(ExternalMaterial&& other) noexcept
{
  if(&other != this)
  {
    externalMaterialPayload = std::move(other.externalMaterialPayload);
    jPayload = std::move(other.jPayload);
    assetType = std::move(other.assetType);
    subPath = std::move(other.subPath);
  }
  return *this;
}

//##################################################################################################
void ExternalMaterial::saveState(tp_utils::JSON& j) const
{
  if(externalMaterialPayload)
    externalMaterialPayload->saveState(j);
  else if(!jPayload.is_null())
    j.update(jPayload);

  j["assetType"] = assetType.toString();
  j["subPath"]   = subPath.toString();
}

//##################################################################################################
void ExternalMaterial::loadState(const tp_utils::JSON& j)
{
  externalMaterialPayload = {};
  jPayload = j;

  assetType = TPJSONString(j, "assetType");
  subPath   = TPJSONString(j, "subPath");
}

//##################################################################################################
void ExternalMaterial::appendFileIDs(std::vector<std::pair<tp_utils::StringID, tp_utils::StringID>>& fileIDs) const
{ 
  fileIDs.push_back({assetType, subPath});
}

//##################################################################################################
void ExternalMaterial::allTextureIDs(std::unordered_set<tp_utils::StringID>& textureIDs, const ExtractTextureIDs& extractTextureIDs) const
{
  extractTextureIDs(*this, textureIDs);
}

//##################################################################################################
void ExternalMaterial::view(const Material& material,
                            const tp_utils::StringID& assetType,
                            const std::function<void(const ExternalMaterial&)>& closure)
{
  for(const auto& extendedMaterial : material.extendedMaterials)
  {
    if(auto m = dynamic_cast<const ExternalMaterial*>(extendedMaterial); m && m->assetType == assetType)
    {
      closure(*m);
      return;
    }
  }
}

}
