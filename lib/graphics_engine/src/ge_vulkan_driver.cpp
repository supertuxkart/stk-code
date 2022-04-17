#include "ge_vulkan_driver.hpp"

#include "ge_main.hpp"

#include "ge_vulkan_2d_renderer.hpp"
#include "ge_vulkan_features.hpp"
#include "ge_vulkan_shader_manager.hpp"
#include "ge_vulkan_texture.hpp"
#include "ge_vulkan_command_loader.hpp"

#ifdef _IRR_COMPILE_WITH_VULKAN_
#include "SDL_vulkan.h"
#include <algorithm>
#include <atomic>
#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>
#include "../source/Irrlicht/os.h"

#if !defined(__APPLE__) || defined(DLOPEN_MOLTENVK)
struct GE_VK_UserPointer
{
    VkInstance instance;
    VkDevice device;
    PFN_vkGetDeviceProcAddr get_device_proc_addr;
    PFN_vkGetInstanceProcAddr get_instance_proc_addr;
};

extern "C" PFN_vkVoidFunction loader(void* user_ptr, const char* name)
{
    VkInstance instance = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    PFN_vkGetDeviceProcAddr get_device_proc_addr = NULL;
    PFN_vkGetInstanceProcAddr get_instance_proc_addr = NULL;

    if (user_ptr)
    {
        instance = ((GE_VK_UserPointer*)user_ptr)->instance;
        device = ((GE_VK_UserPointer*)user_ptr)->device;
        get_device_proc_addr =
            ((GE_VK_UserPointer*)user_ptr)->get_device_proc_addr;
        get_instance_proc_addr =
            ((GE_VK_UserPointer*)user_ptr)->get_instance_proc_addr;
    }

    if (get_instance_proc_addr == NULL)
    {
        get_instance_proc_addr =
            (PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr();
        if (user_ptr)
        {
            ((GE_VK_UserPointer*)user_ptr)->get_instance_proc_addr =
                get_instance_proc_addr;
        }
    }

    // From vulkan.c glad2 with loader enabled
    static std::set<std::string> device_function =
    {{
        "vkAcquireFullScreenExclusiveModeEXT",
        "vkAcquireNextImage2KHR",
        "vkAcquireNextImageKHR",
        "vkAcquirePerformanceConfigurationINTEL",
        "vkAcquireProfilingLockKHR",
        "vkAllocateCommandBuffers",
        "vkAllocateDescriptorSets",
        "vkAllocateMemory",
        "vkBeginCommandBuffer",
        "vkBindAccelerationStructureMemoryKHR",
        "vkBindAccelerationStructureMemoryNV",
        "vkBindBufferMemory",
        "vkBindBufferMemory2",
        "vkBindBufferMemory2KHR",
        "vkBindImageMemory",
        "vkBindImageMemory2",
        "vkBindImageMemory2KHR",
        "vkBuildAccelerationStructureKHR",
        "vkCmdBeginConditionalRenderingEXT",
        "vkCmdBeginDebugUtilsLabelEXT",
        "vkCmdBeginQuery",
        "vkCmdBeginQueryIndexedEXT",
        "vkCmdBeginRenderPass",
        "vkCmdBeginRenderPass2",
        "vkCmdBeginRenderPass2KHR",
        "vkCmdBeginTransformFeedbackEXT",
        "vkCmdBindDescriptorSets",
        "vkCmdBindIndexBuffer",
        "vkCmdBindPipeline",
        "vkCmdBindPipelineShaderGroupNV",
        "vkCmdBindShadingRateImageNV",
        "vkCmdBindTransformFeedbackBuffersEXT",
        "vkCmdBindVertexBuffers",
        "vkCmdBindVertexBuffers2EXT",
        "vkCmdBlitImage",
        "vkCmdBuildAccelerationStructureIndirectKHR",
        "vkCmdBuildAccelerationStructureKHR",
        "vkCmdBuildAccelerationStructureNV",
        "vkCmdClearAttachments",
        "vkCmdClearColorImage",
        "vkCmdClearDepthStencilImage",
        "vkCmdCopyAccelerationStructureKHR",
        "vkCmdCopyAccelerationStructureNV",
        "vkCmdCopyAccelerationStructureToMemoryKHR",
        "vkCmdCopyBuffer",
        "vkCmdCopyBufferToImage",
        "vkCmdCopyImage",
        "vkCmdCopyImageToBuffer",
        "vkCmdCopyMemoryToAccelerationStructureKHR",
        "vkCmdCopyQueryPoolResults",
        "vkCmdDebugMarkerBeginEXT",
        "vkCmdDebugMarkerEndEXT",
        "vkCmdDebugMarkerInsertEXT",
        "vkCmdDispatch",
        "vkCmdDispatchBase",
        "vkCmdDispatchBaseKHR",
        "vkCmdDispatchIndirect",
        "vkCmdDraw",
        "vkCmdDrawIndexed",
        "vkCmdDrawIndexedIndirect",
        "vkCmdDrawIndexedIndirectCount",
        "vkCmdDrawIndexedIndirectCountAMD",
        "vkCmdDrawIndexedIndirectCountKHR",
        "vkCmdDrawIndirect",
        "vkCmdDrawIndirectByteCountEXT",
        "vkCmdDrawIndirectCount",
        "vkCmdDrawIndirectCountAMD",
        "vkCmdDrawIndirectCountKHR",
        "vkCmdDrawMeshTasksIndirectCountNV",
        "vkCmdDrawMeshTasksIndirectNV",
        "vkCmdDrawMeshTasksNV",
        "vkCmdEndConditionalRenderingEXT",
        "vkCmdEndDebugUtilsLabelEXT",
        "vkCmdEndQuery",
        "vkCmdEndQueryIndexedEXT",
        "vkCmdEndRenderPass",
        "vkCmdEndRenderPass2",
        "vkCmdEndRenderPass2KHR",
        "vkCmdEndTransformFeedbackEXT",
        "vkCmdExecuteCommands",
        "vkCmdExecuteGeneratedCommandsNV",
        "vkCmdFillBuffer",
        "vkCmdInsertDebugUtilsLabelEXT",
        "vkCmdNextSubpass",
        "vkCmdNextSubpass2",
        "vkCmdNextSubpass2KHR",
        "vkCmdPipelineBarrier",
        "vkCmdPreprocessGeneratedCommandsNV",
        "vkCmdPushConstants",
        "vkCmdPushDescriptorSetKHR",
        "vkCmdPushDescriptorSetWithTemplateKHR",
        "vkCmdResetEvent",
        "vkCmdResetQueryPool",
        "vkCmdResolveImage",
        "vkCmdSetBlendConstants",
        "vkCmdSetCheckpointNV",
        "vkCmdSetCoarseSampleOrderNV",
        "vkCmdSetCullModeEXT",
        "vkCmdSetDepthBias",
        "vkCmdSetDepthBounds",
        "vkCmdSetDepthBoundsTestEnableEXT",
        "vkCmdSetDepthCompareOpEXT",
        "vkCmdSetDepthTestEnableEXT",
        "vkCmdSetDepthWriteEnableEXT",
        "vkCmdSetDeviceMask",
        "vkCmdSetDeviceMaskKHR",
        "vkCmdSetDiscardRectangleEXT",
        "vkCmdSetEvent",
        "vkCmdSetExclusiveScissorNV",
        "vkCmdSetFrontFaceEXT",
        "vkCmdSetLineStippleEXT",
        "vkCmdSetLineWidth",
        "vkCmdSetPerformanceMarkerINTEL",
        "vkCmdSetPerformanceOverrideINTEL",
        "vkCmdSetPerformanceStreamMarkerINTEL",
        "vkCmdSetPrimitiveTopologyEXT",
        "vkCmdSetSampleLocationsEXT",
        "vkCmdSetScissor",
        "vkCmdSetScissorWithCountEXT",
        "vkCmdSetStencilCompareMask",
        "vkCmdSetStencilOpEXT",
        "vkCmdSetStencilReference",
        "vkCmdSetStencilTestEnableEXT",
        "vkCmdSetStencilWriteMask",
        "vkCmdSetViewport",
        "vkCmdSetViewportShadingRatePaletteNV",
        "vkCmdSetViewportWScalingNV",
        "vkCmdSetViewportWithCountEXT",
        "vkCmdTraceRaysIndirectKHR",
        "vkCmdTraceRaysKHR",
        "vkCmdTraceRaysNV",
        "vkCmdUpdateBuffer",
        "vkCmdWaitEvents",
        "vkCmdWriteAccelerationStructuresPropertiesKHR",
        "vkCmdWriteAccelerationStructuresPropertiesNV",
        "vkCmdWriteBufferMarkerAMD",
        "vkCmdWriteTimestamp",
        "vkCompileDeferredNV",
        "vkCopyAccelerationStructureKHR",
        "vkCopyAccelerationStructureToMemoryKHR",
        "vkCopyMemoryToAccelerationStructureKHR",
        "vkCreateAccelerationStructureKHR",
        "vkCreateAccelerationStructureNV",
        "vkCreateBuffer",
        "vkCreateBufferView",
        "vkCreateCommandPool",
        "vkCreateComputePipelines",
        "vkCreateDeferredOperationKHR",
        "vkCreateDescriptorPool",
        "vkCreateDescriptorSetLayout",
        "vkCreateDescriptorUpdateTemplate",
        "vkCreateDescriptorUpdateTemplateKHR",
        "vkCreateEvent",
        "vkCreateFence",
        "vkCreateFramebuffer",
        "vkCreateGraphicsPipelines",
        "vkCreateImage",
        "vkCreateImageView",
        "vkCreateIndirectCommandsLayoutNV",
        "vkCreatePipelineCache",
        "vkCreatePipelineLayout",
        "vkCreatePrivateDataSlotEXT",
        "vkCreateQueryPool",
        "vkCreateRayTracingPipelinesKHR",
        "vkCreateRayTracingPipelinesNV",
        "vkCreateRenderPass",
        "vkCreateRenderPass2",
        "vkCreateRenderPass2KHR",
        "vkCreateSampler",
        "vkCreateSamplerYcbcrConversion",
        "vkCreateSamplerYcbcrConversionKHR",
        "vkCreateSemaphore",
        "vkCreateShaderModule",
        "vkCreateSharedSwapchainsKHR",
        "vkCreateSwapchainKHR",
        "vkCreateValidationCacheEXT",
        "vkDebugMarkerSetObjectNameEXT",
        "vkDebugMarkerSetObjectTagEXT",
        "vkDeferredOperationJoinKHR",
        "vkDestroyAccelerationStructureKHR",
        "vkDestroyAccelerationStructureNV",
        "vkDestroyBuffer",
        "vkDestroyBufferView",
        "vkDestroyCommandPool",
        "vkDestroyDeferredOperationKHR",
        "vkDestroyDescriptorPool",
        "vkDestroyDescriptorSetLayout",
        "vkDestroyDescriptorUpdateTemplate",
        "vkDestroyDescriptorUpdateTemplateKHR",
        "vkDestroyDevice",
        "vkDestroyEvent",
        "vkDestroyFence",
        "vkDestroyFramebuffer",
        "vkDestroyImage",
        "vkDestroyImageView",
        "vkDestroyIndirectCommandsLayoutNV",
        "vkDestroyPipeline",
        "vkDestroyPipelineCache",
        "vkDestroyPipelineLayout",
        "vkDestroyPrivateDataSlotEXT",
        "vkDestroyQueryPool",
        "vkDestroyRenderPass",
        "vkDestroySampler",
        "vkDestroySamplerYcbcrConversion",
        "vkDestroySamplerYcbcrConversionKHR",
        "vkDestroySemaphore",
        "vkDestroyShaderModule",
        "vkDestroySwapchainKHR",
        "vkDestroyValidationCacheEXT",
        "vkDeviceWaitIdle",
        "vkDisplayPowerControlEXT",
        "vkEndCommandBuffer",
        "vkFlushMappedMemoryRanges",
        "vkFreeCommandBuffers",
        "vkFreeDescriptorSets",
        "vkFreeMemory",
        "vkGetAccelerationStructureDeviceAddressKHR",
        "vkGetAccelerationStructureHandleNV",
        "vkGetAccelerationStructureMemoryRequirementsKHR",
        "vkGetAccelerationStructureMemoryRequirementsNV",
        "vkGetAndroidHardwareBufferPropertiesANDROID",
        "vkGetBufferDeviceAddress",
        "vkGetBufferDeviceAddressEXT",
        "vkGetBufferDeviceAddressKHR",
        "vkGetBufferMemoryRequirements",
        "vkGetBufferMemoryRequirements2",
        "vkGetBufferMemoryRequirements2KHR",
        "vkGetBufferOpaqueCaptureAddress",
        "vkGetBufferOpaqueCaptureAddressKHR",
        "vkGetCalibratedTimestampsEXT",
        "vkGetDeferredOperationMaxConcurrencyKHR",
        "vkGetDeferredOperationResultKHR",
        "vkGetDescriptorSetLayoutSupport",
        "vkGetDescriptorSetLayoutSupportKHR",
        "vkGetDeviceAccelerationStructureCompatibilityKHR",
        "vkGetDeviceGroupPeerMemoryFeatures",
        "vkGetDeviceGroupPeerMemoryFeaturesKHR",
        "vkGetDeviceGroupPresentCapabilitiesKHR",
        "vkGetDeviceGroupSurfacePresentModes2EXT",
        "vkGetDeviceGroupSurfacePresentModesKHR",
        "vkGetDeviceMemoryCommitment",
        "vkGetDeviceMemoryOpaqueCaptureAddress",
        "vkGetDeviceMemoryOpaqueCaptureAddressKHR",
        "vkGetDeviceProcAddr",
        "vkGetDeviceQueue",
        "vkGetDeviceQueue2",
        "vkGetEventStatus",
        "vkGetFenceFdKHR",
        "vkGetFenceStatus",
        "vkGetFenceWin32HandleKHR",
        "vkGetGeneratedCommandsMemoryRequirementsNV",
        "vkGetImageDrmFormatModifierPropertiesEXT",
        "vkGetImageMemoryRequirements",
        "vkGetImageMemoryRequirements2",
        "vkGetImageMemoryRequirements2KHR",
        "vkGetImageSparseMemoryRequirements",
        "vkGetImageSparseMemoryRequirements2",
        "vkGetImageSparseMemoryRequirements2KHR",
        "vkGetImageSubresourceLayout",
        "vkGetImageViewAddressNVX",
        "vkGetImageViewHandleNVX",
        "vkGetMemoryAndroidHardwareBufferANDROID",
        "vkGetMemoryFdKHR",
        "vkGetMemoryFdPropertiesKHR",
        "vkGetMemoryHostPointerPropertiesEXT",
        "vkGetMemoryWin32HandleKHR",
        "vkGetMemoryWin32HandleNV",
        "vkGetMemoryWin32HandlePropertiesKHR",
        "vkGetPastPresentationTimingGOOGLE",
        "vkGetPerformanceParameterINTEL",
        "vkGetPipelineCacheData",
        "vkGetPipelineExecutableInternalRepresentationsKHR",
        "vkGetPipelineExecutablePropertiesKHR",
        "vkGetPipelineExecutableStatisticsKHR",
        "vkGetPrivateDataEXT",
        "vkGetQueryPoolResults",
        "vkGetQueueCheckpointDataNV",
        "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR",
        "vkGetRayTracingShaderGroupHandlesKHR",
        "vkGetRayTracingShaderGroupHandlesNV",
        "vkGetRefreshCycleDurationGOOGLE",
        "vkGetRenderAreaGranularity",
        "vkGetSemaphoreCounterValue",
        "vkGetSemaphoreCounterValueKHR",
        "vkGetSemaphoreFdKHR",
        "vkGetSemaphoreWin32HandleKHR",
        "vkGetShaderInfoAMD",
        "vkGetSwapchainCounterEXT",
        "vkGetSwapchainImagesKHR",
        "vkGetSwapchainStatusKHR",
        "vkGetValidationCacheDataEXT",
        "vkImportFenceFdKHR",
        "vkImportFenceWin32HandleKHR",
        "vkImportSemaphoreFdKHR",
        "vkImportSemaphoreWin32HandleKHR",
        "vkInitializePerformanceApiINTEL",
        "vkInvalidateMappedMemoryRanges",
        "vkMapMemory",
        "vkMergePipelineCaches",
        "vkMergeValidationCachesEXT",
        "vkQueueBeginDebugUtilsLabelEXT",
        "vkQueueBindSparse",
        "vkQueueEndDebugUtilsLabelEXT",
        "vkQueueInsertDebugUtilsLabelEXT",
        "vkQueuePresentKHR",
        "vkQueueSetPerformanceConfigurationINTEL",
        "vkQueueSubmit",
        "vkQueueWaitIdle",
        "vkRegisterDeviceEventEXT",
        "vkRegisterDisplayEventEXT",
        "vkReleaseFullScreenExclusiveModeEXT",
        "vkReleasePerformanceConfigurationINTEL",
        "vkReleaseProfilingLockKHR",
        "vkResetCommandBuffer",
        "vkResetCommandPool",
        "vkResetDescriptorPool",
        "vkResetEvent",
        "vkResetFences",
        "vkResetQueryPool",
        "vkResetQueryPoolEXT",
        "vkSetDebugUtilsObjectNameEXT",
        "vkSetDebugUtilsObjectTagEXT",
        "vkSetEvent",
        "vkSetHdrMetadataEXT",
        "vkSetLocalDimmingAMD",
        "vkSetPrivateDataEXT",
        "vkSignalSemaphore",
        "vkSignalSemaphoreKHR",
        "vkTrimCommandPool",
        "vkTrimCommandPoolKHR",
        "vkUninitializePerformanceApiINTEL",
        "vkUnmapMemory",
        "vkUpdateDescriptorSetWithTemplate",
        "vkUpdateDescriptorSetWithTemplateKHR",
        "vkUpdateDescriptorSets",
        "vkWaitForFences",
        "vkWaitSemaphores",
        "vkWaitSemaphoresKHR",
        "vkWriteAccelerationStructuresPropertiesKHR",
    }};
    /* Exists as a workaround for:
     * https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers/issues/2323
     *
     * `vkGetDeviceProcAddr` does not return NULL for non-device functions.
     */
    if (!get_device_proc_addr && instance)
    {
        get_device_proc_addr = (PFN_vkGetDeviceProcAddr)
            get_instance_proc_addr(instance, "vkGetDeviceProcAddr");
        if (user_ptr)
        {
            ((GE_VK_UserPointer*)user_ptr)->get_device_proc_addr =
                get_device_proc_addr;
        }
    }
    if (device && get_device_proc_addr &&
        device_function.find(name) != device_function.end())
    {
        PFN_vkVoidFunction ret = get_device_proc_addr(device, name);
        if (ret)
            return ret;
    }

    // Workaround for android vulkan driver return NULL function pointer with
    // non-NULL instance
    // See https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkGetInstanceProcAddr.html
    // Also slient the warnings when loading with NULL instance in android
    if (strcmp(name, "vkGetInstanceProcAddr") == 0)
        return (PFN_vkVoidFunction)get_instance_proc_addr;
    static std::set<std::string> instance_function =
    {{
        "vkEnumerateInstanceVersion",
        "vkEnumerateInstanceExtensionProperties",
        "vkEnumerateInstanceLayerProperties",
        "vkCreateInstance",
    }};
    if (instance_function.find(name) != instance_function.end())
        return get_instance_proc_addr(NULL, name);
    else if (instance == VK_NULL_HANDLE &&
        instance_function.find(name) == instance_function.end())
        return NULL;
    return get_instance_proc_addr(instance, name);
}   // loader
#endif

namespace GE
{
std::atomic_bool g_device_created(false);
std::atomic_bool g_schedule_pausing_rendering(false);
std::atomic_bool g_paused_rendering(false);

GEVulkanDriver::GEVulkanDriver(const SIrrlichtCreationParameters& params,
                               io::IFileSystem* io, SDL_Window* window)
              : CNullDriver(io, core::dimension2d<u32>(0, 0)),
                m_params(params)
{
    m_vk.reset(new VK());
    m_physical_device = VK_NULL_HANDLE;
    m_present_queue = VK_NULL_HANDLE;
    m_graphics_family = m_present_family = 0;
    m_graphics_queue_count = 0;
    m_properties = {};
    m_features = {};

    m_current_frame = 0;
    m_image_index = 0;
    m_clear_color = video::SColor(0);
    m_white_texture = NULL;
    m_transparent_texture = NULL;
    m_pre_rotation_matrix = core::matrix4(core::matrix4::EM4CONST_IDENTITY);

    m_window = window;
    g_schedule_pausing_rendering.store(false);
    g_paused_rendering.store(false);
    g_device_created.store(true);

#if defined(__APPLE__)
    MVKConfiguration cfg = {};
    size_t cfg_size = sizeof(MVKConfiguration);
    vkGetMoltenVKConfigurationMVK(VK_NULL_HANDLE, &cfg, &cfg_size);
    // Enable to allow binding all textures at once
    cfg.useMetalArgumentBuffers = VK_TRUE;
    vkSetMoltenVKConfigurationMVK(VK_NULL_HANDLE, &cfg, &cfg_size);
#endif

    createInstance(window);

#if !defined(__APPLE__) || defined(DLOPEN_MOLTENVK)
    GE_VK_UserPointer user_ptr = {};
    user_ptr.instance = m_vk->instance;
    if (gladLoadVulkanUserPtr(NULL,
        (GLADuserptrloadfunc)loader, &user_ptr) == 0)
    {
        throw std::runtime_error("gladLoadVulkanUserPtr failed "
            "with non-NULL instance");
    }
#endif

    if (SDL_Vulkan_CreateSurface(window, m_vk->instance, &m_vk->surface) == SDL_FALSE)
        throw std::runtime_error("SDL_Vulkan_CreateSurface failed");
    int w, h = 0;
    SDL_Vulkan_GetDrawableSize(window, &w, &h);
    ScreenSize.Width = w;
    ScreenSize.Height = h;

    m_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    findPhysicalDevice();
    vkGetPhysicalDeviceProperties(m_physical_device, &m_properties);

#if defined(__APPLE__)
    // Debug usage only
    MVKPhysicalDeviceMetalFeatures mvk_features = {};
    size_t mvk_features_size = sizeof(MVKPhysicalDeviceMetalFeatures);
    vkGetPhysicalDeviceMetalFeaturesMVK(m_physical_device, &mvk_features, &mvk_features_size);
#endif

    GEVulkanFeatures::init(this);
    createDevice();

#if !defined(__APPLE__) || defined(DLOPEN_MOLTENVK)
    user_ptr.device = m_vk->device;
    if (gladLoadVulkanUserPtr(m_physical_device,
        (GLADuserptrloadfunc)loader, &user_ptr) == 0)
    {
        throw std::runtime_error("gladLoadVulkanUserPtr failed with "
            "non-NULL instance and non-NULL m_physical_device");
    }
#endif

    createSwapChain();
    createSyncObjects();
    createSamplers();
    createRenderPass();
    createFramebuffers();

    os::Printer::log("Vulkan version", getVulkanVersionString().c_str());
    os::Printer::log("Vulkan vendor", getVendorInfo().c_str());
    os::Printer::log("Vulkan renderer", m_properties.deviceName);
    os::Printer::log("Vulkan driver version", getDriverVersionString().c_str());
    for (const char* ext : m_device_extensions)
        os::Printer::log("Vulkan enabled extension", ext);

    try
    {
        GEVulkanCommandLoader::init(this);
        createCommandBuffers();

        GEVulkanShaderManager::init(this);
        // For GEVulkanDynamicBuffer
        GE::setVideoDriver(this);
        GEVulkan2dRenderer::init(this);
        createUnicolorTextures();
        GEVulkanFeatures::printStats();
    }
    catch (std::exception& e)
    {
        destroyVulkan();
        throw std::runtime_error("GEVulkanDriver constructor failed");
    }
}   // GEVulkanDriver

// ----------------------------------------------------------------------------
GEVulkanDriver::~GEVulkanDriver()
{
    g_device_created.store(false);
}   // ~GEVulkanDriver

// ----------------------------------------------------------------------------
void GEVulkanDriver::destroyVulkan()
{
    if (m_white_texture)
    {
        m_white_texture->drop();
        m_white_texture = NULL;
    }
    if (m_transparent_texture)
    {
        m_transparent_texture->drop();
        m_transparent_texture = NULL;
    }

    GEVulkan2dRenderer::destroy();
    GEVulkanShaderManager::destroy();

    if (!m_vk->command_buffers.empty())
    {
        vkFreeCommandBuffers(m_vk->device,
            GEVulkanCommandLoader::getCurrentCommandPool(),
            (uint32_t)(m_vk->command_buffers.size()),
            &m_vk->command_buffers[0]);
    }
    GEVulkanCommandLoader::destroy();
    for (std::mutex* m : m_graphics_queue_mutexes)
        delete m;
    m_graphics_queue_mutexes.clear();

    delete m_vk.get();
    m_vk.release();
}   // destroyVulkan

// ----------------------------------------------------------------------------
void GEVulkanDriver::createUnicolorTextures()
{
    constexpr unsigned size = 2;
    std::array<uint8_t, size * size * 4> data;
    data.fill(255);
    video::IImage* img = createImageFromData(video::ECF_A8R8G8B8,
        core::dimension2d<u32>(size, size), data.data(),
        /*ownForeignMemory*/true, /*deleteMemory*/false);
    m_white_texture = new GEVulkanTexture(img, "unicolor_white");
    data.fill(0);
    img = createImageFromData(video::ECF_A8R8G8B8,
        core::dimension2d<u32>(size, size), data.data(),
        /*ownForeignMemory*/true, /*deleteMemory*/false);
    m_transparent_texture = new GEVulkanTexture(img, "unicolor_transparent");
}   // createUnicolorTextures

// ----------------------------------------------------------------------------
void GEVulkanDriver::createInstance(SDL_Window* window)
{
#if !defined(__APPLE__) || defined(DLOPEN_MOLTENVK)
    if (gladLoadVulkanUserPtr(NULL, (GLADuserptrloadfunc)loader, NULL) == 0)
    {
        throw std::runtime_error("gladLoadVulkanUserPtr failed 1st time");
    }
#endif

    unsigned int count = 0;
    if (!SDL_Vulkan_GetInstanceExtensions(window, &count, NULL))
        throw std::runtime_error("SDL_Vulkan_GetInstanceExtensions failed with NULL extensions");
    std::vector<const char*> extensions(count, NULL);
    if (!SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data()))
        throw std::runtime_error("SDL_Vulkan_GetInstanceExtensions failed with extensions vector");

    uint32_t vk_version = 0;
    bool vulkan_1_1 = false;
#if !defined(__APPLE__) || defined(DLOPEN_MOLTENVK)
    PFN_vkEnumerateInstanceVersion e_ver = (PFN_vkEnumerateInstanceVersion)
        vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceVersion");
    vulkan_1_1 = (e_ver && e_ver(&vk_version) == VK_SUCCESS &&
        vk_version >= VK_API_VERSION_1_1);
#else
    vulkan_1_1 = (vkEnumerateInstanceVersion(&vk_version) == VK_SUCCESS &&
        vk_version >= VK_API_VERSION_1_1);
#endif

    uint32_t layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    std::vector<const char*> enabled_validation_layers;
#ifdef ENABLE_VALIDATION
    for (VkLayerProperties& prop : available_layers)
    {
        if (std::string(prop.layerName) == "VK_LAYER_KHRONOS_validation")
            enabled_validation_layers.push_back("VK_LAYER_KHRONOS_validation");
    }
#endif

    VkInstanceCreateInfo create_info = {};
    VkApplicationInfo app_info = {};
    if (vulkan_1_1)
    {
        // From https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkApplicationInfo.html
        // Implementations that support Vulkan 1.1 or later must not return VK_ERROR_INCOMPATIBLE_DRIVER for any value of apiVersion.
        app_info.apiVersion = VK_API_VERSION_1_2;
    }
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.enabledExtensionCount = extensions.size();
    create_info.ppEnabledExtensionNames = extensions.data();
    create_info.pApplicationInfo = &app_info;
    create_info.enabledLayerCount = enabled_validation_layers.size();
    create_info.ppEnabledLayerNames = enabled_validation_layers.data();
    VkResult result = vkCreateInstance(&create_info, NULL, &m_vk->instance);
    if (result != VK_SUCCESS)
        throw std::runtime_error("vkCreateInstance failed");
}   // createInstance

// ----------------------------------------------------------------------------
void GEVulkanDriver::findPhysicalDevice()
{
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(m_vk->instance, &device_count, NULL);

    if (device_count < 1)
        throw std::runtime_error("findPhysicalDevice has < 1 device_count");

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(m_vk->instance, &device_count, &devices[0]);

    for (VkPhysicalDevice& device : devices)
    {
        uint32_t graphics_family = 0;
        uint32_t present_family = 0;
        unsigned graphics_queue_count = 0;

        bool success = findQueueFamilies(device, &graphics_family,
            &graphics_queue_count, &present_family);

        if (!success)
            continue;

        success = checkDeviceExtensions(device);

        if (!success)
            continue;

        VkSurfaceCapabilitiesKHR surface_capabilities;
        std::vector<VkSurfaceFormatKHR> surface_formats;
        std::vector<VkPresentModeKHR> present_modes;

        success = updateSurfaceInformation(device, &surface_capabilities,
            &surface_formats, &present_modes);

        if (!success)
            continue;

        vkGetPhysicalDeviceFeatures(device, &m_features);
        m_graphics_family = graphics_family;
        m_graphics_queue_count = graphics_queue_count;
        m_present_family = present_family;
        m_surface_capabilities = surface_capabilities;
        m_surface_formats = surface_formats;
        m_present_modes = present_modes;
        m_physical_device = device;
        break;
    }

    if (m_physical_device == VK_NULL_HANDLE)
        throw std::runtime_error("findPhysicalDevice m_physical_device is VK_NULL_HANDLE");
}   // findPhysicalDevice

// ----------------------------------------------------------------------------
bool GEVulkanDriver::checkDeviceExtensions(VkPhysicalDevice device)
{
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);

    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count,
        &extensions[0]);

    std::set<std::string> required_extensions(m_device_extensions.begin(),
        m_device_extensions.end());

    for (VkExtensionProperties& extension : extensions)
        required_extensions.erase(extension.extensionName);

    return required_extensions.empty();
}   // checkDeviceExtensions

