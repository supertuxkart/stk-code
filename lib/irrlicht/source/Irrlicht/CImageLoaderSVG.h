// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
// This file is modified by riso from CImageLoaderJPG.h
// nanosvg as parser and rasterizer for SVG files.
// The nanosvg headers are based on those in SDL2_image-2.0.5

#ifndef __C_IMAGE_LOADER_SVG_H_INCLUDED__
#define __C_IMAGE_LOADER_SVG_H_INCLUDED__

#include "IrrCompileConfig.h"

#include "IImageLoader.h"


#include <stdio.h>
#include <string.h>
#include <float.h>
#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"



namespace irr
{
namespace video
{

/*!
	Surface Loader for SVG images
*/
class CImageLoaderSVG : public IImageLoader
{
public:

	//! constructor
	CImageLoaderSVG();

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".tga")
	virtual bool isALoadableFileExtension(const io::path& filename) const;

	//! returns true if the file maybe is able to be loaded by this class
	virtual bool isALoadableFileFormat(io::IReadFile* file) const;

	//! creates a surface from the file
	virtual IImage* loadImage(io::IReadFile* file, bool skip_checking = false) const;

	virtual bool getImageSize(io::IReadFile* file, core::dimension2du* dim) const;
private:
	struct NSVGimage* getSVGImage(io::IReadFile* file, float* scale) const;

	core::dimension2du getImageSize(struct NSVGimage* img, float scale) const;
};


} // end namespace video
} // end namespace irr

#endif

