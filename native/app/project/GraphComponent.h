#pragma once
#include "ProjectFile.h"
#include <vector>

namespace smartflow::project {
struct ComponentBindings {
    Document outputs = Document::object();
    Document controls = Document::object();
};
struct ComponentInstantiation {
    Document document;
    ComponentBindings bindings;
};

// Registry-independent retained template. Instantiation expands ordinary nodes;
// collapsed component nodes and nested component execution are not implemented.
class GraphComponent {
public:
    explicit GraphComponent(Document definition);
    const Document& definition() const { return retained; }
    // Copy a selection without changing its source graph. Every boundary edge
    // must be represented by an explicit interface endpoint.
    static GraphComponent extract(const Document& source, const std::string& graphId,
        const std::vector<std::string>& nodeIds, const std::string& id, const std::string& title,
        const Document& inputs, const Document& outputs, const Document& controls);
    Document catalog(const Document& source) const;
    ComponentInstantiation instantiate(const Document& source, const std::string& graphId,
        const std::string& instanceId, const Document& controls, const Document& inputSources) const;
private:
    Document retained;
};
} // namespace smartflow::project