// ----------------------------------------------------------------------------
bool GEVulkanDriver::updateSurfaceInformation(VkPhysicalDevice device,
                                              VkSurfaceCapabilitiesKHR* surface_capabilities,
                                              std::vector<VkSurfaceFormatKHR>* surface_formats,
                                              std::vector<VkPresentModeKHR>* present_modes)
{
    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_vk->surface, &format_count, NULL);

    if (format_count < 1)
        return false;

    uint32_t mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_vk->surface, &mode_count, NULL);

    if (mode_count < 1)
        return false;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_vk->surface, surface_capabilities);

    (*surface_formats).resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_vk->surface, &format_count,
        &(*surface_formats)[0]);

    (*present_modes).resize(mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_vk->surface, &mode_count,
        &(*present_modes)[0]);

    return true;
}   // updateSurfaceInformation

// ----------------------------------------------------------------------------
bool GEVulkanDriver::findQueueFamilies(VkPhysicalDevice device,
                                       uint32_t* graphics_family,
                                       unsigned* graphics_queue_count,
                                       uint32_t* present_family)
{
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);
    if (queue_family_count == 0)
        return false;

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
        &queue_families[0]);

    bool found_graphics_family = false;
    bool found_present_family = false;

    for (unsigned int i = 0; i < queue_families.size(); i++)
    {
        if (queue_families[i].queueCount > 0 &&
            queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            *graphics_family = i;
            *graphics_queue_count = queue_families[i].queueCount;
            found_graphics_family = true;
            break;
        }
    }

    for (unsigned int i = 0; i < queue_families.size(); i++)
    {
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_vk->surface, &present_support);

        if (queue_families[i].queueCount > 0 && present_support)
        {
            *present_family = i;
            found_present_family = true;
            break;
        }
    }

    return found_graphics_family && found_present_family;
}   // findQueueFamilies

