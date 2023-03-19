#include "ge_vulkan_driver.hpp"

#include "ge_compressor_astc_4x4.hpp"
#include "ge_compressor_bptc_bc7.hpp"
#include "ge_main.hpp"
#include "ge_spm.hpp"
#include "ge_spm_buffer.hpp"

#include "ge_vulkan_2d_renderer.hpp"
#include "ge_vulkan_camera_scene_node.hpp"
#include "ge_vulkan_command_loader.hpp"
#include "ge_vulkan_depth_texture.hpp"
#include "ge_vulkan_draw_call.hpp"
#include "ge_vulkan_dynamic_spm_buffer.hpp"
#include "ge_vulkan_fbo_texture.hpp"
#include "ge_vulkan_features.hpp"
#include "ge_vulkan_mesh_cache.hpp"
#include "ge_vulkan_scene_manager.hpp"
#include "ge_vulkan_shader_manager.hpp"
#include "ge_vulkan_skybox_renderer.hpp"
#include "ge_vulkan_texture_descriptor.hpp"

#include "mini_glm.hpp"

#include "ISceneManager.h"
#include "IrrlichtDevice.h"

#ifdef __ANDROID__
#include <android/log.h>
#endif

#ifdef _IRR_COMPILE_WITH_VULKAN_
#include "SDL_vulkan.h"
#include <algorithm>
#include <atomic>
#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>
#include "../source/Irrlicht/os.h"

extern "C" VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data)
{
    std::string msg = callback_data->pMessage;
    if (msg.find("UNASSIGNED-CoreValidation-Shader-OutputNotConsumed") != std::string::npos)
        return VK_FALSE;
#ifdef __ANDROID__
    android_LogPriority alp;
    switch (message_severity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: alp = ANDROID_LOG_DEBUG;   break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: alp = ANDROID_LOG_INFO;    break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: alp = ANDROID_LOG_WARN;    break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: alp = ANDROID_LOG_ERROR;   break;
    default: alp = ANDROID_LOG_INFO;
    }
    __android_log_print(alp, "VALIDATION:", "%s", msg.c_str());
#else
    printf("%s\n", msg.c_str());
#endif
    return VK_FALSE;
};

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
bool g_debug_print = false;

GEVulkanDriver::GEVulkanDriver(const SIrrlichtCreationParameters& params,
                               io::IFileSystem* io, SDL_Window* window,
                               IrrlichtDevice* device)
              : CNullDriver(io, core::dimension2d<u32>(0, 0)),
                m_params(params), m_irrlicht_device(device),
                m_depth_texture(NULL), m_mesh_texture_descriptor(NULL),
                m_rtt_texture(NULL), m_prev_rtt_texture(NULL),
                m_separate_rtt_texture(NULL), m_rtt_polycount(0),
                m_billboard_quad(NULL), m_current_buffer_idx(0)
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
    m_rtt_clear_color = m_clear_color;
    m_white_texture = NULL;
    m_transparent_texture = NULL;
    m_pre_rotation_matrix = core::matrix4(core::matrix4::EM4CONST_IDENTITY);

    m_window = window;
    m_disable_wait_idle = false;
    g_schedule_pausing_rendering.store(false);
    g_paused_rendering.store(false);
    g_device_created.store(true);

#if defined(__APPLE__)
    MVKConfiguration cfg = {};
    size_t cfg_size = sizeof(MVKConfiguration);
    vkGetMoltenVKConfigurationMVK(VK_NULL_HANDLE, &cfg, &cfg_size);
    // Enable to allow binding all textures at once
    cfg.useMetalArgumentBuffers = MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS_ALWAYS;
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

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {};
    debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_create_info.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_create_info.pfnUserCallback = debug_callback;
    if (g_debug_print && vkCreateDebugUtilsMessengerEXT)
    {
        vkCreateDebugUtilsMessengerEXT(m_vk->instance, &debug_create_info,
            NULL, &m_vk->debug);
    }

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

    VmaAllocatorCreateInfo allocator_create_info = {};
    allocator_create_info.physicalDevice = m_physical_device;
    allocator_create_info.device = m_vk->device;
    allocator_create_info.instance = m_vk->instance;

#if !defined(__APPLE__) || defined(DLOPEN_MOLTENVK)
    VmaVulkanFunctions vulkan_functions = {};
    vulkan_functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkan_functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    allocator_create_info.pVulkanFunctions = &vulkan_functions;
