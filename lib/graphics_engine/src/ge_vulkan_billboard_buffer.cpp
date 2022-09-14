#include "ge_vulkan_billboard_buffer.hpp"

#include "ge_main.hpp"
#include "ge_spm.hpp"
#include "ge_vulkan_driver.hpp"

namespace GE
{
GEVulkanBillboardBuffer::GEVulkanBillboardBuffer(
                                     irr::video::SMaterial& billboard_material)
{
    m_billboard_buffer = static_cast<GESPMBuffer*>
        (getVKDriver()->getBillboardQuad()->getMeshBuffer(0));
    m_material = billboard_material;
}   // GEVulkanBillboardBuffer

}
