//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef SERVER_ONLY

#include "graphics/stk_mesh_scene_node.hpp"

#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/materials.hpp"
#include "graphics/render_info.hpp"
#include "graphics/rtts.hpp"
#include "graphics/shaders.hpp"
#include "graphics/vao_manager.hpp"
#include "tracks/track.hpp"
#include "modes/world.hpp"

#include <IMaterialRenderer.h>
#include <ISceneManager.h>

// ============================================================================
class ColorizeShader : public Shader<ColorizeShader, core::matrix4, 
                                     video::SColorf>
{
public:
    ColorizeShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER, "object_pass.vert",
                            GL_FRAGMENT_SHADER, "colorize.frag");
        assignUniforms("ModelMatrix", "col");
    }

};   // ColorizeShader

// ============================================================================
STKMeshSceneNode::STKMeshSceneNode(irr::scene::IMesh* mesh, ISceneNode* parent, irr::scene::ISceneManager* mgr,
    irr::s32 id, const std::string& debug_name,
    const irr::core::vector3df& position,
    const irr::core::vector3df& rotation,
    const irr::core::vector3df& scale, bool createGLMeshes, RenderInfo* render_info, bool all_parts_colorized) :
    CMeshSceneNode(mesh, parent, mgr, id, position, rotation, scale)
{
    isDisplacement = false;
    immediate_draw = false;
    update_each_frame = false;
    isGlow = false;
    m_got_animated_matrix = false;

    m_debug_name = debug_name;

    if (createGLMeshes)
        this->createGLMeshes(render_info, all_parts_colorized);
}

void STKMeshSceneNode::setReloadEachFrame(bool val)
{
    update_each_frame = val;
    if (val)
        immediate_draw = true;
}

void STKMeshSceneNode::createGLMeshes(RenderInfo* render_info, bool all_parts_colorized)
{
    const u32 mb_count = Mesh->getMeshBufferCount();
    for (u32 i = 0; i < mb_count; ++i)
    {
        scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
        bool affected = false;
        RenderInfo* cur_ri = render_info;
        if (!all_parts_colorized && mb && render_info)
        {
            if (render_info && !render_info->isStatic())
            {
                // Convert to static render info for each mesh buffer
                assert(render_info->getNumberOfHue() == mb_count);
                const float hue = render_info->getDynamicHue(i);
                if (hue > 0.0f)
                {
                    cur_ri = new RenderInfo(hue);
                    m_static_render_info.push_back(cur_ri);
                    affected = true;
                }
                else
                {
                    cur_ri = NULL;
                }
            }
            else
            {
                // Test if material is affected by static hue change
                Material* m = material_manager->getMaterialFor(mb
                    ->getMaterial().getTexture(0), mb);
                if (m->isColorizable())
                    affected = true;
            }
        }

        assert(cur_ri ? cur_ri->isStatic() : true);
        GLmeshes.push_back(allocateMeshBuffer(mb, m_debug_name,
            affected || all_parts_colorized || (cur_ri &&
            cur_ri->isTransparent()) ? cur_ri : NULL));
    }
    isMaterialInitialized = false;
    isGLInitialized = false;
}

void STKMeshSceneNode::cleanGLMeshes()
{
    for (u32 i = 0; i < GLmeshes.size(); ++i)
    {
        GLMesh mesh = GLmeshes[i];
        if (!mesh.vertex_buffer)
            continue;
        if (mesh.vao)
            glDeleteVertexArrays(1, &(mesh.vao));
        if (mesh.vertex_buffer)
            glDeleteBuffers(1, &(mesh.vertex_buffer));
        if (mesh.index_buffer)
            glDeleteBuffers(1, &(mesh.index_buffer));
    }
    GLmeshes.clear();
    for (unsigned i = 0; i < Material::SHADERTYPE_COUNT; i++)
        MeshSolidMaterial[i].clearWithoutDeleting();
    for (unsigned i = 0; i < TM_COUNT; i++)
        TransparentMesh[i].clearWithoutDeleting();
}

