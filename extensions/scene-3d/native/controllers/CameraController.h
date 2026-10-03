#pragma once
#include <tp_math_utils/Ray.h>
#include <QJsonObject>
#include <QRectF>
#include <optional>

namespace smartflow::scene3d {
enum class CameraView { Orbit, Front, Right, Top };
class CameraController {
public:
    float yaw = 35, pitch = 25, span = 6;
    glm::vec3 target{0};
    CameraView view = CameraView::Orbit;
    glm::vec3 forward() const;
    glm::vec3 right() const;
    glm::vec3 up() const;
    QPointF project(glm::vec3 point, QRectF viewport) const;
    tp_math_utils::Ray ray(QPointF point, QRectF viewport) const;
    std::optional<glm::vec3> pointOnPlane(QPointF point, QRectF viewport, glm::vec3 origin, glm::vec3 normal) const;
    void orbit(QPointF delta);
    void pan(QPointF delta, QRectF viewport);
    void zoom(float wheelDelta, QPointF cursor, QRectF viewport);
    void frame(glm::vec3 low, glm::vec3 high);
    QJsonObject state() const;
    void restore(const QJsonObject& state);
private:
    float pixels(QRectF viewport) const;
};
}
