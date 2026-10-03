#pragma once
#include "ExecutionController.h"
#include <QWidget>

class QComboBox;
class QLabel;
class QProgressBar;
class QVBoxLayout;

namespace smartflow {
// A component's public controls and outputs form its tool interface. The graph
// and execution remain shared with Build mode; viewer navigation is independent.
class UseWorkspace final : public QWidget {
    Q_OBJECT
public:
    UseWorkspace(GraphProject& project, ExecutionController& execution,
                 const std::function<OutputViewer*()>& createViewer, QWidget* parent = nullptr);
    ~UseWorkspace() override;
    project::Document saveState() const;
    void restoreState(const project::Document& state);
    std::function<void()> changed;
private:
    void refreshControls();
    void refreshResults();
    GraphProject& document;
    ExecutionController& runner;
    QComboBox* tools;
    QComboBox* outputs;
    QWidget* controls = nullptr;
    QVBoxLayout* controlsLayout;
    QLabel* status;
    QLabel* description;
    QProgressBar* progress;
    OutputViewer* viewer = nullptr;
    QString componentId;
    QString outputPort;
    project::Document retained = project::Document::object();
};
} // namespace smartflow