// ----------------------------------------------------------------------------
void GEVulkanDriver::createDevice()
{
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::vector<float> queue_priority;
    queue_priority.resize(m_graphics_queue_count, 1.0f);

    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = m_graphics_family;
    queue_create_info.queueCount = m_graphics_queue_count;
    queue_create_info.pQueuePriorities = queue_priority.data();
    queue_create_infos.push_back(queue_create_info);

    if (m_present_family != m_graphics_family)
    {
        queue_create_info.queueFamilyIndex = m_present_family;
        queue_create_info.queueCount = 1;
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features = {};
    device_features.shaderSampledImageArrayDynamicIndexing =
        GEVulkanFeatures::supportsBindTexturesAtOnce();

    VkPhysicalDeviceVulkan12Features vulkan12_features = {};
    vulkan12_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    vulkan12_features.descriptorIndexing =
        GEVulkanFeatures::supportsDescriptorIndexing();
    vulkan12_features.shaderSampledImageArrayNonUniformIndexing =
        GEVulkanFeatures::supportsNonUniformIndexing();
    vulkan12_features.descriptorBindingPartiallyBound =
        GEVulkanFeatures::supportsPartiallyBound();

    if (m_features.samplerAnisotropy == VK_TRUE)
        device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = queue_create_infos.size();
    create_info.pQueueCreateInfos = &queue_create_infos[0];
    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount = m_device_extensions.size();
    create_info.ppEnabledExtensionNames = &m_device_extensions[0];
    create_info.enabledLayerCount = 0;
    create_info.pNext = &vulkan12_features;

    VkResult result = vkCreateDevice(m_physical_device, &create_info, NULL, &m_vk->device);

    if (result != VK_SUCCESS)
        throw std::runtime_error("vkCreateDevice failed");

    m_graphics_queue.resize(m_graphics_queue_count);
    for (unsigned i = 0; i < m_graphics_queue_count; i++)
    {
        vkGetDeviceQueue(m_vk->device, m_graphics_family, i,
            &m_graphics_queue[i]);
        m_graphics_queue_mutexes.push_back(new std::mutex());
    }

    if (m_present_family != m_graphics_family)
        vkGetDeviceQueue(m_vk->device, m_present_family, 0, &m_present_queue);
}   // createDevice

// ----------------------------------------------------------------------------
std::unique_lock<std::mutex> GEVulkanDriver::getGraphicsQueue(VkQueue* queue) const
{
    if (m_graphics_queue_count == 0)
        throw std::runtime_error("No graphics queue created");
    if (m_graphics_queue_count == 1)
    {
        *queue = m_graphics_queue[0];
        return std::unique_lock<std::mutex>(*m_graphics_queue_mutexes[0]);
    }
    while (true)
    {
        for (unsigned i = 0; i < m_graphics_queue_count; i++)
        {
            std::unique_lock<std::mutex> lock(*m_graphics_queue_mutexes[i],
                std::defer_lock);
            if (lock.try_lock())
            {
                *queue = m_graphics_queue[i];
                return lock;
            }
        }
    }
}   // getGraphicsQueue

// ----------------------------------------------------------------------------
std::string GEVulkanDriver::getVulkanVersionString() const
{
    std::stringstream vk_version;
    vk_version << VK_VERSION_MAJOR(m_properties.apiVersion) << "." <<
        VK_VERSION_MINOR(m_properties.apiVersion) << "." <<
        VK_VERSION_PATCH(m_properties.apiVersion);
    return vk_version.str();
}   // getVulkanVersionString

// ----------------------------------------------------------------------------
std::string GEVulkanDriver::getDriverVersionString() const
{
#ifdef WIN32
    bool is_win = true;
#else
    bool is_win = false;
#endif

    std::stringstream driver_version;
    // Following https://github.com/SaschaWillems/vulkan.gpuinfo.org/blob/master/includes/functions.php
    if (m_properties.vendorID == 0x10DE)
    {
        // NVIDIA
        driver_version << ((m_properties.driverVersion >> 22) & 0x3ff) << "." <<
            ((m_properties.driverVersion >> 14) & 0xff) << "." <<
            ((m_properties.driverVersion >> 6) & 0xff) << "." <<
            (m_properties.driverVersion & 0x3f);
    }
    else if (m_properties.vendorID == 0x8086 && is_win)
    {
        // Intel on Windows
        driver_version << (m_properties.driverVersion >> 14) << "." <<
            (m_properties.driverVersion & 0x3fff);
    }
    else
    {
        // Use Vulkan version conventions if vendor mapping is not available
        driver_version << VK_VERSION_MAJOR(m_properties.driverVersion) << "." <<
            VK_VERSION_MINOR(m_properties.driverVersion) << "." <<
            VK_VERSION_PATCH(m_properties.driverVersion);
    }
    return driver_version.str();
}   // getDriverVersionString

// ----------------------------------------------------------------------------
void GEVulkanDriver::createSwapChain()
{
    VkSurfaceFormatKHR surface_format = m_surface_formats[0];

    if (m_surface_formats.size() == 1 &&
        m_surface_formats[0].format == VK_FORMAT_UNDEFINED)
    {
        surface_format =
        {
            VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        };
    }
    else
    {
        for (VkSurfaceFormatKHR& available_format : m_surface_formats)
        {
            if (available_format.format == VK_FORMAT_B8G8R8A8_UNORM &&
                available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                surface_format = available_format;
                break;
            }
        }
    }

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    if (m_params.SwapInterval == 0)
    {
        // Workaround for https://gitlab.freedesktop.org/mesa/mesa/-/issues/5516
        bool ignore_mailbox_mode = false;
#ifdef __LINUX__
        ignore_mailbox_mode =  true;
#endif
        if (!ignore_mailbox_mode)
        {
            for (VkPresentModeKHR& available_mode : m_present_modes)
            {
                if (available_mode == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    present_mode = available_mode;
                    goto found_mode;
                }
            }
        }
        for (VkPresentModeKHR& available_mode : m_present_modes)
        {
            if (available_mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                present_mode = available_mode;
                goto found_mode;
            }
        }
        for (VkPresentModeKHR& available_mode : m_present_modes)
        {
            if (available_mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
            {
                present_mode = available_mode;
                goto found_mode;
            }
        }
    }
found_mode:

    // Try to get triple buffering by default
    // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
    uint32_t swap_chain_images_count = m_surface_capabilities.minImageCount + 1;
    if (m_surface_capabilities.maxImageCount > 0 &&
        swap_chain_images_count > m_surface_capabilities.maxImageCount)
    {
        swap_chain_images_count = m_surface_capabilities.maxImageCount;
    }

    int w, h = 0;
    SDL_Vulkan_GetDrawableSize(m_window, &w, &h);
    VkExtent2D max_extent = m_surface_capabilities.maxImageExtent;
    VkExtent2D min_extent = m_surface_capabilities.minImageExtent;
    VkExtent2D actual_extent =
    {
        std::max(
            std::min((unsigned)w, max_extent.width), min_extent.width),
        std::max(
            std::min((unsigned)h, max_extent.height), min_extent.height)
    };

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = m_vk->surface;
    create_info.minImageCount = swap_chain_images_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = actual_extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = { m_graphics_family, m_present_family };
    if (m_graphics_family != m_present_family)
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    if (m_surface_capabilities.currentTransform != VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        if (m_surface_capabilities.currentTransform == VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR ||
            m_surface_capabilities.currentTransform == VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR ||
            m_surface_capabilities.currentTransform == VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
        {
            os::Printer::log("Vulkan preTransform", "using pre-rotation matrix");
        }
        else
        {
            m_surface_capabilities.currentTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
            os::Printer::log("Vulkan preTransform", "forcing identity, may affect performance");
        }
    }
    create_info.preTransform = m_surface_capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    if ((m_surface_capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) == 0)
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    m_swap_chain_extent = actual_extent;
    ScreenSize.Width = m_swap_chain_extent.width;
    ScreenSize.Height = m_swap_chain_extent.height;
    m_clip = getFullscreenClip();
    setViewPort(core::recti(0, 0, ScreenSize.Width, ScreenSize.Height));
    initPreRotationMatrix();
    if (m_surface_capabilities.currentTransform == VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR ||
        m_surface_capabilities.currentTransform == VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
    {
        std::swap(create_info.imageExtent.width, create_info.imageExtent.height);
        std::swap(m_swap_chain_extent.width, m_swap_chain_extent.height);
    }

    VkResult result = vkCreateSwapchainKHR(m_vk->device, &create_info, NULL,
        &m_vk->swap_chain);

    if (result != VK_SUCCESS)
        throw std::runtime_error("vkCreateSwapchainKHR failed");

    vkGetSwapchainImagesKHR(m_vk->device, m_vk->swap_chain, &swap_chain_images_count, NULL);
    m_vk->swap_chain_images.resize(swap_chain_images_count);
    vkGetSwapchainImagesKHR(m_vk->device, m_vk->swap_chain, &swap_chain_images_count,
        &m_vk->swap_chain_images[0]);

    m_swap_chain_image_format = surface_format.format;
    for (unsigned int i = 0; i < m_vk->swap_chain_images.size(); i++)
    {
        VkImageViewCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = m_vk->swap_chain_images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = m_swap_chain_image_format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        VkImageView swap_chain_image_view = VK_NULL_HANDLE;
        VkResult result = vkCreateImageView(m_vk->device, &create_info, NULL,
            &swap_chain_image_view);

        if (result != VK_SUCCESS)
            throw std::runtime_error("vkCreateImageView failed");
        m_vk->swap_chain_image_views.push_back(swap_chain_image_view);
    }
}   // createSwapChain

// ----------------------------------------------------------------------------
void GEVulkanDriver::createSyncObjects()
{
    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (unsigned int i = 0; i < getMaxFrameInFlight(); i++)
    {
        VkSemaphore image_available_semaphore;
        VkResult result = vkCreateSemaphore(m_vk->device, &semaphore_info, NULL,
            &image_available_semaphore);

        if (result != VK_SUCCESS)
        {
            throw std::runtime_error(
                "vkCreateSemaphore on image_available_semaphore failed");
        }

        VkSemaphore render_finished_semaphore;
        result = vkCreateSemaphore(m_vk->device, &semaphore_info, NULL,
            &render_finished_semaphore);

        if (result != VK_SUCCESS)
        {
            throw std::runtime_error(
                "vkCreateSemaphore on render_finished_semaphore failed");
        }

        VkFence in_flight_fence;
        result = vkCreateFence(m_vk->device, &fence_info, NULL,
            &in_flight_fence);

        if (result != VK_SUCCESS)
            throw std::runtime_error("vkCreateFence failed");

        m_vk->image_available_semaphores.push_back(image_available_semaphore);
        m_vk->render_finished_semaphores.push_back(render_finished_semaphore);
        m_vk->in_flight_fences.push_back(in_flight_fence);
    }
}   // createSyncObjects

// ----------------------------------------------------------------------------
void GEVulkanDriver::createCommandBuffers()
{
    std::vector<VkCommandBuffer> buffers(getMaxFrameInFlight());

    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = GEVulkanCommandLoader::getCurrentCommandPool();
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = (uint32_t)buffers.size();

    VkResult result = vkAllocateCommandBuffers(m_vk->device, &alloc_info,
        &buffers[0]);

    if (result != VK_SUCCESS)
        throw std::runtime_error("vkAllocateCommandBuffers failed");

    m_vk->command_buffers = buffers;
}   // createCommandBuffers

// ----------------------------------------------------------------------------
void GEVulkanDriver::createSamplers()
{
    VkSampler sampler = VK_NULL_HANDLE;
    bool supported_anisotropy = m_features.samplerAnisotropy == VK_TRUE;
    // GVS_NEAREST
    VkSamplerCreateInfo sampler_info = {};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_NEAREST;
    sampler_info.minFilter = VK_FILTER_NEAREST;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = supported_anisotropy;
    sampler_info.maxAnisotropy = 1.0;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    VkResult result = vkCreateSampler(m_vk->device, &sampler_info, NULL,
        &sampler);

    if (result != VK_SUCCESS)
        throw std::runtime_error("vkCreateSampler failed for GVS_NEAREST");
    m_vk->samplers[GVS_NEAREST] = sampler;
}   // createSamplers

// ----------------------------------------------------------------------------
void GEVulkanDriver::createRenderPass()
{
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = m_swap_chain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 1> attachments = { color_attachment };
    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = (uint32_t)(attachments.size());
    render_pass_info.pAttachments = &attachments[0];
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    VkResult result = vkCreateRenderPass(m_vk->device, &render_pass_info, NULL,
        &m_vk->render_pass);

    if (result != VK_SUCCESS)
        throw std::runtime_error("vkCreateRenderPass failed");
}   // createRenderPass

// ----------------------------------------------------------------------------
void GEVulkanDriver::createFramebuffers()
{
    const std::vector<VkImageView>& image_views = m_vk->swap_chain_image_views;
    for (unsigned int i = 0; i < image_views.size(); i++)
    {
        std::array<VkImageView, 1> attachments =
        {
            image_views[i]
        };

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = m_vk->render_pass;
        framebuffer_info.attachmentCount = (uint32_t)(attachments.size());
        framebuffer_info.pAttachments = &attachments[0];
        framebuffer_info.width = m_swap_chain_extent.width;
        framebuffer_info.height = m_swap_chain_extent.height;
        framebuffer_info.layers = 1;

        VkFramebuffer swap_chain_framebuffer = VK_NULL_HANDLE;
        VkResult result = vkCreateFramebuffer(m_vk->device, &framebuffer_info,
            NULL, &swap_chain_framebuffer);
        if (result != VK_SUCCESS)
            throw std::runtime_error("vkCreateFramebuffer failed");

        m_vk->swap_chain_framebuffers.push_back(swap_chain_framebuffer);
    }
}   // createFramebuffers

// ----------------------------------------------------------------------------
bool GEVulkanDriver::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                  VkMemoryPropertyFlags properties,
                                  VkBuffer& buffer,
                                  VkDeviceMemory& buffer_memory)
{
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(m_vk->device, &buffer_info, NULL, &buffer);

    if (result != VK_SUCCESS)
        return false;

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(m_vk->device, buffer, &mem_requirements);

    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(m_physical_device, &mem_properties);

    uint32_t memory_type_index = std::numeric_limits<uint32_t>::max();
    uint32_t type_filter = mem_requirements.memoryTypeBits;

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
    {
        if ((type_filter & (1 << i)) &&
            (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            memory_type_index = i;
            break;
        }
    }

    if (memory_type_index == std::numeric_limits<uint32_t>::max())
        return false;

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = memory_type_index;

    result = vkAllocateMemory(m_vk->device, &alloc_info, NULL, &buffer_memory);

    if (result != VK_SUCCESS)
        return false;

    vkBindBufferMemory(m_vk->device, buffer, buffer_memory, 0);

    return true;
}   // createBuffer

// ----------------------------------------------------------------------------
void GEVulkanDriver::copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer,
                                VkDeviceSize size)
{
    VkCommandBuffer command_buffer = GEVulkanCommandLoader::beginSingleTimeCommands();

    VkBufferCopy copy_region = {};
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

    GEVulkanCommandLoader::endSingleTimeCommands(command_buffer);
}   // copyBuffer

// ----------------------------------------------------------------------------
void GEVulkanDriver::OnResize(const core::dimension2d<u32>& size)
{
    CNullDriver::OnResize(size);
    if (g_paused_rendering.load() == false)
    {
        destroySwapChainRelated(false/*handle_surface*/);
        createSwapChainRelated(false/*handle_surface*/);
    }
}   // OnResize

// ----------------------------------------------------------------------------
bool GEVulkanDriver::beginScene(bool backBuffer, bool zBuffer, SColor color,
                                const SExposedVideoData& videoData,
                                core::rect<s32>* sourceRect)
{
    if (g_schedule_pausing_rendering.load())
    {
        pauseRendering();
        g_schedule_pausing_rendering.store(false);
    }

    if (g_paused_rendering.load() ||
        !video::CNullDriver::beginScene(backBuffer, zBuffer, color, videoData,
        sourceRect))
        return false;

    m_clear_color = color;
    VkFence fence = m_vk->in_flight_fences[m_current_frame];
    vkWaitForFences(m_vk->device, 1, &fence, VK_TRUE,
        std::numeric_limits<uint64_t>::max());
    vkResetFences(m_vk->device, 1, &fence);

    VkSemaphore semaphore = m_vk->image_available_semaphores[m_current_frame];
    VkResult result = vkAcquireNextImageKHR(m_vk->device, m_vk->swap_chain,
        std::numeric_limits<uint64_t>::max(), semaphore, VK_NULL_HANDLE,
        &m_image_index);

    return (result != VK_ERROR_OUT_OF_DATE_KHR);
}   // beginScene

// ----------------------------------------------------------------------------
bool GEVulkanDriver::endScene()
{
    if (g_paused_rendering.load())
    {
        GEVulkan2dRenderer::clear();
        return false;
    }

    GEVulkan2dRenderer::render();

    VkSemaphore wait_semaphores[] = {m_vk->image_available_semaphores[m_current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signal_semaphores[] = {m_vk->render_finished_semaphores[m_current_frame]};

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_vk->command_buffers[m_current_frame];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    VkQueue queue = VK_NULL_HANDLE;
    std::unique_lock<std::mutex> ul = getGraphicsQueue(&queue);
    VkResult result = vkQueueSubmit(queue, 1, &submit_info,
        m_vk->in_flight_fences[m_current_frame]);
    ul.unlock();

    if (result != VK_SUCCESS)
        throw std::runtime_error("vkQueueSubmit failed");

    VkSemaphore semaphores[] =
    {
        m_vk->render_finished_semaphores[m_current_frame]
    };
    VkSwapchainKHR swap_chains[] =
    {
        m_vk->swap_chain
    };

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = semaphores;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &m_image_index;

    m_current_frame = (m_current_frame + 1) % getMaxFrameInFlight();

    if (m_present_queue)
        result = vkQueuePresentKHR(m_present_queue, &present_info);
    else
    {
        VkQueue present_queue = VK_NULL_HANDLE;
        std::unique_lock<std::mutex> ul = getGraphicsQueue(&present_queue);
        result = vkQueuePresentKHR(present_queue, &present_info);
        ul.unlock();
    }
    if (!video::CNullDriver::endScene())
        return false;

    return (result != VK_ERROR_OUT_OF_DATE_KHR && result != VK_SUBOPTIMAL_KHR);
}   // endScene

// ----------------------------------------------------------------------------
void GEVulkanDriver::draw2DVertexPrimitiveList(const void* vertices,
                                               u32 vertexCount,
                                               const void* indexList,
                                               u32 primitiveCount,
                                               E_VERTEX_TYPE vType,
                                               scene::E_PRIMITIVE_TYPE pType,
                                               E_INDEX_TYPE iType)
{
    const GEVulkanTexture* texture =
        dynamic_cast<const GEVulkanTexture*>(Material.getTexture(0));
    if (!texture)
        return;
    if (vType != EVT_STANDARD || iType != EIT_16BIT)
        return;
    if (pType == irr::scene::EPT_TRIANGLES)
    {
        S3DVertex* v = (S3DVertex*)vertices;
        u16* i = (u16*)indexList;
        GEVulkan2dRenderer::addVerticesIndices(v, vertexCount, i,
            primitiveCount, texture);
    }
}   // draw2DVertexPrimitiveList

// ----------------------------------------------------------------------------
void GEVulkanDriver::draw2DImage(const video::ITexture* tex,
                                 const core::position2d<s32>& destPos,
                                 const core::rect<s32>& sourceRect,
                                 const core::rect<s32>* clipRect,
                                 SColor color, bool useAlphaChannelOfTexture)
{
    const GEVulkanTexture* texture = dynamic_cast<const GEVulkanTexture*>(tex);
    if (!texture)
        return;

    if (!sourceRect.isValid())
        return;

    core::position2d<s32> targetPos = destPos;
    core::position2d<s32> sourcePos = sourceRect.UpperLeftCorner;
    // This needs to be signed as it may go negative.
    core::dimension2d<s32> sourceSize(sourceRect.getSize());

    if (clipRect)
    {
        if (targetPos.X < clipRect->UpperLeftCorner.X)
        {
            sourceSize.Width += targetPos.X - clipRect->UpperLeftCorner.X;
            if (sourceSize.Width <= 0)
                return;

            sourcePos.X -= targetPos.X - clipRect->UpperLeftCorner.X;
            targetPos.X = clipRect->UpperLeftCorner.X;
        }

        if (targetPos.X + (s32)sourceSize.Width > clipRect->LowerRightCorner.X)
        {
            sourceSize.Width -= (targetPos.X + sourceSize.Width) - clipRect->LowerRightCorner.X;
            if (sourceSize.Width <= 0)
                return;
        }

        if (targetPos.Y < clipRect->UpperLeftCorner.Y)
        {
            sourceSize.Height += targetPos.Y - clipRect->UpperLeftCorner.Y;
            if (sourceSize.Height <= 0)
                return;

            sourcePos.Y -= targetPos.Y - clipRect->UpperLeftCorner.Y;
            targetPos.Y = clipRect->UpperLeftCorner.Y;
        }

        if (targetPos.Y + (s32)sourceSize.Height > clipRect->LowerRightCorner.Y)
        {
            sourceSize.Height -= (targetPos.Y + sourceSize.Height) - clipRect->LowerRightCorner.Y;
            if (sourceSize.Height <= 0)
                return;
        }
    }

    // clip these coordinates

    if (targetPos.X<0)
    {
        sourceSize.Width += targetPos.X;
        if (sourceSize.Width <= 0)
            return;

        sourcePos.X -= targetPos.X;
        targetPos.X = 0;
    }

    const core::dimension2d<u32>& renderTargetSize = getCurrentRenderTargetSize();

    if (targetPos.X + sourceSize.Width > (s32)renderTargetSize.Width)
    {
        sourceSize.Width -= (targetPos.X + sourceSize.Width) - renderTargetSize.Width;
        if (sourceSize.Width <= 0)
            return;
    }

    if (targetPos.Y<0)
    {
        sourceSize.Height += targetPos.Y;
        if (sourceSize.Height <= 0)
            return;

        sourcePos.Y -= targetPos.Y;
        targetPos.Y = 0;
    }

    if (targetPos.Y + sourceSize.Height > (s32)renderTargetSize.Height)
    {
        sourceSize.Height -= (targetPos.Y + sourceSize.Height) - renderTargetSize.Height;
        if (sourceSize.Height <= 0)
            return;
    }

    // ok, we've clipped everything.
    // now draw it.

    core::rect<f32> tcoords;
    tcoords.UpperLeftCorner.X = (((f32)sourcePos.X)) / texture->getSize().Width ;
    tcoords.UpperLeftCorner.Y = (((f32)sourcePos.Y)) / texture->getSize().Height;
    tcoords.LowerRightCorner.X = tcoords.UpperLeftCorner.X + ((f32)(sourceSize.Width) / texture->getSize().Width);
    tcoords.LowerRightCorner.Y = tcoords.UpperLeftCorner.Y + ((f32)(sourceSize.Height) / texture->getSize().Height);

    const core::rect<s32> poss(targetPos, sourceSize);

    S3DVertex vtx[4];
    vtx[0] = S3DVertex((f32)poss.UpperLeftCorner.X, (f32)poss.UpperLeftCorner.Y, 0.0f,
            0.0f, 0.0f, 0.0f, color,
            tcoords.UpperLeftCorner.X, tcoords.UpperLeftCorner.Y);
    vtx[1] = S3DVertex((f32)poss.LowerRightCorner.X, (f32)poss.UpperLeftCorner.Y, 0.0f,
            0.0f, 0.0f, 0.0f, color,
            tcoords.LowerRightCorner.X, tcoords.UpperLeftCorner.Y);
    vtx[2] = S3DVertex((f32)poss.LowerRightCorner.X, (f32)poss.LowerRightCorner.Y, 0.0f,
            0.0f, 0.0f, 0.0f, color,
            tcoords.LowerRightCorner.X, tcoords.LowerRightCorner.Y);
    vtx[3] = S3DVertex((f32)poss.UpperLeftCorner.X, (f32)poss.LowerRightCorner.Y, 0.0f,
            0.0f, 0.0f, 0.0f, color,
            tcoords.UpperLeftCorner.X, tcoords.LowerRightCorner.Y);

    u16 indices[6] = {0,1,2,0,2,3};

    if (clipRect)
        enableScissorTest(*clipRect);
    GEVulkan2dRenderer::addVerticesIndices(&vtx[0], 4, &indices[0], 2, texture);
    if (clipRect)
        disableScissorTest();
}   // draw2DImage

// ----------------------------------------------------------------------------
void GEVulkanDriver::draw2DImage(const video::ITexture* tex,
                                 const core::rect<s32>& destRect,
                                 const core::rect<s32>& sourceRect,
                                 const core::rect<s32>* clipRect,
                                 const video::SColor* const colors,
                                 bool useAlphaChannelOfTexture)
{
    const GEVulkanTexture* texture = dynamic_cast<const GEVulkanTexture*>(tex);
    if (!texture)
        return;

    const core::dimension2d<u32>& ss = texture->getSize();
    core::rect<f32> tcoords;
    tcoords.UpperLeftCorner.X = (f32)sourceRect.UpperLeftCorner.X / (f32)ss.Width;
    tcoords.UpperLeftCorner.Y = (f32)sourceRect.UpperLeftCorner.Y / (f32)ss.Height;
    tcoords.LowerRightCorner.X = (f32)sourceRect.LowerRightCorner.X / (f32)ss.Width;
    tcoords.LowerRightCorner.Y = (f32)sourceRect.LowerRightCorner.Y / (f32)ss.Height;

    const core::dimension2d<u32>& renderTargetSize = getCurrentRenderTargetSize();

    const video::SColor temp[4] =
    {
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF
    };

    const video::SColor* const useColor = colors ? colors : temp;

    S3DVertex vtx[4];
    vtx[0] = S3DVertex((f32)destRect.UpperLeftCorner.X, (f32)destRect.UpperLeftCorner.Y, 0.0f,
            0.0f, 0.0f, 0.0f, useColor[0],
            tcoords.UpperLeftCorner.X, tcoords.UpperLeftCorner.Y);
    vtx[1] = S3DVertex((f32)destRect.LowerRightCorner.X, (f32)destRect.UpperLeftCorner.Y, 0.0f,
            0.0f, 0.0f, 0.0f, useColor[3],
            tcoords.LowerRightCorner.X, tcoords.UpperLeftCorner.Y);
    vtx[2] = S3DVertex((f32)destRect.LowerRightCorner.X, (f32)destRect.LowerRightCorner.Y, 0.0f,
            0.0f, 0.0f, 0.0f, useColor[2],
            tcoords.LowerRightCorner.X, tcoords.LowerRightCorner.Y);
    vtx[3] = S3DVertex((f32)destRect.UpperLeftCorner.X, (f32)destRect.LowerRightCorner.Y, 0.0f,
            0.0f, 0.0f, 0.0f, useColor[1],
            tcoords.UpperLeftCorner.X, tcoords.LowerRightCorner.Y);

    u16 indices[6] = {0,1,2,0,2,3};

    if (clipRect)
        enableScissorTest(*clipRect);
    GEVulkan2dRenderer::addVerticesIndices(&vtx[0], 4, &indices[0], 2, texture);
    if (clipRect)
        disableScissorTest();
}   // draw2DImage

// ----------------------------------------------------------------------------
void GEVulkanDriver::draw2DImageBatch(const video::ITexture* tex,
                          const core::array<core::position2d<s32> >& positions,
                          const core::array<core::rect<s32> >& sourceRects,
                          const core::rect<s32>* clipRect, SColor color,
                          bool useAlphaChannelOfTexture)
{
    const GEVulkanTexture* texture = dynamic_cast<const GEVulkanTexture*>(tex);
    if (!texture)
        return;

    const irr::u32 drawCount = core::min_<u32>(positions.size(), sourceRects.size());

    core::array<S3DVertex> vtx(drawCount * 4);
    core::array<u16> indices(drawCount * 6);

    for(u32 i = 0;i < drawCount;i++)
    {
        core::position2d<s32> targetPos = positions[i];
        core::position2d<s32> sourcePos = sourceRects[i].UpperLeftCorner;
        // This needs to be signed as it may go negative.
        core::dimension2d<s32> sourceSize(sourceRects[i].getSize());

        if (clipRect)
        {
            if (targetPos.X < clipRect->UpperLeftCorner.X)
            {
                sourceSize.Width += targetPos.X - clipRect->UpperLeftCorner.X;
                if (sourceSize.Width <= 0)
                    continue;

                sourcePos.X -= targetPos.X - clipRect->UpperLeftCorner.X;
                targetPos.X = clipRect->UpperLeftCorner.X;
            }

            if (targetPos.X + (s32)sourceSize.Width > clipRect->LowerRightCorner.X)
            {
                sourceSize.Width -= (targetPos.X + sourceSize.Width) - clipRect->LowerRightCorner.X;
                if (sourceSize.Width <= 0)
                    continue;
            }

            if (targetPos.Y < clipRect->UpperLeftCorner.Y)
            {
                sourceSize.Height += targetPos.Y - clipRect->UpperLeftCorner.Y;
                if (sourceSize.Height <= 0)
                    continue;

                sourcePos.Y -= targetPos.Y - clipRect->UpperLeftCorner.Y;
                targetPos.Y = clipRect->UpperLeftCorner.Y;
            }

            if (targetPos.Y + (s32)sourceSize.Height > clipRect->LowerRightCorner.Y)
            {
                sourceSize.Height -= (targetPos.Y + sourceSize.Height) - clipRect->LowerRightCorner.Y;
                if (sourceSize.Height <= 0)
                    continue;
            }
        }

        // clip these coordinates

        if (targetPos.X<0)
        {
            sourceSize.Width += targetPos.X;
            if (sourceSize.Width <= 0)
                continue;

            sourcePos.X -= targetPos.X;
            targetPos.X = 0;
        }

        const core::dimension2d<u32>& renderTargetSize = getCurrentRenderTargetSize();

        if (targetPos.X + sourceSize.Width > (s32)renderTargetSize.Width)
        {
            sourceSize.Width -= (targetPos.X + sourceSize.Width) - renderTargetSize.Width;
            if (sourceSize.Width <= 0)
                continue;
        }

        if (targetPos.Y<0)
        {
            sourceSize.Height += targetPos.Y;
            if (sourceSize.Height <= 0)
                continue;

            sourcePos.Y -= targetPos.Y;
            targetPos.Y = 0;
        }

        if (targetPos.Y + sourceSize.Height > (s32)renderTargetSize.Height)
        {
            sourceSize.Height -= (targetPos.Y + sourceSize.Height) - renderTargetSize.Height;
            if (sourceSize.Height <= 0)
                continue;
        }

        // ok, we've clipped everything.
        // now draw it.

        core::rect<f32> tcoords;
        tcoords.UpperLeftCorner.X = (((f32)sourcePos.X)) / texture->getSize().Width ;
        tcoords.UpperLeftCorner.Y = (((f32)sourcePos.Y)) / texture->getSize().Height;
        tcoords.LowerRightCorner.X = tcoords.UpperLeftCorner.X + ((f32)(sourceSize.Width) / texture->getSize().Width);
        tcoords.LowerRightCorner.Y = tcoords.UpperLeftCorner.Y + ((f32)(sourceSize.Height) / texture->getSize().Height);

        const core::rect<s32> poss(targetPos, sourceSize);

        vtx.push_back(S3DVertex((f32)poss.UpperLeftCorner.X, (f32)poss.UpperLeftCorner.Y, 0.0f,
                0.0f, 0.0f, 0.0f, color,
                tcoords.UpperLeftCorner.X, tcoords.UpperLeftCorner.Y));
        vtx.push_back(S3DVertex((f32)poss.LowerRightCorner.X, (f32)poss.UpperLeftCorner.Y, 0.0f,
                0.0f, 0.0f, 0.0f, color,
                tcoords.LowerRightCorner.X, tcoords.UpperLeftCorner.Y));
        vtx.push_back(S3DVertex((f32)poss.LowerRightCorner.X, (f32)poss.LowerRightCorner.Y, 0.0f,
                0.0f, 0.0f, 0.0f, color,
                tcoords.LowerRightCorner.X, tcoords.LowerRightCorner.Y));
        vtx.push_back(S3DVertex((f32)poss.UpperLeftCorner.X, (f32)poss.LowerRightCorner.Y, 0.0f,
                0.0f, 0.0f, 0.0f, color,
                tcoords.UpperLeftCorner.X, tcoords.LowerRightCorner.Y));

        const u32 curPos = vtx.size()-4;
        indices.push_back(0+curPos);
        indices.push_back(1+curPos);
        indices.push_back(2+curPos);

        indices.push_back(0+curPos);
        indices.push_back(2+curPos);
        indices.push_back(3+curPos);
    }

    if (vtx.size())
    {
        if (clipRect)
            enableScissorTest(*clipRect);
        GEVulkan2dRenderer::addVerticesIndices(vtx.pointer(), vtx.size(),
            indices.pointer(), indices.size() / 3, texture);
        if (clipRect)
            disableScissorTest();
    }
}   // draw2DImageBatch

// ----------------------------------------------------------------------------
void GEVulkanDriver::setViewPort(const core::rect<s32>& area)
{
    core::rect<s32> vp = area;
    core::rect<s32> rendert(0,0, getCurrentRenderTargetSize().Width, getCurrentRenderTargetSize().Height);
    vp.clipAgainst(rendert);
    if (vp.getHeight() > 0 && vp.getWidth() > 0)
        m_viewport = vp;
}   // setViewPort

// ----------------------------------------------------------------------------
void GEVulkanDriver::getRotatedRect2D(VkRect2D* rect)
{
    // https://developer.android.com/games/optimize/vulkan-prerotation
    if (m_surface_capabilities.currentTransform == VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR)
    {
        VkRect2D ret =
        {
            (int)(m_swap_chain_extent.width - rect->extent.height - rect->offset.y),
            rect->offset.x,
            rect->extent.height,
            rect->extent.width
        };
        *rect = ret;
    }
    else if (m_surface_capabilities.currentTransform == VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR)
    {
        VkRect2D ret =
        {
            (int)(m_swap_chain_extent.width - rect->extent.width - rect->offset.x),
            (int)(m_swap_chain_extent.height - rect->extent.height - rect->offset.y),
            rect->extent.width,
            rect->extent.height
        };
        *rect = ret;
    }
    else if (m_surface_capabilities.currentTransform == VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
    {
        VkRect2D ret =
        {
            rect->offset.y,
            (int)(m_swap_chain_extent.height - rect->extent.width - rect->offset.x),
            rect->extent.height,
            rect->extent.width
        };
        *rect = ret;
    }
}   // getRotatedRect2D

// ----------------------------------------------------------------------------
void GEVulkanDriver::getRotatedViewport(VkViewport* vp)
{
    VkRect2D rect;
    rect.offset.x = vp->x;
    rect.offset.y = vp->y;
    rect.extent.width = vp->width;
    rect.extent.height = vp->height;
    getRotatedRect2D(&rect);
    vp->x = rect.offset.x;
    vp->y = rect.offset.y;
    vp->width = rect.extent.width;
    vp->height = rect.extent.height;
}   // getRotatedViewport

// ----------------------------------------------------------------------------
void GEVulkanDriver::initPreRotationMatrix()
{
    const double pi = 3.14159265358979323846;
    if (m_surface_capabilities.currentTransform == VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR)
        m_pre_rotation_matrix.setRotationAxisRadians(90.0 * pi / 180., core::vector3df(0.0f, 0.0f, 1.0f));
    else if (m_surface_capabilities.currentTransform == VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR)
        m_pre_rotation_matrix.setRotationAxisRadians(180.0 * pi / 180., core::vector3df(0.0f, 0.0f, 1.0f));
    else if (m_surface_capabilities.currentTransform == VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
        m_pre_rotation_matrix.setRotationAxisRadians(270.0 * pi / 180., core::vector3df(0.0f, 0.0f, 1.0f));
}   // initPreRotationMatrix

// ----------------------------------------------------------------------------
void GEVulkanDriver::pauseRendering()
{
    if (g_paused_rendering.load() != false)
        return;

    destroySwapChainRelated(true/*handle_surface*/);
    g_paused_rendering.store(true);
}   // pauseRendering

// ----------------------------------------------------------------------------
void GEVulkanDriver::unpauseRendering()
{
    if (g_paused_rendering.load() != true)
        return;

    createSwapChainRelated(true/*handle_surface*/);
    g_paused_rendering.store(false);
}   // unpauseRendering

// ----------------------------------------------------------------------------
void GEVulkanDriver::destroySwapChainRelated(bool handle_surface)
{
    vkDeviceWaitIdle(m_vk->device);
    for (VkFramebuffer& framebuffer : m_vk->swap_chain_framebuffers)
        vkDestroyFramebuffer(m_vk->device, framebuffer, NULL);
    m_vk->swap_chain_framebuffers.clear();
    if (m_vk->render_pass != VK_NULL_HANDLE)
        vkDestroyRenderPass(m_vk->device, m_vk->render_pass, NULL);
    m_vk->render_pass = VK_NULL_HANDLE;
    for (VkImageView& image_view : m_vk->swap_chain_image_views)
        vkDestroyImageView(m_vk->device, image_view, NULL);
    m_vk->swap_chain_image_views.clear();
    if (m_vk->swap_chain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(m_vk->device, m_vk->swap_chain, NULL);
    m_vk->swap_chain = VK_NULL_HANDLE;
    if (handle_surface)
    {
        if (m_vk->surface != VK_NULL_HANDLE)
            vkDestroySurfaceKHR(m_vk->instance, m_vk->surface, NULL);
        m_vk->surface = VK_NULL_HANDLE;
    }
}   // destroySwapChainRelated

// ----------------------------------------------------------------------------
void GEVulkanDriver::createSwapChainRelated(bool handle_surface)
{
    vkDeviceWaitIdle(m_vk->device);
    if (handle_surface)
    {
        if (SDL_Vulkan_CreateSurface(m_window, m_vk->instance, &m_vk->surface) == SDL_FALSE)
            throw std::runtime_error("SDL_Vulkan_CreateSurface failed");
    }
    updateSurfaceInformation(m_physical_device, &m_surface_capabilities,
        &m_surface_formats, &m_present_modes);
    createSwapChain();
    createRenderPass();
    createFramebuffers();
}   // createSwapChainRelated

}

namespace irr
{
namespace video
{
    IVideoDriver* createVulkanDriver(const SIrrlichtCreationParameters& params,
                                     io::IFileSystem* io, SDL_Window* window)
    {
        return new GE::GEVulkanDriver(params, io, window);
    }   // createVulkanDriver
}
}

#ifdef ANDROID
#include <jni.h>
extern "C" JNIEXPORT void JNICALL pauseRenderingJNI(JNIEnv* env, jclass cls)
{
    using namespace GE;
    if (g_device_created.load() == false || g_schedule_pausing_rendering.load() == true)
        return;
    g_schedule_pausing_rendering.store(true);
    if (g_paused_rendering.load() == false)
    {
        while (true)
        {
            if (g_device_created.load() == false || g_paused_rendering.load() == true)
                break;
        }
    }
}   // pauseRenderingJNI

#endif

#endif
