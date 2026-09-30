#pragma once

#include "Globals.h" // IWYU pragma: keep
#include "glm/glm.hpp" // IWYU pragma: keep

#include "json.hpp"

namespace tp_math_utils
{
class ObjectInstance;

//##################################################################################################
//! The configuration of an object at a point in time.
struct TP_MATH_UTILS_EXPORT MeshKeyFrame
{
  size_t t{0}; //!< Time since start of animation

  // The final model matrix used to render the geometry will be multiplied by the model matricies of
  // parent objects, these values are used to build the model matrix of this object:
  glm::vec3 position{0, 0, 0}; //!< Position of the object.
  glm::vec3 rotation{0, 0, 0}; //!< Rotation of the object, in degrees.
  glm::vec3 scale{1, 1, 1};    //!< Scale of the object.


  //##################################################################################################
  static float clampScaleAxis(float v)
  {
    v = std::clamp(v, 0.005f, 10000.0f);
    if(std::isnan(v) || std::isinf(v))
      v = 1.0f;
    return v;
  }

  //##################################################################################################
  static void clampScale(glm::vec3& scale)
  {
    scale.x = clampScaleAxis(scale.x);
    scale.y = clampScaleAxis(scale.y);
    scale.z = clampScaleAxis(scale.z);
  }

  //##################################################################################################
  static float clampAxis(float v)
  {
    v = std::clamp(v, -10000.0f, 10000.0f);
    if(std::isnan(v) || std::isinf(v))
      v = 1.0f;
    return v;
  }

  //##################################################################################################
  static void clampPosition(glm::vec3& position)
  {
    position.x = clampAxis(position.x);
    position.y = clampAxis(position.y);
    position.z = clampAxis(position.z);
  }

  //##################################################################################################
  static void clampRotation(glm::vec3& rotation) { clampPosition(rotation); }

  //################################################################################################
  void saveState(nlohmann::json& j) const;

  //################################################################################################
  void loadState(const nlohmann::json& j);

  //################################################################################################
  //! Calculate the model matrix for this object, this is not yet multiplied with parent matricies.
  glm::mat4 calculateModelMatrix() const;

  //################################################################################################
  //! Calculate the tramslation matrix for this object, this is not yet multiplied with parent matricies.
  glm::mat4 calculateTramslationMatrix() const;

  //################################################################################################
  //! Calculate the rotation matrix for this object, this is not yet multiplied with parent matricies.
  glm::mat4 calculateRotationMatrix() const;

  //################################################################################################
  glm::mat4 calculateObjectToBoundingBoxMatrix() const;

  enum CopyWhat : int
  {
    Scale    = 0b001,
    Position = 0b010,
    Rotation = 0b100
  };

  //################################################################################################
  void populateFromModelMatrix(const glm::mat4& m, int copyWhat=Scale|Position|Rotation);

   //################################################################################################
  void populateFromModelMatrixQuaternion(const glm::mat4& m, int copyWhat=Scale|Position|Rotation);

  //################################################################################################
  void setRotationXYZ(const glm::vec3& rotation);

  //################################################################################################
  [[nodiscard]] glm::vec3 rotationXYZ() const;

  //################################################################################################
  void setRotationZYZ(const glm::vec3& rotation);

  //################################################################################################
  [[nodiscard]] glm::vec3 rotationZYZ() const;

  //################################################################################################
  static MeshKeyFrame lerp(const MeshKeyFrame& before, const MeshKeyFrame& after, float f);

  //################################################################################################
  //! Multiply a stack of transformations into a single transformation.
  static MeshKeyFrame multiply(const std::vector<MeshKeyFrame>& keyFrames);
};

//##################################################################################################
struct ObjectTransformation
{
  MeshKeyFrame origin;
  MeshKeyFrame animation;
  MeshKeyFrame combined;
};

}
