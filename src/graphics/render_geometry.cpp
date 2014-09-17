#include "graphics/irr_driver.hpp"

#include "config/user_config.hpp"
#include "graphics/callbacks.hpp"
#include "graphics/camera.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/lens_flare.hpp"
#include "graphics/light.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/particle_kind_manager.hpp"
#include "graphics/per_camera_node.hpp"
#include "graphics/post_processing.hpp"
#include "graphics/referee.hpp"
#include "graphics/rtts.hpp"
#include "graphics/screenquad.hpp"
#include "graphics/shaders.hpp"
#include "graphics/stkmeshscenenode.hpp"
#include "graphics/wind.hpp"
#include "io/file_manager.hpp"
#include "items/item.hpp"
#include "items/item_manager.hpp"
#include "modes/world.hpp"
#include "physics/physics.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/helpers.hpp"
#include "utils/log.hpp"
#include "utils/profiler.hpp"
#include "utils/tuple.hpp"
#include "stkscenemanager.hpp"

#include <algorithm>

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
    glDrawElementsBaseVertex(ptype, count, itype, (GLvoid *)mesh->vaoOffset, mesh->vaoBaseVertex);
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


template<typename Shader, enum E_VERTEX_TYPE VertexType, int ...List, typename... TupleType>
void renderMeshes1stPass(const std::vector<TexUnit> &TexUnits, std::vector<STK::Tuple<TupleType...> > &meshes)
{
    glUseProgram(Shader::getInstance()->Program);
    if (irr_driver->hasARB_base_instance())
        glBindVertexArray(VAOManager::getInstance()->getVAO(VertexType));
    for (unsigned i = 0; i < meshes.size(); i++)
    {
        std::vector<GLuint> Textures;
        std::vector<uint64_t> Handles;
        GLMesh &mesh = *(STK::tuple_get<0>(meshes.at(i)));
        if (!irr_driver->hasARB_base_instance())
            glBindVertexArray(mesh.vao);
        for (unsigned j = 0; j < TexUnits.size(); j++)
        {
            if (UserConfigParams::m_azdo)
                Handles.push_back(mesh.TextureHandles[TexUnits[j].m_id]);
            else
                Textures.push_back(getTextureGLuint(mesh.textures[TexUnits[j].m_id]));
        }
        if (mesh.VAOType != VertexType)
        {
#ifdef DEBUG
            Log::error("Materials", "Wrong vertex Type associed to pass 1 (hint texture : %s)", mesh.textures[0]->getName().getPath().c_str());
#endif
            continue;
        }

        if (UserConfigParams::m_azdo)
            Shader::getInstance()->SetTextureHandles(Handles);
        else
            Shader::getInstance()->SetTextureUnits(Textures);
        custom_unroll_args<List...>::template exec(Shader::getInstance(), meshes.at(i));
    }
}

template<typename Shader, MeshMaterial Mat, video::E_VERTEX_TYPE VT, InstanceType IT, typename...Args>
void renderInstancedMeshes1stPass(const std::vector<TexUnit> &TexUnits, std::vector<GLMesh *> &meshes, Args...args)
{
    glUseProgram(Shader::getInstance()->Program);
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(VT, IT));
    for (unsigned i = 0; i < meshes.size(); i++)
    {
        std::vector<GLuint> Textures;
        GLMesh *mesh = meshes[i];
#ifdef DEBUG
        if (mesh->VAOType != VT)
            Log::error("RenderGeometry", "Wrong instanced vertex format");
#endif
        for (unsigned j = 0; j < TexUnits.size(); j++)
            Textures.push_back(getTextureGLuint(mesh->textures[TexUnits[j].m_id]));
        Shader::getInstance()->SetTextureUnits(Textures);

        Shader::getInstance()->setUniforms(args...);
        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (const void*)((SolidPassCmd::getInstance()->Offset[Mat] + i) * sizeof(DrawElementsIndirectCommand)));
    }
}

