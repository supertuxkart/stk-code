// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_OGLES1_TEXTURE_H_INCLUDED__
#define __C_OGLES1_TEXTURE_H_INCLUDED__

#include "ITexture.h"
#include "IImage.h"

#include "IrrCompileConfig.h"
#if defined(_IRR_COMPILE_WITH_OGLES1_)

#if defined(_IRR_COMPILE_WITH_IPHONE_DEVICE_)
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#elif defined(_IRR_ANDROID_PLATFORM_)
#include <GLES/gl.h>
#include <GLES/glext.h>
#else
#include <GLES/egl.h>
#endif

namespace irr
{
namespace video
{

class COGLES1Driver;
//! OGLES1 texture.
class COGLES1Texture : public ITexture
{
public:

	//! constructor
	COGLES1Texture(IImage* surface, const io::path& name, COGLES1Driver* driver=0, void* mipmapData=0);

	//! destructor
	virtual ~COGLES1Texture();

	//! lock function
	virtual void* lock(E_TEXTURE_LOCK_MODE mode=ETLM_READ_WRITE, u32 mipmapLevel=0);

	//! unlock function
	virtual void unlock();

	//! Returns original size of the texture (image).
	virtual const core::dimension2d<u32>& getOriginalSize() const;

	//! Returns size of the texture.
	virtual const core::dimension2d<u32>& getSize() const;

	//! returns driver type of texture (=the driver, that created it)
	virtual E_DRIVER_TYPE getDriverType() const;

	//! returns color format of texture
	virtual ECOLOR_FORMAT getColorFormat() const;

	//! returns pitch of texture (in bytes)
	virtual u32 getPitch() const;

	//! return open gl texture name
	GLuint getOGLES1TextureName() const;

	//! return whether this texture has mipmaps
	virtual bool hasMipMaps() const;

	//! Regenerates the mip map levels of the texture.
	virtual void regenerateMipMapLevels(void* mipmapData=0);

	//! Is it a render target?
	virtual bool isRenderTarget() const;

	//! Is it a FrameBufferObject?
	virtual bool isFrameBufferObject() const;

	//! Bind RenderTargetTexture
	void bindRTT();

	//! Unbind RenderTargetTexture
	void unbindRTT();

	//! sets whether this texture is intended to be used as a render target.
	void setIsRenderTarget(bool isTarget);

protected:

	//! protected constructor with basic setup, no GL texture name created, for derived classes
	COGLES1Texture(const io::path& name, COGLES1Driver* driver);

	//! get the desired color format based on texture creation flags and the input format.
	ECOLOR_FORMAT getBestColorFormat(ECOLOR_FORMAT format);

	//! convert the image into an internal image with better properties for this driver.
	void getImageValues(IImage* image);

	//! copies the the texture into an open gl texture.
	void uploadTexture(bool newTexture=true, void* mipmapData=0, u32 mipLevel=0);

	core::dimension2d<u32> ImageSize;
	core::dimension2d<u32> TextureSize;
	ECOLOR_FORMAT ColorFormat;
	COGLES1Driver* Driver;
	IImage* Image;
	IImage* MipImage;

	GLuint TextureName;
	GLint InternalFormat;
	GLenum PixelFormat;
	GLenum PixelType;
	u32 MipLevelStored;

	bool HasMipMaps;
	bool IsRenderTarget;
	bool AutomaticMipmapUpdate;
	bool UseStencil;
	bool ReadOnlyLock;
	bool KeepImage;
};


//! OGLES1 FBO texture.
class COGLES1FBOTexture : public COGLES1Texture
{
public:

	//! FrameBufferObject constructor
	COGLES1FBOTexture(const core::dimension2d<u32>& size, const io::path& name, COGLES1Driver* driver=0, ECOLOR_FORMAT format = ECF_UNKNOWN);

	//! destructor
	virtual ~COGLES1FBOTexture();

	//! Is it a FrameBufferObject?
	virtual bool isFrameBufferObject() const;

	//! Bind RenderTargetTexture
	virtual void bindRTT();

	//! Unbind RenderTargetTexture
	virtual void unbindRTT();

	ITexture* DepthTexture;
protected:
	GLuint ColorFrameBuffer;
};


//! OGLES1 FBO depth texture.
class COGLES1FBODepthTexture : public COGLES1FBOTexture
{
public:
	//! FrameBufferObject depth constructor
	COGLES1FBODepthTexture(const core::dimension2d<u32>& size, const io::path& name, COGLES1Driver* driver=0, bool useStencil=false);

	//! destructor
	virtual ~COGLES1FBODepthTexture();

	//! Bind RenderTargetTexture
	virtual void bindRTT();

	//! Unbind RenderTargetTexture
	virtual void unbindRTT();

	void attach(ITexture*);

protected:
	GLuint DepthRenderBuffer;
	GLuint StencilRenderBuffer;
	bool UseStencil;
};


} // end namespace video
} // end namespace irr

#endif
#endif // _IRR_COMPILE_WITH_OGLES1_
