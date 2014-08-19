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
#include "graphics/stkinstancedscenenode.hpp"
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

#include <algorithm>

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
void renderMeshes1stPass(const std::vector<TexUnit> &TexUnits, std::vector<STK::Tuple<TupleType...> > *meshes)
{
    glUseProgram(Shader::getInstance()->Program);
    glBindVertexArray(getVAO(VertexType));
    for (unsigned i = 0; i < meshes->size(); i++)
    {
        std::vector<GLuint> Textures;
        GLMesh &mesh = *(STK::tuple_get<0>(meshes->at(i)));
        for (unsigned j = 0; j < TexUnits.size(); j++)
        {
            if (!mesh.textures[j])
                mesh.textures[j] = getUnicolorTexture(video::SColor(255, 255, 255, 255));
            compressTexture(mesh.textures[j], TexUnits[j].m_premul_alpha);
            Textures.push_back(getTextureGLuint(mesh.textures[TexUnits[j].m_id]));
        }
        if (mesh.VAOType != VertexType)
        {
#ifdef DEBUG
            Log::error("Materials", "Wrong vertex Type associed to pass 1 (hint texture : %s)", mesh.textures[0]->getName().getPath().c_str());
#endif
            continue;
        }
        Shader::getInstance()->SetTextureUnits(Textures);
        custom_unroll_args<List...>::template exec(Shader::getInstance(), meshes->at(i));
    }
}

template<int...List>
struct instanced_custom_unroll_args;

template<>
struct instanced_custom_unroll_args<>
{
    template<typename T, typename ...TupleTypes, typename ...Args>
    static void exec(const T *Shader, const STK::Tuple<TupleTypes...> &t, Args... args)
    {
        const GLMesh *mesh = STK::tuple_get<0>(t);
        size_t instance_count = STK::tuple_get<1>(t);
        irr_driver->IncreaseObjectCount();
        GLenum ptype = mesh->PrimitiveType;
        GLenum itype = mesh->IndexType;
        size_t count = mesh->IndexCount;

        Shader->setUniforms(args...);
        glDrawElementsInstanced(ptype, count, itype, 0, instance_count);
    }
};

template<int N, int...List>
struct instanced_custom_unroll_args<N, List...>
{
    template<typename T, typename ...TupleTypes, typename ...Args>
    static void exec(const T *Shader, const STK::Tuple<TupleTypes...> &t, Args... args)
    {
        instanced_custom_unroll_args<List...>::template exec<T>(Shader, t, STK::tuple_get<N>(t), args...);
    }
};

template<typename Shader, enum E_VERTEX_TYPE VertexType, int ...List, typename... TupleType>
void renderInstancedMeshes1stPass(const std::vector<TexUnit> &TexUnits, std::vector<STK::Tuple<TupleType...> > *meshes)
{
    glUseProgram(Shader::getInstance()->Program);
    for (unsigned i = 0; i < meshes->size(); i++)
    {
        std::vector<GLuint> Textures;
        GLMesh &mesh = *(STK::tuple_get<0>(meshes->at(i)));
#ifdef DEBUG
        if (mesh.VAOType != VertexType)
            Log::error("RenderGeometry", "Wrong instanced vertex format");
#endif
        glBindVertexArray(mesh.vao);
        for (unsigned j = 0; j < TexUnits.size(); j++)
        {
            if (!mesh.textures[TexUnits[j].m_id])
                mesh.textures[j] = getUnicolorTexture(video::SColor(255, 255, 255, 255));
            compressTexture(mesh.textures[TexUnits[j].m_id], TexUnits[j].m_premul_alpha);
            Textures.push_back(getTextureGLuint(mesh.textures[TexUnits[j].m_id]));
        }
        Shader::getInstance()->SetTextureUnits(Textures);
        instanced_custom_unroll_args<List...>::template exec(Shader::getInstance(), meshes->at(i));
    }
}

static GLsync m_sync;