template<typename Shader, MeshMaterial Mat, video::E_VERTEX_TYPE VT, InstanceType IT, typename...Args>
void multidraw1stPass(Args...args)
{
    glUseProgram(Shader::getInstance()->Program);
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(VT, IT));
    if (SolidPassCmd::getInstance()->Size[Mat])
    {
        Shader::getInstance()->setUniforms(args...);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT,
            (const void*)(SolidPassCmd::getInstance()->Offset[Mat] * sizeof(DrawElementsIndirectCommand)),
            SolidPassCmd::getInstance()->Size[Mat],
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

        std::vector<TexUnit> object_pass1_texunits = TexUnits(TexUnit(0, true) );
        renderMeshes1stPass<MeshShader::ObjectPass1Shader, video::EVT_STANDARD, 2, 1>(object_pass1_texunits, ListMatDefault::getInstance()->SolidPass);
        renderMeshes1stPass<MeshShader::ObjectPass1Shader, video::EVT_2TCOORDS, 2, 1>(object_pass1_texunits, ListMatSplatting::getInstance()->SolidPass);
        renderMeshes1stPass<MeshShader::ObjectRefPass1Shader, video::EVT_STANDARD, 3, 2, 1>(object_pass1_texunits, ListMatUnlit::getInstance()->SolidPass);
        renderMeshes1stPass<MeshShader::ObjectRefPass1Shader, video::EVT_STANDARD, 3, 2, 1>(TexUnits(TexUnit(0, true)), ListMatAlphaRef::getInstance()->SolidPass);
        renderMeshes1stPass<MeshShader::GrassPass1Shader, video::EVT_STANDARD, 3, 2, 1>(TexUnits(TexUnit(0, true)), ListMatGrass::getInstance()->SolidPass);
        renderMeshes1stPass<MeshShader::NormalMapShader, video::EVT_TANGENTS, 2, 1>(TexUnits(
            TexUnit(1, false),
            TexUnit(0, true)
            ), ListMatNormalMap::getInstance()->SolidPass);
        renderMeshes1stPass<MeshShader::ObjectPass1Shader, video::EVT_STANDARD, 2, 1>(object_pass1_texunits, ListMatSphereMap::getInstance()->SolidPass);
        renderMeshes1stPass<MeshShader::ObjectPass1Shader, video::EVT_2TCOORDS, 2, 1>(object_pass1_texunits, ListMatDetails::getInstance()->SolidPass);

        if (UserConfigParams::m_azdo)
        {
            multidraw1stPass<MeshShader::InstancedObjectPass1Shader, MAT_DEFAULT, video::EVT_STANDARD, InstanceTypeSingleTex>();
            multidraw1stPass<MeshShader::InstancedObjectRefPass1Shader, MAT_ALPHA_REF, video::EVT_STANDARD, InstanceTypeSingleTex>();
            multidraw1stPass<MeshShader::InstancedObjectPass1Shader, MAT_SPHEREMAP, video::EVT_STANDARD, InstanceTypeSingleTex>();
            multidraw1stPass<MeshShader::InstancedObjectRefPass1Shader, MAT_UNLIT, video::EVT_STANDARD, InstanceTypeSingleTex>();
            multidraw1stPass<MeshShader::InstancedGrassPass1Shader, MAT_GRASS, video::EVT_STANDARD, InstanceTypeSingleTex>(windDir);

            multidraw1stPass<MeshShader::InstancedNormalMapShader, MAT_NORMAL_MAP, video::EVT_TANGENTS, InstanceTypeDualTex>();
            multidraw1stPass<MeshShader::InstancedObjectPass1Shader, MAT_DETAIL, video::EVT_2TCOORDS, InstanceTypeDualTex>();

        }
        else if (irr_driver->hasARB_draw_indirect())
        {
            // Default
            renderInstancedMeshes1stPass<MeshShader::InstancedObjectPass1Shader, MAT_DEFAULT, video::EVT_STANDARD, InstanceTypeSingleTex>(
                TexUnits(TexUnit(0, true), TexUnit(1, false)), ListInstancedMatDefault::getInstance()->SolidPass);
            // Alpha ref
            renderInstancedMeshes1stPass<MeshShader::InstancedObjectRefPass1Shader, MAT_ALPHA_REF, video::EVT_STANDARD, InstanceTypeSingleTex>(
                TexUnits(TexUnit(0, true), TexUnit(1, false)), ListInstancedMatAlphaRef::getInstance()->SolidPass);
            // Unlit
            renderInstancedMeshes1stPass<MeshShader::InstancedObjectPass1Shader, MAT_UNLIT, video::EVT_STANDARD, InstanceTypeSingleTex>(
            TexUnits(TexUnit(0, true), TexUnit(1, false)), ListInstancedMatUnlit::getInstance()->SolidPass);
            // Spheremap
            renderInstancedMeshes1stPass<MeshShader::InstancedObjectPass1Shader, MAT_SPHEREMAP, video::EVT_STANDARD, InstanceTypeSingleTex>(
                TexUnits(TexUnit(0, true), TexUnit(1, false)), ListInstancedMatSphereMap::getInstance()->SolidPass);
            // Grass
            renderInstancedMeshes1stPass<MeshShader::InstancedGrassPass1Shader, MAT_GRASS, video::EVT_STANDARD, InstanceTypeSingleTex>(
                TexUnits(TexUnit(0, true)), ListInstancedMatGrass::getInstance()->SolidPass, windDir);

            // Detail
            renderInstancedMeshes1stPass<MeshShader::InstancedObjectPass1Shader, MAT_DETAIL, video::EVT_2TCOORDS, InstanceTypeDualTex>(
                TexUnits(TexUnit(0, true), TexUnit(1, false)), ListInstancedMatDetails::getInstance()->SolidPass);

            // Normal Map
            renderInstancedMeshes1stPass<MeshShader::InstancedNormalMapShader, MAT_NORMAL_MAP, video::EVT_TANGENTS, InstanceTypeDualTex>(
                TexUnits(TexUnit(1, false), TexUnit(0, true)), ListInstancedMatNormalMap::getInstance()->SolidPass);
        }
    }
}

