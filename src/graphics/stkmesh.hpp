#ifndef STKMESH_H
#define STKMESH_H

#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"

#include <IMeshSceneNode.h>
#include <IMesh.h>
#include "../lib/irrlicht/source/Irrlicht/CMeshSceneNode.h"

#include <tuple>
#include <vector>

enum GeometricMaterial
{
    FPSM_DEFAULT_STANDARD,
    FPSM_DEFAULT_2TCOORD,
    FPSM_ALPHA_REF_TEXTURE,
    FPSM_NORMAL_MAP,
    FPSM_GRASS,
    FPSM_COUNT
};

enum ShadedMaterial
{
    SM_DEFAULT_STANDARD,
    SM_DEFAULT_TANGENT,
    SM_ALPHA_REF_TEXTURE,
    SM_SPHEREMAP,
    SM_SPLATTING,
    SM_GRASS,
    SM_UNLIT,
    SM_DETAILS,
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
    GLuint vao;
    GLuint vao_shadow_pass;
    GLuint vertex_buffer;
    GLuint index_buffer;
    video::ITexture *textures[6];
    GLenum PrimitiveType;
    GLenum IndexType;
    size_t IndexCount;
    size_t Stride;
    core::matrix4 TextureMatrix;
    size_t vaoBaseVertex;
    size_t vaoOffset;
    video::E_VERTEX_TYPE VAOType;
};

GLMesh allocateMeshBuffer(scene::IMeshBuffer* mb);
void fillLocalBuffer(GLMesh &, scene::IMeshBuffer* mb);
video::E_VERTEX_TYPE getVTXTYPEFromStride(size_t stride);
GLuint createVAO(GLuint vbo, GLuint idx, video::E_VERTEX_TYPE type);
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


template<typename Shader, typename...uniforms>
void draw(GLMesh *mesh, uniforms... Args)
{
    irr_driver->IncreaseObjectCount();
    GLenum ptype = mesh->PrimitiveType;
    GLenum itype = mesh->IndexType;
    size_t count = mesh->IndexCount;

    Shader::setUniforms(Args...);
    glDrawElementsBaseVertex(ptype, count, itype, (GLvoid *)mesh->vaoOffset, mesh->vaoBaseVertex);
}

void drawGrassPass1(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, core::vector3df windDir);

// Pass 2 shader (ie shaders that outputs final color)
class ListDefaultStandardSM
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4> > Arguments;
};

class ListDefaultTangentSM
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4> > Arguments;
};

class ListAlphaRefSM
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4> > Arguments;
};

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

void drawSplatting(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);
void drawGrassPass2(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, core::vector3df windDir);

// Shadow pass
void drawShadowRef(const GLMesh &mesh, const core::matrix4 &ModelMatrix);
void drawShadow(const GLMesh &mesh, const core::matrix4 &ModelMatrix);

template<enum TransparentMaterial T>
class TransparentMeshes
{
public:
    static std::vector<GLMesh *> MeshSet;
    static std::vector<core::matrix4> MVPSet;

    static void reset()
    {
        MeshSet.clear();
        MVPSet.clear();
    }
};

template<enum TransparentMaterial T>
std::vector<GLMesh *> TransparentMeshes<T>::MeshSet;
template<enum TransparentMaterial T>
std::vector<core::matrix4> TransparentMeshes<T>::MVPSet;

// Forward pass (for transparents meshes)
void drawTransparentObject(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TextureMatrix);
void drawTransparentFogObject(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TextureMatrix);
void drawBubble(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);

GeometricMaterial MaterialTypeToGeometricMaterial(video::E_MATERIAL_TYPE, video::E_VERTEX_TYPE);
ShadedMaterial MaterialTypeToShadedMaterial(video::E_MATERIAL_TYPE, irr::video::ITexture **textures, video::E_VERTEX_TYPE tp);
TransparentMaterial MaterialTypeToTransparentMaterial(video::E_MATERIAL_TYPE, f32 MaterialTypeParam);

#endif // STKMESH_H
