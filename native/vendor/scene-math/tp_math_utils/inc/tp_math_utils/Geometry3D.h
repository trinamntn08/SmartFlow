#pragma once

#include "tp_math_utils/Material.h"
#include "tp_math_utils/MeshKeyFrame.h"

#include <unordered_map>

namespace tp_math_utils
{

class Plane;

#define TP_TRIANGLES      0x0004 //!< GL_TRIANGLES
#define TP_TRIANGLE_STRIP 0x0005 //!< GL_TRIANGLE_STRIP
#define TP_TRIANGLE_FAN   0x0006 //!< GL_TRIANGLE_FAN

//##################################################################################################
enum class NormalCalculationMode
{
  None,
  CalculateFaceNormals,
  CalculateVertexNormals,
  CalculateAdaptiveNormals
};

//##################################################################################################
std::vector<std::string> normalCalculationModes();

//##################################################################################################
std::string normalCalculationModeToString(NormalCalculationMode mode);

//##################################################################################################
NormalCalculationMode normalCalculationModeFromString(const std::string& mode);

//##################################################################################################
struct TP_MATH_UTILS_EXPORT Vertex3D
{
  glm::vec3 vert{0,0,0};
  glm::vec2 texture{0,0};
  glm::vec3 normal{0,0,1};

  //################################################################################################
  //! Lerp: (1-u)*v0 + u*v1
  static Vertex3D interpolate(float u, const Vertex3D& v0, const Vertex3D& v1);

  //################################################################################################
  static Vertex3D interpolate(float u, float v, const Vertex3D& v0, const Vertex3D& v1, const Vertex3D& v2);

  //################################################################################################
  //! Needed to make the python interface work.
  bool operator==(const Vertex3D& other) const
  {
    return
        vert == other.vert &&
        texture == other.texture &&
        normal == other.normal;
  };

  //################################################################################################
  void saveState(tp_utils::JSON& j) const;

  //################################################################################################
  void loadState(const tp_utils::JSON& j);
};

typedef std::vector<int> Vertex3DIndexList;

//##################################################################################################
struct TP_MATH_UTILS_EXPORT Indexes3D
{
  int type{0};
  Vertex3DIndexList indexes;

  //################################################################################################
  //! Needed to make the python interface work.
  bool operator==(const Indexes3D& other) const
  {
    return
        type == other.type &&
        indexes == other.indexes;
  };

  //################################################################################################
  void saveState(tp_utils::JSON& j) const;

  //################################################################################################
  void loadState(const tp_utils::JSON& j);
};

typedef std::vector<tp_math_utils::MeshKeyFrame> MeshKeyFrameList;
typedef std::vector<Vertex3D> Vertex3DList;
typedef std::vector<Indexes3D> Indexes3DList;

//##################################################################################################
struct TP_MATH_UTILS_EXPORT Geometry3D
{
  std::vector<std::string> comments;
  Vertex3DList verts;
  Indexes3DList indexes;

  int animationIndex{-1};
  int triangleFan  {TP_TRIANGLE_FAN  };
  int triangleStrip{TP_TRIANGLE_STRIP};
  int triangles    {TP_TRIANGLES     };

  Material material;

  //################################################################################################
  void add(const Geometry3D& other);

  //################################################################################################
  void clear();

  //################################################################################################
  static std::string stats(const std::vector<Geometry3D>& geometry);

  //################################################################################################
  static void debugPrintStats(const std::string& message, const std::vector<Geometry3D>& geometry);

  //################################################################################################
  static std::string statsString(size_t vertCount, size_t indexCount, size_t triangleCount, size_t commentBytes);

  //################################################################################################
  void stats(size_t& vertCount, size_t& indexCount, size_t& triangleCount, size_t& commentBytes) const;

  //################################################################################################
  std::string stats() const;

  //################################################################################################
  std::pair<glm::vec3, glm::vec3> getMinMax() const;

  //################################################################################################
  static void getMinMax(const std::vector<tp_math_utils::Geometry3D>& geometry,
                        glm::vec3& min,
                        glm::vec3& max);

  //################################################################################################
  //! Convert strips and fans into triangles.
  void convertToTriangles();