void STKMeshSceneNode::setMesh(irr::scene::IMesh* mesh)
{
    CMeshSceneNode::setMesh(mesh);
    cleanGLMeshes();
    createGLMeshes();
}

STKMeshSceneNode::~STKMeshSceneNode()
{
    cleanGLMeshes();
}

void STKMeshSceneNode::drawGlow(const GLMesh &mesh)
{
    assert(mesh.VAOType == video::EVT_STANDARD);

    GLenum ptype = mesh.PrimitiveType;
    GLenum itype = mesh.IndexType;
    unsigned int count = mesh.IndexCount;
    ColorizeShader::getInstance()->setUniforms(AbsoluteTransformation, video::SColorf(glowcolor.getRed() / 255.f, glowcolor.getGreen() / 255.f, glowcolor.getBlue() / 255.f));
    glDrawElementsBaseVertex(ptype, count, itype, (GLvoid *)mesh.vaoOffset, mesh.vaoBaseVertex);
}

void STKMeshSceneNode::updatevbo()
{
    for (unsigned i = 0; i < Mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
        if (!mb)
            continue;
        GLMesh &mesh = GLmeshes[i];
        glDeleteBuffers(1, &(mesh.vertex_buffer));
        glDeleteBuffers(1, &(mesh.index_buffer));
        glDeleteVertexArrays(1, &(mesh.vao));

        fillLocalBuffer(mesh, mb);
        mesh.vao = createVAO(mesh.vertex_buffer, mesh.index_buffer, mb->getVertexType());
    }
}

void STKMeshSceneNode::updateNoGL()
{
    Box = Mesh->getBoundingBox();

    if (!isMaterialInitialized)
    {
        irr::video::IVideoDriver* driver = irr_driver->getVideoDriver();
        for (u32 i = 0; i < Mesh->getMeshBufferCount(); ++i)
        {
            scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
            if (!mb)
                continue;

            video::E_MATERIAL_TYPE type = mb->getMaterial().MaterialType;
            f32 MaterialTypeParam = mb->getMaterial().MaterialTypeParam;
            video::IMaterialRenderer* rnd = driver->getMaterialRenderer(type);
            if (!isObject(type))
            {
#ifdef DEBUG
                Log::warn("material", "Unhandled (static) material type : %d", type);
#endif
                continue;
            }

            GLMesh &mesh = GLmeshes[i];
            video::E_VERTEX_TYPE vt = mb->getVertexType();
            Material* material = material_manager->getMaterialFor(mb->getMaterial().getTexture(0), mb);
            if (mesh.m_render_info != NULL && mesh.m_render_info->isTransparent() && !rnd->isTransparent())
            {
                assert(!immediate_draw);
                if (mesh.VAOType == video::EVT_TANGENTS)
                    TransparentMesh[TM_TRANSLUCENT_TAN].push_back(&mesh);
                else
                    TransparentMesh[TM_TRANSLUCENT_STD].push_back(&mesh);
            }
            else if (rnd->isTransparent())
            {
                TransparentMaterial TranspMat = getTransparentMaterialFromType(type, vt, MaterialTypeParam, material);
                if (!immediate_draw)
                    TransparentMesh[TranspMat].push_back(&mesh);
                else
                    additive = (TranspMat == TM_ADDITIVE);
            }
            else
            {
                assert(!isDisplacement);
                Material* material2 = NULL;
                if (mb->getMaterial().getTexture(1) != NULL)
                    material2 = material_manager->getMaterialFor(mb->getMaterial().getTexture(1), mb);
                Material::ShaderType MatType = getMeshMaterialFromType(type, vt, material, material2);
                if (!immediate_draw)
                    MeshSolidMaterial[MatType].push_back(&mesh);
            }
        }
        isMaterialInitialized = true;
    }

    for (u32 i = 0; i < Mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
        if (mb != NULL)
        {
            // Test if texture matrix needs to be updated every frame
            const core::matrix4& mat = getMaterial(i).getTextureMatrix(0);
            if (mat.isIdentity() && !m_got_animated_matrix)
                continue;
            else
            {
                m_got_animated_matrix = true;
                GLmeshes[i].texture_trans.X = mat[8];
                GLmeshes[i].texture_trans.Y = mat[9];
            }
        }
    }
}

