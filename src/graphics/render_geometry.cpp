#include "graphics/irr_driver.hpp"

#include "config/user_config.hpp"
#include "graphics/callbacks.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/post_processing.hpp"
#include "graphics/rtts.hpp"
#include "graphics/shaders.hpp"
#include "modes/world.hpp"
#include "utils/log.hpp"
#include "utils/profiler.hpp"
#include "utils/tuple.hpp"
#include "stkscenemanager.hpp"
#include "utils/profiler.hpp"

#include <S3DVertex.h>

/**
\page render_geometry Geometry Rendering Overview

\section adding_material Adding a solid material

You need to consider twice before adding a new material : in the worst case a material requires 8 shaders :
one for each solid pass, one for shadow pass, one for RSM pass, and you need to double that for instanced version.

You need to declare a new enum in MeshMaterial and to write the corresponding dispatch code in MaterialTypeToMeshMaterial
and to create 2 new List* structures (one for standard and one for instanced version).

Then you need to write the code in stkscenemanager.cpp that will add any mesh with the new material to their corresponding
lists : in handleSTKCommon for the standard version and in the body of PrepareDrawCalls for instanced version.

\section vertex_layout Available Vertex Layout

There are 3 different layout that comes from Irrlicht loading routines :
EVT_STANDARD, EVT_2TCOORDS, EVT_TANGENT.

Below are the attributes for each vertex layout and their predefined location.

\subsection EVT_STANDARD
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 Texcoord;

\subsection EVT_2TCOORDS
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 Texcoord;
layout(location = 4) in vec2 SecondTexcoord;

\subsection EVT_TANGENT
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 Texcoord;
layout(location = 5) in vec3 Tangent;
layout(location = 6) in vec3 Bitangent;

*/

struct DefaultMaterial
{
    typedef MeshShader::InstancedObjectPass1Shader InstancedFirstPassShader;
    typedef MeshShader::InstancedObjectPass2Shader InstancedSecondPassShader;
    typedef MeshShader::InstancedShadowShader InstancedShadowPassShader;
    typedef MeshShader::InstancedRSMShader InstancedRSMShader;
    typedef ListInstancedMatDefault InstancedList;
    typedef MeshShader::ObjectPass1Shader FirstPassShader;
    typedef MeshShader::ObjectPass2Shader SecondPassShader;
    typedef MeshShader::ShadowShader ShadowPassShader;
    typedef MeshShader::RSMShader RSMShader;
    typedef ListMatDefault List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_STANDARD;
    static const enum Material::ShaderType MaterialType = Material::SHADERTYPE_SOLID;
    static const enum InstanceType Instance = InstanceTypeDualTex;
    static const STK::Tuple<size_t> FirstPassTextures;
    static const STK::Tuple<size_t, size_t> SecondPassTextures;
    static const STK::Tuple<> ShadowTextures;
    static const STK::Tuple<size_t> RSMTextures;
};

const STK::Tuple<size_t> DefaultMaterial::FirstPassTextures = STK::Tuple<size_t>(1);
const STK::Tuple<size_t, size_t> DefaultMaterial::SecondPassTextures = STK::Tuple<size_t, size_t>(0, 1);
const STK::Tuple<> DefaultMaterial::ShadowTextures;
const STK::Tuple<size_t> DefaultMaterial::RSMTextures = STK::Tuple<size_t>(0);

struct AlphaRef
{
    typedef MeshShader::InstancedObjectRefPass1Shader InstancedFirstPassShader;
    typedef MeshShader::InstancedObjectRefPass2Shader InstancedSecondPassShader;
    typedef MeshShader::InstancedRefShadowShader InstancedShadowPassShader;
    typedef MeshShader::InstancedRSMShader InstancedRSMShader;
    typedef ListInstancedMatAlphaRef InstancedList;
    typedef MeshShader::ObjectRefPass1Shader FirstPassShader;
    typedef MeshShader::ObjectRefPass2Shader SecondPassShader;
    typedef MeshShader::RefShadowShader ShadowPassShader;
    typedef MeshShader::RSMShader RSMShader;
    typedef ListMatAlphaRef List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_STANDARD;
    static const enum Material::ShaderType MaterialType = Material::SHADERTYPE_ALPHA_TEST;
    static const enum InstanceType Instance = InstanceTypeDualTex;
    static const STK::Tuple<size_t, size_t> FirstPassTextures;
    static const STK::Tuple<size_t, size_t> SecondPassTextures;
    static const STK::Tuple<size_t> ShadowTextures;
    static const STK::Tuple<size_t> RSMTextures;
};

const STK::Tuple<size_t, size_t> AlphaRef::FirstPassTextures = STK::Tuple<size_t, size_t>(0, 1);
const STK::Tuple<size_t, size_t> AlphaRef::SecondPassTextures = STK::Tuple<size_t, size_t>(0, 1);
const STK::Tuple<size_t> AlphaRef::ShadowTextures = STK::Tuple<size_t>(0);
const STK::Tuple<size_t> AlphaRef::RSMTextures = STK::Tuple<size_t>(0);

struct SphereMap
{
    typedef MeshShader::InstancedObjectPass1Shader InstancedFirstPassShader;
    typedef MeshShader::InstancedSphereMapShader InstancedSecondPassShader;
    typedef MeshShader::InstancedShadowShader InstancedShadowPassShader;
    typedef MeshShader::InstancedRSMShader InstancedRSMShader;
    typedef ListInstancedMatSphereMap InstancedList;
    typedef MeshShader::ObjectPass1Shader FirstPassShader;
    typedef MeshShader::SphereMapShader SecondPassShader;
    typedef MeshShader::ShadowShader ShadowPassShader;
    typedef MeshShader::RSMShader RSMShader;
    typedef ListMatSphereMap List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_STANDARD;
    static const enum Material::ShaderType MaterialType = Material::SHADERTYPE_SPHERE_MAP;
    static const enum InstanceType Instance = InstanceTypeDualTex;
    static const STK::Tuple<size_t> FirstPassTextures;
    static const STK::Tuple<size_t> SecondPassTextures;
    static const STK::Tuple<> ShadowTextures;
    static const STK::Tuple<size_t> RSMTextures;
};

const STK::Tuple<size_t> SphereMap::FirstPassTextures = STK::Tuple<size_t>(1);
const STK::Tuple<size_t> SphereMap::SecondPassTextures = STK::Tuple<size_t>(0);
const STK::Tuple<> SphereMap::ShadowTextures;
const STK::Tuple<size_t> SphereMap::RSMTextures = STK::Tuple<size_t>(0);

