#ifndef STKMESH_H
#define STKMESH_H


#include <IMeshSceneNode.h>
#include <IMesh.h>
#include "../lib/irrlicht/source/Irrlicht/CMeshSceneNode.h"

class STKMesh : public irr::scene::CMeshSceneNode
{
public:
	STKMesh(irr::scene::IMesh* mesh, ISceneNode* parent, irr::scene::ISceneManager* mgr,	irr::s32 id,
		const irr::core::vector3df& position = irr::core::vector3df(0,0,0),
		const irr::core::vector3df& rotation = irr::core::vector3df(0,0,0),
		const irr::core::vector3df& scale = irr::core::vector3df(1.0f, 1.0f, 1.0f));
	virtual void render();
};

#endif // STKMESH_H
