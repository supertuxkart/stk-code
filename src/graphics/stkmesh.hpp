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
bool isObject(video::E_MATERIAL_TYPE type);

core::vector3df getWind();

// Pass 1 shader (ie shaders that outputs normals and depth)
class ListDefaultStandardG
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4> > Arguments;
};

class ListDefault2TCoordG
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4> > Arguments;
};

class ListAlphaRefG
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, core::matrix4> > Arguments;
};

class ListNormalG
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4> > Arguments;
};

class ListGrassG
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, core::vector3df> > Arguments;
};

template<typename Shader, typename...uniforms>
void draw(const GLMesh *mesh, uniforms... Args)
{
    irr_driver->IncreaseObjectCount();
    GLenum ptype = mesh->PrimitiveType;
    GLenum itype = mesh->IndexType;
    size_t count = mesh->IndexCount;

    Shader::setUniforms(Args...);
    glDrawElementsBaseVertex(ptype, count, itype, (GLvoid *)mesh->vaoOffset, mesh->vaoBaseVertex);
}


template<typename T, typename...uniforms>
void draw(const T *Shader, const GLMesh *mesh, uniforms... Args)
{
    irr_driver->IncreaseObjectCount();
    GLenum ptype = mesh->PrimitiveType;
    GLenum itype = mesh->IndexType;
    size_t count = mesh->IndexCount;

    Shader->setUniforms(Args...);
    glDrawElementsBaseVertex(ptype, count, itype, (GLvoid *)mesh->vaoOffset, mesh->vaoBaseVertex);
}

// Pass 2 shader (ie shaders that outputs final color)
class ListDefaultStandardSM
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, video::SColorf> > Arguments;
};

class ListDefaultTangentSM
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, video::SColorf> > Arguments;
};

class ListAlphaRefSM
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, video::SColorf> > Arguments;
};

class ListSphereMapSM
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, video::SColorf> > Arguments;
};

class ListSplattingSM
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, video::SColorf> > Arguments;
};

class ListUnlitSM
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4> > Arguments;
};

class ListDetailSM
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, video::SColorf> > Arguments;
};

class ListGrassSM
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::vector3df, video::SColorf> > Arguments;
};

class ListBlendTransparent
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4> > Arguments;
};

class ListAdditiveTransparent
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4> > Arguments;
};

class ListBlendTransparentFog
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, float, float, float, float, float, video::SColorf> > Arguments;
};

class ListAdditiveTransparentFog
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, float, float, float, float, float, video::SColorf> > Arguments;
};

// Forward pass (for transparents meshes)
void drawBubble(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);

GeometricMaterial MaterialTypeToGeometricMaterial(video::E_MATERIAL_TYPE, video::E_VERTEX_TYPE);
ShadedMaterial MaterialTypeToShadedMaterial(video::E_MATERIAL_TYPE, irr::video::ITexture **textures, video::E_VERTEX_TYPE tp);
TransparentMaterial MaterialTypeToTransparentMaterial(video::E_MATERIAL_TYPE, f32 MaterialTypeParam);

#endif // STKMESH_H
