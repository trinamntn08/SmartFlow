#pragma once
#include "SceneData.h"
#include <tp_data/Collection.h>
#include <QJsonObject>

namespace smartflow::scene3d {
// Owns immutable output lifetime and workspace selection, independent of rendering.
class SceneSnapshot {
public:
    void present(std::shared_ptr<const tp_data::Collection> output);
    const SceneMember* scene() const { return value; }
    int selectedIndex() const;
    void select(int index);
    QString selectedId() const { return selection; }
    void restoreSelection(const QJsonObject& state);
    bool bounds(glm::vec3& low, glm::vec3& high) const;
private:
    std::shared_ptr<const tp_data::Collection> collection;
    const SceneMember* value = nullptr;
    QString selection;
    int legacySelection = -1;
};
}
