#pragma once
#include "GraphComponent.h"

namespace smartflow::project {
struct ExpandedComponent {
    Document node;
    ComponentExpansion body;
};
// Disposable executable graph. Retained instance snapshots are never rewritten.
Document expandComponents(const Document& graph, std::vector<ExpandedComponent>& instances,
    std::vector<std::string>& diagnostics);
} // namespace smartflow::project
