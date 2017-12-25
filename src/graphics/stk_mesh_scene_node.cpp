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
#include "graphics/render_info.hpp"
#include "graphics/rtts.hpp"
#include "graphics/shaders.hpp"
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
        GLmeshes.push_back(allocateMeshBuffer(mb, m_debug_name, NULL));
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
        fillLocalBuffer(mesh, mb);
        mesh.vao = createVAO(mesh.vertex_buffer, mesh.index_buffer, mb->getVertexType());
        glBindVertexArray(0);
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

    if (irr_driver->getPhase() == GLOW_PASS)
    {
        ColorizeShader::getInstance()->use();
        for (u32 i = 0; i < Mesh->getMeshBufferCount(); ++i)
        {
            scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
            if (!mb)
                continue;
            glBindVertexArray(GLmeshes[i].vao);
            drawGlow(GLmeshes[i]);
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

