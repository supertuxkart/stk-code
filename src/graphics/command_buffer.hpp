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

#ifndef SERVER_ONLY

#include "graphics/draw_tools.hpp"
#include "graphics/gl_headers.hpp"
#include "graphics/material.hpp"
#include "graphics/materials.hpp"
#include "graphics/vao_manager.hpp"
#include <irrlicht.h>
#include <array>
#include <unordered_map>

// ----------------------------------------------------------------------------
/** Fill origin, orientation and scale attributes
 *  \param node The scene node of the mesh
 *  \param[out] instance The instance to fill
 */
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

// ----------------------------------------------------------------------------
template<typename InstanceData>
struct InstanceFiller
{
    static void add(GLMesh *, const InstanceSettings&, InstanceData &);
};

// ----------------------------------------------------------------------------
/** Fill a command buffer (in video RAM) with meshes data
 *  \param instance_list A vector of scene nodes associated with the same mesh
 *  \param[in,out] instance_buffer Mesh data (position, orientation, textures, etc)
 *  \param[in,out] command_buffer A pointer to meshes data in VRAM.
 *  \param[in,out] instance_buffer_offset Current offset for instance_buffer.
 *                 Will be updated to next offset.
 *  \param[in,out] command_buffer_offset Current offset for command_buffer.
 *                 Will be updated to next offset.
 *  \param[in,out] poly_count Number of triangles. Will be updated.
 */
template<typename T>
void FillInstances_impl(const InstanceList& instance_list,
                        T * instance_buffer,
                        DrawElementsIndirectCommand *command_buffer,
                        unsigned int &instance_buffer_offset,
                        unsigned int &command_buffer_offset,
                        unsigned int&poly_count)
{
    // Should never be empty
    GLMesh *mesh = instance_list.m_mesh;
    unsigned int initial_offset = instance_buffer_offset;

    for (unsigned i = 0; i < instance_list.m_instance_settings.size(); i++)
    {
        InstanceFiller<T>::add(mesh, instance_list.m_instance_settings[i],
            instance_buffer[instance_buffer_offset++]);
        assert(instance_buffer_offset * sizeof(T) < 10000 * sizeof(InstanceDataThreeTex));
    }

    DrawElementsIndirectCommand &CurrentCommand = command_buffer[command_buffer_offset++];
    CurrentCommand.baseVertex = mesh->vaoBaseVertex;
    CurrentCommand.count = mesh->IndexCount;
    CurrentCommand.firstIndex = GLuint(mesh->vaoOffset / 2);
    CurrentCommand.baseInstance = initial_offset;
    CurrentCommand.instanceCount = instance_buffer_offset - initial_offset;
    
    poly_count += (instance_buffer_offset - initial_offset) * mesh->IndexCount / 3;
}

// ----------------------------------------------------------------------------
/** Bind textures for second rendering pass.
 *  \param mesh The mesh which owns the textures
 *  \param prefilled_tex Textures which have been drawn during previous rendering passes.
 */
template<typename T>
void expandTexSecondPass(const GLMesh &mesh,
                         const std::vector<GLuint> &prefilled_tex)
{
    TexExpander<typename T::InstancedSecondPassShader>::template
        expandTex(mesh, T::SecondPassTextures, prefilled_tex[0],
                  prefilled_tex[1], prefilled_tex[2]);
}
                   
// ----------------------------------------------------------------------------
/** Give acces textures for second rendering pass in shaders 
 * without first binding them in order to reduce driver overhead.
 * (require GL_ARB_bindless_texture extension) 
 *  \param handles The handles to textures which have been drawn 
 *                 during previous rendering passes.
 */ 
template<typename T>
void expandHandlesSecondPass(const std::vector<uint64_t> &handles)
{
    uint64_t nulltex[10] = {};
    HandleExpander<typename T::InstancedSecondPassShader>::template
        expand(nulltex, T::SecondPassTextures,
               handles[0], handles[1], handles[2]);
}

