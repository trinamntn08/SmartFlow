#include "UseWorkspace.h"
#include "ViewerCommandAdapter.h"
#include <tp_qt_pipeline_widgets/parameter_editors/DoubleParameterEditor.h>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QJsonDocument>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSplitter>
#include <QVBoxLayout>
#include <algorithm>
#include <map>

namespace smartflow {
UseWorkspace::UseWorkspace(GraphProject& project, ExecutionController& execution,
                           const std::function<OutputViewer*()>& createViewer, QWidget* parent)
    : QWidget(parent), document(project), runner(execution)
{
    setObjectName("useWorkspace");
    auto* layout = new QVBoxLayout(this);
    auto* split = new QSplitter;
    auto* sidebar = new QWidget;
    auto* sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->addWidget(new QLabel("Tool"));
    tools = new QComboBox;
    tools->setObjectName("useComponent");
    sidebarLayout->addWidget(tools);
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    auto* body = new QWidget;
    controlsLayout = new QVBoxLayout(body);
    controlsLayout->addStretch();
    scroll->setWidget(body);
    sidebarLayout->addWidget(scroll, 1);
    split->addWidget(sidebar);
    auto* resultArea = new QWidget;
    auto* resultLayout = new QVBoxLayout(resultArea);
    resultLayout->addWidget(new QLabel("Output"));
    outputs = new QComboBox;
    outputs->setObjectName("useOutput");
    resultLayout->addWidget(outputs);
    if(createViewer) {
        viewer = createViewer();
        attachViewerCommands(*viewer,document,[this] { return componentId; });
        viewer->setObjectName("useResultViewer");
        resultLayout->addWidget(viewer, 1);
        viewer->workspaceStateChanged = [this] { if(changed) changed(); };
    }
    description = new QLabel;
    description->setObjectName("useResultDescription");
    description->setTextFormat(Qt::PlainText);
    description->setWordWrap(true);
    resultLayout->addWidget(description, viewer ? 0 : 1);
    split->addWidget(resultArea);
    split->setStretchFactor(0, 0);
    split->setStretchFactor(1, 1);
    split->setSizes({300, 800});
    layout->addWidget(split, 1);
    progress = new QProgressBar;
    progress->setObjectName("useProgress");
    layout->addWidget(progress);
    status = new QLabel;
    status->setObjectName("useStatus");
    status->setTextFormat(Qt::PlainText);
    status->setWordWrap(true);
    layout->addWidget(status);
    connect(tools, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] {
        componentId = tools->currentData().toString();
        outputPort.clear();
        refreshControls(); refreshResults();
        if(changed) changed();
    });
    connect(outputs, qOverload<int>(&QComboBox::currentIndexChanged), this, [this] {
        outputPort = outputs->currentText();
        refreshResults();
        if(changed) changed();
    });
    connect(&document, &GraphProject::changed, this, [this] { refreshControls(); refreshResults(); });
    connect(&runner, &ExecutionController::updated, this, &UseWorkspace::refreshResults);
    refreshControls(); refreshResults();
}

UseWorkspace::~UseWorkspace()
{
    if(viewer) { viewer->workspaceStateChanged = {}; viewer->commands = {}; }
}

void UseWorkspace::refreshControls()
{
    const QSignalBlocker toolGuard(tools), outputGuard(outputs);
    tools->clear(); outputs->clear();
    std::map<QString, int> titleCounts;
    for(const auto& node : document.selectedGraph()["nodes"]) {
        if(!project::GraphComponent::isInstance(node)) continue;
        const auto id = QString::fromStdString(node["id"].get<std::string>());
        auto* step = document.step(id.toStdString());
        const auto title = step ? document.title(step->delegateName()) : QString("Unavailable component");
        const auto count = ++titleCounts[title];
        tools->addItem(count == 1 ? title : title + QString(" (%1)").arg(count), id);
    }
    auto index = tools->findData(componentId);
    tools->setCurrentIndex(index >= 0 ? index : (tools->count() ? 0 : -1));
    componentId = tools->currentData().toString();
    tools->setEnabled(tools->count() > 1);
    if(controls) {
        controlsLayout->removeWidget(controls);
        controls->hide(); controls->deleteLater();
    }
    controls = new QWidget;
    controlsLayout->insertWidget(0, controls);
    auto* layout = new QVBoxLayout(controls);
    layout->setContentsMargins(0, 0, 0, 0);
    auto* step = document.step(componentId.toStdString());
    if(!step) {
        auto* hint = new QLabel(componentId.isEmpty()
            ? "No tool in this graph yet. Switch to Build, create a component with exposed controls and outputs, then insert it as one node."
            : "This component is unavailable. Switch to Build to inspect its diagnostics. Saved content is retained.");
        hint->setWordWrap(true); layout->addWidget(hint);
    } else {
        auto* heading = new QLabel("Controls");
        auto font = heading->font(); font.setBold(true); heading->setFont(font);
        layout->addWidget(heading);
        if(step->parameters().empty()) layout->addWidget(new QLabel("This tool has no exposed controls."));
        std::vector<tp_pipeline::Parameter> publicControls;
        for(const auto& node : document.selectedGraph()["nodes"])
            if(node["id"] == componentId.toStdString())
                for(const auto& control : node["component"]["controls"])
                    publicControls.push_back(step->parameter(control["id"].get<std::string>()));
        // Use the authored interface order, rather than the parameter map's
        // alphabetical order, so the tool's primary controls appear first.
        for(const auto& parameter : publicControls) {
            auto* label = new QLabel(QString::fromStdString(parameter.name.toString()));
            label->setTextFormat(Qt::PlainText); layout->addWidget(label);
            if(parameter.type != tp_pipeline::doubleSID()) {
                layout->addWidget(new QLabel("Editor unavailable; saved value retained."));
                continue;
            }
            auto* editor = new tp_qt_pipeline_widgets::DoubleParameterEditor(parameter);
            auto* spin = editor->findChild<QDoubleSpinBox*>();
            spin->setObjectName("useParameter_" + QString::fromStdString(parameter.name.toString()));
            spin->setKeyboardTracking(false);
            spin->setSingleStep(tpGetVariantValue<double>(parameter.step, 1.0));
            auto* apply = new QPushButton("Apply");
            apply->setObjectName("useApply_" + QString::fromStdString(parameter.name.toString()));
            layout->addWidget(editor); layout->addWidget(apply);
            const auto id = componentId.toStdString();
            const auto revision = document.revision();
            connect(apply, &QPushButton::clicked, this, [this, editor, parameter, id, revision] {
                // A hidden editor from a previous revision cannot edit a newer snapshot.
                if(document.revision() != revision) return;
                auto next = parameter;
                editor->updateParameter(next);
                document.setParameter(id, next);
            });
        }
        for(const auto& port : step->outputMapping())
            outputs->addItem(QString::fromStdString(port.portName.toString()));
    }
    index = outputs->findText(outputPort);
    outputs->setCurrentIndex(index >= 0 ? index : (outputs->count() ? 0 : -1));
    outputPort = outputs->currentText();
    outputs->setEnabled(outputs->count() > 1);
}

