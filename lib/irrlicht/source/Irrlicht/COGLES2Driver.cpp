// Copyright (C) 2013 Patryk Nadrowski
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// OpenGL ES driver implemented by Christian Stehno and first OpenGL ES 2.0
// driver implemented by Amundis.
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "COGLES2Driver.h"
// needed here also because of the create methods' parameters
#include "CNullDriver.h"

#ifdef _IRR_COMPILE_WITH_OGLES2_

#include "COGLES2Texture.h"
#include "COGLES2MaterialRenderer.h"
#include "COGLES2FixedPipelineRenderer.h"
#include "COGLES2NormalMapRenderer.h"
#include "COGLES2ParallaxMapRenderer.h"
#include "COGLES2Renderer2D.h"
#include "CContextEGL.h"
#include "CImage.h"
#include "os.h"

#if defined(_IRR_COMPILE_WITH_IPHONE_DEVICE_)
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#else
#include <GLES2/gl2.h>
#endif

namespace irr
{
namespace video
{
	bool useCoreContext = true;

//! constructor and init code
	COGLES2Driver::COGLES2Driver(const SIrrlichtCreationParameters& params,
			const SExposedVideoData& data, io::IFileSystem* io
#if defined(_IRR_COMPILE_WITH_IPHONE_DEVICE_)
			, CIrrDeviceIPhone* device
#endif
	)
		: CNullDriver(io, params.WindowSize), COGLES2ExtensionHandler(),
		BridgeCalls(0), CurrentRenderMode(ERM_NONE), ResetRenderStates(true),
		Transformation3DChanged(true), AntiAlias(params.AntiAlias),
		RenderTargetTexture(0), CurrentRendertargetSize(0, 0), ColorFormat(ECF_R8G8B8)
#if defined(_IRR_COMPILE_WITH_EGL_)
		, EglContext(0)
		, EglContextExternal(false)
#elif defined(_IRR_COMPILE_WITH_IPHONE_DEVICE_)
		, ViewFramebuffer(0)
		, ViewRenderbuffer(0)
		, ViewDepthRenderbuffer(0)
#endif
#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_)
		, HDc(0)
#endif
		, Params(params)
	{
#ifdef _DEBUG
		setDebugName("COGLES2Driver");
#endif
		ExposedData = data;

#if defined(_IRR_COMPILE_WITH_EGL_)
		EglContext = new ContextManagerEGL();
		
		ContextEGLParams egl_params;
		egl_params.opengl_api = CEGL_API_OPENGL_ES;
		egl_params.surface_type = CEGL_SURFACE_WINDOW;
		egl_params.handle_srgb = Params.HandleSRGB;
		egl_params.force_legacy_device = Params.ForceLegacyDevice;
		egl_params.with_alpha_channel = Params.WithAlphaChannel;
		egl_params.vsync_enabled = Params.Vsync;
	
#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_)
		egl_params.window = (EGLNativeWindowType)(data.OpenGLWin32.HWnd);
		HDc = GetDC(data.OpenGLWin32.HWnd);
		egl_params.display = (NativeDisplayType)(HDc);
#elif defined(_IRR_COMPILE_WITH_X11_DEVICE_)
		egl_params.window = (EGLNativeWindowType)(data.OpenGLLinux.X11Window);
		egl_params.display = (EGLNativeDisplayType)(data.OpenGLLinux.X11Display);
#elif defined(_IRR_COMPILE_WITH_ANDROID_DEVICE_)
		egl_params.window =	((struct android_app *)(params.PrivateData))->window;
		egl_params.display = NULL;
#endif
		
		EglContext->init(egl_params);
		useCoreContext = !EglContext->isLegacyDevice();

		genericDriverInit(params.WindowSize, params.Stencilbuffer);
		
#ifdef _IRR_COMPILE_WITH_ANDROID_DEVICE_
		int width = 0;
		int height = 0;
		EglContext->getSurfaceDimensions(&width, &height);
        CNullDriver::ScreenSize = core::dimension2d<u32>(width, height);
#endif

#elif defined(_IRR_COMPILE_WITH_IPHONE_DEVICE_)
		Device = device;

        glGenFramebuffers(1, &ViewFramebuffer);
        glGenRenderbuffers(1, &ViewRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, ViewRenderbuffer);

        ExposedData.OGLESIPhone.AppDelegate = Device;
        Device->displayInitialize(&ExposedData.OGLESIPhone.Context, &ExposedData.OGLESIPhone.View);

        GLint backingWidth;
        GLint backingHeight;
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);

        glGenRenderbuffers(1, &ViewDepthRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, ViewDepthRenderbuffer);

        GLenum depthComponent = GL_DEPTH_COMPONENT16;

        if(params.ZBufferBits >= 24)
            depthComponent = GL_DEPTH_COMPONENT24_OES;

        glRenderbufferStorage(GL_RENDERBUFFER, depthComponent, backingWidth, backingHeight);

        glBindFramebuffer(GL_FRAMEBUFFER, ViewFramebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ViewRenderbuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ViewDepthRenderbuffer);

        core::dimension2d<u32> WindowSize(backingWidth, backingHeight);
        CNullDriver::ScreenSize = WindowSize;
        CNullDriver::ViewPort = core::rect<s32>(core::position2d<s32>(0,0), core::dimension2di(WindowSize));

        genericDriverInit(WindowSize, params.Stencilbuffer);
#endif
	}

#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_
	COGLES2Driver::COGLES2Driver(const SIrrlichtCreationParameters& params, 
				  io::IFileSystem* io, CIrrDeviceWayland* device)
		: CNullDriver(io, params.WindowSize), COGLES2ExtensionHandler(),
		BridgeCalls(0), CurrentRenderMode(ERM_NONE), ResetRenderStates(true),
		Transformation3DChanged(true), AntiAlias(params.AntiAlias),
		RenderTargetTexture(0), CurrentRendertargetSize(0, 0), 
		ColorFormat(ECF_R8G8B8), EglContext(0), EglContextExternal(false), 
		Params(params)
	{
		EglContext = device->getEGLContext();
		EglContextExternal = true;
		genericDriverInit(params.WindowSize, params.Stencilbuffer);
	}
#endif
				  