template<typename Shader, enum E_VERTEX_TYPE VertexType, int...List, typename... TupleType>
void renderMeshes2ndPass(const std::vector<TexUnit> &TexUnits, std::vector<STK::Tuple<TupleType...> > &meshes, const std::vector<uint64_t> &Prefilled_Handle,
    const std::vector<GLuint> &Prefilled_Tex)
{
    glUseProgram(Shader::getInstance()->Program);
    if (irr_driver->hasARB_base_instance())
        glBindVertexArray(VAOManager::getInstance()->getVAO(VertexType));
    for (unsigned i = 0; i < meshes.size(); i++)
    {
        std::vector<uint64_t> Handles(Prefilled_Handle);
        std::vector<GLuint> Textures(Prefilled_Tex);
        GLMesh &mesh = *(STK::tuple_get<0>(meshes.at(i)));
        if (!irr_driver->hasARB_base_instance())
            glBindVertexArray(mesh.vao);
        for (unsigned j = 0; j < TexUnits.size(); j++)
        {
            if (UserConfigParams::m_azdo)
                Handles.push_back(mesh.TextureHandles[TexUnits[j].m_id]);
            else
                Textures.push_back(getTextureGLuint(mesh.textures[TexUnits[j].m_id]));
        }

        if (mesh.VAOType != VertexType)
        {
#ifdef DEBUG
            Log::error("Materials", "Wrong vertex Type associed to pass 2 (hint texture : %s)", mesh.textures[0]->getName().getPath().c_str());
#endif
            continue;
        }

        if (UserConfigParams::m_azdo)
            Shader::getInstance()->SetTextureHandles(Handles);
        else
            Shader::getInstance()->SetTextureUnits(Textures);
        custom_unroll_args<List...>::template exec(Shader::getInstance(), meshes.at(i));
    }
}

template<typename Shader, MeshMaterial Mat, video::E_VERTEX_TYPE VT, InstanceType IT, typename...Args>
void renderInstancedMeshes2ndPass(const std::vector<TexUnit> &TexUnits, const std::vector<GLuint> &Prefilled_tex, std::vector<GLMesh *> &meshes, Args...args)
{
    glUseProgram(Shader::getInstance()->Program);
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(VT, IT));
    for (unsigned i = 0; i < meshes.size(); i++)
    {
        GLMesh *mesh = meshes[i];
        std::vector<GLuint> Textures(Prefilled_tex);
        for (unsigned j = 0; j < TexUnits.size(); j++)
            Textures.push_back(getTextureGLuint(mesh->textures[TexUnits[j].m_id]));
        Shader::getInstance()->SetTextureUnits(Textures);
        Shader::getInstance()->setUniforms(args...);
        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (const void*)((SolidPassCmd::getInstance()->Offset[Mat] + i) * sizeof(DrawElementsIndirectCommand)));
    }
}