#if !defined(USE_GLES2)
// ----------------------------------------------------------------------------
/**
 *  \class CommandBuffer
 *  \brief Template class to draw meshes with as few draw calls as possible
 *
 */
template<int N>
class CommandBuffer
{

protected:
    GLuint m_draw_indirect_cmd_id;
    DrawElementsIndirectCommand *m_draw_indirect_cmd;
    
    std::array<std::vector<GLMesh *>, N> m_meshes;
    std::array<unsigned int,N> m_offset;
    std::array<unsigned int,N> m_size;
    
    unsigned int m_poly_count;
    unsigned int m_instance_buffer_offset;
    unsigned int m_command_buffer_offset;

    void clearMeshes();
    void mapIndirectBuffer();
    
    // ------------------------------------------------------------------------
    /** Send in VRAM all meshes associated with same material
     *  \param material_id The id of the material shared by the meshes
     *  \param mesh_map List of meshes
     *  \param[in,out] instance_buffer Meshes data (position, orientation, textures, etc)
     */
    template<typename T, typename MeshMap>
    void fillMaterial(int material_id,
                      MeshMap *mesh_map,
                      T *instance_buffer)
    {
        m_offset[material_id] = m_command_buffer_offset;
        for(auto& instance_list : mesh_map[material_id])
        {
            FillInstances_impl<T>(instance_list.second,
                                  instance_buffer,
                                  m_draw_indirect_cmd,
                                  m_instance_buffer_offset,
                                  m_command_buffer_offset,
                                  m_poly_count);
            if (!CVS->isAZDOEnabled())
                m_meshes[material_id].push_back(instance_list.second.m_mesh);
        }
                
        m_size[material_id] = m_command_buffer_offset - m_offset[material_id];
    }
    
    // ------------------------------------------------------------------------
    /** Send into VRAM all meshes associated with same type of material
     *  \param mesh_map List of meshes to send into VRAM
     *  \param material_list Ids of materials: meshes associated to these materials
     *         will be sent into VRAM
     *  \param instance_type The type of material
     * 
     */
    template<typename InstanceData, typename MeshMap>
    void fillInstanceData(MeshMap *mesh_map,
                          const std::vector<int> &material_list,
                          InstanceType instance_type)
    {        
        InstanceData *instance_buffer;
        
        if (CVS->supportsAsyncInstanceUpload())
        {
            instance_buffer = (InstanceData*)VAOManager::getInstance()->
                              getInstanceBufferPtr(instance_type);
        }
        else
        {
            glBindBuffer(GL_ARRAY_BUFFER,
                         VAOManager::getInstance()->getInstanceBuffer(instance_type));
            instance_buffer = (InstanceData*)
                glMapBufferRange(GL_ARRAY_BUFFER, 0,
                                 10000 * sizeof(InstanceDataThreeTex),
                                 GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        }
        
        for(int material_id: material_list)
        {
            fillMaterial( material_id,
                          mesh_map,
                          instance_buffer);
        }
        
        if (!CVS->supportsAsyncInstanceUpload())
        {
            glUnmapBuffer(GL_ARRAY_BUFFER);
        }
        
    }
    
public:
    CommandBuffer();
    virtual ~CommandBuffer() { glDeleteBuffers(1, &m_draw_indirect_cmd_id); }

    inline unsigned int getPolyCount() const {return m_poly_count;}

    inline void bind() const
    {
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_draw_indirect_cmd_id);        
    }
}; //CommandBuffer

// ----------------------------------------------------------------------------
/**
 *  \class SolidCommandBuffer
 *  This class is used for rendering meshes during solid first pass 
 *  and solid second pass.
 */
class SolidCommandBuffer: public CommandBuffer<static_cast<int>(Material::SHADERTYPE_COUNT)>
{
public:
    SolidCommandBuffer();
    void fill(MeshMap *mesh_map);
    
