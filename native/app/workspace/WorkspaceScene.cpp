#include "WorkspaceScene.h"
#include <QtNodes/internal/ConnectionGraphicsObject.hpp>
#include <set>

namespace smartflow {
void WorkspaceScene::createNode(const QString& type, const QPointF& position)
{
    const auto id=canvas.addNode(type);
    if(id!=QtNodes::InvalidNodeId) canvas.setNodeData(id,QtNodes::NodeRole::Position,position);
}

void WorkspaceScene::deleteSelected()
{
    std::set<std::string> nodes, edges;
    for(const auto id : selectedNodes()) nodes.insert(canvas.projectId(id).toString());
    for(auto* item : selectedItems())
        if(auto* edge=qgraphicsitem_cast<QtNodes::ConnectionGraphicsObject*>(item)) {
            const auto connection=edge->connectionId();
            const auto id=canvas.edgeId(connection);
            if(!id.empty()) edges.insert(id);
        }
    project.commands().edit("Delete selection",[&](auto& candidate) {
        for(const auto& edge : edges) candidate.disconnect(edge);
        for(const auto& node : nodes) candidate.removeNode(node);
    });
}

QMenu* WorkspaceScene::createSceneMenu(QPointF position)
{
    auto* menu=new QMenu;
    menu->setAttribute(Qt::WA_DeleteOnClose);
    for(const auto& entry : project.registry()->stepDelegates()) {
        const auto type=QString::fromStdString(entry.first.toString());
        auto* action=menu->addAction(project.title(entry.first));
        QObject::connect(action,&QAction::triggered,this,[this,type,position] { createNode(type,position); });
    }
    return menu;
}
} // namespace smartflow