template<typename Shader, MeshMaterial Mat, video::E_VERTEX_TYPE VT, InstanceType IT, typename...Args>
void multidraw2ndPass(const std::vector<uint64_t> &Handles, Args... args)
{
    glUseProgram(Shader::getInstance()->Program);
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(VT, IT));
    if (SolidPassCmd::getInstance()->Size[Mat])
    {
        Shader::getInstance()->SetTextureHandles(Handles);
        Shader::getInstance()->setUniforms(args...);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT,
            (const void*)(SolidPassCmd::getInstance()->Offset[Mat] * sizeof(DrawElementsIndirectCommand)),
            SolidPassCmd::getInstance()->Size[Mat],
            sizeof(DrawElementsIndirectCommand));
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

        renderMeshes2ndPass<MeshShader::ObjectPass2Shader, video::EVT_STANDARD, 3, 1>(TexUnits(
            TexUnit(0, true)
        ), ListMatDefault::getInstance()->SolidPass,
        createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<MeshShader::ObjectRefPass2Shader, video::EVT_STANDARD, 3, 1 >(TexUnits(
            TexUnit(0, true)
            ), ListMatAlphaRef::getInstance()->SolidPass, createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<MeshShader::ObjectUnlitShader, video::EVT_STANDARD, 3, 1>(TexUnits(
            TexUnit(0, true)
            ), ListMatUnlit::getInstance()->SolidPass, createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<MeshShader::SplattingShader, video::EVT_2TCOORDS, 1>(TexUnits(
            TexUnit(1, false),
            TexUnit(2, true),
            TexUnit(3, true),
            TexUnit(4, true),
            TexUnit(0, true)
            ), ListMatSplatting::getInstance()->SolidPass, createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<MeshShader::SphereMapShader, video::EVT_STANDARD, 2, 1>(TexUnits(
            TexUnit(0, true)
            ), ListMatSphereMap::getInstance()->SolidPass, createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<MeshShader::DetailledObjectPass2Shader, video::EVT_2TCOORDS, 1>(TexUnits(
            TexUnit(0, true),
            TexUnit(1, true)
            ), ListMatDetails::getInstance()->SolidPass, createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<MeshShader::GrassPass2Shader, video::EVT_STANDARD, 3, 1>(TexUnits(
            TexUnit(0, true)
            ), ListMatGrass::getInstance()->SolidPass, createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);
        renderMeshes2ndPass<MeshShader::ObjectPass2Shader, video::EVT_TANGENTS, 3, 1>(TexUnits(
            TexUnit(0, true)
            ), ListMatNormalMap::getInstance()->SolidPass, createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle), DiffSpecSSAOTex);

        if (UserConfigParams::m_azdo)
        {
            multidraw2ndPass<MeshShader::InstancedObjectPass2Shader, MAT_DEFAULT, video::EVT_STANDARD, InstanceTypeSingleTex>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle, 0));
            multidraw2ndPass<MeshShader::InstancedObjectRefPass2Shader, MAT_ALPHA_REF, video::EVT_STANDARD, InstanceTypeSingleTex>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle, 0));
            multidraw2ndPass<MeshShader::InstancedSphereMapShader, MAT_SPHEREMAP, video::EVT_STANDARD, InstanceTypeSingleTex>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle, 0));
            multidraw2ndPass<MeshShader::InstancedObjectUnlitShader, MAT_UNLIT, video::EVT_STANDARD, InstanceTypeSingleTex>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle, 0));
            SunLightProvider * const cb = (SunLightProvider *)irr_driver->getCallback(ES_SUNLIGHT);
            multidraw2ndPass<MeshShader::InstancedGrassPass2Shader, MAT_GRASS, video::EVT_STANDARD, InstanceTypeSingleTex>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle, DepthHandle, 0), windDir, cb->getPosition());

            multidraw2ndPass<MeshShader::InstancedObjectPass2Shader, MAT_NORMAL_MAP, video::EVT_TANGENTS, InstanceTypeDualTex>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle, 0));
            multidraw2ndPass<MeshShader::InstancedDetailledObjectPass2Shader, MAT_DETAIL, video::EVT_2TCOORDS, InstanceTypeDualTex>(createVector<uint64_t>(DiffuseHandle, SpecularHandle, SSAOHandle, 0, 0));
        }
        else if (irr_driver->hasARB_draw_indirect())
        {
            // Default
            renderInstancedMeshes2ndPass<MeshShader::InstancedObjectPass2Shader, MAT_DEFAULT, video::EVT_STANDARD, InstanceTypeSingleTex>(
                TexUnits(TexUnit(0, true)), DiffSpecSSAOTex, ListInstancedMatDefault::getInstance()->SolidPass);
            // Alpha ref
            renderInstancedMeshes2ndPass<MeshShader::InstancedObjectRefPass2Shader, MAT_ALPHA_REF, video::EVT_STANDARD, InstanceTypeSingleTex>(
                TexUnits(TexUnit(0, true)), DiffSpecSSAOTex, ListInstancedMatAlphaRef::getInstance()->SolidPass);
            // Unlit
            renderInstancedMeshes2ndPass<MeshShader::InstancedObjectUnlitShader, MAT_UNLIT, video::EVT_STANDARD, InstanceTypeSingleTex>(
                TexUnits(TexUnit(0, true)), DiffSpecSSAOTex, ListInstancedMatUnlit::getInstance()->SolidPass);
            // Spheremap
            renderInstancedMeshes2ndPass<MeshShader::InstancedSphereMapShader, MAT_SPHEREMAP, video::EVT_STANDARD, InstanceTypeSingleTex>(
                TexUnits(TexUnit(0, true)), DiffSpecSSAOTex, ListInstancedMatSphereMap::getInstance()->SolidPass);
            // Grass
            SunLightProvider * const cb = (SunLightProvider *)irr_driver->getCallback(ES_SUNLIGHT);
            DiffSpecSSAOTex.push_back(irr_driver->getDepthStencilTexture());
            renderInstancedMeshes2ndPass<MeshShader::InstancedGrassPass2Shader, MAT_GRASS, video::EVT_STANDARD, InstanceTypeSingleTex>(
                TexUnits(TexUnit(0, true)), DiffSpecSSAOTex, ListInstancedMatGrass::getInstance()->SolidPass, windDir, cb->getPosition());
            DiffSpecSSAOTex.pop_back();

            // Details
            renderInstancedMeshes2ndPass<MeshShader::InstancedDetailledObjectPass2Shader, MAT_DETAIL, video::EVT_2TCOORDS, InstanceTypeDualTex>(
                TexUnits(TexUnit(0, true), TexUnit(1, false)), DiffSpecSSAOTex, ListInstancedMatDetails::getInstance()->SolidPass);
            // Normal map
            renderInstancedMeshes2ndPass<MeshShader::InstancedObjectPass2Shader, MAT_NORMAL_MAP, video::EVT_TANGENTS, InstanceTypeDualTex>(
                TexUnits(TexUnit(0, true)), DiffSpecSSAOTex, ListInstancedMatNormalMap::getInstance()->SolidPass);
        }
    }
}