void IrrDriver::renderSolidFirstPass()
{
    m_rtts->getFBO(FBO_NORMAL_AND_DEPTHS).Bind();
    glClearColor(0., 0., 0., 0.);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    irr_driver->setPhase(SOLID_NORMAL_AND_DEPTH_PASS);
    ListMatDefault::getInstance()->clear();
    ListMatAlphaRef::getInstance()->clear();
    ListMatSphereMap::getInstance()->clear();
    ListMatDetails::getInstance()->clear();
    ListMatUnlit::getInstance()->clear();
    ListMatNormalMap::getInstance()->clear();
    ListMatGrass::getInstance()->clear();
    ListMatSplatting::getInstance()->clear();
    ListInstancedMatDefault::getInstance()->clear();
    ListInstancedMatAlphaRef::getInstance()->clear();
    ListInstancedMatGrass::getInstance()->clear();
    ListInstancedMatNormalMap::getInstance()->clear();
    AnimatedListMatDefault::getInstance()->clear();
    AnimatedListMatAlphaRef::getInstance()->clear();
    AnimatedListMatDetails::getInstance()->clear();
    AnimatedListMatUnlit::getInstance()->clear();
    // Add a 30 ms timeout
    GLenum reason = glClientWaitSync(m_sync, GL_SYNC_FLUSH_COMMANDS_BIT, 30000000);
/*    switch (reason)
    {
    case GL_ALREADY_SIGNALED:
        printf("Already Signaled\n");
        break;
    case GL_TIMEOUT_EXPIRED:
        printf("Timeout Expired\n");
        break;
    case GL_CONDITION_SATISFIED:
        printf("Condition Satisfied\n");
        break;
    case GL_WAIT_FAILED:
        printf("Wait Failed\n");
        break;
    }*/
    m_scene_manager->drawAll(scene::ESNRP_SOLID);

    if (!UserConfigParams::m_dynamic_lights)
        return;

    {
        ScopedGPUTimer Timer(getGPUTimer(Q_SOLID_PASS1));

        std::vector<TexUnit> object_pass1_texunits = TexUnits(TexUnit(0, true) );
        renderMeshes1stPass<MeshShader::ObjectPass1Shader, video::EVT_STANDARD, 2, 1>(object_pass1_texunits, ListMatDefault::getInstance());
        renderMeshes1stPass<MeshShader::ObjectPass1Shader, video::EVT_STANDARD, 2, 1>(object_pass1_texunits, ListMatSphereMap::getInstance());
        renderMeshes1stPass<MeshShader::ObjectPass1Shader, video::EVT_2TCOORDS, 2, 1>(object_pass1_texunits, ListMatDetails::getInstance());
        renderMeshes1stPass<MeshShader::ObjectPass1Shader, video::EVT_2TCOORDS, 2, 1>(object_pass1_texunits, ListMatSplatting::getInstance());
        renderMeshes1stPass<MeshShader::ObjectRefPass1Shader, video::EVT_STANDARD, 3, 2, 1>(object_pass1_texunits, ListMatUnlit::getInstance());
        renderMeshes1stPass<MeshShader::ObjectRefPass1Shader, video::EVT_STANDARD, 3, 2, 1>(TexUnits(TexUnit(0, true)), ListMatAlphaRef::getInstance());
        renderMeshes1stPass<MeshShader::GrassPass1Shader, video::EVT_STANDARD, 3, 2, 1>(TexUnits(TexUnit(0, true)), ListMatGrass::getInstance());
        renderMeshes1stPass<MeshShader::NormalMapShader, video::EVT_TANGENTS, 2, 1>(TexUnits(
            TexUnit(1, false),
            TexUnit(0, true)
        ), ListMatNormalMap::getInstance());

        renderMeshes1stPass<MeshShader::ObjectPass1Shader, video::EVT_STANDARD, 2, 1>(TexUnits(TexUnit(0, true)), AnimatedListMatDefault::getInstance());
        renderMeshes1stPass<MeshShader::ObjectRefPass1Shader, video::EVT_STANDARD, 3, 2, 1>(TexUnits(TexUnit(0, true)), AnimatedListMatAlphaRef::getInstance());
        renderMeshes1stPass<MeshShader::ObjectPass1Shader, video::EVT_2TCOORDS, 2, 1>(TexUnits(TexUnit(0, true)), AnimatedListMatDetails::getInstance());
        renderMeshes1stPass<MeshShader::ObjectRefPass1Shader, video::EVT_STANDARD, 3, 2, 1>(TexUnits(TexUnit(0, true)), AnimatedListMatUnlit::getInstance());

        renderInstancedMeshes1stPass<MeshShader::InstancedObjectPass1Shader, video::EVT_STANDARD>(
                    TexUnits(TexUnit(0, true)),
                    ListInstancedMatDefault::getInstance());
        renderInstancedMeshes1stPass<MeshShader::InstancedObjectRefPass1Shader, video::EVT_STANDARD>(
                    TexUnits(TexUnit(0, true)),
                    ListInstancedMatAlphaRef::getInstance());
        renderInstancedMeshes1stPass<MeshShader::InstancedGrassPass1Shader, video::EVT_STANDARD, 2>(
                    TexUnits(TexUnit(0, true)),
                    ListInstancedMatGrass::getInstance());
        renderInstancedMeshes1stPass<MeshShader::InstancedNormalMapShader, video::EVT_TANGENTS>(
            TexUnits(TexUnit(1, false), TexUnit(0, true)),
            ListInstancedMatNormalMap::getInstance());
    }
}

