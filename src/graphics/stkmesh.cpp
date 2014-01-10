#include "stkmesh.hpp"

STKMesh::STKMesh(irr::scene::IMesh* mesh, ISceneNode* parent, irr::scene::ISceneManager* mgr,	irr::s32 id,
	const irr::core::vector3df& position,
	const irr::core::vector3df& rotation,
	const irr::core::vector3df& scale) :
		CMeshSceneNode(mesh, parent, mgr, id, position, rotation, scale)
{

}

void STKMesh::render()
{
	CMeshSceneNode::render();
}