#endif
    if (vmaCreateAllocator(&allocator_create_info, &m_vk->allocator) !=
        VK_SUCCESS)
    {
        throw std::runtime_error("vmaCreateAllocator failed");
    }

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
        createUnicolorTextures();
        GEVulkan2dRenderer::init(this);
        GEVulkanSkyBoxRenderer::init();
        m_mesh_texture_descriptor = new GEVulkanTextureDescriptor(
            GEVulkanShaderManager::getSamplerSize(),
            GEVulkanShaderManager::getMeshTextureLayer(),
            GEVulkanFeatures::supportsBindMeshTexturesAtOnce());
        GECompressorASTC4x4::init();
        GECompressorBPTCBC7::init();
        GEVulkanFeatures::printStats();
    }
    catch (std::exception& e)
    {
        destroyVulkan();
        throw std::runtime_error(std::string(
            "GEVulkanDriver constructor failed: ") + e.what());
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
    GECompressorASTC4x4::destroy();
    if (m_depth_texture)
    {
        m_depth_texture->drop();
        m_depth_texture = NULL;
    }
    if (m_rtt_texture)
    {
        m_rtt_texture->drop();
        m_rtt_texture = NULL;
    }
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
    if (m_billboard_quad)
    {
        getVulkanMeshCache()->removeMesh(m_billboard_quad);
        m_billboard_quad = NULL;
    }

    if (m_irrlicht_device->getSceneManager() &&
        m_irrlicht_device->getSceneManager()->getActiveCamera())
    {
        m_irrlicht_device->getSceneManager()->setActiveCamera(NULL);
    }

    if (m_irrlicht_device->getSceneManager() &&
        m_irrlicht_device->getSceneManager()->getMeshCache())
        getVulkanMeshCache()->destroy();
    delete m_mesh_texture_descriptor;
    GEVulkanSkyBoxRenderer::destroy();
    GEVulkan2dRenderer::destroy();
    GEVulkanShaderManager::destroy();

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

    if (vk_version < VK_API_VERSION_1_1)
    {
        uint32_t extension_count;
        vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);

        std::vector<VkExtensionProperties> instance_extensions(extension_count);
        vkEnumerateInstanceExtensionProperties(NULL, &extension_count,
            &instance_extensions[0]);
        for (auto& ext : instance_extensions)
        {
            if (strcmp(ext.extensionName,
                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0)
            {
                extensions.push_back(
                    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
                break;
            }
        }
    }

    uint32_t layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    VkInstanceCreateInfo create_info = {};
    std::vector<const char*> enabled_validation_layers;

#ifdef ENABLE_VALIDATION
    g_debug_print = true;
    for (VkLayerProperties& prop : available_layers)
    {
        if (strcmp(prop.layerName, "VK_LAYER_KHRONOS_validation") == 0)
            enabled_validation_layers.push_back("VK_LAYER_KHRONOS_validation");
    }

    VkValidationFeaturesEXT validation_features = {};
    validation_features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    validation_features.enabledValidationFeatureCount = 1;
    VkValidationFeatureEnableEXT enabled_validation_features =
        VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT;
    validation_features.pEnabledValidationFeatures = &enabled_validation_features;

    create_info.pNext = &validation_features;
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

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

    VkPhysicalDeviceProperties properties = {};
    vkGetPhysicalDeviceProperties(device, &properties);
    for (auto& ext : extensions)
    {
        if (properties.apiVersion < VK_API_VERSION_1_2 &&
            strcmp(ext.extensionName,
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME) == 0)
        {
            m_device_extensions.push_back(
                VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
        }
        else if (properties.apiVersion < VK_API_VERSION_1_1 &&
            strcmp(ext.extensionName,
            VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME) == 0)
        {
            m_device_extensions.push_back(
                VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);
        }
    }

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
    device_features.multiDrawIndirect =
        GEVulkanFeatures::supportsMultiDrawIndirect();
    device_features.drawIndirectFirstInstance =
        GEVulkanFeatures::supportsMultiDrawIndirect();

    VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features = {};
    descriptor_indexing_features.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    descriptor_indexing_features.shaderSampledImageArrayNonUniformIndexing =
        GEVulkanFeatures::supportsNonUniformIndexing();
    descriptor_indexing_features.descriptorBindingPartiallyBound =
        GEVulkanFeatures::supportsPartiallyBound();

    VkPhysicalDeviceShaderDrawParametersFeatures shader_draw = {};
    shader_draw.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
    shader_draw.shaderDrawParameters =
        GEVulkanFeatures::supportsShaderDrawParameters();
    descriptor_indexing_features.pNext = &shader_draw;

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
    create_info.pNext = &descriptor_indexing_features;

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

    const float scale = getGEConfig()->m_render_scale;
    if (scale != 1.0f)
    {
        core::dimension2du screen_size = ScreenSize;
        screen_size.Width *= scale;
        screen_size.Height *= scale;
        m_rtt_texture = new GEVulkanFBOTexture(this, screen_size,
            true/*create_depth*/);
        m_rtt_texture->createRTT();
    }
    else
    {
        m_depth_texture = new GEVulkanDepthTexture(this,
            core::dimension2du(m_swap_chain_extent.width,
            m_swap_chain_extent.height));
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
    m_vk->command_pools.resize(getMaxFrameInFlight());
    m_vk->command_buffers.resize(getMaxFrameInFlight());
    for (unsigned i = 0; i < getMaxFrameInFlight(); i++)
    {
        VkCommandPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.queueFamilyIndex = m_graphics_family;
        VkResult result = vkCreateCommandPool(m_vk->device, &pool_info,
            NULL, &m_vk->command_pools[i]);
        if (result != VK_SUCCESS)
            throw std::runtime_error("vkCreateCommandPool failed");

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = m_vk->command_pools[i];
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = 1;

        result = vkAllocateCommandBuffers(m_vk->device, &alloc_info,
            &m_vk->command_buffers[i]);
        if (result != VK_SUCCESS)
            throw std::runtime_error("vkAllocateCommandBuffers failed");
    }
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
    sampler_info.maxLod = VK_LOD_CLAMP_NONE;
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

    // GVS_SKYBOX
    sampler_info.anisotropyEnable = false;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    result = vkCreateSampler(m_vk->device, &sampler_info, NULL,
        &sampler);

    if (result != VK_SUCCESS)
        throw std::runtime_error("vkCreateSampler failed for GVS_SKYBOX");
    m_vk->samplers[GVS_SKYBOX] = sampler;

    const float max_aniso = m_properties.limits.maxSamplerAnisotropy;
    // GVS_3D_MESH_MIPMAP_2
    sampler_info.anisotropyEnable = supported_anisotropy;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.maxAnisotropy = std::min(2.0f, max_aniso);
    result = vkCreateSampler(m_vk->device, &sampler_info, NULL,
        &sampler);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(
            "vkCreateSampler failed for GVS_3D_MESH_MIPMAP_2");
    }
    m_vk->samplers[GVS_3D_MESH_MIPMAP_2] = sampler;

    // GVS_3D_MESH_MIPMAP_4
    sampler_info.maxAnisotropy = std::min(4.0f, max_aniso);
    result = vkCreateSampler(m_vk->device, &sampler_info, NULL,
        &sampler);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(
            "vkCreateSampler failed for GVS_3D_MESH_MIPMAP_4");
    }
    m_vk->samplers[GVS_3D_MESH_MIPMAP_4] = sampler;

    // GVS_3D_MESH_MIPMAP_16
    sampler_info.maxAnisotropy = std::min(16.0f, max_aniso);
    result = vkCreateSampler(m_vk->device, &sampler_info, NULL,
        &sampler);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(
            "vkCreateSampler failed for GVS_3D_MESH_MIPMAP_16");
    }
    m_vk->samplers[GVS_3D_MESH_MIPMAP_16] = sampler;

    // GVS_2D_RENDER
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    // Avoid artifacts when resizing down the screen
    sampler_info.maxLod = 0.25f;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_info.maxAnisotropy = 1.0f;
    result = vkCreateSampler(m_vk->device, &sampler_info, NULL,
        &sampler);

    if (result != VK_SUCCESS)
        throw std::runtime_error("vkCreateSampler failed for GVS_2D_RENDER");
    m_vk->samplers[GVS_2D_RENDER] = sampler;
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

    VkAttachmentDescription depth_attachment = {};
    if (m_depth_texture)
        depth_attachment.format = m_depth_texture->getInternalFormat();
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref = {};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    if (m_depth_texture)
        subpass.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    if (m_depth_texture)
        dependency.srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    if (m_depth_texture)
        dependency.dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    if (m_depth_texture)
        dependency.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::vector<VkAttachmentDescription> attachments =
        {
            color_attachment,
        };
    if (m_depth_texture)
        attachments.push_back(depth_attachment);
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
        std::vector<VkImageView> attachments =
        {
            image_views[i]
        };
        if (m_depth_texture)
        {
            attachments.push_back(
                (VkImageView)m_depth_texture->getTextureHandler());
        }
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
                                  VmaAllocationCreateInfo& alloc_create_info,
                                  VkBuffer& buffer,
                                  VmaAllocation& buffer_allocation)
{
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    return vmaCreateBuffer(m_vk->allocator,
        &buffer_info, &alloc_create_info, &buffer, &buffer_allocation, NULL) ==
        VK_SUCCESS;
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
    if (m_billboard_quad == NULL && m_irrlicht_device->getSceneManager() &&
        m_irrlicht_device->getSceneManager()->getMeshCache())
        createBillboardQuad();

    if (g_schedule_pausing_rendering.load())
    {
        pauseRendering();
        g_schedule_pausing_rendering.store(false);
    }

    if (g_paused_rendering.load() ||
        !video::CNullDriver::beginScene(backBuffer, zBuffer, color, videoData,
        sourceRect))
        return false;

    for (GEVulkanDynamicSPMBuffer* buffer : m_dynamic_spm_buffers)
        buffer->updateVertexIndexBuffer(m_current_buffer_idx);

    m_clear_color = color;
    PrimitivesDrawn = m_rtt_polycount;
    m_rtt_polycount = 0;

    if (m_rtt_texture)
    {
        draw2DImage(m_rtt_texture,core::recti(0, 0,
            ScreenSize.Width, ScreenSize.Height),
            core::recti(0, 0,
            m_rtt_texture->getSize().Width, m_rtt_texture->getSize().Height));
    }
    return true;
}   // beginScene

