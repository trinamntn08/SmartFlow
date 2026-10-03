#include "GanttWidget.h"
#include "WorkspaceWindow.h"
#include "ComponentDialogs.h"
#include "PanelWorkspace.h"
#include "UseWorkspace.h"
#include <tp_qt_pipeline_widgets/parameter_editors/DoubleParameterEditor.h>
#include <tp_data/members/NumberMember.h>
#include <tp_data/Collection.h>
#include <QtNodes/GraphicsView>
#include <QtNodes/internal/NodeGraphicsObject.hpp>
#include <QtNodes/internal/UndoCommands.hpp>
#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QTreeWidget>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenuBar>
#include <QMessageBox>
#include <QTimer>
#include <QScrollBar>
#include <QJsonDocument>
#include <QScopedValueRollback>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <tp_data/AbstractMember.h>
#include <cmath>
#include <set>

namespace smartflow {
namespace {
class WorkspaceGraphView final : public QtNodes::GraphicsView {
public:
    using QtNodes::GraphicsView::GraphicsView;
protected:
    void showEvent(QShowEvent* event) override
    {
        // QtNodes refits on every Show, which would reset saved navigation when
        // a widget moves between regions. The workspace handles initial fitting.
        QGraphicsView::showEvent(event);
    }
};

WorkspaceConfiguration numericConfiguration()
{
    WorkspaceConfiguration config;
    config.delegates = numericDelegates();
    config.factory = std::make_shared<tp_data::CollectionFactory>();
    tp_data::createCollectionFactories(*config.factory);
    config.factory->finalize();
    config.nodes = {{"smartflow.numeric.number@1", "Number", "Numeric", "smartflow.numeric", "number", 1},
                    {"smartflow.numeric.add@1", "Add", "Numeric", "smartflow.numeric", "add", 1}};
    for(const auto& node : config.nodes) config.executionPolicies[node.type.toStdString()]={true,{}};
    config.preset = {{"smartflow.numeric.number@1", {0,0}, {{"value",41}}},
                     {"smartflow.numeric.add@1", {300,0}, {}}};
    config.connections = {{0,0,1,0}};
    return config;
}

QString outputText(const StepResult& result)
{
    if(!result.output) return QString::fromStdString(result.error);
    QStringList values;
    for(const auto& member : result.output->members()) {
        if(const auto* number = dynamic_cast<const tp_data::DoubleMember*>(member.get()))
            values << QString::number(number->data, 'g', 12);
        else values << QString::fromStdString(member->type().toString());
    }
    return values.join(", ");
}
}

WorkspaceWindow::WorkspaceWindow()
    : WorkspaceWindow(numericConfiguration()) {}

WorkspaceWindow::WorkspaceWindow(WorkspaceConfiguration configuration)
    : workspaceConfiguration(configuration), delegates(configuration.delegates), document(delegates, configuration.nodes), canvasModel(document),
      canvasScene(canvasModel,document), runner(document, configuration.factory, configuration.executionPolicies)
{
    setWindowTitle("SmartFlow - Graph workspace");
    resize(1180, 760);
    auto* file = menuBar()->addMenu("&File");
    auto* open = file->addAction("&Open...");
    open->setShortcut(QKeySequence::Open);
    open->setObjectName("openProject");
    connect(open, &QAction::triggered, this, [this] {
        const auto path = QFileDialog::getOpenFileName(this, "Open project", filePath,
                                                      "SmartFlow projects (*.smartflow *.json);;All files (*)");
        if(path.isEmpty() || !confirmSave()) return;
        try { openProject(path); }
        catch(const std::exception& error) { QMessageBox::warning(this, "Open failed", error.what()); }
    });
    auto* save = file->addAction("&Save");
    save->setShortcut(QKeySequence::Save);
    save->setObjectName("saveProject");
    connect(save, &QAction::triggered, this, [this] { saveFromDialog(false); });
    auto* saveAs = file->addAction("Save &As...");
    saveAs->setShortcut(QKeySequence::SaveAs);
    connect(saveAs, &QAction::triggered, this, [this] { saveFromDialog(true); });
    auto* components=menuBar()->addMenu("&Components");
    componentMenu=components;
    auto* createComponent=components->addAction("Create from selection...");
    createComponent->setObjectName("createComponent");
    auto* insertComponent=components->addAction("Component library...");
    insertComponent->setObjectName("insertComponent");
    auto* updateComponent=components->addAction("Update selected instance...");
    updateComponent->setObjectName("updateComponentInstance");
    connect(updateComponent,&QAction::triggered,this,[this] {
        const auto selection=canvasScene.selectedNodes();
        if(selection.size()!=1) return;
        ComponentUpdateDialog dialog(document,canvasModel.projectId(*selection.begin()).toString(),this);
        dialog.exec();
    });
    connect(createComponent,&QAction::triggered,this,[this] {
        std::vector<std::string> selection;
        for(const auto id : canvasScene.selectedNodes()) selection.push_back(canvasModel.projectId(id).toString());
        ComponentAuthorDialog dialog(document,std::move(selection),this);
        dialog.exec();
    });
    connect(insertComponent,&QAction::triggered,this,[this] {
        std::set<std::string> before;
        for(const auto& node : document.selectedGraph()["nodes"]) before.insert(node["id"].get<std::string>());
        ComponentLibraryDialog dialog(document,this);
        if(dialog.exec()!=QDialog::Accepted) return;
        auto* view=findChild<QtNodes::GraphicsView*>("graphCanvas");
        // Place copies beside the existing graph, so repeated insertion does
        // not stack nodes on the same viewport center.
        QRectF occupied;
        for(const auto id : canvasModel.allNodeIds()) if(before.count(canvasModel.projectId(id).toString()))
            occupied=occupied.united(canvasScene.nodeGraphicsObject(id)->sceneBoundingRect());
        const QPointF origin(occupied.right()+80,occupied.top());
        canvasScene.clearSelection();
        int index=0;
        for(const auto& node : document.selectedGraph()["nodes"])
            if(!before.count(node["id"].get<std::string>()))
                for(const auto id : canvasModel.allNodeIds()) if(canvasModel.projectId(id).toString()==node["id"]) {
                    canvasModel.setNodeData(id,QtNodes::NodeRole::Position,origin+QPointF((index%3)*240,(index/3)*180));
                    canvasScene.nodeGraphicsObject(id)->setSelected(true);
                    ++index;
                    break;
                }
        view->fitInView(canvasScene.itemsBoundingRect().adjusted(-30,-30,30,30),Qt::KeepAspectRatio);
        captureWorkspace();
        statusBar()->showMessage("Inserted component.",5000);
    });
    connect(components,&QMenu::aboutToShow,this,[this,createComponent,updateComponent] {
        createComponent->setEnabled(!canvasScene.selectedNodes().empty());
        const auto selection=canvasScene.selectedNodes(); bool instance=false;
        if(selection.size()==1) {
            const auto id=canvasModel.projectId(*selection.begin()).toString();
            for(const auto& node : document.selectedGraph()["nodes"])
                if(node["id"]==id && project::GraphComponent::isInstance(node)) instance=true;
        }
        updateComponent->setEnabled(instance);
    });
    auto* toolbar = addToolBar("Workflow");
    toolbar->setMovable(false);
    workspaceMode=new QComboBox;
    workspaceMode->setObjectName("workspaceMode");
    workspaceMode->addItems({"Build", "Use"});
    toolbar->addWidget(workspaceMode);
    toolbar->addSeparator();
    auto* undo = canvasScene.undoStack().createUndoAction(this, "Undo");
    undo->setShortcut(QKeySequence::Undo);
    auto* redo = canvasScene.undoStack().createRedoAction(this, "Redo");
    redo->setShortcut(QKeySequence::Redo);
    toolbar->addAction(undo);
    toolbar->addAction(redo);
    toolbar->addSeparator();
    auto* run = new QPushButton("Run");
    run->setObjectName("runGraph");
    cancelButton = new QPushButton("Cancel");
    cancelButton->setObjectName("cancelGraph");
    auto* live = new QCheckBox("Live updates");
    live->setObjectName("liveUpdates");
    live->setChecked(true);
    toolbar->addWidget(run);
    toolbar->addWidget(cancelButton);
    toolbar->addWidget(live);
    auto* mode=new QComboBox;
    mode->setObjectName("executionMode");
    mode->addItems({"Sequential","Parallel"});
    mode->setToolTip("Parallel runs independent ready nodes; consumers wait for their inputs.");
    auto* threads=new QSpinBox;
    threads->setObjectName("executionThreads"); threads->setRange(1,64); threads->setValue(1);
    threads->setKeyboardTracking(false);
    threads->setToolTip("Total processing thread budget, including managed work inside nodes. Sequential runs one node at a time.");
    toolbar->addWidget(mode); toolbar->addWidget(new QLabel("Threads")); toolbar->addWidget(threads);
    auto schedule=[this,mode,threads] {
        runner.setScheduling(mode->currentIndex()==0 ? ExecutionMode::Sequential : ExecutionMode::Parallel,size_t(threads->value()));
    };
    connect(mode,&QComboBox::currentIndexChanged,this,[schedule](int) { schedule(); });
    connect(threads,&QSpinBox::valueChanged,this,[schedule](int) { schedule(); });
    connect(&runner,&ExecutionController::updated,this,[this,mode,threads] {
        const QSignalBlocker modeGuard(mode), threadGuard(threads);
        mode->setCurrentIndex(runner.options().mode==ExecutionMode::Sequential ? 0 : 1);
        threads->setValue(int(runner.options().maxThreads));
    });
    toolbar->addSeparator();
    auto* library = new QComboBox;
    library->setObjectName("nodeLibraryTypes");
    for(const auto& item : configuration.nodes) library->addItem(item.title, item.type);
    auto* add = new QPushButton("Add node");
    add->setObjectName("addLibraryNode");
    auto* nodeLibrary=new QWidget;
    auto* libraryLayout=new QVBoxLayout(nodeLibrary);
    auto* libraryHint=new QLabel("Choose an operation to add to the graph.");
    libraryHint->setWordWrap(true); libraryLayout->addWidget(libraryHint);
    libraryLayout->addWidget(library); libraryLayout->addWidget(add); libraryLayout->addStretch();

    panels=new PanelWorkspace(this);
    auto* view = new WorkspaceGraphView(&canvasScene);
    view->setObjectName("graphCanvas");
    // Clipboard serialization is not yet a public project contract. Keep one
    // set of window-wide undo shortcuts, including while the inspector has focus.
    for(auto* action : view->actions()) {
        const auto shortcut = action->shortcut();
        if(shortcut == QKeySequence::Copy || shortcut == QKeySequence::Paste ||
           shortcut == QKeySequence::Undo || shortcut == QKeySequence::Redo ||
           action->text().contains("Duplicate")) {
            action->setEnabled(false);
            view->removeAction(action);
        }
    }
    panels->addPanel("graph","Graph",view);
    panels->addPanel("library","Node library",nodeLibrary);
    if(configuration.createViewer) {
        viewer = configuration.createViewer();
        panels->addPanel("viewer","Result viewer",viewer);
        buildOutputs=addToolBar("Build output");
        buildOutputs->setMovable(false);
        buildOutputs->addWidget(new QLabel("Output:"));
        outputPorts=new QComboBox;
        outputPorts->setObjectName("outputPort");
        buildOutputs->addWidget(outputPorts);
        connect(outputPorts,qOverload<int>(&QComboBox::currentIndexChanged),this,[this] {
            selectedPort=outputPorts->currentText();
            refreshResults(); scheduleWorkspaceCapture();
        });
        auto* pin = buildOutputs->addAction("Pin output");
        pin->setObjectName("pinOutput");
        connect(pin, &QAction::triggered, this, [this] {
            pinned = selected;
            pinnedPort=selectedPort;
            refreshResults();
            captureWorkspace();
        });
        pin->setToolTip("Keep the selected node's output in the viewer while editing other nodes");
    }
    auto* panel = new QWidget;
    inspectorLayout = new QVBoxLayout(panel);
    inspectorLayout->addStretch();
    auto* executionPanel=new QWidget;
    auto* executionLayout=new QVBoxLayout(executionPanel);
    executionProgress=new QProgressBar;
    executionProgress->setObjectName("executionProgress");
    executionProgress->setRange(0,1); executionProgress->setValue(0);
    executionProgress->setFormat("No active run");
    executionLayout->addWidget(executionProgress);
    outputLabel = new QLabel("Select a node to inspect its result.");
    outputLabel->setObjectName("selectedOutput");
    outputLabel->setWordWrap(true);
    results = new QTreeWidget;
    results->setObjectName("executionResults");
    results->setColumnCount(4);
    results->setHeaderLabels({"Node", "State", "Result", "Run ms"});
    results->setRootIsDecorated(false);
    executionLayout->addWidget(outputLabel);
    executionLayout->addWidget(results, 1);
    auto* hint = new QLabel("Drag ports to connect. Select a node to edit, then Apply.\n\nSave preserves graph content, canvas layout and viewer settings.");
    hint->setWordWrap(true);
    executionLayout->addWidget(hint);
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(panel);
    panels->addPanel("inspector","Inspector",scroll);
    panels->addPanel("results","Execution results",executionPanel);
    gantt=new GanttWidget;
    gantt->selected=[this](const std::string& id) {
        for(const auto canvasId:canvasModel.allNodeIds())
            if(canvasModel.projectId(canvasId).toString()==id) { selectNode(canvasId); break; }
    };
    panels->addPanel("gantt","Process Gantt",gantt);
    panels->resetLayout();
    workspaces=new QStackedWidget;
    workspaces->addWidget(panels);
    toolWorkspace=new UseWorkspace(document,runner,configuration.createViewer);
    workspaces->addWidget(toolWorkspace);
    setCentralWidget(workspaces);
    layoutMenu=menuBar()->addMenu("&Layout");
    auto* resetLayout=layoutMenu->addAction("Reset widget layout");
    resetLayout->setObjectName("resetWidgetLayout");
    connect(resetLayout,&QAction::triggered,this,[this] {
        panelStateSupported=true; panels->resetLayout(); scheduleWorkspaceCapture();
    });
    panels->changed=[this] { scheduleWorkspaceCapture(); };
    toolWorkspace->changed=[this] { scheduleWorkspaceCapture(); };
    connect(workspaceMode,qOverload<int>(&QComboBox::currentIndexChanged),this,[this](int index) {
        setUseMode(index==1);
    });

    connect(run, &QPushButton::clicked, &runner, &ExecutionController::run);
    connect(cancelButton, &QPushButton::clicked, &runner, &ExecutionController::cancel);
    connect(live, &QCheckBox::toggled, &runner, &ExecutionController::setLive);
    connect(add, &QPushButton::clicked, this, [this, library, view] {
        canvasScene.createNode(library->currentData().toString(),
                               view->mapToScene(view->viewport()->rect().center()));
    });
    connect(&canvasScene, &QGraphicsScene::selectionChanged, this, [this] {
        const auto nodes = canvasScene.selectedNodes();
        selected = nodes.size() == 1 ? canvasModel.projectId(nodes.front()) : tp_utils::StringID();
        refreshInspector();
        refreshResults();
        scheduleWorkspaceCapture();
    });
    connect(&document, &GraphProject::changed, this, [this] { refreshInspector(); refreshResults(); });
    connect(&runner, &ExecutionController::updated, this, &WorkspaceWindow::refreshResults);
    connect(results, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item) {
        const auto id = item->data(0, Qt::UserRole).toString().toStdString();
        for(const auto canvasId : canvasModel.allNodeIds())
            if(canvasModel.projectId(canvasId).toString() == id) { selectNode(canvasId); break; }
    });

