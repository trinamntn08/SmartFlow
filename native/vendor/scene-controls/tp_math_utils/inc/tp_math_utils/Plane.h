#ifndef tp_math_utils_Plane_h
#define tp_math_utils_Plane_h

#include "tp_math_utils/Geometry3D.h"

#include <array>

namespace tp_math_utils
{

//##################################################################################################
class TP_MATH_UTILS_EXPORT Plane
{
public:
  //################################################################################################
  //! Define a plane facing up the y axis
  Plane();

  //################################################################################################
  //! Define a plane with an origin point and a normal
  Plane(const glm::vec3& point, const glm::vec3& normal);

  //################################################################################################
  //! Define a plane using three points on the surface of the plane
  Plane(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3);


  //################################################################################################
  //! Return the plane as an array with a point and a normal
  /*!
  This will retuns the plane described as an origin point and a normal, it will calculate these
  values if they have not yet been set.

  \return An array with two elements in it
  */
  const glm::vec3* pointAndNormal() const;

  //################################################################################################
  //! Define a plane using three points on the surface of the plane
  const glm::vec3* threePoints() const;

  //################################################################################################
  static Plane planeFromPoints(const std::vector<glm::vec3>& points);

  //################################################################################################
  Geometry3D geometry(float scaleI, float scaleJ, float scaleU, float scaleV, size_t tilesI=1, size_t tilesJ=1) const;

private:
  mutable std::array<glm::vec3, 2> m_pointAndNormal{};
  mutable bool m_pointAndNormalValid;

  mutable std::array<glm::vec3, 3> m_threePoints{};
  mutable bool m_threePointsValid;
};

}

#endif