struct UnlitMat
{
    typedef MeshShader::InstancedObjectRefPass1Shader InstancedFirstPassShader;
    typedef MeshShader::InstancedObjectUnlitShader InstancedSecondPassShader;
    typedef MeshShader::InstancedRefShadowShader InstancedShadowPassShader;
    typedef MeshShader::InstancedRSMShader InstancedRSMShader;
    typedef ListInstancedMatUnlit InstancedList;
    typedef MeshShader::ObjectRefPass1Shader FirstPassShader;
    typedef MeshShader::ObjectUnlitShader SecondPassShader;
    typedef MeshShader::RefShadowShader ShadowPassShader;
    typedef MeshShader::RSMShader RSMShader;
    typedef ListMatUnlit List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_STANDARD;
    static const enum Material::ShaderType MaterialType = Material::SHADERTYPE_SOLID_UNLIT;
    static const enum InstanceType Instance = InstanceTypeDualTex;
    static const STK::Tuple<size_t, size_t> FirstPassTextures;
    static const STK::Tuple<size_t> SecondPassTextures;
    static const STK::Tuple<size_t> ShadowTextures;
    static const STK::Tuple<size_t> RSMTextures;
};

const STK::Tuple<size_t, size_t> UnlitMat::FirstPassTextures = STK::Tuple<size_t, size_t>(0, 1);
const STK::Tuple<size_t> UnlitMat::SecondPassTextures = STK::Tuple<size_t>(0);
const STK::Tuple<size_t> UnlitMat::ShadowTextures = STK::Tuple<size_t>(0);
const STK::Tuple<size_t> UnlitMat::RSMTextures = STK::Tuple<size_t>(0);

struct GrassMat
{
    typedef MeshShader::InstancedGrassPass1Shader InstancedFirstPassShader;
    typedef MeshShader::InstancedGrassPass2Shader InstancedSecondPassShader;
    typedef MeshShader::InstancedGrassShadowShader InstancedShadowPassShader;
    typedef MeshShader::InstancedRSMShader InstancedRSMShader;
    typedef ListInstancedMatGrass InstancedList;
    typedef MeshShader::GrassPass1Shader FirstPassShader;
    typedef MeshShader::GrassPass2Shader SecondPassShader;
    typedef MeshShader::GrassShadowShader ShadowPassShader;
    typedef MeshShader::RSMShader RSMShader;
    typedef ListMatGrass List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_STANDARD;
    static const enum Material::ShaderType MaterialType = Material::SHADERTYPE_VEGETATION;
    static const enum InstanceType Instance = InstanceTypeDualTex;
    static const STK::Tuple<size_t, size_t> FirstPassTextures;
    static const STK::Tuple<size_t, size_t> SecondPassTextures;
    static const STK::Tuple<size_t> ShadowTextures;
    static const STK::Tuple<size_t> RSMTextures;
};

const STK::Tuple<size_t, size_t> GrassMat::FirstPassTextures = STK::Tuple<size_t, size_t>(0, 1);
const STK::Tuple<size_t, size_t> GrassMat::SecondPassTextures = STK::Tuple<size_t, size_t>(0, 1);
const STK::Tuple<size_t> GrassMat::ShadowTextures = STK::Tuple<size_t>(0);
const STK::Tuple<size_t> GrassMat::RSMTextures = STK::Tuple<size_t>(0);

struct NormalMat
{
    typedef MeshShader::InstancedNormalMapShader InstancedFirstPassShader;
    typedef MeshShader::InstancedObjectPass2Shader InstancedSecondPassShader;
    typedef MeshShader::InstancedShadowShader InstancedShadowPassShader;
    typedef MeshShader::InstancedRSMShader InstancedRSMShader;
    typedef ListInstancedMatNormalMap InstancedList;
    typedef MeshShader::NormalMapShader FirstPassShader;
    typedef MeshShader::ObjectPass2Shader SecondPassShader;
    typedef MeshShader::ShadowShader ShadowPassShader;
    typedef MeshShader::RSMShader RSMShader;
    typedef ListMatNormalMap List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_TANGENTS;
    static const enum Material::ShaderType MaterialType = Material::SHADERTYPE_NORMAL_MAP;
    static const enum InstanceType Instance = InstanceTypeThreeTex;
    static const STK::Tuple<size_t, size_t> FirstPassTextures;
    static const STK::Tuple<size_t, size_t> SecondPassTextures;
    static const STK::Tuple<> ShadowTextures;
    static const STK::Tuple<size_t> RSMTextures;
};

const STK::Tuple<size_t, size_t> NormalMat::FirstPassTextures = STK::Tuple<size_t, size_t>(2, 1);
const STK::Tuple<size_t, size_t> NormalMat::SecondPassTextures = STK::Tuple<size_t, size_t>(0, 1);
const STK::Tuple<> NormalMat::ShadowTextures;
const STK::Tuple<size_t> NormalMat::RSMTextures = STK::Tuple<size_t>(0);

struct DetailMat
{
    typedef MeshShader::InstancedObjectPass1Shader InstancedFirstPassShader;
    typedef MeshShader::InstancedDetailledObjectPass2Shader InstancedSecondPassShader;
    typedef MeshShader::InstancedShadowShader InstancedShadowPassShader;
    typedef MeshShader::InstancedRSMShader InstancedRSMShader;
    typedef ListInstancedMatDetails InstancedList;
    typedef MeshShader::ObjectPass1Shader FirstPassShader;
    typedef MeshShader::DetailledObjectPass2Shader SecondPassShader;
    typedef MeshShader::ShadowShader ShadowPassShader;
    typedef MeshShader::RSMShader RSMShader;
    typedef ListMatDetails List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_2TCOORDS;
    static const enum Material::ShaderType MaterialType = Material::SHADERTYPE_DETAIL_MAP;
    static const enum InstanceType Instance = InstanceTypeThreeTex;
    static const STK::Tuple<size_t> FirstPassTextures;
    static const STK::Tuple<size_t, size_t, size_t> SecondPassTextures;
    static const STK::Tuple<> ShadowTextures;
    static const STK::Tuple<size_t> RSMTextures;
};

const STK::Tuple<size_t> DetailMat::FirstPassTextures = STK::Tuple<size_t>(1);
const STK::Tuple<size_t, size_t, size_t> DetailMat::SecondPassTextures = STK::Tuple<size_t, size_t, size_t>(0, 2, 1);
const STK::Tuple<> DetailMat::ShadowTextures;
const STK::Tuple<size_t> DetailMat::RSMTextures = STK::Tuple<size_t>(0);