template<enum E_VERTEX_TYPE VertexType, typename... TupleType>
static void renderMeshNormals(std::vector<STK::Tuple<TupleType...> > *meshes)
{
    glUseProgram(MeshShader::NormalVisualizer::getInstance()->Program);
    glBindVertexArray(VAOManager::getInstance()->getVAO(VertexType));
    for (unsigned i = 0; i < meshes->size(); i++)
    {
        GLMesh &mesh = *(STK::tuple_get<0>(meshes->at(i)));

        if (mesh.VAOType != VertexType)
        {
#ifdef DEBUG
            Log::error("Materials", "Wrong vertex Type associed to pass 2 (hint texture : %s)", mesh.textures[0]->getName().getPath().c_str());
#endif
            continue;
        }
        draw(MeshShader::NormalVisualizer::getInstance(), STK::tuple_get<0>(meshes->at(i)), STK::tuple_get<1>(meshes->at(i)), STK::tuple_get<2>(meshes->at(i)), video::SColor(255, 0, 255, 0));
    }
}

void IrrDriver::renderNormalsVisualisation()
{
//    renderMeshNormals<video::EVT_STANDARD>(ListMatDefault::getInstance());
//    renderMeshNormals<video::EVT_STANDARD>(ListMatAlphaRef::getInstance());
//    renderMeshNormals<video::EVT_STANDARD>(ListMatSphereMap::getInstance());
//    renderMeshNormals<video::EVT_STANDARD>(ListMatGrass::getInstance());
//    renderMeshNormals<video::EVT_2TCOORDS>(ListMatDetails::getInstance());
//    renderMeshNormals<video::EVT_STANDARD>(ListMatUnlit::getInstance());
//    renderMeshNormals<video::EVT_2TCOORDS>(ListMatSplatting::getInstance());
//    renderMeshNormals<video::EVT_TANGENTS>(ListMatNormalMap::getInstance());
}

template<typename Shader, enum E_VERTEX_TYPE VertexType, int...List, typename... TupleType>
void renderTransparenPass(const std::vector<TexUnit> &TexUnits, std::vector<STK::Tuple<TupleType...> > *meshes)
{
    glUseProgram(Shader::getInstance()->Program);
    if (irr_driver->hasARB_base_instance())
        glBindVertexArray(VAOManager::getInstance()->getVAO(VertexType));
    for (unsigned i = 0; i < meshes->size(); i++)
    {
        std::vector<uint64_t> Handles;
        std::vector<GLuint> Textures;
        GLMesh &mesh = *(STK::tuple_get<0>(meshes->at(i)));
        if (!irr_driver->hasARB_base_instance())
            glBindVertexArray(mesh.vao);
        for (unsigned j = 0; j < TexUnits.size(); j++)
        {
            if (!mesh.textures[TexUnits[j].m_id])
                mesh.textures[TexUnits[j].m_id] = getUnicolorTexture(video::SColor(255, 255, 255, 255));
            compressTexture(mesh.textures[TexUnits[j].m_id], TexUnits[j].m_premul_alpha);
            if (UserConfigParams::m_azdo)
            {
                if (!mesh.TextureHandles[TexUnits[j].m_id])
                    mesh.TextureHandles[TexUnits[j].m_id] = glGetTextureSamplerHandleARB(getTextureGLuint(mesh.textures[TexUnits[j].m_id]), Shader::getInstance()->SamplersId[Handles.size()]);
                if (!glIsTextureHandleResidentARB(mesh.TextureHandles[TexUnits[j].m_id]))
                    glMakeTextureHandleResidentARB(mesh.TextureHandles[TexUnits[j].m_id]);
                Handles.push_back(mesh.TextureHandles[TexUnits[j].m_id]);
            }
            else
                Textures.push_back(getTextureGLuint(mesh.textures[TexUnits[j].m_id]));
        }

        if (mesh.VAOType != VertexType)
        {
#ifdef DEBUG
            Log::error("Materials", "Wrong vertex Type associed to pass 2 (hint texture : %s)", mesh.textures[0]->getName().getPath().c_str());
#endif
            continue;
        }

        if (UserConfigParams::m_azdo)
            Shader::getInstance()->SetTextureHandles(Handles);
        else
            Shader::getInstance()->SetTextureUnits(Textures);
        custom_unroll_args<List...>::template exec(Shader::getInstance(), meshes->at(i));
    }
}

static video::ITexture *displaceTex = 0;

