#ifndef STKMESH_H
#define STKMESH_H


#include <IMeshSceneNode.h>
#include <IMesh.h>
#include "../lib/irrlicht/source/Irrlicht/CMeshSceneNode.h"
#include "glwrap.hpp"
#include <vector>

enum GeometricMaterial
{
    FPSM_DEFAULT,
    FPSM_ALPHA_REF_TEXTURE,
    FPSM_NORMAL_MAP,
    FPSM_GRASS,
    FPSM_COUNT
};

enum ShadedMaterial
{
    SM_DEFAULT,
    SM_ALPHA_REF_TEXTURE,
    SM_RIMLIT,
    SM_SPHEREMAP,
    SM_SPLATTING,
    SM_GRASS,
    SM_UNLIT,
    SM_CAUSTICS,
    SM_DETAILS,
    SM_UNTEXTURED,
    SM_COUNT
};

enum TransparentMaterial
{
    TM_DEFAULT,
    TM_ADDITIVE,
    TM_BUBBLE,
    TM_COUNT
};

struct GLMesh {
    GLuint vao_first_pass;
    GLuint vao_second_pass;
    GLuint vao_glow_pass;
    GLuint vao_displace_pass;
    GLuint vao_displace_mask_pass;
    GLuint vao_shadow_pass;
    GLuint vertex_buffer;
    GLuint index_buffer;
    video::ITexture *textures[6];
    GLenum PrimitiveType;
    GLenum IndexType;
    size_t IndexCount;
    size_t Stride;
    core::matrix4 TextureMatrix;
};

GLuint createVAO(GLuint vbo, GLuint idx, GLuint attrib_position, GLuint attrib_texcoord, GLuint attrib_second_texcoord, GLuint attrib_normal, GLuint attrib_tangent, GLuint attrib_bitangent, GLuint attrib_color, size_t stride);
GLMesh allocateMeshBuffer(scene::IMeshBuffer* mb);
void initvaostate(GLMesh &mesh, GeometricMaterial GeoMat, ShadedMaterial ShadedMat);
void initvaostate(GLMesh &mesh, TransparentMaterial TranspMat);
core::matrix4 computeMVP(const core::matrix4 &ModelViewProjectionMatrix);
core::matrix4 computeTIMV(const core::matrix4 &TransposeInverseModelView);
bool isObject(video::E_MATERIAL_TYPE type);

core::vector3df getWind();

// Pass 1 shader (ie shaders that outputs normals and depth)
template<enum GeometricMaterial T>
class GroupedFPSM
{
public:
    static std::vector<GLMesh *> MeshSet;
    static std::vector<core::matrix4> MVPSet, TIMVSet;

    static void reset()
    {
        MeshSet.clear();
        MVPSet.clear();
        TIMVSet.clear();
    }
};

template<enum GeometricMaterial T>
std::vector<GLMesh *> GroupedFPSM<T>::MeshSet;
template<enum GeometricMaterial T>
std::vector<core::matrix4> GroupedFPSM<T>::MVPSet;
template<enum GeometricMaterial T>
std::vector<core::matrix4> GroupedFPSM<T>::TIMVSet;

void drawObjectPass1(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView);
void drawNormalPass(const GLMesh &mesh, const core::matrix4 & ModelMatrix, const core::matrix4 &InverseModelMatrix);
void drawObjectRefPass1(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, const core::matrix4 &TextureMatrix);
void drawGrassPass1(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, core::vector3df windDir);

// Pass 2 shader (ie shaders that outputs final color)
template<enum ShadedMaterial T>
class GroupedSM
{
public:
    static std::vector<GLMesh *> MeshSet;
    static std::vector<core::matrix4> MVPSet, TIMVSet;

    static void reset()
    {
        MeshSet.clear();
        MVPSet.clear();
        TIMVSet.clear();
    }
};

template<enum ShadedMaterial T>
std::vector<GLMesh *> GroupedSM<T>::MeshSet;
template<enum ShadedMaterial T>
std::vector<core::matrix4> GroupedSM<T>::MVPSet;
template<enum ShadedMaterial T>
std::vector<core::matrix4> GroupedSM<T>::TIMVSet;

void drawDetailledObjectPass2(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);
void drawObjectPass2(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TextureMatrix);
void drawUntexturedObject(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);
void drawObjectRefPass2(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TextureMatrix);
void drawSphereMap(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView);
void drawSplatting(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);
void drawCaustics(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, core::vector2df dir, core::vector2df dir2);
void drawGrassPass2(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, core::vector3df windDir);
void drawObjectRimLimit(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, const core::matrix4 &TextureMatrix);
void drawObjectUnlit(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);

// Shadow pass
void drawShadowRef(const GLMesh &mesh, const core::matrix4 &ModelMatrix);
void drawShadow(const GLMesh &mesh, const core::matrix4 &ModelMatrix);

// Forward pass (for transparents meshes)
void drawTransparentObject(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TextureMatrix);
void drawTransparentFogObject(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TextureMatrix);
void drawBubble(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);

GeometricMaterial MaterialTypeToGeometricMaterial(video::E_MATERIAL_TYPE);
ShadedMaterial MaterialTypeToShadedMaterial(video::E_MATERIAL_TYPE, irr::video::ITexture **textures);
TransparentMaterial MaterialTypeToTransparentMaterial(video::E_MATERIAL_TYPE, f32 MaterialTypeParam);

#endif // STKMESH_H
