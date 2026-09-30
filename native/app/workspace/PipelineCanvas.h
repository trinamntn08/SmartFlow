#pragma once
#include "GraphProject.h"
#include <QtNodes/DataFlowGraphModel>

namespace smartflow {
// Translates canvas operations (including QtNodes undo commands) into project
// edits. The copied canvas owns only its presentation state and transient IDs.
class PipelineCanvas : public QtNodes::DataFlowGraphModel {
public:
    explicit PipelineCanvas(GraphProject& project);
    QtNodes::NodeId addNode(QString type) override;
    bool deleteNode(QtNodes::NodeId id) override;
    bool connectionPossible(QtNodes::ConnectionId connection) const override;
    void addConnection(QtNodes::ConnectionId connection) override;
    bool deleteConnection(QtNodes::ConnectionId connection) override;
    QJsonObject saveNode(QtNodes::NodeId id) const override;
    void loadNode(const QJsonObject& snapshot) override;
    tp_utils::StringID projectId(QtNodes::NodeId id) const;
private:
    GraphProject& project;
    std::unordered_map<QtNodes::NodeId, tp_utils::StringID> bindings;
};
} // namespace smartflow