	//! destructor
	COGLES2Driver::~COGLES2Driver()
	{
		deleteMaterialRenders();
		delete MaterialRenderer2D;
		deleteAllTextures();

		if (BridgeCalls)
			delete BridgeCalls;

#if defined(_IRR_COMPILE_WITH_EGL_)
		if (!EglContextExternal)
			delete EglContext;
		
#if defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_)
		if (HDc)
			ReleaseDC((ExposedData.OpenGLWin32.HWnd, HDc);
#endif


#elif defined(_IRR_COMPILE_WITH_IPHONE_DEVICE_)
        if (0 != ViewFramebuffer)
        {
            glDeleteFramebuffers(1,&ViewFramebuffer);
            ViewFramebuffer = 0;
        }
        if (0 != ViewRenderbuffer)
        {
            glDeleteRenderbuffers(1,&ViewRenderbuffer);
            ViewRenderbuffer = 0;
        }
        if (0 != ViewDepthRenderbuffer)
        {
            glDeleteRenderbuffers(1,&ViewDepthRenderbuffer);
            ViewDepthRenderbuffer = 0;
        }
#endif
	}

// -----------------------------------------------------------------------
// METHODS
// -----------------------------------------------------------------------


	bool COGLES2Driver::genericDriverInit(const core::dimension2d<u32>& screenSize, bool stencilBuffer)
	{
		Name = glGetString(GL_VERSION);
		printVersion();

		// print renderer information
		vendorName = glGetString(GL_VENDOR);
		os::Printer::log(vendorName.c_str(), ELL_INFORMATION);

		u32 i;
		for (i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
			CurrentTexture[i] = 0;
		// load extensions
		initExtensions(this, stencilBuffer);

		if (!BridgeCalls)
			BridgeCalls = new COGLES2CallBridge(this);

		StencilBuffer = stencilBuffer;

		DriverAttributes->setAttribute("MaxTextures", MaxTextureUnits);
		DriverAttributes->setAttribute("MaxSupportedTextures", MaxSupportedTextures);
//		DriverAttributes->setAttribute("MaxLights", MaxLights);
		DriverAttributes->setAttribute("MaxAnisotropy", MaxAnisotropy);
//		DriverAttributes->setAttribute("MaxUserClipPlanes", MaxUserClipPlanes);
//		DriverAttributes->setAttribute("MaxAuxBuffers", MaxAuxBuffers);
//		DriverAttributes->setAttribute("MaxMultipleRenderTargets", MaxMultipleRenderTargets);
		DriverAttributes->setAttribute("MaxIndices", (s32)MaxIndices);
		DriverAttributes->setAttribute("MaxTextureSize", (s32)MaxTextureSize);
		DriverAttributes->setAttribute("MaxTextureLODBias", MaxTextureLODBias);
		DriverAttributes->setAttribute("Version", Version);
		DriverAttributes->setAttribute("AntiAlias", AntiAlias);

		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		// Reset The Current Viewport
		BridgeCalls->setViewport(core::rect<s32>(0, 0, screenSize.Width, screenSize.Height));

		UserClipPlane.reallocate(0);

		setAmbientLight(SColorf(0.0f, 0.0f, 0.0f, 0.0f));
		glClearDepthf(1.0f);

		//TODO : OpenGL ES 2.0 Port : GL_PERSPECTIVE_CORRECTION_HINT
		//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
		glHint(GL_GENERATE_MIPMAP_HINT, GL_FASTEST);
		glDepthFunc(GL_LEQUAL);
		glFrontFace(GL_CW);

		// create material renderers
		createMaterialRenderers();

		// set the renderstates
		setRenderStates3DMode();

		// set fog mode
		setFog(FogColor, FogType, FogStart, FogEnd, FogDensity, PixelFog, RangeFog);

		// create matrix for flipping textures
		TextureFlipMatrix.buildTextureTransform(0.0f, core::vector2df(0, 0), core::vector2df(0, 1.0f), core::vector2df(1.0f, -1.0f));

		// We need to reset once more at the beginning of the first rendering.
		// This fixes problems with intermediate changes to the material during texture load.
		ResetRenderStates = true;

		testGLError();

		return true;
	}


	void COGLES2Driver::createMaterialRenderers()
	{
		// Load shaders from files (in future shaders will be merged with source code).

		// Fixed pipeline.

		core::stringc shaders_path = IRR_OGLES2_SHADER_PATH;
		if (Params.ShadersPath.size() > 0)
			shaders_path = Params.ShadersPath;

		core::stringc FPVSPath = shaders_path;
		FPVSPath += "COGLES2FixedPipeline.vsh";

		core::stringc FPFSPath = shaders_path;
		FPFSPath += "COGLES2FixedPipeline.fsh";

		io::IReadFile* FPVSFile = FileSystem->createAndOpenFile(FPVSPath);
		io::IReadFile* FPFSFile = FileSystem->createAndOpenFile(FPFSPath);

		c8* FPVSData = 0;
		c8* FPFSData = 0;

		long Size = FPVSFile ? FPVSFile->getSize() : 0;

		if (Size)
		{
			FPVSData = new c8[Size+1];
			FPVSFile->read(FPVSData, Size);
			FPVSData[Size] = 0;
		}

		Size = FPFSFile ? FPFSFile->getSize() : 0;

		if (Size)
		{
			// if both handles are the same we must reset the file
			if (FPFSFile == FPVSFile)
				FPFSFile->seek(0);

			FPFSData = new c8[Size+1];
			FPFSFile->read(FPFSData, Size);
			FPFSData[Size] = 0;
		}

		if (FPVSFile)
			FPVSFile->drop();

		if (FPFSFile)
			FPFSFile->drop();

		// Normal Mapping.

		core::stringc NMVSPath = shaders_path;
		NMVSPath += "COGLES2NormalMap.vsh";

		core::stringc NMFSPath = shaders_path;
		NMFSPath += "COGLES2NormalMap.fsh";

		io::IReadFile* NMVSFile = FileSystem->createAndOpenFile(NMVSPath);
		io::IReadFile* NMFSFile = FileSystem->createAndOpenFile(NMFSPath);

		c8* NMVSData = 0;
		c8* NMFSData = 0;

		Size = NMVSFile ? NMVSFile->getSize() : 0;

		if (Size)
		{
			NMVSData = new c8[Size+1];
			NMVSFile->read(NMVSData, Size);
			NMVSData[Size] = 0;
		}

		Size = NMFSFile ? NMFSFile->getSize() : 0;

		if (Size)
		{
			// if both handles are the same we must reset the file
			if (NMFSFile == NMVSFile)
				NMFSFile->seek(0);

			NMFSData = new c8[Size+1];
			NMFSFile->read(NMFSData, Size);
			NMFSData[Size] = 0;
		}

		if (NMVSFile)
			NMVSFile->drop();

		if (NMFSFile)
			NMFSFile->drop();

		// Parallax Mapping.

		core::stringc PMVSPath = shaders_path;
		PMVSPath += "COGLES2ParallaxMap.vsh";

		core::stringc PMFSPath = shaders_path;
		PMFSPath += "COGLES2ParallaxMap.fsh";

		io::IReadFile* PMVSFile = FileSystem->createAndOpenFile(FPVSPath);
		io::IReadFile* PMFSFile = FileSystem->createAndOpenFile(FPFSPath);

		c8* PMVSData = 0;
		c8* PMFSData = 0;

		Size = PMVSFile ? PMVSFile->getSize() : 0;

		if (Size)
		{
			PMVSData = new c8[Size+1];
			PMVSFile->read(PMVSData, Size);
			PMVSData[Size] = 0;
		}

		Size = PMFSFile ? PMFSFile->getSize() : 0;

		if (Size)
		{
			// if both handles are the same we must reset the file
			if (PMFSFile == PMVSFile)
				PMFSFile->seek(0);

			PMFSData = new c8[Size+1];
			PMFSFile->read(PMFSData, Size);
			PMFSData[Size] = 0;
		}

		if (PMVSFile)
			PMVSFile->drop();

		if (PMFSFile)
			PMFSFile->drop();

		// Create materials.

		addAndDropMaterialRenderer(new COGLES2FixedPipelineRenderer(FPVSData, FPFSData, EMT_SOLID, this));
		addAndDropMaterialRenderer(new COGLES2FixedPipelineRenderer(FPVSData, FPFSData, EMT_SOLID_2_LAYER, this));
		addAndDropMaterialRenderer(new COGLES2FixedPipelineRenderer(FPVSData, FPFSData, EMT_LIGHTMAP, this));
		addAndDropMaterialRenderer(new COGLES2FixedPipelineRenderer(FPVSData, FPFSData, EMT_LIGHTMAP_ADD, this));
		addAndDropMaterialRenderer(new COGLES2FixedPipelineRenderer(FPVSData, FPFSData, EMT_LIGHTMAP_M2, this));
		addAndDropMaterialRenderer(new COGLES2FixedPipelineRenderer(FPVSData, FPFSData, EMT_LIGHTMAP_M4, this));
		addAndDropMaterialRenderer(new COGLES2FixedPipelineRenderer(FPVSData, FPFSData, EMT_LIGHTMAP_LIGHTING, this));
		addAndDropMaterialRenderer(new COGLES2FixedPipelineRenderer(FPVSData, FPFSData, EMT_LIGHTMAP_LIGHTING_M2, this));
		addAndDropMaterialRenderer(new COGLES2FixedPipelineRenderer(FPVSData, FPFSData, EMT_LIGHTMAP_LIGHTING_M4, this));
		addAndDropMaterialRenderer(new COGLES2FixedPipelineRenderer(FPVSData, FPFSData, EMT_DETAIL_MAP, this));
		addAndDropMaterialRenderer(new COGLES2FixedPipelineRenderer(FPVSData, FPFSData, EMT_SPHERE_MAP, this));
		addAndDropMaterialRenderer(new COGLES2FixedPipelineRenderer(FPVSData, FPFSData, EMT_REFLECTION_2_LAYER, this));
		addAndDropMaterialRenderer(new COGLES2FixedPipelineRenderer(FPVSData, FPFSData, EMT_TRANSPARENT_ADD_COLOR, this));
		addAndDropMaterialRenderer(new COGLES2FixedPipelineRenderer(FPVSData, FPFSData, EMT_TRANSPARENT_ALPHA_CHANNEL, this));
		addAndDropMaterialRenderer(new COGLES2FixedPipelineRenderer(FPVSData, FPFSData, EMT_TRANSPARENT_ALPHA_CHANNEL_REF, this));
		addAndDropMaterialRenderer(new COGLES2FixedPipelineRenderer(FPVSData, FPFSData, EMT_TRANSPARENT_VERTEX_ALPHA, this));
		addAndDropMaterialRenderer(new COGLES2FixedPipelineRenderer(FPVSData, FPFSData, EMT_TRANSPARENT_REFLECTION_2_LAYER, this));

		if (!useCoreContext)
		{
			addAndDropMaterialRenderer(new COGLES2NormalMapRenderer(NMVSData, NMFSData, EMT_NORMAL_MAP_SOLID, this));
			addAndDropMaterialRenderer(new COGLES2NormalMapRenderer(NMVSData, NMFSData, EMT_NORMAL_MAP_TRANSPARENT_ADD_COLOR, this));
			addAndDropMaterialRenderer(new COGLES2NormalMapRenderer(NMVSData, NMFSData, EMT_NORMAL_MAP_TRANSPARENT_VERTEX_ALPHA, this));
		}

		addAndDropMaterialRenderer(new COGLES2ParallaxMapRenderer(PMVSData, PMFSData, EMT_PARALLAX_MAP_SOLID, this));
		addAndDropMaterialRenderer(new COGLES2ParallaxMapRenderer(PMVSData, PMFSData, EMT_PARALLAX_MAP_TRANSPARENT_ADD_COLOR, this));
		addAndDropMaterialRenderer(new COGLES2ParallaxMapRenderer(PMVSData, PMFSData, EMT_PARALLAX_MAP_TRANSPARENT_VERTEX_ALPHA, this));

		addAndDropMaterialRenderer(new COGLES2FixedPipelineRenderer(FPVSData, FPFSData, EMT_ONETEXTURE_BLEND, this));
		
		delete[] FPVSData;
		delete[] FPFSData;
		delete[] NMVSData;
		delete[] NMFSData;
		delete[] PMVSData;
		delete[] PMFSData;

		// Create 2D material renderer.

		core::stringc R2DVSPath = shaders_path;
		R2DVSPath += "COGLES2Renderer2D.vsh";

		core::stringc R2DFSPath = shaders_path;
		R2DFSPath += "COGLES2Renderer2D.fsh";

		io::IReadFile* R2DVSFile = FileSystem->createAndOpenFile(R2DVSPath);
		io::IReadFile* R2DFSFile = FileSystem->createAndOpenFile(R2DFSPath);

		c8* R2DVSData = 0;
		c8* R2DFSData = 0;

		Size = R2DVSFile ? R2DVSFile->getSize() : 0;

		if (Size)
		{
			R2DVSData = new c8[Size+1];
			R2DVSFile->read(R2DVSData, Size);
			R2DVSData[Size] = 0;
		}

		Size = R2DFSFile ? R2DFSFile->getSize() : 0;

		if (Size)
		{
			// if both handles are the same we must reset the file
			if (R2DFSFile == PMVSFile)
				R2DFSFile->seek(0);

			R2DFSData = new c8[Size+1];
			R2DFSFile->read(R2DFSData, Size);
			R2DFSData[Size] = 0;
		}

		if (R2DVSFile)
			R2DVSFile->drop();

		if (R2DFSFile)
			R2DFSFile->drop();

		MaterialRenderer2D = new COGLES2Renderer2D(R2DVSData, R2DFSData, this);

		delete[] R2DVSData;
		delete[] R2DFSData;
	}


	//! presents the rendered scene on the screen, returns false if failed
	bool COGLES2Driver::endScene()
	{
		CNullDriver::endScene();

#if defined(_IRR_COMPILE_WITH_EGL_)
		bool res = EglContext->swapBuffers();
		
		if (!res)
		{
			os::Printer::log("Could not swap buffers for OpenGL-ES2 driver.");
			return false;
		}
#elif defined(_IRR_COMPILE_WITH_IPHONE_DEVICE_)
		glFlush();
		glBindRenderbuffer(GL_RENDERBUFFER, ViewRenderbuffer);
		Device->displayEnd();
#endif

		return true;
	}


	//! clears the zbuffer
	bool COGLES2Driver::beginScene(bool backBuffer, bool zBuffer, SColor color,
			const SExposedVideoData& videoData, core::rect<s32>* sourceRect)
	{
		CNullDriver::beginScene(backBuffer, zBuffer, color);

		GLbitfield mask = 0;

		if (backBuffer)
		{
			const f32 inv = 1.0f / 255.0f;
			glClearColor(color.getRed() * inv, color.getGreen() * inv,
						color.getBlue() * inv, color.getAlpha() * inv);

			mask |= GL_COLOR_BUFFER_BIT;
		}

		if (zBuffer)
		{
			glDepthMask(GL_TRUE);
			LastMaterial.ZWriteEnable = true;
			mask |= GL_DEPTH_BUFFER_BIT;
		}

		glClear(mask);
		testGLError();
		return true;
	}


	//! Returns the transformation set by setTransform
	const core::matrix4& COGLES2Driver::getTransform(E_TRANSFORMATION_STATE state) const
	{
		return Matrices[state];
	}


	//! sets transformation
	void COGLES2Driver::setTransform(E_TRANSFORMATION_STATE state, const core::matrix4& mat)
	{
		Matrices[state] = mat;
		Transformation3DChanged = true;
	}


	bool COGLES2Driver::updateVertexHardwareBuffer(SHWBufferLink_opengl *HWBuffer)
	{
		if (!HWBuffer)
			return false;

		const scene::IMeshBuffer* mb = HWBuffer->MeshBuffer;
		const void* vertices = mb->getVertices();
		const u32 vertexCount = mb->getVertexCount();
		const E_VERTEX_TYPE vType = mb->getVertexType();
		const u32 vertexSize = getVertexPitchFromType(vType);

		//buffer vertex data, and convert colours...
		core::array<c8> buffer(vertexSize * vertexCount);
		memcpy(buffer.pointer(), vertices, vertexSize * vertexCount);

		//get or create buffer
		bool newBuffer = false;
		if (!HWBuffer->vbo_verticesID)
		{
			glGenBuffers(1, &HWBuffer->vbo_verticesID);
			if (!HWBuffer->vbo_verticesID) return false;
			newBuffer = true;
		}
		else if (HWBuffer->vbo_verticesSize < vertexCount*vertexSize)
		{
			newBuffer = true;
		}

		glBindBuffer(GL_ARRAY_BUFFER, HWBuffer->vbo_verticesID);

		//copy data to graphics card
		glGetError(); // clear error storage
		if (!newBuffer)
			glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount * vertexSize, buffer.const_pointer());
		else
		{
			HWBuffer->vbo_verticesSize = vertexCount * vertexSize;

			if (HWBuffer->Mapped_Vertex == scene::EHM_STATIC)
				glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexSize, buffer.const_pointer(), GL_STATIC_DRAW);
			else
				glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexSize, buffer.const_pointer(), GL_DYNAMIC_DRAW);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return (glGetError() == GL_NO_ERROR);
	}


