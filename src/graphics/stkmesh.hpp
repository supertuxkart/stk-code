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
    GLuint textures[6];
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
void computeMVP(core::matrix4 &ModelViewProjectionMatrix);
void computeTIMV(core::matrix4 &TransposeInverseModelView);
bool isObject(video::E_MATERIAL_TYPE type);

core::vector3df getWind();

// Pass 1 shader (ie shaders that outputs normals and depth)
void drawObjectPass1(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView);
void drawNormalPass(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView);
void drawObjectRefPass1(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, const core::matrix4 &TextureMatrix);
void drawGrassPass1(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, core::vector3df windDir);

// Pass 2 shader (ie shaders that outputs final color)
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
void drawShadowRef(const GLMesh &mesh);
void drawShadow(const GLMesh &mesh);

// Forward pass (for transparents meshes)
void drawTransparentObject(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TextureMatrix);
void drawTransparentFogObject(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix, const core::matrix4 &TextureMatrix);
void drawBubble(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix);

GeometricMaterial MaterialTypeToGeometricMaterial(video::E_MATERIAL_TYPE);
ShadedMaterial MaterialTypeToShadedMaterial(video::E_MATERIAL_TYPE, GLuint *textures);
TransparentMaterial MaterialTypeToTransparentMaterial(video::E_MATERIAL_TYPE);

#endif // STKMESH_H
