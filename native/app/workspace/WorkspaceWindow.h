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
class QComboBox;

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
    // Dialog-free operations throw on failure and preserve the active file state.
    void openProject(const QString& path);
    void saveProject(const QString& path);
    QString projectPath() const { return filePath; }
    bool projectDirty() const { return document.retained() != savedDocument; }
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    bool firstShow = true;
    void closeEvent(QCloseEvent* event) override;
private:
    void captureWorkspace();
    void scheduleWorkspaceCapture();
    void restoreWorkspace();
    bool restoringWorkspace = true;
    bool capturePending = false;
    quint64 workspaceGeneration = 0;
    bool confirmSave();
    bool saveFromDialog(bool saveAs);
    void refreshFileState();
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
    QComboBox* outputPorts = nullptr;
    QString selectedPort;
    QString pinnedPort;
    OutputViewer* viewer = nullptr;
    tp_utils::StringID pinned;
    tp_utils::StringID selected;
    QString filePath;
    project::Document savedDocument;
};
} // namespace smartflow