    std::vector<QtNodes::NodeId> initialNodes;
    for(const auto& preset : configuration.preset) {
        const auto id = canvasModel.addNode(preset.type);
        initialNodes.push_back(id);
        canvasModel.setNodeData(id, QtNodes::NodeRole::Position, preset.position);
        for(const auto& [name, value] : preset.parameters) {
            auto parameter = document.step(canvasModel.projectId(id))->parameter(name);
            parameter.value = value;
            document.setParameter(canvasModel.projectId(id), parameter);
        }
    }
    for(const auto& edge : configuration.connections)
        canvasModel.addConnection({initialNodes.at(edge.source), QtNodes::PortIndex(edge.output),
                                   initialNodes.at(edge.target), QtNodes::PortIndex(edge.input)});
    if(!initialNodes.empty()) {
        pinned = canvasModel.projectId(initialNodes.back());
        selectNode(initialNodes.front());
    }
    canvasScene.undoStack().clear();
    restoringWorkspace = false;
    captureWorkspace();
    savedDocument = document.retained();
    connect(&canvasModel, &QtNodes::AbstractGraphModel::nodePositionUpdated, this, [this] { scheduleWorkspaceCapture(); });
    connect(view, &QtNodes::GraphicsView::scaleChanged, this, [this] { scheduleWorkspaceCapture(); });
    view->viewport()->installEventFilter(this);
    for(auto* bar : {view->horizontalScrollBar(),view->verticalScrollBar()})
        connect(bar, &QScrollBar::actionTriggered, this, [this] { scheduleWorkspaceCapture(); });
    if(viewer) viewer->workspaceStateChanged = [this] { captureWorkspace(); };
    connect(&document, &GraphProject::changed, this, &WorkspaceWindow::refreshFileState);
    connect(&document.commands(), &project::DocumentHistory::workspaceChanged,
            this, &WorkspaceWindow::refreshFileState);
    refreshFileState();
    runner.run();
}

