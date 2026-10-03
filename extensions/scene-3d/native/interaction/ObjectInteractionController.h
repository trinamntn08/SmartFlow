#pragma once
#include "model/SceneData.h"
#include "controllers/CameraController.h"
#include <WorkspaceExtension.h>

namespace smartflow::scene3d {
enum class InteractionMode { Navigate, Move, Rotate, Scale };
// One gesture owns an original snapshot. Preview is disposable; only finish()
// requests a graph command. This mirrors the legacy gizmo/keyframe separation.
class ObjectInteractionController {
public:
    bool begin(const SceneMember& scene,int selection,InteractionMode mode,int axis,
               QPointF cursor,const CameraController& camera,QRectF viewport,const ViewerCommands& commands);
    void update(QPointF cursor,bool snap);
    QString finish(const ViewerCommands& commands);
    void cancel();
    bool active() const { return bool(original); }
    const SceneMember* preview() const { return current.get(); }
    QString message() const { return status; }
private:
    std::shared_ptr<SceneMember> original,current;
    TransformSource source;
    CameraController camera;
    QRectF viewport;
    QPointF start;
    glm::vec3 startPoint{0},axisDirection{1,0,0};
    int axis=0;
    InteractionMode mode=InteractionMode::Navigate;
    ViewerEditRequest request{};
    std::vector<ViewerParameter> bindings;
    QString status;
    bool valid=false;
};
}
