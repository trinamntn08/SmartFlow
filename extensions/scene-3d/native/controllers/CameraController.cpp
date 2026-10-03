#include "CameraController.h"
#include <adapted/LegacyNavigation.h>
#include <tp_math_utils/Intersection.h>
#include <tp_math_utils/Plane.h>
#include <QJsonArray>
#include <algorithm>
#include <cmath>

namespace smartflow::scene3d {
glm::vec3 CameraController::forward() const
{
    switch(view) {
    case CameraView::Front: return {0,0,1};
    case CameraView::Right: return {1,0,0};
    case CameraView::Top: return {0,1,0};
    default: {
        const auto y = glm::radians(yaw), p = glm::radians(pitch);
        return {std::sin(y)*std::cos(p), std::sin(p), std::cos(y)*std::cos(p)};
    }}
}
glm::vec3 CameraController::right() const
{
    if(view == CameraView::Top || view == CameraView::Front) return {1,0,0};
    if(view == CameraView::Right) return {0,0,-1};
    const auto y = glm::radians(yaw);
    return {std::cos(y),0,-std::sin(y)};
}
glm::vec3 CameraController::up() const { return glm::cross(forward(), right()); }
float CameraController::pixels(QRectF viewport) const
{ return float(std::max(1.0, std::min(viewport.width(), viewport.height()))) / span; }
QPointF CameraController::project(glm::vec3 point, QRectF viewport) const
{
    point -= target;
    return viewport.center() + QPointF(glm::dot(point,right())*pixels(viewport), -glm::dot(point,up())*pixels(viewport));
}
tp_math_utils::Ray CameraController::ray(QPointF point, QRectF viewport) const
{
    const auto delta = point - viewport.center();
    const auto planePoint = target + right()*float(delta.x()/pixels(viewport)) - up()*float(delta.y()/pixels(viewport));
    // Signed viewing-line distances cover surfaces in front of the focal plane
    // without a distant ray origin losing precision.
    return {planePoint, planePoint-forward()};
}
std::optional<glm::vec3> CameraController::pointOnPlane(QPointF point, QRectF viewport, glm::vec3 origin, glm::vec3 normal) const
{
    glm::vec3 result;
    if(tp_math_utils::rayPlaneIntersection(ray(point,viewport), tp_math_utils::Plane(origin,normal), result)) return result;
    return {};
}
void CameraController::orbit(QPointF delta)
{
    view = CameraView::Orbit;
    yaw = std::remainder(yaw + float(delta.x())*0.5f, 360.0f);
    pitch = std::clamp(pitch + float(delta.y())*0.5f, -85.0f, 85.0f);
}
void CameraController::pan(QPointF delta, QRectF viewport)
{
    const auto offset = legacy::orthographicPan(float(delta.x()),float(delta.y()),float(viewport.width()),float(viewport.height()),span);
    target += -right()*offset.x + up()*offset.y;
    target = glm::clamp(target, glm::vec3(-1000000),glm::vec3(1000000));
}
void CameraController::zoom(float wheelDelta, QPointF cursor, QRectF viewport)
{
    // CADController keeps the cursor's focal-plane point fixed across zoom.
    const auto before = pointOnPlane(cursor,viewport,target,forward());
    span = std::clamp(span*std::exp(std::clamp(-wheelDelta/1000.0f,-1.0f,1.0f)),0.1f,1000000.0f);
    const auto after = pointOnPlane(cursor,viewport,target,forward());
    if(before && after) target += *before-*after;
    target = glm::clamp(target,glm::vec3(-1000000),glm::vec3(1000000));
}
void CameraController::frame(glm::vec3 low, glm::vec3 high)
{ target = (low+high)/2.0f; span = std::clamp(glm::length(high-low)*1.4f,0.1f,1000000.0f); }
QJsonObject CameraController::state() const
{ return {{"yaw",yaw},{"pitch",pitch},{"span",span},{"target",QJsonArray{target.x,target.y,target.z}},{"view",int(view)}}; }
void CameraController::restore(const QJsonObject& state)
{
    const auto number = [&](const char* key, double low, double high, float fallback) {
        const auto v = state.value(key); const auto n = v.toDouble(fallback);
        return v.isDouble() && std::isfinite(n) && n>=low && n<=high ? float(n) : fallback;
    };
    yaw=number("yaw",-1000000,1000000,35); pitch=number("pitch",-85,85,25); span=number("span",0.1,1000000,6);
    target=glm::vec3(0);
    const auto values=state.value("target").toArray();
    if(values.size()==3)
        for(int i=0;i<3;++i)
            if(values[i].isDouble() && std::isfinite(values[i].toDouble()) && std::abs(values[i].toDouble())<=1000000)
                target[i]=float(values[i].toDouble());
    const auto mode=number("view",0,3,0);
    view=CameraView(mode==int(mode) ? int(mode) : 0);
}
}