namespace {
bool finiteNumber(const project::Document& value, double low, double high)
{
    if(!value.is_number()) return false;
    const auto n=value.get<double>();
    return std::isfinite(n) && n>=low && n<=high;
}
}

void WorkspaceWindow::scheduleWorkspaceCapture()
{
    if(restoringWorkspace || capturePending) return;
    capturePending=true;
    const auto generation=workspaceGeneration;
    QTimer::singleShot(0,this,[this,generation] {
        if(generation!=workspaceGeneration) return;
        capturePending=false; captureWorkspace();
    });
}

bool WorkspaceWindow::eventFilter(QObject* watched, QEvent* event)
{
    if(event->type()==QEvent::MouseButtonRelease || event->type()==QEvent::Wheel)
        scheduleWorkspaceCapture();
    return QMainWindow::eventFilter(watched,event);
}

void WorkspaceWindow::captureWorkspace()
{
    if(restoringWorkspace) return;
    auto useState=document.retained()["workspace"].value("smartflow.native-use@1",project::Document::object());
    const auto graph=document.selectedGraph()["id"].get<std::string>();
    if(useState.is_object()) {
        auto& tool=useState[graph];
        if(tool.is_null()) tool=project::Document::object();
        if(tool.is_object()) {
            const auto fresh=toolWorkspace->saveState();
            if(fresh.is_object()) tool.update(fresh);
            if(modeStateSupported) tool["mode"]=usingTool ? "use" : "build";
        }
        document.commands().setWorkspaceField("smartflow.native-use@1",useState);
    }
    const auto& workspace=document.retained()["workspace"];
    auto state=workspace.value("smartflow.native-editor@1",project::Document::object());
    // Unrecognized future shapes are left untouched.
    if(!state.is_object()) return;
    if(state.contains(graph) && !state[graph].is_object()) return;
    auto& current=state[graph];
    if(current.is_null()) current=project::Document::object();
    auto positions=current.value("positions",project::Document::object());
    if(positions.is_object()) {
        for(const auto id : canvasModel.allNodeIds()) {
            const auto pos=canvasModel.nodeData(id,QtNodes::NodeRole::Position).value<QPointF>();
            auto& entry=positions[canvasModel.projectId(id).toString()];
            if(entry.is_null()) entry=project::Document::object();
            if(entry.is_object()) { entry["x"]=pos.x(); entry["y"]=pos.y(); }
        }
        current["positions"]=positions;
    }
    current["selection"]=project::Document::array();
    for(const auto id : canvasScene.selectedNodes()) current["selection"].push_back(canvasModel.projectId(id).toString());
    current["pinned"]=pinned.toString();
    current["pinnedPort"]=pinnedPort.toStdString();
    current["selectedPort"]=selectedPort.toStdString();
    if(panelStateSupported && !usingTool) current["panels"]=panels->saveLayout();
    auto* view=findChild<QtNodes::GraphicsView*>("graphCanvas");
    const auto center=view->mapToScene(view->viewport()->rect().center());
    auto navigation=current.value("navigation",project::Document::object());
    // The hidden Build viewport has different geometry. Retain its last visible
    // navigation while the user is operating the tool.
    if(navigation.is_object() && !usingTool) {
        navigation["scale"]=view->transform().m11();
        navigation["x"]=center.x(); navigation["y"]=center.y();
        current["navigation"]=navigation;
    }
    if(viewer && !viewer->workspaceStateKey().isEmpty()) {
        auto& viewers=current["viewers"];
        if(viewers.is_null()) viewers=project::Document::object();
        if(viewers.is_object()) {
            auto& entry=viewers[viewer->workspaceStateKey().toStdString()];
            if(entry.is_null()) entry=project::Document::object();
            if(entry.is_object()) {
                const auto fresh=project::Document::parse(QJsonDocument(viewer->workspaceState()).toJson().toStdString());
                entry.update(fresh);
            }
        }
    }
    document.commands().setWorkspaceField("smartflow.native-editor@1",state);
}

