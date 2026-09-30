#pragma once

#include "tp_math_utils/Material.h"

namespace tp_math_utils
{
//##################################################################################################
class TP_MATH_UTILS_EXPORT OpenGLMaterial : public ExtendedMaterial
{
public:
  glm::vec3 albedo  {0.4f, 0.0f, 0.0f}; //!< mtl: Kd or Ka
  float albedoScale{1.0f};              //!< Multiplied by the albedo

  float alpha{1.0f};                    //!< mtl: d
  float roughness{0.5f};                //!<
  float metalness{0.0f};                //!<
  float transmission{0.0f};             //!< Transmission for refractive materials like glass or water.
  float transmissionRoughness{0.0f};    //!<

  float albedoBrightness{0.0f};         //!< Modify the albedo texture brightness.
  float albedoContrast  {0.0f};         //!< Modify the albedo texture contrast.
  float albedoGamma     {1.0f};         //!< Modify the albedo texture gamma.
  float albedoHue       {0.5f};         //!< Modify the albedo texture hue.
  float albedoSaturation{1.0f};         //!< Modify the albedo texture saturation.
  float albedoValue     {1.0f};         //!< Modify the albedo texture value.
  float albedoFactor    {1.0f};         //!< Modify the albedo texture factor.

  float useAmbient    {1.0f};           //!< Should the ambient light be used to modulate albedo.
  float useDiffuse    {1.0f};           //!< Should the diffuse calculation be used to modulate albedo.
  float useNdotL      {1.0f};           //!< Should the angel of the light be used to calculate the color.
  float useAttenuation{1.0f};           //!< Should the distance to the light be used to attenuate the color.
  float useShadow     {1.0f};           //!< Should the shadow be used in the color calculation.
  float useLightMask  {1.0f};           //!< Should the light mask texture be used in the color calculation.
  float useReflection {1.0f};           //!< Should the reflection be used in the final color calculation.

  bool tileTextures{false};             //!< True to GL_REPEAT textures else GL_CLAMP_TO_EDGE

  bool rayVisibilityShadowCatcher{false};  //!< Blender cycles ray visibility options.

  tp_utils::StringID                  rgbaTexture; //!< RGBA, combined albedo and alpha.
  tp_utils::StringID                 rmttrTexture; //!< RGBA, combined roughness, metalness, transmission, transmission roughness.

  tp_utils::StringID                albedoTexture; //!< RGB,                   mtl: map_Kd or map_Ka
  tp_utils::StringID                 alphaTexture; //!< Grey scale,            mtl: map_d
  tp_utils::StringID               normalsTexture; //!< RGB (XYZ) Unit vector, mtl: map_Bump
  tp_utils::StringID             roughnessTexture; //!< Grey scale.
  tp_utils::StringID             metalnessTexture; //!< Grey scale.
  tp_utils::StringID          transmissionTexture; //!< Grey scale.
  tp_utils::StringID transmissionRoughnessTexture; //!< Grey scale.

  //################################################################################################
  void saveState(tp_utils::JSON& j) const override;

  //################################################################################################
  void loadState(const tp_utils::JSON& j) override;

  //################################################################################################
  glm::vec4 rgba() const
  {
    return {albedo, alpha};
  }

  //################################################################################################
  static void view(const Material& material, const std::function<void(const OpenGLMaterial&)>& closure);

  //################################################################################################
  void allTextureIDs(std::unordered_set<tp_utils::StringID>& textureIDs, const ExtractTextureIDs& extractTextureIDs) const override;

  //################################################################################################
  template<typename T>
  void updateTypedTextures(const T& closure)
  {
    closure(                 "rgbaTexture",                  rgbaTexture, "RGBA"                  );
    closure(                "rmttrTexture",                 rmttrTexture, "RMTTR"                 );

    closure(               "albedoTexture",                albedoTexture, "Albedo"                );
    closure(                "alphaTexture",                 alphaTexture, "Alpha"                 );
    closure(              "normalsTexture",               normalsTexture, "Normals"               );
    closure(            "roughnessTexture",             roughnessTexture, "Roughness"             );
    closure(            "metalnessTexture",             metalnessTexture, "Metalness"             );
    closure(         "transmissionTexture",          transmissionTexture, "Transmission"          );
    closure("transmissionRoughnessTexture", transmissionRoughnessTexture, "Transmission roughness");
  }

  //################################################################################################
  template<typename T>
  void viewTypedTextures(const T& closure) const
  {
    const_cast<OpenGLMaterial*>(this)->updateTypedTextures([&](const auto& type, const auto& value, const auto& pretty){closure(type, value, pretty);});
  }

  //################################################################################################
  template<typename T>
  void updateTextures(const T& closure)
  {
    updateTypedTextures([&](const auto&, auto& value, const auto&){closure(value);});
  }

  //################################################################################################
  template<typename T>
  void viewTextures(const T& closure) const
  {
    viewTypedTextures([&](const auto&, const auto& value, const auto&){closure(value);});
  }
};

}

