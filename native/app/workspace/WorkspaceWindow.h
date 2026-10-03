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
class QProgressBar;
class QStackedWidget;
class QToolBar;
class QMenu;

namespace smartflow {
class PanelWorkspace;
class GanttWidget;
class UseWorkspace;
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
    void loadDocument(project::Document source);
    const WorkspaceConfiguration& configuration() const { return workspaceConfiguration; }
    void saveProject(const QString& path);
    void setUseMode(bool enabled);
    bool useMode() const { return usingTool; }
    QString projectPath() const { return filePath; }
    bool projectDirty() const { return document.retained() != savedDocument; }
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    bool firstShow = true;
    void closeEvent(QCloseEvent* event) override;
private:
    WorkspaceConfiguration workspaceConfiguration;
    PanelWorkspace* panels = nullptr;
    QStackedWidget* workspaces = nullptr;
    UseWorkspace* toolWorkspace = nullptr;
    QComboBox* workspaceMode = nullptr;
    QToolBar* buildOutputs = nullptr;
    QMenu* componentMenu = nullptr;
    QMenu* layoutMenu = nullptr;
    bool usingTool = false;
    bool modeStateSupported = true;
    bool panelStateSupported = true;
    bool navigationRestoredOnLoad = false;
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
    GanttWidget* gantt = nullptr;
    QProgressBar* executionProgress = nullptr;
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
