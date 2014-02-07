#ifndef STKANIMATEDMESH_HPP
#define STKANIMATEDMESH_HPP

#include "../lib/irrlicht/source/Irrlicht/CAnimatedMeshSceneNode.h"
#include <IAnimatedMesh.h>
#include <irrTypes.h>
#include "graphics/stkmesh.hpp"


class STKAnimatedMesh : public irr::scene::CAnimatedMeshSceneNode
{
protected:
	bool firstTime;
	std::vector<GLMesh> GLmeshes;
	core::matrix4 ModelViewProjectionMatrix, TransposeInverseModelView;
	void drawSolid(const GLMesh &mesh, video::E_MATERIAL_TYPE type);
	void drawTransparent(const GLMesh &mesh, video::E_MATERIAL_TYPE type);
public:
  STKAnimatedMesh(irr::scene::IAnimatedMesh* mesh, irr::scene::ISceneNode* parent,
     irr::scene::ISceneManager* mgr, irr::s32 id,
     const irr::core::vector3df& position = irr::core::vector3df(0,0,0),
     const irr::core::vector3df& rotation = irr::core::vector3df(0,0,0),
     const irr::core::vector3df& scale = irr::core::vector3df(1.0f, 1.0f, 1.0f));

  virtual void render();
  virtual void setMesh(irr::scene::IAnimatedMesh* mesh);
};

#endif // STKANIMATEDMESH_HPP
