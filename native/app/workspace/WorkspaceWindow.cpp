#include "WorkspaceWindow.h"
#include "ProjectCommands.h"
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

namespace smartflow {
namespace {
auto dataFactory()
{
    auto factory = std::make_shared<tp_data::CollectionFactory>();
    tp_data::createCollectionFactories(*factory);
    factory->finalize();
    return factory;
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
    : delegates(numericDelegates()), document(delegates), canvasModel(document),
      canvasScene(canvasModel), runner(document, dataFactory())
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
    library->addItem("Number", "smartflow.numeric.number@1");
    library->addItem("Add", "smartflow.numeric.add@1");
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
    split->addWidget(view);
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
    auto* hint = new QLabel("Drag ports to connect. Right-click the canvas to add nodes.\n\n3D viewing and project saving are not available in this preview.");
    hint->setWordWrap(true);
    inspectorLayout->addWidget(hint);
    split->addWidget(panel);
    split->setStretchFactor(0, 1);
    setCentralWidget(split);

    connect(run, &QPushButton::clicked, &runner, &ExecutionController::run);
    connect(cancelButton, &QPushButton::clicked, &runner, &ExecutionController::cancel);
    connect(live, &QCheckBox::toggled, &runner, &ExecutionController::setLive);
    connect(add, &QPushButton::clicked, this, [this, library, view] {
        auto* command = new QtNodes::CreateCommand(&canvasScene, library->currentData().toString(),
                                                  view->mapToScene(view->viewport()->rect().center()));
        canvasScene.undoStack().push(command);
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

    const auto source = canvasModel.addNode("smartflow.numeric.number@1");
    const auto target = canvasModel.addNode("smartflow.numeric.add@1");
    auto initial = document.step(canvasModel.projectId(source))->parameter("value");
    initial.value = 41.0;
    document.setParameter(canvasModel.projectId(source), initial);
    canvasModel.setNodeData(source, QtNodes::NodeRole::Position, QPointF(0, 0));
    canvasModel.setNodeData(target, QtNodes::NodeRole::Position, QPointF(300, 0));
    canvasModel.addConnection({source, 0, target, 0});
    selectNode(source);
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
    if(!step) { layout->addWidget(new QLabel("Select one node to edit its parameters.")); return; }
    auto* title = new QLabel(nodeTitle(step->delegateName()));
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
                canvasScene.undoStack().push(new SetParameterCommand(document, id, next));
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
        const QString state = value.state == StepState::Succeeded ? "Complete" :
                              value.state == StepState::Failed ? "Failed" : "Skipped";
        auto* item = new QTreeWidgetItem(results, {nodeTitle(step->delegateName()), state, outputText(value)});
        item->setData(0, Qt::UserRole, QString::fromStdString(step->id().toString()));
        item->setToolTip(2, outputText(value));
        if(step->id() == selected) outputLabel->setText("Output: " + outputText(value));
    }
    for(int column = 0; column < 3; ++column) results->resizeColumnToContents(column);
}
} // namespace smartflow
