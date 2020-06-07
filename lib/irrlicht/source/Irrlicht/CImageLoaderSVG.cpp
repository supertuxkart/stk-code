// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
// This file is modified by riso from CImageLoaderJPG.h
// nanosvg as parser and rasterizer for SVG files.
// The nanosvg headers are based on those in SDL2_image-2.0.5

#include "CImageLoaderSVG.h"


#include "IReadFile.h"
#include "CImage.h"
#include "os.h"
#include "irrString.h"
#include "CNullDriver.h"

namespace irr
{
namespace video
{


//! constructor
CImageLoaderSVG::CImageLoaderSVG()
{
}

//! set the screen size
void CImageLoaderSVG::setScreenSize(const core::dimension2d<u32> &screen_size)
{
    ScreenSize = screen_size;
}

//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".tga")
bool CImageLoaderSVG::isALoadableFileExtension(const io::path& filename) const
{
    return core::hasFileExtension ( filename, "svg" );
}


//! returns true if the file maybe is able to be loaded by this class
bool CImageLoaderSVG::isALoadableFileFormat(io::IReadFile* file) const
{
    // read first 4095 characters, check if can find "<svg"
    // this implementation refers IMG_isSVG() in SDL2_Image
    long filesize = file->getSize();
    long cropsize = filesize < 4095L ? filesize : 4095L;
    char* data = new char[cropsize+1];

    int readsize = file->read(data,cropsize);
    data[cropsize] = '\0';
    if( (long)readsize != cropsize) {
        os::Printer::log("Couldn't read SVG image file", file->getFileName(), ELL_ERROR);
        delete[] data;
        return 0;
    }
    // check if can find "<svg" in the file
    if ( strstr(data, "<svg") )
    {
        delete[] data;
        return 1;
    }

    delete[] data;
    return 0;
}


//! creates a surface from the file
IImage* CImageLoaderSVG::loadImage(io::IReadFile* file, bool skip_checking) const
{
    // check IMG_LoadSVG_RW
    struct NSVGimage *img = 0;
    struct NSVGrasterizer *rasterizer = 0;
    video::IImage* image = 0;
    int w, h;

    // read all the characters in the svg file to data
    long filesize = file->getSize();
    char* data = new char[filesize+1];
    int readsize = file->read(data,filesize);
    data[filesize] = '\0';
    
    if( (long)readsize != filesize) {
        os::Printer::log("Couldn't read SVG image file", file->getFileName(), ELL_ERROR);
        delete[] data;
        return 0;
    }

    // parse the svg image
    img = nsvgParse(data, "px", 96);
    if ( !img ) {
        os::Printer::log("Couldn't parse SVG image", ELL_ERROR);
        delete[] data;
        return 0;
    }
    delete[] data;

    rasterizer = nsvgCreateRasterizer();
    if ( !rasterizer ) {
        os::Printer::log("Couldn't create SVG rasterizer", ELL_ERROR);
        nsvgDelete( img );
        return 0;
    }

    float scale =1.0f;
    // only rescale the icons
    if ( strstr(file->getFileName().c_str(),"gui/icons/") )
    {
        // determine scaling based on screen size
        float screen_height = ScreenSize.Height;
        float desired_icon_size = 0.21*screen_height + 30.0f; // phenomenological
        scale = desired_icon_size/img->height;
    }

    // create surface
    w = (int)(img->width*scale);
    h = (int)(img->height*scale);
    image = new CImage(ECF_A8R8G8B8, core::dimension2d<u32>(w, h));
    if ( !image ) {
        os::Printer::log("LOAD SVG: create image struct failure", file->getFileName(), ELL_ERROR);
        nsvgDeleteRasterizer( rasterizer );
        nsvgDelete( img );
        return 0;
    }

    // rasterization
    nsvgRasterize(rasterizer, img, 0.0f, 0.0f, scale, (unsigned char *)image->lock(), (int)image->getDimension().Width, (int)image->getDimension().Height, (int)image->getPitch());

    if (image)
        image->unlock();
    // clean up
    nsvgDeleteRasterizer( rasterizer );
    nsvgDelete( img );

    return image;
}


//! creates a loader which is able to load svg images
IImageLoader* createImageLoaderSVG()
{
    return new CImageLoaderSVG();
}


} // end namespace video
} // end namespace irr


