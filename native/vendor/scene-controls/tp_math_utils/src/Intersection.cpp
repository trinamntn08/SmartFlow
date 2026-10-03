#ifndef tp_math_utils_Intersection_h
#define tp_math_utils_Intersection_h

#include "tp_math_utils/Plane.h"
#include "tp_math_utils/Ray.h"
#include <cmath>

namespace tp_math_utils
{

//##################################################################################################
/*!
Return false if ray is parallel to the plane (including ray belongs to the plane),
true otherwise with intersection parameter filled.
*/

template<typename FLOAT_TYPE>
bool rayPlaneIntersectionImpl(const Ray& ray, const Plane& plane, glm::vec<3,FLOAT_TYPE>& intersection)
{
  const auto& Po = plane.pointAndNormal()[0];
  const auto& Pn = plane.pointAndNormal()[1];
  const auto& Ro = ray.p0;
  const auto  Rd = ray.p1-ray.p0;

  // SmartFlow local patch: intersect the viewing line, rejecting parallel,
  // coplanar, degenerate and non-finite inputs (see SMARTFLOW_PATCHES.md).
  const glm::dvec3 direction(Rd), normal(Pn), origin(Ro), point(Po);
  const double lengthProduct = glm::length(direction)*glm::length(normal);
  const double denom = glm::dot(direction, normal);
  if(!std::isfinite(lengthProduct) || lengthProduct == 0.0 ||
     !std::isfinite(denom) || std::fabs(denom) <= 1e-10*lengthProduct)
    return false;
  const auto result = origin + (glm::dot(point-origin, normal)/denom)*direction;
  for(int i=0; i<3; i++)
    if(!std::isfinite(result[i]))
      return false;
  intersection = glm::vec<3,FLOAT_TYPE>(result);
  for(int i=0; i<3; i++)
    if(!std::isfinite(intersection[i]))
      return false;
  return true;
}

//##################################################################################################
bool rayPlaneIntersection(const Ray& ray, const Plane& plane, glm::vec3& intersection)
{
  return rayPlaneIntersectionImpl(ray, plane, intersection);
}

//##################################################################################################
bool rayPlaneIntersection(const Ray& ray, const Plane& plane, glm::dvec3& intersection)
{
  return rayPlaneIntersectionImpl(ray, plane, intersection);
}

}

#endif
