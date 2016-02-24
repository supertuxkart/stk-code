// Copyright (C) 2008 Christian Stehno
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_OGLES_EXTENSION_HANDLER_H_INCLUDED__
#define __C_OGLES_EXTENSION_HANDLER_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_OGLES1_
#if defined(_IRR_COMPILE_WITH_IPHONE_DEVICE_)
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#elif defined(_IRR_ANDROID_PLATFORM_)
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <EGL/egl.h>
#else
#include <GLES/egl.h>
#include <GLES/gl.h>
// seems to be missing...
typedef char GLchar;
#if defined(_IRR_OGLES1_USE_EXTPOINTER_)
#include "gles-ext.h"
#endif
#endif
#include "os.h"
#include "EDriverFeatures.h"

namespace irr
{
namespace video
{
	class COGLES1Driver;
	class COGLES1ExtensionHandler
	{
	public:
		enum EOGLESFeatures
		{
			IRR_AMD_compressed_3DC_texture = 0, //39
			IRR_AMD_compressed_ATC_texture, //40
			IRR_AMD_performance_monitor, //50
			IRR_AMD_program_binary_Z400, //48
			IRR_ANGLE_framebuffer_blit, // 84
			IRR_ANGLE_framebuffer_multisample, // 85
			IRR_APPLE_copy_texture_levels, // 123
			IRR_APPLE_framebuffer_multisample, // 79
			IRR_APPLE_rgb_422, // 77
			IRR_APPLE_sync, // 124
			IRR_APPLE_texture_2D_limited_npot, // 59
			IRR_APPLE_texture_format_BGRA8888, // 80
			IRR_APPLE_texture_max_level, // 81
			IRR_ARB_texture_env_combine, //ogl, IMG simulator
			IRR_ARB_texture_env_dot3, //ogl, IMG simulator
			IRR_ARM_mali_shader_binary, // 82
			IRR_ARM_rgba8, // 83
			IRR_DMP_shader_binary, // 89
			IRR_EXT_blend_minmax, // 65
			IRR_EXT_discard_framebuffer, // 64
			IRR_EXT_frag_depth, // 87
			IRR_EXT_map_buffer_range, // 121
			IRR_EXT_multisampled_render_to_texture, // 106
			IRR_EXT_multi_draw_arrays, // 69
			IRR_EXT_robustness, // 107
			IRR_EXT_read_format_bgra, // 66
			IRR_EXT_shader_texture_lod, // 78
			IRR_EXT_sRGB, // 105
			IRR_EXT_texture_compression_dxt1, //49
			IRR_EXT_texture_filter_anisotropic, //41
			IRR_EXT_texture_format_BGRA8888, //51
			IRR_EXT_texture_lod_bias, // 60
			IRR_EXT_texture_storage, // 108
			IRR_EXT_texture_type_2_10_10_10_REV, //42
			IRR_IMG_multisampled_render_to_texture, // 75
			IRR_IMG_program_binary, // 67
			IRR_IMG_read_format, //53
			IRR_IMG_shader_binary, // 68
			IRR_IMG_texture_compression_pvrtc, //54
			IRR_IMG_texture_env_enhanced_fixed_function, // 58
			IRR_IMG_texture_format_BGRA8888, // replaced by EXT version
			IRR_IMG_user_clip_plane, // 57, was clip_planes
			IRR_IMG_vertex_program, // non-standard
			IRR_NV_coverage_sample, // 73
			IRR_NV_depth_nonlinear, // 74
			IRR_NV_fence, //52
			IRR_OES_blend_equation_separate, //1
			IRR_OES_blend_func_separate, //2
			IRR_OES_blend_subtract, //3
			IRR_OES_byte_coordinates, //4
			IRR_OES_compressed_ETC1_RGB8_texture, //5
			IRR_OES_compressed_paletted_texture, //6
			IRR_OES_depth24, //24
			IRR_OES_depth32, //25
			IRR_OES_depth_texture, //43
			IRR_OES_draw_texture, //7
			IRR_OES_EGL_image, //23
			IRR_OES_EGL_image_external, // 88
			IRR_OES_EGL_sync, // 76
			IRR_OES_element_index_uint, //26
			IRR_OES_extended_matrix_palette, //8
			IRR_OES_fbo_render_mipmap, //27
			IRR_OES_fixed_point, //9
			IRR_OES_fragment_precision_high, //28
			IRR_OES_framebuffer_object, //10
			IRR_OES_get_program_binary, //47
			IRR_OES_mapbuffer, //29
			IRR_OES_matrix_get, //11
			IRR_OES_matrix_palette, //12
			IRR_OES_packed_depth_stencil, //44
			IRR_OES_point_size_array, //14
			IRR_OES_point_sprite, //15
			IRR_OES_query_matrix, //16
			IRR_OES_read_format, //17
			IRR_OES_required_internalformat, // 115
			IRR_OES_rgb8_rgba8, //30
			IRR_OES_single_precision, //18
			IRR_OES_standard_derivatives, //45
			IRR_OES_stencil1, //31
			IRR_OES_stencil4, //32
			IRR_OES_stencil8, //33
			IRR_OES_stencil_wrap, //19
			IRR_OES_texture_3D, //34
			IRR_OES_texture_cube_map, //20
			IRR_OES_texture_env_crossbar, //21
			IRR_OES_texture_float, //36
			IRR_OES_texture_float_linear, //35
			IRR_OES_texture_half_float, //36
			IRR_OES_texture_half_float_linear, //35
			IRR_OES_texture_mirrored_repeat, //22
			IRR_OES_texture_npot, //37
			IRR_OES_vertex_array_object, // 72
			IRR_OES_vertex_half_float, //38
			IRR_OES_vertex_type_10_10_10_2, //46
			IRR_QCOM_driver_control, //55
			IRR_QCOM_extended_get, // 62
			IRR_QCOM_extended_get2, // 63
			IRR_QCOM_performance_monitor_global_mode, //56
			IRR_QCOM_tiled_rendering, // 71
			IRR_QCOM_writeonly_rendering, // 61
			IRR_SUN_multi_draw_arrays, // 70
			IRR_VIV_shader_binary, // 86