    // ----------------------------------------------------------------------------
    /** First rendering pass; draw all meshes associated with the same material
     * Require OpenGL 4.0 (or higher)
     * or GL_ARB_base_instance and GL_ARB_draw_indirect extensions)
     * 
     *  \tparam T The material
     *  \param uniforms Uniforms needed by the shader associated with T material
     */
    template<typename T, typename...Uniforms>
    void drawIndirectFirstPass(Uniforms...uniforms) const
    {
        T::InstancedFirstPassShader::getInstance()->use();
        T::InstancedFirstPassShader::getInstance()->setUniforms(uniforms...);
        handleSkinning(T::InstancedFirstPassShader::getInstance());
        glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType,
                                                                    T::Instance));
        for (unsigned i = 0; i < m_meshes[T::MaterialType].size(); i++)
        {
            GLMesh *mesh = m_meshes[T::MaterialType][i];
#ifdef DEBUG
            if (mesh->VAOType != T::VertexType)
            {
                Log::error("CommandBuffer", "Wrong instanced vertex format (hint : %s)", 
                    mesh->textures[0]->getName().getPath().c_str());
                continue;
            }
#endif
            TexExpander<typename T::InstancedFirstPassShader>::template
                expandTex(*mesh, T::FirstPassTextures);
            if (!mesh->mb->getMaterial().BackfaceCulling)
                glDisable(GL_CULL_FACE);
            glDrawElementsIndirect(GL_TRIANGLES,
                                   GL_UNSIGNED_SHORT,
                                   (const void*)((m_offset[T::MaterialType] + i) * sizeof(DrawElementsIndirectCommand)));
            if (!mesh->mb->getMaterial().BackfaceCulling)
                glEnable(GL_CULL_FACE);
        }
    } //drawIndirectFirstPass

    // ----------------------------------------------------------------------------
    /** First rendering pass; draw all meshes associated with the same material
     * Faster than drawIndirectFirstPass.
     * Require OpenGL AZDO extensions
     *  \tparam T The material
     *  \param uniforms Uniforms needed by the shader associated with T material
     */
    template<typename T, typename...Uniforms>
    void multidrawFirstPass(Uniforms...uniforms) const
    {
        if (m_size[T::MaterialType])
        {
            T::InstancedFirstPassShader::getInstance()->use();
            T::InstancedFirstPassShader::getInstance()->setUniforms(uniforms...);
            handleSkinning(T::InstancedFirstPassShader::getInstance());
            glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType,
                                                                        T::Instance));
            glMultiDrawElementsIndirect(GL_TRIANGLES,
                                        GL_UNSIGNED_SHORT,
                                        (const void*)(m_offset[T::MaterialType] * sizeof(DrawElementsIndirectCommand)),
                                        (int) m_size[T::MaterialType],
                                        sizeof(DrawElementsIndirectCommand));
        }
    }   // multidrawFirstPass

    // ----------------------------------------------------------------------------
    /** Second rendering pass; draw all meshes associated with the same material
     * Require OpenGL 4.0 (or higher)
     * or GL_ARB_base_instance and GL_ARB_draw_indirect extensions)
     * 
     *  \tparam T The material
     *  \param prefilled_tex Textures filled during previous rendering passes (diffuse, depth, etc)
     *  \param uniforms Uniforms needed by the shader associated with T material
     */
    template<typename T, typename...Uniforms>
    void drawIndirectSecondPass(const std::vector<GLuint> &prefilled_tex,
                                Uniforms...uniforms                       ) const
    {
        T::InstancedSecondPassShader::getInstance()->use();
        T::InstancedSecondPassShader::getInstance()->setUniforms(uniforms...);
        handleSkinning(T::InstancedSecondPassShader::getInstance());
        glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType,
                                                                    T::Instance));
        for (unsigned i = 0; i < m_meshes[T::MaterialType].size(); i++)
        {
            GLMesh *mesh = m_meshes[T::MaterialType][i];
            expandTexSecondPass<T>(*mesh, prefilled_tex);
            if (!mesh->mb->getMaterial().BackfaceCulling)
                glDisable(GL_CULL_FACE);
            glDrawElementsIndirect(GL_TRIANGLES,
                                   GL_UNSIGNED_SHORT,
                                   (const void*)((m_offset[T::MaterialType] + i) * sizeof(DrawElementsIndirectCommand)));
            if (!mesh->mb->getMaterial().BackfaceCulling)
                glEnable(GL_CULL_FACE);
        }
    } //drawIndirectSecondPass

    // ----------------------------------------------------------------------------
    /** Second rendering pass; draw all meshes associated with the same material
     * Faster than drawIndirectSecondPass.
     * Require OpenGL AZDO extensions
     * 
     *  \tparam T The material
     *  \param handles Handles to textures filled during previous rendering passes
     *                 (diffuse, depth, etc)
     *  \param uniforms Uniforms needed by the shader associated with T material
     */
    template<typename T, typename...Uniforms>
    void multidraw2ndPass(const std::vector<uint64_t> &handles,
                          Uniforms... uniforms) const
    {
        if (m_size[T::MaterialType])
        {
            T::InstancedSecondPassShader::getInstance()->use();
            T::InstancedSecondPassShader::getInstance()->setUniforms(uniforms...);
            handleSkinning(T::InstancedSecondPassShader::getInstance());
            glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType,
                                                                        T::Instance));
            expandHandlesSecondPass<T>(handles);
            glMultiDrawElementsIndirect(GL_TRIANGLES,
                                        GL_UNSIGNED_SHORT,
                                        (const void*)(m_offset[T::MaterialType] * sizeof(DrawElementsIndirectCommand)),
                                        (int) m_size[T::MaterialType],
                                        sizeof(DrawElementsIndirectCommand));
        }
    }   // multidraw2ndPass

    // ----------------------------------------------------------------------------
    /** Draw normals (debug): draw all meshes associated with the same material
     * Require OpenGL 4.0 (or higher)
     * or GL_ARB_base_instance and GL_ARB_draw_indirect extensions)
     * 
     *  \tparam T The material
     */
    template<typename T>
    void drawIndirectNormals() const
    {
        NormalVisualizer::getInstance()->use();
        NormalVisualizer::getInstance()->setUniforms(video::SColor(255, 0, 255, 0));
        
        glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType,
                                                                    T::Instance));
        for (unsigned i = 0; i < m_meshes[T::MaterialType].size(); i++)
        {
            glDrawElementsIndirect(GL_TRIANGLES,
                                   GL_UNSIGNED_SHORT,
                                   (const void*)((m_offset[T::MaterialType] + i) * sizeof(DrawElementsIndirectCommand)));        }
    }   // drawIndirectNormals

    // ----------------------------------------------------------------------------
    /** Draw normals (debug): draw all meshes associated with the same material
     * Faster than drawIndirectNormals.
     * Require OpenGL AZDO extensions
     * 
     *  \tparam T The material
     */
    template<typename T>
    void multidrawNormals() const
    {

        if (m_size[T::MaterialType])
        {
            NormalVisualizer::getInstance()->use();
            NormalVisualizer::getInstance()->setUniforms(video::SColor(255, 0, 255, 0));

            glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType,
                                                                        T::Instance));
            
            glMultiDrawElementsIndirect(GL_TRIANGLES,
                                        GL_UNSIGNED_SHORT,
                                        (const void*)(m_offset[T::MaterialType] * sizeof(DrawElementsIndirectCommand)),
                                        (int) m_size[T::MaterialType],
                                        sizeof(DrawElementsIndirectCommand));
        }
    }   // multidrawNormals
}; //SolidCommandBuffer

