#include "tp_math_utils/materials/OpenGLMaterial.h"
#include "tp_math_utils/JSONUtils.h"

namespace tp_math_utils
{

//##################################################################################################
void OpenGLMaterial::saveState(tp_utils::JSON& j) const
{
  j["albedo"]                = tp_math_utils::vec3ToJSON(albedo);

  j["albedoScale"]           = albedoScale;
  j["alpha"]                 = alpha;
  j["roughness"]             = roughness;
  j["metalness"]             = metalness;
  j["transmission"]          = transmission;
  j["transmissionRoughness"] = transmissionRoughness;

  j["albedoBrightness"]      = albedoBrightness;
  j["albedoContrast"]        = albedoContrast;
  j["albedoGamma"]           = albedoGamma;
  j["albedoHue"]             = albedoHue;
  j["albedoSaturation"]      = albedoSaturation;
  j["albedoValue"]           = albedoValue;
  j["albedoFactor"]          = albedoFactor;

  j["useAmbient"]            = useAmbient;
  j["useDiffuse"]            = useDiffuse;
  j["useNdotL"]              = useNdotL;
  j["useAttenuation"]        = useAttenuation;
  j["useShadow"]             = useShadow;
  j["useLightMask"]          = useLightMask;
  j["useReflection"]         = useReflection;

  j["tileTextures"]          = tileTextures;

  j["rayVisibilityShadowCatcher"] = rayVisibilityShadowCatcher;

  viewTypedTextures([&](const auto& type, const auto& value, const auto&)
  {
    j[type] = value.toString();
  });
}

//##################################################################################################
void OpenGLMaterial::loadState(const tp_utils::JSON& j)
{
  albedo                = tp_math_utils::vec3FromJSON(j, "albedo", albedo  );

  albedoScale           = TPJSONFloat(j, "albedoScale"   , albedoScale   );

  alpha                 = TPJSONFloat(j, "alpha"                , alpha                );
  roughness             = TPJSONFloat(j, "roughness"            , roughness            );
  metalness             = TPJSONFloat(j, "metalness"            , metalness            );
  transmission          = TPJSONFloat(j, "transmission"         , transmission         );
  transmissionRoughness = TPJSONFloat(j, "transmissionRoughness", transmissionRoughness);

  albedoBrightness      = TPJSONFloat(j, "albedoBrightness", albedoBrightness);
  albedoContrast        = TPJSONFloat(j, "albedoContrast"  , albedoContrast  );
  albedoGamma           = TPJSONFloat(j, "albedoGamma"     , albedoGamma     );
  albedoHue             = TPJSONFloat(j, "albedoHue"       , albedoHue       );
  albedoSaturation      = TPJSONFloat(j, "albedoSaturation", albedoSaturation);
  albedoValue           = TPJSONFloat(j, "albedoValue"     , albedoValue     );
  albedoFactor          = TPJSONFloat(j, "albedoFactor"    , albedoFactor    );

  useAmbient            = TPJSONFloat(j, "useAmbient"    , useAmbient    );
  useDiffuse            = TPJSONFloat(j, "useDiffuse"    , useDiffuse    );
  useNdotL              = TPJSONFloat(j, "useNdotL"      , useNdotL      );
  useAttenuation        = TPJSONFloat(j, "useAttenuation", useAttenuation);
  useShadow             = TPJSONFloat(j, "useShadow"     , useShadow     );
  useLightMask          = TPJSONFloat(j, "useLightMask"  , useLightMask  );
  useReflection         = TPJSONFloat(j, "useReflection" , useReflection );

  tileTextures          = TPJSONBool(j, "tileTextures", false);

  rayVisibilityShadowCatcher = TPJSONBool(j, "rayVisibilityShadowCatcher", false);

  updateTypedTextures([&](const auto& type, auto& value, const auto&)
  {
    value = TPJSONString(j, type);
  });
}

//##################################################################################################
void OpenGLMaterial::view(const Material& material, const std::function<void(const OpenGLMaterial&)>& closure)
{
  for(const auto& extendedMaterial : material.extendedMaterials)
  {
    if(auto m = dynamic_cast<const OpenGLMaterial*>(extendedMaterial); m)
    {
      closure(*m);
      return;
    }
  }

  closure(OpenGLMaterial());
}

//##################################################################################################
void OpenGLMaterial::allTextureIDs(std::unordered_set<tp_utils::StringID>& textureIDs, const ExtractTextureIDs& extractTextureIDs) const
{
  TP_UNUSED(extractTextureIDs);
  viewTextures([&](const tp_utils::StringID& value)
  {
    if(value.isValid())
      textureIDs.insert(value);
  });
}

}
