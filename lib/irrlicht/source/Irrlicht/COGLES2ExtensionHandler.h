// Copyright (C) 2009-2010 Amundis
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// and OpenGL ES driver implemented by Christian Stehno
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_OGLES2_EXTENSION_HANDLER_H_INCLUDED__
#define __C_OGLES2_EXTENSION_HANDLER_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_OGLES2_
#if defined(_IRR_COMPILE_WITH_IPHONE_DEVICE_)
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#else
#include <GLES2/gl2.h>
// seems to be missing...
typedef char GLchar;
#if defined(_IRR_OGLES2_USE_EXTPOINTER_)
#include "gles2-ext.h"
#endif
#endif
#include "os.h"
#include "EDriverFeatures.h"

namespace irr
{
namespace video
{
	class COGLES2Driver;
	class COGLES2ExtensionHandler
	{
	public:
		enum EOGLES2Features
		{
			IRR_AMD_compressed_3DC_texture=0, // 39
			IRR_AMD_compressed_ATC_texture, // 40
			IRR_AMD_performance_monitor, // 50
			IRR_AMD_program_binary_Z400, // 48
			IRR_ANGLE_framebuffer_blit, // 84
			IRR_ANGLE_framebuffer_multisample, // 84
			IRR_ANGLE_instanced_arrays, // 109
			IRR_ANGLE_pack_reverse_row_order, // 110
			IRR_ANGLE_texture_compression_dxt3, // 111
			IRR_ANGLE_texture_compression_dxt5, // 111
			IRR_ANGLE_texture_usage, // 112
			IRR_ANGLE_translated_shader_source, // 113
			IRR_APPLE_copy_texture_levels, // 123
			IRR_APPLE_framebuffer_multisample, // 78
			IRR_APPLE_rgb_422, // 76
			IRR_APPLE_sync, // 124
			IRR_APPLE_texture_2D_limited_npot, // 59
			IRR_APPLE_texture_format_BGRA8888, // 79
			IRR_APPLE_texture_max_level, // 80
			IRR_ARB_texture_env_combine, //ogl, IMG simulator
			IRR_ARB_texture_env_dot3, //ogl, IMG simulator
			IRR_ARM_mali_program_binary, // 120
			IRR_ARM_mali_shader_binary, // 81
			IRR_ARM_rgba8, // 82
			IRR_DMP_shader_binary, // 88
			IRR_EXT_blend_minmax, // 65
			IRR_EXT_color_buffer_half_float, // 97
			IRR_EXT_debug_label, // 98
			IRR_EXT_debug_marker, // 99
			IRR_EXT_discard_framebuffer, // 64
			IRR_EXT_frag_depth, // 86
			IRR_EXT_map_buffer_range, // 121
			IRR_EXT_multisampled_render_to_texture, // 106
			IRR_EXT_multiview_draw_buffers, // 125
			IRR_EXT_multi_draw_arrays, // 69
			IRR_EXT_occlusion_query_boolean, // 100
			IRR_EXT_read_format_bgra, // 66
			IRR_EXT_robustness, // 107
			IRR_EXT_separate_shader_objects, // 101
			IRR_EXT_shader_framebuffer_fetch, // 122
			IRR_EXT_shader_texture_lod, // 77
			IRR_EXT_shadow_samplers, // 102
			IRR_EXT_sRGB, // 105
			IRR_EXT_texture_compression_dxt1, // 49
			IRR_EXT_texture_filter_anisotropic, // 41
			IRR_EXT_texture_format_BGRA8888, // 51
			IRR_EXT_texture_lod_bias, // 60
			IRR_EXT_texture_rg, // 103
			IRR_EXT_texture_storage, // 108
			IRR_EXT_texture_type_2_10_10_10_REV, // 42
			IRR_EXT_unpack_subimage, // 90
			IRR_FJ_shader_binary_GCCSO, // 114
			IRR_IMG_multisampled_render_to_texture, // 74
			IRR_IMG_program_binary, // 67
			IRR_IMG_read_format, // 53
			IRR_IMG_shader_binary, // 68
			IRR_IMG_texture_compression_pvrtc, // 54
			IRR_IMG_texture_env_enhanced_fixed_function, // 58
			IRR_IMG_texture_format_BGRA8888, // replaced by EXT version
			IRR_IMG_user_clip_plane, // 57, was clip_planes
			IRR_IMG_vertex_program, // non-standard
			IRR_KHR_debug, // 118
			IRR_KHR_texture_compression_astc_ldr, // 117
			IRR_NV_coverage_sample, // 72
			IRR_NV_depth_nonlinear, // 73
			IRR_NV_draw_buffers, // 91
			IRR_NV_EGL_stream_consumer_external, // 104
			IRR_NV_fbo_color_attachments, // 92
			IRR_NV_fence, // 52
			IRR_NV_read_buffer, // 93
			IRR_NV_read_buffer_front, // part of 93
			IRR_NV_read_depth, // part of 94
			IRR_NV_read_depth_stencil, // 94
			IRR_NV_read_stencil, // part of 94
			IRR_NV_texture_compression_s3tc_update, // 95
			IRR_NV_texture_npot_2D_mipmap, // 96
			IRR_OES_blend_equation_separate, // 1
			IRR_OES_blend_func_separate, // 2
			IRR_OES_blend_subtract, // 3
			IRR_OES_byte_coordinates, // 4
			IRR_OES_compressed_ETC1_RGB8_texture, // 5
			IRR_OES_compressed_paletted_texture, // 6
			IRR_OES_depth24, // 24
			IRR_OES_depth32, // 25
			IRR_OES_depth_texture, // 43
			IRR_OES_draw_texture, // 7
			IRR_OES_EGL_image, // 23
			IRR_OES_EGL_image_external, // 87
			IRR_OES_EGL_sync, // 75
			IRR_OES_element_index_uint, // 26
			IRR_OES_extended_matrix_palette, // 8
			IRR_OES_fbo_render_mipmap, // 27
			IRR_OES_fixed_point, // 9
			IRR_OES_fragment_precision_high, // 28
			IRR_OES_framebuffer_object, // 10
			IRR_OES_get_program_binary, // 47
			IRR_OES_mapbuffer, // 29
			IRR_OES_matrix_get, // 11
			IRR_OES_matrix_palette, // 12
			IRR_OES_packed_depth_stencil, // 44
			IRR_OES_point_size_array, // 14
			IRR_OES_point_sprite, // 15
			IRR_OES_query_matrix, // 16
			IRR_OES_read_format, // 17
			IRR_OES_required_internalformat, // 115
			IRR_OES_rgb8_rgba8, // 30
			IRR_OES_single_precision, // 18
			IRR_OES_standard_derivatives, // 45
			IRR_OES_stencil1, // 31
			IRR_OES_stencil4, // 32
			IRR_OES_stencil8, // 33
			IRR_OES_stencil_wrap, // 19
			IRR_OES_surfaceless_context, // 116
			IRR_OES_texture_3D, // 34
			IRR_OES_texture_cube_map, // 20
			IRR_OES_texture_env_crossbar, // 21
			IRR_OES_texture_float, // 36
			IRR_OES_texture_float_linear, // 35
			IRR_OES_texture_half_float, // 36
			IRR_OES_texture_half_float_linear, // 35
			IRR_OES_texture_mirrored_repeat, // 22
			IRR_OES_texture_npot, // 37
			IRR_OES_vertex_array_object, // 71
			IRR_OES_vertex_half_float, // 38
			IRR_OES_vertex_type_10_10_10_2, // 46
			IRR_QCOM_alpha_test, // 89
			IRR_QCOM_binning_control, // 119
			IRR_QCOM_driver_control, // 55
			IRR_QCOM_extended_get, // 62
			IRR_QCOM_extended_get2, // 63
			IRR_QCOM_performance_monitor_global_mode, // 56
			IRR_QCOM_tiled_rendering, // 70
			IRR_QCOM_writeonly_rendering, // 61
			IRR_SUN_multi_draw_arrays, // 69
			IRR_VIV_shader_binary, // 85

