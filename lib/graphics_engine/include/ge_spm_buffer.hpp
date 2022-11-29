#ifndef HEADER_GE_SPM_BUFFER_HPP
#define HEADER_GE_SPM_BUFFER_HPP

#include <array>
#include <cstddef>
#include <vector>
#include "IMeshBuffer.h"

#include "ge_vma.hpp"
#include "vulkan_wrapper.h"

namespace GE
{
class GESPMBuffer : public irr::scene::IMeshBuffer
{
protected:
    irr::video::SMaterial m_material;

    std::vector<irr::video::S3DVertexSkinnedMesh> m_vertices;

    std::vector<irr::u16> m_indices;
private:

    irr::core::aabbox3d<irr::f32> m_bounding_box;

    size_t m_vbo_offset;

    size_t m_ibo_offset;

    size_t m_skinning_vbo_offset;

    VkBuffer m_buffer;

    VmaAllocation m_memory;

    bool m_has_skinning;
public:
    // ------------------------------------------------------------------------
    GESPMBuffer()
    {
        m_vbo_offset = 0;
        m_ibo_offset = 0;
        m_skinning_vbo_offset = 0;
        m_buffer = VK_NULL_HANDLE;
        m_memory = VK_NULL_HANDLE;
        m_has_skinning = false;
    }
    // ------------------------------------------------------------------------
    ~GESPMBuffer()                              { destroyVertexIndexBuffer(); }
    // ------------------------------------------------------------------------
    virtual const irr::video::SMaterial& getMaterial() const
                                                         { return m_material; }
    // ------------------------------------------------------------------------
    virtual irr::video::SMaterial& getMaterial()         { return m_material; }
    // ------------------------------------------------------------------------
    virtual const void* getVertices() const       { return m_vertices.data(); }
    // ------------------------------------------------------------------------
    virtual void* getVertices()                   { return m_vertices.data(); }
    // ------------------------------------------------------------------------
    virtual irr::u32 getVertexCount() const       { return m_vertices.size(); }
    // ------------------------------------------------------------------------
    virtual irr::video::E_INDEX_TYPE getIndexType() const
                                              { return irr::video::EIT_16BIT; }
    // ------------------------------------------------------------------------
    virtual const irr::u16* getIndices() const     { return m_indices.data(); }
    // ------------------------------------------------------------------------
    virtual irr::u16* getIndices()                 { return m_indices.data(); }
    // ------------------------------------------------------------------------
    virtual irr::u32 getIndexCount() const         { return m_indices.size(); }
    // ------------------------------------------------------------------------
    virtual const irr::core::aabbox3d<irr::f32>& getBoundingBox() const
                                                     { return m_bounding_box; }
    // ------------------------------------------------------------------------
    virtual void setBoundingBox(const irr::core::aabbox3df& box)
                                                      { m_bounding_box = box; }
    // ------------------------------------------------------------------------
    virtual void recalculateBoundingBox()
    {
        if (m_vertices.empty())
            m_bounding_box.reset(0, 0, 0);
        else
        {
            m_bounding_box.reset(m_vertices[0].m_position);
            for (irr::u32 i = 1; i < m_vertices.size(); i++)
                m_bounding_box.addInternalPoint(m_vertices[i].m_position);
        }
    }
    // ------------------------------------------------------------------------
    virtual irr::video::E_VERTEX_TYPE getVertexType() const
                                       { return irr::video::EVT_SKINNED_MESH; }
    // ------------------------------------------------------------------------
    virtual const irr::core::vector3df& getPosition(irr::u32 i) const
                                           { return m_vertices[i].m_position; }
    // ------------------------------------------------------------------------
    virtual irr::core::vector3df& getPosition(irr::u32 i)
                                           { return m_vertices[i].m_position; }
    // ------------------------------------------------------------------------
    virtual const irr::core::vector3df& getNormal(irr::u32 i) const
    {
        static irr::core::vector3df unused;
        return unused;
    }
    // ------------------------------------------------------------------------
    virtual irr::core::vector3df& getNormal(irr::u32 i)
    {
        static irr::core::vector3df unused;
        return unused;
    }
    // ------------------------------------------------------------------------
    virtual void setNormal(irr::u32 i, const irr::core::vector3df& normal);
    // ------------------------------------------------------------------------
    virtual const irr::core::vector2df& getTCoords(irr::u32 i) const
    {
        static irr::core::vector2df unused;
        return unused;
    }
    // ------------------------------------------------------------------------
    virtual irr::core::vector2df& getTCoords(irr::u32 i)
    {
        static irr::core::vector2df unused;
        return unused;
    }
    // ------------------------------------------------------------------------
    virtual void setTCoords(irr::u32 i, const irr::core::vector2df& tcoords);
    // ------------------------------------------------------------------------
    virtual irr::scene::E_PRIMITIVE_TYPE getPrimitiveType() const
                                          { return irr::scene::EPT_TRIANGLES; }
    // ------------------------------------------------------------------------
    virtual void append(const void* const vertices, irr::u32 num_vertices,
                        const irr::u16* const indices, irr::u32 num_indices)
    {
        if (vertices == getVertices())
            return;

        irr::u32 vertex_count = getVertexCount();
        m_vertices.reserve(vertex_count + num_vertices);
        for (irr::u32 i = 0; i < num_vertices; i++)
        {
            m_vertices.push_back(reinterpret_cast<
                const irr::video::S3DVertexSkinnedMesh*>(vertices)[i]);
            m_bounding_box.addInternalPoint(reinterpret_cast<
               const irr::video::S3DVertexSkinnedMesh*>(vertices)[i].m_position);
        }

        m_indices.reserve(getIndexCount() + num_indices);
        for (irr::u32 i = 0; i < num_indices; i++)
            m_indices.push_back(indices[i] + vertex_count);
    }
    // ------------------------------------------------------------------------
    virtual void append(const IMeshBuffer* const other) {}
    // ------------------------------------------------------------------------
    virtual irr::scene::E_HARDWARE_MAPPING getHardwareMappingHint_Vertex() const
                                              { return irr::scene::EHM_NEVER; }
    // ------------------------------------------------------------------------
    virtual irr::scene::E_HARDWARE_MAPPING getHardwareMappingHint_Index() const
                                              { return irr::scene::EHM_NEVER; }
    // ------------------------------------------------------------------------
    virtual void setHardwareMappingHint(irr::scene::E_HARDWARE_MAPPING NewMappingHint,
                                        irr::scene::E_BUFFER_TYPE Buffer = irr::scene::EBT_VERTEX_AND_INDEX)
                                                                             {}
    // ------------------------------------------------------------------------
    virtual void setDirty(irr::scene::E_BUFFER_TYPE Buffer = irr::scene::EBT_VERTEX_AND_INDEX)
                                                                             {}
    // ------------------------------------------------------------------------
    virtual irr::u32 getChangedID_Vertex() const                  { return 0; }
    // ------------------------------------------------------------------------
    virtual irr::u32 getChangedID_Index() const                   { return 0; }
    // ------------------------------------------------------------------------
    void setVBOOffset(size_t offset)                 { m_vbo_offset = offset; }
    // ------------------------------------------------------------------------
    virtual size_t getVBOOffset() const                { return m_vbo_offset; }
    // ------------------------------------------------------------------------
    void setIBOOffset(size_t offset)                 { m_ibo_offset = offset; }
    // ------------------------------------------------------------------------
    virtual size_t getIBOOffset() const                { return m_ibo_offset; }
    // ------------------------------------------------------------------------
    bool hasSkinning() const                         { return m_has_skinning; }
    // ------------------------------------------------------------------------
    void setHasSkinning(bool val)                     { m_has_skinning = val; }
    // ------------------------------------------------------------------------
    virtual void bindVertexIndexBuffer(VkCommandBuffer cmd)
    {
        VkBuffer buffer = getVkBuffer();
        std::array<VkBuffer, 2> vertex_buffer =
        {{
            buffer,
            buffer
        }};
        std::array<VkDeviceSize, 2> offsets =
        {{
            0,
            m_skinning_vbo_offset
        }};
        vkCmdBindVertexBuffers(cmd, 0, vertex_buffer.size(),
            vertex_buffer.data(), offsets.data());
        vkCmdBindIndexBuffer(cmd, buffer, getIBOOffset(),
            VK_INDEX_TYPE_UINT16);
    }
    // ------------------------------------------------------------------------
    virtual void createVertexIndexBuffer();
    // ------------------------------------------------------------------------
    virtual void destroyVertexIndexBuffer();
    // ------------------------------------------------------------------------
    std::vector<irr::video::S3DVertexSkinnedMesh>& getVerticesVector()
                                                         { return m_vertices; }
    // ------------------------------------------------------------------------
    std::vector<irr::u16>& getIndicesVector()             { return m_indices; }
    // ------------------------------------------------------------------------
    virtual VkBuffer getVkBuffer() const                   { return m_buffer; }
};

} // end namespace GE

#endif


