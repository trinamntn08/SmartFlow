#pragma once
#include "PipelineCanvas.h"
#include "ExecutionController.h"
#include <QtNodes/DataFlowGraphicsScene>
#include <QMainWindow>

class QLabel;
class QVBoxLayout;
class QTreeWidget;
class QPushButton;

namespace smartflow {
class WorkspaceWindow : public QMainWindow {
    Q_OBJECT
public:
    WorkspaceWindow();
    ~WorkspaceWindow() override;
    GraphProject& project() { return document; }
    PipelineCanvas& canvas() { return canvasModel; }
    QtNodes::DataFlowGraphicsScene& scene() { return canvasScene; }
    ExecutionController& execution() { return runner; }
    void selectNode(QtNodes::NodeId id);
private:
    void refreshInspector();
    void refreshResults();
    std::shared_ptr<tp_pipeline::StepDelegateMap> delegates;
    GraphProject document;
    PipelineCanvas canvasModel;
    QtNodes::DataFlowGraphicsScene canvasScene;
    ExecutionController runner;
    QWidget* inspectorBody = nullptr;
    QVBoxLayout* inspectorLayout = nullptr;
    QLabel* outputLabel = nullptr;
    QTreeWidget* results = nullptr;
    QPushButton* cancelButton = nullptr;
    tp_utils::StringID selected;
};
} // namespace smartflow
