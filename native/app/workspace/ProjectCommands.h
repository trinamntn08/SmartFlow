#pragma once
#include "GraphProject.h"
#include <QUndoCommand>

namespace smartflow {
class SetParameterCommand final : public QUndoCommand {
public:
    SetParameterCommand(GraphProject& project, tp_utils::StringID node,
                        tp_pipeline::Parameter next)
        : project(project), node(std::move(node)), next(std::move(next))
    {
        previous = project.step(this->node)->parameter(this->next.name);
        setText("Edit " + QString::fromStdString(this->next.name.toString()));
    }
    void redo() override { project.setParameter(node, next); }
    void undo() override { project.setParameter(node, previous); }
private:
    GraphProject& project;
    tp_utils::StringID node;
    tp_pipeline::Parameter previous, next;
};
} // namespace smartflow