void WorkspaceWindow::restoreWorkspace()
{
    const auto& workspace=document.retained()["workspace"];
    auto state=workspace.value("smartflow.native-editor@1",project::Document::object());
    const auto graph=document.selectedGraph()["id"].get<std::string>();
    const auto current=state.is_object() ? state.value(graph,project::Document::object()) : project::Document::object();
    auto* view=findChild<QtNodes::GraphicsView*>("graphCanvas");
    panels->resetLayout(); panelStateSupported=true;
    if(current.is_object() && current.contains("panels"))
        panelStateSupported=panels->restoreLayout(current["panels"]);
    if(viewer) viewer->restoreWorkspaceState({});
    bool navigationRestored=false;
    if(current.is_object()) {
        const auto positions=current.value("positions",project::Document::object());
        for(const auto id : canvasModel.allNodeIds()) {
            const auto stable=canvasModel.projectId(id).toString();
            if(positions.is_object() && positions.contains(stable)) {
                const auto& pos=positions[stable];
                if(pos.is_object() && pos.contains("x") && pos.contains("y") &&
                   finiteNumber(pos["x"],-1000000,1000000) && finiteNumber(pos["y"],-1000000,1000000))
                    canvasModel.setNodeData(id,QtNodes::NodeRole::Position,QPointF(pos["x"].get<double>(),pos["y"].get<double>()));
            }
            if(current.contains("selection") && current["selection"].is_array())
                for(const auto& selectedId : current["selection"])
                    if(selectedId==stable) canvasScene.nodeGraphicsObject(id)->setSelected(true);
        }
        if(current.contains("pinned") && current["pinned"].is_string()) pinned=current["pinned"].get<std::string>();
        if(current.contains("pinnedPort") && current["pinnedPort"].is_string()) pinnedPort=QString::fromStdString(current["pinnedPort"]);
        if(current.contains("selectedPort") && current["selectedPort"].is_string()) selectedPort=QString::fromStdString(current["selectedPort"]);
        const auto nav=current.value("navigation",project::Document::object());
        if(nav.is_object() && nav.contains("scale") && nav.contains("x") && nav.contains("y") &&
           finiteNumber(nav["scale"],0.01,2) && finiteNumber(nav["x"],-1000000,1000000) && finiteNumber(nav["y"],-1000000,1000000)) {
            view->resetTransform(); view->scale(nav["scale"].get<double>(),nav["scale"].get<double>());
            view->centerOn(nav["x"].get<double>(),nav["y"].get<double>());
            navigationRestored=true;
        }
        const auto viewers=current.value("viewers",project::Document::object());
        if(viewer && viewers.is_object()) {
            const auto data=viewers.value(viewer->workspaceStateKey().toStdString(),project::Document::object());
            if(data.is_object()) viewer->restoreWorkspaceState(QJsonDocument::fromJson(QByteArray::fromStdString(data.dump())).object());
        }
    }
    if(viewer && (!current.is_object() || !current.contains("pinned"))) {
        const auto& graph=document.selectedGraph();
        for(auto node=graph["nodes"].rbegin(); node!=graph["nodes"].rend(); ++node) {
            const auto id=(*node)["id"].get<std::string>();
            const bool hasConsumer=std::any_of(graph["connections"].begin(),graph["connections"].end(),
                [&](const auto& edge) { return edge["source"]["nodeId"]==id; });
            if(!hasConsumer && document.step(id)) { pinned=id; break; }
        }
    }
    if(!navigationRestored) {
        view->resetTransform();
        if(!canvasModel.allNodeIds().empty()) view->fitInView(canvasScene.itemsBoundingRect().adjusted(-30,-30,30,30),Qt::KeepAspectRatio);
        else view->centerOn(0,0);
    }
    navigationRestoredOnLoad=navigationRestored;
    auto useState=workspace.value("smartflow.native-use@1",project::Document::object());
    const auto tool=useState.is_object() ? useState.value(graph,project::Document::object()) : project::Document::object();
    toolWorkspace->restoreState(tool);
    const auto mode=tool.is_object() ? tool.value("mode",project::Document("build")) : project::Document();
    setUseMode(mode=="use");
    modeStateSupported=!tool.is_object() || !tool.contains("mode") || mode=="build" || mode=="use";
}