void UseWorkspace::refreshResults()
{
    QStringList messages;
    for(const auto& issue : document.diagnostics()) messages << QString::fromStdString(issue);
    if(messages.empty()) messages << runner.status();
    const auto& result = runner.result();
    std::shared_ptr<const tp_data::Collection> output;
    if(result) {
        for(const auto& issue : result->diagnostics) messages << QString::fromStdString(issue);
        const auto found = result->steps.find(componentId.toStdString());
        if(found != result->steps.end()) {
            if(!found->second.error.empty()) messages << QString::fromStdString(found->second.error);
            if(found->second.state == StepState::Succeeded) {
                const auto* step = document.step(componentId.toStdString());
                if(step && found->second.output) for(const auto& port : step->outputMapping())
                    if(port.portName.toString() == outputPort.toStdString()) {
                        auto single = std::make_shared<tp_data::Collection>();
                        if(const auto& member = found->second.output->member(port.dataName)) single->addMember(member);
                        output = std::move(single);
                        break;
                    }
            }
        }
        // Execution errors anywhere in the graph must remain visible in Use mode.
        for(const auto& [id, value] : result->steps)
            if(value.state == StepState::Failed && !value.error.empty())
                messages << QString::fromStdString(id.toString() + ": " + value.error);
    }
    if(viewer) viewer->present(output);
    description->setText(output ? (viewer ? viewer->describe(*output) : "Output ready. This configuration has no result viewer.")
                                : "Run the graph to inspect this tool's output.");
    status->setText(messages.join("\n"));
    const auto& running = runner.progress();
    progress->setRange(0, running ? int(std::max(size_t(1), running->total)) : 1);
    progress->setValue(running ? int(running->completed) : 0);
    progress->setFormat(running ? QString("%1/%2 steps finished").arg(qulonglong(running->completed)).arg(qulonglong(running->total))
                                : runner.status());
}

project::Document UseWorkspace::saveState() const
{
    auto state = retained;
    if(!state.is_object()) return state;
    state["component"] = componentId.toStdString();
    state["output"] = outputPort.toStdString();
    if(viewer && !viewer->workspaceStateKey().isEmpty()) {
        auto& viewers = state["viewers"];
        if(viewers.is_null()) viewers = project::Document::object();
        if(viewers.is_object()) {
            auto& entry = viewers[viewer->workspaceStateKey().toStdString()];
            if(entry.is_null()) entry = project::Document::object();
            if(entry.is_object()) entry.update(project::Document::parse(QJsonDocument(viewer->workspaceState()).toJson().toStdString()));
        }
    }
    return state;
}

void UseWorkspace::restoreState(const project::Document& state)
{
    retained = state;
    componentId.clear(); outputPort.clear();
    if(viewer) viewer->restoreWorkspaceState({});
    if(state.is_object()) {
        if(state.contains("component") && state["component"].is_string()) componentId = QString::fromStdString(state["component"]);
        if(state.contains("output") && state["output"].is_string()) outputPort = QString::fromStdString(state["output"]);
        const auto viewers = state.value("viewers", project::Document::object());
        if(viewer && viewers.is_object()) {
            const auto entry = viewers.value(viewer->workspaceStateKey().toStdString(), project::Document::object());
            if(entry.is_object()) viewer->restoreWorkspaceState(QJsonDocument::fromJson(QByteArray::fromStdString(entry.dump())).object());
        }
    }
    refreshControls(); refreshResults();
}
} // namespace smartflow
