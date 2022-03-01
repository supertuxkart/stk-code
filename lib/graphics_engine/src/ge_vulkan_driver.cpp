#include "ge_vulkan_driver.hpp"

#ifdef _IRR_COMPILE_WITH_VULKAN_
const unsigned int MAX_FRAMES_IN_FLIGHT = 2;
#include "SDL_vulkan.h"
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
GEVulkanDriver::GEVulkanDriver(const SIrrlichtCreationParameters& params,
                               io::IFileSystem* io, SDL_Window* window)
              : CNullDriver(io, core::dimension2d<u32>(0, 0)),
                m_params(params)
{
    m_vk.reset(new VK());
    m_physical_device = VK_NULL_HANDLE;
    m_graphics_queue = VK_NULL_HANDLE;
    m_present_queue = VK_NULL_HANDLE;
    m_graphics_family = m_present_family = 0;
    m_properties = {};
    m_features = {};

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

    vkGetPhysicalDeviceProperties(m_physical_device, &m_properties);
    createSwapChain();
    createSyncObjects();
    createCommandPool();
    createCommandBuffers();
    createSamplers();
    os::Printer::log("Vulkan version", getVulkanVersionString().c_str());
    os::Printer::log("Vulkan vendor", getVendorInfo().c_str());
    os::Printer::log("Vulkan renderer", m_properties.deviceName);
    os::Printer::log("Vulkan driver version", getDriverVersionString().c_str());
    for (const char* ext : m_device_extensions)
        os::Printer::log("Vulkan enabled extension", ext);
}   // GEVulkanDriver

// ----------------------------------------------------------------------------
GEVulkanDriver::~GEVulkanDriver()
{
}   // ~GEVulkanDriver

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
    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.enabledExtensionCount = extensions.size();
    create_info.ppEnabledExtensionNames = extensions.data();
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

        bool success = findQueueFamilies(device, &graphics_family, &present_family);

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
    float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = m_graphics_family;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos.push_back(queue_create_info);

    queue_create_info.queueFamilyIndex = m_present_family;
    queue_create_infos.push_back(queue_create_info);

    VkPhysicalDeviceFeatures device_features = {};
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

    VkResult result = vkCreateDevice(m_physical_device, &create_info, NULL, &m_vk->device);

    if (result != VK_SUCCESS)
        throw std::runtime_error("vkCreateDevice failed");

    vkGetDeviceQueue(m_vk->device, m_graphics_family, 0, &m_graphics_queue);
    vkGetDeviceQueue(m_vk->device, m_present_family, 0, &m_present_queue);
}   // createDevice

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
        for (VkPresentModeKHR& available_mode : m_present_modes)
        {
            if (available_mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            {
                present_mode = available_mode;
                break;
            }
        }
    }
    else
    {
        for (VkPresentModeKHR& available_mode : m_present_modes)
        {
            if (available_mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                present_mode = available_mode;
                break;
            }
        }
    }

    VkExtent2D image_extent = m_surface_capabilities.currentExtent;
    if (m_surface_capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
    {
        VkExtent2D max_extent = m_surface_capabilities.maxImageExtent;
        VkExtent2D min_extent = m_surface_capabilities.minImageExtent;

        VkExtent2D actual_extent =
        {
            std::max(
                std::min(ScreenSize.Width, max_extent.width), min_extent.width),
            std::max(
                std::min(ScreenSize.Height, max_extent.height), min_extent.height)
        };
        image_extent = actual_extent;
    }

    // Try to get triple buffering by default
    // https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
    uint32_t swap_chain_images_count = m_surface_capabilities.minImageCount + 1;
    if (m_surface_capabilities.maxImageCount > 0 &&
        swap_chain_images_count > m_surface_capabilities.maxImageCount)
    {
        swap_chain_images_count = m_surface_capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = m_vk->surface;
    create_info.minImageCount = swap_chain_images_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = image_extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (m_graphics_family != m_present_family)
    {
        uint32_t queueFamilyIndices[] =
            { m_graphics_family, m_present_family };
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform = m_surface_capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(m_vk->device, &create_info, NULL,
        &m_vk->swap_chain);

    if (result != VK_SUCCESS)
        throw std::runtime_error("vkCreateSwapchainKHR failed");

    vkGetSwapchainImagesKHR(m_vk->device, m_vk->swap_chain, &swap_chain_images_count, NULL);
    m_vk->swap_chain_images.resize(swap_chain_images_count);
    vkGetSwapchainImagesKHR(m_vk->device, m_vk->swap_chain, &swap_chain_images_count,
        &m_vk->swap_chain_images[0]);

    m_swap_chain_image_format = surface_format.format;
    m_swap_chain_extent = image_extent;

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

    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
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
void GEVulkanDriver::createCommandPool()
{
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = m_graphics_family;

    VkResult result = vkCreateCommandPool(m_vk->device, &pool_info, NULL,
        &m_vk->command_pool);

    if (result != VK_SUCCESS)
        throw std::runtime_error("vkCreateCommandPool failed");
}   // createCommandPool

// ----------------------------------------------------------------------------
void GEVulkanDriver::createCommandBuffers()
{
    std::vector<VkCommandBuffer> buffers(m_vk->swap_chain_images.size());

    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = m_vk->command_pool;
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
VkCommandBuffer GEVulkanDriver::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = m_vk->command_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(m_vk->device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);

    return command_buffer;
}   // beginSingleTimeCommands

// ----------------------------------------------------------------------------
void GEVulkanDriver::endSingleTimeCommands(VkCommandBuffer command_buffer)
{
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(m_graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphics_queue);

    vkFreeCommandBuffers(m_vk->device, m_vk->command_pool, 1, &command_buffer);
}   // beginSingleTimeCommands

// ----------------------------------------------------------------------------
void GEVulkanDriver::OnResize(const core::dimension2d<u32>& size)
{
    CNullDriver::OnResize(size);
}   // OnResize

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
#endif