void WorkspaceWindow::setUseMode(bool enabled)
{
    if(usingTool==enabled) return;
    // Flush Build navigation before its widgets are hidden. The graph layout
    // and selection stay alive and are never replaced by the tool view.
    captureWorkspace();
    usingTool=enabled;
    modeStateSupported=true;
    const QSignalBlocker guard(workspaceMode);
    workspaceMode->setCurrentIndex(enabled ? 1 : 0);
    workspaces->setCurrentWidget(enabled ? static_cast<QWidget*>(toolWorkspace) : static_cast<QWidget*>(panels));
    componentMenu->setEnabled(!enabled);
    layoutMenu->setEnabled(!enabled);
    if(buildOutputs) buildOutputs->setVisible(!enabled);
    captureWorkspace();
}

void WorkspaceWindow::refreshFileState()
{
    setWindowTitle((filePath.isEmpty() ? QString("Untitled") : QFileInfo(filePath).fileName()) + "[*] - SmartFlow");
    setWindowModified(projectDirty());
}

void WorkspaceWindow::saveProject(const QString& path)
{
    captureWorkspace();
    project::write(path, document.retained());
    filePath = QFileInfo(path).absoluteFilePath();
    savedDocument = document.retained();
    refreshFileState();
}

void WorkspaceWindow::openProject(const QString& path)
{
    loadDocument(project::read(path));
    filePath = QFileInfo(path).absoluteFilePath();
    refreshFileState();
}

