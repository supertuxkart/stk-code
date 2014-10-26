#ifndef STKMESH_H
#define STKMESH_H

#include "graphics/irr_driver.hpp"
#include "utils/tuple.hpp"

#include <IMeshSceneNode.h>
#include <IMesh.h>
#include "../lib/irrlicht/source/Irrlicht/CMeshSceneNode.h"

#include <vector>
#include "material.hpp"

class Material;

enum TransparentMaterial
{
    TM_DEFAULT,
    TM_ADDITIVE,
    TM_DISPLACEMENT,
    TM_COUNT
};

struct GLMesh {
    GLuint vao;
    GLuint vertex_buffer;
    GLuint index_buffer;
    video::ITexture *textures[8];
    GLenum PrimitiveType;
    GLenum IndexType;
    size_t IndexCount;
    size_t Stride;
    core::matrix4 TextureMatrix;
    size_t vaoBaseVertex;
    size_t vaoOffset;
    video::E_VERTEX_TYPE VAOType;
    uint64_t TextureHandles[6];
    scene::IMeshBuffer *mb;
#ifdef DEBUG
    std::string debug_name;
#endif
};

GLMesh allocateMeshBuffer(scene::IMeshBuffer* mb, const std::string& debug_name);
void fillLocalBuffer(GLMesh &, scene::IMeshBuffer* mb);
video::E_VERTEX_TYPE getVTXTYPEFromStride(size_t stride);
GLuint createVAO(GLuint vbo, GLuint idx, video::E_VERTEX_TYPE type);
core::matrix4 computeMVP(const core::matrix4 &ModelViewProjectionMatrix);
bool isObject(video::E_MATERIAL_TYPE type);

core::vector3df getWindDir();


class STKMeshCommon
{
protected:
    bool m_culledForPlayerCam;
    bool m_culledForShadowCam[4];
    bool m_culledForRSMCam;
    std::string m_debug_name;

public:
    PtrVector<GLMesh, REF> MeshSolidMaterial[Material::SHADERTYPE_COUNT];
    PtrVector<GLMesh, REF> TransparentMesh[TM_COUNT];
    virtual void updateNoGL() = 0;
    virtual void updateGL() = 0;
    virtual bool glow() const = 0;
    virtual bool isImmediateDraw() const { return false; }
    bool isCulledForPlayerCam() const { return m_culledForPlayerCam; }
    void setCulledForPlayerCam(bool v) { m_culledForPlayerCam = v; }
    bool isCulledForShadowCam(unsigned cascade) const { return m_culledForShadowCam[cascade]; }
    void setCulledForShadowCam(unsigned cascade, bool v) { m_culledForShadowCam[cascade] = v; }
    bool isCulledForRSMCam() const { return m_culledForRSMCam; }
    void setCulledForRSMCam(bool v) { m_culledForRSMCam = v; }
};

template<typename T, typename... Args>
class MeshList : public Singleton<T>
{
public:
    std::vector<STK::Tuple<Args...> > SolidPass, Shadows[4], RSM;
    void clear()
    {
        SolidPass.clear();
        RSM.clear();
        for (unsigned i = 0; i < 4; i++)
            Shadows[i].clear();
    }
};

template<typename T>
class InstancedMeshList : public Singleton<T>
{
public:
    std::vector<GLMesh *> SolidPass, Shadows[4], RSM;
    void clear()
    {
        SolidPass.clear();
        RSM.clear();
        for (unsigned i = 0; i < 4; i++)
            Shadows[i].clear();
    }
};

// -----------------------------------------Mat Default---------------------------------------------------- //
class ListMatDefault : public MeshList<ListMatDefault, GLMesh *, core::matrix4, core::matrix4, core::matrix4>
{};

class ListInstancedMatDefault : public InstancedMeshList<ListInstancedMatDefault>
{};


// -----------------------------------------Mat Alpha Ref---------------------------------------------------- //
class ListMatAlphaRef : public MeshList<ListMatAlphaRef, GLMesh *, core::matrix4, core::matrix4, core::matrix4>
{};

class ListInstancedMatAlphaRef : public InstancedMeshList<ListInstancedMatAlphaRef>
{};

// -----------------------------------------Mat Normap Map---------------------------------------------------- //
class ListMatNormalMap : public MeshList<ListMatNormalMap, GLMesh *, core::matrix4, core::matrix4, core::matrix4>
{};

class ListInstancedMatNormalMap : public InstancedMeshList<ListInstancedMatNormalMap>
{};

// -----------------------------------------Mat Grass---------------------------------------------------- //
class ListMatGrass : public MeshList<ListMatGrass, GLMesh *, core::matrix4, core::matrix4, core::vector3df>
{};

class ListInstancedMatGrass : public InstancedMeshList<ListInstancedMatGrass>
{};

// -----------------------------------------Mat Sphere Map---------------------------------------------------- //
class ListMatSphereMap : public MeshList<ListMatSphereMap, GLMesh *, core::matrix4, core::matrix4, core::matrix4>
{};

class ListInstancedMatSphereMap : public InstancedMeshList<ListInstancedMatSphereMap>
{};

// -----------------------------------------Mat Splatting---------------------------------------------------- //
class ListMatSplatting : public MeshList<ListMatSplatting, GLMesh *, core::matrix4, core::matrix4>
{};

// -----------------------------------------Mat Unlit---------------------------------------------------- //
class ListMatUnlit : public MeshList<ListMatUnlit, GLMesh *, core::matrix4, core::matrix4, core::matrix4>
{};

class ListInstancedMatUnlit : public InstancedMeshList<ListInstancedMatUnlit>
{};

// -----------------------------------------Mat Details---------------------------------------------------- //
class ListMatDetails : public MeshList<ListMatDetails, GLMesh *, core::matrix4, core::matrix4, core::matrix4>
{};

class ListInstancedMatDetails : public InstancedMeshList<ListInstancedMatDetails>
{};

// Transparent
template <typename T, typename ...Args>
class MiscList : public Singleton<T>, public std::vector<STK::Tuple<Args...> >
{};

class ListBlendTransparent : public MiscList<ListBlendTransparent, GLMesh *, core::matrix4, core::matrix4>
{};

class ListAdditiveTransparent : public MiscList<ListAdditiveTransparent, GLMesh *, core::matrix4, core::matrix4>
{};

class ListBlendTransparentFog : public MiscList<ListBlendTransparentFog, GLMesh *, core::matrix4, core::matrix4, float, float, float, float, float, video::SColorf>
{};

class ListAdditiveTransparentFog : public MiscList<ListAdditiveTransparentFog, GLMesh *, core::matrix4, core::matrix4, float, float, float, float, float, video::SColorf>
{};

class ListDisplacement : public MiscList<ListDisplacement, GLMesh *, core::matrix4>
{};

class ListInstancedGlow : public Singleton<ListInstancedGlow>, public std::vector<GLMesh *>
{};

Material::ShaderType MaterialTypeToMeshMaterial(video::E_MATERIAL_TYPE MaterialType, video::E_VERTEX_TYPE tp,
    Material* material, Material* layer2Material);
TransparentMaterial MaterialTypeToTransparentMaterial(video::E_MATERIAL_TYPE, f32 MaterialTypeParam, Material* material);

void InitTextures(GLMesh &mesh, Material::ShaderType);
void InitTexturesTransparent(GLMesh &mesh);

#endif // STKMESH_H
