#include "QtNodes/internal/NodeState.hpp"

#include "QtNodes/internal/ConnectionGraphicsObject.hpp"
#include "QtNodes/internal/NodeGraphicsObject.hpp"

namespace QtNodes {

NodeState::NodeState(NodeGraphicsObject &ngo)
    : _ngo(ngo)
    , _hovered(false)
    , _resizing(false)
    , _connectionForReaction{nullptr}
{
    Q_UNUSED(_ngo);
}

void NodeState::setResizing(bool resizing)
{
    _resizing = resizing;
}

bool NodeState::resizing() const
{
    return _resizing;
}

ConnectionGraphicsObject const *NodeState::connectionForReaction() const
{
    return _connectionForReaction.data();
}

void NodeState::storeConnectionForReaction(ConnectionGraphicsObject const *cgo)
{
    _connectionForReaction = cgo;
}

void NodeState::resetConnectionForReaction()
{
    _connectionForReaction.clear();
}

} // namespace QtNodes
