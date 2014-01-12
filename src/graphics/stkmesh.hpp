#ifndef STKMESH_H
#define STKMESH_H


#include <IMeshSceneNode.h>
#include <IMesh.h>
#include "../lib/irrlicht/source/Irrlicht/CMeshSceneNode.h"
#include "glwrap.hpp"
#include <vector>

class STKMesh : public irr::scene::CMeshSceneNode
{
protected:
	std::vector<GLuint> vertex_buffer, index_buffer, textures;
	std::vector<GLenum> Primitivetype, Indextype;
	std::vector<size_t> Indexcount;
	std::vector<size_t> Stride;

	void draw(unsigned i);
public:
	STKMesh(irr::scene::IMesh* mesh, ISceneNode* parent, irr::scene::ISceneManager* mgr,	irr::s32 id,
		const irr::core::vector3df& position = irr::core::vector3df(0,0,0),
		const irr::core::vector3df& rotation = irr::core::vector3df(0,0,0),
		const irr::core::vector3df& scale = irr::core::vector3df(1.0f, 1.0f, 1.0f));
	virtual void render();
	~STKMesh();
};

#endif // STKMESH_H