void WorkspaceWindow::loadDocument(project::Document source)
{
    const auto& graphs = source["project"]["graphs"];
    if(graphs.empty()) throw project::FileError("This project has no graph to open.");
    const auto graph = graphs.front()["id"].get<std::string>();
    QScopedValueRollback<bool> guard(restoringWorkspace,true);
    ++workspaceGeneration;
    capturePending=false;
    document.commands().replace(std::move(source), graph);
    selected = {};
    pinned = {};
    selectedPort.clear(); pinnedPort.clear();
    canvasModel.resetLayout();
    restoreWorkspace();
    filePath.clear();
    savedDocument = document.retained();
    refreshInspector();
    refreshResults();
    refreshFileState();
}

bool WorkspaceWindow::saveFromDialog(bool saveAs)
{
    auto path = filePath;
    if(saveAs || path.isEmpty())
        path = QFileDialog::getSaveFileName(this, "Save project", path,
                                          "SmartFlow projects (*.smartflow)");
    if(path.isEmpty()) return false;
    try { saveProject(path); return true; }
    catch(const std::exception& error) { QMessageBox::warning(this, "Save failed", error.what()); return false; }
}

bool WorkspaceWindow::confirmSave()
{
    // An untouched loaded workspace may contain more precise navigation than
    // Qt's pixel-rounded scrollbars. Only flush actual pending UI changes.
    if(capturePending) captureWorkspace();
    if(!projectDirty()) return true;
    const auto answer = QMessageBox::warning(this, "Unsaved project", "Save changes to this project?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    if(answer == QMessageBox::Save) return saveFromDialog(false);
    return answer == QMessageBox::Discard;
}

void WorkspaceWindow::showEvent(QShowEvent* event)
{
    const bool clean=!projectDirty();
    QMainWindow::showEvent(event);
    if(firstShow) {
        firstShow=false;
        if(!navigationRestoredOnLoad) {
            auto* view=findChild<QtNodes::GraphicsView*>("graphCanvas");
            view->resetTransform();
            if(!canvasModel.allNodeIds().empty()) view->fitInView(canvasScene.itemsBoundingRect().adjusted(-30,-30,30,30),Qt::KeepAspectRatio);
            else view->centerOn(0,0);
        }
        if(filePath.isEmpty() && clean) {
            captureWorkspace();
            savedDocument=document.retained();
            refreshFileState();
        }
    }
}

void WorkspaceWindow::closeEvent(QCloseEvent* event)
{
    if(confirmSave()) event->accept();
    else event->ignore();
}

WorkspaceWindow::~WorkspaceWindow()
{
    panels->changed={};
    toolWorkspace->changed={};
    // Scene teardown emits selection changes after later members (including
    // the runner and selected ID) have been destroyed. Disconnect while all
    // members still exist, before QObject's automatic disconnection occurs.
    if(viewer) viewer->workspaceStateChanged = {};
    disconnect(&canvasScene, nullptr, this, nullptr);
    disconnect(&document, nullptr, this, nullptr);
    disconnect(&runner, nullptr, this, nullptr);
}

void WorkspaceWindow::selectNode(QtNodes::NodeId id)
{
    canvasScene.clearSelection();
    if(auto* item = canvasScene.nodeGraphicsObject(id)) item->setSelected(true);
}

void WorkspaceWindow::refreshInspector()
{
    if(inspectorBody) {
        inspectorLayout->removeWidget(inspectorBody);
        inspectorBody->hide();
        inspectorBody->deleteLater();
    }
    inspectorBody = new QWidget;
    inspectorLayout->insertWidget(0, inspectorBody);
    auto* layout = new QVBoxLayout(inspectorBody);
    layout->setContentsMargins(0, 0, 0, 12);
    auto* step = document.step(selected);
    if(outputPorts) {
        QSignalBlocker guard(outputPorts);
        outputPorts->clear();
        if(step) for(const auto& port : step->outputMapping())
            outputPorts->addItem(QString::fromStdString(port.portName.toString()));
        auto index=outputPorts->findText(selectedPort);
        outputPorts->setCurrentIndex(index>=0 ? index : (outputPorts->count() ? 0 : -1));
        selectedPort=outputPorts->currentText();
        outputPorts->setEnabled(outputPorts->count()>1);
    }
    if(!step) {
        auto* message=new QLabel(selected.isValid() ? "Node unavailable. Saved content is retained; execution is blocked."
                                                   : "Select one node to edit its parameters.");
        message->setWordWrap(true);
        layout->addWidget(message);
        return;
    }
    auto* title = new QLabel(document.title(step->delegateName()));
    title->setTextFormat(Qt::PlainText);
    auto font = title->font();
    font.setBold(true);
    font.setPointSize(15);
    title->setFont(font);
    layout->addWidget(title);
    for(const auto& entry : step->parameters()) {
        const auto& parameter = entry.second;
        layout->addWidget(new QLabel(QString::fromStdString(parameter.name.toString())));
        if(parameter.type != tp_pipeline::doubleSID()) {
            layout->addWidget(new QLabel("Editor unavailable; parameter retained."));
            continue;
        }
        auto* editor = new tp_qt_pipeline_widgets::DoubleParameterEditor(parameter);
        auto* spin = editor->findChild<QDoubleSpinBox*>();
        spin->setObjectName("parameter_" + QString::fromStdString(parameter.name.toString()));
        spin->setKeyboardTracking(false);
        spin->setSingleStep(tpGetVariantValue<double>(parameter.step, 1.0));
        auto* apply = new QPushButton("Apply");
        apply->setObjectName("applyParameter");
        layout->addWidget(editor);
        layout->addWidget(apply);
        const auto id = selected;
        connect(apply, &QPushButton::clicked, this, [this, editor, id, parameter] {
            auto next = parameter;
            editor->updateParameter(next);
            auto* current = document.step(id);
            if(current && current->parameter(next.name).value != next.value)
                document.setParameter(id, next);
        });
    }
}

void WorkspaceWindow::refreshResults()
{
    statusBar()->showMessage(runner.status());
    cancelButton->setEnabled(runner.busy());
    results->clear();
    outputLabel->setText("Output: " + runner.status());
    const auto& progress=runner.progress();
    gantt->refresh(document,progress);
    canvasModel.showExecutionTimes(progress);
    auto runTime=[&](const tp_utils::StringID& id) {
        if(progress) {
            const auto found=progress->nodes.find(id);
            if(found!=progress->nodes.end() && found->second.startedMs)
                return QString::number(found->second.endedMs.value_or(progress->elapsedMs)-*found->second.startedMs,'f',3);
        }
        return QString("-");
    };
    executionProgress->setRange(0,progress ? int(std::max(size_t(1),progress->total)) : 1);
    executionProgress->setValue(progress ? int(progress->completed) : 0);
    executionProgress->setFormat(progress ? QString("%1/%2 steps finished").arg(qulonglong(progress->completed)).arg(qulonglong(progress->total)) : runner.status());
    const auto& result = runner.result();
    if(viewer) {
        std::shared_ptr<const tp_data::Collection> output;
        if(result) {
            const auto found = result->steps.find(pinned.isValid() ? pinned : selected);
            if(found != result->steps.end() && found->second.state == StepState::Succeeded) {
                output = found->second.output;
                const auto* step=document.step(pinned.isValid() ? pinned : selected);
                const auto port=pinned.isValid() ? pinnedPort : selectedPort;
                if(step && !port.isEmpty() && output) {
                    for(const auto& mapping : step->outputMapping()) if(mapping.portName.toString()==port.toStdString()) {
                        auto single=std::make_shared<tp_data::Collection>();
                        if(const auto& member=output->member(mapping.dataName)) single->addMember(member);
                        output=std::move(single);
                        break;
                    }
                }
            }
        }
        viewer->present(std::move(output));
    }
    if(!document.diagnostics().empty()) {
        QStringList diagnostics;
        for(const auto& issue : document.diagnostics()) diagnostics << QString::fromStdString(issue);
        outputLabel->setText(diagnostics.join("\n"));
        return;
    }
    if(!result) {
        if(progress) for(const auto& node : document.selectedGraph()["nodes"]) {
            const auto id=tp_utils::StringID(node["id"].get<std::string>());
            const auto found=progress->nodes.find(id);
            if(found==progress->nodes.end()) continue;
            const auto& value=found->second;
            QString state;
            switch(value.state) {
            case StepState::Waiting: state="Waiting"; break;
            case StepState::Ready: state="Ready"; break;
            case StepState::Running: state="Running"; break;
            case StepState::Succeeded: state="Complete"; break;
            case StepState::Failed: state="Failed"; break;
            case StepState::Skipped: state="Skipped"; break;
            case StepState::Cancelled: state="Cancelled"; break;
            }
            QString description=QString::fromStdString(value.error);
            if(value.state==StepState::Running && value.fraction)
                description=QString("%1%").arg(int(*value.fraction*100));
            const auto* step=document.step(id);
            auto* item=new QTreeWidgetItem(results,{step ? document.title(step->delegateName()) : QString::fromStdString(id.toString()),state,description,runTime(id)});
            item->setData(0,Qt::UserRole,QString::fromStdString(id.toString()));
        }
        return;
    }
    if(!result->diagnostics.empty()) {
        QStringList errors;
        for(const auto& error : result->diagnostics) errors << QString::fromStdString(error);
        outputLabel->setText(errors.join("\n"));
    }
    for(const auto& node : document.selectedGraph()["nodes"]) {
        auto* step=document.step(node["id"].get<std::string>());
        if(!step) continue;
        const auto found = result->steps.find(step->id());
        if(found == result->steps.end()) continue;
        const auto& value = found->second;
        QString description;
        if(viewer && value.output) description = viewer->describe(*value.output);
        if(description.isEmpty()) description = outputText(value);
        const QString state = value.state == StepState::Succeeded ? "Complete" :
                              value.state == StepState::Failed ? "Failed" : "Skipped";
        auto* item = new QTreeWidgetItem(results, {document.title(step->delegateName()), state, description,runTime(step->id())});
        item->setData(0, Qt::UserRole, QString::fromStdString(step->id().toString()));
        item->setToolTip(2, description);
        if(step->id() == selected) outputLabel->setText("Output: " + description);
    }
    for(int column = 0; column < 4; ++column) results->resizeColumnToContents(column);
}
} // namespace smartflow
