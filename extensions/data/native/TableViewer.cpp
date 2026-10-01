#include "TableViewer.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QVBoxLayout>
#include <QSignalBlocker>
#include <cmath>

namespace smartflow::data {
namespace {
const TableMember* tableOutput(const tp_data::Collection& output)
{
    for(const auto& member : output.members())
        if(const auto* table=dynamic_cast<const TableMember*>(member.get())) return table;
    return nullptr;
}
}
TableViewer::TableViewer()
{
    setObjectName("tableViewer");
    setMinimumSize(360,270);
    auto* layout=new QVBoxLayout(this);
    caption=new QLabel("No current table output");
    layout->addWidget(caption);
    table=new QTableWidget(0,2);
    table->setObjectName("outputTable");
    table->setHorizontalHeaderLabels({"Label","Value"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(table);
    connect(table,&QTableWidget::itemSelectionChanged,this,[this] {
        const auto rows=table->selectionModel()->selectedRows();
        selectedRow=rows.isEmpty() ? -1 : rows.front().row();
        if(workspaceStateChanged) workspaceStateChanged();
    });
}

void TableViewer::present(std::shared_ptr<const tp_data::Collection> output)
{
    const auto* value=output ? tableOutput(*output) : nullptr;
    QSignalBlocker guard(table);
    table->clearContents();
    table->setRowCount(value ? int(value->rows.size()) : 0);
    caption->setText(value ? describe(*output) : "No current table output");
    if(!value) return; // Keep workspace selection during recomputation.
    for(size_t i=0; i<value->rows.size(); ++i) {
        const auto& row=value->rows[i];
        table->setItem(int(i),0,new QTableWidgetItem(QString::fromStdString(row.label)));
        table->setItem(int(i),1,new QTableWidgetItem(QString::number(row.value,'g',12)));
    }
    if(selectedRow>=0 && selectedRow<table->rowCount()) table->selectRow(selectedRow);
}

QString TableViewer::describe(const tp_data::Collection& output) const
{
    const auto* value=tableOutput(output);
    if(!value) return {};
    if(value->rows.size()==3 && value->rows[0].label=="Count" && value->rows[1].label=="Total" && value->rows[2].label=="Mean")
        return QString("Count %1, total %2, mean %3").arg(value->rows[0].value).arg(value->rows[1].value).arg(value->rows[2].value,0,'g',8);
    return QString("%1 row(s)").arg(value->rows.size());
}

QJsonObject TableViewer::workspaceState() const { return {{"selectedRow",selectedRow}}; }
void TableViewer::restoreWorkspaceState(const QJsonObject& state)
{
    const auto value=state.value("selectedRow");
    const auto row=value.toDouble(-1);
    selectedRow=value.isDouble() && std::isfinite(row) && row>=-1 && row<1000 && std::floor(row)==row ? int(row) : -1;
    QSignalBlocker guard(table);
    table->clearSelection();
    if(selectedRow>=0 && selectedRow<table->rowCount()) table->selectRow(selectedRow);
}
} // namespace smartflow::data
