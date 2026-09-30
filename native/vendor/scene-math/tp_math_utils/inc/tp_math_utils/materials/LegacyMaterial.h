#pragma once

#include "tp_math_utils/Material.h"

namespace tp_math_utils
{
namespace Legacy
{
//##################################################################################################
enum class Colorspace
{
  sRGB = 0, //!< The default colorspace
  FilmicSRGB = 1  //!< Newer colorspace
};

//##################################################################################################
std::string colorspaceToString(Colorspace colorspace);

//##################################################################################################
Colorspace colorspaceFromString(const std::string& colorspace);

//##################################################################################################
enum class SSSMethod
{
  ChristensenBurley     = 0,
  RandomWalk            = 1,
  RandomWalkFixedRadius = 2
};

//##################################################################################################
std::string sssMethodToString(SSSMethod sssMethod);

//##################################################################################################
SSSMethod sssMethodFromString(const std::string& sssMethod);

//##################################################################################################
enum class ShaderType
{
  Principled  = 0, //!< The default shaded material.
  None        = 1, //!< Just use the albedo color/texture directly.
};

//##################################################################################################
std::string shaderTypeToString(ShaderType shaderType);

//##################################################################################################
ShaderType shaderTypeFromString(const std::string& shaderType);
}

//##################################################################################################
class TP_MATH_UTILS_EXPORT LegacyMaterial : public ExtendedMaterial
{
public:
  tp_utils::StringID exporterVersion{"Blender3"};
  Legacy::ShaderType shaderType{Legacy::ShaderType::Principled}; //!< What shader should be used to draw the object.

  Legacy::Colorspace albedoColorspace{Legacy::Colorspace::sRGB};

  glm::vec3 sss     {1.0f, 1.0f, 1.0f}; //!< Subsurface scattering color.
  glm::vec3 emission{0.0f, 0.0f, 0.0f}; //!< Emission color.
  glm::vec3 velvet  {0.0f, 0.0f, 0.0f}; //!< Velvet color. OBSOLETE

  float sssScale{0.0f};                 //!< Subsurface scattering factor.
  float emissionScale{0.0f};            //!< Emission factor.
  float velvetScale{0.0f};              //!< Velvet factor. OBSOLETE
  float heightScale{0.01f};             //!< Used to scale subdivision height maps.
  float heightMidlevel{0.5f};           //!< Used to offset subdivision height maps.

  float specular{0.5f};                 //!<
  float ior{1.45f};                     //!< Index of refraction.
  float sheen{0.0f};                    //!<
  float sheenTint{0.5f};                //!<
  float clearCoat{0.0f};                //!<
  float clearCoatRoughness{0.0f};       //!<

  float    iridescentFactor{0.0f}; // OBSOLETE
  float    iridescentOffset{0.0f}; // OBSOLETE
  float iridescentFrequency{0.0f}; // OBSOLETE

  glm::vec3 sssRadius{1.0f,0.2f,0.1f};  //!< Subsurface scattering radius per color.

  Legacy::SSSMethod sssMethod{Legacy::SSSMethod::ChristensenBurley}; //!< Subsurface scattering method.

  float normalStrength{1.0f};           //!< Used to scale normals.

  bool rayVisibilityCamera        {true};  //!< Blender cycles ray visibility options.
  bool rayVisibilityDiffuse       {true};  //!< Blender cycles ray visibility options.
  bool rayVisibilityGlossy        {true};  //!< Blender cycles ray visibility options.
  bool rayVisibilityTransmission  {true};  //!< Blender cycles ray visibility options.
  bool rayVisibilityScatter       {true};  //!< Blender cycles ray visibility options.
  bool rayVisibilityShadow        {true};  //!< Blender cycles ray visibility options.
  bool lightPathReflection       {false};  //!< Light Path setup for reflection rays (used only in IBSG).

  tp_utils::StringID              specularTexture; //!< Grey scale.
  tp_utils::StringID              emissionTexture; //!< RGB.
  tp_utils::StringID                   sssTexture; //!< RGB.
  tp_utils::StringID              sssScaleTexture; //!< Grey scale.
  tp_utils::StringID                heightTexture; //!< Grey scale, Subdivision height.
  tp_utils::StringID                 sheenTexture; //!< Grey scale.
  tp_utils::StringID             sheenTintTexture; //!< Grey scale.
  tp_utils::StringID             clearCoatTexture; //!< Grey scale.
  tp_utils::StringID    clearCoatRoughnessTexture; //!< Grey scale.
  tp_utils::StringID                velvetTexture; //!< OBSOLETE Color
  tp_utils::StringID          velvetFactorTexture; //!< OBSOLETE Grey scale, Mix 1=Velvet, 0=Principled
  tp_utils::StringID      iridescentFactorTexture; //!< OBSOLETE Grey scale, Mix 1=Iridescent color, 0=Base color.
  tp_utils::StringID      iridescentOffsetTexture; //!< OBSOLETE Grey scale, rotate the hue of the iridescent color.
  tp_utils::StringID   iridescentFrequencyTexture; //!< OBSOLETE Grey scale, multiplied with the hue angle to alter the frequency of color change.

  //################################################################################################
  void saveState(tp_utils::JSON& j) const override;

  //################################################################################################
  void loadState(const tp_utils::JSON& j) override;

  //################################################################################################
  static void view(const Material& material, const std::function<void(const LegacyMaterial&)>& closure);

  //################################################################################################
  void allTextureIDs(std::unordered_set<tp_utils::StringID>& textureIDs, const ExtractTextureIDs& extractTextureIDs) const override;

  //################################################################################################
  template<typename T>
  void updateTypedTextures(const T& closure)
  {
    closure(             "specularTexture",              specularTexture, "Specular"              );
    closure(             "emissionTexture",              emissionTexture, "Emission"              );
    closure(                  "sssTexture",                   sssTexture, "SSS"                   );
    closure(             "sssScaleTexture",              sssScaleTexture, "SSS scale"             );
    closure(               "heightTexture",                heightTexture, "Height"                );
    closure(                "sheenTexture",                 sheenTexture, "Sheen"                 );
    closure(            "sheenTintTexture",             sheenTintTexture, "Sheen tint"            );
    closure(            "clearCoatTexture",             clearCoatTexture, "Clear coat"            );
    closure(   "clearCoatRoughnessTexture",    clearCoatRoughnessTexture, "Clear coat roughness"  );
    closure(               "velvetTexture",                velvetTexture, "Velvet"                );
    closure(         "velvetFactorTexture",          velvetFactorTexture, "Velvet factor"         );
    closure(     "iridescentFactorTexture",      iridescentFactorTexture, "Iridescent factor"     );
    closure(     "iridescentOffsetTexture",      iridescentOffsetTexture, "Iridescent offset"     );
    closure(  "iridescentFrequencyTexture",   iridescentFrequencyTexture, "Iridescent frequency"  );
  }

  //################################################################################################
  template<typename T>
  void viewTypedTextures(const T& closure) const
  {
    const_cast<LegacyMaterial*>(this)->updateTypedTextures([&](const auto& type, const auto& value, const auto& pretty){closure(type, value, pretty);});
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