			IRR_OGLES_Feature_Count
		};

		//! queries the features of the driver, returns true if feature is available
		bool queryOpenGLFeature(EOGLESFeatures feature) const
		{
			return FeatureAvailable[feature];
		}

		u16 EGLVersion;
		u16 Version;
		u8 MaxTextureUnits;
		u8 MaxSupportedTextures;
		u8 MaxLights;
		u8 MaxAnisotropy;
		u8 MaxUserClipPlanes;
		u8 MaxAuxBuffers;
		u8 MaxMultipleRenderTargets;
		u32 MaxIndices;
		u32 MaxTextureSize;
		f32 MaxTextureLODBias;
		//! Minimal and maximal supported thickness for lines without smoothing
		GLfloat DimAliasedLine[2];
		//! Minimal and maximal supported thickness for points without smoothing
		GLfloat DimAliasedPoint[2];
		//! Minimal and maximal supported thickness for lines with smoothing
		GLfloat DimSmoothedLine[2];
		//! Minimal and maximal supported thickness for points with smoothing
		GLfloat DimSmoothedPoint[2];
		bool CommonProfile;
		bool MultiTextureExtension;
		bool MultiSamplingExtension;
		bool StencilBuffer;

	protected:
		bool FeatureAvailable[IRR_OGLES_Feature_Count];

		COGLES1ExtensionHandler();

