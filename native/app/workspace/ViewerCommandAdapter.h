#pragma once
#include "GraphProject.h"

namespace smartflow {
// No domain imports. An optional scope limits a Use viewer to one component.
void attachViewerCommands(OutputViewer& viewer, GraphProject& project,
                          std::function<QString()> componentScope = {});
}
