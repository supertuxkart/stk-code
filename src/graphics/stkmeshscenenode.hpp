#ifndef STKMESHSCENENODE_H
#define STKMESHSCENENODE_H

#include "stkmesh.hpp"

class STKMeshSceneNode : public irr::scene::CMeshSceneNode
{
protected:
    std::vector<GLMesh> GLmeshes;
    core::matrix4 ModelViewProjectionMatrix, TransposeInverseModelView, TextureMatrix;
    core::vector3df windDir;
    void drawSolid(const GLMesh &mesh, video::E_MATERIAL_TYPE type);
    void drawTransparent(const GLMesh &mesh, video::E_MATERIAL_TYPE type);

    // Misc passes shaders (glow, displace...)
    void drawGlow(const GLMesh &mesh);
    void drawDisplace(const GLMesh &mesh);
    void drawShadow(const GLMesh &mesh, video::E_MATERIAL_TYPE type);
    void createGLMeshes();
    void cleanGLMeshes();
public:
    STKMeshSceneNode(irr::scene::IMesh* mesh, ISceneNode* parent, irr::scene::ISceneManager* mgr, irr::s32 id,
        const irr::core::vector3df& position = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& rotation = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& scale = irr::core::vector3df(1.0f, 1.0f, 1.0f));
    virtual void render();
    virtual void setMesh(irr::scene::IMesh* mesh);
    void MovingTexture(unsigned, unsigned);
    ~STKMeshSceneNode();
};

#endif