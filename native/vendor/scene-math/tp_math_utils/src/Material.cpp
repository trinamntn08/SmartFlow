#include "tp_math_utils/Material.h"
#include "tp_math_utils/JSONUtils.h"
#include "tp_math_utils/materials/OpenGLMaterial.h"
#include "tp_math_utils/materials/LegacyMaterial.h"
#include "tp_math_utils/materials/Blender4Material.h"
#include "tp_math_utils/materials/ExternalMaterial.h"

#include "glm/gtx/matrix_transform_2d.hpp" // IWYU pragma: keep
#include "glm/gtx/quaternion.hpp" // IWYU pragma: keep

namespace tp_math_utils
{

namespace
{
//##################################################################################################
glm::mat3 skew(const glm::mat3& m_, const glm::vec2& uv)
{
  glm::mat3 m{1.0f};
  m[1][0] = glm::tan(glm::radians(uv.x));
  m[0][1] = glm::tan(glm::radians(uv.y));
  return m_ * m;
}

//##################################################################################################
void cloneExtendedMaterials(const std::vector<ExtendedMaterial*>& from, std::vector<ExtendedMaterial*>& to)
{
  tpDeleteAll(to);
  to.clear();

  for(const auto& extendedMaterial : from)
  {
    if(auto m=dynamic_cast<const OpenGLMaterial*>(extendedMaterial); m)
      to.push_back(new OpenGLMaterial(*m));

    else if(auto m=dynamic_cast<const LegacyMaterial*>(extendedMaterial); m)
      to.push_back(new LegacyMaterial(*m));

    else if(auto m=dynamic_cast<const Blender4Material*>(extendedMaterial); m)
      to.push_back(new Blender4Material(*m));

    else if(auto m=dynamic_cast<const ExternalMaterial*>(extendedMaterial); m)
      to.push_back(new ExternalMaterial(*m));
  }
}
}

//##################################################################################################
ExtendedMaterial::~ExtendedMaterial()
{

}

//##################################################################################################
void ExtendedMaterial::allTextureIDs(std::unordered_set<tp_utils::StringID>& textureIDs, const ExtractTextureIDs& extractTextureIDs) const
{
  TP_UNUSED(textureIDs);
  TP_UNUSED(textureIDs);
  TP_UNUSED(extractTextureIDs);
}

//################################################################################################
void ExtendedMaterial::appendFileIDs(std::vector<std::pair<tp_utils::StringID, tp_utils::StringID>>& fileIDs) const
{
  TP_UNUSED(fileIDs);
}

//##################################################################################################
std::unordered_set<tp_utils::StringID> ExtendedMaterial::allTextures(const ExtractTextureIDs& extractTextureIDs) const
{
  std::unordered_set<tp_utils::StringID> textureIDs;
  allTextureIDs(textureIDs, extractTextureIDs);
  return textureIDs;
}

//##################################################################################################
glm::mat3 UVTransformation::uvMatrix() const
{
  glm::mat3 m{1.0f};
  m = glm::scale(m, scaleUV);
  m = skew(m, skewUV);
  m = glm::rotate(m, glm::radians(rotateUV));
  m = glm::translate(m, translateUV);
  return m;
}

//##################################################################################################
glm::mat3 UVTransformation::lightMaskUVMatrix() const
{
  glm::vec2 center(0.5f, 0.5f);
  glm::mat3 m{1.0f};

  m = glm::translate(m, center);
  m = glm::scale(m,  glm::vec2(1.0f / scaleUV.x, 1.0f / scaleUV.y));
  m = glm::translate(m, -center);

  m = skew(m, -skewUV);

  m = glm::translate(m, center);
  m = glm::rotate(m, glm::radians(-rotateUV));
  m = glm::translate(m, -center);

  m = glm::translate(m, -translateUV);

  return m;
}

//##################################################################################################
glm::mat4 UVTransformation::uvwMatrix() const
{
  if(type == TransformationType::Transform3D)
  {
    glm::mat4 m{1.0f};
    m = glm::translate(m, translateUVW);
    m = m * glm::toMat4(glm::quat(glm::radians(rotateUVW)));
    m = glm::scale(m, scaleUVW);
    m = skew(m, skewUVW);
    return m;
  }
  return glm::mat4(uvMatrix());
}

//##################################################################################################
bool UVTransformation::isIdentity() const
{
  if(type == TransformationType::Transform3D)
    return false;

  if((std::fabs(skewUV.x) + std::fabs(skewUV.y)) > 0.0001f)
    return false;

  if((std::fabs(scaleUV.x-1.0f) + std::fabs(scaleUV.y-1.0f)) > 0.0001f)
    return false;

  if((std::fabs(translateUV.x) + std::fabs(translateUV.y)) > 0.0001f)
    return false;

  if(std::fabs(rotateUV) > 0.0001f)
    return false;

  return true;
}

//##################################################################################################
void UVTransformation::saveState(tp_utils::JSON& j) const
{
  j["skewUVW"]               = tp_math_utils::vec3ToJSON(skewUVW);
  j["scaleUVW"]              = tp_math_utils::vec3ToJSON(scaleUVW);
  j["translateUVW"]          = tp_math_utils::vec3ToJSON(translateUVW);
  j["rotateUVW"]             = tp_math_utils::vec3ToJSON(rotateUVW);
  j["type"]                  = UVTransformation::toString(type);

  j["skewUV"]                = tp_math_utils::vec2ToJSON(skewUV);
  j["scaleUV"]               = tp_math_utils::vec2ToJSON(scaleUV);
  j["translateUV"]           = tp_math_utils::vec2ToJSON(translateUV);
  j["rotateUV"]              = rotateUV;
}

//##################################################################################################
void UVTransformation::loadState(const tp_utils::JSON& j)
{
  (*this) = UVTransformation();

  skewUV       = tp_math_utils::getJSONVec2(j,      "skewUV",      skewUV);
  scaleUV      = tp_math_utils::getJSONVec2(j,     "scaleUV",     scaleUV);
  translateUV  = tp_math_utils::getJSONVec2(j, "translateUV", translateUV);
  rotateUV     = TPJSONFloat               (j,    "rotateUV",    rotateUV);

  skewUVW      = tp_math_utils::getJSONVec3(j,     "skewUVW",      skewUVW);
  scaleUVW     = tp_math_utils::getJSONVec3(j,    "scaleUVW",     scaleUVW);
  translateUVW = tp_math_utils::getJSONVec3(j,"translateUVW", translateUVW);
  rotateUVW    = tp_math_utils::getJSONVec3(j,   "rotateUVW",    rotateUVW);
  type         = UVTransformation::toType(TPJSONString(j,  "type"));
}

//##################################################################################################
std::string exporterVersionToString(const ExporterVersion& exporterVersion)
{
  if(exporterVersion == ExporterVersion::Blender4)
    return "Blender4";

  return "Blender3";
}

//##################################################################################################
ExporterVersion exporterVersionFromString(const std::string& exporterVersion)
{
  if(exporterVersion == "Blender4")
    return ExporterVersion::Blender4;

  return ExporterVersion::Blender3;
}

//##################################################################################################
Material::Material()
{

}

//##################################################################################################
Material::Material(const tp_utils::StringID& name_):
  name(name_)
{

}

//##################################################################################################
Material::Material(const Material& other):
  name(std::move(other.name)),
  uvTransformation(other.uvTransformation),
  exporterVersion(other.exporterVersion)
{
  cloneExtendedMaterials(other.extendedMaterials, extendedMaterials);
}

//##################################################################################################
Material::Material(Material&& other) noexcept:
  name(std::move(other.name)),
  uvTransformation(std::move(other.uvTransformation)),
  exporterVersion(std::move(other.exporterVersion))
{
  std::swap(extendedMaterials, other.extendedMaterials);
}

//##################################################################################################
Material& Material::operator=(const Material& other)
{
  if(this != &other)
  {
    name = other.name;
    uvTransformation = other.uvTransformation;
    exporterVersion  = other.exporterVersion;
    cloneExtendedMaterials(other.extendedMaterials, extendedMaterials);
  }

  return *this;
}

//##################################################################################################
Material& Material::operator=(Material&& other) noexcept
{
  if(this != &other)
  {
    name = std::move(other.name);
    uvTransformation = std::move(other.uvTransformation);
    std::swap(extendedMaterials, other.extendedMaterials);
    exporterVersion = std::move(other.exporterVersion);
  }

  return *this;
}

//##################################################################################################
Material::~Material()
{
  tpDeleteAll(extendedMaterials);
}

//##################################################################################################
OpenGLMaterial* Material::findOrAddOpenGL()
{
  for(auto material : extendedMaterials)
    if(auto m=dynamic_cast<OpenGLMaterial*>(material); m)
      return m;

  auto m = new OpenGLMaterial();
  extendedMaterials.push_back(m);
  return m;
}

//##################################################################################################
LegacyMaterial* Material::findOrAddLegacy()
{
  for(auto material : extendedMaterials)
    if(auto m=dynamic_cast<LegacyMaterial*>(material); m)
      return m;

  auto m = new LegacyMaterial();
  extendedMaterials.push_back(m);
  return m;
}

//##################################################################################################
Blender4Material* Material::findOrAddBlender4()
{
  exporterVersion = ExporterVersion::Blender4;

  for(auto material : extendedMaterials)
    if(auto m=dynamic_cast<Blender4Material*>(material); m)
    {
      return m;
    }

  auto m = new Blender4Material();
  extendedMaterials.push_back(m);
  return m;
}
//##################################################################################################
ExternalMaterial* Material::findOrAddExternal(const tp_utils::StringID& assetType)
{
  for(auto material : extendedMaterials)
    if(auto m=dynamic_cast<ExternalMaterial*>(material); m)
      if(m->assetType == assetType)
        return m;

  auto m = new ExternalMaterial();
  m->assetType = assetType;
  extendedMaterials.push_back(m);
  return m;
}

//##################################################################################################
void Material::removeExternal(const tp_utils::StringID& assetType)
{
  for(auto i=extendedMaterials.begin(); i!=extendedMaterials.end();)
  {
    if(auto m=dynamic_cast<ExternalMaterial*>(*i); m)
    {
      if(m->assetType == assetType)
      {
        i=extendedMaterials.erase(i);
        continue;
      }
    }
    ++i;
  }
}

//##################################################################################################
bool Material::hasExternal(const tp_utils::StringID& assetType) const
{
  for(auto material : extendedMaterials)
    if(auto m=dynamic_cast<ExternalMaterial*>(material); m)
      if(m->assetType == assetType)
        return true;
  return false;
}

//################################################################################################
bool Material::isBlender4Material() const
{
  return exporterVersion == ExporterVersion::Blender4;
}

//##################################################################################################
void Material::updateOpenGL(const std::function<void(OpenGLMaterial&)>& closure) const
{
  for(auto material : extendedMaterials)
    if(auto m=dynamic_cast<OpenGLMaterial*>(material); m)
      return closure(*m);
}

//##################################################################################################
void Material::updateLegacy(const std::function<void(LegacyMaterial&)>& closure) const
{
  for(auto material : extendedMaterials)
    if(auto m=dynamic_cast<LegacyMaterial*>(material); m)
      return closure(*m);
}

//##################################################################################################
void Material::updateBlender4(const std::function<void(Blender4Material&)>& closure) const
{
  for(auto material : extendedMaterials)
    if(auto m=dynamic_cast<Blender4Material*>(material); m)
      return closure(*m);
}

//##################################################################################################
void Material::updateExternal(const tp_utils::StringID& assetType,
                              const std::function<void(ExternalMaterial&)>& closure) const
{
  for(auto material : extendedMaterials)
    if(auto m=dynamic_cast<ExternalMaterial*>(material); m)
      if(m->assetType == assetType)
        return closure(*m);
}

//##################################################################################################
void Material::viewOpenGL(const std::function<void(const OpenGLMaterial&)>& closure) const
{
  OpenGLMaterial::view(*this, closure);
}

//##################################################################################################
void Material::viewLegacy(const std::function<void(const LegacyMaterial&)>& closure) const
{
  LegacyMaterial::view(*this, closure);
}

//##################################################################################################
void Material::viewBlender4(const std::function<void(const Blender4Material&)>& closure) const
{
  Blender4Material::view(*this, closure);
}

//##################################################################################################
void Material::viewExternal(const tp_utils::StringID& assetType,
                            const std::function<void(const ExternalMaterial&)>& closure) const
{
  ExternalMaterial::view(*this, assetType, closure);
}

//##################################################################################################
void Material::allTextureIDs(std::unordered_set<tp_utils::StringID>& textureIDs, const ExtractTextureIDs& extractTextureIDs) const
{
  for(const auto& extendedMaterial : extendedMaterials)
    extendedMaterial->allTextureIDs(textureIDs, extractTextureIDs);
}

//##################################################################################################
void Material::appendFileIDs(std::vector<std::pair<tp_utils::StringID, tp_utils::StringID>>& fileIDs) const
{
  for(const auto& extendedMaterial : extendedMaterials)
   extendedMaterial->appendFileIDs(fileIDs);
}

//##################################################################################################
std::unordered_set<tp_utils::StringID> Material::allTextures(const ExtractTextureIDs& extractTextureIDs) const
{
  std::unordered_set<tp_utils::StringID> textureIDs;
  allTextureIDs(textureIDs, extractTextureIDs);
  return textureIDs;
}

//##################################################################################################
std::vector<std::pair<tp_utils::StringID, tp_utils::StringID>> Material::allFiles() const
{
  std::vector<std::pair<tp_utils::StringID, tp_utils::StringID>> files;
  appendFileIDs(files);
  return files;
}

//##################################################################################################
void Material::saveState(tp_utils::JSON& j) const
{
  j["name"] = name.toString();
  j["version"] = "2.0";
  uvTransformation.saveState(j);
  j["exporterVersion"] = exporterVersionToString(exporterVersion);

  auto& extendedMaterialsJ = j["extendedMaterials"];
  extendedMaterialsJ = tp_utils::JSON::array();
  extendedMaterialsJ.get_ptr<tp_utils::JSON::array_t*>()->reserve(extendedMaterials.size());
  for(auto extendedMaterial : extendedMaterials)
  {
    extendedMaterialsJ.emplace_back();
    auto& extendedMaterialJ = extendedMaterialsJ.back();

    extendedMaterial->saveState(extendedMaterialJ);

    if(dynamic_cast<OpenGLMaterial*>(extendedMaterial))
    {
      extendedMaterialJ["type"] = "OpenGL";
    }

    else if(!isBlender4Material() && dynamic_cast<LegacyMaterial*>(extendedMaterial))
    {
      extendedMaterialJ["type"] = "Legacy";
    }

    else if(isBlender4Material() && dynamic_cast<Blender4Material*>(extendedMaterial))
    {
      extendedMaterialJ["type"] = "Blender4";
    }

    else if(dynamic_cast<ExternalMaterial*>(extendedMaterial))
      extendedMaterialJ["type"] = "External";
  }

}

//##################################################################################################
void Material::saveUVMatrix(tp_utils::JSON& j) const
{
  uvTransformation.saveState(j);
  j["uvMatrix"] = tp_math_utils::mat3ToJSON(uvTransformation.uvMatrix());
  j["isIdentity"] = uvTransformation.isIdentity();
}

//##################################################################################################
void Material::loadState(const tp_utils::JSON& j)
{
  tpDeleteAll(extendedMaterials);
  extendedMaterials.clear();
  exporterVersion = exporterVersionFromString(TPJSONString(j, "exporterVersion"));

  if(TPJSONString(j, "version") == "2.0")
  {
    // New format
    if(auto i=j.find("extendedMaterials"); i!=j.end() && i->is_array())
    {
      extendedMaterials.reserve(i->size());
      for(const auto& extendedMaterialJ : *i)
      {
        ExtendedMaterial* extendedMaterial{nullptr};

        std::string type = TPJSONString(extendedMaterialJ, "type");

        if(type == "OpenGL")
          extendedMaterial = findOrAddOpenGL();

        else if(type == "Legacy")
          extendedMaterial = findOrAddLegacy();

        else if(type == "Blender4")
        {
          extendedMaterial = findOrAddBlender4();
          exporterVersion = ExporterVersion::Blender4;
        }

        else if(type == "External")
        {
          extendedMaterial = new ExternalMaterial();
          extendedMaterials.push_back(extendedMaterial);
        }

        if(extendedMaterial)
          extendedMaterial->loadState(extendedMaterialJ);
      }
    }
  }
  else if(j.find("albedo") != j.end())
  {
    // Legacy format
    findOrAddOpenGL()->loadState(j);

    if(TPJSONString(j, "exporterVersion") == "Blender4")
    {
      findOrAddBlender4()->loadState(j);
    }
    else
      findOrAddLegacy()->loadState(j);
  }

  name = TPJSONString(j, "name");
  uvTransformation.loadState(j);

}

}
