#pragma once
#include <WorkspaceExtension.h>
#include <tp_data/AbstractMember.h>

namespace smartflow::data {
const tp_utils::StringID& tableType();
struct TableRow { std::string label; double value; };
// Small immutable published result, independent of scene-domain types.
class TableMember final : public tp_data::AbstractMember {
public:
    TableMember() : AbstractMember({}, tableType()) {}
    std::vector<TableRow> rows;
};
void contribute(WorkspaceConfiguration& configuration);
WorkspaceConfiguration dataConfiguration();
} // namespace smartflow::data
