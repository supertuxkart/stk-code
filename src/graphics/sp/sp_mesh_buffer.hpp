//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2017 SuperTuxKart-Team
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

#ifndef HEADER_SP_MESH_BUFFER_HPP
#define HEADER_SP_MESH_BUFFER_HPP

#include "graphics/gl_headers.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/sp/sp_instanced_data.hpp"
#include "utils/types.hpp"

#include <IMeshBuffer.h>
#include <S3DVertex.h>

#include <array>
#include <cassert>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

using namespace irr;
using namespace scene;

class Material;

namespace SP
{
class SPTexture;

class SPMeshBuffer : public IMeshBuffer
{
private:
    std::vector<std::tuple<size_t/*first_index_id*/,
        unsigned/*indices_count*/, Material*> > m_stk_material;

    std::vector<std::array<std::shared_ptr<SPTexture>, 6> > m_textures;

    std::unordered_map<std::string, unsigned> m_tex_cmp;

    std::vector<video::S3DVertexSkinnedMesh> m_vertices;

    std::vector<uint16_t> m_indices;

    core::aabbox3d<f32> m_bounding_box;

    std::vector<SPInstancedData> m_ins_dat[DCT_FOR_VAO];

    void* m_ins_dat_mapped_ptr[DCT_FOR_VAO];

    GLuint m_ibo, m_vbo;

    unsigned m_gl_instance_size[DCT_FOR_VAO];

    GLuint m_vao[DCT_FOR_VAO];

    GLuint m_ins_array[DCT_FOR_VAO];

    unsigned m_pitch;

    bool m_uploaded_gl;

    bool m_uploaded_instance;

    bool m_skinned;

    const bool m_vertex_color;

