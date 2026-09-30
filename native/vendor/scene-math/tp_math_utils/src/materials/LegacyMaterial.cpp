#include "tp_math_utils/materials/LegacyMaterial.h"
#include "tp_math_utils/JSONUtils.h"

namespace tp_math_utils
{
namespace Legacy
{
//##################################################################################################
std::string colorspaceToString(Colorspace colorspace)
{
  switch(colorspace)
  {
    case Colorspace::sRGB: return "sRGB";
    case Colorspace::FilmicSRGB: return "Filmic sRGB";
  }
  return "sRGB";
}

//##################################################################################################
Colorspace colorspaceFromString(const std::string& colorspace)
{
  if(colorspace == "Filmic sRGB")
    return Colorspace::FilmicSRGB;
  return Colorspace::sRGB;
}

//##################################################################################################
std::string sssMethodToString(SSSMethod sssMethod)
{
  switch(sssMethod)
  {
    case SSSMethod::ChristensenBurley     : return "ChristensenBurley"    ;
    case SSSMethod::RandomWalk            : return "RandomWalk"           ;
    case SSSMethod::RandomWalkFixedRadius : return "RandomWalkFixedRadius";
  }
  return "ChristensenBurley";
}

//##################################################################################################
SSSMethod sssMethodFromString(const std::string& sssMethod)
{
  if(sssMethod == "RandomWalk"           ) return SSSMethod::RandomWalk;
  if(sssMethod == "RandomWalkFixedRadius") return SSSMethod::RandomWalkFixedRadius;
  return SSSMethod::ChristensenBurley;
}

//##################################################################################################
std::string shaderTypeToString(ShaderType shaderType)
{
  switch(shaderType)
  {
    case ShaderType::Principled   : return "Principled"  ;
    case ShaderType::None         : return "None"        ;
  }
  return "Principled";
}

//##################################################################################################
ShaderType shaderTypeFromString(const std::string& shaderType)
{
  if(shaderType == "None")
    return ShaderType::None;

  return ShaderType::Principled;
}
}
//##################################################################################################
void LegacyMaterial::saveState(tp_utils::JSON& j) const
{
  j["exporterVersion"]       = "Blender3";
  j["shaderType"]            = shaderTypeToString(shaderType);

  j["albedoColorspace"]      = colorspaceToString(albedoColorspace);
  j["sss"]                   = tp_math_utils::vec3ToJSON(sss);
  j["emission"]              = tp_math_utils::vec3ToJSON(emission);
  j["velvet"]                = tp_math_utils::vec3ToJSON(velvet);

  j["sssScale"]              = sssScale;
  j["emissionScale"]         = emissionScale;
  j["velvetScale"]           = velvetScale;
  j["heightScale"]           = heightScale;
  j["heightMidlevel"]        = heightMidlevel;

  j["specular"]              = specular;
  j["ior"]                   = ior;
  j["sheen"]                 = sheen;
  j["sheenTint"]             = sheenTint;
  j["clearCoat"]             = clearCoat;
  j["clearCoatRoughness"]    = clearCoatRoughness;

  j["iridescentFactor"]      = iridescentFactor;
  j["iridescentOffset"]      = iridescentOffset;
  j["iridescentFrequency"]   = iridescentFrequency;

  j["sssRadius"]             = tp_math_utils::vec3ToJSON(sssRadius);
  j["sssMethod"]             = sssMethodToString(sssMethod);

  j["normalStrength"]        = normalStrength;

  j["rayVisibilityCamera"]        = rayVisibilityCamera;
  j["rayVisibilityDiffuse"]       = rayVisibilityDiffuse;
  j["rayVisibilityGlossy"]        = rayVisibilityGlossy;
  j["rayVisibilityTransmission"]  = rayVisibilityTransmission;
  j["rayVisibilityScatter"]       = rayVisibilityScatter;
  j["rayVisibilityShadow"]        = rayVisibilityShadow;
  j["lightPathReflection"]        = lightPathReflection;

  viewTypedTextures([&](const auto& type, const auto& value, const auto&)
  {
    j[type] = value.toString();
  });
}

//##################################################################################################
void LegacyMaterial::loadState(const tp_utils::JSON& j)
{
  exporterVersion       = TPJSONString(j, "exporterVersion", "Blender3");
  shaderType            = Legacy::shaderTypeFromString(TPJSONString(j, "shaderType"));

  albedoColorspace      = Legacy::colorspaceFromString(TPJSONString(j, "albedoColorspace"));
  sss                   = tp_math_utils::vec3FromJSON(j, "sss"     , sss     );
  emission              = tp_math_utils::vec3FromJSON(j, "emission", emission);
  velvet                = tp_math_utils::vec3FromJSON(j, "velvet"  , velvet  );

  sssScale              = TPJSONFloat(j, "sssScale"      , sssScale      );
  emissionScale         = TPJSONFloat(j, "emissionScale" , emissionScale );
  velvetScale           = TPJSONFloat(j, "velvetScale"   , velvetScale   );
  heightScale           = TPJSONFloat(j, "heightScale"   , heightScale   );
  heightMidlevel        = TPJSONFloat(j, "heightMidlevel", heightMidlevel);

  specular              = TPJSONFloat(j, "specular"             , specular             );
  ior                   = TPJSONFloat(j, "ior"                  , ior                  );
  sheen                 = TPJSONFloat(j, "sheen"                , sheen                );
  sheenTint             = TPJSONFloat(j, "sheenTint"            , sheenTint            );
  clearCoat             = TPJSONFloat(j, "clearCoat"            , clearCoat            );
  clearCoatRoughness    = TPJSONFloat(j, "clearCoatRoughness"   , clearCoatRoughness   );
  iridescentFactor      = TPJSONFloat(j, "iridescentFactor"     , iridescentFactor     );
  iridescentOffset      = TPJSONFloat(j, "iridescentOffset"     , iridescentOffset     );
  iridescentFrequency   = TPJSONFloat(j, "iridescentFrequency"  , iridescentFrequency  );

  sssRadius             = tp_math_utils::vec3FromJSON(j, "sssRadius", sssRadius);
  sssMethod             = Legacy::sssMethodFromString(TPJSONString(j, "sssMethod"));

  normalStrength        = TPJSONFloat(j, "normalStrength"  , normalStrength  );

  rayVisibilityCamera        = TPJSONBool(j, "rayVisibilityCamera"       , true );
  rayVisibilityDiffuse       = TPJSONBool(j, "rayVisibilityDiffuse"      , true );
  rayVisibilityGlossy        = TPJSONBool(j, "rayVisibilityGlossy"       , true );
  rayVisibilityTransmission  = TPJSONBool(j, "rayVisibilityTransmission" , true );
  rayVisibilityScatter       = TPJSONBool(j, "rayVisibilityScatter"      , true );
  rayVisibilityShadow        = TPJSONBool(j, "rayVisibilityShadow"       , true );
  lightPathReflection        = TPJSONBool(j, "lightPathReflection"       , false );

  updateTypedTextures([&](const auto& type, auto& value, const auto&)
  {
    value = TPJSONString(j, type);
  });
}

//##################################################################################################
void LegacyMaterial::view(const Material& material, const std::function<void(const LegacyMaterial&)>& closure)
{
  for(const auto& extendedMaterial : material.extendedMaterials)
  {
    if(auto m = dynamic_cast<const LegacyMaterial*>(extendedMaterial); m)
    {
      closure(*m);
      return;
    }
  }

  closure(LegacyMaterial());
}

//##################################################################################################
void LegacyMaterial::allTextureIDs(std::unordered_set<tp_utils::StringID>& textureIDs, const ExtractTextureIDs& extractTextureIDs) const
{
  TP_UNUSED(extractTextureIDs);
  viewTextures([&](const tp_utils::StringID& value)
  {
    if(value.isValid())
      textureIDs.insert(value);
  });
}

}
