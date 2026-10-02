#pragma once
#include "GraphProject.h"
#include <QDialog>
#include <tuple>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QFormLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QPlainTextEdit;

namespace smartflow {
class WorkspaceWindow;
class ComponentAuthorDialog : public QDialog {
public:
    ComponentAuthorDialog(GraphProject& project, std::vector<std::string> selection, QWidget* parent = nullptr,
        project::Document seed = nullptr, bool publish = true);
    void accept() override;
    const project::Document& definition() const { return authored; }
private:
    struct EndpointRow {
        std::string section;
        project::Document endpoint;
        QCheckBox* expose;
        QLineEdit* name;
    };
    GraphProject& graphProject;
    std::vector<std::string> selection;
    quint64 revision;
    std::string identity;
    QLineEdit* title;
    QLabel* error;
    std::vector<EndpointRow> rows;
    project::Document seed;
    project::Document authored;
    bool publish;
};

class ComponentEditDialog : public QDialog {
public:
    ComponentEditDialog(GraphProject& destination, WorkspaceConfiguration configuration,
        project::GraphComponent component, QWidget* parent = nullptr);
    WorkspaceWindow& workspace() { return *draft; }
    void saveCopy(const project::GraphComponent& component);
private:
    GraphProject& destination;
    quint64 revision;
    project::Document original;
    WorkspaceWindow* draft;
    QLabel* error;
};

class ComponentUpdateDialog : public QDialog {
public:
    ComponentUpdateDialog(GraphProject& project, std::string nodeId, QWidget* parent = nullptr);
    void accept() override;
private:
    void refresh();
    GraphProject& graphProject;
    std::string nodeId;
    quint64 revision;
    project::Document snapshot;
    project::Document catalog;
    QComboBox* choices;
    QPlainTextEdit* comparison;
    QLabel* error;
    QPushButton* apply;
};

class ComponentLibraryDialog : public QDialog {
public:
    explicit ComponentLibraryDialog(GraphProject& project, QWidget* parent = nullptr);
    void accept() override;
    // Dialog-free file operations throw on failure. Import is one history edit;
    // export does not alter project, workspace or undo state.
    void importFile(const QString& path);
    void exportFile(const QString& path) const;
    void removeSelected();
private:
    void refreshCatalog(int selected = 0);
    void refreshBindings();
    GraphProject& graphProject;
    quint64 revision;
    project::Document catalog;
    QComboBox* library;
    std::string identity;
    QLabel* error;
    QPushButton* insert;
    QPushButton* exportButton;
    QPushButton* removeButton;
    QPushButton* editButton;
    QCheckBox* collapsed;
    QFormLayout* bindings;
    std::vector<std::pair<std::string,QComboBox*>> inputs;
    std::vector<std::tuple<std::string,QDoubleSpinBox*,double>> controls;
};
} // namespace smartflow
