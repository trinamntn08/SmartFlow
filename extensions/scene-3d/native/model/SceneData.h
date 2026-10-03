#pragma once
#include <tp_data/AbstractMember.h>
#include <tp_math_utils/Geometry3D.h>
#include <tp_math_utils/MeshKeyFrame.h>
#include <QString>
#include <optional>

namespace smartflow::scene3d {
const tp_utils::StringID& sceneType();
// Transient execution provenance; never persisted as authored scene state.
struct TransformSource {
    tp_utils::StringID node;
    tp_math_utils::MeshKeyFrame parameters;
};
struct SceneObject {
    tp_utils::StringID sourceNode;
    tp_math_utils::Geometry3D geometry;
    QString id;
    std::optional<TransformSource> transform;
};
class SceneMember final : public tp_data::AbstractMember {
public:
    SceneMember() : AbstractMember({}, sceneType()) {}
    std::vector<SceneObject> objects;
};
QString scopedObjectId(const tp_utils::StringID& node, const QString& branch, const QString& object);
}
