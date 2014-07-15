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

#include <algorithm>


template<unsigned N>
struct unroll_args_instance
{
    template<typename T, typename ...TupleTypes, typename ...Args>
    static void exec(const T *Shader, const std::tuple<TupleTypes...> &t, Args... args)
    {
        unroll_args_instance<N - 1>::template exec<T>(Shader, t, std::get<N - 1>(t), args...);
    }
};

template<>
struct unroll_args_instance<0>
{
    template<typename T, typename ...TupleTypes, typename ...Args>
    static void exec(const T *Shader, const std::tuple<TupleTypes...> &t, Args... args)
    {
        draw<T>(Shader, args...);
    }
};

template<typename T, typename... TupleType>
void apply_instance(const T *Shader, const std::tuple<TupleType...> &arg)
{
    unroll_args_instance<std::tuple_size<std::tuple<TupleType...> >::value >::template exec<T>(Shader, arg);
}

template<typename Shader, enum E_VERTEX_TYPE VertexType, typename... TupleType>
void renderMeshes1stPass(const std::vector<GLuint> &TexUnits, std::vector<std::tuple<TupleType...> > &meshes)
{
    glUseProgram(Shader::getInstance()->Program);
    glBindVertexArray(getVAO(VertexType));
    for (unsigned i = 0; i < meshes.size(); i++)
    {
        GLMesh &mesh = *(std::get<0>(meshes[i]));
        for (unsigned j = 0; j < TexUnits.size(); j++)
        {
            if (!mesh.textures[j])
                mesh.textures[j] = getUnicolorTexture(video::SColor(255, 255, 255, 255));
            compressTexture(mesh.textures[j], true);
            setTexture(TexUnits[j], getTextureGLuint(mesh.textures[j]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
        }
        if (mesh.VAOType != VertexType)
        {
#ifdef DEBUG
            Log::error("Materials", "Wrong vertex Type associed to pass 1 (hint texture : %s)", mesh.textures[0]->getName().getPath().c_str());
#endif
            continue;
        }
        apply_instance(Shader::getInstance(), meshes[i]);
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
    ListDefaultStandardG::Arguments.clear();
    ListDefault2TCoordG::Arguments.clear();
    ListAlphaRefG::Arguments.clear();
    ListNormalG::Arguments.clear();
    ListGrassG::Arguments.clear();
    m_scene_manager->drawAll(scene::ESNRP_SOLID);

    if (!UserConfigParams::m_dynamic_lights)
        return;

    {
        ScopedGPUTimer Timer(getGPUTimer(Q_SOLID_PASS1));
        renderMeshes1stPass<MeshShader::ObjectPass1Shader, video::EVT_STANDARD>({ MeshShader::ObjectPass1Shader::getInstance<MeshShader::ObjectPass1Shader>()->TU_tex }, ListDefaultStandardG::Arguments);
        renderMeshes1stPass<MeshShader::ObjectPass1Shader, video::EVT_2TCOORDS>({ MeshShader::ObjectPass1Shader::getInstance<MeshShader::ObjectPass1Shader>()->TU_tex }, ListDefault2TCoordG::Arguments);
        renderMeshes1stPass<MeshShader::ObjectRefPass1Shader, video::EVT_STANDARD>({ MeshShader::ObjectRefPass1Shader::getInstance<MeshShader::ObjectRefPass1Shader>()->TU_tex }, ListAlphaRefG::Arguments);
        renderMeshes1stPass<MeshShader::NormalMapShader, video::EVT_TANGENTS>({ MeshShader::NormalMapShader::getInstance<MeshShader::NormalMapShader>()->TU_glossy, MeshShader::NormalMapShader::getInstance<MeshShader::NormalMapShader>()->TU_normalmap }, ListNormalG::Arguments);
        renderMeshes1stPass<MeshShader::GrassPass1Shader, video::EVT_STANDARD>({ MeshShader::GrassPass1Shader::getInstance<MeshShader::GrassPass1Shader>()->TU_tex }, ListGrassG::Arguments);
    }
}

template<typename Shader, enum E_VERTEX_TYPE VertexType, typename... TupleType>
void renderMeshes2ndPass(const std::vector<GLuint> &TexUnits, std::vector<std::tuple<TupleType...> > &meshes)
{
    glUseProgram(Shader::getInstance<Shader>()->Program);
    glBindVertexArray(getVAO(VertexType));
    for (unsigned i = 0; i < meshes.size(); i++)
    {
        GLMesh &mesh = *(std::get<0>(meshes[i]));
        for (unsigned j = 0; j < TexUnits.size(); j++)
        {
            if (!mesh.textures[j])
                mesh.textures[j] = getUnicolorTexture(video::SColor(255, 255, 255, 255));
            compressTexture(mesh.textures[j], true);
            setTexture(TexUnits[j], getTextureGLuint(mesh.textures[j]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
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
        apply_instance(Shader::getInstance<Shader>(), meshes[i]);
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
    ListDefaultStandardSM::Arguments.clear();
    ListDefaultTangentSM::Arguments.clear();
    ListAlphaRefSM::Arguments.clear();
    ListSphereMapSM::Arguments.clear();
    ListUnlitSM::Arguments.clear();
    ListDetailSM::Arguments.clear();
    ListSplattingSM::Arguments.clear();
    ListGrassSM::Arguments.clear();
    setTexture(0, m_rtts->getRenderTarget(RTT_TMP1), GL_NEAREST, GL_NEAREST);
    setTexture(1, m_rtts->getRenderTarget(RTT_TMP2), GL_NEAREST, GL_NEAREST);
    setTexture(2, m_rtts->getRenderTarget(RTT_HALF1_R), GL_LINEAR, GL_LINEAR);

    {
        ScopedGPUTimer Timer(getGPUTimer(Q_SOLID_PASS2));

        m_scene_manager->drawAll(scene::ESNRP_SOLID);

        renderMeshes2ndPass<MeshShader::ObjectPass2Shader, video::EVT_STANDARD>({ MeshShader::ObjectPass2Shader::getInstance<MeshShader::ObjectPass2Shader>()->TU_Albedo }, ListDefaultStandardSM::Arguments);
        renderMeshes2ndPass<MeshShader::ObjectPass2Shader, video::EVT_TANGENTS>({ MeshShader::ObjectPass2Shader::getInstance<MeshShader::ObjectPass2Shader>()->TU_Albedo }, ListDefaultTangentSM::Arguments);
        renderMeshes2ndPass<MeshShader::ObjectRefPass2Shader, video::EVT_STANDARD>({ MeshShader::ObjectRefPass2Shader::getInstance<MeshShader::ObjectRefPass2Shader>()->TU_Albedo }, ListAlphaRefSM::Arguments);
        renderMeshes2ndPass<MeshShader::SphereMapShader, video::EVT_STANDARD>({ MeshShader::SphereMapShader::getInstance<MeshShader::SphereMapShader>()->TU_tex }, ListSphereMapSM::Arguments);
        renderMeshes2ndPass<MeshShader::ObjectUnlitShader, video::EVT_STANDARD>({ MeshShader::ObjectUnlitShader::getInstance<MeshShader::ObjectUnlitShader>()->TU_tex }, ListUnlitSM::Arguments);
        renderMeshes2ndPass<MeshShader::DetailledObjectPass2Shader, video::EVT_2TCOORDS>({ MeshShader::DetailledObjectPass2Shader::getInstance<MeshShader::DetailledObjectPass2Shader>()->TU_Albedo, MeshShader::DetailledObjectPass2Shader::getInstance<MeshShader::DetailledObjectPass2Shader>()->TU_detail }, ListDetailSM::Arguments);
        renderMeshes2ndPass<MeshShader::SplattingShader, video::EVT_2TCOORDS>({ 8, MeshShader::SplattingShader::getInstance<MeshShader::SplattingShader>()->TU_tex_layout, MeshShader::SplattingShader::getInstance<MeshShader::SplattingShader>()->TU_tex_detail0, MeshShader::SplattingShader::getInstance<MeshShader::SplattingShader>()->TU_tex_detail1, MeshShader::SplattingShader::getInstance<MeshShader::SplattingShader>()->TU_tex_detail2, MeshShader::SplattingShader::getInstance<MeshShader::SplattingShader>()->TU_tex_detail3 }, ListSplattingSM::Arguments);
        renderMeshes2ndPass<MeshShader::GrassPass2Shader, video::EVT_STANDARD>({ MeshShader::GrassPass2Shader::getInstance<MeshShader::GrassPass2Shader>()->TU_Albedo }, ListGrassSM::Arguments);
    }
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
        renderMeshes2ndPass<MeshShader::TransparentFogShader, video::EVT_STANDARD>({ MeshShader::TransparentFogShader::getInstance<MeshShader::TransparentFogShader>()->TU_tex }, ListBlendTransparentFog::Arguments);
        glBlendFunc(GL_ONE, GL_ONE);
        renderMeshes2ndPass<MeshShader::TransparentFogShader, video::EVT_STANDARD>({ MeshShader::TransparentFogShader::getInstance<MeshShader::TransparentFogShader>()->TU_tex }, ListAdditiveTransparentFog::Arguments);
    }
    else
    {
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        renderMeshes2ndPass<MeshShader::TransparentShader, video::EVT_STANDARD>({ MeshShader::TransparentShader::getInstance<MeshShader::TransparentShader>()->TU_tex }, ListBlendTransparent::Arguments);
        glBlendFunc(GL_ONE, GL_ONE);
        renderMeshes2ndPass<MeshShader::TransparentShader, video::EVT_STANDARD>({ MeshShader::TransparentShader::getInstance<MeshShader::TransparentShader>()->TU_tex }, ListAdditiveTransparent::Arguments);
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
        const GLMesh &mesh = *(std::get<0>(ListDisplacement::Arguments[i]));
        const core::matrix4 &AbsoluteTransformation = std::get<1>(ListDisplacement::Arguments[i]);
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

        glUseProgram(MeshShader::DisplaceMaskShaderInstance->Program);
        MeshShader::DisplaceMaskShaderInstance->setUniforms(AbsoluteTransformation);
        glDrawElementsBaseVertex(ptype, count, itype, (GLvoid *)mesh.vaoOffset, mesh.vaoBaseVertex);
    }

    irr_driver->getFBO(FBO_DISPLACE).Bind();
    if (!displaceTex)
        displaceTex = irr_driver->getTexture(FileManager::TEXTURE, "displace.png");
    for (unsigned i = 0; i < ListDisplacement::Arguments.size(); i++)
    {
        const GLMesh &mesh = *(std::get<0>(ListDisplacement::Arguments[i]));
        const core::matrix4 &AbsoluteTransformation = std::get<1>(ListDisplacement::Arguments[i]);
        if (mesh.VAOType != video::EVT_2TCOORDS)
            continue;

        GLenum ptype = mesh.PrimitiveType;
        GLenum itype = mesh.IndexType;
        size_t count = mesh.IndexCount;
        // Render the effect
        setTexture(MeshShader::DisplaceShaderInstance->TU_displacement_tex, getTextureGLuint(displaceTex), GL_LINEAR, GL_LINEAR, true);
        setTexture(MeshShader::DisplaceShaderInstance->TU_mask_tex, irr_driver->getRenderTargetTexture(RTT_TMP1), GL_LINEAR, GL_LINEAR, true);
        setTexture(MeshShader::DisplaceShaderInstance->TU_color_tex, irr_driver->getRenderTargetTexture(RTT_COLOR), GL_LINEAR, GL_LINEAR, true);
        setTexture(MeshShader::DisplaceShaderInstance->TU_tex, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR, true);
        glUseProgram(MeshShader::DisplaceShaderInstance->Program);
        MeshShader::DisplaceShaderInstance->setUniforms(AbsoluteTransformation,
            core::vector2df(cb->getDirX(), cb->getDirY()),
            core::vector2df(cb->getDir2X(), cb->getDir2Y()));

        glDrawElementsBaseVertex(ptype, count, itype, (GLvoid *)mesh.vaoOffset, mesh.vaoBaseVertex);
    }

    irr_driver->getFBO(FBO_COLORS).Bind();
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    m_post_processing->renderPassThrough(m_rtts->getRenderTarget(RTT_DISPLACE));
    glDisable(GL_STENCIL_TEST);

}

template<typename T, enum E_VERTEX_TYPE VertexType, typename... Args>
void drawShadow(const T *Shader, const std::vector<GLuint> TextureUnits, const std::vector<std::tuple<GLMesh *, core::matrix4, Args...> >&t)
{
    glUseProgram(Shader->Program);
    glBindVertexArray(getVAO(VertexType));
    for (unsigned i = 0; i < t.size(); i++)
    {
        const GLMesh *mesh = std::get<0>(t[i]);
        irr_driver->IncreaseObjectCount();
        GLenum ptype = mesh->PrimitiveType;
        GLenum itype = mesh->IndexType;
        size_t count = mesh->IndexCount;
        for (unsigned j = 0; j < TextureUnits.size(); j++)
        {
            compressTexture(mesh->textures[j], true);
            setTexture(TextureUnits[j], getTextureGLuint(mesh->textures[j]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
        }

        Shader->setUniforms(std::get<1>(t[i]));
        glDrawElementsInstancedBaseVertex(ptype, count, itype, (GLvoid *)mesh->vaoOffset, 4, mesh->vaoBaseVertex);
    }
}

static void drawShadowGrass(const std::vector<GLuint> TextureUnits, const std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, core::vector3df> > &t)
{
    glUseProgram(MeshShader::GrassShadowShaderInstance->Program);
    glBindVertexArray(getVAO(EVT_STANDARD));
    for (unsigned i = 0; i < t.size(); i++)
    {
        const GLMesh *mesh = std::get<0>(t[i]);
        irr_driver->IncreaseObjectCount();
        GLenum ptype = mesh->PrimitiveType;
        GLenum itype = mesh->IndexType;
        size_t count = mesh->IndexCount;
        for (unsigned j = 0; j < TextureUnits.size(); j++)
        {
            compressTexture(mesh->textures[j], true);
            setTexture(TextureUnits[j], getTextureGLuint(mesh->textures[j]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
        }

        MeshShader::GrassShadowShaderInstance->setUniforms(std::get<1>(t[i]), std::get<3>(t[i]));
        glDrawElementsInstancedBaseVertex(ptype, count, itype, (GLvoid *)mesh->vaoOffset, 4, mesh->vaoBaseVertex);
    }
}

template<enum E_VERTEX_TYPE VertexType, typename... Args>
void drawRSM(const core::matrix4 & rsm_matrix, const std::vector<GLuint> TextureUnits, const std::vector<std::tuple<GLMesh *, core::matrix4, Args...> >&t)
{
    glUseProgram(MeshShader::RSMShader::Program);
    glBindVertexArray(getVAO(VertexType));
    for (unsigned i = 0; i < t.size(); i++)
    {
        const GLMesh *mesh = std::get<0>(t[i]);
        for (unsigned j = 0; j < TextureUnits.size(); j++)
        {
            compressTexture(mesh->textures[j], true);
            setTexture(TextureUnits[j], getTextureGLuint(mesh->textures[j]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
        }
        draw<MeshShader::RSMShader>(mesh, rsm_matrix, std::get<1>(t[i]));
    }
}

void IrrDriver::renderShadows()
{
    irr_driver->setPhase(SHADOW_PASS);
    glDisable(GL_BLEND);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.5, 0.);
    m_rtts->getShadowFBO().Bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    glDrawBuffer(GL_NONE);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, SharedObject::ViewProjectionMatrixesUBO);

    m_scene_manager->drawAll(scene::ESNRP_SOLID);

    drawShadow<MeshShader::ShadowShader, EVT_STANDARD>(MeshShader::ShadowShaderInstance, {}, ListDefaultStandardG::Arguments);
    drawShadow<MeshShader::ShadowShader, EVT_2TCOORDS>(MeshShader::ShadowShaderInstance, {}, ListDefault2TCoordG::Arguments);
    drawShadow<MeshShader::ShadowShader, EVT_TANGENTS>(MeshShader::ShadowShaderInstance, {}, ListNormalG::Arguments);
    drawShadow<MeshShader::RefShadowShader, EVT_STANDARD>(MeshShader::RefShadowShaderInstance, { MeshShader::RefShadowShaderInstance->TU_tex }, ListAlphaRefG::Arguments);
    drawShadowGrass({ MeshShader::GrassShadowShaderInstance->TU_tex }, ListGrassG::Arguments);

    glDisable(GL_POLYGON_OFFSET_FILL);

    if (!UserConfigParams::m_gi)
        return;

    m_rtts->getRSM().Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawRSM<EVT_STANDARD>(rsm_matrix, { MeshShader::RSMShader::TU_tex }, ListDefaultStandardG::Arguments);
    drawRSM<EVT_2TCOORDS>(rsm_matrix, { MeshShader::RSMShader::TU_tex }, ListDefault2TCoordG::Arguments);
}