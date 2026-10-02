#pragma once
#include "GraphProject.h"
#include "pipeline/PipelineExecution.h"
#include <QtNodes/DataFlowGraphModel>

namespace smartflow {
// Display adapter. Semantic commands operate on retained documents; syncing
// the canvas never issues edits. IDs and positions survive semantic undo.
class PipelineCanvas : public QtNodes::DataFlowGraphModel {
public:
    explicit PipelineCanvas(GraphProject& project);
    QtNodes::NodeId addNode(QString type) override;
    bool deleteNode(QtNodes::NodeId id) override;
    bool connectionPossible(QtNodes::ConnectionId connection) const override;
    void addConnection(QtNodes::ConnectionId connection) override;
    bool deleteConnection(QtNodes::ConnectionId connection) override;
    void loadNode(const QJsonObject&) override {} // Not a project or clipboard loader.
    QVariant nodeData(QtNodes::NodeId id, QtNodes::NodeRole role) const override;
    tp_utils::StringID projectId(QtNodes::NodeId id) const;
    std::string edgeId(QtNodes::ConnectionId connection) const;
    void resetLayout();
    void showExecutionTimes(const std::optional<ExecutionProgressSnapshot>& progress);
private:
    void synchronize();
    GraphProject& project;
    std::unordered_map<QtNodes::NodeId, tp_utils::StringID> bindings;
    std::map<std::string,QtNodes::NodeId> identities;
    std::map<std::string,QPointF> positions;
    std::unordered_map<QtNodes::ConnectionId,std::string> edgeBindings;
    bool syncing = false;
};
} // namespace smartflow
