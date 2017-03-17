// Copyright (C) 2013 Patryk Nadrowski
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// OpenGL ES driver implemented by Christian Stehno and first OpenGL ES 2.0
// driver implemented by Amundis.
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_OGLES2_TEXTURE_H_INCLUDED__
#define __C_OGLES2_TEXTURE_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_OGLES2_

#if defined(_IRR_COMPILE_WITH_IPHONE_DEVICE_)
#include <OpenGLES/ES2/gl.h>
#else
#include <GLES2/gl2.h>
#endif

#include "ITexture.h"
#include "IImage.h"
#include "SMaterialLayer.h"

namespace irr
{
namespace video
{

class COGLES2Driver;

//! OpenGL ES 2.0 texture.
class COGLES2Texture : public ITexture
{
public:

	//! Cache structure.
	struct SStatesCache
	{
		SStatesCache() : WrapU(ETC_REPEAT), WrapV(ETC_REPEAT), BilinearFilter(false),
			TrilinearFilter(false), AnisotropicFilter(0), MipMapStatus(false), LODBias(0), IsCached(false)
		{
		}

		u8 WrapU;
		u8 WrapV;
		bool BilinearFilter;
		bool TrilinearFilter;
		u8 AnisotropicFilter;
		bool MipMapStatus;
		s8 LODBias;

		bool IsCached;
	};

	//! constructor
	COGLES2Texture(IImage* surface, const io::path& name, void* mipmapData=0, COGLES2Driver* driver=0);

	//! destructor
	virtual ~COGLES2Texture();

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
	virtual u32 getOpenGLTextureName() const;

	virtual u64 getHandle() { return 0; }

	//! return whether this texture has mipmaps
	virtual bool hasMipMaps() const;

	//! Regenerates the mip map levels of the texture.
	/** Useful after locking and modifying the texture
	\param mipmapData Pointer to raw mipmap data, including all necessary mip levels, in the same format as the main texture image. If not set the mipmaps are derived from the main image. */
	virtual void regenerateMipMapLevels(void* mipmapData=0);

	//! Is it a render target?
	virtual bool isRenderTarget() const;

	//! Is it a FrameBufferObject?
	virtual bool isFrameBufferObject() const;

	//! Bind RenderTargetTexture
	virtual void bindRTT();

	//! Unbind RenderTargetTexture
	virtual void unbindRTT();

	//! sets whether this texture is intended to be used as a render target.
	void setIsRenderTarget(bool isTarget);

	//! Get an access to texture states cache.
	SStatesCache& getStatesCache() const;

	void setImage(IImage* new_image)
	{
		if (Image)
			Image->drop();
		Image = new_image;
	}

protected:

	//! protected constructor with basic setup, no GL texture name created, for derived classes
	COGLES2Texture(const io::path& name, COGLES2Driver* driver);

	//! get the desired color format based on texture creation flags and the input format.
	ECOLOR_FORMAT getBestColorFormat(ECOLOR_FORMAT format);

	//! get important numbers of the image and hw texture
	void getImageValues(IImage* image);

	//! copies the texture into an OpenGL texture.
	/** \param newTexture True if method is called for a newly created texture for the first time. Otherwise call with false to improve memory handling.
	\param mipmapData Pointer to raw mipmap data, including all necessary mip levels, in the same format as the main texture image.
	\param mipLevel If set to non-zero, only that specific miplevel is updated, using the MipImage member. */
	void uploadTexture(bool newTexture=false, void* mipmapData=0, u32 mipLevel=0);

	core::dimension2d<u32> ImageSize;
	core::dimension2d<u32> TextureSize;
	ECOLOR_FORMAT ColorFormat;
	COGLES2Driver* Driver;
	IImage* Image;
	IImage* MipImage;

	GLuint TextureName;
	GLint InternalFormat;
	GLenum PixelFormat;
	GLenum PixelType;

	u8 MipLevelStored;
	bool HasMipMaps;
	bool IsRenderTarget;
	bool AutomaticMipmapUpdate;
	bool ReadOnlyLock;
	bool KeepImage;

	mutable SStatesCache StatesCache;
};

//! OpenGL ES 2.0 FBO texture.
class COGLES2FBOTexture : public COGLES2Texture
{
public:

	//! FrameBufferObject constructor
	COGLES2FBOTexture(const core::dimension2d<u32>& size, const io::path& name,
		COGLES2Driver* driver = 0, const ECOLOR_FORMAT format = ECF_UNKNOWN);

	//! destructor
	virtual ~COGLES2FBOTexture();

	//! Is it a FrameBufferObject?
	virtual bool isFrameBufferObject() const;

	//! Bind RenderTargetTexture
	virtual void bindRTT();

	//! Unbind RenderTargetTexture
	virtual void unbindRTT();

	ITexture* DepthTexture;
	GLuint DepthBufferTexture;
protected:
	GLuint ColorFrameBuffer;
};


//! OpenGL ES 2.0 FBO depth texture.
class COGLES2FBODepthTexture : public COGLES2Texture
{
public:
	//! FrameBufferObject depth constructor
	COGLES2FBODepthTexture(const core::dimension2d<u32>& size, const io::path& name, COGLES2Driver* driver=0, bool useStencil=false);

	//! destructor
	virtual ~COGLES2FBODepthTexture();

	//! Bind RenderTargetTexture
	virtual void bindRTT();

	//! Unbind RenderTargetTexture
	virtual void unbindRTT();

	bool attach(ITexture*);

protected:
	GLuint DepthRenderBuffer;
	GLuint StencilRenderBuffer;
	bool UseStencil;
};


} // end namespace video
} // end namespace irr

#endif
#endif // _IRR_COMPILE_WITH_OGLES2_

