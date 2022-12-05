#ifndef HEADER_GE_VULKAN_DYNAMIC_SPM_BUFFER_HPP
#define HEADER_GE_VULKAN_DYNAMIC_SPM_BUFFER_HPP

#include "ge_spm_buffer.hpp"

namespace GE
{
class GEVulkanDriver;
class GEVulkanDynamicBuffer;

class GEVulkanDynamicSPMBuffer : public GESPMBuffer
{
private:
    GEVulkanDynamicBuffer* m_vertex_buffer;

    GEVulkanDynamicBuffer* m_index_buffer;

    GEVulkanDriver* m_vk;

    uint32_t* m_vertex_update_offsets;

    uint32_t* m_index_update_offsets;
public:
    // ------------------------------------------------------------------------
    GEVulkanDynamicSPMBuffer();
    // ------------------------------------------------------------------------
    ~GEVulkanDynamicSPMBuffer();
    // ------------------------------------------------------------------------
    virtual irr::scene::E_HARDWARE_MAPPING getHardwareMappingHint_Vertex() const
                                             { return irr::scene::EHM_STREAM; }
    // ------------------------------------------------------------------------
    virtual irr::scene::E_HARDWARE_MAPPING getHardwareMappingHint_Index() const
                                             { return irr::scene::EHM_STREAM; }
    // ------------------------------------------------------------------------
    virtual void bindVertexIndexBuffer(VkCommandBuffer cmd)                  {}
    // ------------------------------------------------------------------------
    virtual void createVertexIndexBuffer()                                   {}
    // ------------------------------------------------------------------------
    virtual void destroyVertexIndexBuffer()                                  {}
    // ------------------------------------------------------------------------
    void updateVertexIndexBuffer(int buffer_index);
    // ------------------------------------------------------------------------
    void drawDynamicVertexIndexBuffer(VkCommandBuffer cmd, int buffer_index);
    // ------------------------------------------------------------------------
    void setDirtyOffset(irr::u32 offset,
          irr::scene::E_BUFFER_TYPE buffer = irr::scene::EBT_VERTEX_AND_INDEX);
};

} // end namespace GE

#endif
