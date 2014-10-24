#ifndef STKANIMATEDMESH_HPP
#define STKANIMATEDMESH_HPP

#include "../lib/irrlicht/source/Irrlicht/CAnimatedMeshSceneNode.h"
#include <IAnimatedMesh.h>
#include <irrTypes.h>
#include "graphics/stkmesh.hpp"
#include "utils/ptr_vector.hpp"

class STKAnimatedMesh : public irr::scene::CAnimatedMeshSceneNode, public STKMeshCommon
{
protected:
    bool isMaterialInitialized;
    bool isGLInitialized;
    std::vector<GLMesh> GLmeshes;
    core::matrix4 ModelViewProjectionMatrix;
    void cleanGLMeshes();
public:
    virtual void updateNoGL();
    virtual void updateGL();
  STKAnimatedMesh(irr::scene::IAnimatedMesh* mesh, irr::scene::ISceneNode* parent,
     irr::scene::ISceneManager* mgr, irr::s32 id, const std::string& debug_name,
     const irr::core::vector3df& position = irr::core::vector3df(0,0,0),
     const irr::core::vector3df& rotation = irr::core::vector3df(0,0,0),
     const irr::core::vector3df& scale = irr::core::vector3df(1.0f, 1.0f, 1.0f));
  ~STKAnimatedMesh();

  virtual void render();
  virtual void setMesh(irr::scene::IAnimatedMesh* mesh);
  virtual bool glow() const { return false; }
};

#endif // STKANIMATEDMESH_HPP
