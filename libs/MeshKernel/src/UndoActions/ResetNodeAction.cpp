#include "MeshKernel/UndoActions/ResetNodeAction.hpp"
#include "MeshKernel/Mesh.hpp"

std::unique_ptr<meshkernel::ResetNodeAction> meshkernel::ResetNodeAction::Create(Mesh& mesh, const UInt id, const Point& initial, const Point& updated)
{
    return std::make_unique<ResetNodeAction>(mesh, id, initial, updated);
}

meshkernel::ResetNodeAction::ResetNodeAction(Mesh& mesh, const UInt id, const Point& initial, const Point& updated) : BaseMeshUndoAction<ResetNodeAction, Mesh>(mesh), m_nodeId(id), m_initialNode(initial), m_updatedNode(updated) {}

meshkernel::UInt meshkernel::ResetNodeAction::NodeId() const
{
    return m_nodeId;
}

const meshkernel::Point& meshkernel::ResetNodeAction::InitialNode() const
{
    return m_initialNode;
}

const meshkernel::Point& meshkernel::ResetNodeAction::UpdatedNode() const
{
    return m_updatedNode;
}