void IrrDriver::renderTransparent()
{
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_ALPHA_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glDisable(GL_CULL_FACE);

    irr_driver->setPhase(TRANSPARENT_PASS);

    for (unsigned i = 0; i < ImmediateDrawList::getInstance()->size(); i++)
        ImmediateDrawList::getInstance()->at(i)->render();

    if (irr_driver->hasARB_base_instance())
        glBindVertexArray(VAOManager::getInstance()->getVAO(EVT_STANDARD));

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
    glDisable(GL_ALPHA_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);
    glClear(GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    if (irr_driver->hasARB_base_instance())
        glBindVertexArray(VAOManager::getInstance()->getVAO(EVT_2TCOORDS));
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
        glDrawElementsBaseVertex(ptype, count, itype, (GLvoid *)mesh.vaoOffset, mesh.vaoBaseVertex);
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
            createVector<GLuint>(getTextureGLuint(displaceTex),
                irr_driver->getRenderTargetTexture(RTT_COLOR),
                irr_driver->getRenderTargetTexture(RTT_TMP1),
                getTextureGLuint(mesh.textures[0])));
        glUseProgram(MeshShader::DisplaceShader::getInstance()->Program);
        MeshShader::DisplaceShader::getInstance()->setUniforms(AbsoluteTransformation,
            core::vector2df(cb->getDirX(), cb->getDirY()),
            core::vector2df(cb->getDir2X(), cb->getDir2Y()));

        glDrawElementsBaseVertex(ptype, count, itype, (GLvoid *)mesh.vaoOffset, mesh.vaoBaseVertex);
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
    glDrawElementsBaseVertex(ptype, count, itype, (GLvoid *)mesh->vaoOffset, mesh->vaoBaseVertex);
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

template<typename T, enum E_VERTEX_TYPE VertexType, int...List, typename... Args>
void renderShadow(const std::vector<GLuint> TextureUnits, unsigned cascade, const std::vector<STK::Tuple<Args...> > &t)
{
    glUseProgram(T::getInstance()->Program);
    if (irr_driver->hasARB_base_instance())
        glBindVertexArray(VAOManager::getInstance()->getVAO(VertexType));
    for (unsigned i = 0; i < t.size(); i++)
    {
        std::vector<uint64_t> Handles;
        std::vector<GLuint> Textures;
        GLMesh *mesh = STK::tuple_get<0>(t.at(i));
        if (!irr_driver->hasARB_base_instance())
            glBindVertexArray(mesh->vao);
        for (unsigned j = 0; j < TextureUnits.size(); j++)
        {
            compressTexture(mesh->textures[TextureUnits[j]], true);
            if (UserConfigParams::m_azdo)
                Handles.push_back(mesh->TextureHandles[TextureUnits[j]]);
            else
                Textures.push_back(getTextureGLuint(mesh->textures[TextureUnits[j]]));
        }
        if (UserConfigParams::m_azdo)
            T::getInstance()->SetTextureHandles(Handles);
        else
            T::getInstance()->SetTextureUnits(Textures);
        shadow_custom_unroll_args<List...>::template exec<T>(T::getInstance(), cascade, t.at(i));
    }
}

template<typename Shader, MeshMaterial Mat, video::E_VERTEX_TYPE VT, typename...Args>
void renderInstancedShadow(const std::vector<GLuint> TextureUnits, unsigned cascade, const std::vector<GLMesh *> &t, Args ...args)
{
    glUseProgram(Shader::getInstance()->Program);
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(VT, InstanceTypeShadow));
    for (unsigned i = 0; i < t.size(); i++)
    {
        std::vector<uint64_t> Handles;
        std::vector<GLuint> Textures;
        GLMesh *mesh = t[i];

        for (unsigned j = 0; j < TextureUnits.size(); j++)
            Textures.push_back(getTextureGLuint(mesh->textures[TextureUnits[j]]));

        Shader::getInstance()->SetTextureUnits(Textures);
        Shader::getInstance()->setUniforms(cascade, args...);
        size_t tmp = ShadowPassCmd::getInstance()->Offset[cascade][Mat] + i;
        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (const void*)((tmp) * sizeof(DrawElementsIndirectCommand)));
    }
}

template<typename Shader, MeshMaterial Mat, video::E_VERTEX_TYPE VT, typename...Args>
static void multidrawShadow(unsigned i, Args ...args)
{
    glUseProgram(Shader::getInstance()->Program);
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(VT, InstanceTypeShadow));
    if (ShadowPassCmd::getInstance()->Size[i][Mat])
    {
        Shader::getInstance()->setUniforms(i, args...);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (const void*)(ShadowPassCmd::getInstance()->Offset[i][Mat] * sizeof(DrawElementsIndirectCommand)), ShadowPassCmd::getInstance()->Size[i][Mat], sizeof(DrawElementsIndirectCommand));
    }
}