	bool COGLES2Driver::updateIndexHardwareBuffer(SHWBufferLink_opengl *HWBuffer)
	{
		if (!HWBuffer)
			return false;

		const scene::IMeshBuffer* mb = HWBuffer->MeshBuffer;

		const void* indices = mb->getIndices();
		u32 indexCount = mb->getIndexCount();

		GLenum indexSize;
		switch (mb->getIndexType())
		{
			case(EIT_16BIT):
			{
				indexSize = sizeof(u16);
				break;
			}
			case(EIT_32BIT):
			{
				indexSize = sizeof(u32);
				break;
			}
			default:
			{
				return false;
			}
		}

		//get or create buffer
		bool newBuffer = false;
		if (!HWBuffer->vbo_indicesID)
		{
			glGenBuffers(1, &HWBuffer->vbo_indicesID);
			if (!HWBuffer->vbo_indicesID) return false;
			newBuffer = true;
		}
		else if (HWBuffer->vbo_indicesSize < indexCount*indexSize)
		{
			newBuffer = true;
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, HWBuffer->vbo_indicesID);

		//copy data to graphics card
		glGetError(); // clear error storage
		if (!newBuffer)
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexCount * indexSize, indices);
		else
		{
			HWBuffer->vbo_indicesSize = indexCount * indexSize;

			if (HWBuffer->Mapped_Index == scene::EHM_STATIC)
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * indexSize, indices, GL_STATIC_DRAW);
			else
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * indexSize, indices, GL_DYNAMIC_DRAW);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		return (glGetError() == GL_NO_ERROR);
	}


	//! updates hardware buffer if needed
	bool COGLES2Driver::updateHardwareBuffer(SHWBufferLink *HWBuffer)
	{
		if (!HWBuffer)
			return false;

		if (HWBuffer->Mapped_Vertex != scene::EHM_NEVER)
		{
			if (HWBuffer->ChangedID_Vertex != HWBuffer->MeshBuffer->getChangedID_Vertex()
				|| !((SHWBufferLink_opengl*)HWBuffer)->vbo_verticesID)
			{

				HWBuffer->ChangedID_Vertex = HWBuffer->MeshBuffer->getChangedID_Vertex();

				if (!updateVertexHardwareBuffer((SHWBufferLink_opengl*)HWBuffer))
					return false;
			}
		}

		if (HWBuffer->Mapped_Index != scene::EHM_NEVER)
		{
			if (HWBuffer->ChangedID_Index != HWBuffer->MeshBuffer->getChangedID_Index()
				|| !((SHWBufferLink_opengl*)HWBuffer)->vbo_indicesID)
			{

				HWBuffer->ChangedID_Index = HWBuffer->MeshBuffer->getChangedID_Index();

				if (!updateIndexHardwareBuffer((SHWBufferLink_opengl*)HWBuffer))
					return false;
			}
		}

		return true;
	}


	//! Create hardware buffer from meshbuffer
	COGLES2Driver::SHWBufferLink *COGLES2Driver::createHardwareBuffer(const scene::IMeshBuffer* mb)
	{
		if (!mb || (mb->getHardwareMappingHint_Index() == scene::EHM_NEVER && mb->getHardwareMappingHint_Vertex() == scene::EHM_NEVER))
			return 0;

		SHWBufferLink_opengl *HWBuffer = new SHWBufferLink_opengl(mb);

		//add to map
		HWBufferMap.insert(HWBuffer->MeshBuffer, HWBuffer);

		HWBuffer->ChangedID_Vertex = HWBuffer->MeshBuffer->getChangedID_Vertex();
		HWBuffer->ChangedID_Index = HWBuffer->MeshBuffer->getChangedID_Index();
		HWBuffer->Mapped_Vertex = mb->getHardwareMappingHint_Vertex();
		HWBuffer->Mapped_Index = mb->getHardwareMappingHint_Index();
		HWBuffer->LastUsed = 0;
		HWBuffer->vbo_verticesID = 0;
		HWBuffer->vbo_indicesID = 0;
		HWBuffer->vbo_verticesSize = 0;
		HWBuffer->vbo_indicesSize = 0;

		if (!updateHardwareBuffer(HWBuffer))
		{
			deleteHardwareBuffer(HWBuffer);
			return 0;
		}

		return HWBuffer;
	}


	void COGLES2Driver::deleteHardwareBuffer(SHWBufferLink *_HWBuffer)
	{
		if (!_HWBuffer)
			return;

		SHWBufferLink_opengl *HWBuffer = (SHWBufferLink_opengl*)_HWBuffer;
		if (HWBuffer->vbo_verticesID)
		{
			glDeleteBuffers(1, &HWBuffer->vbo_verticesID);
			HWBuffer->vbo_verticesID = 0;
		}
		if (HWBuffer->vbo_indicesID)
		{
			glDeleteBuffers(1, &HWBuffer->vbo_indicesID);
			HWBuffer->vbo_indicesID = 0;
		}

		CNullDriver::deleteHardwareBuffer(_HWBuffer);
	}


	//! Draw hardware buffer
	void COGLES2Driver::drawHardwareBuffer(SHWBufferLink *_HWBuffer)
	{
		if (!_HWBuffer)
			return;

		SHWBufferLink_opengl *HWBuffer = (SHWBufferLink_opengl*)_HWBuffer;

		updateHardwareBuffer(HWBuffer); //check if update is needed

		HWBuffer->LastUsed = 0;//reset count

		const scene::IMeshBuffer* mb = HWBuffer->MeshBuffer;
		const void *vertices = mb->getVertices();
		const void *indexList = mb->getIndices();

		if (HWBuffer->Mapped_Vertex != scene::EHM_NEVER)
		{
			glBindBuffer(GL_ARRAY_BUFFER, HWBuffer->vbo_verticesID);
			vertices = 0;
		}

		if (HWBuffer->Mapped_Index != scene::EHM_NEVER)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, HWBuffer->vbo_indicesID);
			indexList = 0;
		}


		drawVertexPrimitiveList(vertices, mb->getVertexCount(),
				indexList, mb->getIndexCount() / 3,
				mb->getVertexType(), scene::EPT_TRIANGLES,
				mb->getIndexType());

		if (HWBuffer->Mapped_Vertex != scene::EHM_NEVER)
			glBindBuffer(GL_ARRAY_BUFFER, 0);

		if (HWBuffer->Mapped_Index != scene::EHM_NEVER)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}


	// small helper function to create vertex buffer object adress offsets
	static inline u8* buffer_offset(const long offset)
	{
		return ((u8*)0 + offset);
	}


	//! draws a vertex primitive list
	void COGLES2Driver::drawVertexPrimitiveList(const void* vertices, u32 vertexCount,
			const void* indexList, u32 primitiveCount,
			E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType, E_INDEX_TYPE iType)
	{
		testGLError();
		if (!checkPrimitiveCount(primitiveCount))
			return;

		setRenderStates3DMode();

		drawVertexPrimitiveList2d3d(vertices, vertexCount, (const u16*)indexList, primitiveCount, vType, pType, iType);
	}


	void COGLES2Driver::drawVertexPrimitiveList2d3d(const void* vertices, u32 vertexCount,
			const void* indexList, u32 primitiveCount,
			E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType, E_INDEX_TYPE iType, bool threed)
	{
		if (!primitiveCount || !vertexCount)
			return;

		if (!threed && !checkPrimitiveCount(primitiveCount))
			return;

		CNullDriver::drawVertexPrimitiveList(vertices, vertexCount, indexList, primitiveCount, vType, pType, iType);

		//TODO: treat #ifdef GL_OES_point_size_array outside this if
		{
			glEnableVertexAttribArray(EVA_COLOR);
			glEnableVertexAttribArray(EVA_POSITION);
			if ((pType != scene::EPT_POINTS) && (pType != scene::EPT_POINT_SPRITES))
			{
				glEnableVertexAttribArray(EVA_TCOORD0);
			}
#ifdef GL_OES_point_size_array
			else if (FeatureAvailable[IRR_OES_point_size_array] && (Material.Thickness == 0.0f))
				glEnableClientState(GL_POINT_SIZE_ARRAY_OES);
#endif
			if (threed && (pType != scene::EPT_POINTS) && (pType != scene::EPT_POINT_SPRITES))
			{
				glEnableVertexAttribArray(EVA_NORMAL);
			}

			switch (vType)
			{
			case EVT_STANDARD:
				if (vertices)
				{
#ifdef GL_OES_point_size_array
					if ((pType == scene::EPT_POINTS) || (pType == scene::EPT_POINT_SPRITES))
					{
						if (FeatureAvailable[IRR_OES_point_size_array] && (Material.Thickness == 0.0f))
							glPointSizePointerOES(GL_FLOAT, sizeof(S3DVertex), &(static_cast<const S3DVertex*>(vertices))[0].Normal.X);
					}
					else
#endif
						glVertexAttribPointer(EVA_POSITION, (threed ? 3 : 2), GL_FLOAT, false, sizeof(S3DVertex), &(static_cast<const S3DVertex*>(vertices))[0].Pos);
					if (threed)
						glVertexAttribPointer(EVA_NORMAL, 3, GL_FLOAT, false, sizeof(S3DVertex), &(static_cast<const S3DVertex*>(vertices))[0].Normal);
					glVertexAttribPointer(EVA_COLOR, 4, GL_UNSIGNED_BYTE, true, sizeof(S3DVertex), &(static_cast<const S3DVertex*>(vertices))[0].Color);
					glVertexAttribPointer(EVA_TCOORD0, 2, GL_FLOAT, false, sizeof(S3DVertex), &(static_cast<const S3DVertex*>(vertices))[0].TCoords);

				}
				else
				{
					glVertexAttribPointer(EVA_POSITION, 3, GL_FLOAT, false, sizeof(S3DVertex), 0);
					glVertexAttribPointer(EVA_NORMAL, 3, GL_FLOAT, false, sizeof(S3DVertex), buffer_offset(12));
					glVertexAttribPointer(EVA_COLOR, 4, GL_UNSIGNED_BYTE, true, sizeof(S3DVertex), buffer_offset(24));
					glVertexAttribPointer(EVA_TCOORD0, 2, GL_FLOAT, false, sizeof(S3DVertex), buffer_offset(28));
				}

				//if (CurrentTexture[1])
				//{
				//	// There must be some optimisation here as it uses the same texture coord !
				//	glEnableVertexAttribArray(EVA_TCOORD1);
				//	if (vertices)
				//		glVertexAttribPointer(EVA_TCOORD1, 2, GL_FLOAT, false, sizeof(S3DVertex), &(static_cast<const S3DVertex*>(vertices))[0].TCoords);
				//	else
				//		glVertexAttribPointer(EVA_TCOORD1, 2, GL_FLOAT, false, sizeof(S3DVertex), buffer_offset(28));
				//}
				break;
			case EVT_2TCOORDS:
				//glEnableVertexAttribArray(EVA_TCOORD1);
				if (vertices)
				{
					glVertexAttribPointer(EVA_POSITION, (threed ? 3 : 2), GL_FLOAT, false, sizeof(S3DVertex2TCoords), &(static_cast<const S3DVertex2TCoords*>(vertices))[0].Pos);
					if (threed)
						glVertexAttribPointer(EVA_NORMAL, 3, GL_FLOAT, false, sizeof(S3DVertex2TCoords), &(static_cast<const S3DVertex2TCoords*>(vertices))[0].Normal);
					glVertexAttribPointer(EVA_COLOR, 4, GL_UNSIGNED_BYTE, true, sizeof(S3DVertex2TCoords), &(static_cast<const S3DVertex2TCoords*>(vertices))[0].Color);
					glVertexAttribPointer(EVA_TCOORD0, 2, GL_FLOAT, false, sizeof(S3DVertex2TCoords), &(static_cast<const S3DVertex2TCoords*>(vertices))[0].TCoords);
					//glVertexAttribPointer(EVA_TCOORD1, 2, GL_FLOAT, false, sizeof(S3DVertex2TCoords), &(static_cast<const S3DVertex2TCoords*>(vertices))[0].TCoords2);
				}
				else
				{
					glVertexAttribPointer(EVA_POSITION, 3, GL_FLOAT, false, sizeof(S3DVertex2TCoords), buffer_offset(0));
					glVertexAttribPointer(EVA_NORMAL, 3, GL_FLOAT, false, sizeof(S3DVertex2TCoords), buffer_offset(12));
					glVertexAttribPointer(EVA_COLOR, 4, GL_UNSIGNED_BYTE, true, sizeof(S3DVertex2TCoords), buffer_offset(24));
					glVertexAttribPointer(EVA_TCOORD0, 2, GL_FLOAT, false, sizeof(S3DVertex2TCoords), buffer_offset(28));
					//glVertexAttribPointer(EVA_TCOORD1, 2, GL_FLOAT, false, sizeof(S3DVertex2TCoords), buffer_offset(36));

				}
				break;
			case EVT_TANGENTS:
				glEnableVertexAttribArray(EVA_TANGENT);
				glEnableVertexAttribArray(EVA_BINORMAL);
				if (vertices)
				{
					glVertexAttribPointer(EVA_POSITION, (threed ? 3 : 2), GL_FLOAT, false, sizeof(S3DVertexTangents), &(static_cast<const S3DVertexTangents*>(vertices))[0].Pos);
					if (threed)
						glVertexAttribPointer(EVA_NORMAL, 3, GL_FLOAT, false, sizeof(S3DVertexTangents), &(static_cast<const S3DVertexTangents*>(vertices))[0].Normal);
					glVertexAttribPointer(EVA_COLOR, 4, GL_UNSIGNED_BYTE, true, sizeof(S3DVertexTangents), &(static_cast<const S3DVertexTangents*>(vertices))[0].Color);
					glVertexAttribPointer(EVA_TCOORD0, 2, GL_FLOAT, false, sizeof(S3DVertexTangents), &(static_cast<const S3DVertexTangents*>(vertices))[0].TCoords);
					glVertexAttribPointer(EVA_TANGENT, 3, GL_FLOAT, false, sizeof(S3DVertexTangents), &(static_cast<const S3DVertexTangents*>(vertices))[0].Tangent);
					glVertexAttribPointer(EVA_BINORMAL, 3, GL_FLOAT, false, sizeof(S3DVertexTangents), &(static_cast<const S3DVertexTangents*>(vertices))[0].Binormal);
				}
				else
				{
					glVertexAttribPointer(EVA_POSITION, 3, GL_FLOAT, false, sizeof(S3DVertexTangents), buffer_offset(0));
					glVertexAttribPointer(EVA_NORMAL, 3, GL_FLOAT, false, sizeof(S3DVertexTangents), buffer_offset(12));
					glVertexAttribPointer(EVA_COLOR, 4, GL_UNSIGNED_BYTE, true, sizeof(S3DVertexTangents), buffer_offset(24));
					glVertexAttribPointer(EVA_TCOORD0, 2, GL_FLOAT, false, sizeof(S3DVertexTangents), buffer_offset(28));
					glVertexAttribPointer(EVA_TANGENT, 3, GL_FLOAT, false, sizeof(S3DVertexTangents), buffer_offset(36));
					glVertexAttribPointer(EVA_BINORMAL, 3, GL_FLOAT, false, sizeof(S3DVertexTangents), buffer_offset(48));
				}
				break;
			default:
				break;
			}
		}

		// draw everything
		GLenum indexSize = 0;

		switch (iType)
		{
			case(EIT_16BIT):
			{
				indexSize = GL_UNSIGNED_SHORT;
				break;
			}
			case(EIT_32BIT):
			{
#ifdef GL_OES_element_index_uint
#ifndef GL_UNSIGNED_INT
#define GL_UNSIGNED_INT 0x1405
#endif
				if (FeatureAvailable[IRR_OES_element_index_uint])
					indexSize = GL_UNSIGNED_INT;
				else
#endif
					indexSize = GL_UNSIGNED_SHORT;
				break;
			}
		}

		switch (pType)
		{
			case scene::EPT_POINTS:
			case scene::EPT_POINT_SPRITES:
			{
#ifdef GL_OES_point_sprite
				if (pType == scene::EPT_POINT_SPRITES && FeatureAvailable[IRR_OES_point_sprite])
					glEnable(GL_POINT_SPRITE_OES);
#endif
				// if ==0 we use the point size array
				if (Material.Thickness != 0.f)
				{
//						float quadratic[] = {0.0f, 0.0f, 10.01f};
					//TODO : OpenGL ES 2.0 Port GL_POINT_DISTANCE_ATTENUATION
					//glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, quadratic);
//						float maxParticleSize = 1.0f;
					//TODO : OpenGL ES 2.0 Port GL_POINT_SIZE_MAX
					//glGetFloatv(GL_POINT_SIZE_MAX, &maxParticleSize);
//			maxParticleSize=maxParticleSize<Material.Thickness?maxParticleSize:Material.Thickness;
//			glPointParameterf(GL_POINT_SIZE_MAX,maxParticleSize);
//			glPointParameterf(GL_POINT_SIZE_MIN,Material.Thickness);
					//TODO : OpenGL ES 2.0 Port GL_POINT_FADE_THRESHOLD_SIZE
					//glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, 60.0f);
					//glPointSize(Material.Thickness);
				}
#ifdef GL_OES_point_sprite
				if (pType == scene::EPT_POINT_SPRITES && FeatureAvailable[IRR_OES_point_sprite])
					glTexEnvf(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_TRUE);
#endif
				glDrawArrays(GL_POINTS, 0, primitiveCount);
#ifdef GL_OES_point_sprite
				if (pType == scene::EPT_POINT_SPRITES && FeatureAvailable[IRR_OES_point_sprite])
				{
					glDisable(GL_POINT_SPRITE_OES);
					glTexEnvf(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_FALSE);
				}
#endif
			}
			break;
			case scene::EPT_LINE_STRIP:
				glDrawElements(GL_LINE_STRIP, primitiveCount + 1, indexSize, indexList);
				break;
			case scene::EPT_LINE_LOOP:
				glDrawElements(GL_LINE_LOOP, primitiveCount, indexSize, indexList);
				break;
			case scene::EPT_LINES:
				glDrawElements(GL_LINES, primitiveCount*2, indexSize, indexList);
				break;
			case scene::EPT_TRIANGLE_STRIP:
				glDrawElements(GL_TRIANGLE_STRIP, primitiveCount + 2, indexSize, indexList);
				break;
			case scene::EPT_TRIANGLE_FAN:
				glDrawElements(GL_TRIANGLE_FAN, primitiveCount + 2, indexSize, indexList);
				break;
			case scene::EPT_TRIANGLES:
				glDrawElements((LastMaterial.Wireframe) ? GL_LINES : (LastMaterial.PointCloud) ? GL_POINTS : GL_TRIANGLES, primitiveCount*3, indexSize, indexList);
				break;
			case scene::EPT_QUAD_STRIP:
// TODO ogl-es
//		glDrawElements(GL_QUAD_STRIP, primitiveCount*2+2, indexSize, indexList);
				break;
			case scene::EPT_QUADS:
// TODO ogl-es
//		glDrawElements(GL_QUADS, primitiveCount*4, indexSize, indexList);
				break;
			case scene::EPT_POLYGON:
// TODO ogl-es
//		glDrawElements(GL_POLYGON, primitiveCount, indexSize, indexList);
				break;
		}

		{
			if (vType == EVT_TANGENTS)
			{
				glDisableVertexAttribArray(EVA_TANGENT);
				glDisableVertexAttribArray(EVA_BINORMAL);
			}
			//if ((vType != EVT_STANDARD) || CurrentTexture[1])
			//{
			//	glDisableVertexAttribArray(EVA_TCOORD1);
			//}

#ifdef GL_OES_point_size_array
			if (FeatureAvailable[IRR_OES_point_size_array] && (Material.Thickness == 0.0f))
				glDisableClientState(GL_POINT_SIZE_ARRAY_OES);
#endif
			glDisableVertexAttribArray(EVA_POSITION);
			glDisableVertexAttribArray(EVA_NORMAL);
			glDisableVertexAttribArray(EVA_COLOR);
			glDisableVertexAttribArray(EVA_TCOORD0);
		}
		testGLError();
	}


	//! draws a 2d image, using a color and the alpha channel of the texture
	void COGLES2Driver::draw2DImage(const video::ITexture* texture,
			const core::position2d<s32>& pos,
			const core::rect<s32>& sourceRect,
			const core::rect<s32>* clipRect, SColor color,
			bool useAlphaChannelOfTexture)
	{
		if (!texture)
			return;

		if (!sourceRect.isValid())
			return;

		core::position2d<s32> targetPos(pos);
		core::position2d<s32> sourcePos(sourceRect.UpperLeftCorner);
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

			if (targetPos.X + sourceSize.Width > clipRect->LowerRightCorner.X)
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

			if (targetPos.Y + sourceSize.Height > clipRect->LowerRightCorner.Y)
			{
				sourceSize.Height -= (targetPos.Y + sourceSize.Height) - clipRect->LowerRightCorner.Y;
				if (sourceSize.Height <= 0)
					return;
			}
		}

		// clip these coordinates

		if (targetPos.X < 0)
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

		if (targetPos.Y < 0)
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

		// texcoords need to be flipped horizontally for RTTs
		const bool isRTT = texture->isRenderTarget();
		const core::dimension2d<u32>& ss = texture->getSize();
		const f32 invW = 1.f / static_cast<f32>(ss.Width);
		const f32 invH = 1.f / static_cast<f32>(ss.Height);
		const core::rect<f32> tcoords(
			sourcePos.X * invW,
			(isRTT ? (sourcePos.Y + sourceSize.Height) : sourcePos.Y) * invH,
			(sourcePos.X + sourceSize.Width) * invW,
			(isRTT ? sourcePos.Y : (sourcePos.Y + sourceSize.Height)) * invH);

		const core::rect<s32> poss(targetPos, sourceSize);

		disableTextures(1);
		if (!setActiveTexture(0, texture))
			return;
		setRenderStates2DMode(color.getAlpha() < 255, true, useAlphaChannelOfTexture);

		u16 indices[] = {0, 1, 2, 3};
		S3DVertex vertices[4];
		vertices[0] = S3DVertex((f32)poss.UpperLeftCorner.X, (f32)poss.UpperLeftCorner.Y, 0, 0, 0, 1, color, tcoords.UpperLeftCorner.X, tcoords.UpperLeftCorner.Y);
		vertices[1] = S3DVertex((f32)poss.LowerRightCorner.X, (f32)poss.UpperLeftCorner.Y, 0, 0, 0, 1, color, tcoords.LowerRightCorner.X, tcoords.UpperLeftCorner.Y);
		vertices[2] = S3DVertex((f32)poss.LowerRightCorner.X, (f32)poss.LowerRightCorner.Y, 0, 0, 0, 1, color, tcoords.LowerRightCorner.X, tcoords.LowerRightCorner.Y);
		vertices[3] = S3DVertex((f32)poss.UpperLeftCorner.X, (f32)poss.LowerRightCorner.Y, 0, 0, 0, 1, color, tcoords.UpperLeftCorner.X, tcoords.LowerRightCorner.Y);
		drawVertexPrimitiveList2d3d(vertices, 4, indices, 2, video::EVT_STANDARD, scene::EPT_TRIANGLE_FAN, EIT_16BIT, false);
	}


	void COGLES2Driver::draw2DImageBatch(const video::ITexture* texture,
			const core::array<core::position2d<s32> >& positions,
			const core::array<core::rect<s32> >& sourceRects,
			const core::rect<s32>* clipRect,
			SColor color, bool useAlphaChannelOfTexture)
	{
		if (!texture)
			return;

		if (!setActiveTexture(0, const_cast<video::ITexture*>(texture)))
			return;

		const irr::u32 drawCount = core::min_<u32>(positions.size(), sourceRects.size());

		core::array<S3DVertex> vtx(drawCount * 4);
		core::array<u16> indices(drawCount * 6);

		for (u32 i = 0; i < drawCount; i++)
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

			if (targetPos.X < 0)
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

			if (targetPos.Y < 0)
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

			setRenderStates2DMode(color.getAlpha() < 255, true, useAlphaChannelOfTexture);

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

			const u32 curPos = vtx.size() - 4;
			indices.push_back(0 + curPos);
			indices.push_back(1 + curPos);
			indices.push_back(2 + curPos);

			indices.push_back(0 + curPos);
			indices.push_back(2 + curPos);
			indices.push_back(3 + curPos);
		}

		if (vtx.size())
		{
			drawVertexPrimitiveList2d3d(vtx.pointer(), vtx.size(),
				indices.pointer(), indices.size() / 3,
				EVT_STANDARD, scene::EPT_TRIANGLES,
				EIT_16BIT, false);
		}
	}


	//! The same, but with a four element array of colors, one for each vertex
	void COGLES2Driver::draw2DImage(const video::ITexture* texture,
			const core::rect<s32>& destRect,
			const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect,
			const video::SColor* const colors, bool useAlphaChannelOfTexture)
	{
		if (!texture)
			return;

		// texcoords need to be flipped horizontally for RTTs
		const bool isRTT = texture->isRenderTarget();
		const core::dimension2du& ss = texture->getSize();
		const f32 invW = 1.f / static_cast<f32>(ss.Width);
		const f32 invH = 1.f / static_cast<f32>(ss.Height);
		const core::rect<f32> tcoords(
			sourceRect.UpperLeftCorner.X * invW,
			(isRTT ? sourceRect.LowerRightCorner.Y : sourceRect.UpperLeftCorner.Y) * invH,
			sourceRect.LowerRightCorner.X * invW,
			(isRTT ? sourceRect.UpperLeftCorner.Y : sourceRect.LowerRightCorner.Y) *invH);

		const video::SColor temp[4] =
		{
			0xFFFFFFFF,
			0xFFFFFFFF,
			0xFFFFFFFF,
			0xFFFFFFFF
		};

		const video::SColor* const useColor = colors ? colors : temp;

		disableTextures(1);
		setActiveTexture(0, texture);
		setRenderStates2DMode(useColor[0].getAlpha() < 255 || useColor[1].getAlpha() < 255 ||
							useColor[2].getAlpha() < 255 || useColor[3].getAlpha() < 255,
							true, useAlphaChannelOfTexture);

		if (clipRect)
		{
			if (!clipRect->isValid())
				return;

			glEnable(GL_SCISSOR_TEST);
			const core::dimension2d<u32>& renderTargetSize = getCurrentRenderTargetSize();
			glScissor(clipRect->UpperLeftCorner.X, renderTargetSize.Height - clipRect->LowerRightCorner.Y,
					clipRect->getWidth(), clipRect->getHeight());
		}

		u16 indices[] = {0, 1, 2, 3};
		S3DVertex vertices[4];
		vertices[0] = S3DVertex((f32)destRect.UpperLeftCorner.X, (f32)destRect.UpperLeftCorner.Y, 0, 0, 0, 1, useColor[0], tcoords.UpperLeftCorner.X, tcoords.UpperLeftCorner.Y);
		vertices[1] = S3DVertex((f32)destRect.LowerRightCorner.X, (f32)destRect.UpperLeftCorner.Y, 0, 0, 0, 1, useColor[3], tcoords.LowerRightCorner.X, tcoords.UpperLeftCorner.Y);
		vertices[2] = S3DVertex((f32)destRect.LowerRightCorner.X, (f32)destRect.LowerRightCorner.Y, 0, 0, 0, 1, useColor[2], tcoords.LowerRightCorner.X, tcoords.LowerRightCorner.Y);
		vertices[3] = S3DVertex((f32)destRect.UpperLeftCorner.X, (f32)destRect.LowerRightCorner.Y, 0, 0, 0, 1, useColor[1], tcoords.UpperLeftCorner.X, tcoords.LowerRightCorner.Y);
		drawVertexPrimitiveList2d3d(vertices, 4, indices, 2, video::EVT_STANDARD, scene::EPT_TRIANGLE_FAN, EIT_16BIT, false);

		if (clipRect)
			glDisable(GL_SCISSOR_TEST);
		testGLError();
	}


	//! draws a set of 2d images, using a color and the alpha channel
	void COGLES2Driver::draw2DImageBatch(const video::ITexture* texture,
			const core::position2d<s32>& pos,
			const core::array<core::rect<s32> >& sourceRects,
			const core::array<s32>& indices, s32 kerningWidth,
			const core::rect<s32>* clipRect, SColor color,
			bool useAlphaChannelOfTexture)
	{
		if (!texture)
			return;

		disableTextures(1);
		if (!setActiveTexture(0, texture))
			return;
		setRenderStates2DMode(color.getAlpha() < 255, true, useAlphaChannelOfTexture);

		if (clipRect)
		{
			if (!clipRect->isValid())
				return;

			glEnable(GL_SCISSOR_TEST);
			const core::dimension2d<u32>& renderTargetSize = getCurrentRenderTargetSize();
			glScissor(clipRect->UpperLeftCorner.X, renderTargetSize.Height - clipRect->LowerRightCorner.Y,
					clipRect->getWidth(), clipRect->getHeight());
		}

		const core::dimension2du& ss = texture->getSize();
		core::position2d<s32> targetPos(pos);
		// texcoords need to be flipped horizontally for RTTs
		const bool isRTT = texture->isRenderTarget();
		const f32 invW = 1.f / static_cast<f32>(ss.Width);
		const f32 invH = 1.f / static_cast<f32>(ss.Height);

		core::array<S3DVertex> vertices;
		core::array<u16> quadIndices;
		vertices.reallocate(indices.size()*4);
		quadIndices.reallocate(indices.size()*3);
		for (u32 i = 0; i < indices.size(); ++i)
		{
			const s32 currentIndex = indices[i];
			if (!sourceRects[currentIndex].isValid())
				break;

			const core::rect<f32> tcoords(
				sourceRects[currentIndex].UpperLeftCorner.X * invW,
				(isRTT ? sourceRects[currentIndex].LowerRightCorner.Y : sourceRects[currentIndex].UpperLeftCorner.Y) * invH,
				sourceRects[currentIndex].LowerRightCorner.X * invW,
				(isRTT ? sourceRects[currentIndex].UpperLeftCorner.Y : sourceRects[currentIndex].LowerRightCorner.Y) * invH);

			const core::rect<s32> poss(targetPos, sourceRects[currentIndex].getSize());

			const u32 vstart = vertices.size();
			vertices.push_back(S3DVertex((f32)poss.UpperLeftCorner.X, (f32)poss.UpperLeftCorner.Y, 0, 0, 0, 1, color, tcoords.UpperLeftCorner.X, tcoords.UpperLeftCorner.Y));
			vertices.push_back(S3DVertex((f32)poss.LowerRightCorner.X, (f32)poss.UpperLeftCorner.Y, 0, 0, 0, 1, color, tcoords.LowerRightCorner.X, tcoords.UpperLeftCorner.Y));
			vertices.push_back(S3DVertex((f32)poss.LowerRightCorner.X, (f32)poss.LowerRightCorner.Y, 0, 0, 0, 1, color, tcoords.LowerRightCorner.X, tcoords.LowerRightCorner.Y));
			vertices.push_back(S3DVertex((f32)poss.UpperLeftCorner.X, (f32)poss.LowerRightCorner.Y, 0, 0, 0, 1, color, tcoords.UpperLeftCorner.X, tcoords.LowerRightCorner.Y));
			quadIndices.push_back(vstart);
			quadIndices.push_back(vstart+1);
			quadIndices.push_back(vstart+2);
			quadIndices.push_back(vstart);
			quadIndices.push_back(vstart+2);
			quadIndices.push_back(vstart+3);

			targetPos.X += sourceRects[currentIndex].getWidth();
		}
		if (vertices.size())
			drawVertexPrimitiveList2d3d(vertices.pointer(), vertices.size(),
					quadIndices.pointer(), vertices.size()/2,
					video::EVT_STANDARD, scene::EPT_TRIANGLES,
					EIT_16BIT, false);
		if (clipRect)
			glDisable(GL_SCISSOR_TEST);
		testGLError();
	}


	//! draw a 2d rectangle
	void COGLES2Driver::draw2DRectangle(SColor color,
			const core::rect<s32>& position,
			const core::rect<s32>* clip)
	{
		disableTextures();
		setRenderStates2DMode(color.getAlpha() < 255, false, false);

		core::rect<s32> pos = position;

		if (clip)
			pos.clipAgainst(*clip);

		if (!pos.isValid())
			return;

		u16 indices[] = {0, 1, 2, 3};
		S3DVertex vertices[4];
		vertices[0] = S3DVertex((f32)pos.UpperLeftCorner.X, (f32)pos.UpperLeftCorner.Y, 0, 0, 0, 1, color, 0, 0);
		vertices[1] = S3DVertex((f32)pos.LowerRightCorner.X, (f32)pos.UpperLeftCorner.Y, 0, 0, 0, 1, color, 0, 0);
		vertices[2] = S3DVertex((f32)pos.LowerRightCorner.X, (f32)pos.LowerRightCorner.Y, 0, 0, 0, 1, color, 0, 0);
		vertices[3] = S3DVertex((f32)pos.UpperLeftCorner.X, (f32)pos.LowerRightCorner.Y, 0, 0, 0, 1, color, 0, 0);
		drawVertexPrimitiveList2d3d(vertices, 4, indices, 2, video::EVT_STANDARD, scene::EPT_TRIANGLE_FAN, EIT_16BIT, false);
	}


	//! draw an 2d rectangle
	void COGLES2Driver::draw2DRectangle(const core::rect<s32>& position,
			SColor colorLeftUp, SColor colorRightUp,
			SColor colorLeftDown, SColor colorRightDown,
			const core::rect<s32>* clip)
	{
		core::rect<s32> pos = position;

		if (clip)
			pos.clipAgainst(*clip);

		if (!pos.isValid())
			return;

		disableTextures();

		setRenderStates2DMode(colorLeftUp.getAlpha() < 255 ||
				colorRightUp.getAlpha() < 255 ||
				colorLeftDown.getAlpha() < 255 ||
				colorRightDown.getAlpha() < 255, false, false);

		u16 indices[] = {0, 1, 2, 3};
		S3DVertex vertices[4];
		vertices[0] = S3DVertex((f32)pos.UpperLeftCorner.X, (f32)pos.UpperLeftCorner.Y, 0, 0, 0, 1, colorLeftUp, 0, 0);
		vertices[1] = S3DVertex((f32)pos.LowerRightCorner.X, (f32)pos.UpperLeftCorner.Y, 0, 0, 0, 1, colorRightUp, 0, 0);
		vertices[2] = S3DVertex((f32)pos.LowerRightCorner.X, (f32)pos.LowerRightCorner.Y, 0, 0, 0, 1, colorRightDown, 0, 0);
		vertices[3] = S3DVertex((f32)pos.UpperLeftCorner.X, (f32)pos.LowerRightCorner.Y, 0, 0, 0, 1, colorLeftDown, 0, 0);
		drawVertexPrimitiveList2d3d(vertices, 4, indices, 2, video::EVT_STANDARD, scene::EPT_TRIANGLE_FAN, EIT_16BIT, false);
	}


	//! Draws a 2d line.
	void COGLES2Driver::draw2DLine(const core::position2d<s32>& start,
			const core::position2d<s32>& end, SColor color)
	{
		disableTextures();
		setRenderStates2DMode(color.getAlpha() < 255, false, false);

		u16 indices[] = {0, 1};
		S3DVertex vertices[2];
		vertices[0] = S3DVertex((f32)start.X, (f32)start.Y, 0, 0, 0, 1, color, 0, 0);
		vertices[1] = S3DVertex((f32)end.X, (f32)end.Y, 0, 0, 0, 1, color, 1, 1);
		drawVertexPrimitiveList2d3d(vertices, 2, indices, 1, video::EVT_STANDARD, scene::EPT_LINES, EIT_16BIT, false);
	}


	//! Draws a pixel
	void COGLES2Driver::drawPixel(u32 x, u32 y, const SColor &color)
	{
		const core::dimension2d<u32>& renderTargetSize = getCurrentRenderTargetSize();
		if (x > (u32)renderTargetSize.Width || y > (u32)renderTargetSize.Height)
			return;

		disableTextures();
		setRenderStates2DMode(color.getAlpha() < 255, false, false);

		u16 indices[] = {0};
		S3DVertex vertices[1];
		vertices[0] = S3DVertex((f32)x, (f32)y, 0, 0, 0, 1, color, 0, 0);
		drawVertexPrimitiveList2d3d(vertices, 1, indices, 1, video::EVT_STANDARD, scene::EPT_POINTS, EIT_16BIT, false);
	}


	bool COGLES2Driver::setActiveTexture(u32 stage, const video::ITexture* texture)
	{
		if (stage >= MaxSupportedTextures)
			return false;

		if (CurrentTexture[stage]==texture)
			return true;

		CurrentTexture[stage] = texture;

		if (!texture)
			return true;
		else if (texture->getDriverType() != EDT_OGLES2)
		{
			CurrentTexture[stage] = 0;
			os::Printer::log("Fatal Error: Tried to set a texture not owned by this driver.", ELL_ERROR);
			return false;
		}

		return true;
	}


	bool COGLES2Driver::isActiveTexture(u32 stage)
	{
		return (CurrentTexture[stage]) ? true : false;
	}


	//! disables all textures beginning with the optional fromStage parameter.
	bool COGLES2Driver::disableTextures(u32 fromStage)
	{
		bool result = true;
		for (u32 i = fromStage; i < MaxTextureUnits; ++i)
			result &= setActiveTexture(i, 0);
		return result;
	}


	//! creates a matrix in supplied GLfloat array to pass to OGLES1
	inline void COGLES2Driver::createGLMatrix(float gl_matrix[16], const core::matrix4& m)
	{
		memcpy(gl_matrix, m.pointer(), 16 * sizeof(f32));
	}


	//! creates a opengltexturematrix from a D3D style texture matrix
	inline void COGLES2Driver::createGLTextureMatrix(float *o, const core::matrix4& m)
	{
		o[0] = m[0];
		o[1] = m[1];
		o[2] = 0.f;
		o[3] = 0.f;

		o[4] = m[4];
		o[5] = m[5];
		o[6] = 0.f;
		o[7] = 0.f;

		o[8] = 0.f;
		o[9] = 0.f;
		o[10] = 1.f;
		o[11] = 0.f;

		o[12] = m[8];
		o[13] = m[9];
		o[14] = 0.f;
		o[15] = 1.f;
	}


	//! returns a device dependent texture from a software surface (IImage)
	video::ITexture* COGLES2Driver::createDeviceDependentTexture(IImage* surface, const io::path& name, void* mipmapData)
	{
		return new COGLES2Texture(surface, name, mipmapData, this);
	}


	//! Sets a material.
	void COGLES2Driver::setMaterial(const SMaterial& material)
	{
		Material = material;
		OverrideMaterial.apply(Material);

		for (u32 i = 0; i < MaxTextureUnits; ++i)
			setActiveTexture(i, material.getTexture(i));
	}

	//! prints error if an error happened.
	bool COGLES2Driver::testGLError()
	{
#ifdef _DEBUG
		GLenum g = glGetError();
		switch (g)
		{
			case GL_NO_ERROR:
				return false;
			case GL_INVALID_ENUM:
				os::Printer::log("GL_INVALID_ENUM", ELL_ERROR);
				break;
			case GL_INVALID_VALUE:
				os::Printer::log("GL_INVALID_VALUE", ELL_ERROR);
				break;
			case GL_INVALID_OPERATION:
				os::Printer::log("GL_INVALID_OPERATION", ELL_ERROR);
				break;
			case GL_OUT_OF_MEMORY:
				os::Printer::log("GL_OUT_OF_MEMORY", ELL_ERROR);
				break;
		};
		return true;
#else
		return false;
#endif
	}

	void COGLES2Driver::setRenderStates3DMode()
	{
		if (useCoreContext)
			return;

		if (CurrentRenderMode != ERM_3D)
		{
			// Reset Texture Stages
			BridgeCalls->setBlend(false);
			BridgeCalls->setBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			ResetRenderStates = true;
		}

		if (ResetRenderStates || LastMaterial != Material)
		{
			// unset old material

			// unset last 3d material
			if (CurrentRenderMode == ERM_2D)
				MaterialRenderer2D->OnUnsetMaterial();
			else if (LastMaterial.MaterialType != Material.MaterialType &&
					static_cast<u32>(LastMaterial.MaterialType) < MaterialRenderers.size())
				MaterialRenderers[LastMaterial.MaterialType].Renderer->OnUnsetMaterial();

			// set new material.
			if (static_cast<u32>(Material.MaterialType) < MaterialRenderers.size())
				MaterialRenderers[Material.MaterialType].Renderer->OnSetMaterial(
					Material, LastMaterial, ResetRenderStates, this);

			LastMaterial = Material;
			ResetRenderStates = false;
		}

		if (static_cast<u32>(Material.MaterialType) < MaterialRenderers.size())
			MaterialRenderers[Material.MaterialType].Renderer->OnRender(this, video::EVT_STANDARD);

		CurrentRenderMode = ERM_3D;
	}

	//! Can be called by an IMaterialRenderer to make its work easier.
	void COGLES2Driver::setBasicRenderStates(const SMaterial& material, const SMaterial& lastmaterial, bool resetAllRenderStates)
	{
		if (useCoreContext)
			return;

		// ZBuffer
		if (resetAllRenderStates || lastmaterial.ZBuffer != material.ZBuffer)
		{
			switch (material.ZBuffer)
			{
				case ECFN_NEVER: // it will be ECFN_DISABLED after merge
					BridgeCalls->setDepthTest(false);
					break;
				case ECFN_LESSEQUAL:
					BridgeCalls->setDepthTest(true);
					BridgeCalls->setDepthFunc(GL_LEQUAL);
					break;
				case ECFN_EQUAL:
					BridgeCalls->setDepthTest(true);
					BridgeCalls->setDepthFunc(GL_EQUAL);
					break;
				case ECFN_LESS:
					BridgeCalls->setDepthTest(true);
					BridgeCalls->setDepthFunc(GL_LESS);
					break;
				case ECFN_NOTEQUAL:
					BridgeCalls->setDepthTest(true);
					BridgeCalls->setDepthFunc(GL_NOTEQUAL);
					break;
				case ECFN_GREATEREQUAL:
					BridgeCalls->setDepthTest(true);
					BridgeCalls->setDepthFunc(GL_GEQUAL);
					break;
				case ECFN_GREATER:
					BridgeCalls->setDepthTest(true);
					BridgeCalls->setDepthFunc(GL_GREATER);
					break;
				case ECFN_ALWAYS:
					BridgeCalls->setDepthTest(true);
					BridgeCalls->setDepthFunc(GL_ALWAYS);
					break;
				/*case ECFN_NEVER:
					BridgeCalls->setDepthTest(true);
					BridgeCalls->setDepthFunc(GL_NEVER);
					break;*/
			}
		}

		// ZWrite
	//	if (resetAllRenderStates || lastmaterial.ZWriteEnable != material.ZWriteEnable)
		{
			if (material.ZWriteEnable && (AllowZWriteOnTransparent || !material.isTransparent()))
				BridgeCalls->setDepthMask(true);
			else
				BridgeCalls->setDepthMask(false);
		}

		// Back face culling
		if (resetAllRenderStates || (lastmaterial.FrontfaceCulling != material.FrontfaceCulling) || (lastmaterial.BackfaceCulling != material.BackfaceCulling))
		{
			if ((material.FrontfaceCulling) && (material.BackfaceCulling))
			{
				BridgeCalls->setCullFaceFunc(GL_FRONT_AND_BACK);
				BridgeCalls->setCullFace(true);
			}
			else
			if (material.BackfaceCulling)
			{
				BridgeCalls->setCullFaceFunc(GL_BACK);
				BridgeCalls->setCullFace(true);
			}
			else
			if (material.FrontfaceCulling)
			{
				BridgeCalls->setCullFaceFunc(GL_FRONT);
				BridgeCalls->setCullFace(true);
			}
			else
				BridgeCalls->setCullFace(false);
		}

		// Color Mask
		if (resetAllRenderStates || lastmaterial.ColorMask != material.ColorMask)
		{
			glColorMask(
				(material.ColorMask & ECP_RED)?GL_TRUE:GL_FALSE,
				(material.ColorMask & ECP_GREEN)?GL_TRUE:GL_FALSE,
				(material.ColorMask & ECP_BLUE)?GL_TRUE:GL_FALSE,
				(material.ColorMask & ECP_ALPHA)?GL_TRUE:GL_FALSE);
		}

		// Blend operation
		if (resetAllRenderStates|| lastmaterial.BlendOperation != material.BlendOperation)
		{
			if (material.BlendOperation==EBO_NONE)
				BridgeCalls->setBlend(false);
			else
			{
				BridgeCalls->setBlend(true);

				switch (material.BlendOperation)
				{
				case EBO_ADD:
					glBlendEquation(GL_FUNC_ADD);
					break;
				case EBO_SUBTRACT:
					glBlendEquation(GL_FUNC_SUBTRACT);
					break;
				case EBO_REVSUBTRACT:
					glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
					break;
				default:
					break;
				}
			}
		}

		// Anti aliasing
		if (resetAllRenderStates || lastmaterial.AntiAliasing != material.AntiAliasing)
		{
			if (material.AntiAliasing & EAAM_ALPHA_TO_COVERAGE)
				glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			else if (lastmaterial.AntiAliasing & EAAM_ALPHA_TO_COVERAGE)
				glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		}

		// Texture parameters
		setTextureRenderStates(material, resetAllRenderStates);
	}

	//! Compare in SMaterial doesn't check texture parameters, so we should call this on each OnRender call.
	void COGLES2Driver::setTextureRenderStates(const SMaterial& material, bool resetAllRenderstates)
	{
		if (useCoreContext)
			return;

		// Set textures to TU/TIU and apply filters to them

		for (s32 i = MaxTextureUnits-1; i>= 0; --i)
		{
			const COGLES2Texture* tmpTexture = static_cast<const COGLES2Texture*>(CurrentTexture[i]);

			if (CurrentTexture[i])
				BridgeCalls->setTexture(i);
			else
				continue;

			// This code causes issues on some devices with legacy pipeline
			// and also mipmaps should be handled in STK texture manager,
			// so just disable this part of code
			continue;

			if(resetAllRenderstates)
				tmpTexture->getStatesCache().IsCached = false;

			if(!tmpTexture->getStatesCache().IsCached || material.TextureLayer[i].BilinearFilter != tmpTexture->getStatesCache().BilinearFilter ||
				material.TextureLayer[i].TrilinearFilter != tmpTexture->getStatesCache().TrilinearFilter)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
					(material.TextureLayer[i].BilinearFilter || material.TextureLayer[i].TrilinearFilter) ? GL_LINEAR : GL_NEAREST);

				tmpTexture->getStatesCache().BilinearFilter = material.TextureLayer[i].BilinearFilter;
				tmpTexture->getStatesCache().TrilinearFilter = material.TextureLayer[i].TrilinearFilter;
			}

			if (material.UseMipMaps && CurrentTexture[i]->hasMipMaps())
			{
				if(!tmpTexture->getStatesCache().IsCached || material.TextureLayer[i].BilinearFilter != tmpTexture->getStatesCache().BilinearFilter ||
					material.TextureLayer[i].TrilinearFilter != tmpTexture->getStatesCache().TrilinearFilter || !tmpTexture->getStatesCache().MipMapStatus)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
						material.TextureLayer[i].TrilinearFilter ? GL_LINEAR_MIPMAP_LINEAR :
						material.TextureLayer[i].BilinearFilter ? GL_LINEAR_MIPMAP_NEAREST :
						GL_NEAREST_MIPMAP_NEAREST);

					tmpTexture->getStatesCache().BilinearFilter = material.TextureLayer[i].BilinearFilter;
					tmpTexture->getStatesCache().TrilinearFilter = material.TextureLayer[i].TrilinearFilter;
					tmpTexture->getStatesCache().MipMapStatus = true;
				}
			}
			else
			{
				if(!tmpTexture->getStatesCache().IsCached || material.TextureLayer[i].BilinearFilter != tmpTexture->getStatesCache().BilinearFilter ||
					material.TextureLayer[i].TrilinearFilter != tmpTexture->getStatesCache().TrilinearFilter || tmpTexture->getStatesCache().MipMapStatus)
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
						(material.TextureLayer[i].BilinearFilter || material.TextureLayer[i].TrilinearFilter) ? GL_LINEAR : GL_NEAREST);

					tmpTexture->getStatesCache().BilinearFilter = material.TextureLayer[i].BilinearFilter;
					tmpTexture->getStatesCache().TrilinearFilter = material.TextureLayer[i].TrilinearFilter;
					tmpTexture->getStatesCache().MipMapStatus = false;
				}
			}

	#ifdef GL_EXT_texture_filter_anisotropic
			if (FeatureAvailable[IRR_EXT_texture_filter_anisotropic] &&
				(!tmpTexture->getStatesCache().IsCached || material.TextureLayer[i].AnisotropicFilter != tmpTexture->getStatesCache().AnisotropicFilter))
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT,
					material.TextureLayer[i].AnisotropicFilter>1 ? core::min_(MaxAnisotropy, material.TextureLayer[i].AnisotropicFilter) : 1);

				tmpTexture->getStatesCache().AnisotropicFilter = material.TextureLayer[i].AnisotropicFilter;
			}
	#endif

			if(!tmpTexture->getStatesCache().IsCached || material.TextureLayer[i].TextureWrapU != tmpTexture->getStatesCache().WrapU)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, getTextureWrapMode(material.TextureLayer[i].TextureWrapU));
				tmpTexture->getStatesCache().WrapU = material.TextureLayer[i].TextureWrapU;
			}

			if(!tmpTexture->getStatesCache().IsCached || material.TextureLayer[i].TextureWrapV != tmpTexture->getStatesCache().WrapV)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, getTextureWrapMode(material.TextureLayer[i].TextureWrapV));
				tmpTexture->getStatesCache().WrapV = material.TextureLayer[i].TextureWrapV;
			}

			tmpTexture->getStatesCache().IsCached = true;
		}
	}


	// Get OpenGL ES2.0 texture wrap mode from Irrlicht wrap mode.
	GLint COGLES2Driver::getTextureWrapMode(u8 clamp) const
	{
		switch (clamp)
		{
			case ETC_CLAMP:
			case ETC_CLAMP_TO_EDGE:
			case ETC_CLAMP_TO_BORDER:
				return GL_CLAMP_TO_EDGE;
			case ETC_MIRROR:
				return GL_REPEAT;
			default:
				return GL_REPEAT;
		}
	}


	//! sets the needed renderstates
	void COGLES2Driver::setRenderStates2DMode(bool alpha, bool texture, bool alphaChannel)
	{
		if (useCoreContext)
			return;

		if (CurrentRenderMode != ERM_2D)
		{
			// unset last 3d material
			if (CurrentRenderMode == ERM_3D)
			{
				if (static_cast<u32>(LastMaterial.MaterialType) < MaterialRenderers.size())
					MaterialRenderers[LastMaterial.MaterialType].Renderer->OnUnsetMaterial();
			}

			CurrentRenderMode = ERM_2D;
		}

		if (!OverrideMaterial2DEnabled)
			Material = InitMaterial2D;

		if (OverrideMaterial2DEnabled)
		{
			OverrideMaterial2D.Lighting=false;
			OverrideMaterial2D.ZWriteEnable=false;
			OverrideMaterial2D.ZBuffer=ECFN_NEVER; // it will be ECFN_DISABLED after merge
			OverrideMaterial2D.Lighting=false;

			Material = OverrideMaterial2D;
		}

		if (texture)
			MaterialRenderer2D->setTexture(CurrentTexture[0]);
		else
			MaterialRenderer2D->setTexture(0);

		MaterialRenderer2D->OnSetMaterial(Material, LastMaterial, true, 0);
		LastMaterial = Material;

		// no alphaChannel without texture
		alphaChannel &= texture;

		if (alphaChannel || alpha)
		{
			BridgeCalls->setBlend(true);
			BridgeCalls->setBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
		else
			BridgeCalls->setBlend(false);

		MaterialRenderer2D->OnRender(this, video::EVT_STANDARD);
	}


	//! \return Returns the name of the video driver.
	const wchar_t* COGLES2Driver::getName() const
	{
		return Name.c_str();
	}


	//! deletes all dynamic lights there are
	void COGLES2Driver::deleteAllDynamicLights()
	{
		RequestedLights.clear();
		CNullDriver::deleteAllDynamicLights();
	}


	//! adds a dynamic light
	s32 COGLES2Driver::addDynamicLight(const SLight& light)
	{
		CNullDriver::addDynamicLight(light);

		RequestedLights.push_back(RequestedLight(light));

		u32 newLightIndex = RequestedLights.size() - 1;

		return (s32)newLightIndex;
	}

	//! Turns a dynamic light on or off
	//! \param lightIndex: the index returned by addDynamicLight
	//! \param turnOn: true to turn the light on, false to turn it off
	void COGLES2Driver::turnLightOn(s32 lightIndex, bool turnOn)
	{
		if (lightIndex < 0 || lightIndex >= (s32)RequestedLights.size())
			return;

		RequestedLight & requestedLight = RequestedLights[lightIndex];
		requestedLight.DesireToBeOn = turnOn;
	}


	//! returns the maximal amount of dynamic lights the device can handle
	u32 COGLES2Driver::getMaximalDynamicLightAmount() const
	{
		return 8;
	}


	//! Sets the dynamic ambient light color.
	void COGLES2Driver::setAmbientLight(const SColorf& color)
	{
		AmbientLight = color;
	}

	//! returns the dynamic ambient light color.
	const SColorf& COGLES2Driver::getAmbientLight() const
	{
		return AmbientLight;
	}

	// this code was sent in by Oliver Klems, thank you
	void COGLES2Driver::setViewPort(const core::rect<s32>& area)
	{
		core::rect<s32> vp = area;
		core::rect<s32> rendert(0, 0, getCurrentRenderTargetSize().Width, getCurrentRenderTargetSize().Height);
		vp.clipAgainst(rendert);

		if (vp.getHeight() > 0 && vp.getWidth() > 0)
			BridgeCalls->setViewport(core::rect<s32>(vp.UpperLeftCorner.X, getCurrentRenderTargetSize().Height - vp.UpperLeftCorner.Y - vp.getHeight(), vp.getWidth(), vp.getHeight()));

		ViewPort = vp;
		testGLError();
	}


	//! Draws a shadow volume into the stencil buffer.
	void COGLES2Driver::drawStencilShadowVolume(const core::vector3df* triangles, s32 count, bool zfail)
	{
		if (!StencilBuffer || !count)
			return;

		// unset last 3d material
		if (CurrentRenderMode == ERM_3D &&
			static_cast<u32>(Material.MaterialType) < MaterialRenderers.size())
		{
			MaterialRenderers[Material.MaterialType].Renderer->OnUnsetMaterial();
			ResetRenderStates = true;
		}

		// store current OGLES state
		const GLboolean cullFaceEnabled = glIsEnabled(GL_CULL_FACE);
		GLint cullFaceMode;
		glGetIntegerv(GL_CULL_FACE_MODE, &cullFaceMode);
		GLint depthFunc;
		glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
		GLboolean depthMask;
		glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);

		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE); // no depth buffer writing
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // no color buffer drawing
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0.0f, 1.0f);

		glEnableVertexAttribArray(EVA_POSITION);

		glVertexAttribPointer(EVA_POSITION, 3, GL_FLOAT, false, sizeof(core::vector3df), &triangles[0]);
		glStencilMask(~0);
		glStencilFunc(GL_ALWAYS, 0, ~0);

		GLenum decr = GL_DECR;
		GLenum incr = GL_INCR;
