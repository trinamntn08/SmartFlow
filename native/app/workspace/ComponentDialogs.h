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

namespace smartflow {
class ComponentAuthorDialog : public QDialog {
public:
    ComponentAuthorDialog(GraphProject& project, std::vector<std::string> selection, QWidget* parent = nullptr);
    void accept() override;
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
};

class ComponentLibraryDialog : public QDialog {
public:
    explicit ComponentLibraryDialog(GraphProject& project, QWidget* parent = nullptr);
    void accept() override;
private:
    void refreshBindings();
    GraphProject& graphProject;
    quint64 revision;
    project::Document catalog;
    QComboBox* library;
    std::string identity;
    QLabel* error;
    QPushButton* insert;
    QCheckBox* collapsed;
    QFormLayout* bindings;
    std::vector<std::pair<std::string,QComboBox*>> inputs;
    std::vector<std::tuple<std::string,QDoubleSpinBox*,double>> controls;
};
} // namespace smartflow
