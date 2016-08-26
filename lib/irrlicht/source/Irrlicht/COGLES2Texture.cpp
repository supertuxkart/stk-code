// Copyright (C) 2013 Patryk Nadrowski
// Heavily based on the OpenGL driver implemented by Nikolaus Gebhardt
// OpenGL ES driver implemented by Christian Stehno and first OpenGL ES 2.0
// driver implemented by Amundis.
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_OGLES2_

#include "irrTypes.h"
#include "COGLES2Texture.h"
#include "COGLES2Driver.h"
#include "os.h"
#include "CImage.h"
#include "CColorConverter.h"
#include "IAttributes.h"
#include "IrrlichtDevice.h"

#include "irrString.h"

#if !defined(_IRR_COMPILE_WITH_IPHONE_DEVICE_)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#endif

namespace
{
#ifndef GL_BGRA
// we need to do this for the IMG_BGRA8888 extension
int GL_BGRA=GL_RGBA;
#endif
}

namespace irr
{
namespace video
{

//! constructor for usual textures
COGLES2Texture::COGLES2Texture(IImage* origImage, const io::path& name, void* mipmapData, COGLES2Driver* driver)
	: ITexture(name), ColorFormat(ECF_A8R8G8B8), Driver(driver), Image(0), MipImage(0),
	TextureName(0), InternalFormat(GL_RGBA), PixelFormat(GL_BGRA_EXT),
	PixelType(GL_UNSIGNED_BYTE), MipLevelStored(0),
	IsRenderTarget(false), AutomaticMipmapUpdate(false),
	ReadOnlyLock(false), KeepImage(true)
{
	#ifdef _DEBUG
	setDebugName("COGLES2Texture");
	#endif

	HasMipMaps = Driver->getTextureCreationFlag(ETCF_CREATE_MIP_MAPS);
	getImageValues(origImage);

	glGenTextures(1, &TextureName);

	if (ImageSize==TextureSize)
	{
		Image = Driver->createImage(ColorFormat, ImageSize);
		origImage->copyTo(Image);
	}
	else
	{
		Image = Driver->createImage(ColorFormat, TextureSize);
		// scale texture
		origImage->copyToScaling(Image);
	}
	uploadTexture(true, mipmapData);
	if (!KeepImage)
	{
		Image->drop();
		Image=0;
	}
}


//! constructor for basic setup (only for derived classes)
COGLES2Texture::COGLES2Texture(const io::path& name, COGLES2Driver* driver)
	: ITexture(name), ColorFormat(ECF_A8R8G8B8), Driver(driver), Image(0), MipImage(0),
	TextureName(0), InternalFormat(GL_RGBA), PixelFormat(GL_BGRA_EXT),
	PixelType(GL_UNSIGNED_BYTE), MipLevelStored(0), HasMipMaps(true),
	IsRenderTarget(false), AutomaticMipmapUpdate(false),
	ReadOnlyLock(false), KeepImage(true)
{
	#ifdef _DEBUG
	setDebugName("COGLES2Texture");
	#endif
}


//! destructor
COGLES2Texture::~COGLES2Texture()
{
	if (TextureName)
		glDeleteTextures(1, &TextureName);
	if (Image)
		Image->drop();
}


//! Choose best matching color format, based on texture creation flags
ECOLOR_FORMAT COGLES2Texture::getBestColorFormat(ECOLOR_FORMAT format)
{
	ECOLOR_FORMAT destFormat = ECF_A8R8G8B8;
	switch (format)
	{
		case ECF_A1R5G5B5:
			if (!Driver->getTextureCreationFlag(ETCF_ALWAYS_32_BIT))
				destFormat = ECF_A1R5G5B5;
		break;
		case ECF_R5G6B5:
			if (!Driver->getTextureCreationFlag(ETCF_ALWAYS_32_BIT))
				destFormat = ECF_A1R5G5B5;
		break;
		case ECF_A8R8G8B8:
			if (Driver->getTextureCreationFlag(ETCF_ALWAYS_16_BIT) ||
					Driver->getTextureCreationFlag(ETCF_OPTIMIZED_FOR_SPEED))
				destFormat = ECF_A1R5G5B5;
		break;
		case ECF_R8G8B8:
			if (Driver->getTextureCreationFlag(ETCF_ALWAYS_16_BIT) ||
					Driver->getTextureCreationFlag(ETCF_OPTIMIZED_FOR_SPEED))
				destFormat = ECF_A1R5G5B5;
		default:
		break;
	}
	if (Driver->getTextureCreationFlag(ETCF_NO_ALPHA_CHANNEL))
	{
		switch (destFormat)
		{
			case ECF_A1R5G5B5:
				destFormat = ECF_R5G6B5;
			break;
			case ECF_A8R8G8B8:
				destFormat = ECF_R8G8B8;
			break;
			default:
			break;
		}
	}
	return destFormat;
}


// prepare values ImageSize, TextureSize, and ColorFormat based on image
void COGLES2Texture::getImageValues(IImage* image)
{
	if (!image)
	{
		os::Printer::log("No image for OpenGL texture.", ELL_ERROR);
		return;
	}

	ImageSize = image->getDimension();

	if ( !ImageSize.Width || !ImageSize.Height)
	{
		os::Printer::log("Invalid size of image for OpenGL Texture.", ELL_ERROR);
		return;
	}

	const f32 ratio = (f32)ImageSize.Width/(f32)ImageSize.Height;
	if ((ImageSize.Width>Driver->MaxTextureSize) && (ratio >= 1.0f))
	{
		ImageSize.Width = Driver->MaxTextureSize;
		ImageSize.Height = (u32)(Driver->MaxTextureSize/ratio);
	}
	else if (ImageSize.Height>Driver->MaxTextureSize)
	{
		ImageSize.Height = Driver->MaxTextureSize;
		ImageSize.Width = (u32)(Driver->MaxTextureSize*ratio);
	}
	TextureSize=ImageSize.getOptimalSize(false);
    const core::dimension2du max_size = Driver->getDriverAttributes()
                                 .getAttributeAsDimension2d("MAX_TEXTURE_SIZE");

    if (max_size.Width> 0 && TextureSize.Width > max_size.Width)
    {
        TextureSize.Width = max_size.Width;
    }
    if (max_size.Height> 0 && TextureSize.Height > max_size.Height)
    {
        TextureSize.Height = max_size.Height;
    }

	ColorFormat = getBestColorFormat(image->getColorFormat());
}


//! copies the the texture into an open gl texture.
void COGLES2Texture::uploadTexture(bool newTexture, void* mipmapData, u32 level)
{
	// check which image needs to be uploaded
	IImage* image = level?MipImage:Image;
	if (!image)
	{
		os::Printer::log("No image for OGLES2 texture to upload", ELL_ERROR);
		return;
	}

#ifndef GL_BGRA
	// whoa, pretty badly implemented extension...
	if (Driver->FeatureAvailable[COGLES2ExtensionHandler::IRR_IMG_texture_format_BGRA8888] || Driver->FeatureAvailable[COGLES2ExtensionHandler::IRR_EXT_texture_format_BGRA8888])
		GL_BGRA=0x80E1;
	else
		GL_BGRA=GL_RGBA;
#endif

	GLenum oldInternalFormat = InternalFormat;
	void(*convert)(const void*, s32, void*)=0;
	switch (Image->getColorFormat())
	{
		case ECF_A1R5G5B5:
			InternalFormat=GL_RGBA;
			PixelFormat=GL_RGBA;
			PixelType=GL_UNSIGNED_SHORT_5_5_5_1;
			convert=CColorConverter::convert_A1R5G5B5toR5G5B5A1;
			break;
		case ECF_R5G6B5:
			InternalFormat=GL_RGB;
			PixelFormat=GL_RGB;
			PixelType=GL_UNSIGNED_SHORT_5_6_5;
			break;
		case ECF_R8G8B8:
			InternalFormat=GL_RGB;
			PixelFormat=GL_RGB;
			PixelType=GL_UNSIGNED_BYTE;
			convert=CColorConverter::convert_R8G8B8toB8G8R8;
			break;
		case ECF_A8R8G8B8:
			PixelType=GL_UNSIGNED_BYTE;
			if (!Driver->queryOpenGLFeature(COGLES2ExtensionHandler::IRR_IMG_texture_format_BGRA8888) && !Driver->queryOpenGLFeature(COGLES2ExtensionHandler::IRR_EXT_texture_format_BGRA8888))
			{
				convert=CColorConverter::convert_A8R8G8B8toA8B8G8R8;
				InternalFormat=GL_RGBA;
				PixelFormat=GL_RGBA;
			}
			else
			{
				InternalFormat=GL_BGRA;
				PixelFormat=GL_BGRA;
			}
			break;
		default:
			os::Printer::log("Unsupported texture format", ELL_ERROR);
			break;
	}
	// Hack for iPhone SDK, which requires a different InternalFormat
#ifdef _IRR_IPHONE_PLATFORM_
	if (InternalFormat==GL_BGRA)
		InternalFormat=GL_RGBA;
#endif
	// make sure we don't change the internal format of existing matrices
	if (!newTexture)
		InternalFormat=oldInternalFormat;

    Driver->setActiveTexture(0, this);
	Driver->getBridgeCalls()->setTexture(0);

	if (Driver->testGLError())
		os::Printer::log("Could not bind Texture", ELL_ERROR);

	// mipmap handling for main texture
	if (!level && newTexture)
	{
#ifndef DISABLE_MIPMAPPING
		// auto generate if possible and no mipmap data is given
		if (HasMipMaps && !mipmapData)
		{
			if (Driver->getTextureCreationFlag(ETCF_OPTIMIZED_FOR_SPEED))
				glHint(GL_GENERATE_MIPMAP_HINT, GL_FASTEST);
			else if (Driver->getTextureCreationFlag(ETCF_OPTIMIZED_FOR_QUALITY))
				glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
			else
				glHint(GL_GENERATE_MIPMAP_HINT, GL_DONT_CARE);

			AutomaticMipmapUpdate=true;
		}
		else
		{
			// Either generate manually due to missing capability
			// or use predefined mipmap data
			AutomaticMipmapUpdate=false;
			regenerateMipMapLevels(mipmapData);
		}

		if (HasMipMaps) // might have changed in regenerateMipMapLevels
		{
			// enable bilinear mipmap filter
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            StatesCache.BilinearFilter = true;
            StatesCache.TrilinearFilter = false;
            StatesCache.MipMapStatus = true;
		}
		else
#else
			HasMipMaps=false;
			os::Printer::log("Did not create OpenGL texture mip maps.", ELL_INFORMATION);
#endif
		{
			// enable bilinear filter without mipmaps
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            StatesCache.BilinearFilter = true;
            StatesCache.TrilinearFilter = false;
            StatesCache.MipMapStatus = false;
		}
	}

	// now get image data and upload to GPU
	void* source = image->lock();
	IImage* tmpImage=0;

	if (convert)
	{
		tmpImage = new CImage(image->getColorFormat(), image->getDimension());
		void* dest = tmpImage->lock();
		convert(source, image->getDimension().getArea(), dest);
		image->unlock();
		source = dest;
	}

	if (newTexture)
		glTexImage2D(GL_TEXTURE_2D, level, InternalFormat, image->getDimension().Width,
			image->getDimension().Height, 0, PixelFormat, PixelType, source);
	else
		glTexSubImage2D(GL_TEXTURE_2D, level, 0, 0, image->getDimension().Width,
			image->getDimension().Height, PixelFormat, PixelType, source);

	if (convert)
	{
		tmpImage->unlock();
		tmpImage->drop();
	}
	else
		image->unlock();

	if (AutomaticMipmapUpdate)
		glGenerateMipmap(GL_TEXTURE_2D);

	if (Driver->testGLError())
		os::Printer::log("Could not glTexImage2D", ELL_ERROR);
}


//! lock function
void* COGLES2Texture::lock(E_TEXTURE_LOCK_MODE mode, u32 mipmapLevel)
{
	// store info about which image is locked
	IImage* image = (mipmapLevel==0)?Image:MipImage;

	return image->lock();
}


//! unlock function
void COGLES2Texture::unlock()
{
	// test if miplevel or main texture was locked
	IImage* image = MipImage?MipImage:Image;
	if (!image)
		return;
	// unlock image to see changes
	image->unlock();
	// copy texture data to GPU
	if (!ReadOnlyLock)
		uploadTexture(false, 0, MipLevelStored);
	ReadOnlyLock = false;
	// cleanup local image
	if (MipImage)
	{
		MipImage->drop();
		MipImage=0;
	}
	else if (!KeepImage)
	{
		Image->drop();
		Image=0;
	}
	// update information
	if (Image)
		ColorFormat=Image->getColorFormat();
	else
		ColorFormat=ECF_A8R8G8B8;
}


//! Returns size of the original image.
const core::dimension2d<u32>& COGLES2Texture::getOriginalSize() const
{
	return ImageSize;
}


//! Returns size of the texture.
const core::dimension2d<u32>& COGLES2Texture::getSize() const
{
	return TextureSize;
}


//! returns driver type of texture, i.e. the driver, which created the texture
E_DRIVER_TYPE COGLES2Texture::getDriverType() const
{
	return EDT_OGLES2;
}


//! returns color format of texture
ECOLOR_FORMAT COGLES2Texture::getColorFormat() const
{
	return ColorFormat;
}


//! returns pitch of texture (in bytes)
u32 COGLES2Texture::getPitch() const
{
	if (Image)
		return Image->getPitch();
	else
		return 0;
}


//! return open gl texture name
GLuint COGLES2Texture::getOpenGLTextureName() const
{
	return TextureName;
}


//! Returns whether this texture has mipmaps
bool COGLES2Texture::hasMipMaps() const
{
	return HasMipMaps;
}


//! Regenerates the mip map levels of the texture. Useful after locking and
//! modifying the texture
void COGLES2Texture::regenerateMipMapLevels(void* mipmapData)
{
	if (AutomaticMipmapUpdate || !HasMipMaps || !Image)
		return;
	if ((Image->getDimension().Width==1) && (Image->getDimension().Height==1))
		return;

	// Manually create mipmaps or use prepared version
	u32 width=Image->getDimension().Width;
	u32 height=Image->getDimension().Height;
	u32 i=0;
	u8* target = static_cast<u8*>(mipmapData);
	do
	{
		if (width>1)
			width>>=1;
		if (height>1)
			height>>=1;
		++i;
		if (!target)
			target = new u8[width*height*Image->getBytesPerPixel()];
		// create scaled version if no mipdata available
		if (!mipmapData)
			Image->copyToScaling(target, width, height, Image->getColorFormat());
		glTexImage2D(GL_TEXTURE_2D, i, InternalFormat, width, height,
				0, PixelFormat, PixelType, target);
		// get next prepared mipmap data if available
		if (mipmapData)
		{
			mipmapData = static_cast<u8*>(mipmapData)+width*height*Image->getBytesPerPixel();
			target = static_cast<u8*>(mipmapData);
		}
	}
	while (width!=1 || height!=1);
	// cleanup
	if (!mipmapData)
		delete [] target;
}


bool COGLES2Texture::isRenderTarget() const
{
	return IsRenderTarget;
}


void COGLES2Texture::setIsRenderTarget(bool isTarget)
{
	IsRenderTarget = isTarget;
}


bool COGLES2Texture::isFrameBufferObject() const
{
	return false;
}


//! Bind Render Target Texture
void COGLES2Texture::bindRTT()
{
}


//! Unbind Render Target Texture
void COGLES2Texture::unbindRTT()
{
}


//! Get an access to texture states cache.
COGLES2Texture::SStatesCache& COGLES2Texture::getStatesCache() const
{
	return StatesCache;
}


/* FBO Textures */

// helper function for render to texture
static bool checkOGLES2FBOStatus(COGLES2Driver* Driver);

//! RTT ColorFrameBuffer constructor
COGLES2FBOTexture::COGLES2FBOTexture(const core::dimension2d<u32>& size,
					const io::path& name, COGLES2Driver* driver,
					ECOLOR_FORMAT format)
	: COGLES2Texture(name, driver), DepthTexture(0), ColorFrameBuffer(0)
{
	#ifdef _DEBUG
	setDebugName("COGLES2Texture_FBO");
	#endif

	ImageSize = size;
	TextureSize = size;
	HasMipMaps = false;
	IsRenderTarget = true;
	ColorFormat = getBestColorFormat(format);

	switch (ColorFormat)
	{
	case ECF_A8R8G8B8:
		InternalFormat = GL_RGBA;
		PixelFormat = GL_RGBA;
		PixelType = GL_UNSIGNED_BYTE;
		break;
	case ECF_R8G8B8:
		InternalFormat = GL_RGB;
		PixelFormat = GL_RGB;
		PixelType = GL_UNSIGNED_BYTE;
		break;
		break;
	case ECF_A1R5G5B5:
		InternalFormat = GL_RGBA;
		PixelFormat = GL_RGBA;
		PixelType = GL_UNSIGNED_SHORT_5_5_5_1;
		break;
		break;
	case ECF_R5G6B5:
		InternalFormat = GL_RGB;
		PixelFormat = GL_RGB;
		PixelType = GL_UNSIGNED_SHORT_5_6_5;
		break;
	default:
		os::Printer::log( "color format not handled", ELL_WARNING );
		break;
	}

	// generate frame buffer
	glGenFramebuffers(1, &ColorFrameBuffer);
	bindRTT();

	// generate color texture
	glGenTextures(1, &TextureName);

    Driver->setActiveTexture(0, this);
	Driver->getBridgeCalls()->setTexture(0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	StatesCache.BilinearFilter = true;
    StatesCache.WrapU = ETC_CLAMP_TO_EDGE;
    StatesCache.WrapV = ETC_CLAMP_TO_EDGE;

	glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, ImageSize.Width, ImageSize.Height, 0, PixelFormat, PixelType, 0);

#ifdef _DEBUG
	driver->testGLError();
#endif

	// attach color texture to frame buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TextureName, 0);
#ifdef _DEBUG
	checkOGLES2FBOStatus(Driver);
#endif

	unbindRTT();
}


//! destructor
COGLES2FBOTexture::~COGLES2FBOTexture()
{
	if (DepthTexture)
		if (DepthTexture->drop())
			Driver->removeDepthTexture(DepthTexture);
	if (ColorFrameBuffer)
		glDeleteFramebuffers(1, &ColorFrameBuffer);
}


bool COGLES2FBOTexture::isFrameBufferObject() const
{
	return true;
}


//! Bind Render Target Texture
void COGLES2FBOTexture::bindRTT()
{
	if (ColorFrameBuffer != 0)
		glBindFramebuffer(GL_FRAMEBUFFER, ColorFrameBuffer);
}


//! Unbind Render Target Texture
void COGLES2FBOTexture::unbindRTT()
{
	if (ColorFrameBuffer != 0)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


/* FBO Depth Textures */

//! RTT DepthBuffer constructor
COGLES2FBODepthTexture::COGLES2FBODepthTexture(
		const core::dimension2d<u32>& size,
		const io::path& name,
		COGLES2Driver* driver,
		bool useStencil)
	: COGLES2Texture(name, driver), DepthRenderBuffer(0),
	StencilRenderBuffer(0), UseStencil(useStencil)
{
#ifdef _DEBUG
	setDebugName("COGLES2TextureFBO_Depth");
#endif

	ImageSize = size;
	TextureSize = size;
	InternalFormat = GL_RGBA;
	PixelFormat = GL_RGBA;
	PixelType = GL_UNSIGNED_BYTE;
	HasMipMaps = false;

	if (useStencil)
	{
		glGenRenderbuffers(1, &DepthRenderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, DepthRenderBuffer);
#ifdef GL_OES_packed_depth_stencil
		if (Driver->queryOpenGLFeature(COGLES2ExtensionHandler::IRR_OES_packed_depth_stencil))
		{
			// generate packed depth stencil buffer
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, ImageSize.Width, ImageSize.Height);
			StencilRenderBuffer = DepthRenderBuffer; // stencil is packed with depth
		}
		else // generate separate stencil and depth textures
#endif
		{
			glRenderbufferStorage(GL_RENDERBUFFER, Driver->getZBufferBits(), ImageSize.Width, ImageSize.Height);

			glGenRenderbuffers(1, &StencilRenderBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, StencilRenderBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, ImageSize.Width, ImageSize.Height);
		}
	}
	else
	{
		// generate depth buffer
		glGenRenderbuffers(1, &DepthRenderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, DepthRenderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, Driver->getZBufferBits(), ImageSize.Width, ImageSize.Height);
	}
}


//! destructor
COGLES2FBODepthTexture::~COGLES2FBODepthTexture()
{
	if (DepthRenderBuffer)
		glDeleteRenderbuffers(1, &DepthRenderBuffer);

	if (StencilRenderBuffer && StencilRenderBuffer != DepthRenderBuffer)
		glDeleteRenderbuffers(1, &StencilRenderBuffer);
}


//combine depth texture and rtt
bool COGLES2FBODepthTexture::attach(ITexture* renderTex)
{
	if (!renderTex)
		return false;
	COGLES2FBOTexture* rtt = static_cast<COGLES2FBOTexture*>(renderTex);
	rtt->bindRTT();

	// attach stencil texture to stencil buffer
	if (UseStencil)
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, StencilRenderBuffer);

