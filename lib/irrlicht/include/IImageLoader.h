// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __I_SURFACE_LOADER_H_INCLUDED__
#define __I_SURFACE_LOADER_H_INCLUDED__

#include "IReferenceCounted.h"
#include "IImage.h"
#include "path.h"

namespace irr
{
namespace io
{
	class IReadFile;
} // end namespace io
namespace video
{

//! Class which is able to create a image from a file.
/** If you want the Irrlicht Engine be able to load textures of
currently unsupported file formats (e.g .gif), then implement
this and add your new Surface loader with
IVideoDriver::addExternalImageLoader() to the engine. */
class IImageLoader : public virtual IReferenceCounted
{
public:

	//! Check if the file might be loaded by this class
	/** Check is based on the file extension (e.g. ".tga")
	\param filename Name of file to check.
	\return True if file seems to be loadable. */
	virtual bool isALoadableFileExtension(const io::path& filename) const = 0;

	//! Check if the file might be loaded by this class
	/** Check might look into the file.
	\param file File handle to check.
	\return True if file seems to be loadable. */
	virtual bool isALoadableFileFormat(io::IReadFile* file) const = 0;

	//! Creates a surface from the file
	/** \param file File handle to check.
	\return Pointer to newly created image, or 0 upon error. */
	virtual IImage* loadImage(io::IReadFile* file, bool skip_checking = false) const = 0;
	virtual core::dimension2du getImageSize(io::IReadFile* file) const { return core::dimension2du(0, 0); }
	virtual bool supportThreadedLoading() const { return false; }
};


} // end namespace video
} // end namespace irr

#endif