		bool queryFeature(video::E_VIDEO_DRIVER_FEATURE feature) const
		{
			switch (feature)
			{
				case EVDF_RENDER_TO_TARGET:
				case EVDF_HARDWARE_TL:
					return true;
				case EVDF_MULTITEXTURE:
					return MultiTextureExtension;
				case EVDF_BILINEAR_FILTER:
				case EVDF_MIP_MAP:
					return true;
				case EVDF_MIP_MAP_AUTO_UPDATE:
					return Version>100; // Supported in version 1.1
				case EVDF_STENCIL_BUFFER:
					return StencilBuffer;
				case EVDF_TEXTURE_NSQUARE:
					return true; // non-square is always supported
				case EVDF_TEXTURE_NPOT:
					return FeatureAvailable[IRR_APPLE_texture_2D_limited_npot];
				default:
					return false;
			}
		}

		void dump() const;

		void initExtensions(COGLES1Driver* driver,
#ifdef EGL_VERSION_1_0
				EGLDisplay display,
#endif
				bool withStencil);

	public:
		void extGlBindFramebuffer(GLenum target, GLuint framebuffer)
		{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
			if (pGlBindFramebufferOES)
				pGlBindFramebufferOES(target, framebuffer);
#elif defined(GL_OES_framebuffer_object)
			glBindFramebufferOES(target, framebuffer);
#else
			os::Printer::log("glBindFramebuffer not supported", ELL_ERROR);
#endif
		}

		void extGlDeleteFramebuffers(GLsizei n, const GLuint *framebuffers)
		{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
			if (pGlDeleteFramebuffersOES)
				pGlDeleteFramebuffersOES(n, framebuffers);
#elif defined(GL_OES_framebuffer_object)
			glDeleteFramebuffersOES(n, framebuffers);
#else
			os::Printer::log("glDeleteFramebuffers not supported", ELL_ERROR);
#endif
		}

		void extGlGenFramebuffers(GLsizei n, GLuint *framebuffers)
		{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
			if (pGlGenFramebuffersOES)
				pGlGenFramebuffersOES(n, framebuffers);
#elif defined(GL_OES_framebuffer_object)
			glGenFramebuffersOES(n, framebuffers);
#else
			os::Printer::log("glGenFramebuffers not supported", ELL_ERROR);
#endif
		}

		GLenum extGlCheckFramebufferStatus(GLenum target)
		{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
			if (pGlCheckFramebufferStatusOES)
				return pGlCheckFramebufferStatusOES(target);
			else
				return 0;
#elif defined(GL_OES_framebuffer_object)
			return glCheckFramebufferStatusOES(target);
#else
			os::Printer::log("glCheckFramebufferStatus not supported", ELL_ERROR);
			return 0;
#endif
		}

		void extGlFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
		{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
			if (pGlFramebufferTexture2DOES)
				pGlFramebufferTexture2DOES(target, attachment, textarget, texture, level);
#elif defined(GL_OES_framebuffer_object)
			glFramebufferTexture2DOES(target, attachment, textarget, texture, level);
#else
			os::Printer::log("glFramebufferTexture2D not supported", ELL_ERROR);
#endif
		}

		void extGlBindRenderbuffer(GLenum target, GLuint renderbuffer)
		{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
			if (pGlBindRenderbufferOES)
				pGlBindRenderbufferOES(target, renderbuffer);
#elif defined(GL_OES_framebuffer_object)
			glBindRenderbufferOES(target, renderbuffer);
#else
			os::Printer::log("glBindRenderbuffer not supported", ELL_ERROR);
#endif
		}

		void extGlDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers)
		{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
			if (pGlDeleteRenderbuffersOES)
				pGlDeleteRenderbuffersOES(n, renderbuffers);
#elif defined(GL_OES_framebuffer_object)
			glDeleteRenderbuffersOES(n, renderbuffers);
#else
			os::Printer::log("glDeleteRenderbuffers not supported", ELL_ERROR);
#endif
		}

		void extGlGenRenderbuffers(GLsizei n, GLuint *renderbuffers)
		{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
			if (pGlGenRenderbuffersOES)
				pGlGenRenderbuffersOES(n, renderbuffers);
#elif defined(GL_OES_framebuffer_object)
			glGenRenderbuffersOES(n, renderbuffers);
#else
			os::Printer::log("glGenRenderbuffers not supported", ELL_ERROR);
#endif
		}

