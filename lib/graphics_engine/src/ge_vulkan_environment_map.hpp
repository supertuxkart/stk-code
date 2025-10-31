#ifndef HEADER_GE_VULKAN_ENVIRONMENT_MAP_HPP
#define HEADER_GE_VULKAN_ENVIRONMENT_MAP_HPP

#include "dimension2d.h"

namespace GE
{
class GEVulkanSkyBoxRenderer;

class GEVulkanEnvironmentMap
{
private:
    GEVulkanSkyBoxRenderer* m_skybox;
public:
    // ------------------------------------------------------------------------
    GEVulkanEnvironmentMap(GEVulkanSkyBoxRenderer* skybox);
    // ------------------------------------------------------------------------
    ~GEVulkanEnvironmentMap();
    // ------------------------------------------------------------------------
    /** A much lower resolution than the input cubemap is sufficient because
     *  diffuse reflections are inherently blurry.
     */
    const static irr::core::dimension2du getDiffuseEnvironmentMapSize()
                                    { return irr::core::dimension2du(32, 32); }
    // ------------------------------------------------------------------------
    const static irr::core::dimension2du getSpecularEnvironmentMapSize()
                                  { return irr::core::dimension2du(256, 256); }
    // ------------------------------------------------------------------------
    const static unsigned getDiffuseEnvironmentMapSampleCount()
                                                                { return 256; }
    // ------------------------------------------------------------------------
    void load();
};   // GEVulkanEnvironmentMap

}

#endif