struct SplattingMat
{
    typedef MeshShader::ObjectPass1Shader FirstPassShader;
    typedef MeshShader::SplattingShader SecondPassShader;
    typedef MeshShader::ShadowShader ShadowPassShader;
    typedef MeshShader::SplattingRSMShader RSMShader;
    typedef ListMatSplatting List;
    static const enum video::E_VERTEX_TYPE VertexType = video::EVT_2TCOORDS;
    static const STK::Tuple<size_t> FirstPassTextures;
    static const STK::Tuple<size_t, size_t, size_t, size_t, size_t> SecondPassTextures;
    static const STK::Tuple<> ShadowTextures;
    static const STK::Tuple<size_t, size_t, size_t, size_t, size_t> RSMTextures;
};

const STK::Tuple<size_t> SplattingMat::FirstPassTextures = STK::Tuple<size_t>(6);
const STK::Tuple<size_t, size_t, size_t, size_t, size_t> SplattingMat::SecondPassTextures = STK::Tuple<size_t, size_t, size_t, size_t, size_t>(1, 2, 3, 4, 5);
const STK::Tuple<> SplattingMat::ShadowTextures;
const STK::Tuple<size_t, size_t, size_t, size_t, size_t> SplattingMat::RSMTextures = STK::Tuple<size_t, size_t, size_t, size_t, size_t>(1, 2, 3, 4, 5);

namespace RenderGeometry
{
    struct TexUnit
    {
        GLuint m_id;
        bool m_premul_alpha;

        TexUnit(GLuint id, bool premul_alpha)
        {
            m_id = id;
            m_premul_alpha = premul_alpha;
        }
    };

    template <typename T>
    std::vector<TexUnit> TexUnits(T curr) // required on older clang versions
    {
        std::vector<TexUnit> v;
        v.push_back(curr);
        return v;
    }

    template <typename T, typename... R>
    std::vector<TexUnit> TexUnits(T curr, R... rest) // required on older clang versions
    {
        std::vector<TexUnit> v;
        v.push_back(curr);
        VTexUnits(v, rest...);
        return v;
    }

    template <typename T, typename... R>
    void VTexUnits(std::vector<TexUnit>& v, T curr, R... rest) // required on older clang versions
    {
        v.push_back(curr);
        VTexUnits(v, rest...);
    }

    template <typename T>
    void VTexUnits(std::vector<TexUnit>& v, T curr)
    {
        v.push_back(curr);
    }
}
using namespace RenderGeometry;


template<typename T, typename...uniforms>
void draw(const T *Shader, const GLMesh *mesh, uniforms... Args)
{
    irr_driver->IncreaseObjectCount();
    GLenum ptype = mesh->PrimitiveType;
    GLenum itype = mesh->IndexType;
    size_t count = mesh->IndexCount;

    Shader->setUniforms(Args...);
    glDrawElementsBaseVertex(ptype, (int)count, itype, (GLvoid *)mesh->vaoOffset, (int)mesh->vaoBaseVertex);
}

template<int...List>
struct custom_unroll_args;

template<>
struct custom_unroll_args<>
{
    template<typename T, typename ...TupleTypes, typename ...Args>
    static void exec(const T *Shader, const STK::Tuple<TupleTypes...> &t, Args... args)
    {
        draw<T>(Shader, STK::tuple_get<0>(t), args...);
    }
};

template<int N, int...List>
struct custom_unroll_args<N, List...>
{
    template<typename T, typename ...TupleTypes, typename ...Args>
    static void exec(const T *Shader, const STK::Tuple<TupleTypes...> &t, Args... args)
    {
        custom_unroll_args<List...>::template exec<T>(Shader, t, STK::tuple_get<N>(t), args...);
    }
};

template<typename T, int N>
struct TexExpander_impl
{
    template<typename...TupleArgs, typename... Args>
    static void ExpandTex(GLMesh &mesh, const STK::Tuple<TupleArgs...> &TexSwizzle, Args... args)
    {
        size_t idx = STK::tuple_get<sizeof...(TupleArgs) - N>(TexSwizzle);
        TexExpander_impl<T, N - 1>::template ExpandTex(mesh, TexSwizzle, args..., getTextureGLuint(mesh.textures[idx]));
    }
};

template<typename T>
struct TexExpander_impl<T, 0>
{
    template<typename...TupleArgs, typename... Args>
    static void ExpandTex(GLMesh &mesh, const STK::Tuple<TupleArgs...> &TexSwizzle, Args... args)
    {
        T::getInstance()->SetTextureUnits(args...);
    }
};

template<typename T>
struct TexExpander
{
    template<typename...TupleArgs, typename... Args>
    static void ExpandTex(GLMesh &mesh, const STK::Tuple<TupleArgs...> &TexSwizzle, Args... args)
    {
        TexExpander_impl<T, sizeof...(TupleArgs)>::ExpandTex(mesh, TexSwizzle, args...);
    }
};


template<typename T, int N>
struct HandleExpander_impl
{
    template<typename...TupleArgs, typename... Args>
    static void Expand(uint64_t *TextureHandles, const STK::Tuple<TupleArgs...> &TexSwizzle, Args... args)
    {
        size_t idx = STK::tuple_get<sizeof...(TupleArgs)-N>(TexSwizzle);
        HandleExpander_impl<T, N - 1>::template Expand(TextureHandles, TexSwizzle, args..., TextureHandles[idx]);
    }
};

template<typename T>
struct HandleExpander_impl<T, 0>
{
    template<typename...TupleArgs, typename... Args>
    static void Expand(uint64_t *TextureHandles, const STK::Tuple<TupleArgs...> &TexSwizzle, Args... args)
    {
        T::getInstance()->SetTextureHandles(args...);
    }
};

template<typename T>
struct HandleExpander
{
    template<typename...TupleArgs, typename... Args>
    static void Expand(uint64_t *TextureHandles, const STK::Tuple<TupleArgs...> &TexSwizzle, Args... args)
    {
        HandleExpander_impl<T, sizeof...(TupleArgs)>::Expand(TextureHandles, TexSwizzle, args...);
    }
};

template<typename T, int ...List>
void renderMeshes1stPass()
{
    auto &meshes = T::List::getInstance()->SolidPass;
    glUseProgram(T::FirstPassShader::getInstance()->Program);
    if (irr_driver->hasARB_base_instance())
        glBindVertexArray(VAOManager::getInstance()->getVAO(T::VertexType));
    for (unsigned i = 0; i < meshes.size(); i++)
    {
        std::vector<GLuint> Textures;
        std::vector<uint64_t> Handles;
        GLMesh &mesh = *(STK::tuple_get<0>(meshes.at(i)));
        if (!irr_driver->hasARB_base_instance())
            glBindVertexArray(mesh.vao);
        if (mesh.VAOType != T::VertexType)
        {
#ifdef DEBUG
            Log::error("Materials", "Wrong vertex Type associed to pass 1 (hint texture : %s)", mesh.textures[0]->getName().getPath().c_str());
#endif
            continue;
        }

        if (UserConfigParams::m_azdo)
            HandleExpander<typename T::FirstPassShader>::template Expand(mesh.TextureHandles, T::FirstPassTextures);
        else
            TexExpander<typename T::FirstPassShader>::template ExpandTex(mesh, T::FirstPassTextures);
        custom_unroll_args<List...>::template exec(T::FirstPassShader::getInstance(), meshes.at(i));
    }
}

