#ifndef tp_math_utils_Ray_h
#define tp_math_utils_Ray_h

#include "tp_math_utils/Globals.h"

namespace tp_math_utils
{

//##################################################################################################
struct TP_MATH_UTILS_EXPORT Ray
{
  Ray(const glm::vec3& p0_={0.0f, 0.0f, 0.0f}, const glm::vec3& p1_={0.0f, 0.0f, 0.0f}):
    p0(p0_),
    p1(p1_)
  {
  }

  glm::vec3 p0;
  glm::vec3 p1;
};

//##################################################################################################
struct TP_MATH_UTILS_EXPORT DRay
{
  DRay(const glm::dvec3& p0_={0.0, 0.0, 0.0}, const glm::dvec3& p1_={0.0, 0.0, 0.0}):
    p0(p0_),
    p1(p1_)
  {
  }

  glm::dvec3 p0;
  glm::dvec3 p1;
};

}

#endif