void STKMeshSceneNode::updateGL()
{
    if (isGLInitialized)
        return;
    for (u32 i = 0; i < Mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
        if (!mb)
            continue;
        GLMesh &mesh = GLmeshes[i];

        irr::video::IVideoDriver* driver = irr_driver->getVideoDriver();
        video::E_MATERIAL_TYPE type = mb->getMaterial().MaterialType;
        video::IMaterialRenderer* rnd = driver->getMaterialRenderer(type);


        if (!rnd->isTransparent())
        {
            Material* material = material_manager->getMaterialFor(mb->getMaterial().getTexture(0), mb);
            Material* material2 = NULL;
            if (mb->getMaterial().getTexture(1) != NULL)
                material2 = material_manager->getMaterialFor(mb->getMaterial().getTexture(1), mb);
            Material::ShaderType MatType = getMeshMaterialFromType(type, mb->getVertexType(), material, material2);
            if (!immediate_draw)
                initTextures(mesh, MatType);
        }
        else if (!immediate_draw)
            initTexturesTransparent(mesh);

        if (!immediate_draw && CVS->isARBBaseInstanceUsable())
        {
            std::pair<unsigned, unsigned> p = VAOManager::getInstance()->getBase(mb);
            mesh.vaoBaseVertex = p.first;
            mesh.vaoOffset = p.second;
        }
        else
        {
            fillLocalBuffer(mesh, mb);
            mesh.vao = createVAO(mesh.vertex_buffer, mesh.index_buffer, mb->getVertexType());
            glBindVertexArray(0);
        }
    }
    isGLInitialized = true;
}

void STKMeshSceneNode::OnRegisterSceneNode()
{
    if (isDisplacement)
        SceneManager->registerNodeForRendering(this, scene::ESNRP_TRANSPARENT);
    else
        CMeshSceneNode::OnRegisterSceneNode();
}