template<typename T, typename...Args>
void renderInstancedMeshes1stPass(Args...args)
{
    std::vector<GLMesh *> &meshes = T::InstancedList::getInstance()->SolidPass;
    glUseProgram(T::InstancedFirstPassShader::getInstance()->Program);
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType, T::Instance));
    for (unsigned i = 0; i < meshes.size(); i++)
    {
        std::vector<GLuint> Textures;
        GLMesh *mesh = meshes[i];
#ifdef DEBUG
        if (mesh->VAOType != T::VertexType)
            Log::error("RenderGeometry", "Wrong instanced vertex format (hint : %s)", 
                        mesh->textures[0]->getName().getPath().c_str());
#endif
        TexExpander<typename T::InstancedFirstPassShader>::template ExpandTex(*mesh, T::FirstPassTextures);

        T::InstancedFirstPassShader::getInstance()->setUniforms(args...);
        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (const void*)((SolidPassCmd::getInstance()->Offset[T::MaterialType] + i) * sizeof(DrawElementsIndirectCommand)));
    }
}

template<typename T, typename...Args>
void multidraw1stPass(Args...args)
{
    glUseProgram(T::InstancedFirstPassShader::getInstance()->Program);
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType, T::Instance));
    if (SolidPassCmd::getInstance()->Size[T::MaterialType])
    {
        T::InstancedFirstPassShader::getInstance()->setUniforms(args...);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT,
            (const void*)(SolidPassCmd::getInstance()->Offset[T::MaterialType] * sizeof(DrawElementsIndirectCommand)),
            (int)SolidPassCmd::getInstance()->Size[T::MaterialType],
            sizeof(DrawElementsIndirectCommand));
    }
}

static core::vector3df windDir;

void IrrDriver::renderSolidFirstPass()
{
    windDir = getWindDir();

    if (irr_driver->hasARB_draw_indirect())
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, SolidPassCmd::getInstance()->drawindirectcmd);

    {
        ScopedGPUTimer Timer(getGPUTimer(Q_SOLID_PASS1));
        irr_driver->setPhase(SOLID_NORMAL_AND_DEPTH_PASS);

        for (unsigned i = 0; i < ImmediateDrawList::getInstance()->size(); i++)
            ImmediateDrawList::getInstance()->at(i)->render();

        renderMeshes1stPass<DefaultMaterial, 2, 1>();
        renderMeshes1stPass<SplattingMat, 2, 1>();
        renderMeshes1stPass<UnlitMat, 3, 2, 1>();
        renderMeshes1stPass<AlphaRef, 3, 2, 1>();
        renderMeshes1stPass<GrassMat, 3, 2, 1>();
        renderMeshes1stPass<NormalMat, 2, 1>();
        renderMeshes1stPass<SphereMap, 2, 1>();
        renderMeshes1stPass<DetailMat, 2, 1>();

        if (UserConfigParams::m_azdo)
        {
            multidraw1stPass<DefaultMaterial>();
            multidraw1stPass<AlphaRef>();
            multidraw1stPass<SphereMap>();
            multidraw1stPass<UnlitMat>();
            multidraw1stPass<GrassMat>(windDir);

            multidraw1stPass<NormalMat>();
            multidraw1stPass<DetailMat>();
        }
        else if (irr_driver->hasARB_draw_indirect())
        {
            renderInstancedMeshes1stPass<DefaultMaterial>();
            renderInstancedMeshes1stPass<AlphaRef>();
            renderInstancedMeshes1stPass<UnlitMat>();
            renderInstancedMeshes1stPass<SphereMap>();
            renderInstancedMeshes1stPass<GrassMat>(windDir);
            renderInstancedMeshes1stPass<DetailMat>();
            renderInstancedMeshes1stPass<NormalMat>();
        }
    }
}

template<typename T, int...List>
void renderMeshes2ndPass( const std::vector<uint64_t> &Prefilled_Handle,
    const std::vector<GLuint> &Prefilled_Tex)
{
    auto &meshes = T::List::getInstance()->SolidPass;
    glUseProgram(T::SecondPassShader::getInstance()->Program);
    if (irr_driver->hasARB_base_instance())
        glBindVertexArray(VAOManager::getInstance()->getVAO(T::VertexType));
    for (unsigned i = 0; i < meshes.size(); i++)
    {
        GLMesh &mesh = *(STK::tuple_get<0>(meshes.at(i)));
        if (!irr_driver->hasARB_base_instance())
            glBindVertexArray(mesh.vao);

        if (mesh.VAOType != T::VertexType)
        {
#ifdef DEBUG
            Log::error("Materials", "Wrong vertex Type associed to pass 2 (hint texture : %s)", mesh.textures[0]->getName().getPath().c_str());
#endif
            continue;
        }

        if (UserConfigParams::m_azdo)
            HandleExpander<typename T::SecondPassShader>::template Expand(mesh.TextureHandles, T::SecondPassTextures, Prefilled_Handle[0], Prefilled_Handle[1], Prefilled_Handle[2]);
        else
            TexExpander<typename T::SecondPassShader>::template ExpandTex(mesh, T::SecondPassTextures, Prefilled_Tex[0], Prefilled_Tex[1], Prefilled_Tex[2]);
        custom_unroll_args<List...>::template exec(T::SecondPassShader::getInstance(), meshes.at(i));
    }
}

template<typename T, typename...Args>
void renderInstancedMeshes2ndPass(const std::vector<GLuint> &Prefilled_tex, Args...args)
{
    std::vector<GLMesh *> &meshes = T::InstancedList::getInstance()->SolidPass;
    glUseProgram(T::InstancedSecondPassShader::getInstance()->Program);
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType, T::Instance));
    for (unsigned i = 0; i < meshes.size(); i++)
    {
        GLMesh *mesh = meshes[i];
        TexExpander<typename T::InstancedSecondPassShader>::template ExpandTex(*mesh, T::SecondPassTextures, Prefilled_tex[0], Prefilled_tex[1], Prefilled_tex[2]);
        T::InstancedSecondPassShader::getInstance()->setUniforms(args...);
        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (const void*)((SolidPassCmd::getInstance()->Offset[T::MaterialType] + i) * sizeof(DrawElementsIndirectCommand)));
    }
}


