#ifndef tp_math_utils_Globals_h
#define tp_math_utils_Globals_h

#include "tp_utils/StringID.h" // IWYU pragma: keep

#include "lib_platform/RandomDevice.h"

#if defined(TP_MATH_UTILS_LIBRARY)
#  define TP_MATH_UTILS_EXPORT TP_EXPORT
#else
#  define TP_MATH_UTILS_EXPORT TP_IMPORT
#endif

#include "glm/glm.hpp" // IWYU pragma: keep
#include "glm/ext.hpp" // IWYU pragma: keep

#include <random>
#include <ostream>

//##################################################################################################
inline glm::vec3 tpProj(const glm::mat4& m, const glm::vec3& c)
{
  auto p = m*glm::vec4(c, 1.0f);
  return glm::vec3(p)/p.w;
}

//##################################################################################################
//! Math utils for 3D graphics.
/*!

*/
namespace tp_math_utils
{

//##################################################################################################
//! NOTE: sincos is optimized by the compiler to a single instruction when available
template<typename T>
inline std::pair<T, T> sincos(T rad) { return {std::sin(rad), std::cos(rad)}; };

//##################################################################################################
struct TP_MATH_UTILS_EXPORT RNG
{
  lib_platform::RandomDevice rd;
  std::minstd_rand0 mt;
  std::uniform_real_distribution<float> uniformDist;

  //################################################################################################
  RNG():
    mt(rd()),
    uniformDist(-1.0f, 1.0f)
  {

  }

  //################################################################################################
  //! Returns a pseudo-random floating-point value within [0,1].
  float randF()
  {
    return float(mt())*(1.0f/2147483646.0f);
  }

  //################################################################################################
  //! Returns a pseudo-random floating-point value within [-1,1].
  float randF2()
  {
    return (float(mt())*(2.0f/2147483646.0f))-1.0f;
  }  
  //################################################################################################
  //! Returns a pseudo-random floating-point value within [-1,1].
  float randF2Squared()
  {
    auto r = randF2();
    auto rr = r*r;
    return r<0.0f?-rr:rr;
  }

  //################################################################################################
  //! Returns a random floating-point value, uniformly distributed within the given interval.
  float randUniformDistribution()
  {
    return uniformDist(mt);
  }

  //################################################################################################
  float randRange(float min, float max)
  {
    return min + (randF() * (max-min));
  }

  //################################################################################################
  void randomIndexes(size_t lessThan, size_t count, std::vector<size_t>& results)
  {
    count = tpMin(lessThan, count);

    results.clear();
    results.reserve(count);
    for(size_t c=0; c<count; c++)
    {
      size_t i = mt()%lessThan;

      while(tpContains(results, i))
      {
        i++;
        if(i>=lessThan)
          i=0;
      }

      results.push_back(i);
    }
  }

  //################################################################################################
  size_t randomIndex(size_t lessThan)
  {
    return mt()%lessThan;
  }
};

//##################################################################################################
bool isPowerOf2(size_t v);

//##################################################################################################
//! Round up to nearest power of 2, 0=0, overflow=0
size_t powerOf2(size_t v);

//##################################################################################################
//! Round to the nearest
size_t closestPowerOf2(size_t v);

//##################################################################################################
glm::vec3 rgb2hsv(const glm::vec3& rgb);

//##################################################################################################
glm::vec3 hsv2rgb(const glm::vec3& hsv);

}

//##################################################################################################
template<typename T, glm::precision P>
std::ostream& operator<<(std::ostream& os, const glm::tvec2<T, P>& v)
{
  os << '(' << v.x << ',' << v.y << ')';
  return os;
}

//##################################################################################################
template<typename T, glm::precision P>
std::ostream& operator<<(std::ostream& os, const glm::tvec3<T, P>& v)
{
  os << '(' << v.x << ',' << v.y << ',' << v.z << ')';
  return os;
}

//##################################################################################################
template<typename T, glm::precision P>
std::ostream& operator<<(std::ostream& os, const glm::tvec4<T, P>& v)
{
  os << '(' << v.x << ',' << v.y << ',' << v.z << ',' << v.w << ')';
  return os;
}

//##################################################################################################
template<typename T, glm::precision P>
std::ostream& operator<<(std::ostream& os, const glm::tmat4x4<T, P>& v)
{
  os << "([" << v[0][0] << ',' << v[1][0] << ',' << v[2][0] << ',' << v[3][0] << "]\n";
  os << " [" << v[0][1] << ',' << v[1][1] << ',' << v[2][1] << ',' << v[3][1] << "]\n";
  os << " [" << v[0][2] << ',' << v[1][2] << ',' << v[2][2] << ',' << v[3][2] << "]\n";
  os << " [" << v[0][3] << ',' << v[1][3] << ',' << v[2][3] << ',' << v[3][3] << "])";
  return os;
}

//##################################################################################################
template<typename T, glm::precision P>
std::ostream& operator<<(std::ostream& os, const glm::tmat3x3<T, P>& v)
{
  os << "([" << v[0][0] << ',' << v[1][0] << ',' << v[2][0] << "]\n";
  os << " [" << v[0][1] << ',' << v[1][1] << ',' << v[2][1] << "]\n";
  os << " [" << v[0][2] << ',' << v[1][2] << ',' << v[2][2] << "])";
  return os;
}

namespace std
{
//##################################################################################################
template <>
struct hash<glm::ivec2>
{
  size_t operator()(const glm::ivec2& k) const
  {
    return hash<int>()(k.x) ^ hash<int>()(k.y);
  }
};
}

#endif