void STKMeshSceneNode::render()
{
    irr::video::IVideoDriver* driver = irr_driver->getVideoDriver();

    if (!Mesh || !driver)
        return;

    ++PassCount;

    updateNoGL();
    updateGL();

    bool isTransparent = false;

    for (u32 i = 0; i < Mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
        if (!mb)
            continue;

        video::E_MATERIAL_TYPE type = mb->getMaterial().MaterialType;
        video::IMaterialRenderer* rnd = driver->getMaterialRenderer(type);

        isTransparent = rnd->isTransparent();
        break;
    }

    if ((irr_driver->getPhase() == SOLID_NORMAL_AND_DEPTH_PASS) && immediate_draw && !isTransparent)
    {
        core::matrix4 invmodel;
        AbsoluteTransformation.getInverse(invmodel);

        glDisable(GL_CULL_FACE);
        if (update_each_frame)
            updatevbo();
        ObjectPass1Shader::getInstance()->use();
        // Only untextured
        for (unsigned i = 0; i < GLmeshes.size(); i++)
        {
            irr_driver->increaseObjectCount();
            GLMesh &mesh = GLmeshes[i];
            GLenum ptype = mesh.PrimitiveType;
            GLenum itype = mesh.IndexType;
            unsigned int count = mesh.IndexCount;

#if !defined(USE_GLES2)
            if (CVS->isAZDOEnabled())
            {
                if (!mesh.TextureHandles[0])
                {
                    mesh.TextureHandles[0] = mesh.textures[0]->getHandle();
                }
                ObjectPass1Shader::getInstance()->setTextureHandles(mesh.TextureHandles[0]);
            }
            else
#endif
                ObjectPass1Shader::getInstance()->setTextureUnits(mesh.textures[0]->getOpenGLTextureName());
            ObjectPass1Shader::getInstance()->setUniforms(AbsoluteTransformation, invmodel);
            assert(mesh.vao);
            glBindVertexArray(mesh.vao);
            glDrawElements(ptype, count, itype, 0);
            glBindVertexArray(0);
        }
        glEnable(GL_CULL_FACE);
        return;
    }

    if (irr_driver->getPhase() == SOLID_LIT_PASS  && immediate_draw && !isTransparent)
    {
        core::matrix4 invmodel;
        AbsoluteTransformation.getInverse(invmodel);

        glDisable(GL_CULL_FACE);
        if (update_each_frame && !CVS->isDefferedEnabled())
            updatevbo();
        ObjectPass2Shader::getInstance()->use();
        // Only untextured
        for (unsigned i = 0; i < GLmeshes.size(); i++)
        {
            irr_driver->increaseObjectCount();
            GLMesh &mesh = GLmeshes[i];
            GLenum ptype = mesh.PrimitiveType;
            GLenum itype = mesh.IndexType;
            unsigned int count = mesh.IndexCount;

#if !defined(USE_GLES2)
            if (CVS->isAZDOEnabled())
            {
                GLuint64 DiffuseHandle =
                    glGetTextureSamplerHandleARB(irr_driver->getRenderTargetTexture(RTT_DIFFUSE), 
                                                 ObjectPass2Shader
                                                    ::getInstance()->m_sampler_ids[0]);

                GLuint64 SpecularHandle =
                    glGetTextureSamplerHandleARB(irr_driver->getRenderTargetTexture(RTT_SPECULAR),
                                        ObjectPass2Shader::getInstance()->m_sampler_ids[1]);

                GLuint64 SSAOHandle =
                    glGetTextureSamplerHandleARB(irr_driver->getRenderTargetTexture(RTT_HALF1_R),
                                    ObjectPass2Shader::getInstance()->m_sampler_ids[2]);

                if (!mesh.TextureHandles[0])
                    mesh.TextureHandles[0] = mesh.textures[0]->getHandle();

                if (!mesh.TextureHandles[1])
                    mesh.TextureHandles[1] = mesh.textures[1]->getHandle();

                if (!mesh.TextureHandles[2])
                    mesh.TextureHandles[2] = mesh.textures[2]->getHandle();

                ObjectPass2Shader::getInstance()
                    ->setTextureHandles(DiffuseHandle, SpecularHandle, SSAOHandle,
                                        mesh.TextureHandles[0], mesh.TextureHandles[1],
                                        mesh.TextureHandles[2]);
            }
            else
#endif
                ObjectPass2Shader::getInstance()->setTextureUnits(
                irr_driver->getRenderTargetTexture(RTT_DIFFUSE),
                irr_driver->getRenderTargetTexture(RTT_SPECULAR),
                irr_driver->getRenderTargetTexture(RTT_HALF1_R),
                mesh.textures[0]->getOpenGLTextureName(),
                mesh.textures[1]->getOpenGLTextureName(),
                mesh.textures[2]->getOpenGLTextureName());
            ObjectPass2Shader::getInstance()->setUniforms(AbsoluteTransformation,
                                                                   mesh.texture_trans,
                                                                   core::vector2df(0.0f, 0.0f));
            assert(mesh.vao);
            glBindVertexArray(mesh.vao);
            glDrawElements(ptype, count, itype, 0);
            glBindVertexArray(0);
        }
        glEnable(GL_CULL_FACE);
        return;
    }

    if (irr_driver->getPhase() == GLOW_PASS)
    {
        ColorizeShader::getInstance()->use();
        for (u32 i = 0; i < Mesh->getMeshBufferCount(); ++i)
        {
            scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
            if (!mb)
                continue;
            if (CVS->isARBBaseInstanceUsable())
                glBindVertexArray(VAOManager::getInstance()->getVAO(video::EVT_STANDARD));
            else
                glBindVertexArray(GLmeshes[i].vao);
            drawGlow(GLmeshes[i]);
        }
    }

    if (irr_driver->getPhase() == TRANSPARENT_PASS && isTransparent)
    {
        ModelViewProjectionMatrix = computeMVP(AbsoluteTransformation);

        if (immediate_draw)
        {
            if (update_each_frame)
                updatevbo();
            if (additive)
                glBlendFunc(GL_ONE, GL_ONE);
            else
                glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

            const Track * const track = Track::getCurrentTrack();
            if (track && track->isFogEnabled())
            {
                Shaders::TransparentFogShader::getInstance()->use();
                for (unsigned i = 0; i < GLmeshes.size(); i++)
                {
                    GLMesh &mesh = GLmeshes[i];
                    irr_driver->increaseObjectCount();
                    GLenum ptype = mesh.PrimitiveType;
                    GLenum itype = mesh.IndexType;
                    unsigned int count = mesh.IndexCount;

                    // This function is only called once per frame - thus no need for setters.
                    const float fogmax = track->getFogMax();
                    const float startH = track->getFogStartHeight();
                    const float endH   = track->getFogEndHeight();
                    const float start  = track->getFogStart();
                    const float end    = track->getFogEnd();
                    const video::SColor tmpcol = track->getFogColor();

                    video::SColorf col(tmpcol.getRed() / 255.0f,
                                       tmpcol.getGreen() / 255.0f,
                                       tmpcol.getBlue() / 255.0f);

#if !defined(USE_GLES2)
                    if (CVS->isAZDOEnabled())
                    {
                        if (!mesh.TextureHandles[0])
                            mesh.TextureHandles[0] = mesh.textures[0]->getHandle();
                        Shaders::TransparentFogShader::getInstance()
                                    ->setTextureHandles(mesh.TextureHandles[0]);
                    }
                    else
#endif
                    {
                        Shaders::TransparentFogShader::getInstance()
                            ->setTextureUnits(mesh.textures[0]->getOpenGLTextureName());
                    }
                    Shaders::TransparentFogShader::getInstance()
                           ->setUniforms(AbsoluteTransformation, mesh.texture_trans,
                                         fogmax, startH, endH, start, end, col);

                    assert(mesh.vao);
                    glBindVertexArray(mesh.vao);
                    glDrawElements(ptype, count, itype, 0);
                    glBindVertexArray(0);
                }
            }
            else
            {
                Shaders::TransparentShader::getInstance()->use();
                for (unsigned i = 0; i < GLmeshes.size(); i++)
                {
                    irr_driver->increaseObjectCount();
                    GLMesh &mesh = GLmeshes[i];
                    GLenum ptype = mesh.PrimitiveType;
                    GLenum itype = mesh.IndexType;
                    unsigned int count = mesh.IndexCount;

#if !defined(USE_GLES2)
                    if (CVS->isAZDOEnabled())
                    {
                        if (!mesh.TextureHandles[0])
                            mesh.TextureHandles[0] = mesh.textures[0]->getHandle();
                        Shaders::TransparentShader::getInstance()->setTextureHandles(mesh.TextureHandles[0]);
                    }
                    else
#endif
                        Shaders::TransparentShader::getInstance()->setTextureUnits(mesh.textures[0]->getOpenGLTextureName());

                    Shaders::TransparentShader::getInstance()->setUniforms(AbsoluteTransformation, mesh.texture_trans, 1.0f);
                    assert(mesh.vao);
                    glBindVertexArray(mesh.vao);
                    glDrawElements(ptype, count, itype, 0);
                    glBindVertexArray(0);
                }
            }
            return;
        }
    }
}

void STKMeshSceneNode::setIsDisplacement(bool v)
{
    isDisplacement = v;
    for (u32 i = 0; i < Mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
        if (!mb)
            continue;

        if (isDisplacement)
            mb->getMaterial().MaterialType = Shaders::getShader(ES_DISPLACE);
    }
}

#endif   // !SERVER_ONLY