  //################################################################################################
  //! Loop for each triangle
  template<typename Closure>
  void forEachTriangle(Closure&& closure) const
  {
    for(const auto& indexes : indexes)
    {
      auto calcVMax = [&](size_t sub)
      {
        return (sub>indexes.indexes.size())?0:indexes.indexes.size()-sub;
      };

      auto closureI = [&](int i0, int i1, int i2)
      {
        const Vertex3D& v0 = verts.at(i0);
        const Vertex3D& v1 = verts.at(i1);
        const Vertex3D& v2 = verts.at(i2);

        closure(v0,v1,v2, i0,i1,i2);
      };

      if(indexes.type == triangleFan)
      {
        const auto& firstIndex = indexes.indexes.front();

        size_t vMax = calcVMax(1);
        for(size_t v=1; v<vMax; v++)
        {
          int i0 = firstIndex;
          int i1 = indexes.indexes.at(v);
          int i2 = indexes.indexes.at(v+1);

          closureI(i0, i1, i2);
        }
      }
      else if(indexes.type == triangleStrip)
      {
        size_t vMax = calcVMax(2);

        for(size_t v=0; v<vMax; v++)
        {
          int i0 = indexes.indexes.at(v);
          int i1 = indexes.indexes.at(v+1);
          int i2 = indexes.indexes.at(v+2);

          closureI(i0, i1, i2);
        }
      }
      else if(indexes.type == triangles)
      {
        size_t vMax = calcVMax(2);
        for(size_t v=0; v<vMax; v+=3)
        {
          int i0 = indexes.indexes.at(v);
          int i1 = indexes.indexes.at(v+1);
          int i2 = indexes.indexes.at(v+2);

          closureI(i0, i1, i2);
        }
      }
    }
  }

  //################################################################################################
  //! Convert to triangles and duplicate verts. (nVerts = nFaces*3)
  void breakApartTriangles();

  //################################################################################################
  void calculateNormals(NormalCalculationMode mode, float minDot=0.9f);

  //################################################################################################
  void calculateVertexNormals();

  //################################################################################################
  void calculateFaceNormals();

  //################################################################################################
  void calculateAdaptiveNormals(float minDot=0.9f);

  //################################################################################################
  void combineSimilarVerts();

  //################################################################################################
  void transform(const glm::mat4& m);

  //################################################################################################
  void buildTangentVectors(std::vector<glm::vec3>& tangent) const;

  //################################################################################################
  //! Duplicate and reverse geometry to render back faces.
  void addBackFaces();

  //################################################################################################
  void flipUVs();

  //################################################################################################
  static void flipUVs(std::vector<Geometry3D>& geometry);

  //################################################################################################
  tp_utils::StringID getName() const;

  //################################################################################################
  //! Estimate the memory usage of some geometry.
  static size_t sizeInBytes(const std::vector<Geometry3D>& geometry);

  //################################################################################################
  //! Needed to make the python interface work.
  bool operator==(const Geometry3D& other) const
  {
    return
        comments == other.comments &&
        verts == other.verts &&
        indexes == other.indexes &&
        triangleFan == other.triangleFan &&
        triangleStrip == other.triangleStrip &&
        triangles == other.triangles;
  };

  //################################################################################################
  //! This can be used to compare the results of creating a Geometry3D array from different inputs,
  //! for instance OBJ and JSON formats
  static bool printDataToFile(const std::vector<Geometry3D>& geometry, const std::string& filename);

  //################################################################################################
  void saveState(tp_utils::JSON& j) const;

  //################################################################################################
  void loadState(const tp_utils::JSON& j);
};

//##################################################################################################
typedef std::vector<Geometry3D> Geometry3DList;

//##################################################################################################
struct TP_MATH_UTILS_EXPORT Geometry
{
  std::vector<glm::vec2> geometry;
  glm::mat4 transform{1.0f};
  tp_math_utils::Material material;

  //################################################################################################
  void saveState(tp_utils::JSON& j) const;

  //################################################################################################
  void loadState(const tp_utils::JSON& j);
};

struct TP_MATH_UTILS_EXPORT MeshAnimation
{
  MeshKeyFrameList keyFrames;

  //################################################################################################
  void saveState(nlohmann::json& j) const;

  //################################################################################################
  void loadState(const nlohmann::json& j);
};

//##################################################################################################
typedef std::function<void(const std::vector<tp_math_utils::Geometry3D>&)> GeometryCallback;
typedef std::function<void(const GeometryCallback&)> ViewGeometry;
typedef std::function<void(const tp_utils::StringID&, const GeometryCallback&)> FindGeometry;
typedef std::function<ViewGeometry(const tp_utils::StringID&)> GetViewGeometry;
typedef std::function<void(const std::vector<tp_math_utils::Geometry3D>&, const std::vector<tp_math_utils::Material>&)> GeometryMaterialCallback;
typedef std::unordered_map<tp_utils::StringID, tp_utils::StringID> AlternativeMaterials;

}