#if defined(GL_OES_stencil_wrap)
		if (FeatureAvailable[IRR_OES_stencil_wrap])
		{
			decr = GL_DECR_WRAP_OES;
			incr = GL_INCR_WRAP_OES;
		}
#endif
		glEnable(GL_CULL_FACE);
		if (!zfail)
		{
			// ZPASS Method

			glCullFace(GL_BACK);
			glStencilOp(GL_KEEP, GL_KEEP, incr);
			glDrawArrays(GL_TRIANGLES, 0, count);

			glCullFace(GL_FRONT);
			glStencilOp(GL_KEEP, GL_KEEP, decr);
			glDrawArrays(GL_TRIANGLES, 0, count);
		}
		else
		{
			// ZFAIL Method

			glStencilOp(GL_KEEP, incr, GL_KEEP);
			glCullFace(GL_FRONT);
			glDrawArrays(GL_TRIANGLES, 0, count);

			glStencilOp(GL_KEEP, decr, GL_KEEP);
			glCullFace(GL_BACK);
			glDrawArrays(GL_TRIANGLES, 0, count);
		}

		glDisableVertexAttribArray(EVA_POSITION);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDisable(GL_STENCIL_TEST);
		if (cullFaceEnabled)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
		glCullFace(cullFaceMode);
		glDepthFunc(depthFunc);
		glDepthMask(depthMask);
		testGLError();
	}


	void COGLES2Driver::drawStencilShadow(bool clearStencilBuffer,
			video::SColor leftUpEdge, video::SColor rightUpEdge,
			video::SColor leftDownEdge, video::SColor rightDownEdge)
	{
		if (!StencilBuffer)
			return;

		disableTextures();

		// store attributes
		GLboolean depthMask;
		glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
//			GLint shadeModel;
		//TODO : OpenGL ES 2.0 Port glGetIntegerv
		//glGetIntegerv(GL_SHADE_MODEL, &shadeModel);

		glDepthMask(GL_FALSE);

		//TODO : OpenGL ES 2.0 Port glShadeModel
		//glShadeModel(GL_FLAT);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_NOTEQUAL, 0, ~0);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		// draw a shadow rectangle covering the entire screen using stencil buffer
		//Wrapper->glMatrixMode(GL_MODELVIEW);
		//TODO : OpenGL ES 2.0 Port glPushMatrix
		//glPushMatrix();
		//Wrapper->glLoadIdentity();
		//Wrapper->glMatrixMode(GL_PROJECTION);
		//TODO : OpenGL ES 2.0 Port glPushMatrix
		//glPushMatrix();
		//Wrapper->glLoadIdentity();

		u16 indices[] = {0, 1, 2, 3};
		S3DVertex vertices[4];
		vertices[0] = S3DVertex(-1.f, -1.f, 0.9f, 0, 0, 1, leftDownEdge, 0, 0);
		vertices[1] = S3DVertex(-1.f, 1.f, 0.9f, 0, 0, 1, leftUpEdge, 0, 0);
		vertices[2] = S3DVertex(1.f, 1.f, 0.9f, 0, 0, 1, rightUpEdge, 0, 0);
		vertices[3] = S3DVertex(1.f, -1.f, 0.9f, 0, 0, 1, rightDownEdge, 0, 0);
		drawVertexPrimitiveList2d3d(vertices, 4, indices, 2, video::EVT_STANDARD, scene::EPT_TRIANGLE_FAN, EIT_16BIT, false);

		if (clearStencilBuffer)
			glClear(GL_STENCIL_BUFFER_BIT);

		// restore settings
		//TODO : OpenGL ES 2.0 Port glPopMatrix
		//glPopMatrix();
		//Wrapper->glMatrixMode(GL_MODELVIEW);
		//TODO : OpenGL ES 2.0 Port glPopMatrix
		//glPopMatrix();
		glDisable(GL_STENCIL_TEST);

		glDepthMask(depthMask);
		//TODO : OpenGL ES 2.0 Port glShadeModel
		//glShadeModel(shadeModel);
	}


	//! Draws a 3d line.
	void COGLES2Driver::draw3DLine(const core::vector3df& start,
			const core::vector3df& end, SColor color)
	{
		setRenderStates3DMode();

		u16 indices[] = {0, 1};
		S3DVertex vertices[2];
		vertices[0] = S3DVertex(start.X, start.Y, start.Z, 0, 0, 1, color, 0, 0);
		vertices[1] = S3DVertex(end.X, end.Y, end.Z, 0, 0, 1, color, 0, 0);
		drawVertexPrimitiveList2d3d(vertices, 2, indices, 1, video::EVT_STANDARD, scene::EPT_LINES);
	}


	//! Only used by the internal engine. Used to notify the driver that
	//! the window was resized.
	void COGLES2Driver::OnResize(const core::dimension2d<u32>& size)
	{
		CNullDriver::OnResize(size);
		BridgeCalls->setViewport(core::rect<s32>(0, 0, size.Width, size.Height));
		testGLError();
	}


	//! Returns type of video driver
	E_DRIVER_TYPE COGLES2Driver::getDriverType() const
	{
		return EDT_OGLES2;
	}


	//! returns color format
	ECOLOR_FORMAT COGLES2Driver::getColorFormat() const
	{
		return ColorFormat;
	}


	//! Get a vertex shader constant index.
	s32 COGLES2Driver::getVertexShaderConstantID(const c8* name)
	{
		return getPixelShaderConstantID(name);
	}

	//! Get a pixel shader constant index.
	s32 COGLES2Driver::getPixelShaderConstantID(const c8* name)
	{
		os::Printer::log("Error: Please call services->getPixelShaderConstantID(), not VideoDriver->getPixelShaderConstantID().");
		return -1;
	}

	//! Sets a vertex shader constant.
	void COGLES2Driver::setVertexShaderConstant(const f32* data, s32 startRegister, s32 constantAmount)
	{
		os::Printer::log("Error: Please call services->setVertexShaderConstant(), not VideoDriver->setPixelShaderConstant().");
	}

	//! Sets a pixel shader constant.
	void COGLES2Driver::setPixelShaderConstant(const f32* data, s32 startRegister, s32 constantAmount)
	{
		os::Printer::log("Error: Please call services->setPixelShaderConstant(), not VideoDriver->setPixelShaderConstant().");
	}

	//! Sets a constant for the vertex shader based on an index.
	bool COGLES2Driver::setVertexShaderConstant(s32 index, const f32* floats, int count)
	{
		//pass this along, as in GLSL the same routine is used for both vertex and fragment shaders
		return setPixelShaderConstant(index, floats, count);
	}

	//! Int interface for the above.
	bool COGLES2Driver::setVertexShaderConstant(s32 index, const s32* ints, int count)
	{
		return setPixelShaderConstant(index, ints, count);
	}

	//! Sets a constant for the pixel shader based on an index.
	bool COGLES2Driver::setPixelShaderConstant(s32 index, const f32* floats, int count)
	{
		os::Printer::log("Error: Please call services->setPixelShaderConstant(), not VideoDriver->setPixelShaderConstant().");
		return false;
	}

	//! Int interface for the above.
	bool COGLES2Driver::setPixelShaderConstant(s32 index, const s32* ints, int count)
	{
		os::Printer::log("Error: Please call services->setPixelShaderConstant(), not VideoDriver->setPixelShaderConstant().");
		return false;
	}


	//! Adds a new material renderer to the VideoDriver, using pixel and/or
	//! vertex shaders to render geometry.
	s32 COGLES2Driver::addShaderMaterial(const c8* vertexShaderProgram,
			const c8* pixelShaderProgram,
			IShaderConstantSetCallBack* callback,
			E_MATERIAL_TYPE baseMaterial, s32 userData)
	{
		os::Printer::log("No shader support.");
		return -1;
	}


	//! Adds a new material renderer to the VideoDriver, using GLSL to render geometry.
	s32 COGLES2Driver::addHighLevelShaderMaterial(
			const c8* vertexShaderProgram,
			const c8* vertexShaderEntryPointName,
			E_VERTEX_SHADER_TYPE vsCompileTarget,
			const c8* pixelShaderProgram,
			const c8* pixelShaderEntryPointName,
			E_PIXEL_SHADER_TYPE psCompileTarget,
			const c8* geometryShaderProgram,
			const c8* geometryShaderEntryPointName,
			E_GEOMETRY_SHADER_TYPE gsCompileTarget,
			scene::E_PRIMITIVE_TYPE inType,
			scene::E_PRIMITIVE_TYPE outType,
			u32 verticesOut,
			IShaderConstantSetCallBack* callback,
			E_MATERIAL_TYPE baseMaterial,
			s32 userData, E_GPU_SHADING_LANGUAGE shadingLang)
	{
		s32 nr = -1;
		COGLES2MaterialRenderer* r = new COGLES2MaterialRenderer(
			this, nr, vertexShaderProgram,
			pixelShaderProgram,
			callback, baseMaterial, userData);

		r->drop();
		return nr;
	}

	//! Returns a pointer to the IVideoDriver interface. (Implementation for
	//! IMaterialRendererServices)
	IVideoDriver* COGLES2Driver::getVideoDriver()
	{
		return this;
	}


	//! Returns pointer to the IGPUProgrammingServices interface.
	IGPUProgrammingServices* COGLES2Driver::getGPUProgrammingServices()
	{
		return this;
	}


	ITexture* COGLES2Driver::addRenderTargetTexture(const core::dimension2d<u32>& size,
					const io::path& name,
					const ECOLOR_FORMAT format,
					const bool useStencil)
	{
		//disable mip-mapping
		const bool generateMipLevels = getTextureCreationFlag(ETCF_CREATE_MIP_MAPS);
		setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, false);

		video::ITexture* rtt = 0;

		rtt = new COGLES2FBOTexture(size, name, this, format);
		if (rtt)
		{
			bool success = false;
			addTexture(rtt);

			ITexture* tex = createDepthTexture(rtt);
			if (tex)
			{
				success = static_cast<video::COGLES2FBODepthTexture*>(tex)->attach(rtt);
				if (!success)
				{
					removeDepthTexture(tex);
				}
				tex->drop();
			}
			rtt->drop();
			if (!success)
			{
				removeTexture(rtt);
				rtt=0;
			}
		}

		//restore mip-mapping
		setTextureCreationFlag(ETCF_CREATE_MIP_MAPS, generateMipLevels);

		return rtt;
	}


	//! Returns the maximum amount of primitives
	u32 COGLES2Driver::getMaximalPrimitiveCount() const
	{
		return 65535;
	}


	//! set or reset render target
	bool COGLES2Driver::setRenderTarget(video::ITexture* texture, bool clearBackBuffer,
			bool clearZBuffer, SColor color)
	{
		// check for right driver type

		if (texture && texture->getDriverType() != EDT_OGLES2)
		{
			os::Printer::log("Fatal Error: Tried to set a texture not owned by this driver.", ELL_ERROR);
			return false;
		}

		// check if we should set the previous RT back

		setActiveTexture(0, 0);
		ResetRenderStates = true;
		if (RenderTargetTexture != 0)
		{
			RenderTargetTexture->unbindRTT();
		}

		if (texture)
		{
			// we want to set a new target. so do this.
			BridgeCalls->setViewport(core::rect<s32>(0, 0, texture->getSize().Width, texture->getSize().Height));
			RenderTargetTexture = static_cast<COGLES2Texture*>(texture);
			RenderTargetTexture->bindRTT();
			CurrentRendertargetSize = texture->getSize();
		}
		else
		{
			BridgeCalls->setViewport(core::rect<s32>(0, 0, ScreenSize.Width, ScreenSize.Height));
			RenderTargetTexture = 0;
			CurrentRendertargetSize = core::dimension2d<u32>(0, 0);
		}

		GLbitfield mask = 0;
		if (clearBackBuffer)
		{
			const f32 inv = 1.0f / 255.0f;
			glClearColor(color.getRed() * inv, color.getGreen() * inv,
						color.getBlue() * inv, color.getAlpha() * inv);

			mask |= GL_COLOR_BUFFER_BIT;
		}
		if (clearZBuffer)
		{
			glDepthMask(GL_TRUE);
			LastMaterial.ZWriteEnable = true;
			mask |= GL_DEPTH_BUFFER_BIT;
		}

		glClear(mask);
		testGLError();

		return true;
	}


	// returns the current size of the screen or rendertarget
	const core::dimension2d<u32>& COGLES2Driver::getCurrentRenderTargetSize() const
	{
		if (CurrentRendertargetSize.Width == 0)
			return ScreenSize;
		else
			return CurrentRendertargetSize;
	}


	//! Clears the ZBuffer.
	void COGLES2Driver::clearZBuffer()
	{
		GLboolean enabled = GL_TRUE;
		glGetBooleanv(GL_DEPTH_WRITEMASK, &enabled);

		glDepthMask(GL_TRUE);
		glClear(GL_DEPTH_BUFFER_BIT);

		glDepthMask(enabled);
		testGLError();
	}


	//! Returns an image created from the last rendered frame.
	// We want to read the front buffer to get the latest render finished.
	// This is not possible under ogl-es, though, so one has to call this method
	// outside of the render loop only.
	IImage* COGLES2Driver::createScreenShot(video::ECOLOR_FORMAT format, video::E_RENDER_TARGET target)
	{
		if (target==video::ERT_MULTI_RENDER_TEXTURES || target==video::ERT_RENDER_TEXTURE || target==video::ERT_STEREO_BOTH_BUFFERS)
			return 0;

		GLint internalformat = GL_RGBA;
		GLint type = GL_UNSIGNED_BYTE;
		{
//			glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &internalformat);
//			glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &type);
			// there's a format we don't support ATM
			if (GL_UNSIGNED_SHORT_4_4_4_4 == type)
			{
				internalformat = GL_RGBA;
				type = GL_UNSIGNED_BYTE;
			}
		}

		IImage* newImage = 0;
		if (GL_RGBA == internalformat)
		{
			if (GL_UNSIGNED_BYTE == type)
				newImage = new CImage(ECF_A8R8G8B8, ScreenSize);
			else
				newImage = new CImage(ECF_A1R5G5B5, ScreenSize);
		}
		else
		{
			if (GL_UNSIGNED_BYTE == type)
				newImage = new CImage(ECF_R8G8B8, ScreenSize);
			else
				newImage = new CImage(ECF_R5G6B5, ScreenSize);
		}

		if (!newImage)
			return 0;

		u8* pixels = static_cast<u8*>(newImage->lock());
		if (!pixels)
		{
			newImage->unlock();
			newImage->drop();
			return 0;
		}

		glReadPixels(0, 0, ScreenSize.Width, ScreenSize.Height, internalformat, type, pixels);
		testGLError();

		// opengl images are horizontally flipped, so we have to fix that here.
		const s32 pitch = newImage->getPitch();
		u8* p2 = pixels + (ScreenSize.Height - 1) * pitch;
		u8* tmpBuffer = new u8[pitch];
		for (u32 i = 0; i < ScreenSize.Height; i += 2)
		{
			memcpy(tmpBuffer, pixels, pitch);
			memcpy(pixels, p2, pitch);
			memcpy(p2, tmpBuffer, pitch);
			pixels += pitch;
			p2 -= pitch;
		}
		delete [] tmpBuffer;

		newImage->unlock();

		if (testGLError())
		{
			newImage->drop();
			return 0;
		}
		testGLError();
		return newImage;
	}


	//! get depth texture for the given render target texture
	ITexture* COGLES2Driver::createDepthTexture(ITexture* texture, bool shared)
	{
		if ((texture->getDriverType() != EDT_OGLES2) || (!texture->isRenderTarget()))
			return 0;
		COGLES2Texture* tex = static_cast<COGLES2Texture*>(texture);

		if (!tex->isFrameBufferObject())
			return 0;

		if (shared)
		{
			for (u32 i = 0; i < DepthTextures.size(); ++i)
			{
				if (DepthTextures[i]->getSize() == texture->getSize())
				{
					DepthTextures[i]->grab();
					return DepthTextures[i];
				}
			}
			DepthTextures.push_back(new COGLES2FBODepthTexture(texture->getSize(), "depth1", this));
			return DepthTextures.getLast();
		}
		return (new COGLES2FBODepthTexture(texture->getSize(), "depth1", this));
	}


	void COGLES2Driver::removeDepthTexture(ITexture* texture)
	{
		for (u32 i = 0; i < DepthTextures.size(); ++i)
		{
			if (texture == DepthTextures[i])
			{
				DepthTextures.erase(i);
				return;
			}
		}
	}

	void COGLES2Driver::deleteFramebuffers(s32 n, const u32 *framebuffers)
	{
		glDeleteFramebuffers(n, framebuffers);
	}

	void COGLES2Driver::deleteRenderbuffers(s32 n, const u32 *renderbuffers)
	{
		glDeleteRenderbuffers(n, renderbuffers);
	}

	//! Set/unset a clipping plane.
	bool COGLES2Driver::setClipPlane(u32 index, const core::plane3df& plane, bool enable)
	{
		if (index >= UserClipPlane.size())
			UserClipPlane.push_back(SUserClipPlane());

		UserClipPlane[index].Plane = plane;
		UserClipPlane[index].Enabled = enable;
		return true;
	}

	//! Enable/disable a clipping plane.
	void COGLES2Driver::enableClipPlane(u32 index, bool enable)
	{
		UserClipPlane[index].Enabled = enable;
	}

	//! Get the ClipPlane Count
	u32 COGLES2Driver::getClipPlaneCount() const
	{
		return UserClipPlane.size();
	}

	const core::plane3df& COGLES2Driver::getClipPlane(irr::u32 index) const
	{
		if (index < UserClipPlane.size())
			return UserClipPlane[index].Plane;
		else
			return *((core::plane3df*)0);
	}

	core::dimension2du COGLES2Driver::getMaxTextureSize() const
	{
		return core::dimension2du(MaxTextureSize, MaxTextureSize);
	}

	GLenum COGLES2Driver::getGLBlend(E_BLEND_FACTOR factor) const
	{
		GLenum r = 0;
		switch (factor)
		{
			case EBF_ZERO:			r = GL_ZERO; break;
			case EBF_ONE:			r = GL_ONE; break;
			case EBF_DST_COLOR:		r = GL_DST_COLOR; break;
			case EBF_ONE_MINUS_DST_COLOR:	r = GL_ONE_MINUS_DST_COLOR; break;
			case EBF_SRC_COLOR:		r = GL_SRC_COLOR; break;
			case EBF_ONE_MINUS_SRC_COLOR:	r = GL_ONE_MINUS_SRC_COLOR; break;
			case EBF_SRC_ALPHA:		r = GL_SRC_ALPHA; break;
			case EBF_ONE_MINUS_SRC_ALPHA:	r = GL_ONE_MINUS_SRC_ALPHA; break;
			case EBF_DST_ALPHA:		r = GL_DST_ALPHA; break;
			case EBF_ONE_MINUS_DST_ALPHA:	r = GL_ONE_MINUS_DST_ALPHA; break;
			case EBF_SRC_ALPHA_SATURATE:	r = GL_SRC_ALPHA_SATURATE; break;
		}
		return r;
	}

	GLenum COGLES2Driver::getZBufferBits() const
	{
/*#if defined(GL_OES_depth24)
		if (Driver->queryOpenGLFeature(COGLES2ExtensionHandler::IRR_OES_depth24))
			InternalFormat = GL_DEPTH_COMPONENT24_OES;
		else
#endif
#if defined(GL_OES_depth32)
		if (Driver->queryOpenGLFeature(COGLES2ExtensionHandler::IRR_OES_depth32))
			InternalFormat = GL_DEPTH_COMPONENT32_OES;
		else
#endif*/

		GLenum bits = GL_DEPTH_COMPONENT16;//0;
		/*switch (Params.ZBufferBits)
		{
		case 16:
			bits = GL_DEPTH_COMPONENT16;
			break;
		case 24:
			bits = GL_DEPTH_COMPONENT24;
			break;
		case 32:
			bits = GL_DEPTH_COMPONENT32;
			break;
		default:
			bits = GL_DEPTH_COMPONENT;
			break;
		}*/
		return bits;
	}

	const SMaterial& COGLES2Driver::getCurrentMaterial() const
	{
		return Material;
	}

	COGLES2CallBridge* COGLES2Driver::getBridgeCalls() const
	{
		return BridgeCalls;
	}

	COGLES2CallBridge::COGLES2CallBridge(COGLES2Driver* driver) : Driver(driver),
		BlendSource(GL_ONE), BlendDestination(GL_ZERO), Blend(false),
		CullFaceMode(GL_BACK), CullFace(false),
		DepthFunc(GL_LESS), DepthMask(true), DepthTest(false),
		Program(0), ActiveTexture(GL_TEXTURE0), Viewport(core::rect<s32>(0, 0, 0, 0))
	{
		// Initial OpenGL values from specification.

		for (u32 i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
			Texture[i] = 0;

		glBlendFunc(GL_ONE, GL_ZERO);
		glDisable(GL_BLEND);

		glCullFace(GL_BACK);
		glDisable(GL_CULL_FACE);

		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
		glDisable(GL_DEPTH_TEST);
	}

	void COGLES2CallBridge::setBlendFunc(GLenum source, GLenum destination)
	{
		if(BlendSource != source || BlendDestination != destination)
		{
			glBlendFunc(source, destination);

			BlendSource = source;
			BlendDestination = destination;
		}
	}

	void COGLES2CallBridge::setBlend(bool enable)
	{
		if(Blend != enable)
		{
			if (enable)
				glEnable(GL_BLEND);
			else
				glDisable(GL_BLEND);

			Blend = enable;
		}
	}

	void COGLES2CallBridge::setCullFaceFunc(GLenum mode)
	{
		if(CullFaceMode != mode)
		{
			glCullFace(mode);

			CullFaceMode = mode;
		}
	}

	void COGLES2CallBridge::setCullFace(bool enable)
	{
		if(CullFace != enable)
		{
			if (enable)
				glEnable(GL_CULL_FACE);
			else
				glDisable(GL_CULL_FACE);

			CullFace = enable;
		}
	}

	void COGLES2CallBridge::setDepthFunc(GLenum mode)
	{
		if(DepthFunc != mode)
		{
			glDepthFunc(mode);

			DepthFunc = mode;
		}
	}

	void COGLES2CallBridge::setDepthMask(bool enable)
	{
		if(DepthMask != enable)
		{
			if (enable)
				glDepthMask(GL_TRUE);
			else
				glDepthMask(GL_FALSE);

			DepthMask = enable;
		}
	}

	void COGLES2CallBridge::setDepthTest(bool enable)
	{
		if(DepthTest != enable)
		{
			if (enable)
				glEnable(GL_DEPTH_TEST);
			else
				glDisable(GL_DEPTH_TEST);

			DepthTest = enable;
		}
	}

	void COGLES2CallBridge::setProgram(GLuint program)
	{
		if (Program != program)
		{
			glUseProgram(program);
			Program = program;
		}
	}

	void COGLES2CallBridge::setActiveTexture(GLenum texture)
	{
		if (ActiveTexture != texture)
		{
			glActiveTexture(texture);
			ActiveTexture = texture;
		}
	}

	void COGLES2CallBridge::setTexture(u32 stage)
	{
		if (stage < MATERIAL_MAX_TEXTURES)
		{
			if(Texture[stage] != Driver->CurrentTexture[stage])
			{
				setActiveTexture(GL_TEXTURE0 + stage);

				if(Driver->CurrentTexture[stage])
					glBindTexture(GL_TEXTURE_2D, Driver->CurrentTexture[stage]->getOpenGLTextureName());

				Texture[stage] = Driver->CurrentTexture[stage];
			}
		}
	}

	void COGLES2CallBridge::setViewport(const core::rect<s32>& viewport)
	{
		if (Viewport != viewport)
		{
			glViewport(viewport.UpperLeftCorner.X, viewport.UpperLeftCorner.Y, viewport.LowerRightCorner.X, viewport.LowerRightCorner.Y);
			Viewport = viewport;
		}
	}


} // end namespace
} // end namespace