// ----------------------------------------------------------------------------
/**
 *  \class ShadowCommandBuffer
 *  This class is used for rendering shadows.
 */
class ShadowCommandBuffer: public CommandBuffer<4*static_cast<int>(Material::SHADERTYPE_COUNT)>
{
public:
    ShadowCommandBuffer();
    void fill(MeshMap *mesh_map);
    
    // ----------------------------------------------------------------------------
    /** Draw shadowmaps for meshes with the same material
     * Require OpenGL 4.0 (or higher)
     * or GL_ARB_base_instance and GL_ARB_draw_indirect extensions)
     * 
     *  \tparam T The material
     *  \param uniforms Uniforms needed by the shadow shader associated with T material
     *  \param cascade The cascade id (see cascading shadow maps)
     */    
    template<typename T, typename...Uniforms>
    void drawIndirect(unsigned cascade, Uniforms ...uniforms) const
    {
        T::InstancedShadowPassShader::getInstance()->use();
        T::InstancedShadowPassShader::getInstance()->setUniforms(cascade, uniforms...);
        handleSkinning(T::InstancedShadowPassShader::getInstance());
        glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType,
                                                                    InstanceTypeShadow));
                                                                    
        int material_id = T::MaterialType + cascade * Material::SHADERTYPE_COUNT;

        for (unsigned i = 0; i < m_meshes[material_id].size(); i++)
        {
            GLMesh *mesh = m_meshes[material_id][i];
            
            TexExpander<typename T::InstancedShadowPassShader>::template 
                expandTex(*mesh, T::ShadowTextures);
            glDrawElementsIndirect(GL_TRIANGLES,
                                   GL_UNSIGNED_SHORT,
                                   (const void*)((m_offset[material_id] + i)
                                   * sizeof(DrawElementsIndirectCommand)));
        }  // for i
    }   // drawIndirect
    
    // ----------------------------------------------------------------------------
    /** Draw shadowmaps for meshes with the same material
     * Faster than drawIndirect.
     * Require OpenGL AZDO extensions
     * 
     *  \tparam T The material
     *  \param uniforms Uniforms needed by the shadow shader associated with T material
     *  \param cascade The cascade id (see cascading shadow maps)
     */ 
    template<typename T, typename...Uniforms>
    void multidrawShadow(unsigned cascade, Uniforms ...uniforms) const
    {
        int material_id = T::MaterialType + cascade * Material::SHADERTYPE_COUNT;
                            
        if (m_size[material_id])
        {
            T::InstancedShadowPassShader::getInstance()->use();
            T::InstancedShadowPassShader::getInstance()->setUniforms(cascade, uniforms...);
            handleSkinning(T::InstancedShadowPassShader::getInstance());
            glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType,
                                                                        InstanceTypeShadow));
            glMultiDrawElementsIndirect(GL_TRIANGLES,
                                        GL_UNSIGNED_SHORT,
                                        (const void*)(m_offset[material_id] * sizeof(DrawElementsIndirectCommand)),
                                        (int) m_size[material_id],
                                        sizeof(DrawElementsIndirectCommand));
        }
    }   // multidrawShadow
}; //ShadowCommandBuffer

