#include "ComponentFile.h"

namespace smartflow::project {
GraphComponent readComponent(const QString& path)
{
    return GraphComponent(jsonFile::read(path));
}

void writeComponent(const QString& path, const GraphComponent& component)
{
    jsonFile::write(path, component.definition());
}
} // namespace smartflow::project
