#pragma once
#include "GraphProject.h"
#include "pipeline/PipelineExecution.h"
#include <QWidget>
#include <functional>
class QTreeWidget;
class QLabel;
namespace smartflow {
class GanttWidget final : public QWidget {
public:
    explicit GanttWidget(QWidget* parent = nullptr);
    void refresh(const GraphProject& project, const std::optional<ExecutionProgressSnapshot>& progress);
    std::function<void(const std::string&)> selected;
private:
    QTreeWidget* tree;
    QLabel* summary;
};
}
