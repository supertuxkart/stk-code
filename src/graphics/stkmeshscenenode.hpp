#ifndef STKMESHSCENENODE_H
#define STKMESHSCENENODE_H

#include "stkmesh.hpp"
#include "utils/ptr_vector.hpp"

class STKMeshSceneNode : public irr::scene::CMeshSceneNode, public STKMeshCommon
{
protected:
    std::vector<GLMesh> GLmeshes;
    core::matrix4 ModelViewProjectionMatrix;
    core::vector3df windDir;
    core::vector2df caustic_dir, caustic_dir2;

    // Misc passes shaders (glow, displace...)
    void drawGlow(const GLMesh &mesh);
    void createGLMeshes();
    void cleanGLMeshes();
    void setFirstTimeMaterial();
    void updatevbo();
    bool isMaterialInitialized;
    bool immediate_draw;
    bool update_each_frame;
    bool isDisplacement;
public:
    virtual void update();
    void setReloadEachFrame(bool);
    STKMeshSceneNode(irr::scene::IMesh* mesh, ISceneNode* parent, irr::scene::ISceneManager* mgr, irr::s32 id,
        const irr::core::vector3df& position = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& rotation = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& scale = irr::core::vector3df(1.0f, 1.0f, 1.0f),
        bool createGLMeshes = true);
    virtual void render();
    virtual void setMesh(irr::scene::IMesh* mesh);
    virtual void OnRegisterSceneNode();
    virtual ~STKMeshSceneNode();
    virtual bool isImmediateDraw() const { return immediate_draw; }
    void setIsDisplacement(bool v) {
        isDisplacement = v;
        for (u32 i = 0; i < Mesh->getMeshBufferCount(); ++i)
        {
            scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
            if (!mb)
                continue;
            if (isDisplacement)
                mb->getMaterial().MaterialType = irr_driver->getShader(ES_DISPLACE);
        }
    }
};

#endif
