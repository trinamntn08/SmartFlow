#pragma once
#include "SceneExtension.h"
#include <QPolygonF>

namespace smartflow::scene3d {
// Small opaque-mesh preview. Camera/selection state belongs only to this widget.
class SceneViewer final : public OutputViewer {
public:
    SceneViewer();
    void present(std::shared_ptr<const tp_data::Collection> output) override;
    QString describe(const tp_data::Collection& output) const override;
    void frameScene();
    size_t objectCount() const;
    QPointF cameraAngles() const { return {yaw, pitch}; }
    int selectedObject() const { return selection; }
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
private:
    std::shared_ptr<const tp_data::Collection> collection;
    const SceneMember* scene = nullptr;
    float yaw = 35, pitch = 25, span = 6;
    glm::vec3 target{0};
    QPointF lastPosition, pressPosition;
    bool dragged = false;
    int selection = -1;
    struct PickFace { QPolygonF polygon; int object; };
    std::vector<PickFace> pickFaces;
};
} // namespace smartflow::scene3d