template<typename T, typename...Args>
void multidraw2ndPass(const std::vector<uint64_t> &Handles, Args... args)
{
    glUseProgram(T::InstancedSecondPassShader::getInstance()->Program);
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType, T::Instance));
    uint64_t nulltex[10] = {};
    if (SolidPassCmd::getInstance()->Size[T::MaterialType])
    {
        HandleExpander<typename T::InstancedSecondPassShader>::template Expand(nulltex, T::SecondPassTextures, Handles[0], Handles[1], Handles[2]);
        T::InstancedSecondPassShader::getInstance()->setUniforms(args...);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT,
            (const void*)(SolidPassCmd::getInstance()->Offset[T::MaterialType] * sizeof(DrawElementsIndirectCommand)),
            (int)SolidPassCmd::getInstance()->Size[T::MaterialType],
            (int)sizeof(DrawElementsIndirectCommand));
    }
}

void IrrDriver::renderSolidSecondPass()
{
    irr_driver->setPhase(SOLID_LIT_PASS);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    uint64_t DiffuseHandle = 0, SpecularHandle = 0, SSAOHandle = 0, DepthHandle = 0;

    if (UserConfigParams::m_azdo)
    {
        DiffuseHandle = glGetTextureSamplerHandleARB(m_rtts->getRenderTarget(RTT_DIFFUSE), MeshShader::ObjectPass2Shader::getInstance()->SamplersId[0]);
        if (!glIsTextureHandleResidentARB(DiffuseHandle))
            glMakeTextureHandleResidentARB(DiffuseHandle);

        SpecularHandle = glGetTextureSamplerHandleARB(m_rtts->getRenderTarget(RTT_SPECULAR), MeshShader::ObjectPass2Shader::getInstance()->SamplersId[1]);
        if (!glIsTextureHandleResidentARB(SpecularHandle))
            glMakeTextureHandleResidentARB(SpecularHandle);

        SSAOHandle = glGetTextureSamplerHandleARB(m_rtts->getRenderTarget(RTT_HALF1_R), MeshShader::ObjectPass2Shader::getInstance()->SamplersId[2]);
        if (!glIsTextureHandleResidentARB(SSAOHandle))
            glMakeTextureHandleResidentARB(SSAOHandle);

        DepthHandle = glGetTextureSamplerHandleARB(getDepthStencilTexture(), MeshShader::ObjectPass2Shader::getInstance()->SamplersId[3]);
        if (!glIsTextureHandleResidentARB(DepthHandle))
            glMakeTextureHandleResidentARB(DepthHandle);
    }

    {
        ScopedGPUTimer Timer(getGPUTimer(Q_SOLID_PASS2));

        irr_driver->setPhase(SOLID_LIT_PASS);

        for (unsigned i = 0; i < ImmediateDrawList::getInstance()->size(); i++)
            ImmediateDrawList::getInstance()->at(i)->render();

        std::vector<GLuint> DiffSpecSSAOTex = createVector<GLuint>(m_rtts->getRenderTarget(RTT_DIFFUSE), m_rtts->getRenderTarget(RTT_SPECULAR), m_rtts->getRenderTarget(RTT_HALF1_R));

        renderMeshes2ndPass<DefaultMaterial, 3, 1>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<AlphaRef, 3, 1 >(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<UnlitMat, 3, 1>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<SplattingMat, 1>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<SphereMap, 2, 1>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<DetailMat, 1>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<GrassMat, 3, 1>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<NormalMat, 3, 1>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);

        if (UserConfigParams::m_azdo)
        {
            multidraw2ndPass<DefaultMaterial>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle, 0, 0));
            multidraw2ndPass<AlphaRef>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle, 0, 0));
            multidraw2ndPass<SphereMap>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle, 0));
            multidraw2ndPass<UnlitMat>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle, 0));
            multidraw2ndPass<NormalMat>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle, 0, 0));
            multidraw2ndPass<DetailMat>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle, 0, 0, 0));

            // template does not work with template due to extra depth texture
            {
                SunLightProvider * const cb = (SunLightProvider *)irr_driver->getCallback(ES_SUNLIGHT);
                glUseProgram(GrassMat::InstancedSecondPassShader::getInstance()->Program);
                glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(GrassMat::VertexType, GrassMat::Instance));
                uint64_t nulltex[10] = {};
                if (SolidPassCmd::getInstance()->Size[GrassMat::MaterialType])
                {
                    HandleExpander<GrassMat::InstancedSecondPassShader>::Expand(nulltex, GrassMat::SecondPassTextures, DiffuseHandle, SpecularHandle, SSAOHandle, DepthHandle);
                    GrassMat::InstancedSecondPassShader::getInstance()->setUniforms(windDir, cb->getPosition());
                    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT,
                        (const void*)(SolidPassCmd::getInstance()->Offset[GrassMat::MaterialType] * sizeof(DrawElementsIndirectCommand)),
                        (int)SolidPassCmd::getInstance()->Size[GrassMat::MaterialType],
                        (int)sizeof(DrawElementsIndirectCommand));
                }
            }
        }
        else if (irr_driver->hasARB_draw_indirect())
        {
            renderInstancedMeshes2ndPass<DefaultMaterial>(DiffSpecSSAOTex);
            renderInstancedMeshes2ndPass<AlphaRef>(DiffSpecSSAOTex);
            renderInstancedMeshes2ndPass<UnlitMat>(DiffSpecSSAOTex);
            renderInstancedMeshes2ndPass<SphereMap>(DiffSpecSSAOTex);
            renderInstancedMeshes2ndPass<DetailMat>(DiffSpecSSAOTex);
            renderInstancedMeshes2ndPass<NormalMat>(DiffSpecSSAOTex);

            // template does not work with template due to extra depth texture
            {
                SunLightProvider * const cb = (SunLightProvider *)irr_driver->getCallback(ES_SUNLIGHT);
                std::vector<GLMesh *> &meshes = GrassMat::InstancedList::getInstance()->SolidPass;
                glUseProgram(GrassMat::InstancedSecondPassShader::getInstance()->Program);
                glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(GrassMat::VertexType, GrassMat::Instance));
                for (unsigned i = 0; i < meshes.size(); i++)
                {
                    GLMesh *mesh = meshes[i];
                    TexExpander<GrassMat::InstancedSecondPassShader>::ExpandTex(*mesh, GrassMat::SecondPassTextures, DiffSpecSSAOTex[0], DiffSpecSSAOTex[1], DiffSpecSSAOTex[2], irr_driver->getDepthStencilTexture());
                    GrassMat::InstancedSecondPassShader::getInstance()->setUniforms(windDir, cb->getPosition());
                    glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (const void*)((SolidPassCmd::getInstance()->Offset[GrassMat::MaterialType] + i) * sizeof(DrawElementsIndirectCommand)));
                }
            }
        }
    }
}

