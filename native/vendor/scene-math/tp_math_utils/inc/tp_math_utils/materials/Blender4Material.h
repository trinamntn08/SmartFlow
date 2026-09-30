#pragma once

#include "tp_math_utils/Material.h"

namespace tp_math_utils
{
namespace Blender4
{
//##################################################################################################
enum class Colorspace
{
  sRGB       = 0, //!< The default colorspace
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
  RandomWalkSkin        = 2
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

//##################################################################################################
enum class SpecularDistribution
{
  GGX             = 0,
  MultiScatterGGX = 1,
};

//##################################################################################################
std::string specularDistributionToString(const SpecularDistribution& specularDistribution);

//##################################################################################################
SpecularDistribution specularDistributionFromString(const std::string& specularDistribution);

}

//##################################################################################################
class TP_MATH_UTILS_EXPORT Blender4Material : public ExtendedMaterial
{
public:
  tp_utils::StringID   exporterVersion{"Blender4"};
  Blender4::ShaderType shaderType{Blender4::ShaderType::Principled}; //!< What shader should be used to draw the object.
  Blender4::Colorspace albedoColorspace{Blender4::Colorspace::sRGB};

  float ior                            {1.5f}   ;

  Blender4::SSSMethod sssMethod {Blender4::SSSMethod::ChristensenBurley}; //!< Subsurface scattering method.
  float sssWeight                      {0.0f}   ;
  glm::vec3 sssRadius      {1.0f, 0.2f, 0.1f}   ;
  float sssScale                      {0.05f}   ;
  float sssIOR                         {1.4f}   ;
  float sssAnisotropy                  {0.0f}   ;

  float specularIORLevel               {0.25f}  ;
  glm::vec4 specularTint               {1.0f}   ;
  Blender4::SpecularDistribution specularDistribution{Blender4::SpecularDistribution::GGX};

  float coatWeight                     {0.0f}   ;
  float coatRoughness                  {0.0f}   ;
  float coatIOR                        {0.0f}   ;
  glm::vec4 coatTint                   {1.0f}   ;

  float sheenWeight                    {0.0f}   ;
  float sheenRoughness                 {0.5f}   ;
  glm::vec4 sheenTint                  {1.0f}   ;

  glm::vec4 emissionColor              {1.0f}   ;
  float emissionStrength               {1.0f}   ;

  float diffuseRoughness               {0.0f}   ;

  float anisotropic                    {0.0f}   ;
  float anisotropicRotation            {0.0f}   ;

  float thinFilmThickness              {0.0f}   ;
  float thinFilmIOR                    {1.33f}  ;

  float heightScale                    {0.01f}  ;  //!< Used to scale subdivision height maps.
  float heightMidlevel                 {0.5f}   ;  //!< Used to offset subdivision height maps.

  float normalStrength                 {1.0f}   ;  //!< Used to scale normals.

  bool rayVisibilityCamera             {true}   ;  //!< Blender cycles ray visibility options.
  bool rayVisibilityDiffuse            {true}   ;  //!< Blender cycles ray visibility options.
  bool rayVisibilityGlossy             {true}   ;  //!< Blender cycles ray visibility options.
  bool rayVisibilityTransmission       {true}   ;  //!< Blender cycles ray visibility options.
  bool rayVisibilityScatter            {true}   ;  //!< Blender cycles ray visibility options.
  bool rayVisibilityShadow             {true}   ;  //!< Blender cycles ray visibility options.
  bool lightPathReflection             {false}  ; //!< Light Path setup for reflection rays (used only in IBSG).

  tp_utils::StringID             specularTexture; //!< Grey scale.
  tp_utils::StringID             emissionTexture; //!< RGB.
  tp_utils::StringID                  sssTexture; //!< RGB.
  tp_utils::StringID             sssScaleTexture; //!< Grey scale.
  tp_utils::StringID               heightTexture; //!< Grey scale, Subdivision height.
  tp_utils::StringID                sheenTexture; //!< Grey scale.
  tp_utils::StringID            sheenTintTexture; //!< Grey scale.
  tp_utils::StringID                 coatTexture; //!< Grey scale.
  tp_utils::StringID        coatRoughnessTexture; //!< Grey scale.

  //################################################################################################
  void saveState(tp_utils::JSON& j) const override;

  //################################################################################################
  void loadState(const tp_utils::JSON& j) override;

  //################################################################################################
  static void view(const Material& material, const std::function<void(const Blender4Material&)>& closure);

  //################################################################################################
  void allTextureIDs(std::unordered_set<tp_utils::StringID>& textureIDs, const ExtractTextureIDs& extractTextureIDs) const override;

  //################################################################################################
  template<typename T>
  void updateTypedTextures(const T& closure)
  {
    closure(      "specularTexture",       specularTexture, "Specular IOR Level" );
    closure(      "emissionTexture",       emissionTexture, "Emission color"     );
    closure(           "sssTexture",            sssTexture, "SSS weight"         );
    closure(      "sssScaleTexture",       sssScaleTexture, "SSS scale"          );
    closure(        "heightTexture",         heightTexture, "Height"             );
    closure(         "sheenTexture",          sheenTexture, "Sheen weight"       );
    closure(     "sheenTintTexture",      sheenTintTexture, "Sheen tint"         );
    closure(          "coatTexture",           coatTexture, "Coat weight"        );
    closure( "coatRoughnessTexture",  coatRoughnessTexture, "Coat roughness"     );
  }

  //################################################################################################
  template<typename T>
  void viewTypedTextures(const T& closure) const
  {
    const_cast<Blender4Material*>(this)->updateTypedTextures([&](const auto& type, const auto& value, const auto& pretty){closure(type, value, pretty);});
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
