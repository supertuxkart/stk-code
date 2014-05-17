#ifndef STKMESHSCENENODE_H
#define STKMESHSCENENODE_H

#include "stkmesh.hpp"
#include "utils/ptr_vector.hpp"

class STKMeshSceneNode : public irr::scene::CMeshSceneNode
{
protected:
    PtrVector<GLMesh, REF> GeometricMesh[FPSM_COUNT];
    PtrVector<GLMesh, REF> ShadedMesh[SM_COUNT];
    PtrVector<GLMesh, REF> TransparentMesh[TM_COUNT];
    std::vector<GLMesh> GLmeshes;
    core::matrix4 ModelViewProjectionMatrix, TransposeInverseModelView;
    core::vector3df windDir;
    core::vector2df caustic_dir, caustic_dir2;
    void drawSolidPass1(const GLMesh &mesh, GeometricMaterial type);
    void drawSolidPass2(const GLMesh &mesh, ShadedMaterial type);
    void drawTransparent(const GLMesh &mesh, video::E_MATERIAL_TYPE type);

    // Misc passes shaders (glow, displace...)
    void drawGlow(const GLMesh &mesh);
    void drawDisplace(const GLMesh &mesh);
    void createGLMeshes();
    void cleanGLMeshes();
    void setFirstTimeMaterial();
    void updatevbo();
    bool isMaterialInitialized;
    bool reload_each_frame;
public:
    void setReloadEachFrame(bool);
    STKMeshSceneNode(irr::scene::IMesh* mesh, ISceneNode* parent, irr::scene::ISceneManager* mgr, irr::s32 id,
        const irr::core::vector3df& position = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& rotation = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& scale = irr::core::vector3df(1.0f, 1.0f, 1.0f));
    virtual void render();
    virtual void setMesh(irr::scene::IMesh* mesh);
    virtual ~STKMeshSceneNode();
};

#endif
