#pragma once
#include "model/SceneData.h"
#include "controllers/CameraController.h"
#include <QPainter>

namespace smartflow::scene3d {
class SceneRenderer {
public:
    void paint(QPainter& painter, QRectF viewport, const CameraController& camera,
               const SceneMember& scene, int selection) const;
    int pick(QPointF point, QRectF viewport, const CameraController& camera, const SceneMember& scene) const;
};
}