// ----------------------------------------------------------------------------
bool GEVulkanDriver::endScene()
{
    if (g_paused_rendering.load())
    {
        GEVulkan2dRenderer::clear();
        return false;
    }

    if (m_vk->in_flight_fences.empty() || vkWaitForFences(m_vk->device, 1,
        &m_vk->in_flight_fences[m_current_frame], VK_TRUE, 2000000000) ==
        VK_TIMEOUT)
    {
        // Attempt to restore after out focus in gnome fullscreen
        video::CNullDriver::endScene();
        GEVulkan2dRenderer::clear();
        handleDeletedTextures();
        destroySwapChainRelated(false/*handle_surface*/);
        try
        {
            createSwapChainRelated(false/*handle_surface*/);
        }
        catch (std::exception& e)
        {
            // When minimized in Windows swapchain depth buffer will fail to
            // create
            destroySwapChainRelated(false/*handle_surface*/);
        }
        return true;
    }

    VkFence fence = m_vk->in_flight_fences[m_current_frame];
    vkResetFences(m_vk->device, 1, &fence);
    vkResetCommandPool(m_vk->device, m_vk->command_pools[m_current_frame], 0);

    VkSemaphore semaphore = m_vk->image_available_semaphores[m_current_frame];
    VkResult result = vkAcquireNextImageKHR(m_vk->device, m_vk->swap_chain,
        std::numeric_limits<uint64_t>::max(), semaphore, VK_NULL_HANDLE,
        &m_image_index);

    if (result != VK_SUCCESS)
    {
        video::CNullDriver::endScene();
        GEVulkan2dRenderer::clear();
        handleDeletedTextures();
        return false;
    }

    buildCommandBuffers();

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
    result = vkQueueSubmit(queue, 1, &submit_info,
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
    m_current_buffer_idx =
        (m_current_buffer_idx + 1) % (getMaxFrameInFlight() + 1);

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
    else if (pType == irr::scene::EPT_TRIANGLE_FAN)
    {
        std::vector<uint16_t> new_idx;
        uint16_t* idx = (uint16_t*)indexList;
        for (unsigned i = 0; i < primitiveCount; i++)
        {
            new_idx.push_back(idx[0]);
            new_idx.push_back(idx[i + 1]);
            new_idx.push_back(idx[i + 2]);
        }
        S3DVertex* v = (S3DVertex*)vertices;
        GEVulkan2dRenderer::addVerticesIndices(v, vertexCount, new_idx.data(),
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
    {
        m_viewport = vp;
        if (m_irrlicht_device->getSceneManager() &&
            m_irrlicht_device->getSceneManager()->getActiveCamera())
        {
            GEVulkanCameraSceneNode* cam = static_cast<GEVulkanCameraSceneNode*>
                (m_irrlicht_device->getSceneManager()->getActiveCamera());
            cam->setViewPort(area);
        }
    }
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
void GEVulkanDriver::getRotatedViewport(VkViewport* vp, bool handle_rtt)
{
    if (handle_rtt && m_rtt_texture)
        return;

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

    // Remove any previous scheduled pause to fix sometimes android black
    // screen after resuming
    g_schedule_pausing_rendering.store(false);
}   // unpauseRendering

// ----------------------------------------------------------------------------
void GEVulkanDriver::destroySwapChainRelated(bool handle_surface)
{
    waitIdle();
    if (m_depth_texture)
    {
        m_depth_texture->drop();
        m_depth_texture = NULL;
    }
    if (m_rtt_texture)
    {
        m_rtt_texture->drop();
        m_rtt_texture = NULL;
    }
    for (VkFramebuffer& framebuffer : m_vk->swap_chain_framebuffers)
        vkDestroyFramebuffer(m_vk->device, framebuffer, NULL);
    m_vk->swap_chain_framebuffers.clear();
    if (m_vk->render_pass != VK_NULL_HANDLE)
        vkDestroyRenderPass(m_vk->device, m_vk->render_pass, NULL);
    m_vk->render_pass = VK_NULL_HANDLE;
    for (VkSemaphore& semaphore : m_vk->image_available_semaphores)
        vkDestroySemaphore(m_vk->device, semaphore, NULL);
    m_vk->image_available_semaphores.clear();
    for (VkSemaphore& semaphore : m_vk->render_finished_semaphores)
        vkDestroySemaphore(m_vk->device, semaphore, NULL);
    m_vk->render_finished_semaphores.clear();
    for (VkFence& fence : m_vk->in_flight_fences)
        vkDestroyFence(m_vk->device, fence, NULL);
    m_vk->in_flight_fences.clear();
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
    waitIdle();
    if (handle_surface)
    {
        if (SDL_Vulkan_CreateSurface(m_window, m_vk->instance, &m_vk->surface) == SDL_FALSE)
            throw std::runtime_error("SDL_Vulkan_CreateSurface failed");
    }
    updateSurfaceInformation(m_physical_device, &m_surface_capabilities,
        &m_surface_formats, &m_present_modes);
    createSwapChain();
    createSyncObjects();
    createRenderPass();
    createFramebuffers();
}   // createSwapChainRelated

// ----------------------------------------------------------------------------
void GEVulkanDriver::waitIdle(bool flush_command_loader)
{
    if (m_disable_wait_idle)
        return;
    // Host access to all VkQueue objects created from device must be externally synchronized
    for (std::mutex* m : m_graphics_queue_mutexes)
        m->lock();
    vkDeviceWaitIdle(m_vk->device);
    for (std::mutex* m : m_graphics_queue_mutexes)
        m->unlock();

    if (flush_command_loader)
        GEVulkanCommandLoader::waitIdle();
}   // waitIdle

// ----------------------------------------------------------------------------
GEVulkanMeshCache* GEVulkanDriver::getVulkanMeshCache() const
{
    return static_cast<GEVulkanMeshCache*>
        (m_irrlicht_device->getSceneManager()->getMeshCache());
}   // getVulkanMeshCache

// ----------------------------------------------------------------------------
void GEVulkanDriver::buildCommandBuffers()
{
    std::array<VkClearValue, 2> clear_values = {};
    video::SColorf cf(getClearColor());
    clear_values[0].color =
    {
        cf.getRed(), cf.getGreen(), cf.getBlue(), cf.getAlpha()
    };
    clear_values[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.clearValueCount = (uint32_t)(clear_values.size());
    render_pass_info.pClearValues = &clear_values[0];
    render_pass_info.renderArea.offset = {0, 0};

    if (m_rtt_texture)
    {
        render_pass_info.renderPass = m_rtt_texture->getRTTRenderPass();
        render_pass_info.framebuffer = m_rtt_texture->getRTTFramebuffer();
        render_pass_info.renderArea.extent = { m_rtt_texture->getSize().Width,
            m_rtt_texture->getSize().Height };
    }
    else
    {
        render_pass_info.renderPass = getRenderPass();
        render_pass_info.framebuffer =
            getSwapChainFramebuffers()[getCurrentImageIndex()];
        render_pass_info.renderArea.extent = getSwapChainExtent();
    }

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkResult result = vkBeginCommandBuffer(getCurrentCommandBuffer(),
        &begin_info);
    if (result != VK_SUCCESS)
        return;

    GEVulkan2dRenderer::uploadTrisBuffers();
    for (auto& p : static_cast<GEVulkanSceneManager*>(
        m_irrlicht_device->getSceneManager())->getDrawCalls())
    {
        p.second->uploadDynamicData(this, p.first);
    }

    vkCmdBeginRenderPass(getCurrentCommandBuffer(), &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE);

    for (auto& p : static_cast<GEVulkanSceneManager*>(
        m_irrlicht_device->getSceneManager())->getDrawCalls())
    {
        p.second->render(this, p.first);
        PrimitivesDrawn += p.second->getPolyCount();
    }

    if (m_rtt_texture)
    {
        vkCmdEndRenderPass(getCurrentCommandBuffer());
        // No depth buffer in main framebuffer if RTT is used
        render_pass_info.clearValueCount = 1;
        render_pass_info.renderPass = getRenderPass();
        render_pass_info.framebuffer =
            getSwapChainFramebuffers()[getCurrentImageIndex()];
        render_pass_info.renderArea.extent = getSwapChainExtent();
        vkCmdBeginRenderPass(getCurrentCommandBuffer(), &render_pass_info,
            VK_SUBPASS_CONTENTS_INLINE);
    }

    GEVulkan2dRenderer::render();

    vkCmdEndRenderPass(getCurrentCommandBuffer());
    vkEndCommandBuffer(getCurrentCommandBuffer());

    handleDeletedTextures();
}   // buildCommandBuffers

// ----------------------------------------------------------------------------
VkFormat GEVulkanDriver::findSupportedFormat(const std::vector<VkFormat>& candidates,
                                             VkImageTiling tiling,
                                             VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_physical_device, format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features)
            return format;
        else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
            (props.optimalTilingFeatures & features) == features)
            return format;
    }
    throw std::runtime_error("failed to find supported format!");
}   // findSupportedFormat

// ----------------------------------------------------------------------------
void GEVulkanDriver::handleDeletedTextures()
{
    GEVulkan2dRenderer::handleDeletedTextures();
    m_mesh_texture_descriptor->handleDeletedTextures();
}   // handleDeletedTextures

// ----------------------------------------------------------------------------
ITexture* GEVulkanDriver::addRenderTargetTexture(const core::dimension2d<u32>& size,
    const io::path& name, const ECOLOR_FORMAT format,
    const bool useStencil)
{
    GEVulkanFBOTexture* rtt = new GEVulkanFBOTexture(this, size,
        true/*create_depth*/);
    rtt->createRTT();
    return rtt;
}   // addRenderTargetTexture

// ----------------------------------------------------------------------------
bool GEVulkanDriver::setRenderTarget(video::ITexture* texture,
                                     bool clearBackBuffer, bool clearZBuffer,
                                     SColor color)
{
    GEVulkanFBOTexture* new_rtt = dynamic_cast<GEVulkanFBOTexture*>(texture);
    if (m_separate_rtt_texture == new_rtt)
        return true;
    m_separate_rtt_texture = new_rtt;
    if (m_separate_rtt_texture)
    {
        m_rtt_clear_color = color;
        m_prev_rtt_texture = m_rtt_texture;
        m_rtt_texture = m_separate_rtt_texture;
    }
    else
    {
        m_rtt_texture = m_prev_rtt_texture;
        m_prev_rtt_texture = NULL;
    }
    return true;
}   // setRenderTarget

// ----------------------------------------------------------------------------
void GEVulkanDriver::updateDriver(bool reload_shaders)
{
    waitIdle();
    setDisableWaitIdle(true);
    clearDrawCallsCache();
    destroySwapChainRelated(false/*handle_surface*/);
    if (reload_shaders)
    {
        GEVulkanShaderManager::destroy();
        GEVulkanShaderManager::init(this);
        delete m_mesh_texture_descriptor;
        m_mesh_texture_descriptor = new GEVulkanTextureDescriptor(
            GEVulkanShaderManager::getSamplerSize(),
            GEVulkanShaderManager::getMeshTextureLayer(),
            GEVulkanFeatures::supportsBindMeshTexturesAtOnce());
    }
    createSwapChainRelated(false/*handle_surface*/);
    for (auto& dc : static_cast<GEVulkanSceneManager*>(
        m_irrlicht_device->getSceneManager())->getDrawCalls())
        dc.second = std::unique_ptr<GEVulkanDrawCall>(new GEVulkanDrawCall);
    GEVulkanSkyBoxRenderer::destroy();
    GEVulkan2dRenderer::destroy();
    GEVulkan2dRenderer::init(this);
    setDisableWaitIdle(false);
}   // updateDriver

// ----------------------------------------------------------------------------
void GEVulkanDriver::clearDrawCallsCache()
{
    m_draw_calls_cache.clear();
}   // clearDrawCallsCache

// ----------------------------------------------------------------------------
void GEVulkanDriver::addDrawCallToCache(std::unique_ptr<GEVulkanDrawCall>& dc)
{
    if (getGEConfig()->m_enable_draw_call_cache)
        m_draw_calls_cache.push_back(std::move(dc));
}   // addDrawCallToCache

// ----------------------------------------------------------------------------
std::unique_ptr<GEVulkanDrawCall> GEVulkanDriver::getDrawCallFromCache()
{
    if (!getGEConfig()->m_enable_draw_call_cache || m_draw_calls_cache.empty())
        return std::unique_ptr<GEVulkanDrawCall>(new GEVulkanDrawCall);
    auto dc = std::move(m_draw_calls_cache.back());
    m_draw_calls_cache.pop_back();
    return dc;
}   // getDrawCallFromCache

// ----------------------------------------------------------------------------
void GEVulkanDriver::createBillboardQuad()
{
    m_billboard_quad = new GESPM();
    GESPMBuffer* buffer = new GESPMBuffer();
    /* Vertices are:
    (-1, 1, 0) 2--1 (1, 1, 0)
               |\ |
               | \|
   (-1, -1, 0) 3--0 (1, -1, 0)
               */
    short one_hf = 15360;
    video::S3DVertexSkinnedMesh sp;
    sp.m_position = core::vector3df(1, -1, 0);
    sp.m_normal = MiniGLM::compressVector3(core::vector3df(0, 0, 1));
    sp.m_color = video::SColor((uint32_t)-1);
    sp.m_all_uvs[0] = 15360;
    sp.m_all_uvs[1] = 15360;
    buffer->getVerticesVector().push_back(sp);

    sp.m_position = core::vector3df(1, 1, 0);
    sp.m_all_uvs[0] = one_hf;
    sp.m_all_uvs[1] = 0;
    buffer->getVerticesVector().push_back(sp);

    sp.m_position = core::vector3df(-1, 1, 0);
    sp.m_all_uvs[0] = 0;
    sp.m_all_uvs[1] = 0;
    buffer->getVerticesVector().push_back(sp);

    sp.m_position = core::vector3df(-1, -1, 0);
    sp.m_all_uvs[0] = 0;
    sp.m_all_uvs[1] = one_hf;
    buffer->getVerticesVector().push_back(sp);

    buffer->getIndicesVector().push_back(2);
    buffer->getIndicesVector().push_back(1);
    buffer->getIndicesVector().push_back(0);
    buffer->getIndicesVector().push_back(2);
    buffer->getIndicesVector().push_back(0);
    buffer->getIndicesVector().push_back(3);
    buffer->recalculateBoundingBox();
    m_billboard_quad->addMeshBuffer(buffer);
    m_billboard_quad->finalize();

    std::stringstream oss;
    oss << (uint64_t)m_billboard_quad;
    getVulkanMeshCache()->addMesh(oss.str().c_str(), m_billboard_quad);
    m_billboard_quad->drop();
}   // createBillboardQuad

}

namespace irr
{
namespace video
{
    IVideoDriver* createVulkanDriver(const SIrrlichtCreationParameters& params,
                                     io::IFileSystem* io, SDL_Window* window,
                                     IrrlichtDevice* device)
    {
        return new GE::GEVulkanDriver(params, io, window, device);
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
