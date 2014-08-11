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
#include <functional>

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
struct remap_tuple;

template<>
struct remap_tuple<>
{
    template<typename...OriginalTuples, typename...Args>
    static STK::Tuple <Args...> exec(const STK::Tuple<OriginalTuples...> &oldt, Args... args)
    {
        return STK::make_tuple(args...);
    }
};

template<int N, int...List>
struct remap_tuple<N, List...>
{
    template<typename...OriginalTuples, typename...Args>
    static auto exec(const STK::Tuple<OriginalTuples...> &oldt, Args...args)
        -> decltype(remap_tuple<List...>::template exec(oldt, STK::tuple_get<N>(oldt), args...))
    {
        return remap_tuple<List...>::template exec(oldt, STK::tuple_get<N>(oldt), args...);
    }
};

template<int N>
struct unroll_args_impl;

template<>
struct unroll_args_impl<0>
{
    template<typename ...Args>
    static void exec(std::function<void (Args...)> f, const STK::Tuple<Args...> &t, Args... args)
    {
        f(args...);
    }
};

template<int N>
struct unroll_args_impl
{
    template<typename ...TupleType, typename ...Args>
    static void exec(std::function<void (TupleType...)> f, const STK::Tuple<TupleType...> &t, Args... args)
    {
        unroll_args_impl<N - 1>::exec(f, t, STK::tuple_get<N - 1>(t), args...);
    }
};

template<typename...Args>
static void unroll_args(std::function<void (Args...)> f, const STK::Tuple<Args...> &t)
{
    unroll_args_impl<sizeof...(Args) >::exec(f, t);
}

template<typename T, typename... Args>
void apply_instance(const T *Shader, const GLMesh *mesh, const STK::Tuple<Args...> &t)
{
    std::function<void (Args...)> lambda = [&](Args...args) { draw<T>(Shader, mesh, args...); };
    unroll_args(lambda, t);
}

