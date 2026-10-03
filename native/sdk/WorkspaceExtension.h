#pragma once
#include "ExecutionPolicy.h"
#include <tp_pipeline/StepDelegateMap.h>
#include <tp_data/CollectionFactory.h>
#include <tp_data/Collection.h>
#include <QWidget>
#include <QPointF>
#include <QJsonObject>
#include <functional>
#include <memory>
#include <vector>
#include <optional>

namespace smartflow {
// Initial native contribution contracts for statically bundled extensions.
// These are not a binary plugin ABI or persisted project schema.
struct NodePresentation {
    QString type;
    QString title;
    QString category;
    QString packageId;
    QString typeId;
    int contractVersion = 0;
};

struct ViewerParameter {
    double value, minimum, maximum;
};
struct ViewerParameterEdit {
    QString executionNode, parameter;
    double value;
};
struct ViewerEditRequest {
    quint64 revision;
    QString label;
    std::vector<ViewerParameterEdit> parameters;
};
// Source-level optional command capability. Targets are execution identities;
// the application resolves ordinary nodes or exposed component controls.
struct ViewerCommands {
    std::function<quint64()> revision;
    std::function<std::optional<ViewerParameter>(const QString&,const QString&)> parameter;
    // Empty string on success; otherwise leave the project unchanged.
    std::function<QString(const ViewerEditRequest&)> apply;
};
class OutputViewer : public QWidget {
public:
    using QWidget::QWidget;
    virtual void present(std::shared_ptr<const tp_data::Collection> output) = 0;
    // Opaque versioned viewer state; the platform never interprets domain fields.
    virtual QString workspaceStateKey() const { return {}; }
    virtual QJsonObject workspaceState() const { return {}; }
    virtual void restoreWorkspaceState(const QJsonObject&) {}
    std::function<void()> workspaceStateChanged;
    ViewerCommands commands;
    virtual void projectChanged() {}
    virtual QString describe(const tp_data::Collection&) const { return {}; }
};

struct PresetNode {
    QString type;
    QPointF position;
    std::vector<std::pair<std::string, double>> parameters;
};
struct PresetConnection { size_t source, output, target, input; };

struct WorkspaceConfiguration {
    NodeExecutionPolicies executionPolicies;
    std::shared_ptr<tp_pipeline::StepDelegateMap> delegates;
    std::shared_ptr<tp_data::CollectionFactory> factory;
    std::vector<NodePresentation> nodes;
    std::vector<PresetNode> preset;
    std::vector<PresetConnection> connections;
    std::function<OutputViewer*()> createViewer;
};
} // namespace smartflow
