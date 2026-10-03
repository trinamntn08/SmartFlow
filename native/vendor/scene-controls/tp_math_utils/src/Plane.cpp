#include "tp_math_utils/Plane.h"

namespace tp_math_utils
{

//##################################################################################################
Plane::Plane():
  m_pointAndNormalValid(true),
  m_threePointsValid(true)
{
  m_pointAndNormal[0] = glm::vec3(0.0f, 0.0f, 0.0f);
  m_pointAndNormal[1] = glm::vec3(0.0f, 0.0f, 1.0f);

  m_threePoints[0] = glm::vec3(0.0f, 0.0f, 0.0f);
  m_threePoints[1] = glm::vec3(1.0f, 0.0f, 0.0f);
  m_threePoints[2] = glm::vec3(0.0f, 1.0f, 0.0f);
}

//##################################################################################################
Plane::Plane(const glm::vec3& point, const glm::vec3& normal):
  m_pointAndNormalValid(true),
  m_threePointsValid(false)
{
  m_pointAndNormal[0] = point;
  m_pointAndNormal[1] = normal;

  m_threePoints[0] = glm::vec3(0.0f, 0.0f, 0.0f);
  m_threePoints[1] = glm::vec3(0.0f, 0.0f, 0.0f);
  m_threePoints[2] = glm::vec3(0.0f, 0.0f, 0.0f);
}

//##################################################################################################
Plane::Plane(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3):
  m_pointAndNormalValid(false),
  m_threePointsValid(true)
{
  m_pointAndNormal[0] = glm::vec3(0.0f, 0.0f, 0.0f);
  m_pointAndNormal[1] = glm::vec3(0.0f, 0.0f, 0.0f);

  m_threePoints[0] = p1;
  m_threePoints[1] = p2;
  m_threePoints[2] = p3;
}

//##################################################################################################
const glm::vec3* Plane::pointAndNormal() const
{
  if(!m_pointAndNormalValid)
  {
    m_pointAndNormal[0] = m_threePoints[0];

    const glm::vec3& a = m_threePoints[0];
    const glm::vec3& b = m_threePoints[1];
    const glm::vec3& c = m_threePoints[2];
    m_pointAndNormal[1] = glm::normalize(glm::cross(c - a, b - a));

    m_pointAndNormalValid = true;
  }

  return m_pointAndNormal.data();
}

//##################################################################################################
const glm::vec3* Plane::threePoints() const
{
  if(!m_threePointsValid)
  {
    glm::vec3 normal = m_pointAndNormal[1];
    normal.x = std::fabs(normal.x);
    normal.y = std::fabs(normal.y);
    normal.z = std::fabs(normal.z);

    //This is used to give us a cross product
    glm::vec3 cardinal;

    //Find the shortest component
    if(normal.x<normal.y)
    {
      if(normal.x<normal.z)
        cardinal = glm::vec3(1.0f, 0.0f, 0.0f);
      else
        cardinal = glm::vec3(0.0f, 0.0f, 1.0f);
    }
    else
    {
      if(normal.y<normal.z)
        cardinal = glm::vec3(0.0f, 1.0f, 0.0f);
      else
        cardinal = glm::vec3(0.0f, 0.0f, 1.0f);
    }

    m_threePoints[0] = m_pointAndNormal[0];
    m_threePoints[1] = glm::cross(m_pointAndNormal[1], cardinal);
    m_threePoints[2] = glm::cross(m_pointAndNormal[1], m_threePoints[1]);
    m_threePoints[1] = glm::cross(m_pointAndNormal[1], m_threePoints[2]);
    m_threePoints[1] += m_threePoints[0];
    m_threePoints[2] += m_threePoints[0];

    m_threePointsValid = true;
  }

  return m_threePoints.data();
}

//##################################################################################################
//http://www.ilikebigbits.com/blog/2017/9/24/fitting-a-plane-to-noisy-points-in-3d
// Fit a plane to a collection of points.
// Fast, and accurate to within a few degrees.
// Returns None if the points do not span a plane.
Plane Plane::planeFromPoints(const std::vector<glm::vec3>& points)
{
  if(points.size()<3)
    return Plane();

  glm::dvec3 sum{0.0, 0.0, 0.0};
  double n = double(points.size());

  for(const glm::vec3& p : points)
    sum += p;

  glm::dvec3 centroid = sum * (1.0 / n);

  // Calculate full 3x3 covariance matrix, excluding symmetries:
  double xx = 0.0; double xy = 0.0; double xz = 0.0;
  double yy = 0.0; double yz = 0.0; double zz = 0.0;

  for(const glm::vec3& p : points)
  {
    glm::dvec3 r = glm::dvec3(p) - centroid;
    xx += r.x * r.x;
    xy += r.x * r.y;
    xz += r.x * r.z;
    yy += r.y * r.y;
    yz += r.y * r.z;
    zz += r.z * r.z;
  }

  xx /= n;
  xy /= n;
  xz /= n;
  yy /= n;
  yz /= n;
  zz /= n;

  glm::dvec3 weighted_dir{0.0, 0.0, 0.0};

  {
      double det_x = yy*zz - yz*yz;
      glm::dvec3 axis_dir{det_x, xz*yz - xy*zz, xy*yz - xz*yy};
      double weight = det_x * det_x;

      if(glm::dot(weighted_dir, axis_dir) < 0.0)
        weight = -weight;

      weighted_dir += axis_dir * weight;
  }

  {
      double det_y = xx*zz - xz*xz;
      glm::dvec3 axis_dir{xz*yz - xy*zz, det_y, xy*xz - yz*xx};
      double weight = det_y * det_y;

      if(glm::dot(weighted_dir, axis_dir) < 0.0)
        weight = -weight;

      weighted_dir += axis_dir * weight;
  }

  {
      double det_z = xx*yy - xy*xy;
      glm::dvec3 axis_dir{xy*yz - xz*yy, xy*xz - yz*xx, det_z};
      double weight = det_z * det_z;

      if(glm::dot(weighted_dir, axis_dir) < 0.0)
       weight = -weight;

      weighted_dir += axis_dir * weight;
  }

  glm::dvec3 normal = glm::normalize(weighted_dir);

  if(std::isinf(normal.x) || std::isinf(normal.y) ||std::isinf(normal.z) ||
     std::isnan(normal.x) || std::isnan(normal.y) ||std::isnan(normal.z))
    return Plane();

  return Plane(centroid, normal);
}
//##################################################################################################
Geometry3D Plane::geometry(float scaleI, float scaleJ, float scaleU, float scaleV, size_t tilesI, size_t tilesJ) const
{
  Geometry3D geometry;

  if(tilesI<1 || tilesJ<1)
    return geometry;

  const glm::vec3* points = threePoints();
  glm::vec3 normal = pointAndNormal()[1];

  if(scaleI<0.0f)normal = normal*-1.0f;
  if(scaleJ<0.0f)normal = normal*-1.0f;

  for(size_t i=0; i<=tilesI; i++)
  {
    float fI = float(i)/float(tilesI);
    float currentI = fI * scaleI;
    float currentU = fI * scaleU;

    if(i>0)
    {
      Indexes3D& indexes = geometry.indexes.emplace_back();
      indexes.type = geometry.triangleStrip;
    }

    for(size_t j=0; j<=tilesJ; j++)
    {
      float fJ = float(j)/float(tilesJ);
      float currentJ = fJ * scaleJ;
      float currentV = fJ * scaleV;

      Vertex3D& v = geometry.verts.emplace_back();
      v.normal = normal;
      v.vert = points[0] + (points[1]*currentI) + (points[2]*currentJ);
      v.texture = {currentU, currentV};

      if(i>0)
      {
        Indexes3D& indexes = geometry.indexes.back();
        int idx = int(geometry.verts.size())-1;
        indexes.indexes.push_back(idx);
        indexes.indexes.push_back(idx-(int(tilesJ)+1));
      }
    }
  }

  return geometry;
}

}
