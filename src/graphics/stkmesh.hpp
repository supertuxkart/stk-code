#ifndef STKMESH_H
#define STKMESH_H

#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "utils/tuple.hpp"

#include <IMeshSceneNode.h>
#include <IMesh.h>
#include "../lib/irrlicht/source/Irrlicht/CMeshSceneNode.h"

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
    size_t vaoBaseInstance;
    video::E_VERTEX_TYPE VAOType;
    uint64_t TextureHandles[6];
};

GLMesh allocateMeshBuffer(scene::IMeshBuffer* mb);
void fillLocalBuffer(GLMesh &, scene::IMeshBuffer* mb);
video::E_VERTEX_TYPE getVTXTYPEFromStride(size_t stride);
GLuint createVAO(GLuint vbo, GLuint idx, video::E_VERTEX_TYPE type);
core::matrix4 computeMVP(const core::matrix4 &ModelViewProjectionMatrix);
bool isObject(video::E_MATERIAL_TYPE type);

core::vector3df getWind();

template<typename T, typename... Args>
class MeshList : public Singleton<T>, public std::vector<STK::Tuple<Args...> >
{};

class ListMatDefault : public MeshList<ListMatDefault, GLMesh *, core::matrix4, core::matrix4, core::matrix4>
{};

class ListMatAlphaRef : public MeshList<ListMatAlphaRef, GLMesh *, core::matrix4, core::matrix4, core::matrix4>
{};

class ListMatNormalMap : public MeshList<ListMatNormalMap, GLMesh *, core::matrix4, core::matrix4, core::matrix4>
{};

class ListMatGrass : public MeshList<ListMatGrass, GLMesh *, core::matrix4, core::matrix4, core::vector3df>
{};

class ListMatSphereMap : public MeshList<ListMatSphereMap, GLMesh *, core::matrix4, core::matrix4, core::matrix4>
{};

class ListMatSplatting : public MeshList<ListMatSplatting, GLMesh *, core::matrix4, core::matrix4>
{};

class ListMatUnlit : public MeshList<ListMatUnlit, GLMesh *, core::matrix4, core::matrix4, core::matrix4>
{};

class ListMatDetails : public MeshList<ListMatDetails, GLMesh *, core::matrix4, core::matrix4, core::matrix4>
{};

class ListBlendTransparent : public MeshList<ListBlendTransparent, GLMesh *, core::matrix4, core::matrix4>
{};

class ListAdditiveTransparent : public MeshList<ListAdditiveTransparent, GLMesh *, core::matrix4, core::matrix4>
{};

class ListBlendTransparentFog : public MeshList<ListBlendTransparentFog, GLMesh *, core::matrix4, core::matrix4, float, float, float, float, float, video::SColorf>
{};

class ListAdditiveTransparentFog : public MeshList<ListAdditiveTransparentFog, GLMesh *, core::matrix4, core::matrix4, float, float, float, float, float, video::SColorf>
{};

class ListDisplacement : public MeshList<ListDisplacement, GLMesh *, core::matrix4>
{};

// Forward pass (for transparents meshes)
void drawBubble(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);

MeshMaterial MaterialTypeToMeshMaterial(video::E_MATERIAL_TYPE, video::E_VERTEX_TYPE);
TransparentMaterial MaterialTypeToTransparentMaterial(video::E_MATERIAL_TYPE, f32 MaterialTypeParam);

void InitTextures(GLMesh &mesh, MeshMaterial);

#endif // STKMESH_H