void IrrDriver::renderShadows()
{
    ScopedGPUTimer Timer(getGPUTimer(Q_SHADOWS));

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.5, 0.);
    m_rtts->getShadowFBO().Bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    glDrawBuffer(GL_NONE);


    for (unsigned cascade = 0; cascade < 4; cascade++)
    {

        std::vector<GLuint> noTexUnits;

        renderShadow<MeshShader::ShadowShader, EVT_STANDARD, 1>(noTexUnits, cascade, ListMatDefault::getInstance()->Shadows[cascade]);
        renderShadow<MeshShader::ShadowShader, EVT_STANDARD, 1>(noTexUnits, cascade, ListMatSphereMap::getInstance()->Shadows[cascade]);
        renderShadow<MeshShader::ShadowShader, EVT_2TCOORDS, 1>(noTexUnits, cascade, ListMatDetails::getInstance()->Shadows[cascade]);
        renderShadow<MeshShader::ShadowShader, EVT_2TCOORDS, 1>(noTexUnits, cascade, ListMatSplatting::getInstance()->Shadows[cascade]);
        renderShadow<MeshShader::ShadowShader, EVT_TANGENTS, 1>(noTexUnits, cascade, ListMatNormalMap::getInstance()->Shadows[cascade]);
        renderShadow<MeshShader::RefShadowShader, EVT_STANDARD, 1>(std::vector<GLuint>{ 0 }, cascade, ListMatAlphaRef::getInstance()->Shadows[cascade]);
        renderShadow<MeshShader::RefShadowShader, EVT_STANDARD, 1>(std::vector<GLuint>{ 0 }, cascade, ListMatUnlit::getInstance()->Shadows[cascade]);
        renderShadow<MeshShader::GrassShadowShader, EVT_STANDARD, 3, 1>(std::vector<GLuint>{ 0 }, cascade, ListMatGrass::getInstance()->Shadows[cascade]);

        if (irr_driver->hasARB_draw_indirect())
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, ShadowPassCmd::getInstance()->drawindirectcmd);

        if (UserConfigParams::m_azdo)
        {
            multidrawShadow<MeshShader::InstancedShadowShader, MAT_DEFAULT, video::EVT_STANDARD>(cascade);
            multidrawShadow<MeshShader::InstancedShadowShader, MAT_DETAIL, video::EVT_2TCOORDS>(cascade);
            multidrawShadow<MeshShader::InstancedShadowShader, MAT_NORMAL_MAP, video::EVT_TANGENTS>(cascade);
            multidrawShadow<MeshShader::InstancedRefShadowShader, MAT_ALPHA_REF, video::EVT_STANDARD>(cascade);
            multidrawShadow<MeshShader::InstancedRefShadowShader, MAT_UNLIT, video::EVT_STANDARD>(cascade);
            multidrawShadow<MeshShader::InstancedGrassShadowShader, MAT_GRASS, video::EVT_STANDARD>(cascade, windDir);
        }
        else if (irr_driver->hasARB_draw_indirect())
        {
            renderInstancedShadow<MeshShader::InstancedShadowShader, MAT_DEFAULT, video::EVT_STANDARD>(noTexUnits, cascade, ListInstancedMatDefault::getInstance()->Shadows[cascade]);
            renderInstancedShadow<MeshShader::InstancedShadowShader, MAT_DETAIL, video::EVT_2TCOORDS>(noTexUnits, cascade, ListInstancedMatDetails::getInstance()->Shadows[cascade]);
            renderInstancedShadow<MeshShader::InstancedRefShadowShader, MAT_ALPHA_REF, video::EVT_STANDARD>(std::vector < GLuint > { 0 }, cascade, ListInstancedMatAlphaRef::getInstance()->Shadows[cascade]);
            renderInstancedShadow<MeshShader::InstancedRefShadowShader, MAT_UNLIT, video::EVT_STANDARD>(std::vector < GLuint > { 0 }, cascade, ListInstancedMatUnlit::getInstance()->Shadows[cascade]);
            renderInstancedShadow<MeshShader::InstancedGrassShadowShader, MAT_GRASS, video::EVT_STANDARD>(std::vector < GLuint > { 0 }, cascade, ListInstancedMatGrass::getInstance()->Shadows[cascade], windDir);
            renderInstancedShadow<MeshShader::InstancedShadowShader, MAT_NORMAL_MAP, video::EVT_TANGENTS>(noTexUnits, cascade, ListInstancedMatNormalMap::getInstance()->Shadows[cascade]);
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

template<typename T, enum E_VERTEX_TYPE VertexType, int... Selector, typename... Args>
void drawRSM(const core::matrix4 & rsm_matrix, const std::vector<GLuint> &TextureUnits, const std::vector<STK::Tuple<Args...> > &t)
{
    glUseProgram(T::getInstance()->Program);
    if (irr_driver->hasARB_base_instance())
        glBindVertexArray(VAOManager::getInstance()->getVAO(VertexType));
    for (unsigned i = 0; i < t.size(); i++)
    {
        std::vector<GLuint> Textures;
        GLMesh *mesh = STK::tuple_get<0>(t.at(i));
        if (!irr_driver->hasARB_base_instance())
            glBindVertexArray(mesh->vao);
        for (unsigned j = 0; j < TextureUnits.size(); j++)
            Textures.push_back(getTextureGLuint(mesh->textures[TextureUnits[j]]));
        T::getInstance()->SetTextureUnits(Textures);
        rsm_custom_unroll_args<Selector...>::template exec<T>(rsm_matrix, t.at(i));
    }
}

template<typename Shader, MeshMaterial Mat, video::E_VERTEX_TYPE VT, typename...Args>
void renderRSMShadow(const std::vector<GLuint> TextureUnits, const std::vector<GLMesh *> &t, Args ...args)
{
    glUseProgram(Shader::getInstance()->Program);
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(VT, InstanceTypeRSM));
    for (unsigned i = 0; i < t.size(); i++)
    {
        std::vector<uint64_t> Handles;
        std::vector<GLuint> Textures;
        GLMesh *mesh = t[i];

        for (unsigned j = 0; j < TextureUnits.size(); j++)
            Textures.push_back(getTextureGLuint(mesh->textures[TextureUnits[j]]));

        Shader::getInstance()->SetTextureUnits(Textures);
        Shader::getInstance()->setUniforms(args...);
        glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (const void*)((RSMPassCmd::getInstance()->Offset[Mat] + i)* sizeof(DrawElementsIndirectCommand)));
    }
}

