#pragma once
#include <WorkspaceExtension.h>
#include "model/SceneSnapshot.h"
#include "controllers/CameraController.h"
#include "rendering/SceneRenderer.h"
#include "interaction/ObjectInteractionController.h"
class QComboBox;
class QLabel;

namespace smartflow::scene3d {
// Composition only: immutable scene, camera controller and renderer are independent modules.
class SceneViewer final : public OutputViewer {
public:
    SceneViewer();
    void present(std::shared_ptr<const tp_data::Collection> output) override;
    QString describe(const tp_data::Collection& output) const override;
    QString workspaceStateKey() const override { return "smartflow.scene-3d.viewer@1"; }
    QJsonObject workspaceState() const override;
    void restoreWorkspaceState(const QJsonObject& state) override;
    void frameScene();
    void projectChanged() override;
    size_t objectCount() const { return snapshot.scene() ? snapshot.scene()->objects.size() : 0; }
    QPointF cameraAngles() const { return {camera.yaw,camera.pitch}; }
    int selectedObject() const { return snapshot.selectedIndex(); }
protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void hideEvent(QHideEvent*) override;
private:
    QRectF viewport() const;
    void workspaceChanged();
    void refreshObjects();
    void selectObject(int index);
    void cancelGesture();
    const SceneMember* displayedScene() const;
    int pickedAxis(QPointF point) const;
    SceneSnapshot snapshot;
    CameraController camera;
    SceneRenderer renderer;
    ObjectInteractionController interaction;
    QWidget* tools;
    QComboBox* viewChoice;
    QComboBox* modeChoice;
    QComboBox* axisChoice;
    QComboBox* objectChoice;
    QLabel* status;
    QPointF lastPosition, pressPosition;
    bool dragged=false;
};
}
