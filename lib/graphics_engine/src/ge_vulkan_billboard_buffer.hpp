#ifndef HEADER_GE_VULKAN_BILLBOARD_BUFFER_HPP
#define HEADER_GE_VULKAN_BILLBOARD_BUFFER_HPP

#include "ge_spm_buffer.hpp"

namespace GE
{
class GEVulkanBillboardBuffer : public GESPMBuffer
{
private:
    GESPMBuffer* m_billboard_buffer;
public:
    // ------------------------------------------------------------------------
    GEVulkanBillboardBuffer(irr::video::SMaterial& billboard_material);
    // ------------------------------------------------------------------------
    virtual irr::u32 getIndexCount() const
                                { return m_billboard_buffer->getIndexCount(); }
    // ------------------------------------------------------------------------
    virtual size_t getVBOOffset() const
                                 { return m_billboard_buffer->getVBOOffset(); }
    // ------------------------------------------------------------------------
    virtual size_t getIBOOffset() const
                                 { return m_billboard_buffer->getIBOOffset(); }
    // ------------------------------------------------------------------------
    virtual VkBuffer getVkBuffer() const
                                  { return m_billboard_buffer->getVkBuffer(); }
};

} // end namespace GE

#endif