template<typename Shader, enum E_VERTEX_TYPE VertexType, int...List, typename... TupleType>
void renderMeshes2ndPass(const std::vector<TexUnit> &TexUnits, std::vector<STK::Tuple<TupleType...> > *meshes)
{
    glUseProgram(Shader::getInstance()->Program);
    glBindVertexArray(getVAO(VertexType));
    for (unsigned i = 0; i < meshes->size(); i++)
    {
        std::vector<GLuint> Textures;
        GLMesh &mesh = *(STK::tuple_get<0>(meshes->at(i)));
        for (unsigned j = 0; j < TexUnits.size(); j++)
        {
            if (!mesh.textures[TexUnits[j].m_id])
                mesh.textures[TexUnits[j].m_id] = getUnicolorTexture(video::SColor(255, 255, 255, 255));
            compressTexture(mesh.textures[TexUnits[j].m_id], TexUnits[j].m_premul_alpha);
            Textures.push_back(getTextureGLuint(mesh.textures[TexUnits[j].m_id]));
            if (irr_driver->getLightViz())
            {
                GLint swizzleMask[] = { GL_ONE, GL_ONE, GL_ONE, GL_ALPHA };
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
            else
            {
                GLint swizzleMask[] = { GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
        }

        if (mesh.VAOType != VertexType)
        {
#ifdef DEBUG
            Log::error("Materials", "Wrong vertex Type associed to pass 2 (hint texture : %s)", mesh.textures[0]->getName().getPath().c_str());
#endif
            continue;
        }
        Shader::getInstance()->SetTextureUnits(Textures);
        custom_unroll_args<List...>::template exec(Shader::getInstance(), meshes->at(i));
    }
}

template<typename Shader, int...List, typename... TupleType>
void renderInstancedMeshes2ndPass(const std::vector<TexUnit> &TexUnits, std::vector<STK::Tuple<TupleType...> > *meshes)
{
    glUseProgram(Shader::getInstance()->Program);
    for (unsigned i = 0; i < meshes->size(); i++)
    {
        GLMesh &mesh = *(STK::tuple_get<0>(meshes->at(i)));
        glBindVertexArray(mesh.vao);
        std::vector<GLuint> Textures;
        for (unsigned j = 0; j < TexUnits.size(); j++)
        {
            if (!mesh.textures[TexUnits[j].m_id])
                mesh.textures[TexUnits[j].m_id] = getUnicolorTexture(video::SColor(255, 255, 255, 255));
            compressTexture(mesh.textures[TexUnits[j].m_id], TexUnits[j].m_premul_alpha);
            Textures.push_back(getTextureGLuint(mesh.textures[TexUnits[j].m_id]));
            if (irr_driver->getLightViz())
            {
                GLint swizzleMask[] = { GL_ONE, GL_ONE, GL_ONE, GL_ALPHA };
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
            else
            {
                GLint swizzleMask[] = { GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA };
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
            }
        }
        Shader::getInstance()->SetTextureUnits(Textures);
        instanced_custom_unroll_args<List...>::template exec(Shader::getInstance(), meshes->at(i));
    }
}

void IrrDriver::renderSolidSecondPass()
{
    SColor clearColor(0, 150, 150, 150);
    if (World::getWorld() != NULL)
        clearColor = World::getWorld()->getClearColor();

    glClearColor(clearColor.getRed() / 255.f, clearColor.getGreen() / 255.f,
        clearColor.getBlue() / 255.f, clearColor.getAlpha() / 255.f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (UserConfigParams::m_dynamic_lights)
        glDepthMask(GL_FALSE);
    else
    {
        glDepthMask(GL_TRUE);
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    irr_driver->setPhase(SOLID_LIT_PASS);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glBindSampler(0, 0);
    setTexture(0, m_rtts->getRenderTarget(RTT_TMP1), GL_NEAREST, GL_NEAREST);
    glBindSampler(1, 0);
    setTexture(1, m_rtts->getRenderTarget(RTT_TMP2), GL_NEAREST, GL_NEAREST);
    glBindSampler(2, 0);
    setTexture(2, m_rtts->getRenderTarget(RTT_HALF1_R), GL_LINEAR, GL_LINEAR);

    {
        ScopedGPUTimer Timer(getGPUTimer(Q_SOLID_PASS2));

        m_scene_manager->drawAll(scene::ESNRP_SOLID);

        renderMeshes2ndPass<MeshShader::ObjectPass2Shader, video::EVT_STANDARD, 3, 1>(TexUnits(
            TexUnit(0, true)
        ), ListMatDefault::getInstance());
        renderMeshes2ndPass<MeshShader::ObjectPass2Shader, video::EVT_STANDARD, 3, 1>(TexUnits(
            TexUnit(0, true)
            ), AnimatedListMatDefault::getInstance());

        renderMeshes2ndPass<MeshShader::ObjectRefPass2Shader, video::EVT_STANDARD, 3, 1 >(TexUnits(
            TexUnit(0, true)
        ), ListMatAlphaRef::getInstance());
        renderMeshes2ndPass<MeshShader::ObjectRefPass2Shader, video::EVT_STANDARD, 3, 1 >(TexUnits(
            TexUnit(0, true)
            ), AnimatedListMatAlphaRef::getInstance());

        renderMeshes2ndPass<MeshShader::SphereMapShader, video::EVT_STANDARD, 2, 1>(TexUnits(
            TexUnit(0, true)
        ), ListMatSphereMap::getInstance());

        renderMeshes2ndPass<MeshShader::DetailledObjectPass2Shader, video::EVT_2TCOORDS, 1>(TexUnits(
            TexUnit(0, true),
            TexUnit(1, true)
        ), ListMatDetails::getInstance());
        renderMeshes2ndPass<MeshShader::DetailledObjectPass2Shader, video::EVT_2TCOORDS, 1>(TexUnits(
            TexUnit(0, true),
            TexUnit(1, true)
            ), AnimatedListMatDetails::getInstance());

        renderMeshes2ndPass<MeshShader::GrassPass2Shader, video::EVT_STANDARD, 3, 1>(TexUnits(
            TexUnit(0, true)
        ), ListMatGrass::getInstance());

        renderMeshes2ndPass<MeshShader::ObjectUnlitShader, video::EVT_STANDARD, 3, 1>(TexUnits(
            TexUnit(0, true)
        ), ListMatUnlit::getInstance());
        renderMeshes2ndPass<MeshShader::ObjectUnlitShader, video::EVT_STANDARD, 3, 1>(TexUnits(
            TexUnit(0, true)
            ), AnimatedListMatUnlit::getInstance());

        renderMeshes2ndPass<MeshShader::SplattingShader, video::EVT_2TCOORDS, 1>(TexUnits(
            TexUnit(1, false),
            TexUnit(2, true),
            TexUnit(3, true),
            TexUnit(4, true),
            TexUnit(5, true)
        ), ListMatSplatting::getInstance());

        renderMeshes2ndPass<MeshShader::ObjectPass2Shader, video::EVT_TANGENTS, 3, 1>(TexUnits(
            TexUnit(0, true)
        ), ListMatNormalMap::getInstance());


        renderInstancedMeshes2ndPass<MeshShader::InstancedObjectPass2Shader>(
            TexUnits(TexUnit(0, true)),
            ListInstancedMatDefault::getInstance());
        renderInstancedMeshes2ndPass<MeshShader::InstancedObjectPass2Shader>(
            TexUnits(TexUnit(0, true)),
            ListInstancedMatNormalMap::getInstance());
        renderInstancedMeshes2ndPass<MeshShader::InstancedObjectRefPass2Shader>(
            TexUnits(TexUnit(0, true)),
            ListInstancedMatAlphaRef::getInstance());
        setTexture(4, irr_driver->getDepthStencilTexture(), GL_NEAREST, GL_NEAREST);
        renderInstancedMeshes2ndPass<MeshShader::InstancedGrassPass2Shader, 3, 2>(
            TexUnits(TexUnit(0, true)),
            ListInstancedMatGrass::getInstance());
    }
    m_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

template<enum E_VERTEX_TYPE VertexType, typename... TupleType>
static void renderMeshNormals(std::vector<STK::Tuple<TupleType...> > *meshes)
{
    glUseProgram(MeshShader::NormalVisualizer::getInstance()->Program);
    glBindVertexArray(getVAO(VertexType));
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
    renderMeshNormals<video::EVT_STANDARD>(ListMatDefault::getInstance());
    renderMeshNormals<video::EVT_STANDARD>(ListMatAlphaRef::getInstance());
    renderMeshNormals<video::EVT_STANDARD>(ListMatSphereMap::getInstance());
//    renderMeshNormals<video::EVT_STANDARD>(ListMatGrass::getInstance());
    renderMeshNormals<video::EVT_2TCOORDS>(ListMatDetails::getInstance());
    renderMeshNormals<video::EVT_STANDARD>(ListMatUnlit::getInstance());
    renderMeshNormals<video::EVT_2TCOORDS>(ListMatSplatting::getInstance());
    renderMeshNormals<video::EVT_TANGENTS>(ListMatNormalMap::getInstance());

}



static video::ITexture *displaceTex = 0;

void IrrDriver::renderTransparent()
{
    irr_driver->setPhase(TRANSPARENT_PASS);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_ALPHA_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glDisable(GL_CULL_FACE);
    ListBlendTransparent::getInstance()->clear();
    ListAdditiveTransparent::getInstance()->clear();
    ListBlendTransparentFog::getInstance()->clear();
    ListAdditiveTransparentFog::getInstance()->clear();
    ListDisplacement::getInstance()->clear();
    m_scene_manager->drawAll(scene::ESNRP_TRANSPARENT);

    glBindVertexArray(getVAO(EVT_STANDARD));

    if (World::getWorld() && World::getWorld()->isFogEnabled())
    {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        renderMeshes2ndPass<MeshShader::TransparentFogShader, video::EVT_STANDARD, 8, 7, 6, 5, 4, 3, 2, 1>(TexUnits(
            TexUnit(0, true)
        ), ListBlendTransparentFog::getInstance());
        glBlendFunc(GL_ONE, GL_ONE);
        renderMeshes2ndPass<MeshShader::TransparentFogShader, video::EVT_STANDARD, 8, 7, 6, 5, 4, 3, 2, 1>(TexUnits(
            TexUnit(0, true)
        ), ListAdditiveTransparentFog::getInstance());
    }
    else
    {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        renderMeshes2ndPass<MeshShader::TransparentShader, video::EVT_STANDARD, 2, 1>(TexUnits(
            TexUnit(0, true)
        ), ListBlendTransparent::getInstance());
        glBlendFunc(GL_ONE, GL_ONE);
        renderMeshes2ndPass<MeshShader::TransparentShader, video::EVT_STANDARD, 2, 1>(TexUnits(
            TexUnit(0, true)
        ), ListAdditiveTransparent::getInstance());
    }

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

    glBindVertexArray(getVAO(EVT_2TCOORDS));
    // Generate displace mask
    // Use RTT_TMP4 as displace mask
    irr_driver->getFBO(FBO_TMP1_WITH_DS).Bind();
    for (unsigned i = 0; i < ListDisplacement::getInstance()->size(); i++)
    {
        const GLMesh &mesh = *(STK::tuple_get<0>(ListDisplacement::getInstance()->at(i)));
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
        const core::matrix4 &AbsoluteTransformation = STK::tuple_get<1>(ListDisplacement::getInstance()->at(i));
        if (mesh.VAOType != video::EVT_2TCOORDS)
            continue;

        GLenum ptype = mesh.PrimitiveType;
        GLenum itype = mesh.IndexType;
        size_t count = mesh.IndexCount;
        // Render the effect
        MeshShader::DisplaceShader::getInstance()->SetTextureUnits(
            { getTextureGLuint(displaceTex), irr_driver->getRenderTargetTexture(RTT_COLOR), irr_driver->getRenderTargetTexture(RTT_TMP1), getTextureGLuint(mesh.textures[0]) });
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
void drawShadow(const T *Shader, const GLMesh *mesh, uniforms... Args)
{
    irr_driver->IncreaseObjectCount();
    GLenum ptype = mesh->PrimitiveType;
    GLenum itype = mesh->IndexType;
    size_t count = mesh->IndexCount;

    Shader->setUniforms(Args...);
    glDrawElementsInstancedBaseVertex(ptype, count, itype, (GLvoid *)mesh->vaoOffset, 4, mesh->vaoBaseVertex);
}

template<int...List>
struct shadow_custom_unroll_args;

template<>
struct shadow_custom_unroll_args<>
{
    template<typename T, typename ...TupleTypes, typename ...Args>
    static void exec(const T *Shader, const STK::Tuple<TupleTypes...> &t, Args... args)
    {
        drawShadow<T>(Shader, STK::tuple_get<0>(t), args...);
    }
};

template<int N, int...List>
struct shadow_custom_unroll_args<N, List...>
{
    template<typename T, typename ...TupleTypes, typename ...Args>
    static void exec(const T *Shader, const STK::Tuple<TupleTypes...> &t, Args... args)
    {
        shadow_custom_unroll_args<List...>::template exec<T>(Shader, t, STK::tuple_get<N>(t), args...);
    }
};

template<typename T, enum E_VERTEX_TYPE VertexType, int...List, typename... Args>
void renderShadow(const std::vector<GLuint> TextureUnits, const std::vector<STK::Tuple<Args...> > *t)
{
    glUseProgram(T::getInstance()->Program);
    glBindVertexArray(getVAO(VertexType));
    for (unsigned i = 0; i < t->size(); i++)
    {
        std::vector<GLuint> Textures;
        const GLMesh *mesh = STK::tuple_get<0>(t->at(i));
        for (unsigned j = 0; j < TextureUnits.size(); j++)
        {
            compressTexture(mesh->textures[TextureUnits[j]], true);
            Textures.push_back(getTextureGLuint(mesh->textures[TextureUnits[j]]));
        }
        T::getInstance()->SetTextureUnits(Textures);
        shadow_custom_unroll_args<List...>::template exec<T>(T::getInstance(), t->at(i));
    }
}

template<int...List>
struct instanced_shadow_custom_unroll_args;

template<>
struct instanced_shadow_custom_unroll_args<>
{
    template<typename T, typename ...TupleTypes, typename ...Args>
    static void exec(const T *Shader, const STK::Tuple<TupleTypes...> &t, Args... args)
    {
        const GLMesh *mesh = STK::tuple_get<0>(t);
        size_t instance_count = STK::tuple_get<1>(t);
        irr_driver->IncreaseObjectCount();
        GLenum ptype = mesh->PrimitiveType;
        GLenum itype = mesh->IndexType;
        size_t count = mesh->IndexCount;

        Shader->setUniforms(args...);
        glDrawElementsInstanced(ptype, count, itype, 0, 4 * instance_count);
    }
};

template<int N, int...List>
struct instanced_shadow_custom_unroll_args<N, List...>
{
    template<typename T, typename ...TupleTypes, typename ...Args>
    static void exec(const T *Shader, const STK::Tuple<TupleTypes...> &t, Args... args)
    {
        instanced_shadow_custom_unroll_args<List...>::template exec<T>(Shader, t, STK::tuple_get<N>(t), args...);
    }
};

template<typename T, int...List, typename... Args>
void renderInstancedShadow(const std::vector<GLuint> TextureUnits, const std::vector<STK::Tuple<Args...> > *t)
{
    glUseProgram(T::getInstance()->Program);
    for (unsigned i = 0; i < t->size(); i++)
    {
        std::vector<GLuint> Textures;
        const GLMesh *mesh = STK::tuple_get<0>(t->at(i));
        glBindVertexArray(mesh->vao_shadow_pass);
        for (unsigned j = 0; j < TextureUnits.size(); j++)
        {
            compressTexture(mesh->textures[TextureUnits[j]], true);
            Textures.push_back(getTextureGLuint(mesh->textures[TextureUnits[j]]));
        }
        T::getInstance()->SetTextureUnits(Textures);
        instanced_shadow_custom_unroll_args<List...>::template exec<T>(T::getInstance(), t->at(i));
    }
}

void IrrDriver::renderShadows()
{
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

    irr_driver->setPhase(SHADOW_PASS);
    ListMatDefault::getInstance()->clear();
    ListMatAlphaRef::getInstance()->clear();
    ListMatSphereMap::getInstance()->clear();
    ListMatDetails::getInstance()->clear();
    ListMatUnlit::getInstance()->clear();
    ListMatNormalMap::getInstance()->clear();
    ListMatGrass::getInstance()->clear();
    ListMatSplatting::getInstance()->clear();
    ListInstancedMatDefault::getInstance()->clear();
    ListInstancedMatAlphaRef::getInstance()->clear();
    ListInstancedMatGrass::getInstance()->clear();
    ListInstancedMatNormalMap::getInstance()->clear();
    AnimatedListMatDefault::getInstance()->clear();
    AnimatedListMatAlphaRef::getInstance()->clear();
    AnimatedListMatDetails::getInstance()->clear();
    AnimatedListMatUnlit::getInstance()->clear();
    m_scene_manager->drawAll(scene::ESNRP_SOLID);

    std::vector<GLuint> noTexUnits;
    renderShadow<MeshShader::ShadowShader, EVT_STANDARD, 1>(noTexUnits, ListMatDefault::getInstance());
    renderShadow<MeshShader::ShadowShader, EVT_STANDARD, 1>(noTexUnits, ListMatSphereMap::getInstance());
    renderShadow<MeshShader::ShadowShader, EVT_2TCOORDS, 1>(noTexUnits, ListMatDetails::getInstance());
    renderShadow<MeshShader::ShadowShader, EVT_2TCOORDS, 1>(noTexUnits, ListMatSplatting::getInstance());
    renderShadow<MeshShader::ShadowShader, EVT_TANGENTS, 1>(noTexUnits, ListMatNormalMap::getInstance());
    renderShadow<MeshShader::RefShadowShader, EVT_STANDARD, 1>(std::vector<GLuint>{ 0 }, ListMatAlphaRef::getInstance());
    renderShadow<MeshShader::RefShadowShader, EVT_STANDARD, 1>(std::vector<GLuint>{ 0 }, ListMatUnlit::getInstance());
    renderShadow<MeshShader::GrassShadowShader, EVT_STANDARD, 3, 1>(std::vector<GLuint>{ 0 }, ListMatGrass::getInstance());

    renderShadow<MeshShader::ShadowShader, EVT_STANDARD, 1>(noTexUnits, AnimatedListMatDefault::getInstance());
    renderShadow<MeshShader::RefShadowShader, EVT_STANDARD, 1>(std::vector<GLuint>{ 0 }, AnimatedListMatAlphaRef::getInstance());
    renderShadow<MeshShader::RefShadowShader, EVT_STANDARD, 1>(std::vector<GLuint>{ 0 }, AnimatedListMatUnlit::getInstance());
    renderShadow<MeshShader::ShadowShader, EVT_2TCOORDS, 1>(noTexUnits, AnimatedListMatDetails::getInstance());

    renderInstancedShadow<MeshShader::InstancedShadowShader>(noTexUnits, ListInstancedMatDefault::getInstance());
    renderInstancedShadow<MeshShader::InstancedRefShadowShader>(std::vector<GLuint>{ 0 }, ListInstancedMatAlphaRef::getInstance());
    renderInstancedShadow<MeshShader::InstancedGrassShadowShader, 2>(std::vector<GLuint>{ 0 }, ListInstancedMatGrass::getInstance());
    renderInstancedShadow<MeshShader::InstancedShadowShader>(noTexUnits, ListInstancedMatNormalMap::getInstance());

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
void drawRSM(const core::matrix4 & rsm_matrix, const std::vector<GLuint> &TextureUnits, std::vector<STK::Tuple<Args...> > *t)
{
    glUseProgram(T::getInstance()->Program);
    glBindVertexArray(getVAO(VertexType));
    for (unsigned i = 0; i < t->size(); i++)
    {
        std::vector<GLuint> Textures;
        GLMesh *mesh = STK::tuple_get<0>(t->at(i));
        for (unsigned j = 0; j < TextureUnits.size(); j++)
        {
            if (!mesh->textures[TextureUnits[j]])
                mesh->textures[TextureUnits[j]] = getUnicolorTexture(video::SColor(255, 255, 255, 255));
            compressTexture(mesh->textures[TextureUnits[j]], true);
            Textures.push_back(getTextureGLuint(mesh->textures[TextureUnits[j]]));
        }
        T::getInstance()->SetTextureUnits(Textures);
        rsm_custom_unroll_args<Selector...>::template exec<T>(rsm_matrix, t->at(i));
    }
}

void IrrDriver::renderRSM()
{
    m_rtts->getRSM().Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawRSM<MeshShader::RSMShader, EVT_STANDARD, 3, 1>(rsm_matrix, std::vector<GLuint>{ 0 }, ListMatDefault::getInstance());
    drawRSM<MeshShader::RSMShader, EVT_STANDARD, 3, 1>(rsm_matrix, std::vector<GLuint>{ 0 }, ListMatAlphaRef::getInstance());
    drawRSM<MeshShader::RSMShader, EVT_TANGENTS, 3, 1>(rsm_matrix, std::vector<GLuint>{ 0 }, ListMatNormalMap::getInstance());
    drawRSM<MeshShader::RSMShader, EVT_STANDARD, 3, 1>(rsm_matrix, std::vector<GLuint>{ 0 }, ListMatUnlit::getInstance());
    drawRSM<MeshShader::RSMShader, EVT_2TCOORDS, 3, 1>(rsm_matrix, std::vector<GLuint>{ 0 }, ListMatDetails::getInstance());
    drawRSM<MeshShader::SplattingRSMShader, EVT_2TCOORDS, 1>(rsm_matrix, createVector<GLuint>(1, 2, 3, 4, 5), ListMatSplatting::getInstance());
}