template<typename T>
static void renderInstancedMeshNormals()
{
    std::vector<GLMesh *> &meshes = T::InstancedList::getInstance()->SolidPass;
    glUseProgram(MeshShader::NormalVisualizer::getInstance()->Program);
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType, T::Instance));
    for (unsigned i = 0; i < meshes.size(); i++)
    {
        GLMesh *mesh = meshes[i];
        MeshShader::NormalVisualizer::getInstance()->setUniforms(video::SColor(255, 0, 255, 0));
        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (const void*)((SolidPassCmd::getInstance()->Offset[T::MaterialType] + i) * sizeof(DrawElementsIndirectCommand)));
    }
}

template<typename T>
static void renderMultiMeshNormals()
{
    glUseProgram(MeshShader::NormalVisualizer::getInstance()->Program);
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType, T::Instance));
    if (SolidPassCmd::getInstance()->Size[T::MaterialType])
    {
        MeshShader::NormalVisualizer::getInstance()->setUniforms(video::SColor(255, 0, 255, 0));
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT,
            (const void*)(SolidPassCmd::getInstance()->Offset[T::MaterialType] * sizeof(DrawElementsIndirectCommand)),
            (int)SolidPassCmd::getInstance()->Size[T::MaterialType],
            (int)sizeof(DrawElementsIndirectCommand));
    }
}

void IrrDriver::renderNormalsVisualisation()
{
    if (UserConfigParams::m_azdo) {
        renderMultiMeshNormals<DefaultMaterial>();
        renderMultiMeshNormals<AlphaRef>();
        renderMultiMeshNormals<UnlitMat>();
        renderMultiMeshNormals<SphereMap>();
        renderMultiMeshNormals<DetailMat>();
        renderMultiMeshNormals<NormalMat>();
    }
    else if (irr_driver->hasARB_draw_indirect())
    {
        renderInstancedMeshNormals<DefaultMaterial>();
        renderInstancedMeshNormals<AlphaRef>();
        renderInstancedMeshNormals<UnlitMat>();
        renderInstancedMeshNormals<SphereMap>();
        renderInstancedMeshNormals<DetailMat>();
        renderInstancedMeshNormals<NormalMat>();
    }
}

template<typename Shader, enum video::E_VERTEX_TYPE VertexType, int...List, typename... TupleType>
void renderTransparenPass(const std::vector<TexUnit> &TexUnits, std::vector<STK::Tuple<TupleType...> > *meshes)
{
    glUseProgram(Shader::getInstance()->Program);
    if (irr_driver->hasARB_base_instance())
        glBindVertexArray(VAOManager::getInstance()->getVAO(VertexType));
    for (unsigned i = 0; i < meshes->size(); i++)
    {
        GLMesh &mesh = *(STK::tuple_get<0>(meshes->at(i)));
        if (!irr_driver->hasARB_base_instance())
            glBindVertexArray(mesh.vao);
        if (mesh.VAOType != VertexType)
        {
#ifdef DEBUG
            Log::error("Materials", "Wrong vertex Type associed to pass 2 (hint texture : %s)", mesh.textures[0]->getName().getPath().c_str());
#endif
            continue;
        }

        if (UserConfigParams::m_azdo)
            Shader::getInstance()->SetTextureHandles(mesh.TextureHandles[0]);
        else
            Shader::getInstance()->SetTextureUnits(getTextureGLuint(mesh.textures[0]));
        custom_unroll_args<List...>::template exec(Shader::getInstance(), meshes->at(i));
    }
}

static video::ITexture *displaceTex = 0;

