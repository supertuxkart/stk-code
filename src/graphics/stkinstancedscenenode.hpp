#ifndef STKINSTANCEDSCENENODE_HPP
#define STKINSTANCEDSCENENODE_HPP

#include "stkmesh.hpp"

class STKInstancedSceneNode : public irr::scene::CMeshSceneNode
{
protected:
    std::vector<GLMesh *> GeometricMesh[FPSM_COUNT];
    std::vector<GLMesh *> ShadedMesh[SM_COUNT];
    std::vector<GLMesh> GLmeshes;
    std::vector<float> instance_pos;
    core::matrix4 ModelViewProjectionMatrix, TransposeInverseModelView;
    GLuint instances_vbo;
    void createGLMeshes();
    bool isMaterialInitialized;
    void setFirstTimeMaterial();
    void initinstancedvaostate(GLMesh &mesh, GeometricMaterial GeoMat, ShadedMaterial ShadedMat);
    core::vector3df windDir;
public:
    STKInstancedSceneNode(irr::scene::IMesh* mesh, ISceneNode* parent, irr::scene::ISceneManager* mgr, irr::s32 id,
        const irr::core::vector3df& position = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& rotation = irr::core::vector3df(0, 0, 0),
        const irr::core::vector3df& scale = irr::core::vector3df(1.0f, 1.0f, 1.0f));
    virtual void render();
    void addWorldMatrix(const core::vector3df &);
};

#endif