template<typename Shader, MeshMaterial Mat, enum E_VERTEX_TYPE VertexType, typename... Args>
void multidrawRSM(Args...args)
{
    glUseProgram(Shader::getInstance()->Program);
    glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(VertexType, InstanceTypeRSM));
    if (RSMPassCmd::getInstance()->Size[Mat])
    {
        Shader::getInstance()->setUniforms(args...);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, (const void*)(RSMPassCmd::getInstance()->Offset[Mat] * sizeof(DrawElementsIndirectCommand)), RSMPassCmd::getInstance()->Size[Mat], sizeof(DrawElementsIndirectCommand));
    }
}

void IrrDriver::renderRSM()
{
    ScopedGPUTimer Timer(getGPUTimer(Q_RSM));
    m_rtts->getRSM().Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawRSM<MeshShader::RSMShader, EVT_STANDARD, 3, 1>(rsm_matrix, std::vector<GLuint>{ 0 }, ListMatDefault::getInstance()->RSM);
    drawRSM<MeshShader::RSMShader, EVT_STANDARD, 3, 1>(rsm_matrix, std::vector<GLuint>{ 0 }, ListMatAlphaRef::getInstance()->RSM);
    drawRSM<MeshShader::RSMShader, EVT_TANGENTS, 3, 1>(rsm_matrix, std::vector<GLuint>{ 0 }, ListMatNormalMap::getInstance()->RSM);
    drawRSM<MeshShader::RSMShader, EVT_STANDARD, 3, 1>(rsm_matrix, std::vector<GLuint>{ 0 }, ListMatUnlit::getInstance()->RSM);
    drawRSM<MeshShader::RSMShader, EVT_2TCOORDS, 3, 1>(rsm_matrix, std::vector<GLuint>{ 0 }, ListMatDetails::getInstance()->RSM);
    drawRSM<MeshShader::SplattingRSMShader, EVT_2TCOORDS, 1>(rsm_matrix, createVector<GLuint>(1, 2, 3, 4, 5), ListMatSplatting::getInstance()->RSM);

    if (irr_driver->hasARB_draw_indirect())
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, RSMPassCmd::getInstance()->drawindirectcmd);

    if (UserConfigParams::m_azdo)
    {
        multidrawRSM<MeshShader::InstancedRSMShader, MAT_DEFAULT, video::EVT_STANDARD>(rsm_matrix);
        multidrawRSM<MeshShader::InstancedRSMShader, MAT_NORMAL_MAP, video::EVT_TANGENTS>(rsm_matrix);
        multidrawRSM<MeshShader::InstancedRSMShader, MAT_ALPHA_REF, video::EVT_STANDARD>(rsm_matrix);
        multidrawRSM<MeshShader::InstancedRSMShader, MAT_UNLIT, video::EVT_STANDARD>(rsm_matrix);
        multidrawRSM<MeshShader::InstancedRSMShader, MAT_DETAIL, video::EVT_2TCOORDS>(rsm_matrix);
    }
    else if (irr_driver->hasARB_draw_indirect())
    {
        renderRSMShadow<MeshShader::InstancedRSMShader, MAT_DEFAULT, video::EVT_STANDARD>(std::vector < GLuint > { 0 }, ListInstancedMatDefault::getInstance()->RSM, rsm_matrix);
        renderRSMShadow<MeshShader::InstancedRSMShader, MAT_ALPHA_REF, video::EVT_STANDARD>(std::vector < GLuint > { 0 }, ListInstancedMatAlphaRef::getInstance()->RSM, rsm_matrix);
        renderRSMShadow<MeshShader::InstancedRSMShader, MAT_UNLIT, video::EVT_STANDARD>(std::vector < GLuint > { 0 }, ListInstancedMatUnlit::getInstance()->RSM, rsm_matrix);
        renderRSMShadow<MeshShader::InstancedRSMShader, MAT_NORMAL_MAP, video::EVT_TANGENTS>(std::vector < GLuint > { 0 }, ListInstancedMatNormalMap::getInstance()->RSM, rsm_matrix);
        renderRSMShadow<MeshShader::InstancedRSMShader, MAT_DETAIL, video::EVT_2TCOORDS>(std::vector < GLuint > { 0 }, ListInstancedMatDetails::getInstance()->RSM, rsm_matrix);
    }
}
