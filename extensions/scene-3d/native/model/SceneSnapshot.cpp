#include "SceneSnapshot.h"
#include <cmath>

namespace smartflow::scene3d {
QString scopedObjectId(const tp_utils::StringID& node, const QString& branch, const QString& object)
{
    const auto prefix = QString::fromStdString(node.toString());
    return QString("%1:%2/%3/%4").arg(prefix.size()).arg(prefix, branch, object);
}
void SceneSnapshot::present(std::shared_ptr<const tp_data::Collection> output)
{
    collection = std::move(output);
    value = nullptr;
    if(collection)
        for(const auto& member : collection->members())
            if(const auto* found = dynamic_cast<const SceneMember*>(member.get())) { value = found; break; }
    if(value && selection.isEmpty() && legacySelection >= 0) {
        if(legacySelection < int(value->objects.size())) selection = value->objects[legacySelection].id;
        legacySelection = -1;
    }
}
int SceneSnapshot::selectedIndex() const
{
    if(value && !selection.isEmpty())
        for(int i = 0; i < int(value->objects.size()); ++i)
            if(value->objects[i].id == selection) return i;
    return -1;
}
void SceneSnapshot::select(int index)
{
    selection = value && index >= 0 && index < int(value->objects.size()) ? value->objects[index].id : QString();
    legacySelection = -1;
}
void SceneSnapshot::restoreSelection(const QJsonObject& state)
{
    selection = state.value("selectedObjectId").toString();
    const auto index = state.value("selection");
    legacySelection = selection.isEmpty() && index.isDouble() && index.toDouble() == index.toInt() &&
        index.toInt() >= 0 && index.toInt() < 64 ? index.toInt() : -1;
    if(value && legacySelection >= 0) {
        if(legacySelection < int(value->objects.size())) selection = value->objects[legacySelection].id;
        legacySelection = -1;
    }
}
bool SceneSnapshot::bounds(glm::vec3& low, glm::vec3& high) const
{
    low = glm::vec3(INFINITY); high = glm::vec3(-INFINITY);
    bool found = false;
    if(value)
        for(const auto& object : value->objects)
            for(const auto& vertex : object.geometry.verts) {
                if(!std::isfinite(vertex.vert.x) || !std::isfinite(vertex.vert.y) || !std::isfinite(vertex.vert.z)) continue;
                low = glm::min(low, vertex.vert); high = glm::max(high, vertex.vert); found = true;
            }
    return found;
}
}