		void extGlRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
		{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
			if (pGlRenderbufferStorageOES)
				pGlRenderbufferStorageOES(target, internalformat, width, height);
#elif defined(GL_OES_framebuffer_object)
			glRenderbufferStorageOES(target, internalformat, width, height);
#else
			os::Printer::log("glRenderbufferStorage not supported", ELL_ERROR);
#endif
		}

		void extGlFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
		{
#ifdef _IRR_OGLES1_USE_EXTPOINTER_
			if (pGlFramebufferRenderbufferOES)
				pGlFramebufferRenderbufferOES(target, attachment, renderbuffertarget, renderbuffer);
#elif defined(GL_OES_framebuffer_object)
			glFramebufferRenderbufferOES(target, attachment, renderbuffertarget, renderbuffer);
#else
			os::Printer::log("glFramebufferRenderbuffer not supported", ELL_ERROR);
#endif
		}

		void extGlDrawTex(GLfloat X, GLfloat Y, GLfloat Z, GLfloat W, GLfloat H)
		{
#if defined(_IRR_OGLES1_USE_EXTPOINTER_)
			if (pGlDrawTexfOES)
				pGlDrawTexfOES(X, Y, Z, W, H);
#elif defined(GL_OES_draw_texture)
			glDrawTexfOES(X, Y, Z, W, H);
#else
			os::Printer::log("glDrawTexture not supported", ELL_ERROR);
#endif
		}

		void extGlDrawTex(GLint X, GLint Y, GLint Z, GLint W, GLint H)
		{
#if defined(_IRR_OGLES1_USE_EXTPOINTER_)
			if (pGlDrawTexiOES)
				pGlDrawTexiOES(X, Y, Z, W, H);
#elif defined(GL_OES_draw_texture)
			glDrawTexiOES(X, Y, Z, W, H);
#else
			os::Printer::log("glDrawTexture not supported", ELL_ERROR);
#endif
		}

		void extGlDrawTex(GLfloat* coords)
		{
#if defined(_IRR_OGLES1_USE_EXTPOINTER_)
			if (pGlDrawTexfvOES)
				pGlDrawTexfvOES(coords);
#elif defined(GL_OES_draw_texture)
			glDrawTexfvOES(coords);
#else
			os::Printer::log("glDrawTexture not supported", ELL_ERROR);
#endif
		}

		void extGlDrawTex(GLint* coords)
		{
#if defined(_IRR_OGLES1_USE_EXTPOINTER_)
			if (pGlDrawTexivOES)
				pGlDrawTexivOES(coords);
#elif defined(GL_OES_draw_texture)
			glDrawTexivOES(coords);
#else
			os::Printer::log("glDrawTexture not supported", ELL_ERROR);
#endif
		}

