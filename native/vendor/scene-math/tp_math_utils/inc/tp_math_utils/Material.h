#pragma once

#include "tp_math_utils/Globals.h"

#include "tp_utils/StringID.h"
#include "tp_utils/JSONUtils.h"

#include <utility>

#include <unordered_set>

namespace tp_math_utils
{
class OpenGLMaterial;
class LegacyMaterial;
class ExternalMaterial;
class Blender4Material;

//##################################################################################################
using ExtractTextureIDs = std::function<void(const ExternalMaterial&, std::unordered_set<tp_utils::StringID>&)>;

//##################################################################################################
class TP_MATH_UTILS_EXPORT ExtendedMaterial
{
public:
  //################################################################################################
  virtual ~ExtendedMaterial();

  //################################################################################################
  virtual void saveState(tp_utils::JSON& j) const=0;

  //################################################################################################
  virtual void loadState(const tp_utils::JSON& j)=0;

  //################################################################################################
  virtual void allTextureIDs(std::unordered_set<tp_utils::StringID>& textureIDs, const ExtractTextureIDs& extractTextureIDs) const;

  //################################################################################################
  virtual void appendFileIDs(std::vector<std::pair<tp_utils::StringID, tp_utils::StringID>>& fileIDs) const;

  //################################################################################################
  std::unordered_set<tp_utils::StringID> allTextures(const ExtractTextureIDs& extractTextureIDs) const;
};

//##################################################################################################
struct UVTransformation
{
  enum TransformationType
  {
    Transform2D,
    Transform3D
  };

  glm::vec3      skewUVW{0.0f, 0.0f, 0.0f}; //!< 3D Skew 
  glm::vec3     scaleUVW{1.0f, 1.0f, 1.0f}; //!< 3D Scale
  glm::vec3 translateUVW{0.0f, 0.0f, 0.0f}; //!< 3D Translate
  glm::vec3    rotateUVW{0.0f, 0.0f, 0.0f}; //!< 3D Rotate Degrees.

  glm::vec2      skewUV{0.0f, 0.0f}; //!< Skew the texture coords. m[1][0] = glm::tan(glm::radians(skewUV.x)); m[0][1] = glm::tan(glm::radians(skewUV.y));
  glm::vec2     scaleUV{1.0f, 1.0f}; //!< Scale the texture coords.
  glm::vec2 translateUV{0.0f, 0.0f}; //!< Translate the texture coords.
  float        rotateUV{0.0f};       //!< Rotate the texture coords. Degrees.

  TransformationType type{TransformationType::Transform2D};

  //################################################################################################
  static TransformationType toType(const std::string& type)
  {
    if(type == "Transform2D")
    {
      return TransformationType::Transform2D;
    }
    else if(type == "Transform3D")
    {
      return TransformationType::Transform3D;
    }
    
    return TransformationType::Transform2D;
  }
  //################################################################################################
  static std::string toString(TransformationType type)
  {
    switch(type)
    {
      case TransformationType::Transform2D:
        return "Transform2D";
      case TransformationType::Transform3D:
        return "Transform3D";
      default:
        return "Transform2D";
    }
  }

  //################################################################################################
  bool operator==(const UVTransformation& other) const
  {
    return skewUVW      == other.skewUVW               &&
           scaleUVW     == other.scaleUVW              &&
           translateUVW == other.translateUVW          &&
           rotateUVW    == other.rotateUVW             &&
           skewUV       == other.skewUV                &&
           scaleUV      == other.scaleUV               &&
           translateUV  == other.translateUV           &&
           glm::abs(rotateUV - other.rotateUV) < 1e-6f &&
           type         == other.type;
  }

  //################################################################################################
  bool operator!=(const UVTransformation& other) const
  {
    return !(*this == other);
  }

  //################################################################################################
  glm::mat3 uvMatrix() const;

  //################################################################################################
  glm::mat3 lightMaskUVMatrix() const;

  //################################################################################################
  glm::mat4 uvwMatrix() const;

  //################################################################################################
  bool isIdentity() const;

  //################################################################################################
  void saveState(tp_utils::JSON& j) const;

  //################################################################################################
  void loadState(const tp_utils::JSON& j);
};

//##################################################################################################
enum class ExporterVersion
{
  Blender3,
  Blender4,
};

//##################################################################################################
std::string exporterVersionToString(const ExporterVersion& exporterVersion);

//##################################################################################################
ExporterVersion exporterVersionFromString(const std::string& exporterVersion);

//##################################################################################################
struct TP_MATH_UTILS_EXPORT Material
{
  tp_utils::StringID name;

  std::vector<ExtendedMaterial*> extendedMaterials;

  UVTransformation uvTransformation; //! Modify UV coords.

  ExporterVersion exporterVersion{ExporterVersion::Blender3};

  //################################################################################################
  Material();

  //################################################################################################
  Material(const tp_utils::StringID& name_);

  //################################################################################################
  Material(const Material& other);

  //################################################################################################
  Material(Material&& other) noexcept;

  //################################################################################################
  Material& operator=(const Material& other);

  //################################################################################################
  Material& operator=(Material&& other) noexcept;

  //################################################################################################
  ~Material();

  //################################################################################################
  OpenGLMaterial* findOrAddOpenGL();

  //################################################################################################
  LegacyMaterial* findOrAddLegacy();

  //################################################################################################
  Blender4Material* findOrAddBlender4();

  //################################################################################################
  ExternalMaterial* findOrAddExternal(const tp_utils::StringID& assetType);

  //################################################################################################
  void removeExternal(const tp_utils::StringID& assetType);

  //################################################################################################
  bool hasExternal(const tp_utils::StringID& assetType) const;

  //################################################################################################
  bool isBlender4Material() const;

  //################################################################################################
  void updateOpenGL(const std::function<void(OpenGLMaterial&)>& closure) const;

  //################################################################################################
  void updateLegacy(const std::function<void(LegacyMaterial&)>& closure) const;

  //################################################################################################
  void updateBlender4(const std::function<void(Blender4Material&)>& closure) const;

  //################################################################################################
  void updateExternal(const tp_utils::StringID& assetType,
                    const std::function<void(ExternalMaterial&)>& closure) const;

  //################################################################################################
  void viewOpenGL(const std::function<void(const OpenGLMaterial&)>& closure) const;

  //################################################################################################
  void viewLegacy(const std::function<void(const LegacyMaterial&)>& closure) const;

  //################################################################################################
  void viewBlender4(const std::function<void(const Blender4Material&)>& closure) const;

  //################################################################################################
  void viewExternal(const tp_utils::StringID& assetType,
                    const std::function<void(const ExternalMaterial&)>& closure) const;

  //################################################################################################
  void allTextureIDs(std::unordered_set<tp_utils::StringID>& textureIDs, const ExtractTextureIDs& extractTextureIDs) const;

  //################################################################################################
  void appendFileIDs(std::vector<std::pair<tp_utils::StringID, tp_utils::StringID>>& fileIDs) const;

  //################################################################################################
  std::unordered_set<tp_utils::StringID> allTextures(const ExtractTextureIDs& extractTextureIDs) const;

  //################################################################################################
  std::vector<std::pair<tp_utils::StringID, tp_utils::StringID>> allFiles() const;

  //################################################################################################
  void saveState(tp_utils::JSON& j) const;

  //################################################################################################
  void saveUVMatrix(tp_utils::JSON& j) const;

  //################################################################################################
  void loadState(const tp_utils::JSON& j);
};

}