	// attach depth renderbuffer to depth buffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, DepthRenderBuffer);

	// check the status
	if (!checkOGLES2FBOStatus(Driver))
	{
		os::Printer::log("FBO incomplete");
		return false;
	}
	rtt->DepthTexture=this;
	rtt->DepthBufferTexture = DepthRenderBuffer;
	grab(); // grab the depth buffer, not the RTT
	rtt->unbindRTT();
	return true;
}


//! Bind Render Target Texture
void COGLES2FBODepthTexture::bindRTT()
{
}


//! Unbind Render Target Texture
void COGLES2FBODepthTexture::unbindRTT()
{
}


bool checkOGLES2FBOStatus(COGLES2Driver* Driver)
{
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	switch (status)
	{
		case GL_FRAMEBUFFER_COMPLETE:
			return true;

		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			os::Printer::log("FBO has one or several incomplete image attachments", ELL_ERROR);
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			os::Printer::log("FBO missing an image attachment", ELL_ERROR);
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
			os::Printer::log("FBO has one or several image attachments with different dimensions", ELL_ERROR);
			break;

		case GL_FRAMEBUFFER_UNSUPPORTED:
			os::Printer::log("FBO format unsupported", ELL_ERROR);
			break;

		default:
			break;
	}

	os::Printer::log("FBO error", ELL_ERROR);

	return false;
}


} // end namespace video
} // end namespace irr

#endif // _IRR_COMPILE_WITH_OGLES2_

