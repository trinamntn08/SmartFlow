#pragma once
#include "PipelineCanvas.h"
#include "WorkspaceScene.h"
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
    explicit WorkspaceWindow(WorkspaceConfiguration configuration);
    ~WorkspaceWindow() override;
    GraphProject& project() { return document; }
    PipelineCanvas& canvas() { return canvasModel; }
    WorkspaceScene& scene() { return canvasScene; }
    ExecutionController& execution() { return runner; }
    void selectNode(QtNodes::NodeId id);
private:
    void refreshInspector();
    void refreshResults();
    std::shared_ptr<tp_pipeline::StepDelegateMap> delegates;
    GraphProject document;
    PipelineCanvas canvasModel;
    WorkspaceScene canvasScene;
    ExecutionController runner;
    QWidget* inspectorBody = nullptr;
    QVBoxLayout* inspectorLayout = nullptr;
    QLabel* outputLabel = nullptr;
    QTreeWidget* results = nullptr;
    QPushButton* cancelButton = nullptr;
    OutputViewer* viewer = nullptr;
    tp_utils::StringID pinned;
    tp_utils::StringID selected;
};
} // namespace smartflow
