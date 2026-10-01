#pragma once
#include "PipelineCanvas.h"
#include <QtNodes/DataFlowGraphicsScene>

namespace smartflow {
class WorkspaceScene final : public QtNodes::DataFlowGraphicsScene {
public:
    WorkspaceScene(PipelineCanvas& canvas, GraphProject& project)
        : DataFlowGraphicsScene(canvas), canvas(canvas), project(project) {}
    QUndoStack& undoStack() override { return project.commands().undoStack(); }
    void createNode(const QString& type, const QPointF& position) override;
    void deleteSelected() override;
    void connectNodes(QtNodes::ConnectionId connection) override { canvas.addConnection(connection); }
    void disconnectNodes(QtNodes::ConnectionId connection) override { canvas.deleteConnection(connection); }
    QMenu* createSceneMenu(QPointF position) override;
private:
    PipelineCanvas& canvas;
    GraphProject& project;
};
} // namespace smartflow