			IRR_OGLES2_Feature_Count
		};

		//! queries the features of the driver, returns true if feature is available
		bool queryOpenGLFeature(EOGLES2Features feature) const
		{
			return FeatureAvailable[feature];
		}


	protected:
		COGLES2ExtensionHandler();

		bool queryFeature(video::E_VIDEO_DRIVER_FEATURE feature) const
		{
			switch (feature)
			{
			case EVDF_RENDER_TO_TARGET:
			case EVDF_HARDWARE_TL:
			case EVDF_MULTITEXTURE:
			case EVDF_BILINEAR_FILTER:
			case EVDF_MIP_MAP:
			case EVDF_MIP_MAP_AUTO_UPDATE:
			case EVDF_VERTEX_SHADER_1_1:
			case EVDF_PIXEL_SHADER_1_1:
			case EVDF_PIXEL_SHADER_1_2:
			case EVDF_PIXEL_SHADER_2_0:
			case EVDF_VERTEX_SHADER_2_0:
			case EVDF_ARB_GLSL:
			case EVDF_TEXTURE_NSQUARE:
			case EVDF_TEXTURE_NPOT:
			case EVDF_FRAMEBUFFER_OBJECT:
			case EVDF_VERTEX_BUFFER_OBJECT:
			case EVDF_COLOR_MASK:
			case EVDF_ALPHA_TO_COVERAGE:
			case EVDF_POLYGON_OFFSET:
			case EVDF_TEXTURE_MATRIX:
				return true;
			case EVDF_ARB_VERTEX_PROGRAM_1:
			case EVDF_ARB_FRAGMENT_PROGRAM_1:
			case EVDF_GEOMETRY_SHADER:
			case EVDF_MULTIPLE_RENDER_TARGETS:
			case EVDF_MRT_BLEND:
			case EVDF_MRT_COLOR_MASK:
			case EVDF_MRT_BLEND_FUNC:
			case EVDF_OCCLUSION_QUERY:
				return false;
			case EVDF_BLEND_OPERATIONS:
				return false;
			case EVDF_TEXTURE_COMPRESSED_DXT:
				return false; // NV Tegra need improvements here
			case EVDF_STENCIL_BUFFER:
				return StencilBuffer;
			default:
				return false;
			};
		}

		void dump() const;

        void initExtensions(COGLES2Driver* driver,
                bool withStencil);

	protected:
		u16 Version;
		u8 MaxTextureUnits;
		u8 MaxSupportedTextures;
		u8 MaxAnisotropy;
		u32 MaxIndices;
		u32 MaxTextureSize;
		f32 MaxTextureLODBias;
		//! Minimal and maximal supported thickness for lines without smoothing
		GLfloat DimAliasedLine[2];
		//! Minimal and maximal supported thickness for points without smoothing
		GLfloat DimAliasedPoint[2];
		bool StencilBuffer;
		bool FeatureAvailable[IRR_OGLES2_Feature_Count];
	};

} // end namespace video
} // end namespace irr


#endif // _IRR_COMPILE_WITH_OGLES2_
#endif

