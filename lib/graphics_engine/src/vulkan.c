/**
 * SPDX-License-Identifier: (WTFPL OR CC0-1.0) AND Apache-2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/vulkan.h>

#ifndef GLAD_IMPL_UTIL_C_
#define GLAD_IMPL_UTIL_C_

#ifdef _MSC_VER
#define GLAD_IMPL_UTIL_SSCANF sscanf_s
#else
#define GLAD_IMPL_UTIL_SSCANF sscanf
#endif

#endif /* GLAD_IMPL_UTIL_C_ */

#ifdef __cplusplus
extern "C" {
#endif



int GLAD_VK_VERSION_1_0 = 0;
int GLAD_VK_VERSION_1_1 = 0;
int GLAD_VK_VERSION_1_2 = 0;
int GLAD_VK_VERSION_1_3 = 0;
int GLAD_VK_VERSION_1_4 = 0;
#if defined(VK_ENABLE_BETA_EXTENSIONS)
int GLAD_VK_AMDX_shader_enqueue = 0;

#endif
int GLAD_VK_AMD_anti_lag = 0;
int GLAD_VK_AMD_buffer_marker = 0;
int GLAD_VK_AMD_device_coherent_memory = 0;
int GLAD_VK_AMD_display_native_hdr = 0;
int GLAD_VK_AMD_draw_indirect_count = 0;
int GLAD_VK_AMD_gcn_shader = 0;
int GLAD_VK_AMD_gpu_shader_half_float = 0;
int GLAD_VK_AMD_gpu_shader_int16 = 0;
int GLAD_VK_AMD_memory_overallocation_behavior = 0;
int GLAD_VK_AMD_mixed_attachment_samples = 0;
int GLAD_VK_AMD_negative_viewport_height = 0;
int GLAD_VK_AMD_pipeline_compiler_control = 0;
int GLAD_VK_AMD_rasterization_order = 0;
int GLAD_VK_AMD_shader_ballot = 0;
int GLAD_VK_AMD_shader_core_properties = 0;
int GLAD_VK_AMD_shader_core_properties2 = 0;
int GLAD_VK_AMD_shader_early_and_late_fragment_tests = 0;
int GLAD_VK_AMD_shader_explicit_vertex_parameter = 0;
int GLAD_VK_AMD_shader_fragment_mask = 0;
int GLAD_VK_AMD_shader_image_load_store_lod = 0;
int GLAD_VK_AMD_shader_info = 0;
int GLAD_VK_AMD_shader_trinary_minmax = 0;
int GLAD_VK_AMD_texture_gather_bias_lod = 0;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
int GLAD_VK_ANDROID_external_format_resolve = 0;

#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
int GLAD_VK_ANDROID_external_memory_android_hardware_buffer = 0;

#endif
int GLAD_VK_ARM_pipeline_opacity_micromap = 0;
int GLAD_VK_ARM_rasterization_order_attachment_access = 0;
int GLAD_VK_ARM_render_pass_striped = 0;
int GLAD_VK_ARM_scheduling_controls = 0;
int GLAD_VK_ARM_shader_core_builtins = 0;
int GLAD_VK_ARM_shader_core_properties = 0;
int GLAD_VK_EXT_4444_formats = 0;
int GLAD_VK_EXT_acquire_drm_display = 0;
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
int GLAD_VK_EXT_acquire_xlib_display = 0;

#endif
int GLAD_VK_EXT_astc_decode_mode = 0;
int GLAD_VK_EXT_attachment_feedback_loop_dynamic_state = 0;
int GLAD_VK_EXT_attachment_feedback_loop_layout = 0;
int GLAD_VK_EXT_blend_operation_advanced = 0;
int GLAD_VK_EXT_border_color_swizzle = 0;
int GLAD_VK_EXT_buffer_device_address = 0;
int GLAD_VK_EXT_calibrated_timestamps = 0;
int GLAD_VK_EXT_color_write_enable = 0;
int GLAD_VK_EXT_conditional_rendering = 0;
int GLAD_VK_EXT_conservative_rasterization = 0;
int GLAD_VK_EXT_custom_border_color = 0;
int GLAD_VK_EXT_debug_marker = 0;
int GLAD_VK_EXT_debug_report = 0;
int GLAD_VK_EXT_debug_utils = 0;
int GLAD_VK_EXT_depth_bias_control = 0;
int GLAD_VK_EXT_depth_clamp_control = 0;
int GLAD_VK_EXT_depth_clamp_zero_one = 0;
int GLAD_VK_EXT_depth_clip_control = 0;
int GLAD_VK_EXT_depth_clip_enable = 0;
int GLAD_VK_EXT_depth_range_unrestricted = 0;
int GLAD_VK_EXT_descriptor_buffer = 0;
int GLAD_VK_EXT_descriptor_indexing = 0;
int GLAD_VK_EXT_device_address_binding_report = 0;
int GLAD_VK_EXT_device_fault = 0;
int GLAD_VK_EXT_device_generated_commands = 0;
int GLAD_VK_EXT_device_memory_report = 0;
int GLAD_VK_EXT_direct_mode_display = 0;
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
int GLAD_VK_EXT_directfb_surface = 0;

#endif
int GLAD_VK_EXT_discard_rectangles = 0;
int GLAD_VK_EXT_display_control = 0;
int GLAD_VK_EXT_display_surface_counter = 0;
int GLAD_VK_EXT_dynamic_rendering_unused_attachments = 0;
int GLAD_VK_EXT_extended_dynamic_state = 0;
int GLAD_VK_EXT_extended_dynamic_state2 = 0;
int GLAD_VK_EXT_extended_dynamic_state3 = 0;
int GLAD_VK_EXT_external_memory_acquire_unmodified = 0;
int GLAD_VK_EXT_external_memory_dma_buf = 0;
int GLAD_VK_EXT_external_memory_host = 0;
#if defined(VK_USE_PLATFORM_METAL_EXT)
int GLAD_VK_EXT_external_memory_metal = 0;

#endif
int GLAD_VK_EXT_filter_cubic = 0;
int GLAD_VK_EXT_fragment_density_map = 0;
int GLAD_VK_EXT_fragment_density_map2 = 0;
int GLAD_VK_EXT_fragment_density_map_offset = 0;
int GLAD_VK_EXT_fragment_shader_interlock = 0;
int GLAD_VK_EXT_frame_boundary = 0;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
int GLAD_VK_EXT_full_screen_exclusive = 0;

#endif
int GLAD_VK_EXT_global_priority = 0;
int GLAD_VK_EXT_global_priority_query = 0;
int GLAD_VK_EXT_graphics_pipeline_library = 0;
int GLAD_VK_EXT_hdr_metadata = 0;
int GLAD_VK_EXT_headless_surface = 0;
int GLAD_VK_EXT_host_image_copy = 0;
int GLAD_VK_EXT_host_query_reset = 0;
int GLAD_VK_EXT_image_2d_view_of_3d = 0;
int GLAD_VK_EXT_image_compression_control = 0;
int GLAD_VK_EXT_image_compression_control_swapchain = 0;
int GLAD_VK_EXT_image_drm_format_modifier = 0;
int GLAD_VK_EXT_image_robustness = 0;
int GLAD_VK_EXT_image_sliced_view_of_3d = 0;
int GLAD_VK_EXT_image_view_min_lod = 0;
int GLAD_VK_EXT_index_type_uint8 = 0;
int GLAD_VK_EXT_inline_uniform_block = 0;
int GLAD_VK_EXT_layer_settings = 0;
int GLAD_VK_EXT_legacy_dithering = 0;
int GLAD_VK_EXT_legacy_vertex_attributes = 0;
int GLAD_VK_EXT_line_rasterization = 0;
int GLAD_VK_EXT_load_store_op_none = 0;
int GLAD_VK_EXT_map_memory_placed = 0;
int GLAD_VK_EXT_memory_budget = 0;
int GLAD_VK_EXT_memory_priority = 0;
int GLAD_VK_EXT_mesh_shader = 0;
#if defined(VK_USE_PLATFORM_METAL_EXT)
int GLAD_VK_EXT_metal_objects = 0;

#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
int GLAD_VK_EXT_metal_surface = 0;

#endif
int GLAD_VK_EXT_multi_draw = 0;
int GLAD_VK_EXT_multisampled_render_to_single_sampled = 0;
int GLAD_VK_EXT_mutable_descriptor_type = 0;
int GLAD_VK_EXT_nested_command_buffer = 0;
int GLAD_VK_EXT_non_seamless_cube_map = 0;
int GLAD_VK_EXT_opacity_micromap = 0;
int GLAD_VK_EXT_pageable_device_local_memory = 0;
int GLAD_VK_EXT_pci_bus_info = 0;
int GLAD_VK_EXT_physical_device_drm = 0;
int GLAD_VK_EXT_pipeline_creation_cache_control = 0;
int GLAD_VK_EXT_pipeline_creation_feedback = 0;
int GLAD_VK_EXT_pipeline_library_group_handles = 0;
int GLAD_VK_EXT_pipeline_properties = 0;
int GLAD_VK_EXT_pipeline_protected_access = 0;
int GLAD_VK_EXT_pipeline_robustness = 0;
int GLAD_VK_EXT_post_depth_coverage = 0;
int GLAD_VK_EXT_present_mode_fifo_latest_ready = 0;
int GLAD_VK_EXT_primitive_topology_list_restart = 0;
int GLAD_VK_EXT_primitives_generated_query = 0;
int GLAD_VK_EXT_private_data = 0;
int GLAD_VK_EXT_provoking_vertex = 0;
int GLAD_VK_EXT_queue_family_foreign = 0;
int GLAD_VK_EXT_rasterization_order_attachment_access = 0;
int GLAD_VK_EXT_rgba10x6_formats = 0;
int GLAD_VK_EXT_robustness2 = 0;
int GLAD_VK_EXT_sample_locations = 0;
int GLAD_VK_EXT_sampler_filter_minmax = 0;
int GLAD_VK_EXT_scalar_block_layout = 0;
int GLAD_VK_EXT_separate_stencil_usage = 0;
int GLAD_VK_EXT_shader_atomic_float = 0;
int GLAD_VK_EXT_shader_atomic_float2 = 0;
int GLAD_VK_EXT_shader_demote_to_helper_invocation = 0;
int GLAD_VK_EXT_shader_image_atomic_int64 = 0;
int GLAD_VK_EXT_shader_module_identifier = 0;
int GLAD_VK_EXT_shader_object = 0;
int GLAD_VK_EXT_shader_replicated_composites = 0;
int GLAD_VK_EXT_shader_stencil_export = 0;
int GLAD_VK_EXT_shader_subgroup_ballot = 0;
int GLAD_VK_EXT_shader_subgroup_vote = 0;
int GLAD_VK_EXT_shader_tile_image = 0;
int GLAD_VK_EXT_shader_viewport_index_layer = 0;
int GLAD_VK_EXT_subgroup_size_control = 0;
int GLAD_VK_EXT_subpass_merge_feedback = 0;
int GLAD_VK_EXT_surface_maintenance1 = 0;
int GLAD_VK_EXT_swapchain_colorspace = 0;
int GLAD_VK_EXT_swapchain_maintenance1 = 0;
int GLAD_VK_EXT_texel_buffer_alignment = 0;
int GLAD_VK_EXT_texture_compression_astc_hdr = 0;
int GLAD_VK_EXT_tooling_info = 0;
int GLAD_VK_EXT_transform_feedback = 0;
int GLAD_VK_EXT_validation_cache = 0;
int GLAD_VK_EXT_validation_features = 0;
int GLAD_VK_EXT_validation_flags = 0;
int GLAD_VK_EXT_vertex_attribute_divisor = 0;
int GLAD_VK_EXT_vertex_attribute_robustness = 0;
int GLAD_VK_EXT_vertex_input_dynamic_state = 0;
int GLAD_VK_EXT_ycbcr_2plane_444_formats = 0;
int GLAD_VK_EXT_ycbcr_image_arrays = 0;
#if defined(VK_USE_PLATFORM_FUCHSIA)
int GLAD_VK_FUCHSIA_buffer_collection = 0;

#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
int GLAD_VK_FUCHSIA_external_memory = 0;

#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
int GLAD_VK_FUCHSIA_external_semaphore = 0;

#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
int GLAD_VK_FUCHSIA_imagepipe_surface = 0;

#endif
#if defined(VK_USE_PLATFORM_GGP)
int GLAD_VK_GGP_frame_token = 0;

#endif
#if defined(VK_USE_PLATFORM_GGP)
int GLAD_VK_GGP_stream_descriptor_surface = 0;

#endif
int GLAD_VK_GOOGLE_decorate_string = 0;
int GLAD_VK_GOOGLE_display_timing = 0;
int GLAD_VK_GOOGLE_hlsl_functionality1 = 0;
int GLAD_VK_GOOGLE_surfaceless_query = 0;
int GLAD_VK_GOOGLE_user_type = 0;
int GLAD_VK_HUAWEI_cluster_culling_shader = 0;
int GLAD_VK_HUAWEI_hdr_vivid = 0;
int GLAD_VK_HUAWEI_invocation_mask = 0;
int GLAD_VK_HUAWEI_subpass_shading = 0;
int GLAD_VK_IMG_filter_cubic = 0;
int GLAD_VK_IMG_format_pvrtc = 0;
int GLAD_VK_IMG_relaxed_line_rasterization = 0;
int GLAD_VK_INTEL_performance_query = 0;
int GLAD_VK_INTEL_shader_integer_functions2 = 0;
int GLAD_VK_KHR_16bit_storage = 0;
int GLAD_VK_KHR_8bit_storage = 0;
int GLAD_VK_KHR_acceleration_structure = 0;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
int GLAD_VK_KHR_android_surface = 0;

#endif
int GLAD_VK_KHR_bind_memory2 = 0;
int GLAD_VK_KHR_buffer_device_address = 0;
int GLAD_VK_KHR_calibrated_timestamps = 0;
int GLAD_VK_KHR_compute_shader_derivatives = 0;
int GLAD_VK_KHR_cooperative_matrix = 0;
int GLAD_VK_KHR_copy_commands2 = 0;
int GLAD_VK_KHR_create_renderpass2 = 0;
int GLAD_VK_KHR_dedicated_allocation = 0;
int GLAD_VK_KHR_deferred_host_operations = 0;
int GLAD_VK_KHR_depth_clamp_zero_one = 0;
int GLAD_VK_KHR_depth_stencil_resolve = 0;
int GLAD_VK_KHR_descriptor_update_template = 0;
int GLAD_VK_KHR_device_group = 0;
int GLAD_VK_KHR_device_group_creation = 0;
int GLAD_VK_KHR_display = 0;
int GLAD_VK_KHR_display_swapchain = 0;
int GLAD_VK_KHR_draw_indirect_count = 0;
int GLAD_VK_KHR_driver_properties = 0;
int GLAD_VK_KHR_dynamic_rendering = 0;
int GLAD_VK_KHR_dynamic_rendering_local_read = 0;
int GLAD_VK_KHR_external_fence = 0;
int GLAD_VK_KHR_external_fence_capabilities = 0;
int GLAD_VK_KHR_external_fence_fd = 0;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
int GLAD_VK_KHR_external_fence_win32 = 0;

#endif
int GLAD_VK_KHR_external_memory = 0;
int GLAD_VK_KHR_external_memory_capabilities = 0;
int GLAD_VK_KHR_external_memory_fd = 0;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
int GLAD_VK_KHR_external_memory_win32 = 0;

#endif
int GLAD_VK_KHR_external_semaphore = 0;
int GLAD_VK_KHR_external_semaphore_capabilities = 0;
int GLAD_VK_KHR_external_semaphore_fd = 0;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
int GLAD_VK_KHR_external_semaphore_win32 = 0;

#endif
int GLAD_VK_KHR_format_feature_flags2 = 0;
int GLAD_VK_KHR_fragment_shader_barycentric = 0;
int GLAD_VK_KHR_fragment_shading_rate = 0;
int GLAD_VK_KHR_get_display_properties2 = 0;
int GLAD_VK_KHR_get_memory_requirements2 = 0;
int GLAD_VK_KHR_get_physical_device_properties2 = 0;
int GLAD_VK_KHR_get_surface_capabilities2 = 0;
int GLAD_VK_KHR_global_priority = 0;
int GLAD_VK_KHR_image_format_list = 0;
int GLAD_VK_KHR_imageless_framebuffer = 0;
int GLAD_VK_KHR_incremental_present = 0;
int GLAD_VK_KHR_index_type_uint8 = 0;
int GLAD_VK_KHR_line_rasterization = 0;
int GLAD_VK_KHR_load_store_op_none = 0;
int GLAD_VK_KHR_maintenance1 = 0;
int GLAD_VK_KHR_maintenance2 = 0;
int GLAD_VK_KHR_maintenance3 = 0;
int GLAD_VK_KHR_maintenance4 = 0;
int GLAD_VK_KHR_maintenance5 = 0;
int GLAD_VK_KHR_maintenance6 = 0;
int GLAD_VK_KHR_maintenance7 = 0;
int GLAD_VK_KHR_maintenance8 = 0;
int GLAD_VK_KHR_map_memory2 = 0;
int GLAD_VK_KHR_multiview = 0;
int GLAD_VK_KHR_performance_query = 0;
int GLAD_VK_KHR_pipeline_binary = 0;
int GLAD_VK_KHR_pipeline_executable_properties = 0;
int GLAD_VK_KHR_pipeline_library = 0;
int GLAD_VK_KHR_portability_enumeration = 0;
#if defined(VK_ENABLE_BETA_EXTENSIONS)
int GLAD_VK_KHR_portability_subset = 0;

#endif
int GLAD_VK_KHR_present_id = 0;
int GLAD_VK_KHR_present_wait = 0;
int GLAD_VK_KHR_push_descriptor = 0;
int GLAD_VK_KHR_ray_query = 0;
int GLAD_VK_KHR_ray_tracing_maintenance1 = 0;
int GLAD_VK_KHR_ray_tracing_pipeline = 0;
int GLAD_VK_KHR_ray_tracing_position_fetch = 0;
int GLAD_VK_KHR_relaxed_block_layout = 0;
int GLAD_VK_KHR_robustness2 = 0;
int GLAD_VK_KHR_sampler_mirror_clamp_to_edge = 0;
int GLAD_VK_KHR_sampler_ycbcr_conversion = 0;
int GLAD_VK_KHR_separate_depth_stencil_layouts = 0;
int GLAD_VK_KHR_shader_atomic_int64 = 0;
int GLAD_VK_KHR_shader_bfloat16 = 0;
int GLAD_VK_KHR_shader_clock = 0;
int GLAD_VK_KHR_shader_draw_parameters = 0;
int GLAD_VK_KHR_shader_expect_assume = 0;
int GLAD_VK_KHR_shader_float16_int8 = 0;
int GLAD_VK_KHR_shader_float_controls = 0;
int GLAD_VK_KHR_shader_float_controls2 = 0;
int GLAD_VK_KHR_shader_integer_dot_product = 0;
int GLAD_VK_KHR_shader_maximal_reconvergence = 0;
int GLAD_VK_KHR_shader_non_semantic_info = 0;
int GLAD_VK_KHR_shader_quad_control = 0;
int GLAD_VK_KHR_shader_relaxed_extended_instruction = 0;
int GLAD_VK_KHR_shader_subgroup_extended_types = 0;
int GLAD_VK_KHR_shader_subgroup_rotate = 0;
int GLAD_VK_KHR_shader_subgroup_uniform_control_flow = 0;
int GLAD_VK_KHR_shader_terminate_invocation = 0;
int GLAD_VK_KHR_shared_presentable_image = 0;
int GLAD_VK_KHR_spirv_1_4 = 0;
int GLAD_VK_KHR_storage_buffer_storage_class = 0;
int GLAD_VK_KHR_surface = 0;
int GLAD_VK_KHR_surface_protected_capabilities = 0;
int GLAD_VK_KHR_swapchain = 0;
int GLAD_VK_KHR_swapchain_mutable_format = 0;
int GLAD_VK_KHR_synchronization2 = 0;
int GLAD_VK_KHR_timeline_semaphore = 0;
int GLAD_VK_KHR_uniform_buffer_standard_layout = 0;
int GLAD_VK_KHR_variable_pointers = 0;
int GLAD_VK_KHR_vertex_attribute_divisor = 0;
int GLAD_VK_KHR_vulkan_memory_model = 0;
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
int GLAD_VK_KHR_wayland_surface = 0;

#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
int GLAD_VK_KHR_win32_keyed_mutex = 0;

#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
int GLAD_VK_KHR_win32_surface = 0;

#endif
int GLAD_VK_KHR_workgroup_memory_explicit_layout = 0;
#if defined(VK_USE_PLATFORM_XCB_KHR)
int GLAD_VK_KHR_xcb_surface = 0;

#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
int GLAD_VK_KHR_xlib_surface = 0;

#endif
int GLAD_VK_KHR_zero_initialize_workgroup_memory = 0;
int GLAD_VK_LUNARG_direct_driver_loading = 0;
int GLAD_VK_MESA_image_alignment_control = 0;
int GLAD_VK_MSFT_layered_driver = 0;
#if defined(VK_USE_PLATFORM_IOS_MVK)
int GLAD_VK_MVK_ios_surface = 0;

#endif
#if defined(VK_USE_PLATFORM_MACOS_MVK)
int GLAD_VK_MVK_macos_surface = 0;

#endif
#if defined(VK_USE_PLATFORM_VI_NN)
int GLAD_VK_NN_vi_surface = 0;

#endif
int GLAD_VK_NVX_binary_import = 0;
int GLAD_VK_NVX_image_view_handle = 0;
int GLAD_VK_NVX_multiview_per_view_attributes = 0;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
int GLAD_VK_NV_acquire_winrt_display = 0;

#endif
int GLAD_VK_NV_clip_space_w_scaling = 0;
int GLAD_VK_NV_cluster_acceleration_structure = 0;
int GLAD_VK_NV_command_buffer_inheritance = 0;
int GLAD_VK_NV_compute_shader_derivatives = 0;
int GLAD_VK_NV_cooperative_matrix = 0;
int GLAD_VK_NV_cooperative_matrix2 = 0;
int GLAD_VK_NV_cooperative_vector = 0;
int GLAD_VK_NV_copy_memory_indirect = 0;
int GLAD_VK_NV_corner_sampled_image = 0;
int GLAD_VK_NV_coverage_reduction_mode = 0;
#if defined(VK_ENABLE_BETA_EXTENSIONS)
int GLAD_VK_NV_cuda_kernel_launch = 0;

#endif
int GLAD_VK_NV_dedicated_allocation = 0;
int GLAD_VK_NV_dedicated_allocation_image_aliasing = 0;
int GLAD_VK_NV_descriptor_pool_overallocation = 0;
int GLAD_VK_NV_device_diagnostic_checkpoints = 0;
int GLAD_VK_NV_device_diagnostics_config = 0;
int GLAD_VK_NV_device_generated_commands = 0;
int GLAD_VK_NV_device_generated_commands_compute = 0;
#if defined(VK_ENABLE_BETA_EXTENSIONS)
int GLAD_VK_NV_displacement_micromap = 0;

#endif
int GLAD_VK_NV_display_stereo = 0;
int GLAD_VK_NV_extended_sparse_address_space = 0;
int GLAD_VK_NV_external_compute_queue = 0;
int GLAD_VK_NV_external_memory = 0;
int GLAD_VK_NV_external_memory_capabilities = 0;
int GLAD_VK_NV_external_memory_rdma = 0;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
int GLAD_VK_NV_external_memory_win32 = 0;

#endif
int GLAD_VK_NV_fill_rectangle = 0;
int GLAD_VK_NV_fragment_coverage_to_color = 0;
int GLAD_VK_NV_fragment_shader_barycentric = 0;
int GLAD_VK_NV_fragment_shading_rate_enums = 0;
int GLAD_VK_NV_framebuffer_mixed_samples = 0;
int GLAD_VK_NV_geometry_shader_passthrough = 0;
int GLAD_VK_NV_glsl_shader = 0;
int GLAD_VK_NV_inherited_viewport_scissor = 0;
int GLAD_VK_NV_linear_color_attachment = 0;
int GLAD_VK_NV_low_latency = 0;
int GLAD_VK_NV_low_latency2 = 0;
int GLAD_VK_NV_memory_decompression = 0;
int GLAD_VK_NV_mesh_shader = 0;
int GLAD_VK_NV_optical_flow = 0;
int GLAD_VK_NV_partitioned_acceleration_structure = 0;
int GLAD_VK_NV_per_stage_descriptor_set = 0;
int GLAD_VK_NV_present_barrier = 0;
#if defined(VK_ENABLE_BETA_EXTENSIONS)
int GLAD_VK_NV_present_metering = 0;

#endif
int GLAD_VK_NV_raw_access_chains = 0;
int GLAD_VK_NV_ray_tracing = 0;
int GLAD_VK_NV_ray_tracing_invocation_reorder = 0;
int GLAD_VK_NV_ray_tracing_linear_swept_spheres = 0;
int GLAD_VK_NV_ray_tracing_motion_blur = 0;
int GLAD_VK_NV_ray_tracing_validation = 0;
int GLAD_VK_NV_representative_fragment_test = 0;
int GLAD_VK_NV_sample_mask_override_coverage = 0;
int GLAD_VK_NV_scissor_exclusive = 0;
int GLAD_VK_NV_shader_atomic_float16_vector = 0;
int GLAD_VK_NV_shader_image_footprint = 0;
int GLAD_VK_NV_shader_sm_builtins = 0;
int GLAD_VK_NV_shader_subgroup_partitioned = 0;
int GLAD_VK_NV_shading_rate_image = 0;
int GLAD_VK_NV_viewport_array2 = 0;
int GLAD_VK_NV_viewport_swizzle = 0;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
int GLAD_VK_NV_win32_keyed_mutex = 0;

#endif
int GLAD_VK_QCOM_filter_cubic_clamp = 0;
int GLAD_VK_QCOM_filter_cubic_weights = 0;
int GLAD_VK_QCOM_fragment_density_map_offset = 0;
int GLAD_VK_QCOM_image_processing = 0;
int GLAD_VK_QCOM_image_processing2 = 0;
int GLAD_VK_QCOM_multiview_per_view_render_areas = 0;
int GLAD_VK_QCOM_multiview_per_view_viewports = 0;
int GLAD_VK_QCOM_render_pass_shader_resolve = 0;
int GLAD_VK_QCOM_render_pass_store_ops = 0;
int GLAD_VK_QCOM_render_pass_transform = 0;
int GLAD_VK_QCOM_rotated_copy_commands = 0;
int GLAD_VK_QCOM_tile_memory_heap = 0;
int GLAD_VK_QCOM_tile_properties = 0;
int GLAD_VK_QCOM_tile_shading = 0;
int GLAD_VK_QCOM_ycbcr_degamma = 0;
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
int GLAD_VK_QNX_external_memory_screen_buffer = 0;

#endif
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
int GLAD_VK_QNX_screen_surface = 0;

#endif
int GLAD_VK_SEC_amigo_profiling = 0;
int GLAD_VK_VALVE_descriptor_set_host_mapping = 0;
int GLAD_VK_VALVE_mutable_descriptor_type = 0;



PFN_vkAcquireDrmDisplayEXT glad_vkAcquireDrmDisplayEXT = NULL;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
PFN_vkAcquireFullScreenExclusiveModeEXT glad_vkAcquireFullScreenExclusiveModeEXT = NULL;

#endif
PFN_vkAcquireNextImage2KHR glad_vkAcquireNextImage2KHR = NULL;
PFN_vkAcquireNextImageKHR glad_vkAcquireNextImageKHR = NULL;
PFN_vkAcquirePerformanceConfigurationINTEL glad_vkAcquirePerformanceConfigurationINTEL = NULL;
PFN_vkAcquireProfilingLockKHR glad_vkAcquireProfilingLockKHR = NULL;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
PFN_vkAcquireWinrtDisplayNV glad_vkAcquireWinrtDisplayNV = NULL;

#endif
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
PFN_vkAcquireXlibDisplayEXT glad_vkAcquireXlibDisplayEXT = NULL;

#endif
PFN_vkAllocateCommandBuffers glad_vkAllocateCommandBuffers = NULL;
PFN_vkAllocateDescriptorSets glad_vkAllocateDescriptorSets = NULL;
PFN_vkAllocateMemory glad_vkAllocateMemory = NULL;
PFN_vkAntiLagUpdateAMD glad_vkAntiLagUpdateAMD = NULL;
PFN_vkBeginCommandBuffer glad_vkBeginCommandBuffer = NULL;
PFN_vkBindAccelerationStructureMemoryNV glad_vkBindAccelerationStructureMemoryNV = NULL;
PFN_vkBindBufferMemory glad_vkBindBufferMemory = NULL;
PFN_vkBindBufferMemory2 glad_vkBindBufferMemory2 = NULL;
PFN_vkBindBufferMemory2KHR glad_vkBindBufferMemory2KHR = NULL;
PFN_vkBindImageMemory glad_vkBindImageMemory = NULL;
PFN_vkBindImageMemory2 glad_vkBindImageMemory2 = NULL;
PFN_vkBindImageMemory2KHR glad_vkBindImageMemory2KHR = NULL;
PFN_vkBindOpticalFlowSessionImageNV glad_vkBindOpticalFlowSessionImageNV = NULL;
PFN_vkBuildAccelerationStructuresKHR glad_vkBuildAccelerationStructuresKHR = NULL;
PFN_vkBuildMicromapsEXT glad_vkBuildMicromapsEXT = NULL;
PFN_vkCmdBeginConditionalRenderingEXT glad_vkCmdBeginConditionalRenderingEXT = NULL;
PFN_vkCmdBeginDebugUtilsLabelEXT glad_vkCmdBeginDebugUtilsLabelEXT = NULL;
PFN_vkCmdBeginPerTileExecutionQCOM glad_vkCmdBeginPerTileExecutionQCOM = NULL;
PFN_vkCmdBeginQuery glad_vkCmdBeginQuery = NULL;
PFN_vkCmdBeginQueryIndexedEXT glad_vkCmdBeginQueryIndexedEXT = NULL;
PFN_vkCmdBeginRenderPass glad_vkCmdBeginRenderPass = NULL;
PFN_vkCmdBeginRenderPass2 glad_vkCmdBeginRenderPass2 = NULL;
PFN_vkCmdBeginRenderPass2KHR glad_vkCmdBeginRenderPass2KHR = NULL;
PFN_vkCmdBeginRendering glad_vkCmdBeginRendering = NULL;
PFN_vkCmdBeginRenderingKHR glad_vkCmdBeginRenderingKHR = NULL;
PFN_vkCmdBeginTransformFeedbackEXT glad_vkCmdBeginTransformFeedbackEXT = NULL;
PFN_vkCmdBindDescriptorBufferEmbeddedSamplers2EXT glad_vkCmdBindDescriptorBufferEmbeddedSamplers2EXT = NULL;
PFN_vkCmdBindDescriptorBufferEmbeddedSamplersEXT glad_vkCmdBindDescriptorBufferEmbeddedSamplersEXT = NULL;
PFN_vkCmdBindDescriptorBuffersEXT glad_vkCmdBindDescriptorBuffersEXT = NULL;
PFN_vkCmdBindDescriptorSets glad_vkCmdBindDescriptorSets = NULL;
PFN_vkCmdBindDescriptorSets2 glad_vkCmdBindDescriptorSets2 = NULL;
PFN_vkCmdBindDescriptorSets2KHR glad_vkCmdBindDescriptorSets2KHR = NULL;
PFN_vkCmdBindIndexBuffer glad_vkCmdBindIndexBuffer = NULL;
PFN_vkCmdBindIndexBuffer2 glad_vkCmdBindIndexBuffer2 = NULL;
PFN_vkCmdBindIndexBuffer2KHR glad_vkCmdBindIndexBuffer2KHR = NULL;
PFN_vkCmdBindInvocationMaskHUAWEI glad_vkCmdBindInvocationMaskHUAWEI = NULL;
PFN_vkCmdBindPipeline glad_vkCmdBindPipeline = NULL;
PFN_vkCmdBindPipelineShaderGroupNV glad_vkCmdBindPipelineShaderGroupNV = NULL;
PFN_vkCmdBindShadersEXT glad_vkCmdBindShadersEXT = NULL;
PFN_vkCmdBindShadingRateImageNV glad_vkCmdBindShadingRateImageNV = NULL;
PFN_vkCmdBindTileMemoryQCOM glad_vkCmdBindTileMemoryQCOM = NULL;
PFN_vkCmdBindTransformFeedbackBuffersEXT glad_vkCmdBindTransformFeedbackBuffersEXT = NULL;
PFN_vkCmdBindVertexBuffers glad_vkCmdBindVertexBuffers = NULL;
PFN_vkCmdBindVertexBuffers2 glad_vkCmdBindVertexBuffers2 = NULL;
PFN_vkCmdBindVertexBuffers2EXT glad_vkCmdBindVertexBuffers2EXT = NULL;
PFN_vkCmdBlitImage glad_vkCmdBlitImage = NULL;
PFN_vkCmdBlitImage2 glad_vkCmdBlitImage2 = NULL;
PFN_vkCmdBlitImage2KHR glad_vkCmdBlitImage2KHR = NULL;
PFN_vkCmdBuildAccelerationStructureNV glad_vkCmdBuildAccelerationStructureNV = NULL;
PFN_vkCmdBuildAccelerationStructuresIndirectKHR glad_vkCmdBuildAccelerationStructuresIndirectKHR = NULL;
PFN_vkCmdBuildAccelerationStructuresKHR glad_vkCmdBuildAccelerationStructuresKHR = NULL;
PFN_vkCmdBuildClusterAccelerationStructureIndirectNV glad_vkCmdBuildClusterAccelerationStructureIndirectNV = NULL;
PFN_vkCmdBuildMicromapsEXT glad_vkCmdBuildMicromapsEXT = NULL;
PFN_vkCmdBuildPartitionedAccelerationStructuresNV glad_vkCmdBuildPartitionedAccelerationStructuresNV = NULL;
PFN_vkCmdClearAttachments glad_vkCmdClearAttachments = NULL;
PFN_vkCmdClearColorImage glad_vkCmdClearColorImage = NULL;
PFN_vkCmdClearDepthStencilImage glad_vkCmdClearDepthStencilImage = NULL;
PFN_vkCmdConvertCooperativeVectorMatrixNV glad_vkCmdConvertCooperativeVectorMatrixNV = NULL;
PFN_vkCmdCopyAccelerationStructureKHR glad_vkCmdCopyAccelerationStructureKHR = NULL;
PFN_vkCmdCopyAccelerationStructureNV glad_vkCmdCopyAccelerationStructureNV = NULL;
PFN_vkCmdCopyAccelerationStructureToMemoryKHR glad_vkCmdCopyAccelerationStructureToMemoryKHR = NULL;
PFN_vkCmdCopyBuffer glad_vkCmdCopyBuffer = NULL;
PFN_vkCmdCopyBuffer2 glad_vkCmdCopyBuffer2 = NULL;
PFN_vkCmdCopyBuffer2KHR glad_vkCmdCopyBuffer2KHR = NULL;
PFN_vkCmdCopyBufferToImage glad_vkCmdCopyBufferToImage = NULL;
PFN_vkCmdCopyBufferToImage2 glad_vkCmdCopyBufferToImage2 = NULL;
PFN_vkCmdCopyBufferToImage2KHR glad_vkCmdCopyBufferToImage2KHR = NULL;
PFN_vkCmdCopyImage glad_vkCmdCopyImage = NULL;
PFN_vkCmdCopyImage2 glad_vkCmdCopyImage2 = NULL;
PFN_vkCmdCopyImage2KHR glad_vkCmdCopyImage2KHR = NULL;
PFN_vkCmdCopyImageToBuffer glad_vkCmdCopyImageToBuffer = NULL;
PFN_vkCmdCopyImageToBuffer2 glad_vkCmdCopyImageToBuffer2 = NULL;
PFN_vkCmdCopyImageToBuffer2KHR glad_vkCmdCopyImageToBuffer2KHR = NULL;
PFN_vkCmdCopyMemoryIndirectNV glad_vkCmdCopyMemoryIndirectNV = NULL;
PFN_vkCmdCopyMemoryToAccelerationStructureKHR glad_vkCmdCopyMemoryToAccelerationStructureKHR = NULL;
PFN_vkCmdCopyMemoryToImageIndirectNV glad_vkCmdCopyMemoryToImageIndirectNV = NULL;
PFN_vkCmdCopyMemoryToMicromapEXT glad_vkCmdCopyMemoryToMicromapEXT = NULL;
PFN_vkCmdCopyMicromapEXT glad_vkCmdCopyMicromapEXT = NULL;
PFN_vkCmdCopyMicromapToMemoryEXT glad_vkCmdCopyMicromapToMemoryEXT = NULL;
PFN_vkCmdCopyQueryPoolResults glad_vkCmdCopyQueryPoolResults = NULL;
PFN_vkCmdCuLaunchKernelNVX glad_vkCmdCuLaunchKernelNVX = NULL;
#if defined(VK_ENABLE_BETA_EXTENSIONS)
PFN_vkCmdCudaLaunchKernelNV glad_vkCmdCudaLaunchKernelNV = NULL;

#endif
PFN_vkCmdDebugMarkerBeginEXT glad_vkCmdDebugMarkerBeginEXT = NULL;
PFN_vkCmdDebugMarkerEndEXT glad_vkCmdDebugMarkerEndEXT = NULL;
PFN_vkCmdDebugMarkerInsertEXT glad_vkCmdDebugMarkerInsertEXT = NULL;
PFN_vkCmdDecompressMemoryIndirectCountNV glad_vkCmdDecompressMemoryIndirectCountNV = NULL;
PFN_vkCmdDecompressMemoryNV glad_vkCmdDecompressMemoryNV = NULL;
PFN_vkCmdDispatch glad_vkCmdDispatch = NULL;
PFN_vkCmdDispatchBase glad_vkCmdDispatchBase = NULL;
PFN_vkCmdDispatchBaseKHR glad_vkCmdDispatchBaseKHR = NULL;
#if defined(VK_ENABLE_BETA_EXTENSIONS)
PFN_vkCmdDispatchGraphAMDX glad_vkCmdDispatchGraphAMDX = NULL;

#endif
#if defined(VK_ENABLE_BETA_EXTENSIONS)
PFN_vkCmdDispatchGraphIndirectAMDX glad_vkCmdDispatchGraphIndirectAMDX = NULL;

#endif
#if defined(VK_ENABLE_BETA_EXTENSIONS)
PFN_vkCmdDispatchGraphIndirectCountAMDX glad_vkCmdDispatchGraphIndirectCountAMDX = NULL;

#endif
PFN_vkCmdDispatchIndirect glad_vkCmdDispatchIndirect = NULL;
PFN_vkCmdDispatchTileQCOM glad_vkCmdDispatchTileQCOM = NULL;
PFN_vkCmdDraw glad_vkCmdDraw = NULL;
PFN_vkCmdDrawClusterHUAWEI glad_vkCmdDrawClusterHUAWEI = NULL;
PFN_vkCmdDrawClusterIndirectHUAWEI glad_vkCmdDrawClusterIndirectHUAWEI = NULL;
PFN_vkCmdDrawIndexed glad_vkCmdDrawIndexed = NULL;
PFN_vkCmdDrawIndexedIndirect glad_vkCmdDrawIndexedIndirect = NULL;
PFN_vkCmdDrawIndexedIndirectCount glad_vkCmdDrawIndexedIndirectCount = NULL;
PFN_vkCmdDrawIndexedIndirectCountAMD glad_vkCmdDrawIndexedIndirectCountAMD = NULL;
PFN_vkCmdDrawIndexedIndirectCountKHR glad_vkCmdDrawIndexedIndirectCountKHR = NULL;
PFN_vkCmdDrawIndirect glad_vkCmdDrawIndirect = NULL;
PFN_vkCmdDrawIndirectByteCountEXT glad_vkCmdDrawIndirectByteCountEXT = NULL;
PFN_vkCmdDrawIndirectCount glad_vkCmdDrawIndirectCount = NULL;
PFN_vkCmdDrawIndirectCountAMD glad_vkCmdDrawIndirectCountAMD = NULL;
PFN_vkCmdDrawIndirectCountKHR glad_vkCmdDrawIndirectCountKHR = NULL;
PFN_vkCmdDrawMeshTasksEXT glad_vkCmdDrawMeshTasksEXT = NULL;
PFN_vkCmdDrawMeshTasksIndirectCountEXT glad_vkCmdDrawMeshTasksIndirectCountEXT = NULL;
PFN_vkCmdDrawMeshTasksIndirectCountNV glad_vkCmdDrawMeshTasksIndirectCountNV = NULL;
PFN_vkCmdDrawMeshTasksIndirectEXT glad_vkCmdDrawMeshTasksIndirectEXT = NULL;
PFN_vkCmdDrawMeshTasksIndirectNV glad_vkCmdDrawMeshTasksIndirectNV = NULL;
PFN_vkCmdDrawMeshTasksNV glad_vkCmdDrawMeshTasksNV = NULL;
PFN_vkCmdDrawMultiEXT glad_vkCmdDrawMultiEXT = NULL;
PFN_vkCmdDrawMultiIndexedEXT glad_vkCmdDrawMultiIndexedEXT = NULL;
PFN_vkCmdEndConditionalRenderingEXT glad_vkCmdEndConditionalRenderingEXT = NULL;
PFN_vkCmdEndDebugUtilsLabelEXT glad_vkCmdEndDebugUtilsLabelEXT = NULL;
PFN_vkCmdEndPerTileExecutionQCOM glad_vkCmdEndPerTileExecutionQCOM = NULL;
PFN_vkCmdEndQuery glad_vkCmdEndQuery = NULL;
PFN_vkCmdEndQueryIndexedEXT glad_vkCmdEndQueryIndexedEXT = NULL;
PFN_vkCmdEndRenderPass glad_vkCmdEndRenderPass = NULL;
PFN_vkCmdEndRenderPass2 glad_vkCmdEndRenderPass2 = NULL;
PFN_vkCmdEndRenderPass2KHR glad_vkCmdEndRenderPass2KHR = NULL;
PFN_vkCmdEndRendering glad_vkCmdEndRendering = NULL;
PFN_vkCmdEndRendering2EXT glad_vkCmdEndRendering2EXT = NULL;
PFN_vkCmdEndRenderingKHR glad_vkCmdEndRenderingKHR = NULL;
PFN_vkCmdEndTransformFeedbackEXT glad_vkCmdEndTransformFeedbackEXT = NULL;
PFN_vkCmdExecuteCommands glad_vkCmdExecuteCommands = NULL;
PFN_vkCmdExecuteGeneratedCommandsEXT glad_vkCmdExecuteGeneratedCommandsEXT = NULL;
PFN_vkCmdExecuteGeneratedCommandsNV glad_vkCmdExecuteGeneratedCommandsNV = NULL;
PFN_vkCmdFillBuffer glad_vkCmdFillBuffer = NULL;
#if defined(VK_ENABLE_BETA_EXTENSIONS)
PFN_vkCmdInitializeGraphScratchMemoryAMDX glad_vkCmdInitializeGraphScratchMemoryAMDX = NULL;

#endif
PFN_vkCmdInsertDebugUtilsLabelEXT glad_vkCmdInsertDebugUtilsLabelEXT = NULL;
PFN_vkCmdNextSubpass glad_vkCmdNextSubpass = NULL;
PFN_vkCmdNextSubpass2 glad_vkCmdNextSubpass2 = NULL;
PFN_vkCmdNextSubpass2KHR glad_vkCmdNextSubpass2KHR = NULL;
PFN_vkCmdOpticalFlowExecuteNV glad_vkCmdOpticalFlowExecuteNV = NULL;
PFN_vkCmdPipelineBarrier glad_vkCmdPipelineBarrier = NULL;
PFN_vkCmdPipelineBarrier2 glad_vkCmdPipelineBarrier2 = NULL;
PFN_vkCmdPipelineBarrier2KHR glad_vkCmdPipelineBarrier2KHR = NULL;
PFN_vkCmdPreprocessGeneratedCommandsEXT glad_vkCmdPreprocessGeneratedCommandsEXT = NULL;
PFN_vkCmdPreprocessGeneratedCommandsNV glad_vkCmdPreprocessGeneratedCommandsNV = NULL;
PFN_vkCmdPushConstants glad_vkCmdPushConstants = NULL;
PFN_vkCmdPushConstants2 glad_vkCmdPushConstants2 = NULL;
PFN_vkCmdPushConstants2KHR glad_vkCmdPushConstants2KHR = NULL;
PFN_vkCmdPushDescriptorSet glad_vkCmdPushDescriptorSet = NULL;
PFN_vkCmdPushDescriptorSet2 glad_vkCmdPushDescriptorSet2 = NULL;
PFN_vkCmdPushDescriptorSet2KHR glad_vkCmdPushDescriptorSet2KHR = NULL;
PFN_vkCmdPushDescriptorSetKHR glad_vkCmdPushDescriptorSetKHR = NULL;
PFN_vkCmdPushDescriptorSetWithTemplate glad_vkCmdPushDescriptorSetWithTemplate = NULL;
PFN_vkCmdPushDescriptorSetWithTemplate2 glad_vkCmdPushDescriptorSetWithTemplate2 = NULL;
PFN_vkCmdPushDescriptorSetWithTemplate2KHR glad_vkCmdPushDescriptorSetWithTemplate2KHR = NULL;
PFN_vkCmdPushDescriptorSetWithTemplateKHR glad_vkCmdPushDescriptorSetWithTemplateKHR = NULL;
PFN_vkCmdResetEvent glad_vkCmdResetEvent = NULL;
PFN_vkCmdResetEvent2 glad_vkCmdResetEvent2 = NULL;
PFN_vkCmdResetEvent2KHR glad_vkCmdResetEvent2KHR = NULL;
PFN_vkCmdResetQueryPool glad_vkCmdResetQueryPool = NULL;
PFN_vkCmdResolveImage glad_vkCmdResolveImage = NULL;
PFN_vkCmdResolveImage2 glad_vkCmdResolveImage2 = NULL;
PFN_vkCmdResolveImage2KHR glad_vkCmdResolveImage2KHR = NULL;
PFN_vkCmdSetAlphaToCoverageEnableEXT glad_vkCmdSetAlphaToCoverageEnableEXT = NULL;
PFN_vkCmdSetAlphaToOneEnableEXT glad_vkCmdSetAlphaToOneEnableEXT = NULL;
PFN_vkCmdSetAttachmentFeedbackLoopEnableEXT glad_vkCmdSetAttachmentFeedbackLoopEnableEXT = NULL;
PFN_vkCmdSetBlendConstants glad_vkCmdSetBlendConstants = NULL;
PFN_vkCmdSetCheckpointNV glad_vkCmdSetCheckpointNV = NULL;
PFN_vkCmdSetCoarseSampleOrderNV glad_vkCmdSetCoarseSampleOrderNV = NULL;
PFN_vkCmdSetColorBlendAdvancedEXT glad_vkCmdSetColorBlendAdvancedEXT = NULL;
PFN_vkCmdSetColorBlendEnableEXT glad_vkCmdSetColorBlendEnableEXT = NULL;
PFN_vkCmdSetColorBlendEquationEXT glad_vkCmdSetColorBlendEquationEXT = NULL;
PFN_vkCmdSetColorWriteEnableEXT glad_vkCmdSetColorWriteEnableEXT = NULL;
PFN_vkCmdSetColorWriteMaskEXT glad_vkCmdSetColorWriteMaskEXT = NULL;
PFN_vkCmdSetConservativeRasterizationModeEXT glad_vkCmdSetConservativeRasterizationModeEXT = NULL;
PFN_vkCmdSetCoverageModulationModeNV glad_vkCmdSetCoverageModulationModeNV = NULL;
PFN_vkCmdSetCoverageModulationTableEnableNV glad_vkCmdSetCoverageModulationTableEnableNV = NULL;
PFN_vkCmdSetCoverageModulationTableNV glad_vkCmdSetCoverageModulationTableNV = NULL;
PFN_vkCmdSetCoverageReductionModeNV glad_vkCmdSetCoverageReductionModeNV = NULL;
PFN_vkCmdSetCoverageToColorEnableNV glad_vkCmdSetCoverageToColorEnableNV = NULL;
PFN_vkCmdSetCoverageToColorLocationNV glad_vkCmdSetCoverageToColorLocationNV = NULL;
PFN_vkCmdSetCullMode glad_vkCmdSetCullMode = NULL;
PFN_vkCmdSetCullModeEXT glad_vkCmdSetCullModeEXT = NULL;
PFN_vkCmdSetDepthBias glad_vkCmdSetDepthBias = NULL;
PFN_vkCmdSetDepthBias2EXT glad_vkCmdSetDepthBias2EXT = NULL;
PFN_vkCmdSetDepthBiasEnable glad_vkCmdSetDepthBiasEnable = NULL;
PFN_vkCmdSetDepthBiasEnableEXT glad_vkCmdSetDepthBiasEnableEXT = NULL;
PFN_vkCmdSetDepthBounds glad_vkCmdSetDepthBounds = NULL;
PFN_vkCmdSetDepthBoundsTestEnable glad_vkCmdSetDepthBoundsTestEnable = NULL;
PFN_vkCmdSetDepthBoundsTestEnableEXT glad_vkCmdSetDepthBoundsTestEnableEXT = NULL;
PFN_vkCmdSetDepthClampEnableEXT glad_vkCmdSetDepthClampEnableEXT = NULL;
PFN_vkCmdSetDepthClampRangeEXT glad_vkCmdSetDepthClampRangeEXT = NULL;
PFN_vkCmdSetDepthClipEnableEXT glad_vkCmdSetDepthClipEnableEXT = NULL;
PFN_vkCmdSetDepthClipNegativeOneToOneEXT glad_vkCmdSetDepthClipNegativeOneToOneEXT = NULL;
PFN_vkCmdSetDepthCompareOp glad_vkCmdSetDepthCompareOp = NULL;
PFN_vkCmdSetDepthCompareOpEXT glad_vkCmdSetDepthCompareOpEXT = NULL;
PFN_vkCmdSetDepthTestEnable glad_vkCmdSetDepthTestEnable = NULL;
PFN_vkCmdSetDepthTestEnableEXT glad_vkCmdSetDepthTestEnableEXT = NULL;
PFN_vkCmdSetDepthWriteEnable glad_vkCmdSetDepthWriteEnable = NULL;
PFN_vkCmdSetDepthWriteEnableEXT glad_vkCmdSetDepthWriteEnableEXT = NULL;
PFN_vkCmdSetDescriptorBufferOffsets2EXT glad_vkCmdSetDescriptorBufferOffsets2EXT = NULL;
PFN_vkCmdSetDescriptorBufferOffsetsEXT glad_vkCmdSetDescriptorBufferOffsetsEXT = NULL;
PFN_vkCmdSetDeviceMask glad_vkCmdSetDeviceMask = NULL;
PFN_vkCmdSetDeviceMaskKHR glad_vkCmdSetDeviceMaskKHR = NULL;
PFN_vkCmdSetDiscardRectangleEXT glad_vkCmdSetDiscardRectangleEXT = NULL;
PFN_vkCmdSetDiscardRectangleEnableEXT glad_vkCmdSetDiscardRectangleEnableEXT = NULL;
PFN_vkCmdSetDiscardRectangleModeEXT glad_vkCmdSetDiscardRectangleModeEXT = NULL;
PFN_vkCmdSetEvent glad_vkCmdSetEvent = NULL;
PFN_vkCmdSetEvent2 glad_vkCmdSetEvent2 = NULL;
PFN_vkCmdSetEvent2KHR glad_vkCmdSetEvent2KHR = NULL;
PFN_vkCmdSetExclusiveScissorEnableNV glad_vkCmdSetExclusiveScissorEnableNV = NULL;
PFN_vkCmdSetExclusiveScissorNV glad_vkCmdSetExclusiveScissorNV = NULL;
PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT glad_vkCmdSetExtraPrimitiveOverestimationSizeEXT = NULL;
PFN_vkCmdSetFragmentShadingRateEnumNV glad_vkCmdSetFragmentShadingRateEnumNV = NULL;
PFN_vkCmdSetFragmentShadingRateKHR glad_vkCmdSetFragmentShadingRateKHR = NULL;
PFN_vkCmdSetFrontFace glad_vkCmdSetFrontFace = NULL;
PFN_vkCmdSetFrontFaceEXT glad_vkCmdSetFrontFaceEXT = NULL;
PFN_vkCmdSetLineRasterizationModeEXT glad_vkCmdSetLineRasterizationModeEXT = NULL;
PFN_vkCmdSetLineStipple glad_vkCmdSetLineStipple = NULL;
PFN_vkCmdSetLineStippleEXT glad_vkCmdSetLineStippleEXT = NULL;
PFN_vkCmdSetLineStippleEnableEXT glad_vkCmdSetLineStippleEnableEXT = NULL;
PFN_vkCmdSetLineStippleKHR glad_vkCmdSetLineStippleKHR = NULL;
PFN_vkCmdSetLineWidth glad_vkCmdSetLineWidth = NULL;
PFN_vkCmdSetLogicOpEXT glad_vkCmdSetLogicOpEXT = NULL;
PFN_vkCmdSetLogicOpEnableEXT glad_vkCmdSetLogicOpEnableEXT = NULL;
PFN_vkCmdSetPatchControlPointsEXT glad_vkCmdSetPatchControlPointsEXT = NULL;
PFN_vkCmdSetPerformanceMarkerINTEL glad_vkCmdSetPerformanceMarkerINTEL = NULL;
PFN_vkCmdSetPerformanceOverrideINTEL glad_vkCmdSetPerformanceOverrideINTEL = NULL;
PFN_vkCmdSetPerformanceStreamMarkerINTEL glad_vkCmdSetPerformanceStreamMarkerINTEL = NULL;
PFN_vkCmdSetPolygonModeEXT glad_vkCmdSetPolygonModeEXT = NULL;
PFN_vkCmdSetPrimitiveRestartEnable glad_vkCmdSetPrimitiveRestartEnable = NULL;
PFN_vkCmdSetPrimitiveRestartEnableEXT glad_vkCmdSetPrimitiveRestartEnableEXT = NULL;
PFN_vkCmdSetPrimitiveTopology glad_vkCmdSetPrimitiveTopology = NULL;
PFN_vkCmdSetPrimitiveTopologyEXT glad_vkCmdSetPrimitiveTopologyEXT = NULL;
PFN_vkCmdSetProvokingVertexModeEXT glad_vkCmdSetProvokingVertexModeEXT = NULL;
PFN_vkCmdSetRasterizationSamplesEXT glad_vkCmdSetRasterizationSamplesEXT = NULL;
PFN_vkCmdSetRasterizationStreamEXT glad_vkCmdSetRasterizationStreamEXT = NULL;
PFN_vkCmdSetRasterizerDiscardEnable glad_vkCmdSetRasterizerDiscardEnable = NULL;
PFN_vkCmdSetRasterizerDiscardEnableEXT glad_vkCmdSetRasterizerDiscardEnableEXT = NULL;
PFN_vkCmdSetRayTracingPipelineStackSizeKHR glad_vkCmdSetRayTracingPipelineStackSizeKHR = NULL;
PFN_vkCmdSetRenderingAttachmentLocations glad_vkCmdSetRenderingAttachmentLocations = NULL;
PFN_vkCmdSetRenderingAttachmentLocationsKHR glad_vkCmdSetRenderingAttachmentLocationsKHR = NULL;
PFN_vkCmdSetRenderingInputAttachmentIndices glad_vkCmdSetRenderingInputAttachmentIndices = NULL;
PFN_vkCmdSetRenderingInputAttachmentIndicesKHR glad_vkCmdSetRenderingInputAttachmentIndicesKHR = NULL;
PFN_vkCmdSetRepresentativeFragmentTestEnableNV glad_vkCmdSetRepresentativeFragmentTestEnableNV = NULL;
PFN_vkCmdSetSampleLocationsEXT glad_vkCmdSetSampleLocationsEXT = NULL;
PFN_vkCmdSetSampleLocationsEnableEXT glad_vkCmdSetSampleLocationsEnableEXT = NULL;
PFN_vkCmdSetSampleMaskEXT glad_vkCmdSetSampleMaskEXT = NULL;
PFN_vkCmdSetScissor glad_vkCmdSetScissor = NULL;
PFN_vkCmdSetScissorWithCount glad_vkCmdSetScissorWithCount = NULL;
PFN_vkCmdSetScissorWithCountEXT glad_vkCmdSetScissorWithCountEXT = NULL;
PFN_vkCmdSetShadingRateImageEnableNV glad_vkCmdSetShadingRateImageEnableNV = NULL;
PFN_vkCmdSetStencilCompareMask glad_vkCmdSetStencilCompareMask = NULL;
PFN_vkCmdSetStencilOp glad_vkCmdSetStencilOp = NULL;
PFN_vkCmdSetStencilOpEXT glad_vkCmdSetStencilOpEXT = NULL;
PFN_vkCmdSetStencilReference glad_vkCmdSetStencilReference = NULL;
PFN_vkCmdSetStencilTestEnable glad_vkCmdSetStencilTestEnable = NULL;
PFN_vkCmdSetStencilTestEnableEXT glad_vkCmdSetStencilTestEnableEXT = NULL;
PFN_vkCmdSetStencilWriteMask glad_vkCmdSetStencilWriteMask = NULL;
PFN_vkCmdSetTessellationDomainOriginEXT glad_vkCmdSetTessellationDomainOriginEXT = NULL;
PFN_vkCmdSetVertexInputEXT glad_vkCmdSetVertexInputEXT = NULL;
PFN_vkCmdSetViewport glad_vkCmdSetViewport = NULL;
PFN_vkCmdSetViewportShadingRatePaletteNV glad_vkCmdSetViewportShadingRatePaletteNV = NULL;
PFN_vkCmdSetViewportSwizzleNV glad_vkCmdSetViewportSwizzleNV = NULL;
PFN_vkCmdSetViewportWScalingEnableNV glad_vkCmdSetViewportWScalingEnableNV = NULL;
PFN_vkCmdSetViewportWScalingNV glad_vkCmdSetViewportWScalingNV = NULL;
PFN_vkCmdSetViewportWithCount glad_vkCmdSetViewportWithCount = NULL;
PFN_vkCmdSetViewportWithCountEXT glad_vkCmdSetViewportWithCountEXT = NULL;
PFN_vkCmdSubpassShadingHUAWEI glad_vkCmdSubpassShadingHUAWEI = NULL;
PFN_vkCmdTraceRaysIndirect2KHR glad_vkCmdTraceRaysIndirect2KHR = NULL;
PFN_vkCmdTraceRaysIndirectKHR glad_vkCmdTraceRaysIndirectKHR = NULL;
PFN_vkCmdTraceRaysKHR glad_vkCmdTraceRaysKHR = NULL;
PFN_vkCmdTraceRaysNV glad_vkCmdTraceRaysNV = NULL;
PFN_vkCmdUpdateBuffer glad_vkCmdUpdateBuffer = NULL;
PFN_vkCmdUpdatePipelineIndirectBufferNV glad_vkCmdUpdatePipelineIndirectBufferNV = NULL;
PFN_vkCmdWaitEvents glad_vkCmdWaitEvents = NULL;
PFN_vkCmdWaitEvents2 glad_vkCmdWaitEvents2 = NULL;
PFN_vkCmdWaitEvents2KHR glad_vkCmdWaitEvents2KHR = NULL;
PFN_vkCmdWriteAccelerationStructuresPropertiesKHR glad_vkCmdWriteAccelerationStructuresPropertiesKHR = NULL;
PFN_vkCmdWriteAccelerationStructuresPropertiesNV glad_vkCmdWriteAccelerationStructuresPropertiesNV = NULL;
PFN_vkCmdWriteBufferMarker2AMD glad_vkCmdWriteBufferMarker2AMD = NULL;
PFN_vkCmdWriteBufferMarkerAMD glad_vkCmdWriteBufferMarkerAMD = NULL;
PFN_vkCmdWriteMicromapsPropertiesEXT glad_vkCmdWriteMicromapsPropertiesEXT = NULL;
PFN_vkCmdWriteTimestamp glad_vkCmdWriteTimestamp = NULL;
PFN_vkCmdWriteTimestamp2 glad_vkCmdWriteTimestamp2 = NULL;
PFN_vkCmdWriteTimestamp2KHR glad_vkCmdWriteTimestamp2KHR = NULL;
PFN_vkCompileDeferredNV glad_vkCompileDeferredNV = NULL;
PFN_vkConvertCooperativeVectorMatrixNV glad_vkConvertCooperativeVectorMatrixNV = NULL;
PFN_vkCopyAccelerationStructureKHR glad_vkCopyAccelerationStructureKHR = NULL;
PFN_vkCopyAccelerationStructureToMemoryKHR glad_vkCopyAccelerationStructureToMemoryKHR = NULL;
PFN_vkCopyImageToImage glad_vkCopyImageToImage = NULL;
PFN_vkCopyImageToImageEXT glad_vkCopyImageToImageEXT = NULL;
PFN_vkCopyImageToMemory glad_vkCopyImageToMemory = NULL;
PFN_vkCopyImageToMemoryEXT glad_vkCopyImageToMemoryEXT = NULL;
PFN_vkCopyMemoryToAccelerationStructureKHR glad_vkCopyMemoryToAccelerationStructureKHR = NULL;
PFN_vkCopyMemoryToImage glad_vkCopyMemoryToImage = NULL;
PFN_vkCopyMemoryToImageEXT glad_vkCopyMemoryToImageEXT = NULL;
PFN_vkCopyMemoryToMicromapEXT glad_vkCopyMemoryToMicromapEXT = NULL;
PFN_vkCopyMicromapEXT glad_vkCopyMicromapEXT = NULL;
PFN_vkCopyMicromapToMemoryEXT glad_vkCopyMicromapToMemoryEXT = NULL;
PFN_vkCreateAccelerationStructureKHR glad_vkCreateAccelerationStructureKHR = NULL;
PFN_vkCreateAccelerationStructureNV glad_vkCreateAccelerationStructureNV = NULL;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
PFN_vkCreateAndroidSurfaceKHR glad_vkCreateAndroidSurfaceKHR = NULL;

#endif
PFN_vkCreateBuffer glad_vkCreateBuffer = NULL;
#if defined(VK_USE_PLATFORM_FUCHSIA)
PFN_vkCreateBufferCollectionFUCHSIA glad_vkCreateBufferCollectionFUCHSIA = NULL;

#endif
PFN_vkCreateBufferView glad_vkCreateBufferView = NULL;
PFN_vkCreateCommandPool glad_vkCreateCommandPool = NULL;
PFN_vkCreateComputePipelines glad_vkCreateComputePipelines = NULL;
PFN_vkCreateCuFunctionNVX glad_vkCreateCuFunctionNVX = NULL;
PFN_vkCreateCuModuleNVX glad_vkCreateCuModuleNVX = NULL;
#if defined(VK_ENABLE_BETA_EXTENSIONS)
PFN_vkCreateCudaFunctionNV glad_vkCreateCudaFunctionNV = NULL;

#endif
#if defined(VK_ENABLE_BETA_EXTENSIONS)
PFN_vkCreateCudaModuleNV glad_vkCreateCudaModuleNV = NULL;

#endif
PFN_vkCreateDebugReportCallbackEXT glad_vkCreateDebugReportCallbackEXT = NULL;
PFN_vkCreateDebugUtilsMessengerEXT glad_vkCreateDebugUtilsMessengerEXT = NULL;
PFN_vkCreateDeferredOperationKHR glad_vkCreateDeferredOperationKHR = NULL;
PFN_vkCreateDescriptorPool glad_vkCreateDescriptorPool = NULL;
PFN_vkCreateDescriptorSetLayout glad_vkCreateDescriptorSetLayout = NULL;
PFN_vkCreateDescriptorUpdateTemplate glad_vkCreateDescriptorUpdateTemplate = NULL;
PFN_vkCreateDescriptorUpdateTemplateKHR glad_vkCreateDescriptorUpdateTemplateKHR = NULL;
PFN_vkCreateDevice glad_vkCreateDevice = NULL;
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
PFN_vkCreateDirectFBSurfaceEXT glad_vkCreateDirectFBSurfaceEXT = NULL;

#endif
PFN_vkCreateDisplayModeKHR glad_vkCreateDisplayModeKHR = NULL;
PFN_vkCreateDisplayPlaneSurfaceKHR glad_vkCreateDisplayPlaneSurfaceKHR = NULL;
PFN_vkCreateEvent glad_vkCreateEvent = NULL;
#if defined(VK_ENABLE_BETA_EXTENSIONS)
PFN_vkCreateExecutionGraphPipelinesAMDX glad_vkCreateExecutionGraphPipelinesAMDX = NULL;

#endif
PFN_vkCreateExternalComputeQueueNV glad_vkCreateExternalComputeQueueNV = NULL;
PFN_vkCreateFence glad_vkCreateFence = NULL;
PFN_vkCreateFramebuffer glad_vkCreateFramebuffer = NULL;
PFN_vkCreateGraphicsPipelines glad_vkCreateGraphicsPipelines = NULL;
PFN_vkCreateHeadlessSurfaceEXT glad_vkCreateHeadlessSurfaceEXT = NULL;
#if defined(VK_USE_PLATFORM_IOS_MVK)
PFN_vkCreateIOSSurfaceMVK glad_vkCreateIOSSurfaceMVK = NULL;

#endif
PFN_vkCreateImage glad_vkCreateImage = NULL;
#if defined(VK_USE_PLATFORM_FUCHSIA)
PFN_vkCreateImagePipeSurfaceFUCHSIA glad_vkCreateImagePipeSurfaceFUCHSIA = NULL;

#endif
PFN_vkCreateImageView glad_vkCreateImageView = NULL;
PFN_vkCreateIndirectCommandsLayoutEXT glad_vkCreateIndirectCommandsLayoutEXT = NULL;
PFN_vkCreateIndirectCommandsLayoutNV glad_vkCreateIndirectCommandsLayoutNV = NULL;
PFN_vkCreateIndirectExecutionSetEXT glad_vkCreateIndirectExecutionSetEXT = NULL;
PFN_vkCreateInstance glad_vkCreateInstance = NULL;
#if defined(VK_USE_PLATFORM_MACOS_MVK)
PFN_vkCreateMacOSSurfaceMVK glad_vkCreateMacOSSurfaceMVK = NULL;

#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
PFN_vkCreateMetalSurfaceEXT glad_vkCreateMetalSurfaceEXT = NULL;

#endif
PFN_vkCreateMicromapEXT glad_vkCreateMicromapEXT = NULL;
PFN_vkCreateOpticalFlowSessionNV glad_vkCreateOpticalFlowSessionNV = NULL;
PFN_vkCreatePipelineBinariesKHR glad_vkCreatePipelineBinariesKHR = NULL;
PFN_vkCreatePipelineCache glad_vkCreatePipelineCache = NULL;
PFN_vkCreatePipelineLayout glad_vkCreatePipelineLayout = NULL;
PFN_vkCreatePrivateDataSlot glad_vkCreatePrivateDataSlot = NULL;
PFN_vkCreatePrivateDataSlotEXT glad_vkCreatePrivateDataSlotEXT = NULL;
PFN_vkCreateQueryPool glad_vkCreateQueryPool = NULL;
PFN_vkCreateRayTracingPipelinesKHR glad_vkCreateRayTracingPipelinesKHR = NULL;
PFN_vkCreateRayTracingPipelinesNV glad_vkCreateRayTracingPipelinesNV = NULL;
PFN_vkCreateRenderPass glad_vkCreateRenderPass = NULL;
PFN_vkCreateRenderPass2 glad_vkCreateRenderPass2 = NULL;
PFN_vkCreateRenderPass2KHR glad_vkCreateRenderPass2KHR = NULL;
PFN_vkCreateSampler glad_vkCreateSampler = NULL;
PFN_vkCreateSamplerYcbcrConversion glad_vkCreateSamplerYcbcrConversion = NULL;
PFN_vkCreateSamplerYcbcrConversionKHR glad_vkCreateSamplerYcbcrConversionKHR = NULL;
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
PFN_vkCreateScreenSurfaceQNX glad_vkCreateScreenSurfaceQNX = NULL;

#endif
PFN_vkCreateSemaphore glad_vkCreateSemaphore = NULL;
PFN_vkCreateShaderModule glad_vkCreateShaderModule = NULL;
PFN_vkCreateShadersEXT glad_vkCreateShadersEXT = NULL;
PFN_vkCreateSharedSwapchainsKHR glad_vkCreateSharedSwapchainsKHR = NULL;
#if defined(VK_USE_PLATFORM_GGP)
PFN_vkCreateStreamDescriptorSurfaceGGP glad_vkCreateStreamDescriptorSurfaceGGP = NULL;

#endif
PFN_vkCreateSwapchainKHR glad_vkCreateSwapchainKHR = NULL;
PFN_vkCreateValidationCacheEXT glad_vkCreateValidationCacheEXT = NULL;
#if defined(VK_USE_PLATFORM_VI_NN)
PFN_vkCreateViSurfaceNN glad_vkCreateViSurfaceNN = NULL;

#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
PFN_vkCreateWaylandSurfaceKHR glad_vkCreateWaylandSurfaceKHR = NULL;

#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
PFN_vkCreateWin32SurfaceKHR glad_vkCreateWin32SurfaceKHR = NULL;

#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
PFN_vkCreateXcbSurfaceKHR glad_vkCreateXcbSurfaceKHR = NULL;

#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
PFN_vkCreateXlibSurfaceKHR glad_vkCreateXlibSurfaceKHR = NULL;

#endif
PFN_vkDebugMarkerSetObjectNameEXT glad_vkDebugMarkerSetObjectNameEXT = NULL;
PFN_vkDebugMarkerSetObjectTagEXT glad_vkDebugMarkerSetObjectTagEXT = NULL;
PFN_vkDebugReportMessageEXT glad_vkDebugReportMessageEXT = NULL;
PFN_vkDeferredOperationJoinKHR glad_vkDeferredOperationJoinKHR = NULL;
PFN_vkDestroyAccelerationStructureKHR glad_vkDestroyAccelerationStructureKHR = NULL;
PFN_vkDestroyAccelerationStructureNV glad_vkDestroyAccelerationStructureNV = NULL;
PFN_vkDestroyBuffer glad_vkDestroyBuffer = NULL;
#if defined(VK_USE_PLATFORM_FUCHSIA)
PFN_vkDestroyBufferCollectionFUCHSIA glad_vkDestroyBufferCollectionFUCHSIA = NULL;

#endif
PFN_vkDestroyBufferView glad_vkDestroyBufferView = NULL;
PFN_vkDestroyCommandPool glad_vkDestroyCommandPool = NULL;
PFN_vkDestroyCuFunctionNVX glad_vkDestroyCuFunctionNVX = NULL;
PFN_vkDestroyCuModuleNVX glad_vkDestroyCuModuleNVX = NULL;
#if defined(VK_ENABLE_BETA_EXTENSIONS)
PFN_vkDestroyCudaFunctionNV glad_vkDestroyCudaFunctionNV = NULL;

#endif
#if defined(VK_ENABLE_BETA_EXTENSIONS)
PFN_vkDestroyCudaModuleNV glad_vkDestroyCudaModuleNV = NULL;

#endif
PFN_vkDestroyDebugReportCallbackEXT glad_vkDestroyDebugReportCallbackEXT = NULL;
PFN_vkDestroyDebugUtilsMessengerEXT glad_vkDestroyDebugUtilsMessengerEXT = NULL;
PFN_vkDestroyDeferredOperationKHR glad_vkDestroyDeferredOperationKHR = NULL;
PFN_vkDestroyDescriptorPool glad_vkDestroyDescriptorPool = NULL;
PFN_vkDestroyDescriptorSetLayout glad_vkDestroyDescriptorSetLayout = NULL;
PFN_vkDestroyDescriptorUpdateTemplate glad_vkDestroyDescriptorUpdateTemplate = NULL;
PFN_vkDestroyDescriptorUpdateTemplateKHR glad_vkDestroyDescriptorUpdateTemplateKHR = NULL;
PFN_vkDestroyDevice glad_vkDestroyDevice = NULL;
PFN_vkDestroyEvent glad_vkDestroyEvent = NULL;
PFN_vkDestroyExternalComputeQueueNV glad_vkDestroyExternalComputeQueueNV = NULL;
PFN_vkDestroyFence glad_vkDestroyFence = NULL;
PFN_vkDestroyFramebuffer glad_vkDestroyFramebuffer = NULL;
PFN_vkDestroyImage glad_vkDestroyImage = NULL;
PFN_vkDestroyImageView glad_vkDestroyImageView = NULL;
PFN_vkDestroyIndirectCommandsLayoutEXT glad_vkDestroyIndirectCommandsLayoutEXT = NULL;
PFN_vkDestroyIndirectCommandsLayoutNV glad_vkDestroyIndirectCommandsLayoutNV = NULL;
PFN_vkDestroyIndirectExecutionSetEXT glad_vkDestroyIndirectExecutionSetEXT = NULL;
PFN_vkDestroyInstance glad_vkDestroyInstance = NULL;
PFN_vkDestroyMicromapEXT glad_vkDestroyMicromapEXT = NULL;
PFN_vkDestroyOpticalFlowSessionNV glad_vkDestroyOpticalFlowSessionNV = NULL;
PFN_vkDestroyPipeline glad_vkDestroyPipeline = NULL;
PFN_vkDestroyPipelineBinaryKHR glad_vkDestroyPipelineBinaryKHR = NULL;
PFN_vkDestroyPipelineCache glad_vkDestroyPipelineCache = NULL;
PFN_vkDestroyPipelineLayout glad_vkDestroyPipelineLayout = NULL;
PFN_vkDestroyPrivateDataSlot glad_vkDestroyPrivateDataSlot = NULL;
PFN_vkDestroyPrivateDataSlotEXT glad_vkDestroyPrivateDataSlotEXT = NULL;
PFN_vkDestroyQueryPool glad_vkDestroyQueryPool = NULL;
PFN_vkDestroyRenderPass glad_vkDestroyRenderPass = NULL;
PFN_vkDestroySampler glad_vkDestroySampler = NULL;
PFN_vkDestroySamplerYcbcrConversion glad_vkDestroySamplerYcbcrConversion = NULL;
PFN_vkDestroySamplerYcbcrConversionKHR glad_vkDestroySamplerYcbcrConversionKHR = NULL;
PFN_vkDestroySemaphore glad_vkDestroySemaphore = NULL;
PFN_vkDestroyShaderEXT glad_vkDestroyShaderEXT = NULL;
PFN_vkDestroyShaderModule glad_vkDestroyShaderModule = NULL;
PFN_vkDestroySurfaceKHR glad_vkDestroySurfaceKHR = NULL;
PFN_vkDestroySwapchainKHR glad_vkDestroySwapchainKHR = NULL;
PFN_vkDestroyValidationCacheEXT glad_vkDestroyValidationCacheEXT = NULL;
PFN_vkDeviceWaitIdle glad_vkDeviceWaitIdle = NULL;
PFN_vkDisplayPowerControlEXT glad_vkDisplayPowerControlEXT = NULL;
PFN_vkEndCommandBuffer glad_vkEndCommandBuffer = NULL;
PFN_vkEnumerateDeviceExtensionProperties glad_vkEnumerateDeviceExtensionProperties = NULL;
PFN_vkEnumerateDeviceLayerProperties glad_vkEnumerateDeviceLayerProperties = NULL;
PFN_vkEnumerateInstanceExtensionProperties glad_vkEnumerateInstanceExtensionProperties = NULL;
PFN_vkEnumerateInstanceLayerProperties glad_vkEnumerateInstanceLayerProperties = NULL;
PFN_vkEnumerateInstanceVersion glad_vkEnumerateInstanceVersion = NULL;
PFN_vkEnumeratePhysicalDeviceGroups glad_vkEnumeratePhysicalDeviceGroups = NULL;
PFN_vkEnumeratePhysicalDeviceGroupsKHR glad_vkEnumeratePhysicalDeviceGroupsKHR = NULL;
PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR glad_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR = NULL;
PFN_vkEnumeratePhysicalDevices glad_vkEnumeratePhysicalDevices = NULL;
#if defined(VK_USE_PLATFORM_METAL_EXT)
PFN_vkExportMetalObjectsEXT glad_vkExportMetalObjectsEXT = NULL;

#endif
PFN_vkFlushMappedMemoryRanges glad_vkFlushMappedMemoryRanges = NULL;
PFN_vkFreeCommandBuffers glad_vkFreeCommandBuffers = NULL;
PFN_vkFreeDescriptorSets glad_vkFreeDescriptorSets = NULL;
PFN_vkFreeMemory glad_vkFreeMemory = NULL;
PFN_vkGetAccelerationStructureBuildSizesKHR glad_vkGetAccelerationStructureBuildSizesKHR = NULL;
PFN_vkGetAccelerationStructureDeviceAddressKHR glad_vkGetAccelerationStructureDeviceAddressKHR = NULL;
PFN_vkGetAccelerationStructureHandleNV glad_vkGetAccelerationStructureHandleNV = NULL;
PFN_vkGetAccelerationStructureMemoryRequirementsNV glad_vkGetAccelerationStructureMemoryRequirementsNV = NULL;
PFN_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT glad_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT = NULL;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
PFN_vkGetAndroidHardwareBufferPropertiesANDROID glad_vkGetAndroidHardwareBufferPropertiesANDROID = NULL;

#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
PFN_vkGetBufferCollectionPropertiesFUCHSIA glad_vkGetBufferCollectionPropertiesFUCHSIA = NULL;

#endif
PFN_vkGetBufferDeviceAddress glad_vkGetBufferDeviceAddress = NULL;
PFN_vkGetBufferDeviceAddressEXT glad_vkGetBufferDeviceAddressEXT = NULL;
PFN_vkGetBufferDeviceAddressKHR glad_vkGetBufferDeviceAddressKHR = NULL;
PFN_vkGetBufferMemoryRequirements glad_vkGetBufferMemoryRequirements = NULL;
PFN_vkGetBufferMemoryRequirements2 glad_vkGetBufferMemoryRequirements2 = NULL;
PFN_vkGetBufferMemoryRequirements2KHR glad_vkGetBufferMemoryRequirements2KHR = NULL;
PFN_vkGetBufferOpaqueCaptureAddress glad_vkGetBufferOpaqueCaptureAddress = NULL;
PFN_vkGetBufferOpaqueCaptureAddressKHR glad_vkGetBufferOpaqueCaptureAddressKHR = NULL;
PFN_vkGetBufferOpaqueCaptureDescriptorDataEXT glad_vkGetBufferOpaqueCaptureDescriptorDataEXT = NULL;
PFN_vkGetCalibratedTimestampsEXT glad_vkGetCalibratedTimestampsEXT = NULL;
PFN_vkGetCalibratedTimestampsKHR glad_vkGetCalibratedTimestampsKHR = NULL;
PFN_vkGetClusterAccelerationStructureBuildSizesNV glad_vkGetClusterAccelerationStructureBuildSizesNV = NULL;
#if defined(VK_ENABLE_BETA_EXTENSIONS)
PFN_vkGetCudaModuleCacheNV glad_vkGetCudaModuleCacheNV = NULL;

#endif
PFN_vkGetDeferredOperationMaxConcurrencyKHR glad_vkGetDeferredOperationMaxConcurrencyKHR = NULL;
PFN_vkGetDeferredOperationResultKHR glad_vkGetDeferredOperationResultKHR = NULL;
PFN_vkGetDescriptorEXT glad_vkGetDescriptorEXT = NULL;
PFN_vkGetDescriptorSetHostMappingVALVE glad_vkGetDescriptorSetHostMappingVALVE = NULL;
PFN_vkGetDescriptorSetLayoutBindingOffsetEXT glad_vkGetDescriptorSetLayoutBindingOffsetEXT = NULL;
PFN_vkGetDescriptorSetLayoutHostMappingInfoVALVE glad_vkGetDescriptorSetLayoutHostMappingInfoVALVE = NULL;
PFN_vkGetDescriptorSetLayoutSizeEXT glad_vkGetDescriptorSetLayoutSizeEXT = NULL;
PFN_vkGetDescriptorSetLayoutSupport glad_vkGetDescriptorSetLayoutSupport = NULL;
PFN_vkGetDescriptorSetLayoutSupportKHR glad_vkGetDescriptorSetLayoutSupportKHR = NULL;
PFN_vkGetDeviceAccelerationStructureCompatibilityKHR glad_vkGetDeviceAccelerationStructureCompatibilityKHR = NULL;
PFN_vkGetDeviceBufferMemoryRequirements glad_vkGetDeviceBufferMemoryRequirements = NULL;
PFN_vkGetDeviceBufferMemoryRequirementsKHR glad_vkGetDeviceBufferMemoryRequirementsKHR = NULL;
PFN_vkGetDeviceFaultInfoEXT glad_vkGetDeviceFaultInfoEXT = NULL;
PFN_vkGetDeviceGroupPeerMemoryFeatures glad_vkGetDeviceGroupPeerMemoryFeatures = NULL;
PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR glad_vkGetDeviceGroupPeerMemoryFeaturesKHR = NULL;
PFN_vkGetDeviceGroupPresentCapabilitiesKHR glad_vkGetDeviceGroupPresentCapabilitiesKHR = NULL;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
PFN_vkGetDeviceGroupSurfacePresentModes2EXT glad_vkGetDeviceGroupSurfacePresentModes2EXT = NULL;

#endif
PFN_vkGetDeviceGroupSurfacePresentModesKHR glad_vkGetDeviceGroupSurfacePresentModesKHR = NULL;
PFN_vkGetDeviceImageMemoryRequirements glad_vkGetDeviceImageMemoryRequirements = NULL;
PFN_vkGetDeviceImageMemoryRequirementsKHR glad_vkGetDeviceImageMemoryRequirementsKHR = NULL;
PFN_vkGetDeviceImageSparseMemoryRequirements glad_vkGetDeviceImageSparseMemoryRequirements = NULL;
PFN_vkGetDeviceImageSparseMemoryRequirementsKHR glad_vkGetDeviceImageSparseMemoryRequirementsKHR = NULL;
PFN_vkGetDeviceImageSubresourceLayout glad_vkGetDeviceImageSubresourceLayout = NULL;
PFN_vkGetDeviceImageSubresourceLayoutKHR glad_vkGetDeviceImageSubresourceLayoutKHR = NULL;
PFN_vkGetDeviceMemoryCommitment glad_vkGetDeviceMemoryCommitment = NULL;
PFN_vkGetDeviceMemoryOpaqueCaptureAddress glad_vkGetDeviceMemoryOpaqueCaptureAddress = NULL;
PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR glad_vkGetDeviceMemoryOpaqueCaptureAddressKHR = NULL;
PFN_vkGetDeviceMicromapCompatibilityEXT glad_vkGetDeviceMicromapCompatibilityEXT = NULL;
PFN_vkGetDeviceProcAddr glad_vkGetDeviceProcAddr = NULL;
PFN_vkGetDeviceQueue glad_vkGetDeviceQueue = NULL;
PFN_vkGetDeviceQueue2 glad_vkGetDeviceQueue2 = NULL;
PFN_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI glad_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI = NULL;
PFN_vkGetDisplayModeProperties2KHR glad_vkGetDisplayModeProperties2KHR = NULL;
PFN_vkGetDisplayModePropertiesKHR glad_vkGetDisplayModePropertiesKHR = NULL;
PFN_vkGetDisplayPlaneCapabilities2KHR glad_vkGetDisplayPlaneCapabilities2KHR = NULL;
PFN_vkGetDisplayPlaneCapabilitiesKHR glad_vkGetDisplayPlaneCapabilitiesKHR = NULL;
PFN_vkGetDisplayPlaneSupportedDisplaysKHR glad_vkGetDisplayPlaneSupportedDisplaysKHR = NULL;
PFN_vkGetDrmDisplayEXT glad_vkGetDrmDisplayEXT = NULL;
PFN_vkGetDynamicRenderingTilePropertiesQCOM glad_vkGetDynamicRenderingTilePropertiesQCOM = NULL;
PFN_vkGetEventStatus glad_vkGetEventStatus = NULL;
#if defined(VK_ENABLE_BETA_EXTENSIONS)
PFN_vkGetExecutionGraphPipelineNodeIndexAMDX glad_vkGetExecutionGraphPipelineNodeIndexAMDX = NULL;

#endif
#if defined(VK_ENABLE_BETA_EXTENSIONS)
PFN_vkGetExecutionGraphPipelineScratchSizeAMDX glad_vkGetExecutionGraphPipelineScratchSizeAMDX = NULL;

#endif
PFN_vkGetExternalComputeQueueDataNV glad_vkGetExternalComputeQueueDataNV = NULL;
PFN_vkGetFenceFdKHR glad_vkGetFenceFdKHR = NULL;
PFN_vkGetFenceStatus glad_vkGetFenceStatus = NULL;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
PFN_vkGetFenceWin32HandleKHR glad_vkGetFenceWin32HandleKHR = NULL;

#endif
PFN_vkGetFramebufferTilePropertiesQCOM glad_vkGetFramebufferTilePropertiesQCOM = NULL;
PFN_vkGetGeneratedCommandsMemoryRequirementsEXT glad_vkGetGeneratedCommandsMemoryRequirementsEXT = NULL;
PFN_vkGetGeneratedCommandsMemoryRequirementsNV glad_vkGetGeneratedCommandsMemoryRequirementsNV = NULL;
PFN_vkGetImageDrmFormatModifierPropertiesEXT glad_vkGetImageDrmFormatModifierPropertiesEXT = NULL;
PFN_vkGetImageMemoryRequirements glad_vkGetImageMemoryRequirements = NULL;
PFN_vkGetImageMemoryRequirements2 glad_vkGetImageMemoryRequirements2 = NULL;
PFN_vkGetImageMemoryRequirements2KHR glad_vkGetImageMemoryRequirements2KHR = NULL;
PFN_vkGetImageOpaqueCaptureDescriptorDataEXT glad_vkGetImageOpaqueCaptureDescriptorDataEXT = NULL;
PFN_vkGetImageSparseMemoryRequirements glad_vkGetImageSparseMemoryRequirements = NULL;
PFN_vkGetImageSparseMemoryRequirements2 glad_vkGetImageSparseMemoryRequirements2 = NULL;
PFN_vkGetImageSparseMemoryRequirements2KHR glad_vkGetImageSparseMemoryRequirements2KHR = NULL;
PFN_vkGetImageSubresourceLayout glad_vkGetImageSubresourceLayout = NULL;
PFN_vkGetImageSubresourceLayout2 glad_vkGetImageSubresourceLayout2 = NULL;
PFN_vkGetImageSubresourceLayout2EXT glad_vkGetImageSubresourceLayout2EXT = NULL;
PFN_vkGetImageSubresourceLayout2KHR glad_vkGetImageSubresourceLayout2KHR = NULL;
PFN_vkGetImageViewAddressNVX glad_vkGetImageViewAddressNVX = NULL;
PFN_vkGetImageViewHandle64NVX glad_vkGetImageViewHandle64NVX = NULL;
PFN_vkGetImageViewHandleNVX glad_vkGetImageViewHandleNVX = NULL;
PFN_vkGetImageViewOpaqueCaptureDescriptorDataEXT glad_vkGetImageViewOpaqueCaptureDescriptorDataEXT = NULL;
PFN_vkGetInstanceProcAddr glad_vkGetInstanceProcAddr = NULL;
PFN_vkGetLatencyTimingsNV glad_vkGetLatencyTimingsNV = NULL;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
PFN_vkGetMemoryAndroidHardwareBufferANDROID glad_vkGetMemoryAndroidHardwareBufferANDROID = NULL;

#endif
PFN_vkGetMemoryFdKHR glad_vkGetMemoryFdKHR = NULL;
PFN_vkGetMemoryFdPropertiesKHR glad_vkGetMemoryFdPropertiesKHR = NULL;
PFN_vkGetMemoryHostPointerPropertiesEXT glad_vkGetMemoryHostPointerPropertiesEXT = NULL;
#if defined(VK_USE_PLATFORM_METAL_EXT)
PFN_vkGetMemoryMetalHandleEXT glad_vkGetMemoryMetalHandleEXT = NULL;

#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
PFN_vkGetMemoryMetalHandlePropertiesEXT glad_vkGetMemoryMetalHandlePropertiesEXT = NULL;

#endif
PFN_vkGetMemoryRemoteAddressNV glad_vkGetMemoryRemoteAddressNV = NULL;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
PFN_vkGetMemoryWin32HandleKHR glad_vkGetMemoryWin32HandleKHR = NULL;

#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
PFN_vkGetMemoryWin32HandleNV glad_vkGetMemoryWin32HandleNV = NULL;

#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
PFN_vkGetMemoryWin32HandlePropertiesKHR glad_vkGetMemoryWin32HandlePropertiesKHR = NULL;

#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
PFN_vkGetMemoryZirconHandleFUCHSIA glad_vkGetMemoryZirconHandleFUCHSIA = NULL;

#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
PFN_vkGetMemoryZirconHandlePropertiesFUCHSIA glad_vkGetMemoryZirconHandlePropertiesFUCHSIA = NULL;

#endif
PFN_vkGetMicromapBuildSizesEXT glad_vkGetMicromapBuildSizesEXT = NULL;
PFN_vkGetPartitionedAccelerationStructuresBuildSizesNV glad_vkGetPartitionedAccelerationStructuresBuildSizesNV = NULL;
PFN_vkGetPastPresentationTimingGOOGLE glad_vkGetPastPresentationTimingGOOGLE = NULL;
PFN_vkGetPerformanceParameterINTEL glad_vkGetPerformanceParameterINTEL = NULL;
PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT glad_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT = NULL;
PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR glad_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR = NULL;
PFN_vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV glad_vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV = NULL;
PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR glad_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR = NULL;
PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV glad_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV = NULL;
PFN_vkGetPhysicalDeviceCooperativeVectorPropertiesNV glad_vkGetPhysicalDeviceCooperativeVectorPropertiesNV = NULL;
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
PFN_vkGetPhysicalDeviceDirectFBPresentationSupportEXT glad_vkGetPhysicalDeviceDirectFBPresentationSupportEXT = NULL;

#endif
PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR glad_vkGetPhysicalDeviceDisplayPlaneProperties2KHR = NULL;
PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR glad_vkGetPhysicalDeviceDisplayPlanePropertiesKHR = NULL;
PFN_vkGetPhysicalDeviceDisplayProperties2KHR glad_vkGetPhysicalDeviceDisplayProperties2KHR = NULL;
PFN_vkGetPhysicalDeviceDisplayPropertiesKHR glad_vkGetPhysicalDeviceDisplayPropertiesKHR = NULL;
PFN_vkGetPhysicalDeviceExternalBufferProperties glad_vkGetPhysicalDeviceExternalBufferProperties = NULL;
PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR glad_vkGetPhysicalDeviceExternalBufferPropertiesKHR = NULL;
PFN_vkGetPhysicalDeviceExternalFenceProperties glad_vkGetPhysicalDeviceExternalFenceProperties = NULL;
PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR glad_vkGetPhysicalDeviceExternalFencePropertiesKHR = NULL;
PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV glad_vkGetPhysicalDeviceExternalImageFormatPropertiesNV = NULL;
PFN_vkGetPhysicalDeviceExternalSemaphoreProperties glad_vkGetPhysicalDeviceExternalSemaphoreProperties = NULL;
PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR glad_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR = NULL;
PFN_vkGetPhysicalDeviceFeatures glad_vkGetPhysicalDeviceFeatures = NULL;
PFN_vkGetPhysicalDeviceFeatures2 glad_vkGetPhysicalDeviceFeatures2 = NULL;
PFN_vkGetPhysicalDeviceFeatures2KHR glad_vkGetPhysicalDeviceFeatures2KHR = NULL;
PFN_vkGetPhysicalDeviceFormatProperties glad_vkGetPhysicalDeviceFormatProperties = NULL;
PFN_vkGetPhysicalDeviceFormatProperties2 glad_vkGetPhysicalDeviceFormatProperties2 = NULL;
PFN_vkGetPhysicalDeviceFormatProperties2KHR glad_vkGetPhysicalDeviceFormatProperties2KHR = NULL;
PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR glad_vkGetPhysicalDeviceFragmentShadingRatesKHR = NULL;
PFN_vkGetPhysicalDeviceImageFormatProperties glad_vkGetPhysicalDeviceImageFormatProperties = NULL;
PFN_vkGetPhysicalDeviceImageFormatProperties2 glad_vkGetPhysicalDeviceImageFormatProperties2 = NULL;
PFN_vkGetPhysicalDeviceImageFormatProperties2KHR glad_vkGetPhysicalDeviceImageFormatProperties2KHR = NULL;
PFN_vkGetPhysicalDeviceMemoryProperties glad_vkGetPhysicalDeviceMemoryProperties = NULL;
PFN_vkGetPhysicalDeviceMemoryProperties2 glad_vkGetPhysicalDeviceMemoryProperties2 = NULL;
PFN_vkGetPhysicalDeviceMemoryProperties2KHR glad_vkGetPhysicalDeviceMemoryProperties2KHR = NULL;
PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT glad_vkGetPhysicalDeviceMultisamplePropertiesEXT = NULL;
PFN_vkGetPhysicalDeviceOpticalFlowImageFormatsNV glad_vkGetPhysicalDeviceOpticalFlowImageFormatsNV = NULL;
PFN_vkGetPhysicalDevicePresentRectanglesKHR glad_vkGetPhysicalDevicePresentRectanglesKHR = NULL;
PFN_vkGetPhysicalDeviceProperties glad_vkGetPhysicalDeviceProperties = NULL;
PFN_vkGetPhysicalDeviceProperties2 glad_vkGetPhysicalDeviceProperties2 = NULL;
PFN_vkGetPhysicalDeviceProperties2KHR glad_vkGetPhysicalDeviceProperties2KHR = NULL;
PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR glad_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR = NULL;
PFN_vkGetPhysicalDeviceQueueFamilyProperties glad_vkGetPhysicalDeviceQueueFamilyProperties = NULL;
PFN_vkGetPhysicalDeviceQueueFamilyProperties2 glad_vkGetPhysicalDeviceQueueFamilyProperties2 = NULL;
PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR glad_vkGetPhysicalDeviceQueueFamilyProperties2KHR = NULL;
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
PFN_vkGetPhysicalDeviceScreenPresentationSupportQNX glad_vkGetPhysicalDeviceScreenPresentationSupportQNX = NULL;

#endif
PFN_vkGetPhysicalDeviceSparseImageFormatProperties glad_vkGetPhysicalDeviceSparseImageFormatProperties = NULL;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 glad_vkGetPhysicalDeviceSparseImageFormatProperties2 = NULL;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR glad_vkGetPhysicalDeviceSparseImageFormatProperties2KHR = NULL;
PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV glad_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV = NULL;
PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT glad_vkGetPhysicalDeviceSurfaceCapabilities2EXT = NULL;
PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR glad_vkGetPhysicalDeviceSurfaceCapabilities2KHR = NULL;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR glad_vkGetPhysicalDeviceSurfaceCapabilitiesKHR = NULL;
PFN_vkGetPhysicalDeviceSurfaceFormats2KHR glad_vkGetPhysicalDeviceSurfaceFormats2KHR = NULL;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR glad_vkGetPhysicalDeviceSurfaceFormatsKHR = NULL;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
PFN_vkGetPhysicalDeviceSurfacePresentModes2EXT glad_vkGetPhysicalDeviceSurfacePresentModes2EXT = NULL;

#endif
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR glad_vkGetPhysicalDeviceSurfacePresentModesKHR = NULL;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR glad_vkGetPhysicalDeviceSurfaceSupportKHR = NULL;
PFN_vkGetPhysicalDeviceToolProperties glad_vkGetPhysicalDeviceToolProperties = NULL;
PFN_vkGetPhysicalDeviceToolPropertiesEXT glad_vkGetPhysicalDeviceToolPropertiesEXT = NULL;
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR glad_vkGetPhysicalDeviceWaylandPresentationSupportKHR = NULL;

#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR glad_vkGetPhysicalDeviceWin32PresentationSupportKHR = NULL;

#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR glad_vkGetPhysicalDeviceXcbPresentationSupportKHR = NULL;

#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR glad_vkGetPhysicalDeviceXlibPresentationSupportKHR = NULL;

#endif
PFN_vkGetPipelineBinaryDataKHR glad_vkGetPipelineBinaryDataKHR = NULL;
PFN_vkGetPipelineCacheData glad_vkGetPipelineCacheData = NULL;
PFN_vkGetPipelineExecutableInternalRepresentationsKHR glad_vkGetPipelineExecutableInternalRepresentationsKHR = NULL;
PFN_vkGetPipelineExecutablePropertiesKHR glad_vkGetPipelineExecutablePropertiesKHR = NULL;
PFN_vkGetPipelineExecutableStatisticsKHR glad_vkGetPipelineExecutableStatisticsKHR = NULL;
PFN_vkGetPipelineIndirectDeviceAddressNV glad_vkGetPipelineIndirectDeviceAddressNV = NULL;
PFN_vkGetPipelineIndirectMemoryRequirementsNV glad_vkGetPipelineIndirectMemoryRequirementsNV = NULL;
PFN_vkGetPipelineKeyKHR glad_vkGetPipelineKeyKHR = NULL;
PFN_vkGetPipelinePropertiesEXT glad_vkGetPipelinePropertiesEXT = NULL;
PFN_vkGetPrivateData glad_vkGetPrivateData = NULL;
PFN_vkGetPrivateDataEXT glad_vkGetPrivateDataEXT = NULL;
PFN_vkGetQueryPoolResults glad_vkGetQueryPoolResults = NULL;
PFN_vkGetQueueCheckpointData2NV glad_vkGetQueueCheckpointData2NV = NULL;
PFN_vkGetQueueCheckpointDataNV glad_vkGetQueueCheckpointDataNV = NULL;
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
PFN_vkGetRandROutputDisplayEXT glad_vkGetRandROutputDisplayEXT = NULL;

#endif
PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR glad_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR = NULL;
PFN_vkGetRayTracingShaderGroupHandlesKHR glad_vkGetRayTracingShaderGroupHandlesKHR = NULL;
PFN_vkGetRayTracingShaderGroupHandlesNV glad_vkGetRayTracingShaderGroupHandlesNV = NULL;
PFN_vkGetRayTracingShaderGroupStackSizeKHR glad_vkGetRayTracingShaderGroupStackSizeKHR = NULL;
PFN_vkGetRefreshCycleDurationGOOGLE glad_vkGetRefreshCycleDurationGOOGLE = NULL;
PFN_vkGetRenderAreaGranularity glad_vkGetRenderAreaGranularity = NULL;
PFN_vkGetRenderingAreaGranularity glad_vkGetRenderingAreaGranularity = NULL;
PFN_vkGetRenderingAreaGranularityKHR glad_vkGetRenderingAreaGranularityKHR = NULL;
PFN_vkGetSamplerOpaqueCaptureDescriptorDataEXT glad_vkGetSamplerOpaqueCaptureDescriptorDataEXT = NULL;
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
PFN_vkGetScreenBufferPropertiesQNX glad_vkGetScreenBufferPropertiesQNX = NULL;

#endif
PFN_vkGetSemaphoreCounterValue glad_vkGetSemaphoreCounterValue = NULL;
PFN_vkGetSemaphoreCounterValueKHR glad_vkGetSemaphoreCounterValueKHR = NULL;
PFN_vkGetSemaphoreFdKHR glad_vkGetSemaphoreFdKHR = NULL;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
PFN_vkGetSemaphoreWin32HandleKHR glad_vkGetSemaphoreWin32HandleKHR = NULL;

#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
PFN_vkGetSemaphoreZirconHandleFUCHSIA glad_vkGetSemaphoreZirconHandleFUCHSIA = NULL;

#endif
PFN_vkGetShaderBinaryDataEXT glad_vkGetShaderBinaryDataEXT = NULL;
PFN_vkGetShaderInfoAMD glad_vkGetShaderInfoAMD = NULL;
PFN_vkGetShaderModuleCreateInfoIdentifierEXT glad_vkGetShaderModuleCreateInfoIdentifierEXT = NULL;
PFN_vkGetShaderModuleIdentifierEXT glad_vkGetShaderModuleIdentifierEXT = NULL;
PFN_vkGetSwapchainCounterEXT glad_vkGetSwapchainCounterEXT = NULL;
PFN_vkGetSwapchainImagesKHR glad_vkGetSwapchainImagesKHR = NULL;
PFN_vkGetSwapchainStatusKHR glad_vkGetSwapchainStatusKHR = NULL;
PFN_vkGetValidationCacheDataEXT glad_vkGetValidationCacheDataEXT = NULL;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
PFN_vkGetWinrtDisplayNV glad_vkGetWinrtDisplayNV = NULL;

#endif
PFN_vkImportFenceFdKHR glad_vkImportFenceFdKHR = NULL;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
PFN_vkImportFenceWin32HandleKHR glad_vkImportFenceWin32HandleKHR = NULL;

#endif
PFN_vkImportSemaphoreFdKHR glad_vkImportSemaphoreFdKHR = NULL;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
PFN_vkImportSemaphoreWin32HandleKHR glad_vkImportSemaphoreWin32HandleKHR = NULL;

#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
PFN_vkImportSemaphoreZirconHandleFUCHSIA glad_vkImportSemaphoreZirconHandleFUCHSIA = NULL;

#endif
PFN_vkInitializePerformanceApiINTEL glad_vkInitializePerformanceApiINTEL = NULL;
PFN_vkInvalidateMappedMemoryRanges glad_vkInvalidateMappedMemoryRanges = NULL;
PFN_vkLatencySleepNV glad_vkLatencySleepNV = NULL;
PFN_vkMapMemory glad_vkMapMemory = NULL;
PFN_vkMapMemory2 glad_vkMapMemory2 = NULL;
PFN_vkMapMemory2KHR glad_vkMapMemory2KHR = NULL;
PFN_vkMergePipelineCaches glad_vkMergePipelineCaches = NULL;
PFN_vkMergeValidationCachesEXT glad_vkMergeValidationCachesEXT = NULL;
PFN_vkQueueBeginDebugUtilsLabelEXT glad_vkQueueBeginDebugUtilsLabelEXT = NULL;
PFN_vkQueueBindSparse glad_vkQueueBindSparse = NULL;
PFN_vkQueueEndDebugUtilsLabelEXT glad_vkQueueEndDebugUtilsLabelEXT = NULL;
PFN_vkQueueInsertDebugUtilsLabelEXT glad_vkQueueInsertDebugUtilsLabelEXT = NULL;
PFN_vkQueueNotifyOutOfBandNV glad_vkQueueNotifyOutOfBandNV = NULL;
PFN_vkQueuePresentKHR glad_vkQueuePresentKHR = NULL;
PFN_vkQueueSetPerformanceConfigurationINTEL glad_vkQueueSetPerformanceConfigurationINTEL = NULL;
PFN_vkQueueSubmit glad_vkQueueSubmit = NULL;
PFN_vkQueueSubmit2 glad_vkQueueSubmit2 = NULL;
PFN_vkQueueSubmit2KHR glad_vkQueueSubmit2KHR = NULL;
PFN_vkQueueWaitIdle glad_vkQueueWaitIdle = NULL;
PFN_vkRegisterDeviceEventEXT glad_vkRegisterDeviceEventEXT = NULL;
PFN_vkRegisterDisplayEventEXT glad_vkRegisterDisplayEventEXT = NULL;
PFN_vkReleaseCapturedPipelineDataKHR glad_vkReleaseCapturedPipelineDataKHR = NULL;
PFN_vkReleaseDisplayEXT glad_vkReleaseDisplayEXT = NULL;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
PFN_vkReleaseFullScreenExclusiveModeEXT glad_vkReleaseFullScreenExclusiveModeEXT = NULL;

#endif
PFN_vkReleasePerformanceConfigurationINTEL glad_vkReleasePerformanceConfigurationINTEL = NULL;
PFN_vkReleaseProfilingLockKHR glad_vkReleaseProfilingLockKHR = NULL;
PFN_vkReleaseSwapchainImagesEXT glad_vkReleaseSwapchainImagesEXT = NULL;
PFN_vkResetCommandBuffer glad_vkResetCommandBuffer = NULL;
PFN_vkResetCommandPool glad_vkResetCommandPool = NULL;
PFN_vkResetDescriptorPool glad_vkResetDescriptorPool = NULL;
PFN_vkResetEvent glad_vkResetEvent = NULL;
PFN_vkResetFences glad_vkResetFences = NULL;
PFN_vkResetQueryPool glad_vkResetQueryPool = NULL;
PFN_vkResetQueryPoolEXT glad_vkResetQueryPoolEXT = NULL;
#if defined(VK_USE_PLATFORM_FUCHSIA)
PFN_vkSetBufferCollectionBufferConstraintsFUCHSIA glad_vkSetBufferCollectionBufferConstraintsFUCHSIA = NULL;

#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
PFN_vkSetBufferCollectionImageConstraintsFUCHSIA glad_vkSetBufferCollectionImageConstraintsFUCHSIA = NULL;

#endif
PFN_vkSetDebugUtilsObjectNameEXT glad_vkSetDebugUtilsObjectNameEXT = NULL;
PFN_vkSetDebugUtilsObjectTagEXT glad_vkSetDebugUtilsObjectTagEXT = NULL;
PFN_vkSetDeviceMemoryPriorityEXT glad_vkSetDeviceMemoryPriorityEXT = NULL;
PFN_vkSetEvent glad_vkSetEvent = NULL;
PFN_vkSetHdrMetadataEXT glad_vkSetHdrMetadataEXT = NULL;
PFN_vkSetLatencyMarkerNV glad_vkSetLatencyMarkerNV = NULL;
PFN_vkSetLatencySleepModeNV glad_vkSetLatencySleepModeNV = NULL;
PFN_vkSetLocalDimmingAMD glad_vkSetLocalDimmingAMD = NULL;
PFN_vkSetPrivateData glad_vkSetPrivateData = NULL;
PFN_vkSetPrivateDataEXT glad_vkSetPrivateDataEXT = NULL;
PFN_vkSignalSemaphore glad_vkSignalSemaphore = NULL;
PFN_vkSignalSemaphoreKHR glad_vkSignalSemaphoreKHR = NULL;
PFN_vkSubmitDebugUtilsMessageEXT glad_vkSubmitDebugUtilsMessageEXT = NULL;
PFN_vkTransitionImageLayout glad_vkTransitionImageLayout = NULL;
PFN_vkTransitionImageLayoutEXT glad_vkTransitionImageLayoutEXT = NULL;
PFN_vkTrimCommandPool glad_vkTrimCommandPool = NULL;
PFN_vkTrimCommandPoolKHR glad_vkTrimCommandPoolKHR = NULL;
PFN_vkUninitializePerformanceApiINTEL glad_vkUninitializePerformanceApiINTEL = NULL;
PFN_vkUnmapMemory glad_vkUnmapMemory = NULL;
PFN_vkUnmapMemory2 glad_vkUnmapMemory2 = NULL;
PFN_vkUnmapMemory2KHR glad_vkUnmapMemory2KHR = NULL;
PFN_vkUpdateDescriptorSetWithTemplate glad_vkUpdateDescriptorSetWithTemplate = NULL;
PFN_vkUpdateDescriptorSetWithTemplateKHR glad_vkUpdateDescriptorSetWithTemplateKHR = NULL;
PFN_vkUpdateDescriptorSets glad_vkUpdateDescriptorSets = NULL;
PFN_vkUpdateIndirectExecutionSetPipelineEXT glad_vkUpdateIndirectExecutionSetPipelineEXT = NULL;
PFN_vkUpdateIndirectExecutionSetShaderEXT glad_vkUpdateIndirectExecutionSetShaderEXT = NULL;
PFN_vkWaitForFences glad_vkWaitForFences = NULL;
PFN_vkWaitForPresentKHR glad_vkWaitForPresentKHR = NULL;
PFN_vkWaitSemaphores glad_vkWaitSemaphores = NULL;
PFN_vkWaitSemaphoresKHR glad_vkWaitSemaphoresKHR = NULL;
PFN_vkWriteAccelerationStructuresPropertiesKHR glad_vkWriteAccelerationStructuresPropertiesKHR = NULL;
PFN_vkWriteMicromapsPropertiesEXT glad_vkWriteMicromapsPropertiesEXT = NULL;


static void glad_vk_load_VK_VERSION_1_0( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_VERSION_1_0) return;
    glad_vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers) load(userptr, "vkAllocateCommandBuffers");
    glad_vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets) load(userptr, "vkAllocateDescriptorSets");
    glad_vkAllocateMemory = (PFN_vkAllocateMemory) load(userptr, "vkAllocateMemory");
    glad_vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer) load(userptr, "vkBeginCommandBuffer");
    glad_vkBindBufferMemory = (PFN_vkBindBufferMemory) load(userptr, "vkBindBufferMemory");
    glad_vkBindImageMemory = (PFN_vkBindImageMemory) load(userptr, "vkBindImageMemory");
    glad_vkCmdBeginQuery = (PFN_vkCmdBeginQuery) load(userptr, "vkCmdBeginQuery");
    glad_vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass) load(userptr, "vkCmdBeginRenderPass");
    glad_vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets) load(userptr, "vkCmdBindDescriptorSets");
    glad_vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer) load(userptr, "vkCmdBindIndexBuffer");
    glad_vkCmdBindPipeline = (PFN_vkCmdBindPipeline) load(userptr, "vkCmdBindPipeline");
    glad_vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers) load(userptr, "vkCmdBindVertexBuffers");
    glad_vkCmdBlitImage = (PFN_vkCmdBlitImage) load(userptr, "vkCmdBlitImage");
    glad_vkCmdClearAttachments = (PFN_vkCmdClearAttachments) load(userptr, "vkCmdClearAttachments");
    glad_vkCmdClearColorImage = (PFN_vkCmdClearColorImage) load(userptr, "vkCmdClearColorImage");
    glad_vkCmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage) load(userptr, "vkCmdClearDepthStencilImage");
    glad_vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer) load(userptr, "vkCmdCopyBuffer");
    glad_vkCmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage) load(userptr, "vkCmdCopyBufferToImage");
    glad_vkCmdCopyImage = (PFN_vkCmdCopyImage) load(userptr, "vkCmdCopyImage");
    glad_vkCmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer) load(userptr, "vkCmdCopyImageToBuffer");
    glad_vkCmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults) load(userptr, "vkCmdCopyQueryPoolResults");
    glad_vkCmdDispatch = (PFN_vkCmdDispatch) load(userptr, "vkCmdDispatch");
    glad_vkCmdDispatchIndirect = (PFN_vkCmdDispatchIndirect) load(userptr, "vkCmdDispatchIndirect");
    glad_vkCmdDraw = (PFN_vkCmdDraw) load(userptr, "vkCmdDraw");
    glad_vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed) load(userptr, "vkCmdDrawIndexed");
    glad_vkCmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect) load(userptr, "vkCmdDrawIndexedIndirect");
    glad_vkCmdDrawIndirect = (PFN_vkCmdDrawIndirect) load(userptr, "vkCmdDrawIndirect");
    glad_vkCmdEndQuery = (PFN_vkCmdEndQuery) load(userptr, "vkCmdEndQuery");
    glad_vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass) load(userptr, "vkCmdEndRenderPass");
    glad_vkCmdExecuteCommands = (PFN_vkCmdExecuteCommands) load(userptr, "vkCmdExecuteCommands");
    glad_vkCmdFillBuffer = (PFN_vkCmdFillBuffer) load(userptr, "vkCmdFillBuffer");
    glad_vkCmdNextSubpass = (PFN_vkCmdNextSubpass) load(userptr, "vkCmdNextSubpass");
    glad_vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier) load(userptr, "vkCmdPipelineBarrier");
    glad_vkCmdPushConstants = (PFN_vkCmdPushConstants) load(userptr, "vkCmdPushConstants");
    glad_vkCmdResetEvent = (PFN_vkCmdResetEvent) load(userptr, "vkCmdResetEvent");
    glad_vkCmdResetQueryPool = (PFN_vkCmdResetQueryPool) load(userptr, "vkCmdResetQueryPool");
    glad_vkCmdResolveImage = (PFN_vkCmdResolveImage) load(userptr, "vkCmdResolveImage");
    glad_vkCmdSetBlendConstants = (PFN_vkCmdSetBlendConstants) load(userptr, "vkCmdSetBlendConstants");
    glad_vkCmdSetDepthBias = (PFN_vkCmdSetDepthBias) load(userptr, "vkCmdSetDepthBias");
    glad_vkCmdSetDepthBounds = (PFN_vkCmdSetDepthBounds) load(userptr, "vkCmdSetDepthBounds");
    glad_vkCmdSetEvent = (PFN_vkCmdSetEvent) load(userptr, "vkCmdSetEvent");
    glad_vkCmdSetLineWidth = (PFN_vkCmdSetLineWidth) load(userptr, "vkCmdSetLineWidth");
    glad_vkCmdSetScissor = (PFN_vkCmdSetScissor) load(userptr, "vkCmdSetScissor");
    glad_vkCmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask) load(userptr, "vkCmdSetStencilCompareMask");
    glad_vkCmdSetStencilReference = (PFN_vkCmdSetStencilReference) load(userptr, "vkCmdSetStencilReference");
    glad_vkCmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask) load(userptr, "vkCmdSetStencilWriteMask");
    glad_vkCmdSetViewport = (PFN_vkCmdSetViewport) load(userptr, "vkCmdSetViewport");
    glad_vkCmdUpdateBuffer = (PFN_vkCmdUpdateBuffer) load(userptr, "vkCmdUpdateBuffer");
    glad_vkCmdWaitEvents = (PFN_vkCmdWaitEvents) load(userptr, "vkCmdWaitEvents");
    glad_vkCmdWriteTimestamp = (PFN_vkCmdWriteTimestamp) load(userptr, "vkCmdWriteTimestamp");
    glad_vkCreateBuffer = (PFN_vkCreateBuffer) load(userptr, "vkCreateBuffer");
    glad_vkCreateBufferView = (PFN_vkCreateBufferView) load(userptr, "vkCreateBufferView");
    glad_vkCreateCommandPool = (PFN_vkCreateCommandPool) load(userptr, "vkCreateCommandPool");
    glad_vkCreateComputePipelines = (PFN_vkCreateComputePipelines) load(userptr, "vkCreateComputePipelines");
    glad_vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool) load(userptr, "vkCreateDescriptorPool");
    glad_vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout) load(userptr, "vkCreateDescriptorSetLayout");
    glad_vkCreateDevice = (PFN_vkCreateDevice) load(userptr, "vkCreateDevice");
    glad_vkCreateEvent = (PFN_vkCreateEvent) load(userptr, "vkCreateEvent");
    glad_vkCreateFence = (PFN_vkCreateFence) load(userptr, "vkCreateFence");
    glad_vkCreateFramebuffer = (PFN_vkCreateFramebuffer) load(userptr, "vkCreateFramebuffer");
    glad_vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines) load(userptr, "vkCreateGraphicsPipelines");
    glad_vkCreateImage = (PFN_vkCreateImage) load(userptr, "vkCreateImage");
    glad_vkCreateImageView = (PFN_vkCreateImageView) load(userptr, "vkCreateImageView");
    glad_vkCreateInstance = (PFN_vkCreateInstance) load(userptr, "vkCreateInstance");
    glad_vkCreatePipelineCache = (PFN_vkCreatePipelineCache) load(userptr, "vkCreatePipelineCache");
    glad_vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout) load(userptr, "vkCreatePipelineLayout");
    glad_vkCreateQueryPool = (PFN_vkCreateQueryPool) load(userptr, "vkCreateQueryPool");
    glad_vkCreateRenderPass = (PFN_vkCreateRenderPass) load(userptr, "vkCreateRenderPass");
    glad_vkCreateSampler = (PFN_vkCreateSampler) load(userptr, "vkCreateSampler");
    glad_vkCreateSemaphore = (PFN_vkCreateSemaphore) load(userptr, "vkCreateSemaphore");
    glad_vkCreateShaderModule = (PFN_vkCreateShaderModule) load(userptr, "vkCreateShaderModule");
    glad_vkDestroyBuffer = (PFN_vkDestroyBuffer) load(userptr, "vkDestroyBuffer");
    glad_vkDestroyBufferView = (PFN_vkDestroyBufferView) load(userptr, "vkDestroyBufferView");
    glad_vkDestroyCommandPool = (PFN_vkDestroyCommandPool) load(userptr, "vkDestroyCommandPool");
    glad_vkDestroyDescriptorPool = (PFN_vkDestroyDescriptorPool) load(userptr, "vkDestroyDescriptorPool");
    glad_vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout) load(userptr, "vkDestroyDescriptorSetLayout");
    glad_vkDestroyDevice = (PFN_vkDestroyDevice) load(userptr, "vkDestroyDevice");
    glad_vkDestroyEvent = (PFN_vkDestroyEvent) load(userptr, "vkDestroyEvent");
    glad_vkDestroyFence = (PFN_vkDestroyFence) load(userptr, "vkDestroyFence");
    glad_vkDestroyFramebuffer = (PFN_vkDestroyFramebuffer) load(userptr, "vkDestroyFramebuffer");
    glad_vkDestroyImage = (PFN_vkDestroyImage) load(userptr, "vkDestroyImage");
    glad_vkDestroyImageView = (PFN_vkDestroyImageView) load(userptr, "vkDestroyImageView");
    glad_vkDestroyInstance = (PFN_vkDestroyInstance) load(userptr, "vkDestroyInstance");
    glad_vkDestroyPipeline = (PFN_vkDestroyPipeline) load(userptr, "vkDestroyPipeline");
    glad_vkDestroyPipelineCache = (PFN_vkDestroyPipelineCache) load(userptr, "vkDestroyPipelineCache");
    glad_vkDestroyPipelineLayout = (PFN_vkDestroyPipelineLayout) load(userptr, "vkDestroyPipelineLayout");
    glad_vkDestroyQueryPool = (PFN_vkDestroyQueryPool) load(userptr, "vkDestroyQueryPool");
    glad_vkDestroyRenderPass = (PFN_vkDestroyRenderPass) load(userptr, "vkDestroyRenderPass");
    glad_vkDestroySampler = (PFN_vkDestroySampler) load(userptr, "vkDestroySampler");
    glad_vkDestroySemaphore = (PFN_vkDestroySemaphore) load(userptr, "vkDestroySemaphore");
    glad_vkDestroyShaderModule = (PFN_vkDestroyShaderModule) load(userptr, "vkDestroyShaderModule");
    glad_vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle) load(userptr, "vkDeviceWaitIdle");
    glad_vkEndCommandBuffer = (PFN_vkEndCommandBuffer) load(userptr, "vkEndCommandBuffer");
    glad_vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties) load(userptr, "vkEnumerateDeviceExtensionProperties");
    glad_vkEnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties) load(userptr, "vkEnumerateDeviceLayerProperties");
    glad_vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties) load(userptr, "vkEnumerateInstanceExtensionProperties");
    glad_vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties) load(userptr, "vkEnumerateInstanceLayerProperties");
    glad_vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices) load(userptr, "vkEnumeratePhysicalDevices");
    glad_vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges) load(userptr, "vkFlushMappedMemoryRanges");
    glad_vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers) load(userptr, "vkFreeCommandBuffers");
    glad_vkFreeDescriptorSets = (PFN_vkFreeDescriptorSets) load(userptr, "vkFreeDescriptorSets");
    glad_vkFreeMemory = (PFN_vkFreeMemory) load(userptr, "vkFreeMemory");
    glad_vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements) load(userptr, "vkGetBufferMemoryRequirements");
    glad_vkGetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment) load(userptr, "vkGetDeviceMemoryCommitment");
    glad_vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr) load(userptr, "vkGetDeviceProcAddr");
    glad_vkGetDeviceQueue = (PFN_vkGetDeviceQueue) load(userptr, "vkGetDeviceQueue");
    glad_vkGetEventStatus = (PFN_vkGetEventStatus) load(userptr, "vkGetEventStatus");
    glad_vkGetFenceStatus = (PFN_vkGetFenceStatus) load(userptr, "vkGetFenceStatus");
    glad_vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements) load(userptr, "vkGetImageMemoryRequirements");
    glad_vkGetImageSparseMemoryRequirements = (PFN_vkGetImageSparseMemoryRequirements) load(userptr, "vkGetImageSparseMemoryRequirements");
    glad_vkGetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout) load(userptr, "vkGetImageSubresourceLayout");
    glad_vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) load(userptr, "vkGetInstanceProcAddr");
    glad_vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures) load(userptr, "vkGetPhysicalDeviceFeatures");
    glad_vkGetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties) load(userptr, "vkGetPhysicalDeviceFormatProperties");
    glad_vkGetPhysicalDeviceImageFormatProperties = (PFN_vkGetPhysicalDeviceImageFormatProperties) load(userptr, "vkGetPhysicalDeviceImageFormatProperties");
    glad_vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties) load(userptr, "vkGetPhysicalDeviceMemoryProperties");
    glad_vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties) load(userptr, "vkGetPhysicalDeviceProperties");
    glad_vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties) load(userptr, "vkGetPhysicalDeviceQueueFamilyProperties");
    glad_vkGetPhysicalDeviceSparseImageFormatProperties = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties) load(userptr, "vkGetPhysicalDeviceSparseImageFormatProperties");
    glad_vkGetPipelineCacheData = (PFN_vkGetPipelineCacheData) load(userptr, "vkGetPipelineCacheData");
    glad_vkGetQueryPoolResults = (PFN_vkGetQueryPoolResults) load(userptr, "vkGetQueryPoolResults");
    glad_vkGetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity) load(userptr, "vkGetRenderAreaGranularity");
    glad_vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges) load(userptr, "vkInvalidateMappedMemoryRanges");
    glad_vkMapMemory = (PFN_vkMapMemory) load(userptr, "vkMapMemory");
    glad_vkMergePipelineCaches = (PFN_vkMergePipelineCaches) load(userptr, "vkMergePipelineCaches");
    glad_vkQueueBindSparse = (PFN_vkQueueBindSparse) load(userptr, "vkQueueBindSparse");
    glad_vkQueueSubmit = (PFN_vkQueueSubmit) load(userptr, "vkQueueSubmit");
    glad_vkQueueWaitIdle = (PFN_vkQueueWaitIdle) load(userptr, "vkQueueWaitIdle");
    glad_vkResetCommandBuffer = (PFN_vkResetCommandBuffer) load(userptr, "vkResetCommandBuffer");
    glad_vkResetCommandPool = (PFN_vkResetCommandPool) load(userptr, "vkResetCommandPool");
    glad_vkResetDescriptorPool = (PFN_vkResetDescriptorPool) load(userptr, "vkResetDescriptorPool");
    glad_vkResetEvent = (PFN_vkResetEvent) load(userptr, "vkResetEvent");
    glad_vkResetFences = (PFN_vkResetFences) load(userptr, "vkResetFences");
    glad_vkSetEvent = (PFN_vkSetEvent) load(userptr, "vkSetEvent");
    glad_vkUnmapMemory = (PFN_vkUnmapMemory) load(userptr, "vkUnmapMemory");
    glad_vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets) load(userptr, "vkUpdateDescriptorSets");
    glad_vkWaitForFences = (PFN_vkWaitForFences) load(userptr, "vkWaitForFences");
}
static void glad_vk_load_VK_VERSION_1_1( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_VERSION_1_1) return;
    glad_vkBindBufferMemory2 = (PFN_vkBindBufferMemory2) load(userptr, "vkBindBufferMemory2");
    glad_vkBindImageMemory2 = (PFN_vkBindImageMemory2) load(userptr, "vkBindImageMemory2");
    glad_vkCmdDispatchBase = (PFN_vkCmdDispatchBase) load(userptr, "vkCmdDispatchBase");
    glad_vkCmdSetDeviceMask = (PFN_vkCmdSetDeviceMask) load(userptr, "vkCmdSetDeviceMask");
    glad_vkCreateDescriptorUpdateTemplate = (PFN_vkCreateDescriptorUpdateTemplate) load(userptr, "vkCreateDescriptorUpdateTemplate");
    glad_vkCreateSamplerYcbcrConversion = (PFN_vkCreateSamplerYcbcrConversion) load(userptr, "vkCreateSamplerYcbcrConversion");
    glad_vkDestroyDescriptorUpdateTemplate = (PFN_vkDestroyDescriptorUpdateTemplate) load(userptr, "vkDestroyDescriptorUpdateTemplate");
    glad_vkDestroySamplerYcbcrConversion = (PFN_vkDestroySamplerYcbcrConversion) load(userptr, "vkDestroySamplerYcbcrConversion");
    glad_vkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion) load(userptr, "vkEnumerateInstanceVersion");
    glad_vkEnumeratePhysicalDeviceGroups = (PFN_vkEnumeratePhysicalDeviceGroups) load(userptr, "vkEnumeratePhysicalDeviceGroups");
    glad_vkGetBufferMemoryRequirements2 = (PFN_vkGetBufferMemoryRequirements2) load(userptr, "vkGetBufferMemoryRequirements2");
    glad_vkGetDescriptorSetLayoutSupport = (PFN_vkGetDescriptorSetLayoutSupport) load(userptr, "vkGetDescriptorSetLayoutSupport");
    glad_vkGetDeviceGroupPeerMemoryFeatures = (PFN_vkGetDeviceGroupPeerMemoryFeatures) load(userptr, "vkGetDeviceGroupPeerMemoryFeatures");
    glad_vkGetDeviceQueue2 = (PFN_vkGetDeviceQueue2) load(userptr, "vkGetDeviceQueue2");
    glad_vkGetImageMemoryRequirements2 = (PFN_vkGetImageMemoryRequirements2) load(userptr, "vkGetImageMemoryRequirements2");
    glad_vkGetImageSparseMemoryRequirements2 = (PFN_vkGetImageSparseMemoryRequirements2) load(userptr, "vkGetImageSparseMemoryRequirements2");
    glad_vkGetPhysicalDeviceExternalBufferProperties = (PFN_vkGetPhysicalDeviceExternalBufferProperties) load(userptr, "vkGetPhysicalDeviceExternalBufferProperties");
    glad_vkGetPhysicalDeviceExternalFenceProperties = (PFN_vkGetPhysicalDeviceExternalFenceProperties) load(userptr, "vkGetPhysicalDeviceExternalFenceProperties");
    glad_vkGetPhysicalDeviceExternalSemaphoreProperties = (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties) load(userptr, "vkGetPhysicalDeviceExternalSemaphoreProperties");
    glad_vkGetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2) load(userptr, "vkGetPhysicalDeviceFeatures2");
    glad_vkGetPhysicalDeviceFormatProperties2 = (PFN_vkGetPhysicalDeviceFormatProperties2) load(userptr, "vkGetPhysicalDeviceFormatProperties2");
    glad_vkGetPhysicalDeviceImageFormatProperties2 = (PFN_vkGetPhysicalDeviceImageFormatProperties2) load(userptr, "vkGetPhysicalDeviceImageFormatProperties2");
    glad_vkGetPhysicalDeviceMemoryProperties2 = (PFN_vkGetPhysicalDeviceMemoryProperties2) load(userptr, "vkGetPhysicalDeviceMemoryProperties2");
    glad_vkGetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2) load(userptr, "vkGetPhysicalDeviceProperties2");
    glad_vkGetPhysicalDeviceQueueFamilyProperties2 = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2) load(userptr, "vkGetPhysicalDeviceQueueFamilyProperties2");
    glad_vkGetPhysicalDeviceSparseImageFormatProperties2 = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2) load(userptr, "vkGetPhysicalDeviceSparseImageFormatProperties2");
    glad_vkTrimCommandPool = (PFN_vkTrimCommandPool) load(userptr, "vkTrimCommandPool");
    glad_vkUpdateDescriptorSetWithTemplate = (PFN_vkUpdateDescriptorSetWithTemplate) load(userptr, "vkUpdateDescriptorSetWithTemplate");
}
static void glad_vk_load_VK_VERSION_1_2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_VERSION_1_2) return;
    glad_vkCmdBeginRenderPass2 = (PFN_vkCmdBeginRenderPass2) load(userptr, "vkCmdBeginRenderPass2");
    glad_vkCmdDrawIndexedIndirectCount = (PFN_vkCmdDrawIndexedIndirectCount) load(userptr, "vkCmdDrawIndexedIndirectCount");
    glad_vkCmdDrawIndirectCount = (PFN_vkCmdDrawIndirectCount) load(userptr, "vkCmdDrawIndirectCount");
    glad_vkCmdEndRenderPass2 = (PFN_vkCmdEndRenderPass2) load(userptr, "vkCmdEndRenderPass2");
    glad_vkCmdNextSubpass2 = (PFN_vkCmdNextSubpass2) load(userptr, "vkCmdNextSubpass2");
    glad_vkCreateRenderPass2 = (PFN_vkCreateRenderPass2) load(userptr, "vkCreateRenderPass2");
    glad_vkGetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress) load(userptr, "vkGetBufferDeviceAddress");
    glad_vkGetBufferOpaqueCaptureAddress = (PFN_vkGetBufferOpaqueCaptureAddress) load(userptr, "vkGetBufferOpaqueCaptureAddress");
    glad_vkGetDeviceMemoryOpaqueCaptureAddress = (PFN_vkGetDeviceMemoryOpaqueCaptureAddress) load(userptr, "vkGetDeviceMemoryOpaqueCaptureAddress");
    glad_vkGetSemaphoreCounterValue = (PFN_vkGetSemaphoreCounterValue) load(userptr, "vkGetSemaphoreCounterValue");
    glad_vkResetQueryPool = (PFN_vkResetQueryPool) load(userptr, "vkResetQueryPool");
    glad_vkSignalSemaphore = (PFN_vkSignalSemaphore) load(userptr, "vkSignalSemaphore");
    glad_vkWaitSemaphores = (PFN_vkWaitSemaphores) load(userptr, "vkWaitSemaphores");
}
static void glad_vk_load_VK_VERSION_1_3( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_VERSION_1_3) return;
    glad_vkCmdBeginRendering = (PFN_vkCmdBeginRendering) load(userptr, "vkCmdBeginRendering");
    glad_vkCmdBindVertexBuffers2 = (PFN_vkCmdBindVertexBuffers2) load(userptr, "vkCmdBindVertexBuffers2");
    glad_vkCmdBlitImage2 = (PFN_vkCmdBlitImage2) load(userptr, "vkCmdBlitImage2");
    glad_vkCmdCopyBuffer2 = (PFN_vkCmdCopyBuffer2) load(userptr, "vkCmdCopyBuffer2");
    glad_vkCmdCopyBufferToImage2 = (PFN_vkCmdCopyBufferToImage2) load(userptr, "vkCmdCopyBufferToImage2");
    glad_vkCmdCopyImage2 = (PFN_vkCmdCopyImage2) load(userptr, "vkCmdCopyImage2");
    glad_vkCmdCopyImageToBuffer2 = (PFN_vkCmdCopyImageToBuffer2) load(userptr, "vkCmdCopyImageToBuffer2");
    glad_vkCmdEndRendering = (PFN_vkCmdEndRendering) load(userptr, "vkCmdEndRendering");
    glad_vkCmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2) load(userptr, "vkCmdPipelineBarrier2");
    glad_vkCmdResetEvent2 = (PFN_vkCmdResetEvent2) load(userptr, "vkCmdResetEvent2");
    glad_vkCmdResolveImage2 = (PFN_vkCmdResolveImage2) load(userptr, "vkCmdResolveImage2");
    glad_vkCmdSetCullMode = (PFN_vkCmdSetCullMode) load(userptr, "vkCmdSetCullMode");
    glad_vkCmdSetDepthBiasEnable = (PFN_vkCmdSetDepthBiasEnable) load(userptr, "vkCmdSetDepthBiasEnable");
    glad_vkCmdSetDepthBoundsTestEnable = (PFN_vkCmdSetDepthBoundsTestEnable) load(userptr, "vkCmdSetDepthBoundsTestEnable");
    glad_vkCmdSetDepthCompareOp = (PFN_vkCmdSetDepthCompareOp) load(userptr, "vkCmdSetDepthCompareOp");
    glad_vkCmdSetDepthTestEnable = (PFN_vkCmdSetDepthTestEnable) load(userptr, "vkCmdSetDepthTestEnable");
    glad_vkCmdSetDepthWriteEnable = (PFN_vkCmdSetDepthWriteEnable) load(userptr, "vkCmdSetDepthWriteEnable");
    glad_vkCmdSetEvent2 = (PFN_vkCmdSetEvent2) load(userptr, "vkCmdSetEvent2");
    glad_vkCmdSetFrontFace = (PFN_vkCmdSetFrontFace) load(userptr, "vkCmdSetFrontFace");
    glad_vkCmdSetPrimitiveRestartEnable = (PFN_vkCmdSetPrimitiveRestartEnable) load(userptr, "vkCmdSetPrimitiveRestartEnable");
    glad_vkCmdSetPrimitiveTopology = (PFN_vkCmdSetPrimitiveTopology) load(userptr, "vkCmdSetPrimitiveTopology");
    glad_vkCmdSetRasterizerDiscardEnable = (PFN_vkCmdSetRasterizerDiscardEnable) load(userptr, "vkCmdSetRasterizerDiscardEnable");
    glad_vkCmdSetScissorWithCount = (PFN_vkCmdSetScissorWithCount) load(userptr, "vkCmdSetScissorWithCount");
    glad_vkCmdSetStencilOp = (PFN_vkCmdSetStencilOp) load(userptr, "vkCmdSetStencilOp");
    glad_vkCmdSetStencilTestEnable = (PFN_vkCmdSetStencilTestEnable) load(userptr, "vkCmdSetStencilTestEnable");
    glad_vkCmdSetViewportWithCount = (PFN_vkCmdSetViewportWithCount) load(userptr, "vkCmdSetViewportWithCount");
    glad_vkCmdWaitEvents2 = (PFN_vkCmdWaitEvents2) load(userptr, "vkCmdWaitEvents2");
    glad_vkCmdWriteTimestamp2 = (PFN_vkCmdWriteTimestamp2) load(userptr, "vkCmdWriteTimestamp2");
    glad_vkCreatePrivateDataSlot = (PFN_vkCreatePrivateDataSlot) load(userptr, "vkCreatePrivateDataSlot");
    glad_vkDestroyPrivateDataSlot = (PFN_vkDestroyPrivateDataSlot) load(userptr, "vkDestroyPrivateDataSlot");
    glad_vkGetDeviceBufferMemoryRequirements = (PFN_vkGetDeviceBufferMemoryRequirements) load(userptr, "vkGetDeviceBufferMemoryRequirements");
    glad_vkGetDeviceImageMemoryRequirements = (PFN_vkGetDeviceImageMemoryRequirements) load(userptr, "vkGetDeviceImageMemoryRequirements");
    glad_vkGetDeviceImageSparseMemoryRequirements = (PFN_vkGetDeviceImageSparseMemoryRequirements) load(userptr, "vkGetDeviceImageSparseMemoryRequirements");
    glad_vkGetPhysicalDeviceToolProperties = (PFN_vkGetPhysicalDeviceToolProperties) load(userptr, "vkGetPhysicalDeviceToolProperties");
    glad_vkGetPrivateData = (PFN_vkGetPrivateData) load(userptr, "vkGetPrivateData");
    glad_vkQueueSubmit2 = (PFN_vkQueueSubmit2) load(userptr, "vkQueueSubmit2");
    glad_vkSetPrivateData = (PFN_vkSetPrivateData) load(userptr, "vkSetPrivateData");
}
static void glad_vk_load_VK_VERSION_1_4( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_VERSION_1_4) return;
    glad_vkCmdBindDescriptorSets2 = (PFN_vkCmdBindDescriptorSets2) load(userptr, "vkCmdBindDescriptorSets2");
    glad_vkCmdBindIndexBuffer2 = (PFN_vkCmdBindIndexBuffer2) load(userptr, "vkCmdBindIndexBuffer2");
    glad_vkCmdPushConstants2 = (PFN_vkCmdPushConstants2) load(userptr, "vkCmdPushConstants2");
    glad_vkCmdPushDescriptorSet = (PFN_vkCmdPushDescriptorSet) load(userptr, "vkCmdPushDescriptorSet");
    glad_vkCmdPushDescriptorSet2 = (PFN_vkCmdPushDescriptorSet2) load(userptr, "vkCmdPushDescriptorSet2");
    glad_vkCmdPushDescriptorSetWithTemplate = (PFN_vkCmdPushDescriptorSetWithTemplate) load(userptr, "vkCmdPushDescriptorSetWithTemplate");
    glad_vkCmdPushDescriptorSetWithTemplate2 = (PFN_vkCmdPushDescriptorSetWithTemplate2) load(userptr, "vkCmdPushDescriptorSetWithTemplate2");
    glad_vkCmdSetLineStipple = (PFN_vkCmdSetLineStipple) load(userptr, "vkCmdSetLineStipple");
    glad_vkCmdSetRenderingAttachmentLocations = (PFN_vkCmdSetRenderingAttachmentLocations) load(userptr, "vkCmdSetRenderingAttachmentLocations");
    glad_vkCmdSetRenderingInputAttachmentIndices = (PFN_vkCmdSetRenderingInputAttachmentIndices) load(userptr, "vkCmdSetRenderingInputAttachmentIndices");
    glad_vkCopyImageToImage = (PFN_vkCopyImageToImage) load(userptr, "vkCopyImageToImage");
    glad_vkCopyImageToMemory = (PFN_vkCopyImageToMemory) load(userptr, "vkCopyImageToMemory");
    glad_vkCopyMemoryToImage = (PFN_vkCopyMemoryToImage) load(userptr, "vkCopyMemoryToImage");
    glad_vkGetDeviceImageSubresourceLayout = (PFN_vkGetDeviceImageSubresourceLayout) load(userptr, "vkGetDeviceImageSubresourceLayout");
    glad_vkGetImageSubresourceLayout2 = (PFN_vkGetImageSubresourceLayout2) load(userptr, "vkGetImageSubresourceLayout2");
    glad_vkGetRenderingAreaGranularity = (PFN_vkGetRenderingAreaGranularity) load(userptr, "vkGetRenderingAreaGranularity");
    glad_vkMapMemory2 = (PFN_vkMapMemory2) load(userptr, "vkMapMemory2");
    glad_vkTransitionImageLayout = (PFN_vkTransitionImageLayout) load(userptr, "vkTransitionImageLayout");
    glad_vkUnmapMemory2 = (PFN_vkUnmapMemory2) load(userptr, "vkUnmapMemory2");
}
#if defined(VK_ENABLE_BETA_EXTENSIONS)
static void glad_vk_load_VK_AMDX_shader_enqueue( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_AMDX_shader_enqueue) return;
    glad_vkCmdDispatchGraphAMDX = (PFN_vkCmdDispatchGraphAMDX) load(userptr, "vkCmdDispatchGraphAMDX");
    glad_vkCmdDispatchGraphIndirectAMDX = (PFN_vkCmdDispatchGraphIndirectAMDX) load(userptr, "vkCmdDispatchGraphIndirectAMDX");
    glad_vkCmdDispatchGraphIndirectCountAMDX = (PFN_vkCmdDispatchGraphIndirectCountAMDX) load(userptr, "vkCmdDispatchGraphIndirectCountAMDX");
    glad_vkCmdInitializeGraphScratchMemoryAMDX = (PFN_vkCmdInitializeGraphScratchMemoryAMDX) load(userptr, "vkCmdInitializeGraphScratchMemoryAMDX");
    glad_vkCreateExecutionGraphPipelinesAMDX = (PFN_vkCreateExecutionGraphPipelinesAMDX) load(userptr, "vkCreateExecutionGraphPipelinesAMDX");
    glad_vkGetExecutionGraphPipelineNodeIndexAMDX = (PFN_vkGetExecutionGraphPipelineNodeIndexAMDX) load(userptr, "vkGetExecutionGraphPipelineNodeIndexAMDX");
    glad_vkGetExecutionGraphPipelineScratchSizeAMDX = (PFN_vkGetExecutionGraphPipelineScratchSizeAMDX) load(userptr, "vkGetExecutionGraphPipelineScratchSizeAMDX");
}

#endif
static void glad_vk_load_VK_AMD_anti_lag( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_AMD_anti_lag) return;
    glad_vkAntiLagUpdateAMD = (PFN_vkAntiLagUpdateAMD) load(userptr, "vkAntiLagUpdateAMD");
}
static void glad_vk_load_VK_AMD_buffer_marker( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_AMD_buffer_marker) return;
    glad_vkCmdWriteBufferMarker2AMD = (PFN_vkCmdWriteBufferMarker2AMD) load(userptr, "vkCmdWriteBufferMarker2AMD");
    glad_vkCmdWriteBufferMarkerAMD = (PFN_vkCmdWriteBufferMarkerAMD) load(userptr, "vkCmdWriteBufferMarkerAMD");
}
static void glad_vk_load_VK_AMD_display_native_hdr( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_AMD_display_native_hdr) return;
    glad_vkSetLocalDimmingAMD = (PFN_vkSetLocalDimmingAMD) load(userptr, "vkSetLocalDimmingAMD");
}
static void glad_vk_load_VK_AMD_draw_indirect_count( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_AMD_draw_indirect_count) return;
    glad_vkCmdDrawIndexedIndirectCountAMD = (PFN_vkCmdDrawIndexedIndirectCountAMD) load(userptr, "vkCmdDrawIndexedIndirectCountAMD");
    glad_vkCmdDrawIndirectCountAMD = (PFN_vkCmdDrawIndirectCountAMD) load(userptr, "vkCmdDrawIndirectCountAMD");
}
static void glad_vk_load_VK_AMD_shader_info( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_AMD_shader_info) return;
    glad_vkGetShaderInfoAMD = (PFN_vkGetShaderInfoAMD) load(userptr, "vkGetShaderInfoAMD");
}
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
static void glad_vk_load_VK_ANDROID_external_memory_android_hardware_buffer( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_ANDROID_external_memory_android_hardware_buffer) return;
    glad_vkGetAndroidHardwareBufferPropertiesANDROID = (PFN_vkGetAndroidHardwareBufferPropertiesANDROID) load(userptr, "vkGetAndroidHardwareBufferPropertiesANDROID");
    glad_vkGetMemoryAndroidHardwareBufferANDROID = (PFN_vkGetMemoryAndroidHardwareBufferANDROID) load(userptr, "vkGetMemoryAndroidHardwareBufferANDROID");
}

#endif
static void glad_vk_load_VK_EXT_acquire_drm_display( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_acquire_drm_display) return;
    glad_vkAcquireDrmDisplayEXT = (PFN_vkAcquireDrmDisplayEXT) load(userptr, "vkAcquireDrmDisplayEXT");
    glad_vkGetDrmDisplayEXT = (PFN_vkGetDrmDisplayEXT) load(userptr, "vkGetDrmDisplayEXT");
}
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
static void glad_vk_load_VK_EXT_acquire_xlib_display( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_acquire_xlib_display) return;
    glad_vkAcquireXlibDisplayEXT = (PFN_vkAcquireXlibDisplayEXT) load(userptr, "vkAcquireXlibDisplayEXT");
    glad_vkGetRandROutputDisplayEXT = (PFN_vkGetRandROutputDisplayEXT) load(userptr, "vkGetRandROutputDisplayEXT");
}

#endif
static void glad_vk_load_VK_EXT_attachment_feedback_loop_dynamic_state( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_attachment_feedback_loop_dynamic_state) return;
    glad_vkCmdSetAttachmentFeedbackLoopEnableEXT = (PFN_vkCmdSetAttachmentFeedbackLoopEnableEXT) load(userptr, "vkCmdSetAttachmentFeedbackLoopEnableEXT");
}
static void glad_vk_load_VK_EXT_buffer_device_address( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_buffer_device_address) return;
    glad_vkGetBufferDeviceAddressEXT = (PFN_vkGetBufferDeviceAddressEXT) load(userptr, "vkGetBufferDeviceAddressEXT");
}
static void glad_vk_load_VK_EXT_calibrated_timestamps( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_calibrated_timestamps) return;
    glad_vkGetCalibratedTimestampsEXT = (PFN_vkGetCalibratedTimestampsEXT) load(userptr, "vkGetCalibratedTimestampsEXT");
    glad_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT = (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT) load(userptr, "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");
}
static void glad_vk_load_VK_EXT_color_write_enable( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_color_write_enable) return;
    glad_vkCmdSetColorWriteEnableEXT = (PFN_vkCmdSetColorWriteEnableEXT) load(userptr, "vkCmdSetColorWriteEnableEXT");
}
static void glad_vk_load_VK_EXT_conditional_rendering( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_conditional_rendering) return;
    glad_vkCmdBeginConditionalRenderingEXT = (PFN_vkCmdBeginConditionalRenderingEXT) load(userptr, "vkCmdBeginConditionalRenderingEXT");
    glad_vkCmdEndConditionalRenderingEXT = (PFN_vkCmdEndConditionalRenderingEXT) load(userptr, "vkCmdEndConditionalRenderingEXT");
}
static void glad_vk_load_VK_EXT_debug_marker( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_debug_marker) return;
    glad_vkCmdDebugMarkerBeginEXT = (PFN_vkCmdDebugMarkerBeginEXT) load(userptr, "vkCmdDebugMarkerBeginEXT");
    glad_vkCmdDebugMarkerEndEXT = (PFN_vkCmdDebugMarkerEndEXT) load(userptr, "vkCmdDebugMarkerEndEXT");
    glad_vkCmdDebugMarkerInsertEXT = (PFN_vkCmdDebugMarkerInsertEXT) load(userptr, "vkCmdDebugMarkerInsertEXT");
    glad_vkDebugMarkerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT) load(userptr, "vkDebugMarkerSetObjectNameEXT");
    glad_vkDebugMarkerSetObjectTagEXT = (PFN_vkDebugMarkerSetObjectTagEXT) load(userptr, "vkDebugMarkerSetObjectTagEXT");
}
static void glad_vk_load_VK_EXT_debug_report( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_debug_report) return;
    glad_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) load(userptr, "vkCreateDebugReportCallbackEXT");
    glad_vkDebugReportMessageEXT = (PFN_vkDebugReportMessageEXT) load(userptr, "vkDebugReportMessageEXT");
    glad_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) load(userptr, "vkDestroyDebugReportCallbackEXT");
}
static void glad_vk_load_VK_EXT_debug_utils( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_debug_utils) return;
    glad_vkCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT) load(userptr, "vkCmdBeginDebugUtilsLabelEXT");
    glad_vkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT) load(userptr, "vkCmdEndDebugUtilsLabelEXT");
    glad_vkCmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT) load(userptr, "vkCmdInsertDebugUtilsLabelEXT");
    glad_vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT) load(userptr, "vkCreateDebugUtilsMessengerEXT");
    glad_vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT) load(userptr, "vkDestroyDebugUtilsMessengerEXT");
    glad_vkQueueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT) load(userptr, "vkQueueBeginDebugUtilsLabelEXT");
    glad_vkQueueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT) load(userptr, "vkQueueEndDebugUtilsLabelEXT");
    glad_vkQueueInsertDebugUtilsLabelEXT = (PFN_vkQueueInsertDebugUtilsLabelEXT) load(userptr, "vkQueueInsertDebugUtilsLabelEXT");
    glad_vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT) load(userptr, "vkSetDebugUtilsObjectNameEXT");
    glad_vkSetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT) load(userptr, "vkSetDebugUtilsObjectTagEXT");
    glad_vkSubmitDebugUtilsMessageEXT = (PFN_vkSubmitDebugUtilsMessageEXT) load(userptr, "vkSubmitDebugUtilsMessageEXT");
}
static void glad_vk_load_VK_EXT_depth_bias_control( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_depth_bias_control) return;
    glad_vkCmdSetDepthBias2EXT = (PFN_vkCmdSetDepthBias2EXT) load(userptr, "vkCmdSetDepthBias2EXT");
}
static void glad_vk_load_VK_EXT_depth_clamp_control( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_depth_clamp_control) return;
    glad_vkCmdSetDepthClampRangeEXT = (PFN_vkCmdSetDepthClampRangeEXT) load(userptr, "vkCmdSetDepthClampRangeEXT");
}
static void glad_vk_load_VK_EXT_descriptor_buffer( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_descriptor_buffer) return;
    glad_vkCmdBindDescriptorBufferEmbeddedSamplersEXT = (PFN_vkCmdBindDescriptorBufferEmbeddedSamplersEXT) load(userptr, "vkCmdBindDescriptorBufferEmbeddedSamplersEXT");
    glad_vkCmdBindDescriptorBuffersEXT = (PFN_vkCmdBindDescriptorBuffersEXT) load(userptr, "vkCmdBindDescriptorBuffersEXT");
    glad_vkCmdSetDescriptorBufferOffsetsEXT = (PFN_vkCmdSetDescriptorBufferOffsetsEXT) load(userptr, "vkCmdSetDescriptorBufferOffsetsEXT");
    glad_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT = (PFN_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT) load(userptr, "vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT");
    glad_vkGetBufferOpaqueCaptureDescriptorDataEXT = (PFN_vkGetBufferOpaqueCaptureDescriptorDataEXT) load(userptr, "vkGetBufferOpaqueCaptureDescriptorDataEXT");
    glad_vkGetDescriptorEXT = (PFN_vkGetDescriptorEXT) load(userptr, "vkGetDescriptorEXT");
    glad_vkGetDescriptorSetLayoutBindingOffsetEXT = (PFN_vkGetDescriptorSetLayoutBindingOffsetEXT) load(userptr, "vkGetDescriptorSetLayoutBindingOffsetEXT");
    glad_vkGetDescriptorSetLayoutSizeEXT = (PFN_vkGetDescriptorSetLayoutSizeEXT) load(userptr, "vkGetDescriptorSetLayoutSizeEXT");
    glad_vkGetImageOpaqueCaptureDescriptorDataEXT = (PFN_vkGetImageOpaqueCaptureDescriptorDataEXT) load(userptr, "vkGetImageOpaqueCaptureDescriptorDataEXT");
    glad_vkGetImageViewOpaqueCaptureDescriptorDataEXT = (PFN_vkGetImageViewOpaqueCaptureDescriptorDataEXT) load(userptr, "vkGetImageViewOpaqueCaptureDescriptorDataEXT");
    glad_vkGetSamplerOpaqueCaptureDescriptorDataEXT = (PFN_vkGetSamplerOpaqueCaptureDescriptorDataEXT) load(userptr, "vkGetSamplerOpaqueCaptureDescriptorDataEXT");
}
static void glad_vk_load_VK_EXT_device_fault( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_device_fault) return;
    glad_vkGetDeviceFaultInfoEXT = (PFN_vkGetDeviceFaultInfoEXT) load(userptr, "vkGetDeviceFaultInfoEXT");
}
static void glad_vk_load_VK_EXT_device_generated_commands( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_device_generated_commands) return;
    glad_vkCmdExecuteGeneratedCommandsEXT = (PFN_vkCmdExecuteGeneratedCommandsEXT) load(userptr, "vkCmdExecuteGeneratedCommandsEXT");
    glad_vkCmdPreprocessGeneratedCommandsEXT = (PFN_vkCmdPreprocessGeneratedCommandsEXT) load(userptr, "vkCmdPreprocessGeneratedCommandsEXT");
    glad_vkCreateIndirectCommandsLayoutEXT = (PFN_vkCreateIndirectCommandsLayoutEXT) load(userptr, "vkCreateIndirectCommandsLayoutEXT");
    glad_vkCreateIndirectExecutionSetEXT = (PFN_vkCreateIndirectExecutionSetEXT) load(userptr, "vkCreateIndirectExecutionSetEXT");
    glad_vkDestroyIndirectCommandsLayoutEXT = (PFN_vkDestroyIndirectCommandsLayoutEXT) load(userptr, "vkDestroyIndirectCommandsLayoutEXT");
    glad_vkDestroyIndirectExecutionSetEXT = (PFN_vkDestroyIndirectExecutionSetEXT) load(userptr, "vkDestroyIndirectExecutionSetEXT");
    glad_vkGetGeneratedCommandsMemoryRequirementsEXT = (PFN_vkGetGeneratedCommandsMemoryRequirementsEXT) load(userptr, "vkGetGeneratedCommandsMemoryRequirementsEXT");
    glad_vkUpdateIndirectExecutionSetPipelineEXT = (PFN_vkUpdateIndirectExecutionSetPipelineEXT) load(userptr, "vkUpdateIndirectExecutionSetPipelineEXT");
    glad_vkUpdateIndirectExecutionSetShaderEXT = (PFN_vkUpdateIndirectExecutionSetShaderEXT) load(userptr, "vkUpdateIndirectExecutionSetShaderEXT");
}
static void glad_vk_load_VK_EXT_direct_mode_display( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_direct_mode_display) return;
    glad_vkReleaseDisplayEXT = (PFN_vkReleaseDisplayEXT) load(userptr, "vkReleaseDisplayEXT");
}
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
static void glad_vk_load_VK_EXT_directfb_surface( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_directfb_surface) return;
    glad_vkCreateDirectFBSurfaceEXT = (PFN_vkCreateDirectFBSurfaceEXT) load(userptr, "vkCreateDirectFBSurfaceEXT");
    glad_vkGetPhysicalDeviceDirectFBPresentationSupportEXT = (PFN_vkGetPhysicalDeviceDirectFBPresentationSupportEXT) load(userptr, "vkGetPhysicalDeviceDirectFBPresentationSupportEXT");
}

#endif
static void glad_vk_load_VK_EXT_discard_rectangles( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_discard_rectangles) return;
    glad_vkCmdSetDiscardRectangleEXT = (PFN_vkCmdSetDiscardRectangleEXT) load(userptr, "vkCmdSetDiscardRectangleEXT");
    glad_vkCmdSetDiscardRectangleEnableEXT = (PFN_vkCmdSetDiscardRectangleEnableEXT) load(userptr, "vkCmdSetDiscardRectangleEnableEXT");
    glad_vkCmdSetDiscardRectangleModeEXT = (PFN_vkCmdSetDiscardRectangleModeEXT) load(userptr, "vkCmdSetDiscardRectangleModeEXT");
}
static void glad_vk_load_VK_EXT_display_control( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_display_control) return;
    glad_vkDisplayPowerControlEXT = (PFN_vkDisplayPowerControlEXT) load(userptr, "vkDisplayPowerControlEXT");
    glad_vkGetSwapchainCounterEXT = (PFN_vkGetSwapchainCounterEXT) load(userptr, "vkGetSwapchainCounterEXT");
    glad_vkRegisterDeviceEventEXT = (PFN_vkRegisterDeviceEventEXT) load(userptr, "vkRegisterDeviceEventEXT");
    glad_vkRegisterDisplayEventEXT = (PFN_vkRegisterDisplayEventEXT) load(userptr, "vkRegisterDisplayEventEXT");
}
static void glad_vk_load_VK_EXT_display_surface_counter( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_display_surface_counter) return;
    glad_vkGetPhysicalDeviceSurfaceCapabilities2EXT = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT) load(userptr, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
}
static void glad_vk_load_VK_EXT_extended_dynamic_state( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_extended_dynamic_state) return;
    glad_vkCmdBindVertexBuffers2EXT = (PFN_vkCmdBindVertexBuffers2EXT) load(userptr, "vkCmdBindVertexBuffers2EXT");
    glad_vkCmdSetCullModeEXT = (PFN_vkCmdSetCullModeEXT) load(userptr, "vkCmdSetCullModeEXT");
    glad_vkCmdSetDepthBoundsTestEnableEXT = (PFN_vkCmdSetDepthBoundsTestEnableEXT) load(userptr, "vkCmdSetDepthBoundsTestEnableEXT");
    glad_vkCmdSetDepthCompareOpEXT = (PFN_vkCmdSetDepthCompareOpEXT) load(userptr, "vkCmdSetDepthCompareOpEXT");
    glad_vkCmdSetDepthTestEnableEXT = (PFN_vkCmdSetDepthTestEnableEXT) load(userptr, "vkCmdSetDepthTestEnableEXT");
    glad_vkCmdSetDepthWriteEnableEXT = (PFN_vkCmdSetDepthWriteEnableEXT) load(userptr, "vkCmdSetDepthWriteEnableEXT");
    glad_vkCmdSetFrontFaceEXT = (PFN_vkCmdSetFrontFaceEXT) load(userptr, "vkCmdSetFrontFaceEXT");
    glad_vkCmdSetPrimitiveTopologyEXT = (PFN_vkCmdSetPrimitiveTopologyEXT) load(userptr, "vkCmdSetPrimitiveTopologyEXT");
    glad_vkCmdSetScissorWithCountEXT = (PFN_vkCmdSetScissorWithCountEXT) load(userptr, "vkCmdSetScissorWithCountEXT");
    glad_vkCmdSetStencilOpEXT = (PFN_vkCmdSetStencilOpEXT) load(userptr, "vkCmdSetStencilOpEXT");
    glad_vkCmdSetStencilTestEnableEXT = (PFN_vkCmdSetStencilTestEnableEXT) load(userptr, "vkCmdSetStencilTestEnableEXT");
    glad_vkCmdSetViewportWithCountEXT = (PFN_vkCmdSetViewportWithCountEXT) load(userptr, "vkCmdSetViewportWithCountEXT");
}
static void glad_vk_load_VK_EXT_extended_dynamic_state2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_extended_dynamic_state2) return;
    glad_vkCmdSetDepthBiasEnableEXT = (PFN_vkCmdSetDepthBiasEnableEXT) load(userptr, "vkCmdSetDepthBiasEnableEXT");
    glad_vkCmdSetLogicOpEXT = (PFN_vkCmdSetLogicOpEXT) load(userptr, "vkCmdSetLogicOpEXT");
    glad_vkCmdSetPatchControlPointsEXT = (PFN_vkCmdSetPatchControlPointsEXT) load(userptr, "vkCmdSetPatchControlPointsEXT");
    glad_vkCmdSetPrimitiveRestartEnableEXT = (PFN_vkCmdSetPrimitiveRestartEnableEXT) load(userptr, "vkCmdSetPrimitiveRestartEnableEXT");
    glad_vkCmdSetRasterizerDiscardEnableEXT = (PFN_vkCmdSetRasterizerDiscardEnableEXT) load(userptr, "vkCmdSetRasterizerDiscardEnableEXT");
}
static void glad_vk_load_VK_EXT_extended_dynamic_state3( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_extended_dynamic_state3) return;
    glad_vkCmdSetAlphaToCoverageEnableEXT = (PFN_vkCmdSetAlphaToCoverageEnableEXT) load(userptr, "vkCmdSetAlphaToCoverageEnableEXT");
    glad_vkCmdSetAlphaToOneEnableEXT = (PFN_vkCmdSetAlphaToOneEnableEXT) load(userptr, "vkCmdSetAlphaToOneEnableEXT");
    glad_vkCmdSetColorBlendAdvancedEXT = (PFN_vkCmdSetColorBlendAdvancedEXT) load(userptr, "vkCmdSetColorBlendAdvancedEXT");
    glad_vkCmdSetColorBlendEnableEXT = (PFN_vkCmdSetColorBlendEnableEXT) load(userptr, "vkCmdSetColorBlendEnableEXT");
    glad_vkCmdSetColorBlendEquationEXT = (PFN_vkCmdSetColorBlendEquationEXT) load(userptr, "vkCmdSetColorBlendEquationEXT");
    glad_vkCmdSetColorWriteMaskEXT = (PFN_vkCmdSetColorWriteMaskEXT) load(userptr, "vkCmdSetColorWriteMaskEXT");
    glad_vkCmdSetConservativeRasterizationModeEXT = (PFN_vkCmdSetConservativeRasterizationModeEXT) load(userptr, "vkCmdSetConservativeRasterizationModeEXT");
    glad_vkCmdSetCoverageModulationModeNV = (PFN_vkCmdSetCoverageModulationModeNV) load(userptr, "vkCmdSetCoverageModulationModeNV");
    glad_vkCmdSetCoverageModulationTableEnableNV = (PFN_vkCmdSetCoverageModulationTableEnableNV) load(userptr, "vkCmdSetCoverageModulationTableEnableNV");
    glad_vkCmdSetCoverageModulationTableNV = (PFN_vkCmdSetCoverageModulationTableNV) load(userptr, "vkCmdSetCoverageModulationTableNV");
    glad_vkCmdSetCoverageReductionModeNV = (PFN_vkCmdSetCoverageReductionModeNV) load(userptr, "vkCmdSetCoverageReductionModeNV");
    glad_vkCmdSetCoverageToColorEnableNV = (PFN_vkCmdSetCoverageToColorEnableNV) load(userptr, "vkCmdSetCoverageToColorEnableNV");
    glad_vkCmdSetCoverageToColorLocationNV = (PFN_vkCmdSetCoverageToColorLocationNV) load(userptr, "vkCmdSetCoverageToColorLocationNV");
    glad_vkCmdSetDepthClampEnableEXT = (PFN_vkCmdSetDepthClampEnableEXT) load(userptr, "vkCmdSetDepthClampEnableEXT");
    glad_vkCmdSetDepthClipEnableEXT = (PFN_vkCmdSetDepthClipEnableEXT) load(userptr, "vkCmdSetDepthClipEnableEXT");
    glad_vkCmdSetDepthClipNegativeOneToOneEXT = (PFN_vkCmdSetDepthClipNegativeOneToOneEXT) load(userptr, "vkCmdSetDepthClipNegativeOneToOneEXT");
    glad_vkCmdSetExtraPrimitiveOverestimationSizeEXT = (PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT) load(userptr, "vkCmdSetExtraPrimitiveOverestimationSizeEXT");
    glad_vkCmdSetLineRasterizationModeEXT = (PFN_vkCmdSetLineRasterizationModeEXT) load(userptr, "vkCmdSetLineRasterizationModeEXT");
    glad_vkCmdSetLineStippleEnableEXT = (PFN_vkCmdSetLineStippleEnableEXT) load(userptr, "vkCmdSetLineStippleEnableEXT");
    glad_vkCmdSetLogicOpEnableEXT = (PFN_vkCmdSetLogicOpEnableEXT) load(userptr, "vkCmdSetLogicOpEnableEXT");
    glad_vkCmdSetPolygonModeEXT = (PFN_vkCmdSetPolygonModeEXT) load(userptr, "vkCmdSetPolygonModeEXT");
    glad_vkCmdSetProvokingVertexModeEXT = (PFN_vkCmdSetProvokingVertexModeEXT) load(userptr, "vkCmdSetProvokingVertexModeEXT");
    glad_vkCmdSetRasterizationSamplesEXT = (PFN_vkCmdSetRasterizationSamplesEXT) load(userptr, "vkCmdSetRasterizationSamplesEXT");
    glad_vkCmdSetRasterizationStreamEXT = (PFN_vkCmdSetRasterizationStreamEXT) load(userptr, "vkCmdSetRasterizationStreamEXT");
    glad_vkCmdSetRepresentativeFragmentTestEnableNV = (PFN_vkCmdSetRepresentativeFragmentTestEnableNV) load(userptr, "vkCmdSetRepresentativeFragmentTestEnableNV");
    glad_vkCmdSetSampleLocationsEnableEXT = (PFN_vkCmdSetSampleLocationsEnableEXT) load(userptr, "vkCmdSetSampleLocationsEnableEXT");
    glad_vkCmdSetSampleMaskEXT = (PFN_vkCmdSetSampleMaskEXT) load(userptr, "vkCmdSetSampleMaskEXT");
    glad_vkCmdSetShadingRateImageEnableNV = (PFN_vkCmdSetShadingRateImageEnableNV) load(userptr, "vkCmdSetShadingRateImageEnableNV");
    glad_vkCmdSetTessellationDomainOriginEXT = (PFN_vkCmdSetTessellationDomainOriginEXT) load(userptr, "vkCmdSetTessellationDomainOriginEXT");
    glad_vkCmdSetViewportSwizzleNV = (PFN_vkCmdSetViewportSwizzleNV) load(userptr, "vkCmdSetViewportSwizzleNV");
    glad_vkCmdSetViewportWScalingEnableNV = (PFN_vkCmdSetViewportWScalingEnableNV) load(userptr, "vkCmdSetViewportWScalingEnableNV");
}
static void glad_vk_load_VK_EXT_external_memory_host( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_external_memory_host) return;
    glad_vkGetMemoryHostPointerPropertiesEXT = (PFN_vkGetMemoryHostPointerPropertiesEXT) load(userptr, "vkGetMemoryHostPointerPropertiesEXT");
}
#if defined(VK_USE_PLATFORM_METAL_EXT)
static void glad_vk_load_VK_EXT_external_memory_metal( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_external_memory_metal) return;
    glad_vkGetMemoryMetalHandleEXT = (PFN_vkGetMemoryMetalHandleEXT) load(userptr, "vkGetMemoryMetalHandleEXT");
    glad_vkGetMemoryMetalHandlePropertiesEXT = (PFN_vkGetMemoryMetalHandlePropertiesEXT) load(userptr, "vkGetMemoryMetalHandlePropertiesEXT");
}

#endif
static void glad_vk_load_VK_EXT_fragment_density_map_offset( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_fragment_density_map_offset) return;
    glad_vkCmdEndRendering2EXT = (PFN_vkCmdEndRendering2EXT) load(userptr, "vkCmdEndRendering2EXT");
}
#if defined(VK_USE_PLATFORM_WIN32_KHR)
static void glad_vk_load_VK_EXT_full_screen_exclusive( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_full_screen_exclusive) return;
    glad_vkAcquireFullScreenExclusiveModeEXT = (PFN_vkAcquireFullScreenExclusiveModeEXT) load(userptr, "vkAcquireFullScreenExclusiveModeEXT");
    glad_vkGetDeviceGroupSurfacePresentModes2EXT = (PFN_vkGetDeviceGroupSurfacePresentModes2EXT) load(userptr, "vkGetDeviceGroupSurfacePresentModes2EXT");
    glad_vkGetPhysicalDeviceSurfacePresentModes2EXT = (PFN_vkGetPhysicalDeviceSurfacePresentModes2EXT) load(userptr, "vkGetPhysicalDeviceSurfacePresentModes2EXT");
    glad_vkReleaseFullScreenExclusiveModeEXT = (PFN_vkReleaseFullScreenExclusiveModeEXT) load(userptr, "vkReleaseFullScreenExclusiveModeEXT");
}

#endif
static void glad_vk_load_VK_EXT_hdr_metadata( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_hdr_metadata) return;
    glad_vkSetHdrMetadataEXT = (PFN_vkSetHdrMetadataEXT) load(userptr, "vkSetHdrMetadataEXT");
}
static void glad_vk_load_VK_EXT_headless_surface( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_headless_surface) return;
    glad_vkCreateHeadlessSurfaceEXT = (PFN_vkCreateHeadlessSurfaceEXT) load(userptr, "vkCreateHeadlessSurfaceEXT");
}
static void glad_vk_load_VK_EXT_host_image_copy( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_host_image_copy) return;
    glad_vkCopyImageToImageEXT = (PFN_vkCopyImageToImageEXT) load(userptr, "vkCopyImageToImageEXT");
    glad_vkCopyImageToMemoryEXT = (PFN_vkCopyImageToMemoryEXT) load(userptr, "vkCopyImageToMemoryEXT");
    glad_vkCopyMemoryToImageEXT = (PFN_vkCopyMemoryToImageEXT) load(userptr, "vkCopyMemoryToImageEXT");
    glad_vkGetImageSubresourceLayout2EXT = (PFN_vkGetImageSubresourceLayout2EXT) load(userptr, "vkGetImageSubresourceLayout2EXT");
    glad_vkTransitionImageLayoutEXT = (PFN_vkTransitionImageLayoutEXT) load(userptr, "vkTransitionImageLayoutEXT");
}
static void glad_vk_load_VK_EXT_host_query_reset( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_host_query_reset) return;
    glad_vkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT) load(userptr, "vkResetQueryPoolEXT");
}
static void glad_vk_load_VK_EXT_image_compression_control( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_image_compression_control) return;
    glad_vkGetImageSubresourceLayout2EXT = (PFN_vkGetImageSubresourceLayout2EXT) load(userptr, "vkGetImageSubresourceLayout2EXT");
}
static void glad_vk_load_VK_EXT_image_drm_format_modifier( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_image_drm_format_modifier) return;
    glad_vkGetImageDrmFormatModifierPropertiesEXT = (PFN_vkGetImageDrmFormatModifierPropertiesEXT) load(userptr, "vkGetImageDrmFormatModifierPropertiesEXT");
}
static void glad_vk_load_VK_EXT_line_rasterization( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_line_rasterization) return;
    glad_vkCmdSetLineStippleEXT = (PFN_vkCmdSetLineStippleEXT) load(userptr, "vkCmdSetLineStippleEXT");
}
static void glad_vk_load_VK_EXT_mesh_shader( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_mesh_shader) return;
    glad_vkCmdDrawMeshTasksEXT = (PFN_vkCmdDrawMeshTasksEXT) load(userptr, "vkCmdDrawMeshTasksEXT");
    glad_vkCmdDrawMeshTasksIndirectCountEXT = (PFN_vkCmdDrawMeshTasksIndirectCountEXT) load(userptr, "vkCmdDrawMeshTasksIndirectCountEXT");
    glad_vkCmdDrawMeshTasksIndirectEXT = (PFN_vkCmdDrawMeshTasksIndirectEXT) load(userptr, "vkCmdDrawMeshTasksIndirectEXT");
}
#if defined(VK_USE_PLATFORM_METAL_EXT)
static void glad_vk_load_VK_EXT_metal_objects( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_metal_objects) return;
    glad_vkExportMetalObjectsEXT = (PFN_vkExportMetalObjectsEXT) load(userptr, "vkExportMetalObjectsEXT");
}

#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
static void glad_vk_load_VK_EXT_metal_surface( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_metal_surface) return;
    glad_vkCreateMetalSurfaceEXT = (PFN_vkCreateMetalSurfaceEXT) load(userptr, "vkCreateMetalSurfaceEXT");
}

#endif
static void glad_vk_load_VK_EXT_multi_draw( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_multi_draw) return;
    glad_vkCmdDrawMultiEXT = (PFN_vkCmdDrawMultiEXT) load(userptr, "vkCmdDrawMultiEXT");
    glad_vkCmdDrawMultiIndexedEXT = (PFN_vkCmdDrawMultiIndexedEXT) load(userptr, "vkCmdDrawMultiIndexedEXT");
}
static void glad_vk_load_VK_EXT_opacity_micromap( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_opacity_micromap) return;
    glad_vkBuildMicromapsEXT = (PFN_vkBuildMicromapsEXT) load(userptr, "vkBuildMicromapsEXT");
    glad_vkCmdBuildMicromapsEXT = (PFN_vkCmdBuildMicromapsEXT) load(userptr, "vkCmdBuildMicromapsEXT");
    glad_vkCmdCopyMemoryToMicromapEXT = (PFN_vkCmdCopyMemoryToMicromapEXT) load(userptr, "vkCmdCopyMemoryToMicromapEXT");
    glad_vkCmdCopyMicromapEXT = (PFN_vkCmdCopyMicromapEXT) load(userptr, "vkCmdCopyMicromapEXT");
    glad_vkCmdCopyMicromapToMemoryEXT = (PFN_vkCmdCopyMicromapToMemoryEXT) load(userptr, "vkCmdCopyMicromapToMemoryEXT");
    glad_vkCmdWriteMicromapsPropertiesEXT = (PFN_vkCmdWriteMicromapsPropertiesEXT) load(userptr, "vkCmdWriteMicromapsPropertiesEXT");
    glad_vkCopyMemoryToMicromapEXT = (PFN_vkCopyMemoryToMicromapEXT) load(userptr, "vkCopyMemoryToMicromapEXT");
    glad_vkCopyMicromapEXT = (PFN_vkCopyMicromapEXT) load(userptr, "vkCopyMicromapEXT");
    glad_vkCopyMicromapToMemoryEXT = (PFN_vkCopyMicromapToMemoryEXT) load(userptr, "vkCopyMicromapToMemoryEXT");
    glad_vkCreateMicromapEXT = (PFN_vkCreateMicromapEXT) load(userptr, "vkCreateMicromapEXT");
    glad_vkDestroyMicromapEXT = (PFN_vkDestroyMicromapEXT) load(userptr, "vkDestroyMicromapEXT");
    glad_vkGetDeviceMicromapCompatibilityEXT = (PFN_vkGetDeviceMicromapCompatibilityEXT) load(userptr, "vkGetDeviceMicromapCompatibilityEXT");
    glad_vkGetMicromapBuildSizesEXT = (PFN_vkGetMicromapBuildSizesEXT) load(userptr, "vkGetMicromapBuildSizesEXT");
    glad_vkWriteMicromapsPropertiesEXT = (PFN_vkWriteMicromapsPropertiesEXT) load(userptr, "vkWriteMicromapsPropertiesEXT");
}
static void glad_vk_load_VK_EXT_pageable_device_local_memory( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_pageable_device_local_memory) return;
    glad_vkSetDeviceMemoryPriorityEXT = (PFN_vkSetDeviceMemoryPriorityEXT) load(userptr, "vkSetDeviceMemoryPriorityEXT");
}
static void glad_vk_load_VK_EXT_pipeline_properties( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_pipeline_properties) return;
    glad_vkGetPipelinePropertiesEXT = (PFN_vkGetPipelinePropertiesEXT) load(userptr, "vkGetPipelinePropertiesEXT");
}
static void glad_vk_load_VK_EXT_private_data( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_private_data) return;
    glad_vkCreatePrivateDataSlotEXT = (PFN_vkCreatePrivateDataSlotEXT) load(userptr, "vkCreatePrivateDataSlotEXT");
    glad_vkDestroyPrivateDataSlotEXT = (PFN_vkDestroyPrivateDataSlotEXT) load(userptr, "vkDestroyPrivateDataSlotEXT");
    glad_vkGetPrivateDataEXT = (PFN_vkGetPrivateDataEXT) load(userptr, "vkGetPrivateDataEXT");
    glad_vkSetPrivateDataEXT = (PFN_vkSetPrivateDataEXT) load(userptr, "vkSetPrivateDataEXT");
}
static void glad_vk_load_VK_EXT_sample_locations( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_sample_locations) return;
    glad_vkCmdSetSampleLocationsEXT = (PFN_vkCmdSetSampleLocationsEXT) load(userptr, "vkCmdSetSampleLocationsEXT");
    glad_vkGetPhysicalDeviceMultisamplePropertiesEXT = (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT) load(userptr, "vkGetPhysicalDeviceMultisamplePropertiesEXT");
}
static void glad_vk_load_VK_EXT_shader_module_identifier( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_shader_module_identifier) return;
    glad_vkGetShaderModuleCreateInfoIdentifierEXT = (PFN_vkGetShaderModuleCreateInfoIdentifierEXT) load(userptr, "vkGetShaderModuleCreateInfoIdentifierEXT");
    glad_vkGetShaderModuleIdentifierEXT = (PFN_vkGetShaderModuleIdentifierEXT) load(userptr, "vkGetShaderModuleIdentifierEXT");
}
static void glad_vk_load_VK_EXT_shader_object( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_shader_object) return;
    glad_vkCmdBindShadersEXT = (PFN_vkCmdBindShadersEXT) load(userptr, "vkCmdBindShadersEXT");
    glad_vkCmdBindVertexBuffers2EXT = (PFN_vkCmdBindVertexBuffers2EXT) load(userptr, "vkCmdBindVertexBuffers2EXT");
    glad_vkCmdSetAlphaToCoverageEnableEXT = (PFN_vkCmdSetAlphaToCoverageEnableEXT) load(userptr, "vkCmdSetAlphaToCoverageEnableEXT");
    glad_vkCmdSetAlphaToOneEnableEXT = (PFN_vkCmdSetAlphaToOneEnableEXT) load(userptr, "vkCmdSetAlphaToOneEnableEXT");
    glad_vkCmdSetColorBlendAdvancedEXT = (PFN_vkCmdSetColorBlendAdvancedEXT) load(userptr, "vkCmdSetColorBlendAdvancedEXT");
    glad_vkCmdSetColorBlendEnableEXT = (PFN_vkCmdSetColorBlendEnableEXT) load(userptr, "vkCmdSetColorBlendEnableEXT");
    glad_vkCmdSetColorBlendEquationEXT = (PFN_vkCmdSetColorBlendEquationEXT) load(userptr, "vkCmdSetColorBlendEquationEXT");
    glad_vkCmdSetColorWriteMaskEXT = (PFN_vkCmdSetColorWriteMaskEXT) load(userptr, "vkCmdSetColorWriteMaskEXT");
    glad_vkCmdSetConservativeRasterizationModeEXT = (PFN_vkCmdSetConservativeRasterizationModeEXT) load(userptr, "vkCmdSetConservativeRasterizationModeEXT");
    glad_vkCmdSetCoverageModulationModeNV = (PFN_vkCmdSetCoverageModulationModeNV) load(userptr, "vkCmdSetCoverageModulationModeNV");
    glad_vkCmdSetCoverageModulationTableEnableNV = (PFN_vkCmdSetCoverageModulationTableEnableNV) load(userptr, "vkCmdSetCoverageModulationTableEnableNV");
    glad_vkCmdSetCoverageModulationTableNV = (PFN_vkCmdSetCoverageModulationTableNV) load(userptr, "vkCmdSetCoverageModulationTableNV");
    glad_vkCmdSetCoverageReductionModeNV = (PFN_vkCmdSetCoverageReductionModeNV) load(userptr, "vkCmdSetCoverageReductionModeNV");
    glad_vkCmdSetCoverageToColorEnableNV = (PFN_vkCmdSetCoverageToColorEnableNV) load(userptr, "vkCmdSetCoverageToColorEnableNV");
    glad_vkCmdSetCoverageToColorLocationNV = (PFN_vkCmdSetCoverageToColorLocationNV) load(userptr, "vkCmdSetCoverageToColorLocationNV");
    glad_vkCmdSetCullModeEXT = (PFN_vkCmdSetCullModeEXT) load(userptr, "vkCmdSetCullModeEXT");
    glad_vkCmdSetDepthBiasEnableEXT = (PFN_vkCmdSetDepthBiasEnableEXT) load(userptr, "vkCmdSetDepthBiasEnableEXT");
    glad_vkCmdSetDepthBoundsTestEnableEXT = (PFN_vkCmdSetDepthBoundsTestEnableEXT) load(userptr, "vkCmdSetDepthBoundsTestEnableEXT");
    glad_vkCmdSetDepthClampEnableEXT = (PFN_vkCmdSetDepthClampEnableEXT) load(userptr, "vkCmdSetDepthClampEnableEXT");
    glad_vkCmdSetDepthClampRangeEXT = (PFN_vkCmdSetDepthClampRangeEXT) load(userptr, "vkCmdSetDepthClampRangeEXT");
    glad_vkCmdSetDepthClipEnableEXT = (PFN_vkCmdSetDepthClipEnableEXT) load(userptr, "vkCmdSetDepthClipEnableEXT");
    glad_vkCmdSetDepthClipNegativeOneToOneEXT = (PFN_vkCmdSetDepthClipNegativeOneToOneEXT) load(userptr, "vkCmdSetDepthClipNegativeOneToOneEXT");
    glad_vkCmdSetDepthCompareOpEXT = (PFN_vkCmdSetDepthCompareOpEXT) load(userptr, "vkCmdSetDepthCompareOpEXT");
    glad_vkCmdSetDepthTestEnableEXT = (PFN_vkCmdSetDepthTestEnableEXT) load(userptr, "vkCmdSetDepthTestEnableEXT");
    glad_vkCmdSetDepthWriteEnableEXT = (PFN_vkCmdSetDepthWriteEnableEXT) load(userptr, "vkCmdSetDepthWriteEnableEXT");
    glad_vkCmdSetExtraPrimitiveOverestimationSizeEXT = (PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT) load(userptr, "vkCmdSetExtraPrimitiveOverestimationSizeEXT");
    glad_vkCmdSetFrontFaceEXT = (PFN_vkCmdSetFrontFaceEXT) load(userptr, "vkCmdSetFrontFaceEXT");
    glad_vkCmdSetLineRasterizationModeEXT = (PFN_vkCmdSetLineRasterizationModeEXT) load(userptr, "vkCmdSetLineRasterizationModeEXT");
    glad_vkCmdSetLineStippleEnableEXT = (PFN_vkCmdSetLineStippleEnableEXT) load(userptr, "vkCmdSetLineStippleEnableEXT");
    glad_vkCmdSetLogicOpEXT = (PFN_vkCmdSetLogicOpEXT) load(userptr, "vkCmdSetLogicOpEXT");
    glad_vkCmdSetLogicOpEnableEXT = (PFN_vkCmdSetLogicOpEnableEXT) load(userptr, "vkCmdSetLogicOpEnableEXT");
    glad_vkCmdSetPatchControlPointsEXT = (PFN_vkCmdSetPatchControlPointsEXT) load(userptr, "vkCmdSetPatchControlPointsEXT");
    glad_vkCmdSetPolygonModeEXT = (PFN_vkCmdSetPolygonModeEXT) load(userptr, "vkCmdSetPolygonModeEXT");
    glad_vkCmdSetPrimitiveRestartEnableEXT = (PFN_vkCmdSetPrimitiveRestartEnableEXT) load(userptr, "vkCmdSetPrimitiveRestartEnableEXT");
    glad_vkCmdSetPrimitiveTopologyEXT = (PFN_vkCmdSetPrimitiveTopologyEXT) load(userptr, "vkCmdSetPrimitiveTopologyEXT");
    glad_vkCmdSetProvokingVertexModeEXT = (PFN_vkCmdSetProvokingVertexModeEXT) load(userptr, "vkCmdSetProvokingVertexModeEXT");
    glad_vkCmdSetRasterizationSamplesEXT = (PFN_vkCmdSetRasterizationSamplesEXT) load(userptr, "vkCmdSetRasterizationSamplesEXT");
    glad_vkCmdSetRasterizationStreamEXT = (PFN_vkCmdSetRasterizationStreamEXT) load(userptr, "vkCmdSetRasterizationStreamEXT");
    glad_vkCmdSetRasterizerDiscardEnableEXT = (PFN_vkCmdSetRasterizerDiscardEnableEXT) load(userptr, "vkCmdSetRasterizerDiscardEnableEXT");
    glad_vkCmdSetRepresentativeFragmentTestEnableNV = (PFN_vkCmdSetRepresentativeFragmentTestEnableNV) load(userptr, "vkCmdSetRepresentativeFragmentTestEnableNV");
    glad_vkCmdSetSampleLocationsEnableEXT = (PFN_vkCmdSetSampleLocationsEnableEXT) load(userptr, "vkCmdSetSampleLocationsEnableEXT");
    glad_vkCmdSetSampleMaskEXT = (PFN_vkCmdSetSampleMaskEXT) load(userptr, "vkCmdSetSampleMaskEXT");
    glad_vkCmdSetScissorWithCountEXT = (PFN_vkCmdSetScissorWithCountEXT) load(userptr, "vkCmdSetScissorWithCountEXT");
    glad_vkCmdSetShadingRateImageEnableNV = (PFN_vkCmdSetShadingRateImageEnableNV) load(userptr, "vkCmdSetShadingRateImageEnableNV");
    glad_vkCmdSetStencilOpEXT = (PFN_vkCmdSetStencilOpEXT) load(userptr, "vkCmdSetStencilOpEXT");
    glad_vkCmdSetStencilTestEnableEXT = (PFN_vkCmdSetStencilTestEnableEXT) load(userptr, "vkCmdSetStencilTestEnableEXT");
    glad_vkCmdSetTessellationDomainOriginEXT = (PFN_vkCmdSetTessellationDomainOriginEXT) load(userptr, "vkCmdSetTessellationDomainOriginEXT");
    glad_vkCmdSetVertexInputEXT = (PFN_vkCmdSetVertexInputEXT) load(userptr, "vkCmdSetVertexInputEXT");
    glad_vkCmdSetViewportSwizzleNV = (PFN_vkCmdSetViewportSwizzleNV) load(userptr, "vkCmdSetViewportSwizzleNV");
    glad_vkCmdSetViewportWScalingEnableNV = (PFN_vkCmdSetViewportWScalingEnableNV) load(userptr, "vkCmdSetViewportWScalingEnableNV");
    glad_vkCmdSetViewportWithCountEXT = (PFN_vkCmdSetViewportWithCountEXT) load(userptr, "vkCmdSetViewportWithCountEXT");
    glad_vkCreateShadersEXT = (PFN_vkCreateShadersEXT) load(userptr, "vkCreateShadersEXT");
    glad_vkDestroyShaderEXT = (PFN_vkDestroyShaderEXT) load(userptr, "vkDestroyShaderEXT");
    glad_vkGetShaderBinaryDataEXT = (PFN_vkGetShaderBinaryDataEXT) load(userptr, "vkGetShaderBinaryDataEXT");
}
static void glad_vk_load_VK_EXT_swapchain_maintenance1( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_swapchain_maintenance1) return;
    glad_vkReleaseSwapchainImagesEXT = (PFN_vkReleaseSwapchainImagesEXT) load(userptr, "vkReleaseSwapchainImagesEXT");
}
static void glad_vk_load_VK_EXT_tooling_info( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_tooling_info) return;
    glad_vkGetPhysicalDeviceToolPropertiesEXT = (PFN_vkGetPhysicalDeviceToolPropertiesEXT) load(userptr, "vkGetPhysicalDeviceToolPropertiesEXT");
}
static void glad_vk_load_VK_EXT_transform_feedback( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_transform_feedback) return;
    glad_vkCmdBeginQueryIndexedEXT = (PFN_vkCmdBeginQueryIndexedEXT) load(userptr, "vkCmdBeginQueryIndexedEXT");
    glad_vkCmdBeginTransformFeedbackEXT = (PFN_vkCmdBeginTransformFeedbackEXT) load(userptr, "vkCmdBeginTransformFeedbackEXT");
    glad_vkCmdBindTransformFeedbackBuffersEXT = (PFN_vkCmdBindTransformFeedbackBuffersEXT) load(userptr, "vkCmdBindTransformFeedbackBuffersEXT");
    glad_vkCmdDrawIndirectByteCountEXT = (PFN_vkCmdDrawIndirectByteCountEXT) load(userptr, "vkCmdDrawIndirectByteCountEXT");
    glad_vkCmdEndQueryIndexedEXT = (PFN_vkCmdEndQueryIndexedEXT) load(userptr, "vkCmdEndQueryIndexedEXT");
    glad_vkCmdEndTransformFeedbackEXT = (PFN_vkCmdEndTransformFeedbackEXT) load(userptr, "vkCmdEndTransformFeedbackEXT");
}
static void glad_vk_load_VK_EXT_validation_cache( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_validation_cache) return;
    glad_vkCreateValidationCacheEXT = (PFN_vkCreateValidationCacheEXT) load(userptr, "vkCreateValidationCacheEXT");
    glad_vkDestroyValidationCacheEXT = (PFN_vkDestroyValidationCacheEXT) load(userptr, "vkDestroyValidationCacheEXT");
    glad_vkGetValidationCacheDataEXT = (PFN_vkGetValidationCacheDataEXT) load(userptr, "vkGetValidationCacheDataEXT");
    glad_vkMergeValidationCachesEXT = (PFN_vkMergeValidationCachesEXT) load(userptr, "vkMergeValidationCachesEXT");
}
static void glad_vk_load_VK_EXT_vertex_input_dynamic_state( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_EXT_vertex_input_dynamic_state) return;
    glad_vkCmdSetVertexInputEXT = (PFN_vkCmdSetVertexInputEXT) load(userptr, "vkCmdSetVertexInputEXT");
}
#if defined(VK_USE_PLATFORM_FUCHSIA)
static void glad_vk_load_VK_FUCHSIA_buffer_collection( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_FUCHSIA_buffer_collection) return;
    glad_vkCreateBufferCollectionFUCHSIA = (PFN_vkCreateBufferCollectionFUCHSIA) load(userptr, "vkCreateBufferCollectionFUCHSIA");
    glad_vkDestroyBufferCollectionFUCHSIA = (PFN_vkDestroyBufferCollectionFUCHSIA) load(userptr, "vkDestroyBufferCollectionFUCHSIA");
    glad_vkGetBufferCollectionPropertiesFUCHSIA = (PFN_vkGetBufferCollectionPropertiesFUCHSIA) load(userptr, "vkGetBufferCollectionPropertiesFUCHSIA");
    glad_vkSetBufferCollectionBufferConstraintsFUCHSIA = (PFN_vkSetBufferCollectionBufferConstraintsFUCHSIA) load(userptr, "vkSetBufferCollectionBufferConstraintsFUCHSIA");
    glad_vkSetBufferCollectionImageConstraintsFUCHSIA = (PFN_vkSetBufferCollectionImageConstraintsFUCHSIA) load(userptr, "vkSetBufferCollectionImageConstraintsFUCHSIA");
}

#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
static void glad_vk_load_VK_FUCHSIA_external_memory( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_FUCHSIA_external_memory) return;
    glad_vkGetMemoryZirconHandleFUCHSIA = (PFN_vkGetMemoryZirconHandleFUCHSIA) load(userptr, "vkGetMemoryZirconHandleFUCHSIA");
    glad_vkGetMemoryZirconHandlePropertiesFUCHSIA = (PFN_vkGetMemoryZirconHandlePropertiesFUCHSIA) load(userptr, "vkGetMemoryZirconHandlePropertiesFUCHSIA");
}

#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
static void glad_vk_load_VK_FUCHSIA_external_semaphore( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_FUCHSIA_external_semaphore) return;
    glad_vkGetSemaphoreZirconHandleFUCHSIA = (PFN_vkGetSemaphoreZirconHandleFUCHSIA) load(userptr, "vkGetSemaphoreZirconHandleFUCHSIA");
    glad_vkImportSemaphoreZirconHandleFUCHSIA = (PFN_vkImportSemaphoreZirconHandleFUCHSIA) load(userptr, "vkImportSemaphoreZirconHandleFUCHSIA");
}

#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
static void glad_vk_load_VK_FUCHSIA_imagepipe_surface( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_FUCHSIA_imagepipe_surface) return;
    glad_vkCreateImagePipeSurfaceFUCHSIA = (PFN_vkCreateImagePipeSurfaceFUCHSIA) load(userptr, "vkCreateImagePipeSurfaceFUCHSIA");
}

#endif
#if defined(VK_USE_PLATFORM_GGP)
static void glad_vk_load_VK_GGP_stream_descriptor_surface( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_GGP_stream_descriptor_surface) return;
    glad_vkCreateStreamDescriptorSurfaceGGP = (PFN_vkCreateStreamDescriptorSurfaceGGP) load(userptr, "vkCreateStreamDescriptorSurfaceGGP");
}

#endif
static void glad_vk_load_VK_GOOGLE_display_timing( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_GOOGLE_display_timing) return;
    glad_vkGetPastPresentationTimingGOOGLE = (PFN_vkGetPastPresentationTimingGOOGLE) load(userptr, "vkGetPastPresentationTimingGOOGLE");
    glad_vkGetRefreshCycleDurationGOOGLE = (PFN_vkGetRefreshCycleDurationGOOGLE) load(userptr, "vkGetRefreshCycleDurationGOOGLE");
}
static void glad_vk_load_VK_HUAWEI_cluster_culling_shader( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_HUAWEI_cluster_culling_shader) return;
    glad_vkCmdDrawClusterHUAWEI = (PFN_vkCmdDrawClusterHUAWEI) load(userptr, "vkCmdDrawClusterHUAWEI");
    glad_vkCmdDrawClusterIndirectHUAWEI = (PFN_vkCmdDrawClusterIndirectHUAWEI) load(userptr, "vkCmdDrawClusterIndirectHUAWEI");
}
static void glad_vk_load_VK_HUAWEI_invocation_mask( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_HUAWEI_invocation_mask) return;
    glad_vkCmdBindInvocationMaskHUAWEI = (PFN_vkCmdBindInvocationMaskHUAWEI) load(userptr, "vkCmdBindInvocationMaskHUAWEI");
}
static void glad_vk_load_VK_HUAWEI_subpass_shading( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_HUAWEI_subpass_shading) return;
    glad_vkCmdSubpassShadingHUAWEI = (PFN_vkCmdSubpassShadingHUAWEI) load(userptr, "vkCmdSubpassShadingHUAWEI");
    glad_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI = (PFN_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI) load(userptr, "vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI");
}
static void glad_vk_load_VK_INTEL_performance_query( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_INTEL_performance_query) return;
    glad_vkAcquirePerformanceConfigurationINTEL = (PFN_vkAcquirePerformanceConfigurationINTEL) load(userptr, "vkAcquirePerformanceConfigurationINTEL");
    glad_vkCmdSetPerformanceMarkerINTEL = (PFN_vkCmdSetPerformanceMarkerINTEL) load(userptr, "vkCmdSetPerformanceMarkerINTEL");
    glad_vkCmdSetPerformanceOverrideINTEL = (PFN_vkCmdSetPerformanceOverrideINTEL) load(userptr, "vkCmdSetPerformanceOverrideINTEL");
    glad_vkCmdSetPerformanceStreamMarkerINTEL = (PFN_vkCmdSetPerformanceStreamMarkerINTEL) load(userptr, "vkCmdSetPerformanceStreamMarkerINTEL");
    glad_vkGetPerformanceParameterINTEL = (PFN_vkGetPerformanceParameterINTEL) load(userptr, "vkGetPerformanceParameterINTEL");
    glad_vkInitializePerformanceApiINTEL = (PFN_vkInitializePerformanceApiINTEL) load(userptr, "vkInitializePerformanceApiINTEL");
    glad_vkQueueSetPerformanceConfigurationINTEL = (PFN_vkQueueSetPerformanceConfigurationINTEL) load(userptr, "vkQueueSetPerformanceConfigurationINTEL");
    glad_vkReleasePerformanceConfigurationINTEL = (PFN_vkReleasePerformanceConfigurationINTEL) load(userptr, "vkReleasePerformanceConfigurationINTEL");
    glad_vkUninitializePerformanceApiINTEL = (PFN_vkUninitializePerformanceApiINTEL) load(userptr, "vkUninitializePerformanceApiINTEL");
}
static void glad_vk_load_VK_KHR_acceleration_structure( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_acceleration_structure) return;
    glad_vkBuildAccelerationStructuresKHR = (PFN_vkBuildAccelerationStructuresKHR) load(userptr, "vkBuildAccelerationStructuresKHR");
    glad_vkCmdBuildAccelerationStructuresIndirectKHR = (PFN_vkCmdBuildAccelerationStructuresIndirectKHR) load(userptr, "vkCmdBuildAccelerationStructuresIndirectKHR");
    glad_vkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR) load(userptr, "vkCmdBuildAccelerationStructuresKHR");
    glad_vkCmdCopyAccelerationStructureKHR = (PFN_vkCmdCopyAccelerationStructureKHR) load(userptr, "vkCmdCopyAccelerationStructureKHR");
    glad_vkCmdCopyAccelerationStructureToMemoryKHR = (PFN_vkCmdCopyAccelerationStructureToMemoryKHR) load(userptr, "vkCmdCopyAccelerationStructureToMemoryKHR");
    glad_vkCmdCopyMemoryToAccelerationStructureKHR = (PFN_vkCmdCopyMemoryToAccelerationStructureKHR) load(userptr, "vkCmdCopyMemoryToAccelerationStructureKHR");
    glad_vkCmdWriteAccelerationStructuresPropertiesKHR = (PFN_vkCmdWriteAccelerationStructuresPropertiesKHR) load(userptr, "vkCmdWriteAccelerationStructuresPropertiesKHR");
    glad_vkCopyAccelerationStructureKHR = (PFN_vkCopyAccelerationStructureKHR) load(userptr, "vkCopyAccelerationStructureKHR");
    glad_vkCopyAccelerationStructureToMemoryKHR = (PFN_vkCopyAccelerationStructureToMemoryKHR) load(userptr, "vkCopyAccelerationStructureToMemoryKHR");
    glad_vkCopyMemoryToAccelerationStructureKHR = (PFN_vkCopyMemoryToAccelerationStructureKHR) load(userptr, "vkCopyMemoryToAccelerationStructureKHR");
    glad_vkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR) load(userptr, "vkCreateAccelerationStructureKHR");
    glad_vkDestroyAccelerationStructureKHR = (PFN_vkDestroyAccelerationStructureKHR) load(userptr, "vkDestroyAccelerationStructureKHR");
    glad_vkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR) load(userptr, "vkGetAccelerationStructureBuildSizesKHR");
    glad_vkGetAccelerationStructureDeviceAddressKHR = (PFN_vkGetAccelerationStructureDeviceAddressKHR) load(userptr, "vkGetAccelerationStructureDeviceAddressKHR");
    glad_vkGetDeviceAccelerationStructureCompatibilityKHR = (PFN_vkGetDeviceAccelerationStructureCompatibilityKHR) load(userptr, "vkGetDeviceAccelerationStructureCompatibilityKHR");
    glad_vkWriteAccelerationStructuresPropertiesKHR = (PFN_vkWriteAccelerationStructuresPropertiesKHR) load(userptr, "vkWriteAccelerationStructuresPropertiesKHR");
}
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
static void glad_vk_load_VK_KHR_android_surface( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_android_surface) return;
    glad_vkCreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR) load(userptr, "vkCreateAndroidSurfaceKHR");
}

#endif
static void glad_vk_load_VK_KHR_bind_memory2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_bind_memory2) return;
    glad_vkBindBufferMemory2KHR = (PFN_vkBindBufferMemory2KHR) load(userptr, "vkBindBufferMemory2KHR");
    glad_vkBindImageMemory2KHR = (PFN_vkBindImageMemory2KHR) load(userptr, "vkBindImageMemory2KHR");
}
static void glad_vk_load_VK_KHR_buffer_device_address( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_buffer_device_address) return;
    glad_vkGetBufferDeviceAddressKHR = (PFN_vkGetBufferDeviceAddressKHR) load(userptr, "vkGetBufferDeviceAddressKHR");
    glad_vkGetBufferOpaqueCaptureAddressKHR = (PFN_vkGetBufferOpaqueCaptureAddressKHR) load(userptr, "vkGetBufferOpaqueCaptureAddressKHR");
    glad_vkGetDeviceMemoryOpaqueCaptureAddressKHR = (PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR) load(userptr, "vkGetDeviceMemoryOpaqueCaptureAddressKHR");
}
static void glad_vk_load_VK_KHR_calibrated_timestamps( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_calibrated_timestamps) return;
    glad_vkGetCalibratedTimestampsKHR = (PFN_vkGetCalibratedTimestampsKHR) load(userptr, "vkGetCalibratedTimestampsKHR");
    glad_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR = (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsKHR) load(userptr, "vkGetPhysicalDeviceCalibrateableTimeDomainsKHR");
}
static void glad_vk_load_VK_KHR_cooperative_matrix( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_cooperative_matrix) return;
    glad_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR = (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR) load(userptr, "vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR");
}
static void glad_vk_load_VK_KHR_copy_commands2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_copy_commands2) return;
    glad_vkCmdBlitImage2KHR = (PFN_vkCmdBlitImage2KHR) load(userptr, "vkCmdBlitImage2KHR");
    glad_vkCmdCopyBuffer2KHR = (PFN_vkCmdCopyBuffer2KHR) load(userptr, "vkCmdCopyBuffer2KHR");
    glad_vkCmdCopyBufferToImage2KHR = (PFN_vkCmdCopyBufferToImage2KHR) load(userptr, "vkCmdCopyBufferToImage2KHR");
    glad_vkCmdCopyImage2KHR = (PFN_vkCmdCopyImage2KHR) load(userptr, "vkCmdCopyImage2KHR");
    glad_vkCmdCopyImageToBuffer2KHR = (PFN_vkCmdCopyImageToBuffer2KHR) load(userptr, "vkCmdCopyImageToBuffer2KHR");
    glad_vkCmdResolveImage2KHR = (PFN_vkCmdResolveImage2KHR) load(userptr, "vkCmdResolveImage2KHR");
}
static void glad_vk_load_VK_KHR_create_renderpass2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_create_renderpass2) return;
    glad_vkCmdBeginRenderPass2KHR = (PFN_vkCmdBeginRenderPass2KHR) load(userptr, "vkCmdBeginRenderPass2KHR");
    glad_vkCmdEndRenderPass2KHR = (PFN_vkCmdEndRenderPass2KHR) load(userptr, "vkCmdEndRenderPass2KHR");
    glad_vkCmdNextSubpass2KHR = (PFN_vkCmdNextSubpass2KHR) load(userptr, "vkCmdNextSubpass2KHR");
    glad_vkCreateRenderPass2KHR = (PFN_vkCreateRenderPass2KHR) load(userptr, "vkCreateRenderPass2KHR");
}
static void glad_vk_load_VK_KHR_deferred_host_operations( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_deferred_host_operations) return;
    glad_vkCreateDeferredOperationKHR = (PFN_vkCreateDeferredOperationKHR) load(userptr, "vkCreateDeferredOperationKHR");
    glad_vkDeferredOperationJoinKHR = (PFN_vkDeferredOperationJoinKHR) load(userptr, "vkDeferredOperationJoinKHR");
    glad_vkDestroyDeferredOperationKHR = (PFN_vkDestroyDeferredOperationKHR) load(userptr, "vkDestroyDeferredOperationKHR");
    glad_vkGetDeferredOperationMaxConcurrencyKHR = (PFN_vkGetDeferredOperationMaxConcurrencyKHR) load(userptr, "vkGetDeferredOperationMaxConcurrencyKHR");
    glad_vkGetDeferredOperationResultKHR = (PFN_vkGetDeferredOperationResultKHR) load(userptr, "vkGetDeferredOperationResultKHR");
}
static void glad_vk_load_VK_KHR_descriptor_update_template( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_descriptor_update_template) return;
    glad_vkCmdPushDescriptorSetWithTemplateKHR = (PFN_vkCmdPushDescriptorSetWithTemplateKHR) load(userptr, "vkCmdPushDescriptorSetWithTemplateKHR");
    glad_vkCreateDescriptorUpdateTemplateKHR = (PFN_vkCreateDescriptorUpdateTemplateKHR) load(userptr, "vkCreateDescriptorUpdateTemplateKHR");
    glad_vkDestroyDescriptorUpdateTemplateKHR = (PFN_vkDestroyDescriptorUpdateTemplateKHR) load(userptr, "vkDestroyDescriptorUpdateTemplateKHR");
    glad_vkUpdateDescriptorSetWithTemplateKHR = (PFN_vkUpdateDescriptorSetWithTemplateKHR) load(userptr, "vkUpdateDescriptorSetWithTemplateKHR");
}
static void glad_vk_load_VK_KHR_device_group( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_device_group) return;
    glad_vkAcquireNextImage2KHR = (PFN_vkAcquireNextImage2KHR) load(userptr, "vkAcquireNextImage2KHR");
    glad_vkCmdDispatchBaseKHR = (PFN_vkCmdDispatchBaseKHR) load(userptr, "vkCmdDispatchBaseKHR");
    glad_vkCmdSetDeviceMaskKHR = (PFN_vkCmdSetDeviceMaskKHR) load(userptr, "vkCmdSetDeviceMaskKHR");
    glad_vkGetDeviceGroupPeerMemoryFeaturesKHR = (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR) load(userptr, "vkGetDeviceGroupPeerMemoryFeaturesKHR");
    glad_vkGetDeviceGroupPresentCapabilitiesKHR = (PFN_vkGetDeviceGroupPresentCapabilitiesKHR) load(userptr, "vkGetDeviceGroupPresentCapabilitiesKHR");
    glad_vkGetDeviceGroupSurfacePresentModesKHR = (PFN_vkGetDeviceGroupSurfacePresentModesKHR) load(userptr, "vkGetDeviceGroupSurfacePresentModesKHR");
    glad_vkGetPhysicalDevicePresentRectanglesKHR = (PFN_vkGetPhysicalDevicePresentRectanglesKHR) load(userptr, "vkGetPhysicalDevicePresentRectanglesKHR");
}
static void glad_vk_load_VK_KHR_device_group_creation( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_device_group_creation) return;
    glad_vkEnumeratePhysicalDeviceGroupsKHR = (PFN_vkEnumeratePhysicalDeviceGroupsKHR) load(userptr, "vkEnumeratePhysicalDeviceGroupsKHR");
}
static void glad_vk_load_VK_KHR_display( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_display) return;
    glad_vkCreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR) load(userptr, "vkCreateDisplayModeKHR");
    glad_vkCreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR) load(userptr, "vkCreateDisplayPlaneSurfaceKHR");
    glad_vkGetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR) load(userptr, "vkGetDisplayModePropertiesKHR");
    glad_vkGetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR) load(userptr, "vkGetDisplayPlaneCapabilitiesKHR");
    glad_vkGetDisplayPlaneSupportedDisplaysKHR = (PFN_vkGetDisplayPlaneSupportedDisplaysKHR) load(userptr, "vkGetDisplayPlaneSupportedDisplaysKHR");
    glad_vkGetPhysicalDeviceDisplayPlanePropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR) load(userptr, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    glad_vkGetPhysicalDeviceDisplayPropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR) load(userptr, "vkGetPhysicalDeviceDisplayPropertiesKHR");
}
static void glad_vk_load_VK_KHR_display_swapchain( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_display_swapchain) return;
    glad_vkCreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR) load(userptr, "vkCreateSharedSwapchainsKHR");
}
static void glad_vk_load_VK_KHR_draw_indirect_count( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_draw_indirect_count) return;
    glad_vkCmdDrawIndexedIndirectCountKHR = (PFN_vkCmdDrawIndexedIndirectCountKHR) load(userptr, "vkCmdDrawIndexedIndirectCountKHR");
    glad_vkCmdDrawIndirectCountKHR = (PFN_vkCmdDrawIndirectCountKHR) load(userptr, "vkCmdDrawIndirectCountKHR");
}
static void glad_vk_load_VK_KHR_dynamic_rendering( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_dynamic_rendering) return;
    glad_vkCmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR) load(userptr, "vkCmdBeginRenderingKHR");
    glad_vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR) load(userptr, "vkCmdEndRenderingKHR");
}
static void glad_vk_load_VK_KHR_dynamic_rendering_local_read( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_dynamic_rendering_local_read) return;
    glad_vkCmdSetRenderingAttachmentLocationsKHR = (PFN_vkCmdSetRenderingAttachmentLocationsKHR) load(userptr, "vkCmdSetRenderingAttachmentLocationsKHR");
    glad_vkCmdSetRenderingInputAttachmentIndicesKHR = (PFN_vkCmdSetRenderingInputAttachmentIndicesKHR) load(userptr, "vkCmdSetRenderingInputAttachmentIndicesKHR");
}
static void glad_vk_load_VK_KHR_external_fence_capabilities( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_external_fence_capabilities) return;
    glad_vkGetPhysicalDeviceExternalFencePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR) load(userptr, "vkGetPhysicalDeviceExternalFencePropertiesKHR");
}
static void glad_vk_load_VK_KHR_external_fence_fd( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_external_fence_fd) return;
    glad_vkGetFenceFdKHR = (PFN_vkGetFenceFdKHR) load(userptr, "vkGetFenceFdKHR");
    glad_vkImportFenceFdKHR = (PFN_vkImportFenceFdKHR) load(userptr, "vkImportFenceFdKHR");
}
#if defined(VK_USE_PLATFORM_WIN32_KHR)
static void glad_vk_load_VK_KHR_external_fence_win32( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_external_fence_win32) return;
    glad_vkGetFenceWin32HandleKHR = (PFN_vkGetFenceWin32HandleKHR) load(userptr, "vkGetFenceWin32HandleKHR");
    glad_vkImportFenceWin32HandleKHR = (PFN_vkImportFenceWin32HandleKHR) load(userptr, "vkImportFenceWin32HandleKHR");
}

#endif
static void glad_vk_load_VK_KHR_external_memory_capabilities( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_external_memory_capabilities) return;
    glad_vkGetPhysicalDeviceExternalBufferPropertiesKHR = (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR) load(userptr, "vkGetPhysicalDeviceExternalBufferPropertiesKHR");
}
static void glad_vk_load_VK_KHR_external_memory_fd( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_external_memory_fd) return;
    glad_vkGetMemoryFdKHR = (PFN_vkGetMemoryFdKHR) load(userptr, "vkGetMemoryFdKHR");
    glad_vkGetMemoryFdPropertiesKHR = (PFN_vkGetMemoryFdPropertiesKHR) load(userptr, "vkGetMemoryFdPropertiesKHR");
}
#if defined(VK_USE_PLATFORM_WIN32_KHR)
static void glad_vk_load_VK_KHR_external_memory_win32( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_external_memory_win32) return;
    glad_vkGetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR) load(userptr, "vkGetMemoryWin32HandleKHR");
    glad_vkGetMemoryWin32HandlePropertiesKHR = (PFN_vkGetMemoryWin32HandlePropertiesKHR) load(userptr, "vkGetMemoryWin32HandlePropertiesKHR");
}

#endif
static void glad_vk_load_VK_KHR_external_semaphore_capabilities( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_external_semaphore_capabilities) return;
    glad_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR) load(userptr, "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
}
static void glad_vk_load_VK_KHR_external_semaphore_fd( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_external_semaphore_fd) return;
    glad_vkGetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR) load(userptr, "vkGetSemaphoreFdKHR");
    glad_vkImportSemaphoreFdKHR = (PFN_vkImportSemaphoreFdKHR) load(userptr, "vkImportSemaphoreFdKHR");
}
#if defined(VK_USE_PLATFORM_WIN32_KHR)
static void glad_vk_load_VK_KHR_external_semaphore_win32( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_external_semaphore_win32) return;
    glad_vkGetSemaphoreWin32HandleKHR = (PFN_vkGetSemaphoreWin32HandleKHR) load(userptr, "vkGetSemaphoreWin32HandleKHR");
    glad_vkImportSemaphoreWin32HandleKHR = (PFN_vkImportSemaphoreWin32HandleKHR) load(userptr, "vkImportSemaphoreWin32HandleKHR");
}

#endif
static void glad_vk_load_VK_KHR_fragment_shading_rate( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_fragment_shading_rate) return;
    glad_vkCmdSetFragmentShadingRateKHR = (PFN_vkCmdSetFragmentShadingRateKHR) load(userptr, "vkCmdSetFragmentShadingRateKHR");
    glad_vkGetPhysicalDeviceFragmentShadingRatesKHR = (PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR) load(userptr, "vkGetPhysicalDeviceFragmentShadingRatesKHR");
}
static void glad_vk_load_VK_KHR_get_display_properties2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_get_display_properties2) return;
    glad_vkGetDisplayModeProperties2KHR = (PFN_vkGetDisplayModeProperties2KHR) load(userptr, "vkGetDisplayModeProperties2KHR");
    glad_vkGetDisplayPlaneCapabilities2KHR = (PFN_vkGetDisplayPlaneCapabilities2KHR) load(userptr, "vkGetDisplayPlaneCapabilities2KHR");
    glad_vkGetPhysicalDeviceDisplayPlaneProperties2KHR = (PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR) load(userptr, "vkGetPhysicalDeviceDisplayPlaneProperties2KHR");
    glad_vkGetPhysicalDeviceDisplayProperties2KHR = (PFN_vkGetPhysicalDeviceDisplayProperties2KHR) load(userptr, "vkGetPhysicalDeviceDisplayProperties2KHR");
}
static void glad_vk_load_VK_KHR_get_memory_requirements2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_get_memory_requirements2) return;
    glad_vkGetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2KHR) load(userptr, "vkGetBufferMemoryRequirements2KHR");
    glad_vkGetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2KHR) load(userptr, "vkGetImageMemoryRequirements2KHR");
    glad_vkGetImageSparseMemoryRequirements2KHR = (PFN_vkGetImageSparseMemoryRequirements2KHR) load(userptr, "vkGetImageSparseMemoryRequirements2KHR");
}
static void glad_vk_load_VK_KHR_get_physical_device_properties2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_get_physical_device_properties2) return;
    glad_vkGetPhysicalDeviceFeatures2KHR = (PFN_vkGetPhysicalDeviceFeatures2KHR) load(userptr, "vkGetPhysicalDeviceFeatures2KHR");
    glad_vkGetPhysicalDeviceFormatProperties2KHR = (PFN_vkGetPhysicalDeviceFormatProperties2KHR) load(userptr, "vkGetPhysicalDeviceFormatProperties2KHR");
    glad_vkGetPhysicalDeviceImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR) load(userptr, "vkGetPhysicalDeviceImageFormatProperties2KHR");
    glad_vkGetPhysicalDeviceMemoryProperties2KHR = (PFN_vkGetPhysicalDeviceMemoryProperties2KHR) load(userptr, "vkGetPhysicalDeviceMemoryProperties2KHR");
    glad_vkGetPhysicalDeviceProperties2KHR = (PFN_vkGetPhysicalDeviceProperties2KHR) load(userptr, "vkGetPhysicalDeviceProperties2KHR");
    glad_vkGetPhysicalDeviceQueueFamilyProperties2KHR = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR) load(userptr, "vkGetPhysicalDeviceQueueFamilyProperties2KHR");
    glad_vkGetPhysicalDeviceSparseImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR) load(userptr, "vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
}
static void glad_vk_load_VK_KHR_get_surface_capabilities2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_get_surface_capabilities2) return;
    glad_vkGetPhysicalDeviceSurfaceCapabilities2KHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR) load(userptr, "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    glad_vkGetPhysicalDeviceSurfaceFormats2KHR = (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR) load(userptr, "vkGetPhysicalDeviceSurfaceFormats2KHR");
}
static void glad_vk_load_VK_KHR_line_rasterization( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_line_rasterization) return;
    glad_vkCmdSetLineStippleKHR = (PFN_vkCmdSetLineStippleKHR) load(userptr, "vkCmdSetLineStippleKHR");
}
static void glad_vk_load_VK_KHR_maintenance1( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_maintenance1) return;
    glad_vkTrimCommandPoolKHR = (PFN_vkTrimCommandPoolKHR) load(userptr, "vkTrimCommandPoolKHR");
}
static void glad_vk_load_VK_KHR_maintenance3( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_maintenance3) return;
    glad_vkGetDescriptorSetLayoutSupportKHR = (PFN_vkGetDescriptorSetLayoutSupportKHR) load(userptr, "vkGetDescriptorSetLayoutSupportKHR");
}
static void glad_vk_load_VK_KHR_maintenance4( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_maintenance4) return;
    glad_vkGetDeviceBufferMemoryRequirementsKHR = (PFN_vkGetDeviceBufferMemoryRequirementsKHR) load(userptr, "vkGetDeviceBufferMemoryRequirementsKHR");
    glad_vkGetDeviceImageMemoryRequirementsKHR = (PFN_vkGetDeviceImageMemoryRequirementsKHR) load(userptr, "vkGetDeviceImageMemoryRequirementsKHR");
    glad_vkGetDeviceImageSparseMemoryRequirementsKHR = (PFN_vkGetDeviceImageSparseMemoryRequirementsKHR) load(userptr, "vkGetDeviceImageSparseMemoryRequirementsKHR");
}
static void glad_vk_load_VK_KHR_maintenance5( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_maintenance5) return;
    glad_vkCmdBindIndexBuffer2KHR = (PFN_vkCmdBindIndexBuffer2KHR) load(userptr, "vkCmdBindIndexBuffer2KHR");
    glad_vkGetDeviceImageSubresourceLayoutKHR = (PFN_vkGetDeviceImageSubresourceLayoutKHR) load(userptr, "vkGetDeviceImageSubresourceLayoutKHR");
    glad_vkGetImageSubresourceLayout2KHR = (PFN_vkGetImageSubresourceLayout2KHR) load(userptr, "vkGetImageSubresourceLayout2KHR");
    glad_vkGetRenderingAreaGranularityKHR = (PFN_vkGetRenderingAreaGranularityKHR) load(userptr, "vkGetRenderingAreaGranularityKHR");
}
static void glad_vk_load_VK_KHR_maintenance6( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_maintenance6) return;
    glad_vkCmdBindDescriptorBufferEmbeddedSamplers2EXT = (PFN_vkCmdBindDescriptorBufferEmbeddedSamplers2EXT) load(userptr, "vkCmdBindDescriptorBufferEmbeddedSamplers2EXT");
    glad_vkCmdBindDescriptorSets2KHR = (PFN_vkCmdBindDescriptorSets2KHR) load(userptr, "vkCmdBindDescriptorSets2KHR");
    glad_vkCmdPushConstants2KHR = (PFN_vkCmdPushConstants2KHR) load(userptr, "vkCmdPushConstants2KHR");
    glad_vkCmdPushDescriptorSet2KHR = (PFN_vkCmdPushDescriptorSet2KHR) load(userptr, "vkCmdPushDescriptorSet2KHR");
    glad_vkCmdPushDescriptorSetWithTemplate2KHR = (PFN_vkCmdPushDescriptorSetWithTemplate2KHR) load(userptr, "vkCmdPushDescriptorSetWithTemplate2KHR");
    glad_vkCmdSetDescriptorBufferOffsets2EXT = (PFN_vkCmdSetDescriptorBufferOffsets2EXT) load(userptr, "vkCmdSetDescriptorBufferOffsets2EXT");
}
static void glad_vk_load_VK_KHR_map_memory2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_map_memory2) return;
    glad_vkMapMemory2KHR = (PFN_vkMapMemory2KHR) load(userptr, "vkMapMemory2KHR");
    glad_vkUnmapMemory2KHR = (PFN_vkUnmapMemory2KHR) load(userptr, "vkUnmapMemory2KHR");
}
static void glad_vk_load_VK_KHR_performance_query( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_performance_query) return;
    glad_vkAcquireProfilingLockKHR = (PFN_vkAcquireProfilingLockKHR) load(userptr, "vkAcquireProfilingLockKHR");
    glad_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR = (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR) load(userptr, "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    glad_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR = (PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR) load(userptr, "vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR");
    glad_vkReleaseProfilingLockKHR = (PFN_vkReleaseProfilingLockKHR) load(userptr, "vkReleaseProfilingLockKHR");
}
static void glad_vk_load_VK_KHR_pipeline_binary( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_pipeline_binary) return;
    glad_vkCreatePipelineBinariesKHR = (PFN_vkCreatePipelineBinariesKHR) load(userptr, "vkCreatePipelineBinariesKHR");
    glad_vkDestroyPipelineBinaryKHR = (PFN_vkDestroyPipelineBinaryKHR) load(userptr, "vkDestroyPipelineBinaryKHR");
    glad_vkGetPipelineBinaryDataKHR = (PFN_vkGetPipelineBinaryDataKHR) load(userptr, "vkGetPipelineBinaryDataKHR");
    glad_vkGetPipelineKeyKHR = (PFN_vkGetPipelineKeyKHR) load(userptr, "vkGetPipelineKeyKHR");
    glad_vkReleaseCapturedPipelineDataKHR = (PFN_vkReleaseCapturedPipelineDataKHR) load(userptr, "vkReleaseCapturedPipelineDataKHR");
}
static void glad_vk_load_VK_KHR_pipeline_executable_properties( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_pipeline_executable_properties) return;
    glad_vkGetPipelineExecutableInternalRepresentationsKHR = (PFN_vkGetPipelineExecutableInternalRepresentationsKHR) load(userptr, "vkGetPipelineExecutableInternalRepresentationsKHR");
    glad_vkGetPipelineExecutablePropertiesKHR = (PFN_vkGetPipelineExecutablePropertiesKHR) load(userptr, "vkGetPipelineExecutablePropertiesKHR");
    glad_vkGetPipelineExecutableStatisticsKHR = (PFN_vkGetPipelineExecutableStatisticsKHR) load(userptr, "vkGetPipelineExecutableStatisticsKHR");
}
static void glad_vk_load_VK_KHR_present_wait( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_present_wait) return;
    glad_vkWaitForPresentKHR = (PFN_vkWaitForPresentKHR) load(userptr, "vkWaitForPresentKHR");
}
static void glad_vk_load_VK_KHR_push_descriptor( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_push_descriptor) return;
    glad_vkCmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR) load(userptr, "vkCmdPushDescriptorSetKHR");
    glad_vkCmdPushDescriptorSetWithTemplateKHR = (PFN_vkCmdPushDescriptorSetWithTemplateKHR) load(userptr, "vkCmdPushDescriptorSetWithTemplateKHR");
}
static void glad_vk_load_VK_KHR_ray_tracing_maintenance1( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_ray_tracing_maintenance1) return;
    glad_vkCmdTraceRaysIndirect2KHR = (PFN_vkCmdTraceRaysIndirect2KHR) load(userptr, "vkCmdTraceRaysIndirect2KHR");
}
static void glad_vk_load_VK_KHR_ray_tracing_pipeline( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_ray_tracing_pipeline) return;
    glad_vkCmdSetRayTracingPipelineStackSizeKHR = (PFN_vkCmdSetRayTracingPipelineStackSizeKHR) load(userptr, "vkCmdSetRayTracingPipelineStackSizeKHR");
    glad_vkCmdTraceRaysIndirectKHR = (PFN_vkCmdTraceRaysIndirectKHR) load(userptr, "vkCmdTraceRaysIndirectKHR");
    glad_vkCmdTraceRaysKHR = (PFN_vkCmdTraceRaysKHR) load(userptr, "vkCmdTraceRaysKHR");
    glad_vkCreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR) load(userptr, "vkCreateRayTracingPipelinesKHR");
    glad_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR = (PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR) load(userptr, "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR");
    glad_vkGetRayTracingShaderGroupHandlesKHR = (PFN_vkGetRayTracingShaderGroupHandlesKHR) load(userptr, "vkGetRayTracingShaderGroupHandlesKHR");
    glad_vkGetRayTracingShaderGroupStackSizeKHR = (PFN_vkGetRayTracingShaderGroupStackSizeKHR) load(userptr, "vkGetRayTracingShaderGroupStackSizeKHR");
}
static void glad_vk_load_VK_KHR_sampler_ycbcr_conversion( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_sampler_ycbcr_conversion) return;
    glad_vkCreateSamplerYcbcrConversionKHR = (PFN_vkCreateSamplerYcbcrConversionKHR) load(userptr, "vkCreateSamplerYcbcrConversionKHR");
    glad_vkDestroySamplerYcbcrConversionKHR = (PFN_vkDestroySamplerYcbcrConversionKHR) load(userptr, "vkDestroySamplerYcbcrConversionKHR");
}
static void glad_vk_load_VK_KHR_shared_presentable_image( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_shared_presentable_image) return;
    glad_vkGetSwapchainStatusKHR = (PFN_vkGetSwapchainStatusKHR) load(userptr, "vkGetSwapchainStatusKHR");
}
static void glad_vk_load_VK_KHR_surface( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_surface) return;
    glad_vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR) load(userptr, "vkDestroySurfaceKHR");
    glad_vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR) load(userptr, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    glad_vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR) load(userptr, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    glad_vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR) load(userptr, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    glad_vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR) load(userptr, "vkGetPhysicalDeviceSurfaceSupportKHR");
}
static void glad_vk_load_VK_KHR_swapchain( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_swapchain) return;
    glad_vkAcquireNextImage2KHR = (PFN_vkAcquireNextImage2KHR) load(userptr, "vkAcquireNextImage2KHR");
    glad_vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR) load(userptr, "vkAcquireNextImageKHR");
    glad_vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR) load(userptr, "vkCreateSwapchainKHR");
    glad_vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR) load(userptr, "vkDestroySwapchainKHR");
    glad_vkGetDeviceGroupPresentCapabilitiesKHR = (PFN_vkGetDeviceGroupPresentCapabilitiesKHR) load(userptr, "vkGetDeviceGroupPresentCapabilitiesKHR");
    glad_vkGetDeviceGroupSurfacePresentModesKHR = (PFN_vkGetDeviceGroupSurfacePresentModesKHR) load(userptr, "vkGetDeviceGroupSurfacePresentModesKHR");
    glad_vkGetPhysicalDevicePresentRectanglesKHR = (PFN_vkGetPhysicalDevicePresentRectanglesKHR) load(userptr, "vkGetPhysicalDevicePresentRectanglesKHR");
    glad_vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR) load(userptr, "vkGetSwapchainImagesKHR");
    glad_vkQueuePresentKHR = (PFN_vkQueuePresentKHR) load(userptr, "vkQueuePresentKHR");
}
static void glad_vk_load_VK_KHR_synchronization2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_synchronization2) return;
    glad_vkCmdPipelineBarrier2KHR = (PFN_vkCmdPipelineBarrier2KHR) load(userptr, "vkCmdPipelineBarrier2KHR");
    glad_vkCmdResetEvent2KHR = (PFN_vkCmdResetEvent2KHR) load(userptr, "vkCmdResetEvent2KHR");
    glad_vkCmdSetEvent2KHR = (PFN_vkCmdSetEvent2KHR) load(userptr, "vkCmdSetEvent2KHR");
    glad_vkCmdWaitEvents2KHR = (PFN_vkCmdWaitEvents2KHR) load(userptr, "vkCmdWaitEvents2KHR");
    glad_vkCmdWriteTimestamp2KHR = (PFN_vkCmdWriteTimestamp2KHR) load(userptr, "vkCmdWriteTimestamp2KHR");
    glad_vkQueueSubmit2KHR = (PFN_vkQueueSubmit2KHR) load(userptr, "vkQueueSubmit2KHR");
}
static void glad_vk_load_VK_KHR_timeline_semaphore( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_timeline_semaphore) return;
    glad_vkGetSemaphoreCounterValueKHR = (PFN_vkGetSemaphoreCounterValueKHR) load(userptr, "vkGetSemaphoreCounterValueKHR");
    glad_vkSignalSemaphoreKHR = (PFN_vkSignalSemaphoreKHR) load(userptr, "vkSignalSemaphoreKHR");
    glad_vkWaitSemaphoresKHR = (PFN_vkWaitSemaphoresKHR) load(userptr, "vkWaitSemaphoresKHR");
}
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
static void glad_vk_load_VK_KHR_wayland_surface( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_wayland_surface) return;
    glad_vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR) load(userptr, "vkCreateWaylandSurfaceKHR");
    glad_vkGetPhysicalDeviceWaylandPresentationSupportKHR = (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR) load(userptr, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
}

#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
static void glad_vk_load_VK_KHR_win32_surface( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_win32_surface) return;
    glad_vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR) load(userptr, "vkCreateWin32SurfaceKHR");
    glad_vkGetPhysicalDeviceWin32PresentationSupportKHR = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR) load(userptr, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
}

#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
static void glad_vk_load_VK_KHR_xcb_surface( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_xcb_surface) return;
    glad_vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR) load(userptr, "vkCreateXcbSurfaceKHR");
    glad_vkGetPhysicalDeviceXcbPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR) load(userptr, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
}

#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
static void glad_vk_load_VK_KHR_xlib_surface( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_KHR_xlib_surface) return;
    glad_vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR) load(userptr, "vkCreateXlibSurfaceKHR");
    glad_vkGetPhysicalDeviceXlibPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR) load(userptr, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
}

#endif
#if defined(VK_USE_PLATFORM_IOS_MVK)
static void glad_vk_load_VK_MVK_ios_surface( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_MVK_ios_surface) return;
    glad_vkCreateIOSSurfaceMVK = (PFN_vkCreateIOSSurfaceMVK) load(userptr, "vkCreateIOSSurfaceMVK");
}

#endif
#if defined(VK_USE_PLATFORM_MACOS_MVK)
static void glad_vk_load_VK_MVK_macos_surface( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_MVK_macos_surface) return;
    glad_vkCreateMacOSSurfaceMVK = (PFN_vkCreateMacOSSurfaceMVK) load(userptr, "vkCreateMacOSSurfaceMVK");
}

#endif
#if defined(VK_USE_PLATFORM_VI_NN)
static void glad_vk_load_VK_NN_vi_surface( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NN_vi_surface) return;
    glad_vkCreateViSurfaceNN = (PFN_vkCreateViSurfaceNN) load(userptr, "vkCreateViSurfaceNN");
}

#endif
static void glad_vk_load_VK_NVX_binary_import( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NVX_binary_import) return;
    glad_vkCmdCuLaunchKernelNVX = (PFN_vkCmdCuLaunchKernelNVX) load(userptr, "vkCmdCuLaunchKernelNVX");
    glad_vkCreateCuFunctionNVX = (PFN_vkCreateCuFunctionNVX) load(userptr, "vkCreateCuFunctionNVX");
    glad_vkCreateCuModuleNVX = (PFN_vkCreateCuModuleNVX) load(userptr, "vkCreateCuModuleNVX");
    glad_vkDestroyCuFunctionNVX = (PFN_vkDestroyCuFunctionNVX) load(userptr, "vkDestroyCuFunctionNVX");
    glad_vkDestroyCuModuleNVX = (PFN_vkDestroyCuModuleNVX) load(userptr, "vkDestroyCuModuleNVX");
}
static void glad_vk_load_VK_NVX_image_view_handle( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NVX_image_view_handle) return;
    glad_vkGetImageViewAddressNVX = (PFN_vkGetImageViewAddressNVX) load(userptr, "vkGetImageViewAddressNVX");
    glad_vkGetImageViewHandle64NVX = (PFN_vkGetImageViewHandle64NVX) load(userptr, "vkGetImageViewHandle64NVX");
    glad_vkGetImageViewHandleNVX = (PFN_vkGetImageViewHandleNVX) load(userptr, "vkGetImageViewHandleNVX");
}
#if defined(VK_USE_PLATFORM_WIN32_KHR)
static void glad_vk_load_VK_NV_acquire_winrt_display( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_acquire_winrt_display) return;
    glad_vkAcquireWinrtDisplayNV = (PFN_vkAcquireWinrtDisplayNV) load(userptr, "vkAcquireWinrtDisplayNV");
    glad_vkGetWinrtDisplayNV = (PFN_vkGetWinrtDisplayNV) load(userptr, "vkGetWinrtDisplayNV");
}

#endif
static void glad_vk_load_VK_NV_clip_space_w_scaling( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_clip_space_w_scaling) return;
    glad_vkCmdSetViewportWScalingNV = (PFN_vkCmdSetViewportWScalingNV) load(userptr, "vkCmdSetViewportWScalingNV");
}
static void glad_vk_load_VK_NV_cluster_acceleration_structure( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_cluster_acceleration_structure) return;
    glad_vkCmdBuildClusterAccelerationStructureIndirectNV = (PFN_vkCmdBuildClusterAccelerationStructureIndirectNV) load(userptr, "vkCmdBuildClusterAccelerationStructureIndirectNV");
    glad_vkGetClusterAccelerationStructureBuildSizesNV = (PFN_vkGetClusterAccelerationStructureBuildSizesNV) load(userptr, "vkGetClusterAccelerationStructureBuildSizesNV");
}
static void glad_vk_load_VK_NV_cooperative_matrix( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_cooperative_matrix) return;
    glad_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV = (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV) load(userptr, "vkGetPhysicalDeviceCooperativeMatrixPropertiesNV");
}
static void glad_vk_load_VK_NV_cooperative_matrix2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_cooperative_matrix2) return;
    glad_vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV = (PFN_vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV) load(userptr, "vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV");
}
static void glad_vk_load_VK_NV_cooperative_vector( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_cooperative_vector) return;
    glad_vkCmdConvertCooperativeVectorMatrixNV = (PFN_vkCmdConvertCooperativeVectorMatrixNV) load(userptr, "vkCmdConvertCooperativeVectorMatrixNV");
    glad_vkConvertCooperativeVectorMatrixNV = (PFN_vkConvertCooperativeVectorMatrixNV) load(userptr, "vkConvertCooperativeVectorMatrixNV");
    glad_vkGetPhysicalDeviceCooperativeVectorPropertiesNV = (PFN_vkGetPhysicalDeviceCooperativeVectorPropertiesNV) load(userptr, "vkGetPhysicalDeviceCooperativeVectorPropertiesNV");
}
static void glad_vk_load_VK_NV_copy_memory_indirect( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_copy_memory_indirect) return;
    glad_vkCmdCopyMemoryIndirectNV = (PFN_vkCmdCopyMemoryIndirectNV) load(userptr, "vkCmdCopyMemoryIndirectNV");
    glad_vkCmdCopyMemoryToImageIndirectNV = (PFN_vkCmdCopyMemoryToImageIndirectNV) load(userptr, "vkCmdCopyMemoryToImageIndirectNV");
}
static void glad_vk_load_VK_NV_coverage_reduction_mode( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_coverage_reduction_mode) return;
    glad_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV = (PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV) load(userptr, "vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV");
}
#if defined(VK_ENABLE_BETA_EXTENSIONS)
static void glad_vk_load_VK_NV_cuda_kernel_launch( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_cuda_kernel_launch) return;
    glad_vkCmdCudaLaunchKernelNV = (PFN_vkCmdCudaLaunchKernelNV) load(userptr, "vkCmdCudaLaunchKernelNV");
    glad_vkCreateCudaFunctionNV = (PFN_vkCreateCudaFunctionNV) load(userptr, "vkCreateCudaFunctionNV");
    glad_vkCreateCudaModuleNV = (PFN_vkCreateCudaModuleNV) load(userptr, "vkCreateCudaModuleNV");
    glad_vkDestroyCudaFunctionNV = (PFN_vkDestroyCudaFunctionNV) load(userptr, "vkDestroyCudaFunctionNV");
    glad_vkDestroyCudaModuleNV = (PFN_vkDestroyCudaModuleNV) load(userptr, "vkDestroyCudaModuleNV");
    glad_vkGetCudaModuleCacheNV = (PFN_vkGetCudaModuleCacheNV) load(userptr, "vkGetCudaModuleCacheNV");
}

#endif
static void glad_vk_load_VK_NV_device_diagnostic_checkpoints( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_device_diagnostic_checkpoints) return;
    glad_vkCmdSetCheckpointNV = (PFN_vkCmdSetCheckpointNV) load(userptr, "vkCmdSetCheckpointNV");
    glad_vkGetQueueCheckpointData2NV = (PFN_vkGetQueueCheckpointData2NV) load(userptr, "vkGetQueueCheckpointData2NV");
    glad_vkGetQueueCheckpointDataNV = (PFN_vkGetQueueCheckpointDataNV) load(userptr, "vkGetQueueCheckpointDataNV");
}
static void glad_vk_load_VK_NV_device_generated_commands( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_device_generated_commands) return;
    glad_vkCmdBindPipelineShaderGroupNV = (PFN_vkCmdBindPipelineShaderGroupNV) load(userptr, "vkCmdBindPipelineShaderGroupNV");
    glad_vkCmdExecuteGeneratedCommandsNV = (PFN_vkCmdExecuteGeneratedCommandsNV) load(userptr, "vkCmdExecuteGeneratedCommandsNV");
    glad_vkCmdPreprocessGeneratedCommandsNV = (PFN_vkCmdPreprocessGeneratedCommandsNV) load(userptr, "vkCmdPreprocessGeneratedCommandsNV");
    glad_vkCreateIndirectCommandsLayoutNV = (PFN_vkCreateIndirectCommandsLayoutNV) load(userptr, "vkCreateIndirectCommandsLayoutNV");
    glad_vkDestroyIndirectCommandsLayoutNV = (PFN_vkDestroyIndirectCommandsLayoutNV) load(userptr, "vkDestroyIndirectCommandsLayoutNV");
    glad_vkGetGeneratedCommandsMemoryRequirementsNV = (PFN_vkGetGeneratedCommandsMemoryRequirementsNV) load(userptr, "vkGetGeneratedCommandsMemoryRequirementsNV");
}
static void glad_vk_load_VK_NV_device_generated_commands_compute( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_device_generated_commands_compute) return;
    glad_vkCmdUpdatePipelineIndirectBufferNV = (PFN_vkCmdUpdatePipelineIndirectBufferNV) load(userptr, "vkCmdUpdatePipelineIndirectBufferNV");
    glad_vkGetPipelineIndirectDeviceAddressNV = (PFN_vkGetPipelineIndirectDeviceAddressNV) load(userptr, "vkGetPipelineIndirectDeviceAddressNV");
    glad_vkGetPipelineIndirectMemoryRequirementsNV = (PFN_vkGetPipelineIndirectMemoryRequirementsNV) load(userptr, "vkGetPipelineIndirectMemoryRequirementsNV");
}
static void glad_vk_load_VK_NV_external_compute_queue( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_external_compute_queue) return;
    glad_vkCreateExternalComputeQueueNV = (PFN_vkCreateExternalComputeQueueNV) load(userptr, "vkCreateExternalComputeQueueNV");
    glad_vkDestroyExternalComputeQueueNV = (PFN_vkDestroyExternalComputeQueueNV) load(userptr, "vkDestroyExternalComputeQueueNV");
    glad_vkGetExternalComputeQueueDataNV = (PFN_vkGetExternalComputeQueueDataNV) load(userptr, "vkGetExternalComputeQueueDataNV");
}
static void glad_vk_load_VK_NV_external_memory_capabilities( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_external_memory_capabilities) return;
    glad_vkGetPhysicalDeviceExternalImageFormatPropertiesNV = (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV) load(userptr, "vkGetPhysicalDeviceExternalImageFormatPropertiesNV");
}
static void glad_vk_load_VK_NV_external_memory_rdma( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_external_memory_rdma) return;
    glad_vkGetMemoryRemoteAddressNV = (PFN_vkGetMemoryRemoteAddressNV) load(userptr, "vkGetMemoryRemoteAddressNV");
}
#if defined(VK_USE_PLATFORM_WIN32_KHR)
static void glad_vk_load_VK_NV_external_memory_win32( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_external_memory_win32) return;
    glad_vkGetMemoryWin32HandleNV = (PFN_vkGetMemoryWin32HandleNV) load(userptr, "vkGetMemoryWin32HandleNV");
}

#endif
static void glad_vk_load_VK_NV_fragment_shading_rate_enums( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_fragment_shading_rate_enums) return;
    glad_vkCmdSetFragmentShadingRateEnumNV = (PFN_vkCmdSetFragmentShadingRateEnumNV) load(userptr, "vkCmdSetFragmentShadingRateEnumNV");
}
static void glad_vk_load_VK_NV_low_latency2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_low_latency2) return;
    glad_vkGetLatencyTimingsNV = (PFN_vkGetLatencyTimingsNV) load(userptr, "vkGetLatencyTimingsNV");
    glad_vkLatencySleepNV = (PFN_vkLatencySleepNV) load(userptr, "vkLatencySleepNV");
    glad_vkQueueNotifyOutOfBandNV = (PFN_vkQueueNotifyOutOfBandNV) load(userptr, "vkQueueNotifyOutOfBandNV");
    glad_vkSetLatencyMarkerNV = (PFN_vkSetLatencyMarkerNV) load(userptr, "vkSetLatencyMarkerNV");
    glad_vkSetLatencySleepModeNV = (PFN_vkSetLatencySleepModeNV) load(userptr, "vkSetLatencySleepModeNV");
}
static void glad_vk_load_VK_NV_memory_decompression( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_memory_decompression) return;
    glad_vkCmdDecompressMemoryIndirectCountNV = (PFN_vkCmdDecompressMemoryIndirectCountNV) load(userptr, "vkCmdDecompressMemoryIndirectCountNV");
    glad_vkCmdDecompressMemoryNV = (PFN_vkCmdDecompressMemoryNV) load(userptr, "vkCmdDecompressMemoryNV");
}
static void glad_vk_load_VK_NV_mesh_shader( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_mesh_shader) return;
    glad_vkCmdDrawMeshTasksIndirectCountNV = (PFN_vkCmdDrawMeshTasksIndirectCountNV) load(userptr, "vkCmdDrawMeshTasksIndirectCountNV");
    glad_vkCmdDrawMeshTasksIndirectNV = (PFN_vkCmdDrawMeshTasksIndirectNV) load(userptr, "vkCmdDrawMeshTasksIndirectNV");
    glad_vkCmdDrawMeshTasksNV = (PFN_vkCmdDrawMeshTasksNV) load(userptr, "vkCmdDrawMeshTasksNV");
}
static void glad_vk_load_VK_NV_optical_flow( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_optical_flow) return;
    glad_vkBindOpticalFlowSessionImageNV = (PFN_vkBindOpticalFlowSessionImageNV) load(userptr, "vkBindOpticalFlowSessionImageNV");
    glad_vkCmdOpticalFlowExecuteNV = (PFN_vkCmdOpticalFlowExecuteNV) load(userptr, "vkCmdOpticalFlowExecuteNV");
    glad_vkCreateOpticalFlowSessionNV = (PFN_vkCreateOpticalFlowSessionNV) load(userptr, "vkCreateOpticalFlowSessionNV");
    glad_vkDestroyOpticalFlowSessionNV = (PFN_vkDestroyOpticalFlowSessionNV) load(userptr, "vkDestroyOpticalFlowSessionNV");
    glad_vkGetPhysicalDeviceOpticalFlowImageFormatsNV = (PFN_vkGetPhysicalDeviceOpticalFlowImageFormatsNV) load(userptr, "vkGetPhysicalDeviceOpticalFlowImageFormatsNV");
}
static void glad_vk_load_VK_NV_partitioned_acceleration_structure( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_partitioned_acceleration_structure) return;
    glad_vkCmdBuildPartitionedAccelerationStructuresNV = (PFN_vkCmdBuildPartitionedAccelerationStructuresNV) load(userptr, "vkCmdBuildPartitionedAccelerationStructuresNV");
    glad_vkGetPartitionedAccelerationStructuresBuildSizesNV = (PFN_vkGetPartitionedAccelerationStructuresBuildSizesNV) load(userptr, "vkGetPartitionedAccelerationStructuresBuildSizesNV");
}
static void glad_vk_load_VK_NV_ray_tracing( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_ray_tracing) return;
    glad_vkBindAccelerationStructureMemoryNV = (PFN_vkBindAccelerationStructureMemoryNV) load(userptr, "vkBindAccelerationStructureMemoryNV");
    glad_vkCmdBuildAccelerationStructureNV = (PFN_vkCmdBuildAccelerationStructureNV) load(userptr, "vkCmdBuildAccelerationStructureNV");
    glad_vkCmdCopyAccelerationStructureNV = (PFN_vkCmdCopyAccelerationStructureNV) load(userptr, "vkCmdCopyAccelerationStructureNV");
    glad_vkCmdTraceRaysNV = (PFN_vkCmdTraceRaysNV) load(userptr, "vkCmdTraceRaysNV");
    glad_vkCmdWriteAccelerationStructuresPropertiesNV = (PFN_vkCmdWriteAccelerationStructuresPropertiesNV) load(userptr, "vkCmdWriteAccelerationStructuresPropertiesNV");
    glad_vkCompileDeferredNV = (PFN_vkCompileDeferredNV) load(userptr, "vkCompileDeferredNV");
    glad_vkCreateAccelerationStructureNV = (PFN_vkCreateAccelerationStructureNV) load(userptr, "vkCreateAccelerationStructureNV");
    glad_vkCreateRayTracingPipelinesNV = (PFN_vkCreateRayTracingPipelinesNV) load(userptr, "vkCreateRayTracingPipelinesNV");
    glad_vkDestroyAccelerationStructureNV = (PFN_vkDestroyAccelerationStructureNV) load(userptr, "vkDestroyAccelerationStructureNV");
    glad_vkGetAccelerationStructureHandleNV = (PFN_vkGetAccelerationStructureHandleNV) load(userptr, "vkGetAccelerationStructureHandleNV");
    glad_vkGetAccelerationStructureMemoryRequirementsNV = (PFN_vkGetAccelerationStructureMemoryRequirementsNV) load(userptr, "vkGetAccelerationStructureMemoryRequirementsNV");
    glad_vkGetRayTracingShaderGroupHandlesNV = (PFN_vkGetRayTracingShaderGroupHandlesNV) load(userptr, "vkGetRayTracingShaderGroupHandlesNV");
}
static void glad_vk_load_VK_NV_scissor_exclusive( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_scissor_exclusive) return;
    glad_vkCmdSetExclusiveScissorEnableNV = (PFN_vkCmdSetExclusiveScissorEnableNV) load(userptr, "vkCmdSetExclusiveScissorEnableNV");
    glad_vkCmdSetExclusiveScissorNV = (PFN_vkCmdSetExclusiveScissorNV) load(userptr, "vkCmdSetExclusiveScissorNV");
}
static void glad_vk_load_VK_NV_shading_rate_image( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_NV_shading_rate_image) return;
    glad_vkCmdBindShadingRateImageNV = (PFN_vkCmdBindShadingRateImageNV) load(userptr, "vkCmdBindShadingRateImageNV");
    glad_vkCmdSetCoarseSampleOrderNV = (PFN_vkCmdSetCoarseSampleOrderNV) load(userptr, "vkCmdSetCoarseSampleOrderNV");
    glad_vkCmdSetViewportShadingRatePaletteNV = (PFN_vkCmdSetViewportShadingRatePaletteNV) load(userptr, "vkCmdSetViewportShadingRatePaletteNV");
}
static void glad_vk_load_VK_QCOM_tile_memory_heap( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_QCOM_tile_memory_heap) return;
    glad_vkCmdBindTileMemoryQCOM = (PFN_vkCmdBindTileMemoryQCOM) load(userptr, "vkCmdBindTileMemoryQCOM");
}
static void glad_vk_load_VK_QCOM_tile_properties( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_QCOM_tile_properties) return;
    glad_vkGetDynamicRenderingTilePropertiesQCOM = (PFN_vkGetDynamicRenderingTilePropertiesQCOM) load(userptr, "vkGetDynamicRenderingTilePropertiesQCOM");
    glad_vkGetFramebufferTilePropertiesQCOM = (PFN_vkGetFramebufferTilePropertiesQCOM) load(userptr, "vkGetFramebufferTilePropertiesQCOM");
}
static void glad_vk_load_VK_QCOM_tile_shading( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_QCOM_tile_shading) return;
    glad_vkCmdBeginPerTileExecutionQCOM = (PFN_vkCmdBeginPerTileExecutionQCOM) load(userptr, "vkCmdBeginPerTileExecutionQCOM");
    glad_vkCmdDispatchTileQCOM = (PFN_vkCmdDispatchTileQCOM) load(userptr, "vkCmdDispatchTileQCOM");
    glad_vkCmdEndPerTileExecutionQCOM = (PFN_vkCmdEndPerTileExecutionQCOM) load(userptr, "vkCmdEndPerTileExecutionQCOM");
}
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
static void glad_vk_load_VK_QNX_external_memory_screen_buffer( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_QNX_external_memory_screen_buffer) return;
    glad_vkGetScreenBufferPropertiesQNX = (PFN_vkGetScreenBufferPropertiesQNX) load(userptr, "vkGetScreenBufferPropertiesQNX");
}

#endif
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
static void glad_vk_load_VK_QNX_screen_surface( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_QNX_screen_surface) return;
    glad_vkCreateScreenSurfaceQNX = (PFN_vkCreateScreenSurfaceQNX) load(userptr, "vkCreateScreenSurfaceQNX");
    glad_vkGetPhysicalDeviceScreenPresentationSupportQNX = (PFN_vkGetPhysicalDeviceScreenPresentationSupportQNX) load(userptr, "vkGetPhysicalDeviceScreenPresentationSupportQNX");
}

#endif
static void glad_vk_load_VK_VALVE_descriptor_set_host_mapping( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_VK_VALVE_descriptor_set_host_mapping) return;
    glad_vkGetDescriptorSetHostMappingVALVE = (PFN_vkGetDescriptorSetHostMappingVALVE) load(userptr, "vkGetDescriptorSetHostMappingVALVE");
    glad_vkGetDescriptorSetLayoutHostMappingInfoVALVE = (PFN_vkGetDescriptorSetLayoutHostMappingInfoVALVE) load(userptr, "vkGetDescriptorSetLayoutHostMappingInfoVALVE");
}



static int glad_vk_get_extensions( VkPhysicalDevice physical_device, uint32_t *out_extension_count, char ***out_extensions) {
    uint32_t i;
    uint32_t instance_extension_count = 0;
    uint32_t device_extension_count = 0;
    uint32_t max_extension_count = 0;
    uint32_t total_extension_count = 0;
    char **extensions = NULL;
    VkExtensionProperties *ext_properties = NULL;
    VkResult result;

    if (glad_vkEnumerateInstanceExtensionProperties == NULL || (physical_device != NULL && glad_vkEnumerateDeviceExtensionProperties == NULL)) {
        return 0;
    }

    result = glad_vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
    if (result != VK_SUCCESS) {
        return 0;
    }

    if (physical_device != NULL) {
        result = glad_vkEnumerateDeviceExtensionProperties(physical_device, NULL, &device_extension_count, NULL);
        if (result != VK_SUCCESS) {
            return 0;
        }
    }

    total_extension_count = instance_extension_count + device_extension_count;
    if (total_extension_count <= 0) {
        return 0;
    }

    max_extension_count = instance_extension_count > device_extension_count
        ? instance_extension_count : device_extension_count;

    ext_properties = (VkExtensionProperties*) malloc(max_extension_count * sizeof(VkExtensionProperties));
    if (ext_properties == NULL) {
        goto glad_vk_get_extensions_error;
    }

    result = glad_vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, ext_properties);
    if (result != VK_SUCCESS) {
        goto glad_vk_get_extensions_error;
    }

    extensions = (char**) calloc(total_extension_count, sizeof(char*));
    if (extensions == NULL) {
        goto glad_vk_get_extensions_error;
    }

    for (i = 0; i < instance_extension_count; ++i) {
        VkExtensionProperties ext = ext_properties[i];

        size_t extension_name_length = strlen(ext.extensionName) + 1;
        extensions[i] = (char*) malloc(extension_name_length * sizeof(char));
        if (extensions[i] == NULL) {
            goto glad_vk_get_extensions_error;
        }
        memcpy(extensions[i], ext.extensionName, extension_name_length * sizeof(char));
    }

    if (physical_device != NULL) {
        result = glad_vkEnumerateDeviceExtensionProperties(physical_device, NULL, &device_extension_count, ext_properties);
        if (result != VK_SUCCESS) {
            goto glad_vk_get_extensions_error;
        }

        for (i = 0; i < device_extension_count; ++i) {
            VkExtensionProperties ext = ext_properties[i];

            size_t extension_name_length = strlen(ext.extensionName) + 1;
            extensions[instance_extension_count + i] = (char*) malloc(extension_name_length * sizeof(char));
            if (extensions[instance_extension_count + i] == NULL) {
                goto glad_vk_get_extensions_error;
            }
            memcpy(extensions[instance_extension_count + i], ext.extensionName, extension_name_length * sizeof(char));
        }
    }

    free((void*) ext_properties);

    *out_extension_count = total_extension_count;
    *out_extensions = extensions;

    return 1;

glad_vk_get_extensions_error:
    free((void*) ext_properties);
    if (extensions != NULL) {
        for (i = 0; i < total_extension_count; ++i) {
            free((void*) extensions[i]);
        }
        free(extensions);
    }
    return 0;
}

static void glad_vk_free_extensions(uint32_t extension_count, char **extensions) {
    uint32_t i;

    for(i = 0; i < extension_count ; ++i) {
        free((void*) (extensions[i]));
    }

    free((void*) extensions);
}

static int glad_vk_has_extension(const char *name, uint32_t extension_count, char **extensions) {
    uint32_t i;

    for (i = 0; i < extension_count; ++i) {
        if(extensions[i] != NULL && strcmp(name, extensions[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

static GLADapiproc glad_vk_get_proc_from_userptr(void *userptr, const char* name) {
    return (GLAD_GNUC_EXTENSION (GLADapiproc (*)(const char *name)) userptr)(name);
}

static int glad_vk_find_extensions_vulkan( VkPhysicalDevice physical_device) {
    uint32_t extension_count = 0;
    char **extensions = NULL;
    if (!glad_vk_get_extensions(physical_device, &extension_count, &extensions)) return 0;

#if defined(VK_ENABLE_BETA_EXTENSIONS)
    GLAD_VK_AMDX_shader_enqueue = glad_vk_has_extension("VK_AMDX_shader_enqueue", extension_count, extensions);

#endif
    GLAD_VK_AMD_anti_lag = glad_vk_has_extension("VK_AMD_anti_lag", extension_count, extensions);
    GLAD_VK_AMD_buffer_marker = glad_vk_has_extension("VK_AMD_buffer_marker", extension_count, extensions);
    GLAD_VK_AMD_device_coherent_memory = glad_vk_has_extension("VK_AMD_device_coherent_memory", extension_count, extensions);
    GLAD_VK_AMD_display_native_hdr = glad_vk_has_extension("VK_AMD_display_native_hdr", extension_count, extensions);
    GLAD_VK_AMD_draw_indirect_count = glad_vk_has_extension("VK_AMD_draw_indirect_count", extension_count, extensions);
    GLAD_VK_AMD_gcn_shader = glad_vk_has_extension("VK_AMD_gcn_shader", extension_count, extensions);
    GLAD_VK_AMD_gpu_shader_half_float = glad_vk_has_extension("VK_AMD_gpu_shader_half_float", extension_count, extensions);
    GLAD_VK_AMD_gpu_shader_int16 = glad_vk_has_extension("VK_AMD_gpu_shader_int16", extension_count, extensions);
    GLAD_VK_AMD_memory_overallocation_behavior = glad_vk_has_extension("VK_AMD_memory_overallocation_behavior", extension_count, extensions);
    GLAD_VK_AMD_mixed_attachment_samples = glad_vk_has_extension("VK_AMD_mixed_attachment_samples", extension_count, extensions);
    GLAD_VK_AMD_negative_viewport_height = glad_vk_has_extension("VK_AMD_negative_viewport_height", extension_count, extensions);
    GLAD_VK_AMD_pipeline_compiler_control = glad_vk_has_extension("VK_AMD_pipeline_compiler_control", extension_count, extensions);
    GLAD_VK_AMD_rasterization_order = glad_vk_has_extension("VK_AMD_rasterization_order", extension_count, extensions);
    GLAD_VK_AMD_shader_ballot = glad_vk_has_extension("VK_AMD_shader_ballot", extension_count, extensions);
    GLAD_VK_AMD_shader_core_properties = glad_vk_has_extension("VK_AMD_shader_core_properties", extension_count, extensions);
    GLAD_VK_AMD_shader_core_properties2 = glad_vk_has_extension("VK_AMD_shader_core_properties2", extension_count, extensions);
    GLAD_VK_AMD_shader_early_and_late_fragment_tests = glad_vk_has_extension("VK_AMD_shader_early_and_late_fragment_tests", extension_count, extensions);
    GLAD_VK_AMD_shader_explicit_vertex_parameter = glad_vk_has_extension("VK_AMD_shader_explicit_vertex_parameter", extension_count, extensions);
    GLAD_VK_AMD_shader_fragment_mask = glad_vk_has_extension("VK_AMD_shader_fragment_mask", extension_count, extensions);
    GLAD_VK_AMD_shader_image_load_store_lod = glad_vk_has_extension("VK_AMD_shader_image_load_store_lod", extension_count, extensions);
    GLAD_VK_AMD_shader_info = glad_vk_has_extension("VK_AMD_shader_info", extension_count, extensions);
    GLAD_VK_AMD_shader_trinary_minmax = glad_vk_has_extension("VK_AMD_shader_trinary_minmax", extension_count, extensions);
    GLAD_VK_AMD_texture_gather_bias_lod = glad_vk_has_extension("VK_AMD_texture_gather_bias_lod", extension_count, extensions);
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    GLAD_VK_ANDROID_external_format_resolve = glad_vk_has_extension("VK_ANDROID_external_format_resolve", extension_count, extensions);

#endif
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    GLAD_VK_ANDROID_external_memory_android_hardware_buffer = glad_vk_has_extension("VK_ANDROID_external_memory_android_hardware_buffer", extension_count, extensions);

#endif
    GLAD_VK_ARM_pipeline_opacity_micromap = glad_vk_has_extension("VK_ARM_pipeline_opacity_micromap", extension_count, extensions);
    GLAD_VK_ARM_rasterization_order_attachment_access = glad_vk_has_extension("VK_ARM_rasterization_order_attachment_access", extension_count, extensions);
    GLAD_VK_ARM_render_pass_striped = glad_vk_has_extension("VK_ARM_render_pass_striped", extension_count, extensions);
    GLAD_VK_ARM_scheduling_controls = glad_vk_has_extension("VK_ARM_scheduling_controls", extension_count, extensions);
    GLAD_VK_ARM_shader_core_builtins = glad_vk_has_extension("VK_ARM_shader_core_builtins", extension_count, extensions);
    GLAD_VK_ARM_shader_core_properties = glad_vk_has_extension("VK_ARM_shader_core_properties", extension_count, extensions);
    GLAD_VK_EXT_4444_formats = glad_vk_has_extension("VK_EXT_4444_formats", extension_count, extensions);
    GLAD_VK_EXT_acquire_drm_display = glad_vk_has_extension("VK_EXT_acquire_drm_display", extension_count, extensions);
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    GLAD_VK_EXT_acquire_xlib_display = glad_vk_has_extension("VK_EXT_acquire_xlib_display", extension_count, extensions);

#endif
    GLAD_VK_EXT_astc_decode_mode = glad_vk_has_extension("VK_EXT_astc_decode_mode", extension_count, extensions);
    GLAD_VK_EXT_attachment_feedback_loop_dynamic_state = glad_vk_has_extension("VK_EXT_attachment_feedback_loop_dynamic_state", extension_count, extensions);
    GLAD_VK_EXT_attachment_feedback_loop_layout = glad_vk_has_extension("VK_EXT_attachment_feedback_loop_layout", extension_count, extensions);
    GLAD_VK_EXT_blend_operation_advanced = glad_vk_has_extension("VK_EXT_blend_operation_advanced", extension_count, extensions);
    GLAD_VK_EXT_border_color_swizzle = glad_vk_has_extension("VK_EXT_border_color_swizzle", extension_count, extensions);
    GLAD_VK_EXT_buffer_device_address = glad_vk_has_extension("VK_EXT_buffer_device_address", extension_count, extensions);
    GLAD_VK_EXT_calibrated_timestamps = glad_vk_has_extension("VK_EXT_calibrated_timestamps", extension_count, extensions);
    GLAD_VK_EXT_color_write_enable = glad_vk_has_extension("VK_EXT_color_write_enable", extension_count, extensions);
    GLAD_VK_EXT_conditional_rendering = glad_vk_has_extension("VK_EXT_conditional_rendering", extension_count, extensions);
    GLAD_VK_EXT_conservative_rasterization = glad_vk_has_extension("VK_EXT_conservative_rasterization", extension_count, extensions);
    GLAD_VK_EXT_custom_border_color = glad_vk_has_extension("VK_EXT_custom_border_color", extension_count, extensions);
    GLAD_VK_EXT_debug_marker = glad_vk_has_extension("VK_EXT_debug_marker", extension_count, extensions);
    GLAD_VK_EXT_debug_report = glad_vk_has_extension("VK_EXT_debug_report", extension_count, extensions);
    GLAD_VK_EXT_debug_utils = glad_vk_has_extension("VK_EXT_debug_utils", extension_count, extensions);
    GLAD_VK_EXT_depth_bias_control = glad_vk_has_extension("VK_EXT_depth_bias_control", extension_count, extensions);
    GLAD_VK_EXT_depth_clamp_control = glad_vk_has_extension("VK_EXT_depth_clamp_control", extension_count, extensions);
    GLAD_VK_EXT_depth_clamp_zero_one = glad_vk_has_extension("VK_EXT_depth_clamp_zero_one", extension_count, extensions);
    GLAD_VK_EXT_depth_clip_control = glad_vk_has_extension("VK_EXT_depth_clip_control", extension_count, extensions);
    GLAD_VK_EXT_depth_clip_enable = glad_vk_has_extension("VK_EXT_depth_clip_enable", extension_count, extensions);
    GLAD_VK_EXT_depth_range_unrestricted = glad_vk_has_extension("VK_EXT_depth_range_unrestricted", extension_count, extensions);
    GLAD_VK_EXT_descriptor_buffer = glad_vk_has_extension("VK_EXT_descriptor_buffer", extension_count, extensions);
    GLAD_VK_EXT_descriptor_indexing = glad_vk_has_extension("VK_EXT_descriptor_indexing", extension_count, extensions);
    GLAD_VK_EXT_device_address_binding_report = glad_vk_has_extension("VK_EXT_device_address_binding_report", extension_count, extensions);
    GLAD_VK_EXT_device_fault = glad_vk_has_extension("VK_EXT_device_fault", extension_count, extensions);
    GLAD_VK_EXT_device_generated_commands = glad_vk_has_extension("VK_EXT_device_generated_commands", extension_count, extensions);
    GLAD_VK_EXT_device_memory_report = glad_vk_has_extension("VK_EXT_device_memory_report", extension_count, extensions);
    GLAD_VK_EXT_direct_mode_display = glad_vk_has_extension("VK_EXT_direct_mode_display", extension_count, extensions);
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    GLAD_VK_EXT_directfb_surface = glad_vk_has_extension("VK_EXT_directfb_surface", extension_count, extensions);

#endif
    GLAD_VK_EXT_discard_rectangles = glad_vk_has_extension("VK_EXT_discard_rectangles", extension_count, extensions);
    GLAD_VK_EXT_display_control = glad_vk_has_extension("VK_EXT_display_control", extension_count, extensions);
    GLAD_VK_EXT_display_surface_counter = glad_vk_has_extension("VK_EXT_display_surface_counter", extension_count, extensions);
    GLAD_VK_EXT_dynamic_rendering_unused_attachments = glad_vk_has_extension("VK_EXT_dynamic_rendering_unused_attachments", extension_count, extensions);
    GLAD_VK_EXT_extended_dynamic_state = glad_vk_has_extension("VK_EXT_extended_dynamic_state", extension_count, extensions);
    GLAD_VK_EXT_extended_dynamic_state2 = glad_vk_has_extension("VK_EXT_extended_dynamic_state2", extension_count, extensions);
    GLAD_VK_EXT_extended_dynamic_state3 = glad_vk_has_extension("VK_EXT_extended_dynamic_state3", extension_count, extensions);
    GLAD_VK_EXT_external_memory_acquire_unmodified = glad_vk_has_extension("VK_EXT_external_memory_acquire_unmodified", extension_count, extensions);
    GLAD_VK_EXT_external_memory_dma_buf = glad_vk_has_extension("VK_EXT_external_memory_dma_buf", extension_count, extensions);
    GLAD_VK_EXT_external_memory_host = glad_vk_has_extension("VK_EXT_external_memory_host", extension_count, extensions);
#if defined(VK_USE_PLATFORM_METAL_EXT)
    GLAD_VK_EXT_external_memory_metal = glad_vk_has_extension("VK_EXT_external_memory_metal", extension_count, extensions);

#endif
    GLAD_VK_EXT_filter_cubic = glad_vk_has_extension("VK_EXT_filter_cubic", extension_count, extensions);
    GLAD_VK_EXT_fragment_density_map = glad_vk_has_extension("VK_EXT_fragment_density_map", extension_count, extensions);
    GLAD_VK_EXT_fragment_density_map2 = glad_vk_has_extension("VK_EXT_fragment_density_map2", extension_count, extensions);
    GLAD_VK_EXT_fragment_density_map_offset = glad_vk_has_extension("VK_EXT_fragment_density_map_offset", extension_count, extensions);
    GLAD_VK_EXT_fragment_shader_interlock = glad_vk_has_extension("VK_EXT_fragment_shader_interlock", extension_count, extensions);
    GLAD_VK_EXT_frame_boundary = glad_vk_has_extension("VK_EXT_frame_boundary", extension_count, extensions);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    GLAD_VK_EXT_full_screen_exclusive = glad_vk_has_extension("VK_EXT_full_screen_exclusive", extension_count, extensions);

#endif
    GLAD_VK_EXT_global_priority = glad_vk_has_extension("VK_EXT_global_priority", extension_count, extensions);
    GLAD_VK_EXT_global_priority_query = glad_vk_has_extension("VK_EXT_global_priority_query", extension_count, extensions);
    GLAD_VK_EXT_graphics_pipeline_library = glad_vk_has_extension("VK_EXT_graphics_pipeline_library", extension_count, extensions);
    GLAD_VK_EXT_hdr_metadata = glad_vk_has_extension("VK_EXT_hdr_metadata", extension_count, extensions);
    GLAD_VK_EXT_headless_surface = glad_vk_has_extension("VK_EXT_headless_surface", extension_count, extensions);
    GLAD_VK_EXT_host_image_copy = glad_vk_has_extension("VK_EXT_host_image_copy", extension_count, extensions);
    GLAD_VK_EXT_host_query_reset = glad_vk_has_extension("VK_EXT_host_query_reset", extension_count, extensions);
    GLAD_VK_EXT_image_2d_view_of_3d = glad_vk_has_extension("VK_EXT_image_2d_view_of_3d", extension_count, extensions);
    GLAD_VK_EXT_image_compression_control = glad_vk_has_extension("VK_EXT_image_compression_control", extension_count, extensions);
    GLAD_VK_EXT_image_compression_control_swapchain = glad_vk_has_extension("VK_EXT_image_compression_control_swapchain", extension_count, extensions);
    GLAD_VK_EXT_image_drm_format_modifier = glad_vk_has_extension("VK_EXT_image_drm_format_modifier", extension_count, extensions);
    GLAD_VK_EXT_image_robustness = glad_vk_has_extension("VK_EXT_image_robustness", extension_count, extensions);
    GLAD_VK_EXT_image_sliced_view_of_3d = glad_vk_has_extension("VK_EXT_image_sliced_view_of_3d", extension_count, extensions);
    GLAD_VK_EXT_image_view_min_lod = glad_vk_has_extension("VK_EXT_image_view_min_lod", extension_count, extensions);
    GLAD_VK_EXT_index_type_uint8 = glad_vk_has_extension("VK_EXT_index_type_uint8", extension_count, extensions);
    GLAD_VK_EXT_inline_uniform_block = glad_vk_has_extension("VK_EXT_inline_uniform_block", extension_count, extensions);
    GLAD_VK_EXT_layer_settings = glad_vk_has_extension("VK_EXT_layer_settings", extension_count, extensions);
    GLAD_VK_EXT_legacy_dithering = glad_vk_has_extension("VK_EXT_legacy_dithering", extension_count, extensions);
    GLAD_VK_EXT_legacy_vertex_attributes = glad_vk_has_extension("VK_EXT_legacy_vertex_attributes", extension_count, extensions);
    GLAD_VK_EXT_line_rasterization = glad_vk_has_extension("VK_EXT_line_rasterization", extension_count, extensions);
    GLAD_VK_EXT_load_store_op_none = glad_vk_has_extension("VK_EXT_load_store_op_none", extension_count, extensions);
    GLAD_VK_EXT_map_memory_placed = glad_vk_has_extension("VK_EXT_map_memory_placed", extension_count, extensions);
    GLAD_VK_EXT_memory_budget = glad_vk_has_extension("VK_EXT_memory_budget", extension_count, extensions);
    GLAD_VK_EXT_memory_priority = glad_vk_has_extension("VK_EXT_memory_priority", extension_count, extensions);
    GLAD_VK_EXT_mesh_shader = glad_vk_has_extension("VK_EXT_mesh_shader", extension_count, extensions);
#if defined(VK_USE_PLATFORM_METAL_EXT)
    GLAD_VK_EXT_metal_objects = glad_vk_has_extension("VK_EXT_metal_objects", extension_count, extensions);

#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
    GLAD_VK_EXT_metal_surface = glad_vk_has_extension("VK_EXT_metal_surface", extension_count, extensions);

#endif
    GLAD_VK_EXT_multi_draw = glad_vk_has_extension("VK_EXT_multi_draw", extension_count, extensions);
    GLAD_VK_EXT_multisampled_render_to_single_sampled = glad_vk_has_extension("VK_EXT_multisampled_render_to_single_sampled", extension_count, extensions);
    GLAD_VK_EXT_mutable_descriptor_type = glad_vk_has_extension("VK_EXT_mutable_descriptor_type", extension_count, extensions);
    GLAD_VK_EXT_nested_command_buffer = glad_vk_has_extension("VK_EXT_nested_command_buffer", extension_count, extensions);
    GLAD_VK_EXT_non_seamless_cube_map = glad_vk_has_extension("VK_EXT_non_seamless_cube_map", extension_count, extensions);
    GLAD_VK_EXT_opacity_micromap = glad_vk_has_extension("VK_EXT_opacity_micromap", extension_count, extensions);
    GLAD_VK_EXT_pageable_device_local_memory = glad_vk_has_extension("VK_EXT_pageable_device_local_memory", extension_count, extensions);
    GLAD_VK_EXT_pci_bus_info = glad_vk_has_extension("VK_EXT_pci_bus_info", extension_count, extensions);
    GLAD_VK_EXT_physical_device_drm = glad_vk_has_extension("VK_EXT_physical_device_drm", extension_count, extensions);
    GLAD_VK_EXT_pipeline_creation_cache_control = glad_vk_has_extension("VK_EXT_pipeline_creation_cache_control", extension_count, extensions);
    GLAD_VK_EXT_pipeline_creation_feedback = glad_vk_has_extension("VK_EXT_pipeline_creation_feedback", extension_count, extensions);
    GLAD_VK_EXT_pipeline_library_group_handles = glad_vk_has_extension("VK_EXT_pipeline_library_group_handles", extension_count, extensions);
    GLAD_VK_EXT_pipeline_properties = glad_vk_has_extension("VK_EXT_pipeline_properties", extension_count, extensions);
    GLAD_VK_EXT_pipeline_protected_access = glad_vk_has_extension("VK_EXT_pipeline_protected_access", extension_count, extensions);
    GLAD_VK_EXT_pipeline_robustness = glad_vk_has_extension("VK_EXT_pipeline_robustness", extension_count, extensions);
    GLAD_VK_EXT_post_depth_coverage = glad_vk_has_extension("VK_EXT_post_depth_coverage", extension_count, extensions);
    GLAD_VK_EXT_present_mode_fifo_latest_ready = glad_vk_has_extension("VK_EXT_present_mode_fifo_latest_ready", extension_count, extensions);
    GLAD_VK_EXT_primitive_topology_list_restart = glad_vk_has_extension("VK_EXT_primitive_topology_list_restart", extension_count, extensions);
    GLAD_VK_EXT_primitives_generated_query = glad_vk_has_extension("VK_EXT_primitives_generated_query", extension_count, extensions);
    GLAD_VK_EXT_private_data = glad_vk_has_extension("VK_EXT_private_data", extension_count, extensions);
    GLAD_VK_EXT_provoking_vertex = glad_vk_has_extension("VK_EXT_provoking_vertex", extension_count, extensions);
    GLAD_VK_EXT_queue_family_foreign = glad_vk_has_extension("VK_EXT_queue_family_foreign", extension_count, extensions);
    GLAD_VK_EXT_rasterization_order_attachment_access = glad_vk_has_extension("VK_EXT_rasterization_order_attachment_access", extension_count, extensions);
    GLAD_VK_EXT_rgba10x6_formats = glad_vk_has_extension("VK_EXT_rgba10x6_formats", extension_count, extensions);
    GLAD_VK_EXT_robustness2 = glad_vk_has_extension("VK_EXT_robustness2", extension_count, extensions);
    GLAD_VK_EXT_sample_locations = glad_vk_has_extension("VK_EXT_sample_locations", extension_count, extensions);
    GLAD_VK_EXT_sampler_filter_minmax = glad_vk_has_extension("VK_EXT_sampler_filter_minmax", extension_count, extensions);
    GLAD_VK_EXT_scalar_block_layout = glad_vk_has_extension("VK_EXT_scalar_block_layout", extension_count, extensions);
    GLAD_VK_EXT_separate_stencil_usage = glad_vk_has_extension("VK_EXT_separate_stencil_usage", extension_count, extensions);
    GLAD_VK_EXT_shader_atomic_float = glad_vk_has_extension("VK_EXT_shader_atomic_float", extension_count, extensions);
    GLAD_VK_EXT_shader_atomic_float2 = glad_vk_has_extension("VK_EXT_shader_atomic_float2", extension_count, extensions);
    GLAD_VK_EXT_shader_demote_to_helper_invocation = glad_vk_has_extension("VK_EXT_shader_demote_to_helper_invocation", extension_count, extensions);
    GLAD_VK_EXT_shader_image_atomic_int64 = glad_vk_has_extension("VK_EXT_shader_image_atomic_int64", extension_count, extensions);
    GLAD_VK_EXT_shader_module_identifier = glad_vk_has_extension("VK_EXT_shader_module_identifier", extension_count, extensions);
    GLAD_VK_EXT_shader_object = glad_vk_has_extension("VK_EXT_shader_object", extension_count, extensions);
    GLAD_VK_EXT_shader_replicated_composites = glad_vk_has_extension("VK_EXT_shader_replicated_composites", extension_count, extensions);
    GLAD_VK_EXT_shader_stencil_export = glad_vk_has_extension("VK_EXT_shader_stencil_export", extension_count, extensions);
    GLAD_VK_EXT_shader_subgroup_ballot = glad_vk_has_extension("VK_EXT_shader_subgroup_ballot", extension_count, extensions);
    GLAD_VK_EXT_shader_subgroup_vote = glad_vk_has_extension("VK_EXT_shader_subgroup_vote", extension_count, extensions);
    GLAD_VK_EXT_shader_tile_image = glad_vk_has_extension("VK_EXT_shader_tile_image", extension_count, extensions);
    GLAD_VK_EXT_shader_viewport_index_layer = glad_vk_has_extension("VK_EXT_shader_viewport_index_layer", extension_count, extensions);
    GLAD_VK_EXT_subgroup_size_control = glad_vk_has_extension("VK_EXT_subgroup_size_control", extension_count, extensions);
    GLAD_VK_EXT_subpass_merge_feedback = glad_vk_has_extension("VK_EXT_subpass_merge_feedback", extension_count, extensions);
    GLAD_VK_EXT_surface_maintenance1 = glad_vk_has_extension("VK_EXT_surface_maintenance1", extension_count, extensions);
    GLAD_VK_EXT_swapchain_colorspace = glad_vk_has_extension("VK_EXT_swapchain_colorspace", extension_count, extensions);
    GLAD_VK_EXT_swapchain_maintenance1 = glad_vk_has_extension("VK_EXT_swapchain_maintenance1", extension_count, extensions);
    GLAD_VK_EXT_texel_buffer_alignment = glad_vk_has_extension("VK_EXT_texel_buffer_alignment", extension_count, extensions);
    GLAD_VK_EXT_texture_compression_astc_hdr = glad_vk_has_extension("VK_EXT_texture_compression_astc_hdr", extension_count, extensions);
    GLAD_VK_EXT_tooling_info = glad_vk_has_extension("VK_EXT_tooling_info", extension_count, extensions);
    GLAD_VK_EXT_transform_feedback = glad_vk_has_extension("VK_EXT_transform_feedback", extension_count, extensions);
    GLAD_VK_EXT_validation_cache = glad_vk_has_extension("VK_EXT_validation_cache", extension_count, extensions);
    GLAD_VK_EXT_validation_features = glad_vk_has_extension("VK_EXT_validation_features", extension_count, extensions);
    GLAD_VK_EXT_validation_flags = glad_vk_has_extension("VK_EXT_validation_flags", extension_count, extensions);
    GLAD_VK_EXT_vertex_attribute_divisor = glad_vk_has_extension("VK_EXT_vertex_attribute_divisor", extension_count, extensions);
    GLAD_VK_EXT_vertex_attribute_robustness = glad_vk_has_extension("VK_EXT_vertex_attribute_robustness", extension_count, extensions);
    GLAD_VK_EXT_vertex_input_dynamic_state = glad_vk_has_extension("VK_EXT_vertex_input_dynamic_state", extension_count, extensions);
    GLAD_VK_EXT_ycbcr_2plane_444_formats = glad_vk_has_extension("VK_EXT_ycbcr_2plane_444_formats", extension_count, extensions);
    GLAD_VK_EXT_ycbcr_image_arrays = glad_vk_has_extension("VK_EXT_ycbcr_image_arrays", extension_count, extensions);
#if defined(VK_USE_PLATFORM_FUCHSIA)
    GLAD_VK_FUCHSIA_buffer_collection = glad_vk_has_extension("VK_FUCHSIA_buffer_collection", extension_count, extensions);

#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
    GLAD_VK_FUCHSIA_external_memory = glad_vk_has_extension("VK_FUCHSIA_external_memory", extension_count, extensions);

#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
    GLAD_VK_FUCHSIA_external_semaphore = glad_vk_has_extension("VK_FUCHSIA_external_semaphore", extension_count, extensions);

#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
    GLAD_VK_FUCHSIA_imagepipe_surface = glad_vk_has_extension("VK_FUCHSIA_imagepipe_surface", extension_count, extensions);

#endif
#if defined(VK_USE_PLATFORM_GGP)
    GLAD_VK_GGP_frame_token = glad_vk_has_extension("VK_GGP_frame_token", extension_count, extensions);

#endif
#if defined(VK_USE_PLATFORM_GGP)
    GLAD_VK_GGP_stream_descriptor_surface = glad_vk_has_extension("VK_GGP_stream_descriptor_surface", extension_count, extensions);

#endif
    GLAD_VK_GOOGLE_decorate_string = glad_vk_has_extension("VK_GOOGLE_decorate_string", extension_count, extensions);
    GLAD_VK_GOOGLE_display_timing = glad_vk_has_extension("VK_GOOGLE_display_timing", extension_count, extensions);
    GLAD_VK_GOOGLE_hlsl_functionality1 = glad_vk_has_extension("VK_GOOGLE_hlsl_functionality1", extension_count, extensions);
    GLAD_VK_GOOGLE_surfaceless_query = glad_vk_has_extension("VK_GOOGLE_surfaceless_query", extension_count, extensions);
    GLAD_VK_GOOGLE_user_type = glad_vk_has_extension("VK_GOOGLE_user_type", extension_count, extensions);
    GLAD_VK_HUAWEI_cluster_culling_shader = glad_vk_has_extension("VK_HUAWEI_cluster_culling_shader", extension_count, extensions);
    GLAD_VK_HUAWEI_hdr_vivid = glad_vk_has_extension("VK_HUAWEI_hdr_vivid", extension_count, extensions);
    GLAD_VK_HUAWEI_invocation_mask = glad_vk_has_extension("VK_HUAWEI_invocation_mask", extension_count, extensions);
    GLAD_VK_HUAWEI_subpass_shading = glad_vk_has_extension("VK_HUAWEI_subpass_shading", extension_count, extensions);
    GLAD_VK_IMG_filter_cubic = glad_vk_has_extension("VK_IMG_filter_cubic", extension_count, extensions);
    GLAD_VK_IMG_format_pvrtc = glad_vk_has_extension("VK_IMG_format_pvrtc", extension_count, extensions);
    GLAD_VK_IMG_relaxed_line_rasterization = glad_vk_has_extension("VK_IMG_relaxed_line_rasterization", extension_count, extensions);
    GLAD_VK_INTEL_performance_query = glad_vk_has_extension("VK_INTEL_performance_query", extension_count, extensions);
    GLAD_VK_INTEL_shader_integer_functions2 = glad_vk_has_extension("VK_INTEL_shader_integer_functions2", extension_count, extensions);
    GLAD_VK_KHR_16bit_storage = glad_vk_has_extension("VK_KHR_16bit_storage", extension_count, extensions);
    GLAD_VK_KHR_8bit_storage = glad_vk_has_extension("VK_KHR_8bit_storage", extension_count, extensions);
    GLAD_VK_KHR_acceleration_structure = glad_vk_has_extension("VK_KHR_acceleration_structure", extension_count, extensions);
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    GLAD_VK_KHR_android_surface = glad_vk_has_extension("VK_KHR_android_surface", extension_count, extensions);

#endif
    GLAD_VK_KHR_bind_memory2 = glad_vk_has_extension("VK_KHR_bind_memory2", extension_count, extensions);
    GLAD_VK_KHR_buffer_device_address = glad_vk_has_extension("VK_KHR_buffer_device_address", extension_count, extensions);
    GLAD_VK_KHR_calibrated_timestamps = glad_vk_has_extension("VK_KHR_calibrated_timestamps", extension_count, extensions);
    GLAD_VK_KHR_compute_shader_derivatives = glad_vk_has_extension("VK_KHR_compute_shader_derivatives", extension_count, extensions);
    GLAD_VK_KHR_cooperative_matrix = glad_vk_has_extension("VK_KHR_cooperative_matrix", extension_count, extensions);
    GLAD_VK_KHR_copy_commands2 = glad_vk_has_extension("VK_KHR_copy_commands2", extension_count, extensions);
    GLAD_VK_KHR_create_renderpass2 = glad_vk_has_extension("VK_KHR_create_renderpass2", extension_count, extensions);
    GLAD_VK_KHR_dedicated_allocation = glad_vk_has_extension("VK_KHR_dedicated_allocation", extension_count, extensions);
    GLAD_VK_KHR_deferred_host_operations = glad_vk_has_extension("VK_KHR_deferred_host_operations", extension_count, extensions);
    GLAD_VK_KHR_depth_clamp_zero_one = glad_vk_has_extension("VK_KHR_depth_clamp_zero_one", extension_count, extensions);
    GLAD_VK_KHR_depth_stencil_resolve = glad_vk_has_extension("VK_KHR_depth_stencil_resolve", extension_count, extensions);
    GLAD_VK_KHR_descriptor_update_template = glad_vk_has_extension("VK_KHR_descriptor_update_template", extension_count, extensions);
    GLAD_VK_KHR_device_group = glad_vk_has_extension("VK_KHR_device_group", extension_count, extensions);
    GLAD_VK_KHR_device_group_creation = glad_vk_has_extension("VK_KHR_device_group_creation", extension_count, extensions);
    GLAD_VK_KHR_display = glad_vk_has_extension("VK_KHR_display", extension_count, extensions);
    GLAD_VK_KHR_display_swapchain = glad_vk_has_extension("VK_KHR_display_swapchain", extension_count, extensions);
    GLAD_VK_KHR_draw_indirect_count = glad_vk_has_extension("VK_KHR_draw_indirect_count", extension_count, extensions);
    GLAD_VK_KHR_driver_properties = glad_vk_has_extension("VK_KHR_driver_properties", extension_count, extensions);
    GLAD_VK_KHR_dynamic_rendering = glad_vk_has_extension("VK_KHR_dynamic_rendering", extension_count, extensions);
    GLAD_VK_KHR_dynamic_rendering_local_read = glad_vk_has_extension("VK_KHR_dynamic_rendering_local_read", extension_count, extensions);
    GLAD_VK_KHR_external_fence = glad_vk_has_extension("VK_KHR_external_fence", extension_count, extensions);
    GLAD_VK_KHR_external_fence_capabilities = glad_vk_has_extension("VK_KHR_external_fence_capabilities", extension_count, extensions);
    GLAD_VK_KHR_external_fence_fd = glad_vk_has_extension("VK_KHR_external_fence_fd", extension_count, extensions);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    GLAD_VK_KHR_external_fence_win32 = glad_vk_has_extension("VK_KHR_external_fence_win32", extension_count, extensions);

#endif
    GLAD_VK_KHR_external_memory = glad_vk_has_extension("VK_KHR_external_memory", extension_count, extensions);
    GLAD_VK_KHR_external_memory_capabilities = glad_vk_has_extension("VK_KHR_external_memory_capabilities", extension_count, extensions);
    GLAD_VK_KHR_external_memory_fd = glad_vk_has_extension("VK_KHR_external_memory_fd", extension_count, extensions);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    GLAD_VK_KHR_external_memory_win32 = glad_vk_has_extension("VK_KHR_external_memory_win32", extension_count, extensions);

#endif
    GLAD_VK_KHR_external_semaphore = glad_vk_has_extension("VK_KHR_external_semaphore", extension_count, extensions);
    GLAD_VK_KHR_external_semaphore_capabilities = glad_vk_has_extension("VK_KHR_external_semaphore_capabilities", extension_count, extensions);
    GLAD_VK_KHR_external_semaphore_fd = glad_vk_has_extension("VK_KHR_external_semaphore_fd", extension_count, extensions);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    GLAD_VK_KHR_external_semaphore_win32 = glad_vk_has_extension("VK_KHR_external_semaphore_win32", extension_count, extensions);

#endif
    GLAD_VK_KHR_format_feature_flags2 = glad_vk_has_extension("VK_KHR_format_feature_flags2", extension_count, extensions);
    GLAD_VK_KHR_fragment_shader_barycentric = glad_vk_has_extension("VK_KHR_fragment_shader_barycentric", extension_count, extensions);
    GLAD_VK_KHR_fragment_shading_rate = glad_vk_has_extension("VK_KHR_fragment_shading_rate", extension_count, extensions);
    GLAD_VK_KHR_get_display_properties2 = glad_vk_has_extension("VK_KHR_get_display_properties2", extension_count, extensions);
    GLAD_VK_KHR_get_memory_requirements2 = glad_vk_has_extension("VK_KHR_get_memory_requirements2", extension_count, extensions);
    GLAD_VK_KHR_get_physical_device_properties2 = glad_vk_has_extension("VK_KHR_get_physical_device_properties2", extension_count, extensions);
    GLAD_VK_KHR_get_surface_capabilities2 = glad_vk_has_extension("VK_KHR_get_surface_capabilities2", extension_count, extensions);
    GLAD_VK_KHR_global_priority = glad_vk_has_extension("VK_KHR_global_priority", extension_count, extensions);
    GLAD_VK_KHR_image_format_list = glad_vk_has_extension("VK_KHR_image_format_list", extension_count, extensions);
    GLAD_VK_KHR_imageless_framebuffer = glad_vk_has_extension("VK_KHR_imageless_framebuffer", extension_count, extensions);
    GLAD_VK_KHR_incremental_present = glad_vk_has_extension("VK_KHR_incremental_present", extension_count, extensions);
    GLAD_VK_KHR_index_type_uint8 = glad_vk_has_extension("VK_KHR_index_type_uint8", extension_count, extensions);
    GLAD_VK_KHR_line_rasterization = glad_vk_has_extension("VK_KHR_line_rasterization", extension_count, extensions);
    GLAD_VK_KHR_load_store_op_none = glad_vk_has_extension("VK_KHR_load_store_op_none", extension_count, extensions);
    GLAD_VK_KHR_maintenance1 = glad_vk_has_extension("VK_KHR_maintenance1", extension_count, extensions);
    GLAD_VK_KHR_maintenance2 = glad_vk_has_extension("VK_KHR_maintenance2", extension_count, extensions);
    GLAD_VK_KHR_maintenance3 = glad_vk_has_extension("VK_KHR_maintenance3", extension_count, extensions);
    GLAD_VK_KHR_maintenance4 = glad_vk_has_extension("VK_KHR_maintenance4", extension_count, extensions);
    GLAD_VK_KHR_maintenance5 = glad_vk_has_extension("VK_KHR_maintenance5", extension_count, extensions);
    GLAD_VK_KHR_maintenance6 = glad_vk_has_extension("VK_KHR_maintenance6", extension_count, extensions);
    GLAD_VK_KHR_maintenance7 = glad_vk_has_extension("VK_KHR_maintenance7", extension_count, extensions);
    GLAD_VK_KHR_maintenance8 = glad_vk_has_extension("VK_KHR_maintenance8", extension_count, extensions);
    GLAD_VK_KHR_map_memory2 = glad_vk_has_extension("VK_KHR_map_memory2", extension_count, extensions);
    GLAD_VK_KHR_multiview = glad_vk_has_extension("VK_KHR_multiview", extension_count, extensions);
    GLAD_VK_KHR_performance_query = glad_vk_has_extension("VK_KHR_performance_query", extension_count, extensions);
    GLAD_VK_KHR_pipeline_binary = glad_vk_has_extension("VK_KHR_pipeline_binary", extension_count, extensions);
    GLAD_VK_KHR_pipeline_executable_properties = glad_vk_has_extension("VK_KHR_pipeline_executable_properties", extension_count, extensions);
    GLAD_VK_KHR_pipeline_library = glad_vk_has_extension("VK_KHR_pipeline_library", extension_count, extensions);
    GLAD_VK_KHR_portability_enumeration = glad_vk_has_extension("VK_KHR_portability_enumeration", extension_count, extensions);
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    GLAD_VK_KHR_portability_subset = glad_vk_has_extension("VK_KHR_portability_subset", extension_count, extensions);

#endif
    GLAD_VK_KHR_present_id = glad_vk_has_extension("VK_KHR_present_id", extension_count, extensions);
    GLAD_VK_KHR_present_wait = glad_vk_has_extension("VK_KHR_present_wait", extension_count, extensions);
    GLAD_VK_KHR_push_descriptor = glad_vk_has_extension("VK_KHR_push_descriptor", extension_count, extensions);
    GLAD_VK_KHR_ray_query = glad_vk_has_extension("VK_KHR_ray_query", extension_count, extensions);
    GLAD_VK_KHR_ray_tracing_maintenance1 = glad_vk_has_extension("VK_KHR_ray_tracing_maintenance1", extension_count, extensions);
    GLAD_VK_KHR_ray_tracing_pipeline = glad_vk_has_extension("VK_KHR_ray_tracing_pipeline", extension_count, extensions);
    GLAD_VK_KHR_ray_tracing_position_fetch = glad_vk_has_extension("VK_KHR_ray_tracing_position_fetch", extension_count, extensions);
    GLAD_VK_KHR_relaxed_block_layout = glad_vk_has_extension("VK_KHR_relaxed_block_layout", extension_count, extensions);
    GLAD_VK_KHR_robustness2 = glad_vk_has_extension("VK_KHR_robustness2", extension_count, extensions);
    GLAD_VK_KHR_sampler_mirror_clamp_to_edge = glad_vk_has_extension("VK_KHR_sampler_mirror_clamp_to_edge", extension_count, extensions);
    GLAD_VK_KHR_sampler_ycbcr_conversion = glad_vk_has_extension("VK_KHR_sampler_ycbcr_conversion", extension_count, extensions);
    GLAD_VK_KHR_separate_depth_stencil_layouts = glad_vk_has_extension("VK_KHR_separate_depth_stencil_layouts", extension_count, extensions);
    GLAD_VK_KHR_shader_atomic_int64 = glad_vk_has_extension("VK_KHR_shader_atomic_int64", extension_count, extensions);
    GLAD_VK_KHR_shader_bfloat16 = glad_vk_has_extension("VK_KHR_shader_bfloat16", extension_count, extensions);
    GLAD_VK_KHR_shader_clock = glad_vk_has_extension("VK_KHR_shader_clock", extension_count, extensions);
    GLAD_VK_KHR_shader_draw_parameters = glad_vk_has_extension("VK_KHR_shader_draw_parameters", extension_count, extensions);
    GLAD_VK_KHR_shader_expect_assume = glad_vk_has_extension("VK_KHR_shader_expect_assume", extension_count, extensions);
    GLAD_VK_KHR_shader_float16_int8 = glad_vk_has_extension("VK_KHR_shader_float16_int8", extension_count, extensions);
    GLAD_VK_KHR_shader_float_controls = glad_vk_has_extension("VK_KHR_shader_float_controls", extension_count, extensions);
    GLAD_VK_KHR_shader_float_controls2 = glad_vk_has_extension("VK_KHR_shader_float_controls2", extension_count, extensions);
    GLAD_VK_KHR_shader_integer_dot_product = glad_vk_has_extension("VK_KHR_shader_integer_dot_product", extension_count, extensions);
    GLAD_VK_KHR_shader_maximal_reconvergence = glad_vk_has_extension("VK_KHR_shader_maximal_reconvergence", extension_count, extensions);
    GLAD_VK_KHR_shader_non_semantic_info = glad_vk_has_extension("VK_KHR_shader_non_semantic_info", extension_count, extensions);
    GLAD_VK_KHR_shader_quad_control = glad_vk_has_extension("VK_KHR_shader_quad_control", extension_count, extensions);
    GLAD_VK_KHR_shader_relaxed_extended_instruction = glad_vk_has_extension("VK_KHR_shader_relaxed_extended_instruction", extension_count, extensions);
    GLAD_VK_KHR_shader_subgroup_extended_types = glad_vk_has_extension("VK_KHR_shader_subgroup_extended_types", extension_count, extensions);
    GLAD_VK_KHR_shader_subgroup_rotate = glad_vk_has_extension("VK_KHR_shader_subgroup_rotate", extension_count, extensions);
    GLAD_VK_KHR_shader_subgroup_uniform_control_flow = glad_vk_has_extension("VK_KHR_shader_subgroup_uniform_control_flow", extension_count, extensions);
    GLAD_VK_KHR_shader_terminate_invocation = glad_vk_has_extension("VK_KHR_shader_terminate_invocation", extension_count, extensions);
    GLAD_VK_KHR_shared_presentable_image = glad_vk_has_extension("VK_KHR_shared_presentable_image", extension_count, extensions);
    GLAD_VK_KHR_spirv_1_4 = glad_vk_has_extension("VK_KHR_spirv_1_4", extension_count, extensions);
    GLAD_VK_KHR_storage_buffer_storage_class = glad_vk_has_extension("VK_KHR_storage_buffer_storage_class", extension_count, extensions);
    GLAD_VK_KHR_surface = glad_vk_has_extension("VK_KHR_surface", extension_count, extensions);
    GLAD_VK_KHR_surface_protected_capabilities = glad_vk_has_extension("VK_KHR_surface_protected_capabilities", extension_count, extensions);
    GLAD_VK_KHR_swapchain = glad_vk_has_extension("VK_KHR_swapchain", extension_count, extensions);
    GLAD_VK_KHR_swapchain_mutable_format = glad_vk_has_extension("VK_KHR_swapchain_mutable_format", extension_count, extensions);
    GLAD_VK_KHR_synchronization2 = glad_vk_has_extension("VK_KHR_synchronization2", extension_count, extensions);
    GLAD_VK_KHR_timeline_semaphore = glad_vk_has_extension("VK_KHR_timeline_semaphore", extension_count, extensions);
    GLAD_VK_KHR_uniform_buffer_standard_layout = glad_vk_has_extension("VK_KHR_uniform_buffer_standard_layout", extension_count, extensions);
    GLAD_VK_KHR_variable_pointers = glad_vk_has_extension("VK_KHR_variable_pointers", extension_count, extensions);
    GLAD_VK_KHR_vertex_attribute_divisor = glad_vk_has_extension("VK_KHR_vertex_attribute_divisor", extension_count, extensions);
    GLAD_VK_KHR_vulkan_memory_model = glad_vk_has_extension("VK_KHR_vulkan_memory_model", extension_count, extensions);
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    GLAD_VK_KHR_wayland_surface = glad_vk_has_extension("VK_KHR_wayland_surface", extension_count, extensions);

#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    GLAD_VK_KHR_win32_keyed_mutex = glad_vk_has_extension("VK_KHR_win32_keyed_mutex", extension_count, extensions);

#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    GLAD_VK_KHR_win32_surface = glad_vk_has_extension("VK_KHR_win32_surface", extension_count, extensions);

#endif
    GLAD_VK_KHR_workgroup_memory_explicit_layout = glad_vk_has_extension("VK_KHR_workgroup_memory_explicit_layout", extension_count, extensions);
#if defined(VK_USE_PLATFORM_XCB_KHR)
    GLAD_VK_KHR_xcb_surface = glad_vk_has_extension("VK_KHR_xcb_surface", extension_count, extensions);

#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    GLAD_VK_KHR_xlib_surface = glad_vk_has_extension("VK_KHR_xlib_surface", extension_count, extensions);

#endif
    GLAD_VK_KHR_zero_initialize_workgroup_memory = glad_vk_has_extension("VK_KHR_zero_initialize_workgroup_memory", extension_count, extensions);
    GLAD_VK_LUNARG_direct_driver_loading = glad_vk_has_extension("VK_LUNARG_direct_driver_loading", extension_count, extensions);
    GLAD_VK_MESA_image_alignment_control = glad_vk_has_extension("VK_MESA_image_alignment_control", extension_count, extensions);
    GLAD_VK_MSFT_layered_driver = glad_vk_has_extension("VK_MSFT_layered_driver", extension_count, extensions);
#if defined(VK_USE_PLATFORM_IOS_MVK)
    GLAD_VK_MVK_ios_surface = glad_vk_has_extension("VK_MVK_ios_surface", extension_count, extensions);

#endif
#if defined(VK_USE_PLATFORM_MACOS_MVK)
    GLAD_VK_MVK_macos_surface = glad_vk_has_extension("VK_MVK_macos_surface", extension_count, extensions);

#endif
#if defined(VK_USE_PLATFORM_VI_NN)
    GLAD_VK_NN_vi_surface = glad_vk_has_extension("VK_NN_vi_surface", extension_count, extensions);

#endif
    GLAD_VK_NVX_binary_import = glad_vk_has_extension("VK_NVX_binary_import", extension_count, extensions);
    GLAD_VK_NVX_image_view_handle = glad_vk_has_extension("VK_NVX_image_view_handle", extension_count, extensions);
    GLAD_VK_NVX_multiview_per_view_attributes = glad_vk_has_extension("VK_NVX_multiview_per_view_attributes", extension_count, extensions);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    GLAD_VK_NV_acquire_winrt_display = glad_vk_has_extension("VK_NV_acquire_winrt_display", extension_count, extensions);

#endif
    GLAD_VK_NV_clip_space_w_scaling = glad_vk_has_extension("VK_NV_clip_space_w_scaling", extension_count, extensions);
    GLAD_VK_NV_cluster_acceleration_structure = glad_vk_has_extension("VK_NV_cluster_acceleration_structure", extension_count, extensions);
    GLAD_VK_NV_command_buffer_inheritance = glad_vk_has_extension("VK_NV_command_buffer_inheritance", extension_count, extensions);
    GLAD_VK_NV_compute_shader_derivatives = glad_vk_has_extension("VK_NV_compute_shader_derivatives", extension_count, extensions);
    GLAD_VK_NV_cooperative_matrix = glad_vk_has_extension("VK_NV_cooperative_matrix", extension_count, extensions);
    GLAD_VK_NV_cooperative_matrix2 = glad_vk_has_extension("VK_NV_cooperative_matrix2", extension_count, extensions);
    GLAD_VK_NV_cooperative_vector = glad_vk_has_extension("VK_NV_cooperative_vector", extension_count, extensions);
    GLAD_VK_NV_copy_memory_indirect = glad_vk_has_extension("VK_NV_copy_memory_indirect", extension_count, extensions);
    GLAD_VK_NV_corner_sampled_image = glad_vk_has_extension("VK_NV_corner_sampled_image", extension_count, extensions);
    GLAD_VK_NV_coverage_reduction_mode = glad_vk_has_extension("VK_NV_coverage_reduction_mode", extension_count, extensions);
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    GLAD_VK_NV_cuda_kernel_launch = glad_vk_has_extension("VK_NV_cuda_kernel_launch", extension_count, extensions);

#endif
    GLAD_VK_NV_dedicated_allocation = glad_vk_has_extension("VK_NV_dedicated_allocation", extension_count, extensions);
    GLAD_VK_NV_dedicated_allocation_image_aliasing = glad_vk_has_extension("VK_NV_dedicated_allocation_image_aliasing", extension_count, extensions);
    GLAD_VK_NV_descriptor_pool_overallocation = glad_vk_has_extension("VK_NV_descriptor_pool_overallocation", extension_count, extensions);
    GLAD_VK_NV_device_diagnostic_checkpoints = glad_vk_has_extension("VK_NV_device_diagnostic_checkpoints", extension_count, extensions);
    GLAD_VK_NV_device_diagnostics_config = glad_vk_has_extension("VK_NV_device_diagnostics_config", extension_count, extensions);
    GLAD_VK_NV_device_generated_commands = glad_vk_has_extension("VK_NV_device_generated_commands", extension_count, extensions);
    GLAD_VK_NV_device_generated_commands_compute = glad_vk_has_extension("VK_NV_device_generated_commands_compute", extension_count, extensions);
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    GLAD_VK_NV_displacement_micromap = glad_vk_has_extension("VK_NV_displacement_micromap", extension_count, extensions);

#endif
    GLAD_VK_NV_display_stereo = glad_vk_has_extension("VK_NV_display_stereo", extension_count, extensions);
    GLAD_VK_NV_extended_sparse_address_space = glad_vk_has_extension("VK_NV_extended_sparse_address_space", extension_count, extensions);
    GLAD_VK_NV_external_compute_queue = glad_vk_has_extension("VK_NV_external_compute_queue", extension_count, extensions);
    GLAD_VK_NV_external_memory = glad_vk_has_extension("VK_NV_external_memory", extension_count, extensions);
    GLAD_VK_NV_external_memory_capabilities = glad_vk_has_extension("VK_NV_external_memory_capabilities", extension_count, extensions);
    GLAD_VK_NV_external_memory_rdma = glad_vk_has_extension("VK_NV_external_memory_rdma", extension_count, extensions);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    GLAD_VK_NV_external_memory_win32 = glad_vk_has_extension("VK_NV_external_memory_win32", extension_count, extensions);

#endif
    GLAD_VK_NV_fill_rectangle = glad_vk_has_extension("VK_NV_fill_rectangle", extension_count, extensions);
    GLAD_VK_NV_fragment_coverage_to_color = glad_vk_has_extension("VK_NV_fragment_coverage_to_color", extension_count, extensions);
    GLAD_VK_NV_fragment_shader_barycentric = glad_vk_has_extension("VK_NV_fragment_shader_barycentric", extension_count, extensions);
    GLAD_VK_NV_fragment_shading_rate_enums = glad_vk_has_extension("VK_NV_fragment_shading_rate_enums", extension_count, extensions);
    GLAD_VK_NV_framebuffer_mixed_samples = glad_vk_has_extension("VK_NV_framebuffer_mixed_samples", extension_count, extensions);
    GLAD_VK_NV_geometry_shader_passthrough = glad_vk_has_extension("VK_NV_geometry_shader_passthrough", extension_count, extensions);
    GLAD_VK_NV_glsl_shader = glad_vk_has_extension("VK_NV_glsl_shader", extension_count, extensions);
    GLAD_VK_NV_inherited_viewport_scissor = glad_vk_has_extension("VK_NV_inherited_viewport_scissor", extension_count, extensions);
    GLAD_VK_NV_linear_color_attachment = glad_vk_has_extension("VK_NV_linear_color_attachment", extension_count, extensions);
    GLAD_VK_NV_low_latency = glad_vk_has_extension("VK_NV_low_latency", extension_count, extensions);
    GLAD_VK_NV_low_latency2 = glad_vk_has_extension("VK_NV_low_latency2", extension_count, extensions);
    GLAD_VK_NV_memory_decompression = glad_vk_has_extension("VK_NV_memory_decompression", extension_count, extensions);
    GLAD_VK_NV_mesh_shader = glad_vk_has_extension("VK_NV_mesh_shader", extension_count, extensions);
    GLAD_VK_NV_optical_flow = glad_vk_has_extension("VK_NV_optical_flow", extension_count, extensions);
    GLAD_VK_NV_partitioned_acceleration_structure = glad_vk_has_extension("VK_NV_partitioned_acceleration_structure", extension_count, extensions);
    GLAD_VK_NV_per_stage_descriptor_set = glad_vk_has_extension("VK_NV_per_stage_descriptor_set", extension_count, extensions);
    GLAD_VK_NV_present_barrier = glad_vk_has_extension("VK_NV_present_barrier", extension_count, extensions);
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    GLAD_VK_NV_present_metering = glad_vk_has_extension("VK_NV_present_metering", extension_count, extensions);

#endif
    GLAD_VK_NV_raw_access_chains = glad_vk_has_extension("VK_NV_raw_access_chains", extension_count, extensions);
    GLAD_VK_NV_ray_tracing = glad_vk_has_extension("VK_NV_ray_tracing", extension_count, extensions);
    GLAD_VK_NV_ray_tracing_invocation_reorder = glad_vk_has_extension("VK_NV_ray_tracing_invocation_reorder", extension_count, extensions);
    GLAD_VK_NV_ray_tracing_linear_swept_spheres = glad_vk_has_extension("VK_NV_ray_tracing_linear_swept_spheres", extension_count, extensions);
    GLAD_VK_NV_ray_tracing_motion_blur = glad_vk_has_extension("VK_NV_ray_tracing_motion_blur", extension_count, extensions);
    GLAD_VK_NV_ray_tracing_validation = glad_vk_has_extension("VK_NV_ray_tracing_validation", extension_count, extensions);
    GLAD_VK_NV_representative_fragment_test = glad_vk_has_extension("VK_NV_representative_fragment_test", extension_count, extensions);
    GLAD_VK_NV_sample_mask_override_coverage = glad_vk_has_extension("VK_NV_sample_mask_override_coverage", extension_count, extensions);
    GLAD_VK_NV_scissor_exclusive = glad_vk_has_extension("VK_NV_scissor_exclusive", extension_count, extensions);
    GLAD_VK_NV_shader_atomic_float16_vector = glad_vk_has_extension("VK_NV_shader_atomic_float16_vector", extension_count, extensions);
    GLAD_VK_NV_shader_image_footprint = glad_vk_has_extension("VK_NV_shader_image_footprint", extension_count, extensions);
    GLAD_VK_NV_shader_sm_builtins = glad_vk_has_extension("VK_NV_shader_sm_builtins", extension_count, extensions);
    GLAD_VK_NV_shader_subgroup_partitioned = glad_vk_has_extension("VK_NV_shader_subgroup_partitioned", extension_count, extensions);
    GLAD_VK_NV_shading_rate_image = glad_vk_has_extension("VK_NV_shading_rate_image", extension_count, extensions);
    GLAD_VK_NV_viewport_array2 = glad_vk_has_extension("VK_NV_viewport_array2", extension_count, extensions);
    GLAD_VK_NV_viewport_swizzle = glad_vk_has_extension("VK_NV_viewport_swizzle", extension_count, extensions);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    GLAD_VK_NV_win32_keyed_mutex = glad_vk_has_extension("VK_NV_win32_keyed_mutex", extension_count, extensions);

#endif
    GLAD_VK_QCOM_filter_cubic_clamp = glad_vk_has_extension("VK_QCOM_filter_cubic_clamp", extension_count, extensions);
    GLAD_VK_QCOM_filter_cubic_weights = glad_vk_has_extension("VK_QCOM_filter_cubic_weights", extension_count, extensions);
    GLAD_VK_QCOM_fragment_density_map_offset = glad_vk_has_extension("VK_QCOM_fragment_density_map_offset", extension_count, extensions);
    GLAD_VK_QCOM_image_processing = glad_vk_has_extension("VK_QCOM_image_processing", extension_count, extensions);
    GLAD_VK_QCOM_image_processing2 = glad_vk_has_extension("VK_QCOM_image_processing2", extension_count, extensions);
    GLAD_VK_QCOM_multiview_per_view_render_areas = glad_vk_has_extension("VK_QCOM_multiview_per_view_render_areas", extension_count, extensions);
    GLAD_VK_QCOM_multiview_per_view_viewports = glad_vk_has_extension("VK_QCOM_multiview_per_view_viewports", extension_count, extensions);
    GLAD_VK_QCOM_render_pass_shader_resolve = glad_vk_has_extension("VK_QCOM_render_pass_shader_resolve", extension_count, extensions);
    GLAD_VK_QCOM_render_pass_store_ops = glad_vk_has_extension("VK_QCOM_render_pass_store_ops", extension_count, extensions);
    GLAD_VK_QCOM_render_pass_transform = glad_vk_has_extension("VK_QCOM_render_pass_transform", extension_count, extensions);
    GLAD_VK_QCOM_rotated_copy_commands = glad_vk_has_extension("VK_QCOM_rotated_copy_commands", extension_count, extensions);
    GLAD_VK_QCOM_tile_memory_heap = glad_vk_has_extension("VK_QCOM_tile_memory_heap", extension_count, extensions);
    GLAD_VK_QCOM_tile_properties = glad_vk_has_extension("VK_QCOM_tile_properties", extension_count, extensions);
    GLAD_VK_QCOM_tile_shading = glad_vk_has_extension("VK_QCOM_tile_shading", extension_count, extensions);
    GLAD_VK_QCOM_ycbcr_degamma = glad_vk_has_extension("VK_QCOM_ycbcr_degamma", extension_count, extensions);
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    GLAD_VK_QNX_external_memory_screen_buffer = glad_vk_has_extension("VK_QNX_external_memory_screen_buffer", extension_count, extensions);

#endif
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    GLAD_VK_QNX_screen_surface = glad_vk_has_extension("VK_QNX_screen_surface", extension_count, extensions);

#endif
    GLAD_VK_SEC_amigo_profiling = glad_vk_has_extension("VK_SEC_amigo_profiling", extension_count, extensions);
    GLAD_VK_VALVE_descriptor_set_host_mapping = glad_vk_has_extension("VK_VALVE_descriptor_set_host_mapping", extension_count, extensions);
    GLAD_VK_VALVE_mutable_descriptor_type = glad_vk_has_extension("VK_VALVE_mutable_descriptor_type", extension_count, extensions);

    GLAD_UNUSED(&glad_vk_has_extension);

    glad_vk_free_extensions(extension_count, extensions);

    return 1;
}

static int glad_vk_find_core_vulkan( VkPhysicalDevice physical_device) {
    int major = 1;
    int minor = 0;

#ifdef VK_VERSION_1_1
    if (glad_vkEnumerateInstanceVersion != NULL) {
        uint32_t version;
        VkResult result;

        result = glad_vkEnumerateInstanceVersion(&version);
        if (result == VK_SUCCESS) {
            major = (int) VK_VERSION_MAJOR(version);
            minor = (int) VK_VERSION_MINOR(version);
        }
    }
#endif

    if (physical_device != NULL && glad_vkGetPhysicalDeviceProperties != NULL) {
        VkPhysicalDeviceProperties properties;
        glad_vkGetPhysicalDeviceProperties(physical_device, &properties);

        major = (int) VK_VERSION_MAJOR(properties.apiVersion);
        minor = (int) VK_VERSION_MINOR(properties.apiVersion);
    }

    GLAD_VK_VERSION_1_0 = (major == 1 && minor >= 0) || major > 1;
    GLAD_VK_VERSION_1_1 = (major == 1 && minor >= 1) || major > 1;
    GLAD_VK_VERSION_1_2 = (major == 1 && minor >= 2) || major > 1;
    GLAD_VK_VERSION_1_3 = (major == 1 && minor >= 3) || major > 1;
    GLAD_VK_VERSION_1_4 = (major == 1 && minor >= 4) || major > 1;

    return GLAD_MAKE_VERSION(major, minor);
}

int gladLoadVulkanUserPtr( VkPhysicalDevice physical_device, GLADuserptrloadfunc load, void *userptr) {
    int version;

#ifdef VK_VERSION_1_1
    glad_vkEnumerateInstanceVersion  = (PFN_vkEnumerateInstanceVersion) load(userptr, "vkEnumerateInstanceVersion");
#endif
    version = glad_vk_find_core_vulkan( physical_device);
    if (!version) {
        return 0;
    }

    glad_vk_load_VK_VERSION_1_0(load, userptr);
    glad_vk_load_VK_VERSION_1_1(load, userptr);
    glad_vk_load_VK_VERSION_1_2(load, userptr);
    glad_vk_load_VK_VERSION_1_3(load, userptr);
    glad_vk_load_VK_VERSION_1_4(load, userptr);

    if (!glad_vk_find_extensions_vulkan( physical_device)) return 0;
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    glad_vk_load_VK_AMDX_shader_enqueue(load, userptr);

#endif
    glad_vk_load_VK_AMD_anti_lag(load, userptr);
    glad_vk_load_VK_AMD_buffer_marker(load, userptr);
    glad_vk_load_VK_AMD_display_native_hdr(load, userptr);
    glad_vk_load_VK_AMD_draw_indirect_count(load, userptr);
    glad_vk_load_VK_AMD_shader_info(load, userptr);
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    glad_vk_load_VK_ANDROID_external_memory_android_hardware_buffer(load, userptr);

#endif
    glad_vk_load_VK_EXT_acquire_drm_display(load, userptr);
#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
    glad_vk_load_VK_EXT_acquire_xlib_display(load, userptr);

#endif
    glad_vk_load_VK_EXT_attachment_feedback_loop_dynamic_state(load, userptr);
    glad_vk_load_VK_EXT_buffer_device_address(load, userptr);
    glad_vk_load_VK_EXT_calibrated_timestamps(load, userptr);
    glad_vk_load_VK_EXT_color_write_enable(load, userptr);
    glad_vk_load_VK_EXT_conditional_rendering(load, userptr);
    glad_vk_load_VK_EXT_debug_marker(load, userptr);
    glad_vk_load_VK_EXT_debug_report(load, userptr);
    glad_vk_load_VK_EXT_debug_utils(load, userptr);
    glad_vk_load_VK_EXT_depth_bias_control(load, userptr);
    glad_vk_load_VK_EXT_depth_clamp_control(load, userptr);
    glad_vk_load_VK_EXT_descriptor_buffer(load, userptr);
    glad_vk_load_VK_EXT_device_fault(load, userptr);
    glad_vk_load_VK_EXT_device_generated_commands(load, userptr);
    glad_vk_load_VK_EXT_direct_mode_display(load, userptr);
#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    glad_vk_load_VK_EXT_directfb_surface(load, userptr);

#endif
    glad_vk_load_VK_EXT_discard_rectangles(load, userptr);
    glad_vk_load_VK_EXT_display_control(load, userptr);
    glad_vk_load_VK_EXT_display_surface_counter(load, userptr);
    glad_vk_load_VK_EXT_extended_dynamic_state(load, userptr);
    glad_vk_load_VK_EXT_extended_dynamic_state2(load, userptr);
    glad_vk_load_VK_EXT_extended_dynamic_state3(load, userptr);
    glad_vk_load_VK_EXT_external_memory_host(load, userptr);
#if defined(VK_USE_PLATFORM_METAL_EXT)
    glad_vk_load_VK_EXT_external_memory_metal(load, userptr);

#endif
    glad_vk_load_VK_EXT_fragment_density_map_offset(load, userptr);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    glad_vk_load_VK_EXT_full_screen_exclusive(load, userptr);

#endif
    glad_vk_load_VK_EXT_hdr_metadata(load, userptr);
    glad_vk_load_VK_EXT_headless_surface(load, userptr);
    glad_vk_load_VK_EXT_host_image_copy(load, userptr);
    glad_vk_load_VK_EXT_host_query_reset(load, userptr);
    glad_vk_load_VK_EXT_image_compression_control(load, userptr);
    glad_vk_load_VK_EXT_image_drm_format_modifier(load, userptr);
    glad_vk_load_VK_EXT_line_rasterization(load, userptr);
    glad_vk_load_VK_EXT_mesh_shader(load, userptr);
#if defined(VK_USE_PLATFORM_METAL_EXT)
    glad_vk_load_VK_EXT_metal_objects(load, userptr);

#endif
#if defined(VK_USE_PLATFORM_METAL_EXT)
    glad_vk_load_VK_EXT_metal_surface(load, userptr);

#endif
    glad_vk_load_VK_EXT_multi_draw(load, userptr);
    glad_vk_load_VK_EXT_opacity_micromap(load, userptr);
    glad_vk_load_VK_EXT_pageable_device_local_memory(load, userptr);
    glad_vk_load_VK_EXT_pipeline_properties(load, userptr);
    glad_vk_load_VK_EXT_private_data(load, userptr);
    glad_vk_load_VK_EXT_sample_locations(load, userptr);
    glad_vk_load_VK_EXT_shader_module_identifier(load, userptr);
    glad_vk_load_VK_EXT_shader_object(load, userptr);
    glad_vk_load_VK_EXT_swapchain_maintenance1(load, userptr);
    glad_vk_load_VK_EXT_tooling_info(load, userptr);
    glad_vk_load_VK_EXT_transform_feedback(load, userptr);
    glad_vk_load_VK_EXT_validation_cache(load, userptr);
    glad_vk_load_VK_EXT_vertex_input_dynamic_state(load, userptr);
#if defined(VK_USE_PLATFORM_FUCHSIA)
    glad_vk_load_VK_FUCHSIA_buffer_collection(load, userptr);

#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
    glad_vk_load_VK_FUCHSIA_external_memory(load, userptr);

#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
    glad_vk_load_VK_FUCHSIA_external_semaphore(load, userptr);

#endif
#if defined(VK_USE_PLATFORM_FUCHSIA)
    glad_vk_load_VK_FUCHSIA_imagepipe_surface(load, userptr);

#endif
#if defined(VK_USE_PLATFORM_GGP)
    glad_vk_load_VK_GGP_stream_descriptor_surface(load, userptr);

#endif
    glad_vk_load_VK_GOOGLE_display_timing(load, userptr);
    glad_vk_load_VK_HUAWEI_cluster_culling_shader(load, userptr);
    glad_vk_load_VK_HUAWEI_invocation_mask(load, userptr);
    glad_vk_load_VK_HUAWEI_subpass_shading(load, userptr);
    glad_vk_load_VK_INTEL_performance_query(load, userptr);
    glad_vk_load_VK_KHR_acceleration_structure(load, userptr);
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    glad_vk_load_VK_KHR_android_surface(load, userptr);

#endif
    glad_vk_load_VK_KHR_bind_memory2(load, userptr);
    glad_vk_load_VK_KHR_buffer_device_address(load, userptr);
    glad_vk_load_VK_KHR_calibrated_timestamps(load, userptr);
    glad_vk_load_VK_KHR_cooperative_matrix(load, userptr);
    glad_vk_load_VK_KHR_copy_commands2(load, userptr);
    glad_vk_load_VK_KHR_create_renderpass2(load, userptr);
    glad_vk_load_VK_KHR_deferred_host_operations(load, userptr);
    glad_vk_load_VK_KHR_descriptor_update_template(load, userptr);
    glad_vk_load_VK_KHR_device_group(load, userptr);
    glad_vk_load_VK_KHR_device_group_creation(load, userptr);
    glad_vk_load_VK_KHR_display(load, userptr);
    glad_vk_load_VK_KHR_display_swapchain(load, userptr);
    glad_vk_load_VK_KHR_draw_indirect_count(load, userptr);
    glad_vk_load_VK_KHR_dynamic_rendering(load, userptr);
    glad_vk_load_VK_KHR_dynamic_rendering_local_read(load, userptr);
    glad_vk_load_VK_KHR_external_fence_capabilities(load, userptr);
    glad_vk_load_VK_KHR_external_fence_fd(load, userptr);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    glad_vk_load_VK_KHR_external_fence_win32(load, userptr);

#endif
    glad_vk_load_VK_KHR_external_memory_capabilities(load, userptr);
    glad_vk_load_VK_KHR_external_memory_fd(load, userptr);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    glad_vk_load_VK_KHR_external_memory_win32(load, userptr);

#endif
    glad_vk_load_VK_KHR_external_semaphore_capabilities(load, userptr);
    glad_vk_load_VK_KHR_external_semaphore_fd(load, userptr);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    glad_vk_load_VK_KHR_external_semaphore_win32(load, userptr);

#endif
    glad_vk_load_VK_KHR_fragment_shading_rate(load, userptr);
    glad_vk_load_VK_KHR_get_display_properties2(load, userptr);
    glad_vk_load_VK_KHR_get_memory_requirements2(load, userptr);
    glad_vk_load_VK_KHR_get_physical_device_properties2(load, userptr);
    glad_vk_load_VK_KHR_get_surface_capabilities2(load, userptr);
    glad_vk_load_VK_KHR_line_rasterization(load, userptr);
    glad_vk_load_VK_KHR_maintenance1(load, userptr);
    glad_vk_load_VK_KHR_maintenance3(load, userptr);
    glad_vk_load_VK_KHR_maintenance4(load, userptr);
    glad_vk_load_VK_KHR_maintenance5(load, userptr);
    glad_vk_load_VK_KHR_maintenance6(load, userptr);
    glad_vk_load_VK_KHR_map_memory2(load, userptr);
    glad_vk_load_VK_KHR_performance_query(load, userptr);
    glad_vk_load_VK_KHR_pipeline_binary(load, userptr);
    glad_vk_load_VK_KHR_pipeline_executable_properties(load, userptr);
    glad_vk_load_VK_KHR_present_wait(load, userptr);
    glad_vk_load_VK_KHR_push_descriptor(load, userptr);
    glad_vk_load_VK_KHR_ray_tracing_maintenance1(load, userptr);
    glad_vk_load_VK_KHR_ray_tracing_pipeline(load, userptr);
    glad_vk_load_VK_KHR_sampler_ycbcr_conversion(load, userptr);
    glad_vk_load_VK_KHR_shared_presentable_image(load, userptr);
    glad_vk_load_VK_KHR_surface(load, userptr);
    glad_vk_load_VK_KHR_swapchain(load, userptr);
    glad_vk_load_VK_KHR_synchronization2(load, userptr);
    glad_vk_load_VK_KHR_timeline_semaphore(load, userptr);
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    glad_vk_load_VK_KHR_wayland_surface(load, userptr);

#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    glad_vk_load_VK_KHR_win32_surface(load, userptr);

#endif
#if defined(VK_USE_PLATFORM_XCB_KHR)
    glad_vk_load_VK_KHR_xcb_surface(load, userptr);

#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
    glad_vk_load_VK_KHR_xlib_surface(load, userptr);

#endif
#if defined(VK_USE_PLATFORM_IOS_MVK)
    glad_vk_load_VK_MVK_ios_surface(load, userptr);

#endif
#if defined(VK_USE_PLATFORM_MACOS_MVK)
    glad_vk_load_VK_MVK_macos_surface(load, userptr);

#endif
#if defined(VK_USE_PLATFORM_VI_NN)
    glad_vk_load_VK_NN_vi_surface(load, userptr);

#endif
    glad_vk_load_VK_NVX_binary_import(load, userptr);
    glad_vk_load_VK_NVX_image_view_handle(load, userptr);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    glad_vk_load_VK_NV_acquire_winrt_display(load, userptr);

#endif
    glad_vk_load_VK_NV_clip_space_w_scaling(load, userptr);
    glad_vk_load_VK_NV_cluster_acceleration_structure(load, userptr);
    glad_vk_load_VK_NV_cooperative_matrix(load, userptr);
    glad_vk_load_VK_NV_cooperative_matrix2(load, userptr);
    glad_vk_load_VK_NV_cooperative_vector(load, userptr);
    glad_vk_load_VK_NV_copy_memory_indirect(load, userptr);
    glad_vk_load_VK_NV_coverage_reduction_mode(load, userptr);
#if defined(VK_ENABLE_BETA_EXTENSIONS)
    glad_vk_load_VK_NV_cuda_kernel_launch(load, userptr);

#endif
    glad_vk_load_VK_NV_device_diagnostic_checkpoints(load, userptr);
    glad_vk_load_VK_NV_device_generated_commands(load, userptr);
    glad_vk_load_VK_NV_device_generated_commands_compute(load, userptr);
    glad_vk_load_VK_NV_external_compute_queue(load, userptr);
    glad_vk_load_VK_NV_external_memory_capabilities(load, userptr);
    glad_vk_load_VK_NV_external_memory_rdma(load, userptr);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    glad_vk_load_VK_NV_external_memory_win32(load, userptr);

#endif
    glad_vk_load_VK_NV_fragment_shading_rate_enums(load, userptr);
    glad_vk_load_VK_NV_low_latency2(load, userptr);
    glad_vk_load_VK_NV_memory_decompression(load, userptr);
    glad_vk_load_VK_NV_mesh_shader(load, userptr);
    glad_vk_load_VK_NV_optical_flow(load, userptr);
    glad_vk_load_VK_NV_partitioned_acceleration_structure(load, userptr);
    glad_vk_load_VK_NV_ray_tracing(load, userptr);
    glad_vk_load_VK_NV_scissor_exclusive(load, userptr);
    glad_vk_load_VK_NV_shading_rate_image(load, userptr);
    glad_vk_load_VK_QCOM_tile_memory_heap(load, userptr);
    glad_vk_load_VK_QCOM_tile_properties(load, userptr);
    glad_vk_load_VK_QCOM_tile_shading(load, userptr);
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    glad_vk_load_VK_QNX_external_memory_screen_buffer(load, userptr);

#endif
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    glad_vk_load_VK_QNX_screen_surface(load, userptr);

#endif
    glad_vk_load_VK_VALVE_descriptor_set_host_mapping(load, userptr);


    return version;
}


int gladLoadVulkan( VkPhysicalDevice physical_device, GLADloadfunc load) {
    return gladLoadVulkanUserPtr( physical_device, glad_vk_get_proc_from_userptr, GLAD_GNUC_EXTENSION (void*) load);
}



 


#ifdef __cplusplus
}
#endif