template<typename Shader, enum E_VERTEX_TYPE VertexType, int ...List, typename... TupleType>
void renderMeshes1stPass(const std::vector<TexUnit> &TexUnits, std::vector<STK::Tuple<TupleType...> > &meshes)
{
    glUseProgram(Shader::getInstance()->Program);
    glBindVertexArray(getVAO(VertexType));
    for (unsigned i = 0; i < meshes.size(); i++)
    {
        GLMesh &mesh = *(STK::tuple_get<0>(meshes[i]));
        for (unsigned j = 0; j < TexUnits.size(); j++)
        {
            if (!mesh.textures[j])
                mesh.textures[j] = getUnicolorTexture(video::SColor(255, 255, 255, 255));
            compressTexture(mesh.textures[j], TexUnits[j].m_premul_alpha);
            setTexture(TexUnits[j].m_id, getTextureGLuint(mesh.textures[j]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
        }
        if (mesh.VAOType != VertexType)
        {
#ifdef DEBUG
            Log::error("Materials", "Wrong vertex Type associed to pass 1 (hint texture : %s)", mesh.textures[0]->getName().getPath().c_str());
#endif
            continue;
        }
        auto remapped_tuple = remap_tuple<List...>::exec(meshes[i]);
        apply_instance<Shader>(Shader::getInstance(), &mesh, remapped_tuple);
    }
}

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
    ListMatDefault::Arguments.clear();
    ListMatAlphaRef::Arguments.clear();
    ListMatSphereMap::Arguments.clear();
    ListMatDetails::Arguments.clear();
    ListMatUnlit::Arguments.clear();
    ListMatNormalMap::Arguments.clear();
    ListMatGrass::Arguments.clear();
    ListMatSplatting::Arguments.clear();
    m_scene_manager->drawAll(scene::ESNRP_SOLID);

    if (!UserConfigParams::m_dynamic_lights)
        return;

    {
        ScopedGPUTimer Timer(getGPUTimer(Q_SOLID_PASS1));

        std::vector<TexUnit> object_pass1_texunits = TexUnits(TexUnit(MeshShader::ObjectPass1Shader::getInstance()->TU_tex, true) );
        renderMeshes1stPass<MeshShader::ObjectPass1Shader, video::EVT_STANDARD, 2, 1>(object_pass1_texunits, ListMatDefault::Arguments);
        renderMeshes1stPass<MeshShader::ObjectPass1Shader, video::EVT_STANDARD, 2, 1>(object_pass1_texunits, ListMatSphereMap::Arguments);
        renderMeshes1stPass<MeshShader::ObjectPass1Shader, video::EVT_2TCOORDS, 2, 1>(object_pass1_texunits, ListMatDetails::Arguments);
        renderMeshes1stPass<MeshShader::ObjectPass1Shader, video::EVT_2TCOORDS, 2, 1>(object_pass1_texunits, ListMatSplatting::Arguments);
        renderMeshes1stPass<MeshShader::ObjectRefPass1Shader, video::EVT_STANDARD, 3, 2, 1>(object_pass1_texunits, ListMatUnlit::Arguments);
        renderMeshes1stPass<MeshShader::ObjectRefPass1Shader, video::EVT_STANDARD, 3, 2, 1>(TexUnits(TexUnit(MeshShader::ObjectRefPass1Shader::getInstance()->TU_tex, true)), ListMatAlphaRef::Arguments);
        renderMeshes1stPass<MeshShader::GrassPass1Shader, video::EVT_STANDARD, 3, 2, 1>(TexUnits(TexUnit(MeshShader::GrassPass1Shader::getInstance()->TU_tex, true)), ListMatGrass::Arguments);
        renderMeshes1stPass<MeshShader::NormalMapShader, video::EVT_TANGENTS, 2, 1>(TexUnits(
            TexUnit(MeshShader::NormalMapShader::getInstance()->TU_glossy, true),
            TexUnit(MeshShader::NormalMapShader::getInstance()->TU_normalmap, false)
        ), ListMatNormalMap::Arguments);
    }
}

template<typename Shader, enum E_VERTEX_TYPE VertexType, int...List, typename... TupleType>
void renderMeshes2ndPass(const std::vector<TexUnit> &TexUnits, std::vector<STK::Tuple<TupleType...> > &meshes)
{
    glUseProgram(Shader::getInstance()->Program);
    glBindVertexArray(getVAO(VertexType));
    for (unsigned i = 0; i < meshes.size(); i++)
    {
        GLMesh &mesh = *(STK::tuple_get<0>(meshes[i]));
        for (unsigned j = 0; j < TexUnits.size(); j++)
        {
            if (!mesh.textures[j])
                mesh.textures[j] = getUnicolorTexture(video::SColor(255, 255, 255, 255));
            compressTexture(mesh.textures[j], TexUnits[j].m_premul_alpha);
            setTexture(TexUnits[j].m_id, getTextureGLuint(mesh.textures[j]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
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
        auto remapped_tuple = remap_tuple<List...>::exec(meshes[i]);
        apply_instance<Shader>(Shader::getInstance(), &mesh, remapped_tuple);
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
    setTexture(0, m_rtts->getRenderTarget(RTT_TMP1), GL_NEAREST, GL_NEAREST);
    setTexture(1, m_rtts->getRenderTarget(RTT_TMP2), GL_NEAREST, GL_NEAREST);
    setTexture(2, m_rtts->getRenderTarget(RTT_HALF1_R), GL_LINEAR, GL_LINEAR);

    {
        ScopedGPUTimer Timer(getGPUTimer(Q_SOLID_PASS2));

        m_scene_manager->drawAll(scene::ESNRP_SOLID);

        renderMeshes2ndPass<MeshShader::ObjectPass2Shader, video::EVT_STANDARD, 3, 1>(TexUnits(
            TexUnit(MeshShader::ObjectPass2Shader::getInstance()->TU_Albedo, true)
        ), ListMatDefault::Arguments);

        renderMeshes2ndPass<MeshShader::ObjectRefPass2Shader, video::EVT_STANDARD, 3, 1 >(TexUnits(
            TexUnit(MeshShader::ObjectRefPass2Shader::getInstance()->TU_Albedo, true)
        ), ListMatAlphaRef::Arguments);

        renderMeshes2ndPass<MeshShader::SphereMapShader, video::EVT_STANDARD, 2, 1>(TexUnits(
            TexUnit(MeshShader::SphereMapShader::getInstance()->TU_tex, true)
        ), ListMatSphereMap::Arguments);

        renderMeshes2ndPass<MeshShader::DetailledObjectPass2Shader, video::EVT_2TCOORDS, 1>(TexUnits(
            TexUnit(MeshShader::DetailledObjectPass2Shader::getInstance()->TU_Albedo, true),
            TexUnit(MeshShader::DetailledObjectPass2Shader::getInstance()->TU_detail, true)
        ), ListMatDetails::Arguments);

        renderMeshes2ndPass<MeshShader::GrassPass2Shader, video::EVT_STANDARD, 3, 1>(TexUnits(
            TexUnit(MeshShader::GrassPass2Shader::getInstance()->TU_Albedo, true)
        ), ListMatGrass::Arguments);

        renderMeshes2ndPass<MeshShader::ObjectUnlitShader, video::EVT_STANDARD, 1>(TexUnits(
            TexUnit(MeshShader::ObjectUnlitShader::getInstance()->TU_tex, true)
        ), ListMatUnlit::Arguments);

        renderMeshes2ndPass<MeshShader::SplattingShader, video::EVT_2TCOORDS, 1>(TexUnits(
            TexUnit(8, true),
            TexUnit(MeshShader::SplattingShader::getInstance()->TU_tex_layout, false),
            TexUnit(MeshShader::SplattingShader::getInstance()->TU_tex_detail0, true),
            TexUnit(MeshShader::SplattingShader::getInstance()->TU_tex_detail1, true),
            TexUnit(MeshShader::SplattingShader::getInstance()->TU_tex_detail2, true),
            TexUnit(MeshShader::SplattingShader::getInstance()->TU_tex_detail3, true)
        ), ListMatSplatting::Arguments);

        renderMeshes2ndPass<MeshShader::ObjectPass2Shader, video::EVT_TANGENTS, 3, 1>(TexUnits(
            TexUnit(MeshShader::ObjectPass2Shader::getInstance()->TU_Albedo, true)
        ), ListMatNormalMap::Arguments);
    }
}

template<enum E_VERTEX_TYPE VertexType, typename... TupleType>
static void renderMeshNormals(std::vector<STK::Tuple<TupleType...> > &meshes)
{
    glUseProgram(MeshShader::NormalVisualizer::getInstance()->Program);
    glBindVertexArray(getVAO(VertexType));
    for (unsigned i = 0; i < meshes.size(); i++)
    {
        GLMesh &mesh = *(STK::tuple_get<0>(meshes[i]));

        if (mesh.VAOType != VertexType)
        {
#ifdef DEBUG
            Log::error("Materials", "Wrong vertex Type associed to pass 2 (hint texture : %s)", mesh.textures[0]->getName().getPath().c_str());
#endif
            continue;
        }
        draw(MeshShader::NormalVisualizer::getInstance(), STK::tuple_get<0>(meshes[i]), STK::tuple_get<1>(meshes[i]), STK::tuple_get<2>(meshes[i]), video::SColor(255, 0, 255, 0));
    }
}

void IrrDriver::renderNormalsVisualisation()
{
    renderMeshNormals<video::EVT_STANDARD>(ListMatDefault::Arguments);
    renderMeshNormals<video::EVT_STANDARD>(ListMatAlphaRef::Arguments);
    renderMeshNormals<video::EVT_STANDARD>(ListMatSphereMap::Arguments);
//    renderMeshNormals<video::EVT_STANDARD>(ListMatGrass::Arguments);
    renderMeshNormals<video::EVT_2TCOORDS>(ListMatDetails::Arguments);
    renderMeshNormals<video::EVT_STANDARD>(ListMatUnlit::Arguments);
    renderMeshNormals<video::EVT_2TCOORDS>(ListMatSplatting::Arguments);
    renderMeshNormals<video::EVT_TANGENTS>(ListMatNormalMap::Arguments);

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
    ListBlendTransparent::Arguments.clear();
    ListAdditiveTransparent::Arguments.clear();
    ListBlendTransparentFog::Arguments.clear();
    ListAdditiveTransparentFog::Arguments.clear();
    ListDisplacement::Arguments.clear();
    m_scene_manager->drawAll(scene::ESNRP_TRANSPARENT);

    glBindVertexArray(getVAO(EVT_STANDARD));

    if (World::getWorld() && World::getWorld()->isFogEnabled())
    {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        renderMeshes2ndPass<MeshShader::TransparentFogShader, video::EVT_STANDARD, 8, 7, 6, 5, 4, 3, 2, 1>(TexUnits(
            TexUnit(MeshShader::TransparentFogShader::getInstance()->TU_tex, true)
        ), ListBlendTransparentFog::Arguments);
        glBlendFunc(GL_ONE, GL_ONE);
        renderMeshes2ndPass<MeshShader::TransparentFogShader, video::EVT_STANDARD, 8, 7, 6, 5, 4, 3, 2, 1>(TexUnits(
            TexUnit(MeshShader::TransparentFogShader::getInstance()->TU_tex, true)
        ), ListAdditiveTransparentFog::Arguments);
    }
    else
    {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        renderMeshes2ndPass<MeshShader::TransparentShader, video::EVT_STANDARD, 2, 1>(TexUnits(
            TexUnit(MeshShader::TransparentShader::getInstance()->TU_tex, true)
        ), ListBlendTransparent::Arguments);
        glBlendFunc(GL_ONE, GL_ONE);
        renderMeshes2ndPass<MeshShader::TransparentShader, video::EVT_STANDARD, 2, 1>(TexUnits(
            TexUnit(MeshShader::TransparentShader::getInstance()->TU_tex, true)
        ), ListAdditiveTransparent::Arguments);
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
    for (unsigned i = 0; i < ListDisplacement::Arguments.size(); i++)
    {
        const GLMesh &mesh = *(STK::tuple_get<0>(ListDisplacement::Arguments[i]));
        const core::matrix4 &AbsoluteTransformation = STK::tuple_get<1>(ListDisplacement::Arguments[i]);
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
    for (unsigned i = 0; i < ListDisplacement::Arguments.size(); i++)
    {
        const GLMesh &mesh = *(STK::tuple_get<0>(ListDisplacement::Arguments[i]));
        const core::matrix4 &AbsoluteTransformation = STK::tuple_get<1>(ListDisplacement::Arguments[i]);
        if (mesh.VAOType != video::EVT_2TCOORDS)
            continue;

        GLenum ptype = mesh.PrimitiveType;
        GLenum itype = mesh.IndexType;
        size_t count = mesh.IndexCount;
        // Render the effect
        setTexture(MeshShader::DisplaceShader::getInstance()->TU_displacement_tex, getTextureGLuint(displaceTex), GL_LINEAR, GL_LINEAR, true);
        setTexture(MeshShader::DisplaceShader::getInstance()->TU_mask_tex, irr_driver->getRenderTargetTexture(RTT_TMP1), GL_LINEAR, GL_LINEAR, true);
        setTexture(MeshShader::DisplaceShader::getInstance()->TU_color_tex, irr_driver->getRenderTargetTexture(RTT_COLOR), GL_LINEAR, GL_LINEAR, true);
        setTexture(MeshShader::DisplaceShader::getInstance()->TU_tex, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR, true);
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


template<typename T, typename...Args>
void drawShadowTuple(const T *Shader, const GLMesh *mesh, const STK::Tuple<Args...> &t)
{
    std::function<void (Args...)> lambda = [&](Args...args) {  drawShadow<T>(Shader, mesh, args...); };
    unroll_args(lambda, t);
}

template<typename T, enum E_VERTEX_TYPE VertexType, int...List, typename... Args>
void renderShadow(const std::vector<GLuint> TextureUnits, const std::vector<STK::Tuple<GLMesh *, core::matrix4, Args...> >&t)
{
    glUseProgram(T::getInstance()->Program);
    glBindVertexArray(getVAO(VertexType));
    for (unsigned i = 0; i < t.size(); i++)
    {
        const GLMesh *mesh = STK::tuple_get<0>(t[i]);
        for (unsigned j = 0; j < TextureUnits.size(); j++)
        {
            compressTexture(mesh->textures[j], true);
            setTexture(TextureUnits[j], getTextureGLuint(mesh->textures[j]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
        }

        auto remapped_tuple = remap_tuple<List...>::exec(t[i]);
        drawShadowTuple<T>(T::getInstance(), mesh, remapped_tuple);
    }
}

template<typename T, typename... Args>
void drawRSMTuple(const core::matrix4 &rsm_matrix, const GLMesh *mesh, const STK::Tuple<Args...> &t)
{
    std::function<void (Args...)> lambda = [&](Args...args) {draw<T>(T::getInstance(), mesh, rsm_matrix, args...); };
    unroll_args(lambda, t);
}

template<typename T, enum E_VERTEX_TYPE VertexType, int... Selector, typename... Args>
void drawRSM(const core::matrix4 & rsm_matrix, const std::vector<GLuint> TextureUnits, const std::vector<STK::Tuple<GLMesh *, core::matrix4, Args...> >&t)
{
    glUseProgram(T::getInstance()->Program);
    glBindVertexArray(getVAO(VertexType));
    for (unsigned i = 0; i < t.size(); i++)
    {
        GLMesh *mesh = STK::tuple_get<0>(t[i]);
        for (unsigned j = 0; j < TextureUnits.size(); j++)
        {
            if (!mesh->textures[j])
                mesh->textures[j] = getUnicolorTexture(video::SColor(255, 255, 255, 255));
            compressTexture(mesh->textures[j], true);
            setTexture(TextureUnits[j], getTextureGLuint(mesh->textures[j]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
        }
        auto remapped_tuple = remap_tuple<Selector...>::template exec(t[i]);
        drawRSMTuple<T>(rsm_matrix, mesh, remapped_tuple);
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
    ListMatDefault::Arguments.clear();
    ListMatAlphaRef::Arguments.clear();
    ListMatSphereMap::Arguments.clear();
    ListMatDetails::Arguments.clear();
    ListMatUnlit::Arguments.clear();
    ListMatNormalMap::Arguments.clear();
    ListMatGrass::Arguments.clear();
    ListMatSplatting::Arguments.clear();
    m_scene_manager->drawAll(scene::ESNRP_SOLID);

    std::vector<GLuint> noTexUnits;
    renderShadow<MeshShader::ShadowShader, EVT_STANDARD, 1>(noTexUnits, ListMatDefault::Arguments);
    renderShadow<MeshShader::ShadowShader, EVT_STANDARD, 1>(noTexUnits, ListMatSphereMap::Arguments);
    renderShadow<MeshShader::ShadowShader, EVT_2TCOORDS, 1>(noTexUnits, ListMatDetails::Arguments);
    renderShadow<MeshShader::ShadowShader, EVT_2TCOORDS, 1>(noTexUnits, ListMatSplatting::Arguments);
    renderShadow<MeshShader::ShadowShader, EVT_TANGENTS, 1>(noTexUnits, ListMatNormalMap::Arguments);
    renderShadow<MeshShader::RefShadowShader, EVT_STANDARD, 1>(std::vector<GLuint>{ MeshShader::RefShadowShader::getInstance()->TU_tex }, ListMatAlphaRef::Arguments);
    renderShadow<MeshShader::RefShadowShader, EVT_STANDARD, 1>(std::vector<GLuint>{ MeshShader::RefShadowShader::getInstance()->TU_tex }, ListMatUnlit::Arguments);
    renderShadow<MeshShader::GrassShadowShader, EVT_STANDARD, 3, 1>(std::vector<GLuint>{ MeshShader::GrassShadowShader::getInstance()->TU_tex }, ListMatGrass::Arguments);

    glDisable(GL_POLYGON_OFFSET_FILL);

    if (!UserConfigParams::m_gi)
        return;

    m_rtts->getRSM().Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawRSM<MeshShader::RSMShader, EVT_STANDARD, 3, 1>(rsm_matrix, std::vector<GLuint>{ MeshShader::RSMShader::getInstance()->TU_tex }, ListMatDefault::Arguments);
    drawRSM<MeshShader::RSMShader, EVT_STANDARD, 3, 1>(rsm_matrix, std::vector<GLuint>{ MeshShader::RSMShader::getInstance()->TU_tex }, ListMatAlphaRef::Arguments);
//    drawRSM<EVT_STANDARD, 2, 1>(rsm_matrix, std::vector<GLuint>{ MeshShader::RSMShader::getInstance()->TU_tex }, ListMatSphereMap::Arguments);
    drawRSM<MeshShader::RSMShader, EVT_STANDARD, 3, 1>(rsm_matrix, std::vector<GLuint>{ MeshShader::RSMShader::getInstance()->TU_tex }, ListMatUnlit::Arguments);
    drawRSM<MeshShader::RSMShader, EVT_2TCOORDS, 3, 1>(rsm_matrix, std::vector<GLuint>{ MeshShader::RSMShader::getInstance()->TU_tex }, ListMatDetails::Arguments);
    drawRSM<MeshShader::SplattingRSMShader, EVT_2TCOORDS, 1>(rsm_matrix,
        std::vector<GLuint>{
            8,
            MeshShader::SplattingRSMShader::getInstance()->TU_layout,
            MeshShader::SplattingRSMShader::getInstance()->TU_detail0,
            MeshShader::SplattingRSMShader::getInstance()->TU_detail1,
            MeshShader::SplattingRSMShader::getInstance()->TU_detail2,
            MeshShader::SplattingRSMShader::getInstance()->TU_detail3},
        ListMatSplatting::Arguments);
}
