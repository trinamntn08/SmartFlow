#include "tp_math_utils/materials/Blender4Material.h"
#include "tp_math_utils/JSONUtils.h"

namespace tp_math_utils
{
namespace Blender4
{
//##################################################################################################
std::string colorspaceToString(Colorspace colorspace)
{
  switch(colorspace)
  {
    case Colorspace::sRGB       : return "sRGB";
    case Colorspace::FilmicSRGB : return "Filmic sRGB";
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
    case SSSMethod::ChristensenBurley : return "ChristensenBurley"    ;
    case SSSMethod::RandomWalk        : return "RandomWalk";
    case SSSMethod::RandomWalkSkin    : return "RandomWalkSkin"       ;
  }
  return "ChristensenBurley";
}

//##################################################################################################
SSSMethod sssMethodFromString(const std::string& sssMethod)
{
  if(sssMethod == "RandomWalk"    ) return SSSMethod::RandomWalk;
  if(sssMethod == "RandomWalkSkin") return SSSMethod::RandomWalkSkin;
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

//##################################################################################################
std::string specularDistributionToString(const SpecularDistribution& specularDistribution)
{
  switch(specularDistribution)
  {
    case SpecularDistribution::GGX                : return "GGX"             ;
    case SpecularDistribution::MultiScatterGGX    : return "MultiScatterGGX" ;
  }
  return "GGX";
}

//##################################################################################################
SpecularDistribution specularDistributionFromString(const std::string& specularDistribution)
{
  if(specularDistribution == "MultiScatterGGX")
    return SpecularDistribution::MultiScatterGGX;

  return SpecularDistribution::GGX;
}

}

//##################################################################################################
void Blender4Material::saveState(tp_utils::JSON& j) const
{
  j["exporterVersion"]             = "Blender4";
  j["shaderType"]                  = Blender4::shaderTypeToString(shaderType);

  j["albedoColorspace"]            = Blender4::colorspaceToString(albedoColorspace);
  j["ior"]                         = ior;

  j["sssMethod"]                   = Blender4::sssMethodToString(sssMethod);
  j["sssWeight"]                   = sssWeight;
  j["sssRadius"]                   = tp_math_utils::vec3ToJSON(sssRadius);
  j["sssScale"]                    = sssScale;
  j["sssIOR"]                      = sssIOR;
  j["sssAnisotropy"]               = sssAnisotropy;

  j["specularIORLevel"]            = specularIORLevel;
  j["specularTint"]                = tp_math_utils::vec4ToJSON(specularTint);
  j["specularDistribution"]        = Blender4::specularDistributionToString(specularDistribution);

  j["coatWeight"]                  = coatWeight;
  j["coatRoughness"]               = coatRoughness;
  j["coatIOR"]                     = coatIOR;
  j["coatTint"]                    = tp_math_utils::vec4ToJSON(coatTint);

  j["sheenWeight"]                 = sheenWeight;
  j["sheenRoughness"]              = sheenRoughness;
  j["sheenTint"]                   = tp_math_utils::vec4ToJSON(sheenTint);

  j["emissionColor"]               = tp_math_utils::vec4ToJSON(emissionColor);
  j["emissionStrength"]            = emissionStrength;

  j["diffuseRoughness"]            = diffuseRoughness;

  j["anisotropic"]                 = anisotropic   ;
  j["anisotropicRotation"]         = anisotropicRotation;

  j["thinFilmThickness"]           = thinFilmThickness   ;
  j["thinFilmIOR"]                 = thinFilmIOR  ;

  j["heightScale"]                 = heightScale;
  j["heightMidlevel"]              = heightMidlevel;

  j["normalStrength"]              = normalStrength;

  j["rayVisibilityCamera"]         = rayVisibilityCamera;
  j["rayVisibilityDiffuse"]        = rayVisibilityDiffuse;
  j["rayVisibilityGlossy"]         = rayVisibilityGlossy;
  j["rayVisibilityTransmission"]   = rayVisibilityTransmission;
  j["rayVisibilityScatter"]        = rayVisibilityScatter;
  j["rayVisibilityShadow"]         = rayVisibilityShadow;
  j["lightPathReflection"]         = lightPathReflection;

  viewTypedTextures([&](const auto& type, const auto& value, const auto&)
  {
    j[type] = value.toString();
  });
}

//##################################################################################################
void Blender4Material::loadState(const tp_utils::JSON& j)
{
  exporterVersion            = TPJSONString(j, "exporterVersion", "Blender4");
  shaderType                 = Blender4::shaderTypeFromString(TPJSONString(j, "shaderType"));
  albedoColorspace           = Blender4::colorspaceFromString(TPJSONString(j, "albedoColorspace"));

  ior                        = TPJSONFloat(j, "ior", ior);

  sssMethod                  = Blender4::sssMethodFromString(TPJSONString(j, "sssMethod"));
  sssWeight                  = TPJSONFloat(j, "sssWeight"     , sssWeight);
  sssRadius                  = tp_math_utils::vec3FromJSON(j, "sssRadius", sssRadius);
  sssScale                   = TPJSONFloat(j, "sssScale"      , sssScale           );
  sssIOR                     = TPJSONFloat(j, "sssIOR"        , sssIOR             );
  sssAnisotropy              = TPJSONFloat(j, "sssAnisotropy" , sssAnisotropy      );

  specularIORLevel           = TPJSONFloat(j, "specularIORLevel", specularIORLevel);
  specularTint               = tp_math_utils::vec4FromJSON(j, "specularTint", specularTint);
  specularDistribution       = Blender4::specularDistributionFromString(TPJSONString(j, "specularDistribution"));

  coatWeight                 = TPJSONFloat(j, "coatWeight"    , coatWeight       );
  coatRoughness              = TPJSONFloat(j, "coatRoughness" , coatRoughness    );
  coatIOR                    = TPJSONFloat(j, "coatIOR"       , coatIOR          );
  coatTint                   = tp_math_utils::vec4FromJSON(j, "coatTint", coatTint  );

  sheenWeight                = TPJSONFloat(j, "sheenWeight"        , sheenWeight       );
  sheenRoughness             = TPJSONFloat(j, "sheenRoughness"     , sheenRoughness    );
  sheenTint                  = tp_math_utils::vec4FromJSON(j, "sheenTint", sheenTint   );

  emissionColor              = tp_math_utils::vec4FromJSON(j, "emissionColor", emissionColor);
  emissionStrength           = TPJSONFloat(j, "emissionStrength", emissionStrength );

  diffuseRoughness           = TPJSONFloat(j, "diffuseRoughness", diffuseRoughness);

  anisotropic                = TPJSONFloat(j, "anisotropic", anisotropic)  ;
  anisotropicRotation        = TPJSONFloat(j, "anisotropicRotation", anisotropicRotation) ;

  thinFilmThickness          = TPJSONFloat(j, "thinFilmThickness",thinFilmThickness)   ;
  thinFilmIOR                = TPJSONFloat(j, "thinFilmIOR", thinFilmIOR) ;

  heightScale                = TPJSONFloat(j, "heightScale", heightScale   );
  heightMidlevel             = TPJSONFloat(j, "heightMidlevel", heightMidlevel);

  normalStrength             = TPJSONFloat(j, "normalStrength", normalStrength  );

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
void Blender4Material::view(const Material& material, const std::function<void(const Blender4Material&)>& closure)
{
  for(const auto& extendedMaterial : material.extendedMaterials)
  {
    if(auto m = dynamic_cast<const Blender4Material*>(extendedMaterial); m)
    {
      closure(*m);
      return;
    }
  }

  closure(Blender4Material());
}

//##################################################################################################
void Blender4Material::allTextureIDs(std::unordered_set<tp_utils::StringID>& textureIDs, const ExtractTextureIDs& extractTextureIDs) const
{
  TP_UNUSED(extractTextureIDs);
  viewTextures([&](const tp_utils::StringID& value)
  {
    if(value.isValid())
      textureIDs.insert(value);
  });
}

}
