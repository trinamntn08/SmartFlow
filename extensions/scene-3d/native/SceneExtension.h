#pragma once
#include <WorkspaceExtension.h>
#include <tp_data/AbstractMember.h>
#include <tp_math_utils/Geometry3D.h>

namespace smartflow::scene3d {
const tp_utils::StringID& sceneType();
struct SceneObject {
    tp_utils::StringID sourceNode;
    tp_math_utils::Geometry3D geometry;
};
class SceneMember final : public tp_data::AbstractMember {
public:
    SceneMember() : AbstractMember({}, sceneType()) {}
    std::vector<SceneObject> objects;
};
void contribute(WorkspaceConfiguration& configuration);
WorkspaceConfiguration sceneConfiguration();
} // namespace smartflow::scene3d