// ----------------------------------------------------------------------------
/**
 *  \class ReflectiveShadowMapCommandBuffer
 *  This class is used for rendering the reflective shadow map once per track.
 */
class ReflectiveShadowMapCommandBuffer: public CommandBuffer<static_cast<int>(Material::SHADERTYPE_COUNT)>
{
public:
    ReflectiveShadowMapCommandBuffer();
    void fill(MeshMap *mesh_map);
    
    // ----------------------------------------------------------------------------
    /** Draw reflective shadow map for meshes with the same material
     * Require OpenGL 4.0 (or higher)
     * or GL_ARB_base_instance and GL_ARB_draw_indirect extensions)
     * 
     *  \tparam T The material
     *  \param uniforms Uniforms needed by the shadow shader associated with T material
     */    
    template<typename T, typename...Uniforms>
    void drawIndirect(Uniforms ...uniforms) const
    {
        T::InstancedRSMShader::getInstance()->use();
        T::InstancedRSMShader::getInstance()->setUniforms(uniforms...);
        
        glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType,
                                                                    InstanceTypeRSM));
                                                                    
        for (unsigned i = 0; i < m_meshes[T::MaterialType].size(); i++)
        {
            GLMesh *mesh = m_meshes[T::MaterialType][i];

            TexExpander<typename T::InstancedRSMShader>::template expandTex(*mesh, T::RSMTextures);
            glDrawElementsIndirect(GL_TRIANGLES,
                                   GL_UNSIGNED_SHORT,
                                   (const void*)((m_offset[T::MaterialType] + i) * sizeof(DrawElementsIndirectCommand)));
        }
    } //drawIndirect
    
    // ----------------------------------------------------------------------------
    /** Draw reflective shadow map for meshes with the same material
     * Faster than drawIndirect.
     * Require OpenGL AZDO extensions
     * 
     *  \tparam T The material
     *  \param uniforms Uniforms needed by the shadow shader associated with T material
     */ 
    template<typename T, typename... Uniforms>
    void multidraw(Uniforms...uniforms) const
    {
        if (m_size[T::MaterialType])
        {
            T::InstancedRSMShader::getInstance()->use();
            T::InstancedRSMShader::getInstance()->setUniforms(uniforms...);
        
            glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(T::VertexType,
                                                                        InstanceTypeRSM));
            glMultiDrawElementsIndirect(GL_TRIANGLES,
                                        GL_UNSIGNED_SHORT,
                                        (const void*)(m_offset[T::MaterialType] * sizeof(DrawElementsIndirectCommand)),
                                        (int) m_size[T::MaterialType],
                                        sizeof(DrawElementsIndirectCommand));
        }
    }   // multidraw
}; //ReflectiveShadowMapCommandBuffer


