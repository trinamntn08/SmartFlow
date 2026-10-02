#pragma once
#include "GraphComponent.h"

namespace smartflow::project {
// One standalone schema-v1 definition, without workspace, catalog or assets.
// Reading validates structure without requiring installed node packages.
GraphComponent readComponent(const QString& path);
void writeComponent(const QString& path, const GraphComponent& component);
} // namespace smartflow::project