void IrrDriver::renderTransparent()
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glDisable(GL_CULL_FACE);

    irr_driver->setPhase(TRANSPARENT_PASS);

    for (unsigned i = 0; i < ImmediateDrawList::getInstance()->size(); i++)
        ImmediateDrawList::getInstance()->at(i)->render();

    if (irr_driver->hasARB_base_instance())
        glBindVertexArray(VAOManager::getInstance()->getVAO(video::EVT_STANDARD));

    if (World::getWorld() && World::getWorld()->isFogEnabled())
    {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        renderTransparenPass<MeshShader::TransparentFogShader, video::EVT_STANDARD, 8, 7, 6, 5, 4, 3, 2, 1>(TexUnits(
            TexUnit(0, true)), ListBlendTransparentFog::getInstance());
        glBlendFunc(GL_ONE, GL_ONE);
        renderTransparenPass<MeshShader::TransparentFogShader, video::EVT_STANDARD, 8, 7, 6, 5, 4, 3, 2, 1>(TexUnits(
            TexUnit(0, true)), ListAdditiveTransparentFog::getInstance());
    }
    else
    {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        renderTransparenPass<MeshShader::TransparentShader, video::EVT_STANDARD, 2, 1>(TexUnits(
            TexUnit(0, true)), ListBlendTransparent::getInstance());
        glBlendFunc(GL_ONE, GL_ONE);
        renderTransparenPass<MeshShader::TransparentShader, video::EVT_STANDARD, 2, 1>(TexUnits(
            TexUnit(0, true)), ListAdditiveTransparent::getInstance());
    }

    for (unsigned i = 0; i < BillBoardList::getInstance()->size(); i++)
        BillBoardList::getInstance()->at(i)->render();

    if (!UserConfigParams::m_dynamic_lights)
        return;

    // Render displacement nodes
    irr_driver->getFBO(FBO_TMP1_WITH_DS).Bind();
    glClear(GL_COLOR_BUFFER_BIT);
    irr_driver->getFBO(FBO_DISPLACE).Bind();
    glClear(GL_COLOR_BUFFER_BIT);

    DisplaceProvider * const cb = (DisplaceProvider *)irr_driver->getCallback(ES_DISPLACE);
    cb->update();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);
    glClear(GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    if (irr_driver->hasARB_base_instance())
        glBindVertexArray(VAOManager::getInstance()->getVAO(video::EVT_2TCOORDS));
    // Generate displace mask
    // Use RTT_TMP4 as displace mask
    irr_driver->getFBO(FBO_TMP1_WITH_DS).Bind();
    for (unsigned i = 0; i < ListDisplacement::getInstance()->size(); i++)
    {
        const GLMesh &mesh = *(STK::tuple_get<0>(ListDisplacement::getInstance()->at(i)));
        if (!irr_driver->hasARB_base_instance())
            glBindVertexArray(mesh.vao);
        const core::matrix4 &AbsoluteTransformation = STK::tuple_get<1>(ListDisplacement::getInstance()->at(i));
        if (mesh.VAOType != video::EVT_2TCOORDS)
        {
#ifdef DEBUG
            Log::error("Materials", "Displacement has wrong vertex type");
#endif
            continue;
        }

        GLenum ptype = mesh.PrimitiveType;
        GLenum itype = mesh.IndexType;
        size_t count = mesh.IndexCount;

        glUseProgram(MeshShader::DisplaceMaskShader::getInstance()->Program);
        MeshShader::DisplaceMaskShader::getInstance()->setUniforms(AbsoluteTransformation);
        glDrawElementsBaseVertex(ptype, (int)count, itype,
                                 (GLvoid *)mesh.vaoOffset, (int)mesh.vaoBaseVertex);
    }

    irr_driver->getFBO(FBO_DISPLACE).Bind();
    if (!displaceTex)
        displaceTex = irr_driver->getTexture(FileManager::TEXTURE, "displace.png");
    for (unsigned i = 0; i < ListDisplacement::getInstance()->size(); i++)
    {
        const GLMesh &mesh = *(STK::tuple_get<0>(ListDisplacement::getInstance()->at(i)));
        if (!irr_driver->hasARB_base_instance())
            glBindVertexArray(mesh.vao);
        const core::matrix4 &AbsoluteTransformation = STK::tuple_get<1>(ListDisplacement::getInstance()->at(i));
        if (mesh.VAOType != video::EVT_2TCOORDS)
            continue;

        GLenum ptype = mesh.PrimitiveType;
        GLenum itype = mesh.IndexType;
        size_t count = mesh.IndexCount;
        // Render the effect
        MeshShader::DisplaceShader::getInstance()->SetTextureUnits(
            getTextureGLuint(displaceTex),
            irr_driver->getRenderTargetTexture(RTT_COLOR),
            irr_driver->getRenderTargetTexture(RTT_TMP1),
            getTextureGLuint(mesh.textures[0]));
        glUseProgram(MeshShader::DisplaceShader::getInstance()->Program);
        MeshShader::DisplaceShader::getInstance()->setUniforms(AbsoluteTransformation,
            core::vector2df(cb->getDirX(), cb->getDirY()),
            core::vector2df(cb->getDir2X(), cb->getDir2Y()));

        glDrawElementsBaseVertex(ptype, (int)count, itype, (GLvoid *)mesh.vaoOffset, (int)mesh.vaoBaseVertex);
    }

    irr_driver->getFBO(FBO_COLORS).Bind();
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    m_post_processing->renderPassThrough(m_rtts->getRenderTarget(RTT_DISPLACE));
    glDisable(GL_STENCIL_TEST);

}

template<typename T, typename...uniforms>
void drawShadow(const T *Shader, unsigned cascade, const GLMesh *mesh, uniforms... Args)
{
    irr_driver->IncreaseObjectCount();
    GLenum ptype = mesh->PrimitiveType;
    GLenum itype = mesh->IndexType;
    size_t count = mesh->IndexCount;

    Shader->setUniforms(cascade, Args...);
    glDrawElementsBaseVertex(ptype, (int)count, itype, (GLvoid *)mesh->vaoOffset, (int)mesh->vaoBaseVertex);
}

template<int...List>
struct shadow_custom_unroll_args;

template<>
struct shadow_custom_unroll_args<>
{
    template<typename T, typename ...TupleTypes, typename ...Args>
    static void exec(const T *Shader, unsigned cascade, const STK::Tuple<TupleTypes...> &t, Args... args)
    {
        drawShadow<T>(Shader, cascade, STK::tuple_get<0>(t), args...);
    }
};

template<int N, int...List>
struct shadow_custom_unroll_args<N, List...>
{
    template<typename T, typename ...TupleTypes, typename ...Args>
    static void exec(const T *Shader, unsigned cascade, const STK::Tuple<TupleTypes...> &t, Args... args)
    {
        shadow_custom_unroll_args<List...>::template exec<T>(Shader, cascade, t, STK::tuple_get<N>(t), args...);
    }
};

template<typename T, int...List>
void renderShadow(unsigned cascade)
{
    auto &t = T::List::getInstance()->Shadows[cascade];
    glUseProgram(T::ShadowPassShader::getInstance()->Program);
    if (irr_driver->hasARB_base_instance())
        glBindVertexArray(VAOManager::getInstance()->getVAO(T::VertexType));
    for (unsigned i = 0; i < t.size(); i++)
    {
        GLMesh *mesh = STK::tuple_get<0>(t.at(i));
        if (!irr_driver->hasARB_base_instance())
            glBindVertexArray(mesh->vao);
        if (UserConfigParams::m_azdo)
            HandleExpander<typename T::ShadowPassShader>::template Expand(mesh->TextureHandles, T::ShadowTextures);
        else
            TexExpander<typename T::ShadowPassShader>::template ExpandTex(*mesh, T::ShadowTextures);
        shadow_custom_unroll_args<List...>::template exec<typename T::ShadowPassShader>(T::ShadowPassShader::getInstance(), cascade, t.at(i));
    }
}

template<typename T, typename...Args>
void renderInstancedShadow(unsigned cascade, Args ...args)
{
    glUseProgram(T::InstancedShadowPassShader::getInstance()->Program);
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType, InstanceTypeShadow));
    std::vector<GLMesh *> &t = T::InstancedList::getInstance()->Shadows[cascade];
    for (unsigned i = 0; i < t.size(); i++)
    {
        GLMesh *mesh = t[i];

        TexExpander<typename T::InstancedShadowPassShader>::template ExpandTex(*mesh, T::ShadowTextures);
        T::InstancedShadowPassShader::getInstance()->setUniforms(cascade, args...);
        size_t tmp = ShadowPassCmd::getInstance()->Offset[cascade][T::MaterialType] + i;
        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (const void*)((tmp) * sizeof(DrawElementsIndirectCommand)));
    }

}

template<typename T, typename...Args>
static void multidrawShadow(unsigned i, Args ...args)
{
    glUseProgram(T::InstancedShadowPassShader::getInstance()->Program);
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType, InstanceTypeShadow));
    if (ShadowPassCmd::getInstance()->Size[i][T::MaterialType])
    {
        T::InstancedShadowPassShader::getInstance()->setUniforms(i, args...);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, 
            (const void*)(ShadowPassCmd::getInstance()->Offset[i][T::MaterialType] * sizeof(DrawElementsIndirectCommand)),
            (int)ShadowPassCmd::getInstance()->Size[i][T::MaterialType], sizeof(DrawElementsIndirectCommand));
    }
}