// ----------------------------------------------------------------------------
/**
 *  \class InstancedColorizeShader
 *  Draw meshes with glow color.
 */
class InstancedColorizeShader : public Shader<InstancedColorizeShader>
{
public:
    InstancedColorizeShader()
    {
        loadProgram(OBJECT, GL_VERTEX_SHADER,   "glow_object.vert",
                            GL_FRAGMENT_SHADER, "glow_object.frag");
        assignUniforms();
    }   // InstancedColorizeShader
};   // InstancedColorizeShader

// ----------------------------------------------------------------------------
/**
 *  \class GlowCommandBuffer
 *  This class is used for rendering glowing meshes.
 */
class GlowCommandBuffer: public CommandBuffer<1>
{
public:
    GlowCommandBuffer();
    void fill(MeshMap *mesh_map);

    // ----------------------------------------------------------------------------
    /** Draw glowing meshes.
     * Require OpenGL 4.0 (or higher)
     * or GL_ARB_base_instance and GL_ARB_draw_indirect extensions)
     */      
    void drawIndirect() const
    {
        InstancedColorizeShader::getInstance()->use();
        glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(irr::video::EVT_STANDARD,
                                                                    InstanceTypeGlow));
        for (unsigned i = 0; i < m_meshes[0].size(); i++)
        {
            glDrawElementsIndirect(GL_TRIANGLES,
                                   GL_UNSIGNED_SHORT,
                                   (const void*)((m_offset[0] + i) * sizeof(DrawElementsIndirectCommand)));
        }
    } //drawIndirect
    
    // ----------------------------------------------------------------------------
    /** Draw glowing meshes.
     * Faster than drawIndirect.
     * Require OpenGL AZDO extensions
     */  
    void multidraw() const
    {
        InstancedColorizeShader::getInstance()->use();
        glBindVertexArray(VAOManager::getInstance()->getInstanceVAO(irr::video::EVT_STANDARD,
                                                                    InstanceTypeGlow));
        if (m_size[0])
        {
            glMultiDrawElementsIndirect(GL_TRIANGLES,
                                        GL_UNSIGNED_SHORT,
                                        (const void*)(m_offset[0] * sizeof(DrawElementsIndirectCommand)),
                                        (int) m_size[0],
                                        sizeof(DrawElementsIndirectCommand));
        }
    } // multidraw
};
#endif   // !defined(USE_GLES2)
#endif   // !SERVER_ONLY
#endif   // HEADER_COMMAND_BUFFER_HPP
