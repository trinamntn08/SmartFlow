#include "WorkspaceWindow.h"
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
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QTreeWidget>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QScrollArea>

namespace smartflow {
namespace {
WorkspaceConfiguration numericConfiguration()
{
    WorkspaceConfiguration config;
    config.delegates = numericDelegates();
    config.factory = std::make_shared<tp_data::CollectionFactory>();
    tp_data::createCollectionFactories(*config.factory);
    config.factory->finalize();
    config.nodes = {{"smartflow.numeric.number@1", "Number", "Numeric", "smartflow.numeric", "number", 1},
                    {"smartflow.numeric.add@1", "Add", "Numeric", "smartflow.numeric", "add", 1}};
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
    : delegates(configuration.delegates), document(delegates, configuration.nodes), canvasModel(document),
      canvasScene(canvasModel,document), runner(document, configuration.factory)
{
    setWindowTitle("SmartFlow - Graph workspace");
    resize(1180, 760);
    auto* toolbar = addToolBar("Graph");
    toolbar->setMovable(false);
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
    live->setChecked(true);
    toolbar->addWidget(run);
    toolbar->addWidget(cancelButton);
    toolbar->addWidget(live);
    toolbar->addSeparator();
    auto* library = new QComboBox;
    for(const auto& item : configuration.nodes) library->addItem(item.title, item.type);
    toolbar->addWidget(library);
    auto* add = new QPushButton("Add node");
    toolbar->addWidget(add);

    auto* split = new QSplitter;
    auto* view = new QtNodes::GraphicsView(&canvasScene);
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
    auto* views = new QSplitter(Qt::Vertical);
    views->addWidget(view);
    if(configuration.createViewer) {
        viewer = configuration.createViewer();
        views->addWidget(viewer);
        views->setSizes({300, 430});
        auto* pin = toolbar->addAction("Pin output");
        connect(pin, &QAction::triggered, this, [this] {
            pinned = selected;
            refreshResults();
        });
        pin->setToolTip("Keep the selected node's output in the viewer while editing other nodes");
    }
    split->addWidget(views);
    auto* panel = new QWidget;
    panel->setMinimumWidth(290);
    panel->setMaximumWidth(410);
    inspectorLayout = new QVBoxLayout(panel);
    outputLabel = new QLabel("Select a node to inspect its result.");
    outputLabel->setObjectName("selectedOutput");
    outputLabel->setWordWrap(true);
    results = new QTreeWidget;
    results->setObjectName("executionResults");
    results->setColumnCount(3);
    results->setHeaderLabels({"Node", "State", "Result"});
    results->setRootIsDecorated(false);
    inspectorLayout->addWidget(outputLabel);
    inspectorLayout->addWidget(results, 1);
    auto* hint = new QLabel("Drag ports to connect. Select a node to edit, then Apply.\n\nProject saving is not available in this preview.");
    hint->setWordWrap(true);
    inspectorLayout->addWidget(hint);
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(panel);
    scroll->setMinimumWidth(315);
    split->addWidget(scroll);
    split->setStretchFactor(0, 1);
    setCentralWidget(split);

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
    runner.run();
}

WorkspaceWindow::~WorkspaceWindow()
{
    // Scene teardown emits selection changes after later members (including
    // the runner and selected ID) have been destroyed. Disconnect while all
    // members still exist, before QObject's automatic disconnection occurs.
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
    if(!step) {
        auto* message=new QLabel(selected.isValid() ? "Node unavailable. Saved content is retained; execution is blocked."
                                                   : "Select one node to edit its parameters.");
        message->setWordWrap(true);
        layout->addWidget(message);
        return;
    }
    auto* title = new QLabel(document.title(step->delegateName()));
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
    const auto& result = runner.result();
    if(viewer) {
        std::shared_ptr<const tp_data::Collection> output;
        if(result) {
            const auto found = result->steps.find(pinned.isValid() ? pinned : selected);
            if(found != result->steps.end() && found->second.state == StepState::Succeeded)
                output = found->second.output;
        }
        viewer->present(std::move(output));
    }
    if(!document.diagnostics().empty()) {
        QStringList diagnostics;
        for(const auto& issue : document.diagnostics()) diagnostics << QString::fromStdString(issue);
        outputLabel->setText(diagnostics.join("\n"));
        return;
    }
    if(!result) return;
    if(!result->diagnostics.empty()) {
        QStringList errors;
        for(const auto& error : result->diagnostics) errors << QString::fromStdString(error);
        outputLabel->setText(errors.join("\n"));
    }
    for(auto* step : document.graph().steps()) {
        const auto found = result->steps.find(step->id());
        if(found == result->steps.end()) continue;
        const auto& value = found->second;
        QString description;
        if(viewer && value.output) description = viewer->describe(*value.output);
        if(description.isEmpty()) description = outputText(value);
        const QString state = value.state == StepState::Succeeded ? "Complete" :
                              value.state == StepState::Failed ? "Failed" : "Skipped";
        auto* item = new QTreeWidgetItem(results, {document.title(step->delegateName()), state, description});
        item->setData(0, Qt::UserRole, QString::fromStdString(step->id().toString()));
        item->setToolTip(2, description);
        if(step->id() == selected) outputLabel->setText("Output: " + description);
    }
    for(int column = 0; column < 3; ++column) results->resizeColumnToContents(column);
}
} // namespace smartflow
