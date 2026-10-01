#pragma once
#include "DataExtension.h"

class QTableWidget;
class QLabel;
namespace smartflow::data {
class TableViewer final : public OutputViewer {
public:
    TableViewer();
    void present(std::shared_ptr<const tp_data::Collection> output) override;
    QString describe(const tp_data::Collection& output) const override;
    QString workspaceStateKey() const override { return "smartflow.data.table-viewer@1"; }
    QJsonObject workspaceState() const override;
    void restoreWorkspaceState(const QJsonObject& state) override;
private:
    QTableWidget* table;
    QLabel* caption;
    int selectedRow=-1;
};
} // namespace smartflow::data
