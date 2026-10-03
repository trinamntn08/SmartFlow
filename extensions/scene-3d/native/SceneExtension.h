#pragma once
#include <WorkspaceExtension.h>
#include "model/SceneData.h"

namespace smartflow::scene3d {
void contribute(WorkspaceConfiguration& configuration);
WorkspaceConfiguration sceneConfiguration();
} // namespace smartflow::scene3d
