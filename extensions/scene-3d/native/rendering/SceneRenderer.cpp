#include "SceneRenderer.h"
#include <tp_math_utils/materials/OpenGLMaterial.h>
#include <QPolygonF>
#include <algorithm>
#include <limits>
#include <cmath>

namespace smartflow::scene3d {
void SceneRenderer::paint(QPainter& painter, QRectF viewport, const CameraController& camera,
                          const SceneMember& scene, int selection) const
{
    const auto project = [&](glm::vec3 v) { return camera.project(v,viewport); };
    painter.setPen(QPen(QColor(48,57,69),1));
    for(int i=-5;i<=5;++i) {
        painter.drawLine(project({float(i),-1,-5}),project({float(i),-1,5}));
        painter.drawLine(project({-5,-1,float(i)}),project({5,-1,float(i)}));
    }
    struct Face { QPolygonF polygon; QColor color; float depth; int object; };
    std::vector<Face> faces;
    int index=0;
    for(const auto& object : scene.objects) {
        glm::vec3 albedo(0.65f);
        object.geometry.material.viewOpenGL([&](const auto& material) { albedo=material.albedo; });
        object.geometry.forEachTriangle([&](const auto& a,const auto& b,const auto& c,int,int,int) {
            auto normal=glm::cross(b.vert-a.vert,c.vert-a.vert);
            if(glm::length(normal)<1e-7f) return;
            normal=glm::normalize(normal);
            if(glm::dot(normal,camera.forward())<=0) return;
            const float light=0.28f+0.72f*std::max(0.0f,glm::dot(normal,glm::normalize(glm::vec3(-1,2,3))));
            const auto color=glm::clamp(albedo*light,glm::vec3(0),glm::vec3(1));
            faces.push_back({QPolygonF({project(a.vert),project(b.vert),project(c.vert)}),
                QColor::fromRgbF(color.r,color.g,color.b),glm::dot((a.vert+b.vert+c.vert)/3.0f-camera.target,camera.forward()),index});
        });
        ++index;
    }
    // Small primitive preview; intersecting surfaces still need a depth-buffered renderer.
    std::stable_sort(faces.begin(),faces.end(),[](const auto& a,const auto& b) { return a.depth<b.depth; });
    for(const auto& face : faces) {
        painter.setBrush(face.color);
        painter.setPen(QPen(face.object==selection ? QColor(255,207,94) : face.color.darker(115),face.object==selection ? 2 : 1));
        painter.drawPolygon(face.polygon);
    }
}
int SceneRenderer::pick(QPointF point, QRectF viewport, const CameraController& camera, const SceneMember& scene) const
{
    const auto ray=camera.ray(point,viewport);
    const glm::dvec3 origin(ray.p0), direction(-camera.forward());
    double nearest=std::numeric_limits<double>::infinity(); int result=-1, index=0;
    for(const auto& object : scene.objects) {
        object.geometry.forEachTriangle([&](const auto& a,const auto& b,const auto& c,int,int,int) {
            const glm::dvec3 edge1=glm::dvec3(b.vert)-glm::dvec3(a.vert), edge2=glm::dvec3(c.vert)-glm::dvec3(a.vert);
            const auto p=glm::cross(direction,edge2); const auto determinant=glm::dot(edge1,p);
            if(determinant<=1e-12) return;
            const auto tvec=origin-glm::dvec3(a.vert);
            const auto u=glm::dot(tvec,p)/determinant;
            const auto q=glm::cross(tvec,edge1); const auto v=glm::dot(direction,q)/determinant;
            if(u<0 || v<0 || u+v>1) return;
            const auto distance=glm::dot(edge2,q)/determinant;
            if(std::isfinite(distance) && distance<nearest) { nearest=distance; result=index; }
        });
        ++index;
    }
    return result;
}
}
