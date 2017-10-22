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

#include "graphics/stk_animated_mesh.hpp"

#include "graphics/central_settings.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/render_info.hpp"
#include "graphics/shared_gpu_objects.hpp"
#include "graphics/stk_mesh.hpp"
#include "graphics/vao_manager.hpp"

#include <IMaterialRenderer.h>
#include <ISceneManager.h>
#include "../../lib/irrlicht/source/Irrlicht/CSkinnedMesh.h"

using namespace irr;

STKAnimatedMesh::STKAnimatedMesh(irr::scene::IAnimatedMesh* mesh, irr::scene::ISceneNode* parent,
irr::scene::ISceneManager* mgr, s32 id, const std::string& debug_name,
const core::vector3df& position,
const core::vector3df& rotation,
const core::vector3df& scale, RenderInfo* render_info, bool all_parts_colorized) :
    CAnimatedMeshSceneNode(mesh, parent, mgr, id, position, rotation, scale),
    m_skinned_mesh(NULL), m_skinning_offset(-1)
{
    isGLInitialized = false;
    isMaterialInitialized = false;
    m_got_animated_matrix = false;
    m_mesh_render_info = render_info;
    m_all_parts_colorized = all_parts_colorized;
#ifdef DEBUG
    m_debug_name = debug_name;
#endif
    resetSkinningState(mesh);
}

STKAnimatedMesh::~STKAnimatedMesh()
{
    cleanGLMeshes();
}

void STKAnimatedMesh::cleanGLMeshes()
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

void STKAnimatedMesh::setMesh(scene::IAnimatedMesh* mesh)
{
    isGLInitialized = false;
    isMaterialInitialized = false;
    cleanGLMeshes();
    CAnimatedMeshSceneNode::setMesh(mesh);
    resetSkinningState(mesh);
}

