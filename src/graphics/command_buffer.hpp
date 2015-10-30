//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015 SuperTuxKart-Team
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

#ifndef HEADER_COMMAND_BUFFER_HPP
#define HEADER_COMMAND_BUFFER_HPP

#include "graphics/gl_headers.hpp"
#include "graphics/material.hpp"

#include "graphics/stk_mesh_scene_node.hpp"
#include "graphics/vao_manager.hpp"
#include <irrlicht.h>
#include <unordered_map>


typedef std::vector<std::pair<GLMesh *, irr::scene::ISceneNode*> > InstanceList;
typedef std::unordered_map <irr::scene::IMeshBuffer *, InstanceList > MeshMap;


template<typename InstanceData>
void fillOriginOrientationScale(scene::ISceneNode *node, InstanceData &instance)
{
    const core::matrix4 &mat = node->getAbsoluteTransformation();
    const core::vector3df &Origin = mat.getTranslation();
    const core::vector3df &Orientation = mat.getRotationDegrees();
    const core::vector3df &Scale = mat.getScale();
    instance.Origin.X = Origin.X;
    instance.Origin.Y = Origin.Y;
    instance.Origin.Z = Origin.Z;
    instance.Orientation.X = Orientation.X;
    instance.Orientation.Y = Orientation.Y;
    instance.Orientation.Z = Orientation.Z;
    instance.Scale.X = Scale.X;
    instance.Scale.Y = Scale.Y;
    instance.Scale.Z = Scale.Z;        
}

template<typename InstanceData>
struct InstanceFiller
{
    static void add(GLMesh *, scene::ISceneNode *, InstanceData &);
};

template<typename T>
void FillInstances_impl(InstanceList instance_list,
                        T * instance_buffer,
                        DrawElementsIndirectCommand *command_buffer,
                        size_t &instance_buffer_offset,
                        size_t &command_buffer_offset,
                        size_t &poly_count)
{
    // Should never be empty
    GLMesh *mesh = instance_list.front().first;
    size_t initial_offset = instance_buffer_offset;

    for (unsigned i = 0; i < instance_list.size(); i++)
    {
        auto &Tp = instance_list[i];
        scene::ISceneNode *node = Tp.second;
        InstanceFiller<T>::add(mesh, node, instance_buffer[instance_buffer_offset++]);
        assert(instance_buffer_offset * sizeof(T) < 10000 * sizeof(InstanceDataDualTex)); //TODO
    }

    DrawElementsIndirectCommand &CurrentCommand = command_buffer[command_buffer_offset++];
    CurrentCommand.baseVertex = mesh->vaoBaseVertex;
    CurrentCommand.count = mesh->IndexCount;
    CurrentCommand.firstIndex = mesh->vaoOffset / 2;
    CurrentCommand.baseInstance = initial_offset;
    CurrentCommand.instanceCount = instance_buffer_offset - initial_offset;

    poly_count += (instance_buffer_offset - initial_offset) * mesh->IndexCount / 3;
}

template<typename T>
void FillInstances( const MeshMap &gathered_GL_mesh,
                    std::vector<GLMesh *> &instanced_list,
                    T *instance_buffer,
                    DrawElementsIndirectCommand *command_buffer,
                    size_t &instance_buffer_offset,
                    size_t &command_buffer_offset,
                    size_t &poly_count)
{
    auto It = gathered_GL_mesh.begin(), E = gathered_GL_mesh.end();
    for (; It != E; ++It)
    {
        FillInstances_impl<T>(It->second, instance_buffer, command_buffer, instance_buffer_offset, command_buffer_offset, poly_count);
        if (!CVS->isAZDOEnabled())
            instanced_list.push_back(It->second.front().first);
    }
}


class CommandBuffer
{
protected:
    GLuint m_draw_indirect_cmd_id;
    DrawElementsIndirectCommand *m_draw_indirect_cmd;
    size_t *m_offset;
    size_t *m_size;
    size_t m_poly_count;
    size_t m_instance_buffer_offset;
    size_t m_command_buffer_offset;

    template<typename T>
    void fillMaterial(int material_id,
                      MeshMap *mesh_map,
                      std::vector<GLMesh *> &instanced_list,
                      T *instance_buffer);

public:
    CommandBuffer();
    virtual ~CommandBuffer();

    inline size_t getPolyCount() {return m_poly_count;}

    inline void bind()
    {
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_draw_indirect_cmd_id);        
    }
 
     /** Draw the i-th mesh with the specified material
     * (require GL_ARB_base_instance and GL_ARB_draw_indirect extensions)
     *  \param shader_type Only render meshes with the specified material
     */ 
    inline void drawIndirect(Material::ShaderType shader_type, int i)
    {
        glDrawElementsIndirect(GL_TRIANGLES,
                               GL_UNSIGNED_SHORT,
                               (const void*)(m_offset[static_cast<int>(shader_type) + i] * sizeof(DrawElementsIndirectCommand)));
    }
 
    /** Draw the meshes with the specified material
     * (require AZDO extensions)
     *  \param shader_type Only render meshes with the specified material
     */ 
    inline void multidrawIndirect(Material::ShaderType shader_type)
    {
        glMultiDrawElementsIndirect(GL_TRIANGLES,
                                    GL_UNSIGNED_SHORT,
                                    (const void*)(m_offset[static_cast<int>(shader_type)] * sizeof(DrawElementsIndirectCommand)),
                                    (int) m_size[static_cast<int>(shader_type)],
                                    sizeof(DrawElementsIndirectCommand));
    }
};

class SolidCommandBuffer: public CommandBuffer
{
public:
    SolidCommandBuffer();
    void fill(MeshMap *mesh_map, std::vector<GLMesh *> instanced_lists[]);
};

class ShadowCommandBuffer: public CommandBuffer
{
public:
    ShadowCommandBuffer();
    void fill(MeshMap *mesh_map, std::vector<GLMesh *> instanced_lists[]);
};

class ReflectiveShadowMapCommandBuffer: public CommandBuffer
{
public:
    ReflectiveShadowMapCommandBuffer();
    void fill(MeshMap *mesh_map, std::vector<GLMesh *> instanced_lists[]);
};

class GlowCommandBuffer: public CommandBuffer
{
public:
    GlowCommandBuffer();
    void fill(MeshMap *mesh_map, std::vector<GLMesh *> instanced_lists[]);
};

#endif //HEADER_COMMAND_BUFFER_HPP
