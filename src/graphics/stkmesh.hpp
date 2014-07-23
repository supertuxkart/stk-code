#ifndef STKMESH_H
#define STKMESH_H

#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"

#include <IMeshSceneNode.h>
#include <IMesh.h>
#include "../lib/irrlicht/source/Irrlicht/CMeshSceneNode.h"

#include <tuple>
#include <vector>

enum MeshMaterial
{
    MAT_DEFAULT,
    MAT_ALPHA_REF,
    MAT_NORMAL_MAP,
    MAT_GRASS,
    MAT_SPHEREMAP,
    MAT_SPLATTING,
    MAT_UNLIT,
    MAT_DETAIL,
    MAT_COUNT
};

enum TransparentMaterial
{
    TM_DEFAULT,
    TM_ADDITIVE,
    TM_BUBBLE,
    TM_DISPLACEMENT,
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
core::matrix4 computeMVP(const core::matrix4 &ModelViewProjectionMatrix);
bool isObject(video::E_MATERIAL_TYPE type);

core::vector3df getWind();

// Pass 1 shader (ie shaders that outputs normals and depth)
class ListMatDefault
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, core::matrix4, video::SColorf> > Arguments;
};

class ListMatAlphaRef
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, core::matrix4, video::SColorf> > Arguments;
};

class ListMatNormalMap
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, core::matrix4, video::SColorf> > Arguments;
};

class ListMatGrass
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, core::vector3df, video::SColorf> > Arguments;
};

class ListMatSphereMap
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, core::matrix4, video::SColorf> > Arguments;
};

class ListMatSplatting
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, video::SColorf> > Arguments;
};

class ListMatUnlit
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4> > Arguments;
};

class ListMatDetails
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, core::matrix4, video::SColorf> > Arguments;
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

class ListDisplacement
{
public:
    static std::vector<std::tuple<GLMesh *, core::matrix4> > Arguments;
};

// Forward pass (for transparents meshes)
void drawBubble(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);

MeshMaterial MaterialTypeToMeshMaterial(video::E_MATERIAL_TYPE, video::E_VERTEX_TYPE);
TransparentMaterial MaterialTypeToTransparentMaterial(video::E_MATERIAL_TYPE, f32 MaterialTypeParam);

#endif // STKMESH_H
