#pragma once
#include <tp_pipeline/StepDelegateMap.h>
#include <tp_data/CollectionFactory.h>
#include <tp_data/Collection.h>
#include <QWidget>
#include <QPointF>
#include <functional>
#include <memory>
#include <vector>

namespace smartflow {
// Initial native contribution contracts for statically bundled extensions.
// These are not a binary plugin ABI or persisted project schema.
struct NodePresentation {
    QString type;
    QString title;
    QString category;
};

class OutputViewer : public QWidget {
public:
    using QWidget::QWidget;
    virtual void present(std::shared_ptr<const tp_data::Collection> output) = 0;
    virtual QString describe(const tp_data::Collection&) const { return {}; }
};

struct PresetNode {
    QString type;
    QPointF position;
    std::vector<std::pair<std::string, double>> parameters;
};
struct PresetConnection { size_t source, output, target, input; };

struct WorkspaceConfiguration {
    std::shared_ptr<tp_pipeline::StepDelegateMap> delegates;
    std::shared_ptr<tp_data::CollectionFactory> factory;
    std::vector<NodePresentation> nodes;
    std::vector<PresetNode> preset;
    std::vector<PresetConnection> connections;
    std::function<OutputViewer*()> createViewer;
};
} // namespace smartflow