    // ------------------------------------------------------------------------
    bool initTexture();

public:
    SPMeshBuffer(bool vertex_color = true) : m_vertex_color(vertex_color)
    {
#ifdef _DEBUG
        setDebugName("SMeshBuffer");
#endif
        m_stk_material.resize(1, std::make_tuple(0u, 0u, nullptr));

        for (unsigned i = 0; i < DCT_FOR_VAO; i++)
        {
            m_ins_dat_mapped_ptr[i] = NULL;
            m_gl_instance_size[i] = 1;
            m_vao[i] = 0;
            m_ins_array[i] = 0;
        }

        m_pitch = 0;
        m_ibo = 0;
        m_vbo = 0;
        m_uploaded_gl = false;
        m_uploaded_instance = false;
        m_skinned = false;
    }
    // ------------------------------------------------------------------------
    ~SPMeshBuffer();
    // ------------------------------------------------------------------------
    bool combineMeshBuffer(SPMeshBuffer* spmb)
    {
        // We only use 16bit vertices
        if (spmb->m_vertices.size() + m_vertices.size() > 65536)
        {
            return false;
        }
        const uint16_t old_vtx_count = (uint16_t)m_vertices.size();
        m_vertices.insert(m_vertices.end(), spmb->m_vertices.begin(),
            spmb->m_vertices.end());
        for (uint16_t& idx : spmb->m_indices)
        {
            idx += old_vtx_count;
        }
        m_stk_material.emplace_back(getIndexCount(), spmb->getIndexCount(),
            std::get<2>(spmb->m_stk_material[0]));
        m_indices.insert(m_indices.end(), spmb->m_indices.begin(),
            spmb->m_indices.end());
        return true;
    }
    // ------------------------------------------------------------------------
    void bindVAO(DrawCallType dct) const     { glBindVertexArray(m_vao[dct]); }
    // ------------------------------------------------------------------------
    void draw(DrawCallType dct, int material_id = -1,
              bool bindless_texture = false) const
    {
        glBindVertexArray(m_vao[dct]);
        if (material_id == -1 || bindless_texture)
        {
            /*if (bindless_texture)
            // Only nvidia gpu allow texture with non-dynamically-uniform
            // handles (texture handles that aren't constant within a same draw)
            {
                for (unsigned i = 0; i < m_stk_material.size(); i++)
                {
                    glDrawElementsInstanced(GL_TRIANGLES,
                        std::get<1>(m_stk_material[i]),
                        GL_UNSIGNED_SHORT,
                        (void*)(std::get<0>(m_stk_material[i]) << 1),
                        (unsigned)m_ins_dat[dct].size());
                }
            }
            else*/
            {
                glDrawElementsInstanced(GL_TRIANGLES, getIndexCount(),
                    GL_UNSIGNED_SHORT, 0, (unsigned)m_ins_dat[dct].size());
            }
        }
        else
        {
            glDrawElementsInstanced(GL_TRIANGLES,
                std::get<1>(m_stk_material[material_id]),
                GL_UNSIGNED_SHORT,
                (void*)(std::get<0>(m_stk_material[material_id]) << 1),
                (unsigned)m_ins_dat[dct].size());
        }
    }
    // ------------------------------------------------------------------------
    void initDrawMaterial();
    // ------------------------------------------------------------------------
    void enableSkinningData()                             { m_skinned = true; }
    // ------------------------------------------------------------------------
    void uploadGLMesh();
    // ------------------------------------------------------------------------
    Material* getSTKMaterial(unsigned first_index = 0) const
    {
        for (unsigned i = 0; i < m_stk_material.size(); i++)
        {
            if (i == unsigned(m_stk_material.size() - 1) ||
                (first_index >= std::get<0>(m_stk_material[i]) &&
                first_index < std::get<0>(m_stk_material[i + 1])))
            {
                return std::get<2>(m_stk_material[i]);
            }
        }
        assert(false);
        return NULL;
    }
    // ------------------------------------------------------------------------
    void enableTextureMatrix(unsigned mat_id);
    // ------------------------------------------------------------------------
    std::array<std::shared_ptr<SPTexture>, 6>
        getSPTextures(unsigned first_index = 0) const
    {
        assert(m_stk_material.size() == m_textures.size());
        for (unsigned i = 0; i < m_stk_material.size(); i++)
        {
            if (i == unsigned(m_stk_material.size() - 1) ||
                (first_index >= std::get<0>(m_stk_material[i]) &&
                first_index < std::get<0>(m_stk_material[i + 1])))
            {
                return m_textures[i];
            }
        }
        assert(false);
        return std::array<std::shared_ptr<SPTexture>, 6> ();
    }
    // ------------------------------------------------------------------------
    std::array<std::shared_ptr<SPTexture>, 6>
        getSPTexturesByMaterialID(int material_id) const
    {
        assert((size_t)material_id < m_textures.size());
        return m_textures[material_id];
    }
    // ------------------------------------------------------------------------
    std::vector<Material*> getAllSTKMaterials() const
    {
        std::vector<Material*> ret;
        for (unsigned i = 0; i < m_stk_material.size(); i++)
        {
            ret.push_back(std::get<2>(m_stk_material[i]));
        }
        return ret;
    }
    // ------------------------------------------------------------------------
    const std::unordered_map<std::string, unsigned>& getTextureCompare() const
                                                          { return m_tex_cmp; }
    // ------------------------------------------------------------------------
    int getMaterialID(const std::string& tex_cmp) const
    {
        auto itr = m_tex_cmp.find(tex_cmp);
        if (itr != m_tex_cmp.end())
        {
            return (int)itr->second;
        }
        return -1;
    }
    // ------------------------------------------------------------------------
    void addInstanceData(const SPInstancedData& id, DrawCallType dct)
    {
        if (m_uploaded_instance)
        {
            for (unsigned i = 0; i < DCT_FOR_VAO; i++)
            {
                m_ins_dat[i].clear();
                m_uploaded_instance = false;
            }
        }
        m_ins_dat[dct].push_back(id);
    }
    // ------------------------------------------------------------------------
    void uploadInstanceData();
    // ------------------------------------------------------------------------
    void recreateVAO(unsigned i);
    // ------------------------------------------------------------------------
    video::S3DVertexSkinnedMesh* getSPMVertex()
    {
        return m_vertices.data();
    }
    // ------------------------------------------------------------------------
    void addSPMVertex(const video::S3DVertexSkinnedMesh& v)
    {
        m_vertices.push_back(v);
    }
    // ------------------------------------------------------------------------
    void setIndices(std::vector<uint16_t>& indices)
    {
        m_indices = std::move(indices);
    }
    // ------------------------------------------------------------------------
    void setSTKMaterial(Material* m)
    {
        m_stk_material[0] = std::make_tuple(0u, getIndexCount(), m);
    }
    // ------------------------------------------------------------------------
    virtual const video::SMaterial& getMaterial() const
    {
        static video::SMaterial unused;
        return unused;
    }
    // ------------------------------------------------------------------------
    virtual video::SMaterial& getMaterial()
    {
        static video::SMaterial unused;
        return unused;
    }
    // ------------------------------------------------------------------------
    virtual const void* getVertices() const
    {
        return m_vertices.data();
    }
    // ------------------------------------------------------------------------
    virtual void* getVertices()
    {
        return m_vertices.data();
    }
    // ------------------------------------------------------------------------
    virtual u32 getVertexCount() const
    {
        return (unsigned)m_vertices.size();
    }
    // ------------------------------------------------------------------------
    virtual video::E_INDEX_TYPE getIndexType() const
    {
        return video::EIT_16BIT;
    }
    // ------------------------------------------------------------------------
    virtual const u16* getIndices() const
    {
        return m_indices.data();
    }
    // ------------------------------------------------------------------------
    virtual u16* getIndices()
    {
        return m_indices.data();
    }
    // ------------------------------------------------------------------------
    virtual u32 getIndexCount() const
    {
        return (unsigned)m_indices.size();
    }
    // ------------------------------------------------------------------------
    virtual const core::aabbox3d<f32>& getBoundingBox() const
    {
        return m_bounding_box;
    }
    // ------------------------------------------------------------------------
    virtual void setBoundingBox(const core::aabbox3df& box)
    {
        m_bounding_box = box;
    }
    // ------------------------------------------------------------------------
    virtual void recalculateBoundingBox()
    {
        if (m_vertices.empty())
        {
            m_bounding_box.reset(0.0f, 0.0f, 0.0f);
        }
        else
        {
            m_bounding_box.reset(m_vertices[0].m_position);
            for (u32 i = 1; i < m_vertices.size(); i++)
            {
                m_bounding_box.addInternalPoint(m_vertices[i].m_position);
            }
        }
    }
    // ------------------------------------------------------------------------
    virtual video::E_VERTEX_TYPE getVertexType() const
    {
        return video::EVT_SKINNED_MESH;
    }
    // ------------------------------------------------------------------------
    virtual const core::vector3df& getPosition(u32 i) const
    {
        return m_vertices[i].m_position;
    }
    // ------------------------------------------------------------------------
    virtual core::vector3df& getPosition(u32 i)
    {
        return m_vertices[i].m_position;
    }
    // ------------------------------------------------------------------------
    virtual const core::vector3df& getNormal(u32 i) const
    {
        static core::vector3df unused;
        return unused;
    }
    // ------------------------------------------------------------------------
    virtual core::vector3df& getNormal(u32 i)
    {
        static core::vector3df unused;
        return unused;
    }
    // ------------------------------------------------------------------------
    virtual const core::vector2df& getTCoords(u32 i) const
    {
        static core::vector2df unused;
        return unused;
    }
    // ------------------------------------------------------------------------
    virtual core::vector2df& getTCoords(u32 i)
    {
        static core::vector2df unused;
        return unused;
    }
    // ------------------------------------------------------------------------
    virtual scene::E_PRIMITIVE_TYPE getPrimitiveType() const
    {
        return EPT_TRIANGLES;
    }
    // ------------------------------------------------------------------------
    virtual void append(const void* const vertices, u32 numm_vertices,
                        const u16* const indices, u32 numm_indices) {}
    // ------------------------------------------------------------------------
    virtual void append(const IMeshBuffer* const other) {}
    // ------------------------------------------------------------------------
    virtual E_HARDWARE_MAPPING getHardwareMappingHint_Vertex() const
    {
        return EHM_NEVER;
    }
    // ------------------------------------------------------------------------
    virtual E_HARDWARE_MAPPING getHardwareMappingHint_Index() const
    {
        return EHM_NEVER;
    }
    // ------------------------------------------------------------------------
    virtual void setHardwareMappingHint(E_HARDWARE_MAPPING,
                                        E_BUFFER_TYPE Buffer) {}
    // ------------------------------------------------------------------------
    virtual void setDirty(E_BUFFER_TYPE Buffer=EBT_VERTEX_AND_INDEX) {}
    // ------------------------------------------------------------------------
    virtual u32 getChangedID_Vertex() const { return 0; }
    // ------------------------------------------------------------------------
    virtual u32 getChangedID_Index() const { return 0; }

};

}

#endif