		// we need to implement some methods which have been extensions in the original OpenGL driver
		void extGlActiveTexture(GLenum texture)
		{
			glActiveTexture(texture);
		}
		void extGlClientActiveTexture(GLenum texture)
		{
			glClientActiveTexture(texture);
		}
		void extGlGenBuffers(GLsizei n, GLuint *buffers)
		{
			glGenBuffers(n, buffers);
		}
		void extGlBindBuffer(GLenum target, GLuint buffer)
		{
			glBindBuffer(target, buffer);
		}
		void extGlBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
		{
			glBufferData(target, size, data, usage);
		}
		void extGlBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data)
		{
			glBufferSubData(target, offset, size, data);
		}
		void extGlDeleteBuffers(GLsizei n, const GLuint *buffers)
		{
			glDeleteBuffers(n, buffers);
		}
		void extGlPointParameterf(GLint loc, GLfloat f)
		{
			glPointParameterf(loc, f);
		}
		void extGlPointParameterfv(GLint loc, const GLfloat *v)
		{
			glPointParameterfv(loc, v);
		}

//	private:
#if defined(_IRR_OGLES1_USE_EXTPOINTER_)
		typedef void (GL_APIENTRYP PFNGLDRAWTEXIOES) (GLint x, GLint y, GLint z, GLint width, GLint height);
		typedef void (GL_APIENTRYP PFNGLDRAWTEXIVOES) (const GLint* coords);
		typedef void (GL_APIENTRYP PFNGLDRAWTEXFOES) (GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height);
		typedef void (GL_APIENTRYP PFNGLDRAWTEXFVOES) (const GLfloat* coords);
		typedef GLboolean (GL_APIENTRYP PFNGLISRENDERBUFFEROES) (GLuint renderbuffer);
		typedef void (GL_APIENTRYP PFNGLBINDRENDERBUFFEROES) (GLenum target, GLuint renderbuffer);
		typedef void (GL_APIENTRYP PFNGLDELETERENDERBUFFERSOES) (GLsizei n, const GLuint* renderbuffers);
		typedef void (GL_APIENTRYP PFNGLGENRENDERBUFFERSOES) (GLsizei n, GLuint* renderbuffers);
		typedef void (GL_APIENTRYP PFNGLRENDERBUFFERSTORAGEOES) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
		typedef void (GL_APIENTRYP PFNGLGETRENDERBUFFERPARAMETERIVOES) (GLenum target, GLenum pname, GLint* params);
		typedef GLboolean (GL_APIENTRYP PFNGLISFRAMEBUFFEROES) (GLuint framebuffer);
		typedef void (GL_APIENTRYP PFNGLBINDFRAMEBUFFEROES) (GLenum target, GLuint framebuffer);
		typedef void (GL_APIENTRYP PFNGLDELETEFRAMEBUFFERSOES) (GLsizei n, const GLuint* framebuffers);
		typedef void (GL_APIENTRYP PFNGLGENFRAMEBUFFERSOES) (GLsizei n, GLuint* framebuffers);
		typedef GLenum (GL_APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSOES) (GLenum target);
		typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFEROES) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
		typedef void (GL_APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DOES) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
		typedef void (GL_APIENTRYP PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOES) (GLenum target, GLenum attachment, GLenum pname, GLint* params);
		typedef void (GL_APIENTRYP PFNGLGENERATEMIPMAPOES) (GLenum target);

		PFNGLDRAWTEXIOES pGlDrawTexiOES;
		PFNGLDRAWTEXFOES pGlDrawTexfOES;
		PFNGLDRAWTEXIVOES pGlDrawTexivOES;
		PFNGLDRAWTEXFVOES pGlDrawTexfvOES;
		PFNGLBINDRENDERBUFFEROES pGlBindRenderbufferOES;
		PFNGLDELETERENDERBUFFERSOES pGlDeleteRenderbuffersOES;
		PFNGLGENRENDERBUFFERSOES pGlGenRenderbuffersOES;
		PFNGLRENDERBUFFERSTORAGEOES pGlRenderbufferStorageOES;
		PFNGLBINDFRAMEBUFFEROES pGlBindFramebufferOES;
		PFNGLDELETEFRAMEBUFFERSOES pGlDeleteFramebuffersOES;
		PFNGLGENFRAMEBUFFERSOES pGlGenFramebuffersOES;
		PFNGLCHECKFRAMEBUFFERSTATUSOES pGlCheckFramebufferStatusOES;
		PFNGLFRAMEBUFFERRENDERBUFFEROES pGlFramebufferRenderbufferOES;
		PFNGLFRAMEBUFFERTEXTURE2DOES pGlFramebufferTexture2DOES;
		PFNGLGENERATEMIPMAPOES pGlGenerateMipMapOES;
#endif
	};

} // end namespace video
} // end namespace irr


#endif // _IRR_COMPILE_WITH_OGLES1_
#endif