void STKAnimatedMesh::updateNoGL()
{
    scene::IMesh* m = getMeshForCurrentFrame();

    if (m)
        Box = m->getBoundingBox();
    else
    {
        Log::error("animated mesh", "Animated Mesh returned no mesh to render.");
        return;
    }

    if (!isMaterialInitialized)
    {
        video::IVideoDriver* driver = SceneManager->getVideoDriver();
        const u32 mb_count = m->getMeshBufferCount();
        for (u32 i = 0; i < mb_count; ++i)
        {
            scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
            if (!mb)
                continue;

            scene::SSkinMeshBuffer* ssmb = NULL;
            video::E_VERTEX_TYPE prev_type = mb->getVertexType();
            if (m_skinned_mesh)
            {
                ssmb = dynamic_cast<scene::SSkinMeshBuffer*>(mb);
                ssmb->VertexType = video::EVT_SKINNED_MESH;
            }

            bool affected = false;
            RenderInfo* cur_ri = m_mesh_render_info;
            if (!m_all_parts_colorized && mb && cur_ri)
            {
                if (m_mesh_render_info && !m_mesh_render_info->isStatic())
                {
                    // Convert to static render info for each mesh buffer
                    assert(m_mesh_render_info->getNumberOfHue() == mb_count);
                    const float hue = m_mesh_render_info->getDynamicHue(i);
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
                affected || m_all_parts_colorized || (cur_ri
                && cur_ri->isTransparent()) ? cur_ri : NULL));

            if (m_skinned_mesh) ssmb->VertexType = prev_type;
        }

        for (u32 i = 0; i < m->getMeshBufferCount(); ++i)
        {
            scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
            if (!mb)
                continue;

            scene::SSkinMeshBuffer* ssmb = NULL;
            video::E_VERTEX_TYPE prev_type = mb->getVertexType();
            if (m_skinned_mesh)
            {
                ssmb = dynamic_cast<scene::SSkinMeshBuffer*>(mb);
                ssmb->VertexType = video::EVT_SKINNED_MESH;
            }

            video::E_MATERIAL_TYPE type = mb->getMaterial().MaterialType;
            f32 MaterialTypeParam = mb->getMaterial().MaterialTypeParam;
            video::IMaterialRenderer* rnd = driver->getMaterialRenderer(type);
            if (!isObject(type))
            {
#ifdef DEBUG
                Log::warn("material", "Unhandled (animated) material type : %d", type);
#endif
                continue;
            }
            GLMesh &mesh = GLmeshes[i];
            video::E_VERTEX_TYPE vt = mb->getVertexType();
            Material* material = material_manager->getMaterialFor(mb->getMaterial().getTexture(0), mb);

            if (rnd->isTransparent())
            {
                TransparentMaterial TranspMat = getTransparentMaterialFromType(type, vt, MaterialTypeParam, material);
                TransparentMesh[TranspMat].push_back(&mesh);
            }
            else if (mesh.m_render_info != NULL && mesh.m_render_info->isTransparent())
            {
                if (mesh.VAOType == video::EVT_SKINNED_MESH)
                    TransparentMesh[TM_TRANSLUCENT_SKN].push_back(&mesh);
                else if (mesh.VAOType == video::EVT_TANGENTS)
                    TransparentMesh[TM_TRANSLUCENT_TAN].push_back(&mesh);
                else
                    TransparentMesh[TM_TRANSLUCENT_STD].push_back(&mesh);
            }
            else
            {
                Material::ShaderType MatType = getMeshMaterialFromType(type, vt, material, NULL);
                MeshSolidMaterial[MatType].push_back(&mesh);
            }
            if (m_skinned_mesh) ssmb->VertexType = prev_type;
        }
        isMaterialInitialized = true;
    }

    for (u32 i = 0; i < m->getMeshBufferCount(); ++i)
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

void STKAnimatedMesh::updateGL()
{
    if (!isGLInitialized)
    {
        for (u32 i = 0; i < Mesh->getMeshBufferCount(); ++i)
        {
            scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
            if (!mb)
                continue;
            scene::SSkinMeshBuffer* ssmb = NULL;
            video::E_VERTEX_TYPE prev_type = mb->getVertexType();
            if (m_skinned_mesh)
            {
                ssmb = dynamic_cast<scene::SSkinMeshBuffer*>(mb);
                ssmb->VertexType = video::EVT_SKINNED_MESH;
            }

            video::IVideoDriver* driver = SceneManager->getVideoDriver();
            video::E_MATERIAL_TYPE type = mb->getMaterial().MaterialType;
            video::IMaterialRenderer* rnd = driver->getMaterialRenderer(type);
            GLMesh &mesh = GLmeshes[i];

            if (!rnd->isTransparent())
            {
                Material* material = material_manager->getMaterialFor(mb->getMaterial().getTexture(0), mb);
                Material* material2 = NULL;
                if (mb->getMaterial().getTexture(1) != NULL)
                    material2 = material_manager->getMaterialFor(mb->getMaterial().getTexture(1), mb);

                Material::ShaderType MatType = getMeshMaterialFromType(type, mb->getVertexType(), material, material2);
                initTextures(mesh, MatType);
            }
            else
                initTexturesTransparent(mesh);

            if (CVS->isARBBaseInstanceUsable())
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
            if (m_skinned_mesh) ssmb->VertexType = prev_type;
        }
        isGLInitialized = true;
    }

    if (useHardwareSkinning() && m_skinned_mesh->getTotalJoints() == 0) return;

    scene::IMesh* m = getMeshForCurrentFrame();
    if (useHardwareSkinning())
    {
        m_skinning_offset = -1;
        return;
    }

    for (u32 i = 0; i<m->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = m->getMeshBuffer(i);
        const video::SMaterial& material = ReadOnlyMaterials ? mb->getMaterial() : Materials[i];
        if (isObject(material.MaterialType))
        {

            size_t size = mb->getVertexCount() * GLmeshes[i].Stride, offset = GLmeshes[i].vaoBaseVertex * GLmeshes[i].Stride;
            void *buf;
            if (CVS->supportsAsyncInstanceUpload())
            {
                buf = VAOManager::getInstance()->getVBOPtr(mb->getVertexType());
                buf = (char *)buf + offset;
            }
            else
            {
                glBindVertexArray(0);
                if (CVS->isARBBaseInstanceUsable())
                    glBindBuffer(GL_ARRAY_BUFFER, VAOManager::getInstance()->getVBO(mb->getVertexType()));
                else
                    glBindBuffer(GL_ARRAY_BUFFER, GLmeshes[i].vertex_buffer);
                GLbitfield bitfield = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT;
                buf = glMapBufferRange(GL_ARRAY_BUFFER, offset, size, bitfield);
            }
            memcpy(buf, mb->getVertices(), size);
            if (!CVS->supportsAsyncInstanceUpload())
            {
                glUnmapBuffer(GL_ARRAY_BUFFER);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
        }
    }

}

void STKAnimatedMesh::render()
{
    ++PassCount;

    updateNoGL();
    updateGL();
}

int STKAnimatedMesh::getTotalJoints() const
{
    return m_skinned_mesh->getTotalJoints();
}

void STKAnimatedMesh::resetSkinningState(scene::IAnimatedMesh* mesh)
{
    if (!CVS->supportsHardwareSkinning()) return;
    m_skinning_offset = -1;
    m_skinned_mesh = NULL;
    if (mesh == NULL) return;
    for (u32 i = 0; i < mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
        if (!mb)
            continue;
        if (mb->getVertexType() == video::EVT_2TCOORDS)
            return;
    }
    setHardwareSkinning(true);
    if (m_skinned_mesh)
        m_skinned_mesh->convertForSkinning();
}

scene::IMesh* STKAnimatedMesh::getMeshForCurrentFrame(SkinningCallback sc,
                                                      int offset)
{
    if (!useHardwareSkinning())
        return scene::CAnimatedMeshSceneNode::getMeshForCurrentFrame();
    if (m_skinning_offset == -1)
        return Mesh;

    return scene::CAnimatedMeshSceneNode::getMeshForCurrentFrame
        (uploadJoints, m_skinning_offset);
}

void STKAnimatedMesh::setHardwareSkinning(bool val)
{
    if (!CVS->supportsHardwareSkinning()) return;
    if (val)
        m_skinned_mesh = dynamic_cast<scene::CSkinnedMesh*>(Mesh);
    else
        m_skinned_mesh = NULL;
}

void STKAnimatedMesh::uploadJoints(const irr::core::matrix4& m,
                                   int joint, int offset)
{
    assert(offset != -1);
#ifdef USE_GLES2
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, (offset >> 6) + joint, 16, 1, GL_RGBA,
        GL_FLOAT, m.pointer());
#else
    glBufferSubData(GL_TEXTURE_BUFFER, offset + joint * 16 * sizeof(float),
        16 * sizeof(float), m.pointer());
#endif
}

#endif   // !SERVER_ONLY
