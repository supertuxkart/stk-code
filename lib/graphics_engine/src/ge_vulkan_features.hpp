#ifndef HEADER_GE_VULKAN_FEATURES_HPP
#define HEADER_GE_VULKAN_FEATURES_HPP

#include "vulkan_wrapper.h"

namespace GE
{
class GEVulkanDriver;
namespace GEVulkanFeatures
{
// ----------------------------------------------------------------------------
void init(GEVulkanDriver*);
// ----------------------------------------------------------------------------
void printStats();
// ----------------------------------------------------------------------------
bool supportsBindTexturesAtOnce();
// ----------------------------------------------------------------------------
bool supportsDescriptorIndexing();
// ----------------------------------------------------------------------------
bool supportsNonUniformIndexing();
// ----------------------------------------------------------------------------
bool supportsDifferentTexturePerDraw();
// ----------------------------------------------------------------------------
bool supportsPartiallyBound();
};   // GEVulkanFeatures

}

#endif
