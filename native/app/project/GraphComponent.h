#pragma once
#include "ProjectFile.h"

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
    ComponentInstantiation instantiate(const Document& source, const std::string& graphId,
        const std::string& instanceId, const Document& controls, const Document& inputSources) const;
private:
    Document retained;
};
} // namespace smartflow::project