#endif // _IRR_COMPILE_WITH_OGLES2_

namespace irr
{
namespace video
{

#if !defined(_IRR_COMPILE_WITH_IPHONE_DEVICE_) && (defined(_IRR_COMPILE_WITH_X11_DEVICE_) || defined(_IRR_COMPILE_WITH_SDL_DEVICE_) || defined(_IRR_COMPILE_WITH_WINDOWS_DEVICE_) || defined(_IRR_COMPILE_WITH_ANDROID_DEVICE_))
	IVideoDriver* createOGLES2Driver(const SIrrlichtCreationParameters& params,
			video::SExposedVideoData& data, io::IFileSystem* io)
	{
#ifdef _IRR_COMPILE_WITH_OGLES2_
		return new COGLES2Driver(params, data, io);
#else
		return 0;
#endif // _IRR_COMPILE_WITH_OGLES2_
	}
#endif

// -----------------------------------
// WAYLAND VERSION
// -----------------------------------
#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_
	IVideoDriver* createOGLES2Driver(const SIrrlichtCreationParameters& params, 
			io::IFileSystem* io, CIrrDeviceWayland* device)
	{
#ifdef _IRR_COMPILE_WITH_OGLES2_
		return new COGLES2Driver(params, io, device);
#else
		return 0;
#endif // _IRR_COMPILE_WITH_OGLES2_
	}
		
#endif

// -----------------------------------
// MACOSX VERSION
// -----------------------------------
#if defined(_IRR_COMPILE_WITH_OSX_DEVICE_)
	IVideoDriver* createOGLES2Driver(const SIrrlichtCreationParameters& params,
			io::IFileSystem* io, CIrrDeviceMacOSX *device)
	{
#ifdef _IRR_COMPILE_WITH_OGLES2_
		return new COGLES2Driver(params, io, device);
#else
		return 0;
#endif // _IRR_COMPILE_WITH_OGLES2_
	}
#endif // _IRR_COMPILE_WITH_OSX_DEVICE_

// -----------------------------------
// IPHONE VERSION
// -----------------------------------
#if defined(_IRR_COMPILE_WITH_IPHONE_DEVICE_)
	IVideoDriver* createOGLES2Driver(const SIrrlichtCreationParameters& params,
			video::SExposedVideoData& data, io::IFileSystem* io,
			CIrrDeviceIPhone* device)
	{
#ifdef _IRR_COMPILE_WITH_OGLES2_
		return new COGLES2Driver(params, data, io, device);
#else
		return 0;
#endif // _IRR_COMPILE_WITH_OGLES2_
	}
#endif // _IRR_COMPILE_WITH_IPHONE_DEVICE_

} // end namespace
} // end namespace
