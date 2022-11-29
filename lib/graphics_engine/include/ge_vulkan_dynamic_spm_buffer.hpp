#ifndef HEADER_GE_VULKAN_DYNAMIC_SPM_BUFFER_HPP
#define HEADER_GE_VULKAN_DYNAMIC_SPM_BUFFER_HPP

#include "ge_spm_buffer.hpp"

namespace GE
{
class GEVulkanDynamicBuffer;

class GEVulkanDynamicSPMBuffer : public GESPMBuffer
{
private:
    GEVulkanDynamicBuffer* m_vertex_buffer;

    GEVulkanDynamicBuffer* m_index_buffer;
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
};

} // end namespace GE

#endif
