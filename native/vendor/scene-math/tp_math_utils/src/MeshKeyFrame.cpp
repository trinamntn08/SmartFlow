#include "tp_math_utils/MeshKeyFrame.h"

#include "tp_math_utils/JSONUtils.h"

#include "glm/gtx/euler_angles.hpp" // IWYU pragma: keep
#include "glm/gtx/matrix_decompose.hpp"  // IWYU pragma: keep
#include "tp_utils/DebugUtils.h"
namespace tp_math_utils
{

//##################################################################################################
void MeshKeyFrame::saveState(nlohmann::json& j) const
{
  j["time"] = t;

  j["position"] = tp_math_utils::vec3ToJSON(position);
  j["rotation"] = tp_math_utils::vec3ToJSON(rotation);
  j["scale"] = tp_math_utils::vec3ToJSON(scale);
}

//##################################################################################################
void MeshKeyFrame::loadState(const nlohmann::json& j)
{
  {
    if(auto i = j.find("time"); i!=j.end() && i->is_number_unsigned())
      t = size_t(*i);
    else if(i = j.find("t"); i!=j.end() && i->is_number_unsigned())
      t = size_t(*i);
    else
      t = 0;
  }

  position = tp_math_utils::getJSONVec3(j, "position", position);
  rotation = tp_math_utils::getJSONVec3(j, "rotation", rotation);
  scale = tp_math_utils::getJSONVec3(j, "scale", scale);

  clampScale(scale);
  clampPosition(position);
  clampRotation(rotation);
}

//##################################################################################################
glm::mat4 MeshKeyFrame::calculateModelMatrix() const
{
  glm::mat4 m{1.0f};

  m = glm::translate(m, position);

  auto r = glm::radians(rotation);
  m = m * glm::eulerAngleXYZ(r.x, r.y, r.z);
  m = glm::scale(m, scale);

  return m;
}

//##################################################################################################
glm::mat4 MeshKeyFrame::calculateTramslationMatrix() const
{
  glm::mat4 m{1.0f};
  return glm::translate(m, position);
}

//##################################################################################################
glm::mat4 MeshKeyFrame::calculateRotationMatrix() const
{
  auto r = glm::radians(rotation);
  return glm::eulerAngleXYZ(r.x, r.y, r.z);
}

//##################################################################################################
glm::mat4 MeshKeyFrame::calculateObjectToBoundingBoxMatrix() const
{
  glm::mat4 m{1.0f};

  auto r = glm::radians(rotation);
  m = m * glm::eulerAngleXYZ(r.x, r.y, r.z);
  m = glm::scale(m, scale);

  return m;
}

//##################################################################################################
void MeshKeyFrame::populateFromModelMatrix(const glm::mat4& m, int copyWhat)
{
  glm::vec3 scale;
  glm::quat rotation;
  glm::vec3 translation;
  glm::vec3 skew;
  glm::vec4 perspective;
  if(glm::decompose(m, scale, rotation, translation, skew, perspective))
  {
    if(copyWhat&Scale)
    {
      this->scale = scale;
      clampScale(this->scale);
    }

    if(copyWhat&Position)
    {
      this->position = translation;
      clampPosition(this->position);
    }

    if(copyWhat&Rotation)
    {
      glm::extractEulerAngleXYZ(glm::mat4(rotation),
                                this->rotation.x,
                                this->rotation.y,
                                this->rotation.z);
      this->rotation.x = glm::degrees(this->rotation.x);
      this->rotation.y = glm::degrees(this->rotation.y);
      this->rotation.z = glm::degrees(this->rotation.z);
      clampRotation(this->rotation);
    }
  }
}
//##################################################################################################
void MeshKeyFrame::populateFromModelMatrixQuaternion(const glm::mat4& m, int copyWhat)
{
  glm::vec3 scale;
  glm::quat rotation;
  glm::vec3 translation;
  glm::vec3 skew;
  glm::vec4 perspective;
  if(glm::decompose(m, scale, rotation, translation, skew, perspective))
  {
    if(copyWhat&Scale)
    {
      this->scale.x = glm::length(glm::vec3(m[0]));
      this->scale.y = glm::length(glm::vec3(m[1]));
      this->scale.z = glm::length(glm::vec3(m[2]));

      clampScale(this->scale);
    }

    if(copyWhat&Position)
    {
      this->position = translation;
      clampPosition(this->position);
    }

    if(copyWhat&Rotation)
    {
      glm::vec3 eulerRad = glm::eulerAngles(rotation);

      this->rotation.x = glm::degrees(eulerRad.x);
      this->rotation.y = glm::degrees(eulerRad.y);
      this->rotation.z = glm::degrees(eulerRad.z);

      clampRotation(this->rotation);
    }
  }
}

//##################################################################################################
void MeshKeyFrame::setRotationXYZ(const glm::vec3& rotation)
{
  this->rotation = rotation;
  clampRotation(this->rotation);
}

//##################################################################################################
glm::vec3 MeshKeyFrame::rotationXYZ() const
{
  return rotation;
}

//##################################################################################################
void MeshKeyFrame::setRotationZYZ(const glm::vec3& rotation)
{
  glm::vec3 v = glm::radians(rotation);
  glm::mat4 m = glm::eulerAngleZYZ(v.x, v.y, v.z);
  glm::extractEulerAngleXYZ(m, v.x, v.y, v.z);
  this->rotation = glm::degrees(v);
  clampRotation(this->rotation);
}

//##################################################################################################
glm::vec3 MeshKeyFrame::rotationZYZ() const
{
  glm::vec3 v = glm::radians(rotation);
  glm::mat4 m = glm::eulerAngleXYZ(v.x, v.y, v.z);
  glm::extractEulerAngleZYZ(m, v.x, v.y, v.z);
  return glm::degrees(v);
}

//##################################################################################################
MeshKeyFrame MeshKeyFrame::lerp(const MeshKeyFrame& before, const MeshKeyFrame& after, float f)
{
  MeshKeyFrame interpolated = after;

  interpolated.position = glm::lerp(before.position, after.position, f);

  glm::quat qBefore = glm::quat_cast(before.calculateRotationMatrix());
  glm::quat qAfter  = glm::quat_cast(after.calculateRotationMatrix());

  glm::mat4 m = glm::mat4_cast(glm::slerp(qBefore, qAfter, f));
  glm::vec3 v{0.0f,0.0f,0.0f};
  glm::extractEulerAngleXYZ(m, v.x, v.y, v.z);
  interpolated.rotation = glm::degrees(v);

  clampScale(interpolated.scale);
  clampPosition(interpolated.position);
  clampRotation(interpolated.rotation);

  return interpolated;
}

//##################################################################################################
MeshKeyFrame MeshKeyFrame::multiply(const std::vector<MeshKeyFrame>& keyFrames)
{
  glm::mat4 m{1.0f};

  for(const auto& keyFrame : keyFrames)
    m *= keyFrame.calculateModelMatrix();

  MeshKeyFrame result;
  result.populateFromModelMatrix(m);
  return result;
}

}