void IrrDriver::renderShadows()
{
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.5, 0.);
    m_rtts->getShadowFBO().Bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    glDrawBuffer(GL_NONE);


    for (unsigned cascade = 0; cascade < 4; cascade++)
    {
        ScopedGPUTimer Timer(getGPUTimer(Q_SHADOWS_CASCADE0 + cascade));

        renderShadow<DefaultMaterial, 1>(cascade);
        renderShadow<SphereMap, 1>(cascade);
        renderShadow<DetailMat, 1>(cascade);
        renderShadow<SplattingMat, 1>(cascade);
        renderShadow<NormalMat, 1>(cascade);
        renderShadow<AlphaRef, 1>(cascade);
        renderShadow<UnlitMat, 1>(cascade);
        renderShadow<GrassMat, 3, 1>(cascade);

        if (irr_driver->hasARB_draw_indirect())
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ShadowPassCmd::getInstance()->drawindirectcmd);

        if (UserConfigParams::m_azdo)
        {
            multidrawShadow<DefaultMaterial>(cascade);
            multidrawShadow<DetailMat>(cascade);
            multidrawShadow<NormalMat>(cascade);
            multidrawShadow<AlphaRef>(cascade);
            multidrawShadow<UnlitMat>(cascade);
            multidrawShadow<GrassMat>(cascade, windDir);
        }
        else if (irr_driver->hasARB_draw_indirect())
        {
            renderInstancedShadow<DefaultMaterial>(cascade);
            renderInstancedShadow<DetailMat>(cascade);
            renderInstancedShadow<AlphaRef>(cascade);
            renderInstancedShadow<UnlitMat>(cascade);
            renderInstancedShadow<GrassMat>(cascade, windDir);
            renderInstancedShadow<NormalMat>(cascade);
        }
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
}



template<int...List>
struct rsm_custom_unroll_args;

template<>
struct rsm_custom_unroll_args<>
{
    template<typename T, typename ...TupleTypes, typename ...Args>
    static void exec(const core::matrix4 &rsm_matrix, const STK::Tuple<TupleTypes...> &t, Args... args)
    {
        draw<T>(T::getInstance(), STK::tuple_get<0>(t), rsm_matrix, args...);
    }
};

template<int N, int...List>
struct rsm_custom_unroll_args<N, List...>
{
    template<typename T, typename ...TupleTypes, typename ...Args>
    static void exec(const core::matrix4 &rsm_matrix, const STK::Tuple<TupleTypes...> &t, Args... args)
    {
        rsm_custom_unroll_args<List...>::template exec<T>(rsm_matrix, t, STK::tuple_get<N>(t), args...);
    }
};

template<typename T, int... Selector>
void drawRSM(const core::matrix4 & rsm_matrix)
{
    glUseProgram(T::RSMShader::getInstance()->Program);
    if (irr_driver->hasARB_base_instance())
        glBindVertexArray(VAOManager::getInstance()->getVAO(T::VertexType));
    auto t = T::List::getInstance()->RSM;
    for (unsigned i = 0; i < t.size(); i++)
    {
        std::vector<GLuint> Textures;
        GLMesh *mesh = STK::tuple_get<0>(t.at(i));
        if (!irr_driver->hasARB_base_instance())
            glBindVertexArray(mesh->vao);
        if (UserConfigParams::m_azdo)
            HandleExpander<typename T::RSMShader>::template Expand(mesh->TextureHandles, T::RSMTextures);
        else
            TexExpander<typename T::RSMShader>::template ExpandTex(*mesh, T::RSMTextures);
        rsm_custom_unroll_args<Selector...>::template exec<typename T::RSMShader>(rsm_matrix, t.at(i));
    }
}

template<typename T, typename...Args>
void renderRSMShadow(Args ...args)
{
    glUseProgram(T::InstancedRSMShader::getInstance()->Program);
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType, InstanceTypeRSM));
    auto t = T::InstancedList::getInstance()->RSM;
    for (unsigned i = 0; i < t.size(); i++)
    {
        std::vector<GLuint> Textures;
        GLMesh *mesh = t[i];

        TexExpander<typename T::InstancedRSMShader>::template ExpandTex(*mesh, T::RSMTextures);
        T::InstancedRSMShader::getInstance()->setUniforms(args...);
        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (const void*)((RSMPassCmd::getInstance()->Offset[T::MaterialType] + i)* sizeof(DrawElementsIndirectCommand)));
    }
}

template<typename T, typename... Args>
void multidrawRSM(Args...args)
{
    glUseProgram(T::InstancedRSMShader::getInstance()->Program);
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType, InstanceTypeRSM));
    if (RSMPassCmd::getInstance()->Size[T::MaterialType])
    {
        T::InstancedRSMShader::getInstance()->setUniforms(args...);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT,
            (const void*)(RSMPassCmd::getInstance()->Offset[T::MaterialType] * sizeof(DrawElementsIndirectCommand)),
            (int)RSMPassCmd::getInstance()->Size[T::MaterialType], sizeof(DrawElementsIndirectCommand));
    }
}

void IrrDriver::renderRSM()
{
    ScopedGPUTimer Timer(getGPUTimer(Q_RSM));
    m_rtts->getRSM().Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawRSM<DefaultMaterial, 3, 1>(rsm_matrix);
    drawRSM<AlphaRef, 3, 1>(rsm_matrix);
    drawRSM<NormalMat, 3, 1>(rsm_matrix);
    drawRSM<UnlitMat, 3, 1>(rsm_matrix);
    drawRSM<DetailMat, 3, 1>(rsm_matrix);
    drawRSM<SplattingMat, 1>(rsm_matrix);

    if (irr_driver->hasARB_draw_indirect())
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, RSMPassCmd::getInstance()->drawindirectcmd);

    if (UserConfigParams::m_azdo)
    {
        multidrawRSM<DefaultMaterial>(rsm_matrix);
        multidrawRSM<NormalMat>(rsm_matrix);
        multidrawRSM<AlphaRef>(rsm_matrix);
        multidrawRSM<UnlitMat>(rsm_matrix);
        multidrawRSM<DetailMat>(rsm_matrix);
    }
    else if (irr_driver->hasARB_draw_indirect())
    {
        renderRSMShadow<DefaultMaterial>(rsm_matrix);
        renderRSMShadow<AlphaRef>(rsm_matrix);
        renderRSMShadow<UnlitMat>(rsm_matrix);
        renderRSMShadow<NormalMat>(rsm_matrix);
        renderRSMShadow<DetailMat>(rsm_matrix);
    }
}
