// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CImageLoaderJPG.h"

#ifdef _IRR_COMPILE_WITH_JPG_LOADER_

#include "IReadFile.h"
#include "CImage.h"
#include "os.h"
#include "irrString.h"

#include <stdio.h>

namespace irr
{
namespace video
{

//! constructor
CImageLoaderJPG::CImageLoaderJPG()
{
	#ifdef _DEBUG
	setDebugName("CImageLoaderJPG");
	#endif
}



//! destructor
CImageLoaderJPG::~CImageLoaderJPG()
{
}



//! returns true if the file maybe is able to be loaded by this class
//! based on the file extension (e.g. ".tga")
bool CImageLoaderJPG::isALoadableFileExtension(const io::path& filename) const
{
	return core::hasFileExtension ( filename, "jpg", "jpeg" );
}


#ifdef _IRR_COMPILE_WITH_LIBJPEG_

/* portions derived from IJG code */

#define readbyte(a,b) do if(((a)=getc((b))) == EOF) return 0; while (0)
#define readword(a,b) do { int cc_=0,dd_=0; \
                          if((cc_=getc((b))) == EOF \
               || (dd_=getc((b))) == EOF) return 0; \
                          (a) = (cc_<<8) + (dd_); \
                          } while(0)

int scanhead (FILE * infile, unsigned * image_width, unsigned * image_height) {
  int marker=0;
  int dummy=0;
  if ( !infile || getc(infile) != 0xFF || getc(infile) != 0xD8 )
    return 0;

  for (;
      ;){

    int discarded_bytes=0;
    readbyte(marker,infile);
    while (marker != 0xFF) {
      discarded_bytes++;
      readbyte(marker,infile);
    }
    do readbyte(marker,infile); while (marker == 0xFF);

    if (discarded_bytes != 0) return 0;

    switch (marker) {
    case 0xC0:
    case 0xC1:
    case 0xC2:
    case 0xC3:
    case 0xC5:
    case 0xC6:
    case 0xC7:
    case 0xC9:
    case 0xCA:
    case 0xCB:
    case 0xCD:
    case 0xCE:
    case 0xCF: {
      readword(dummy,infile);  /* usual parameter length count */
      readbyte(dummy,infile);
      readword((*image_height),infile);
      readword((*image_width),infile);
      readbyte(dummy,infile);

      return 1;
      break;
      }
    case 0xDA:
    case 0xD9:
      return 0;
    default: {
      int length = 0;
      readword(length,infile);

      if (length < 2)
        return 0;
      length -= 2;
      while (length > 0) {
        readbyte(dummy, infile);
        length--;
      }
    }
    break;
    }
  }
}


    // struct for handling jpeg errors
    struct irr_jpeg_error_mgr
    {
        // public jpeg error fields
        struct jpeg_error_mgr pub;

        // for longjmp, to return to caller on a fatal error
        jmp_buf setjmp_buffer;
    };

void CImageLoaderJPG::init_source (j_decompress_ptr cinfo)
{
	// DO NOTHING
}



boolean CImageLoaderJPG::fill_input_buffer (j_decompress_ptr cinfo)
{
	// DO NOTHING
	return TRUE;
}



void CImageLoaderJPG::skip_input_data (j_decompress_ptr cinfo, long count)
{
	jpeg_source_mgr * src = cinfo->src;
	if(count > 0)
	{
		src->bytes_in_buffer -= count;
		src->next_input_byte += count;
	}
}



void CImageLoaderJPG::term_source (j_decompress_ptr cinfo)
{
	// DO NOTHING
}


void CImageLoaderJPG::error_exit (j_common_ptr cinfo)
{
	// unfortunately we need to use a goto rather than throwing an exception
	// as gcc crashes under linux crashes when using throw from within
	// extern c code

	// Always display the message
	(*cinfo->err->output_message) (cinfo);

	// cinfo->err really points to a irr_error_mgr struct
	irr_jpeg_error_mgr *myerr = (irr_jpeg_error_mgr*) cinfo->err;

	longjmp(myerr->setjmp_buffer, 1);
}


void CImageLoaderJPG::output_message(j_common_ptr cinfo)
{
	// display the error message.
	//c8 temp1[JMSG_LENGTH_MAX];
	//(*cinfo->err->format_message)(cinfo, temp1);
	//core::stringc errMsg("JPEG FATAL ERROR in ");
	//errMsg += core::stringc(Filename);
	//os::Printer::log(errMsg.c_str(),temp1, ELL_ERROR);
}
#endif // _IRR_COMPILE_WITH_LIBJPEG_

//! returns true if the file maybe is able to be loaded by this class
bool CImageLoaderJPG::isALoadableFileFormat(io::IReadFile* file) const
{
	#ifndef _IRR_COMPILE_WITH_LIBJPEG_
	return false;
	#else

	if (!(file && file->seek(0)))
		return false;
	unsigned char header[3];
	size_t headerLen = file->read(header, sizeof(header));
	return headerLen >= 3 && !memcmp(header, "\xFF\xD8\xFF", 3);
	#endif
}

//! creates a surface from the file
IImage* CImageLoaderJPG::loadImage(io::IReadFile* file, bool skip_checking) const
{
	#ifndef _IRR_COMPILE_WITH_LIBJPEG_
	//os::Printer::log("Can't load as not compiled with _IRR_COMPILE_WITH_LIBJPEG_:", file->getFileName(), ELL_DEBUG);
	return 0;
	#else

	if (!file)
		return 0;

	u8 **rowPtr=0;
	u8* input = new u8[file->getSize()];
	file->read(input, file->getSize());

	// allocate and initialize JPEG decompression object
	struct jpeg_decompress_struct cinfo;
	struct irr_jpeg_error_mgr jerr;

	//We have to set up the error handler first, in case the initialization
	//step fails.  (Unlikely, but it could happen if you are out of memory.)
	//This routine fills in the contents of struct jerr, and returns jerr's
	//address which we place into the link field in cinfo.

	cinfo.err = jpeg_std_error(&jerr.pub);
	cinfo.err->error_exit = error_exit;
	cinfo.err->output_message = output_message;

	// compatibility fudge:
	// we need to use setjmp/longjmp for error handling as gcc-linux
	// crashes when throwing within external c code
	if (setjmp(jerr.setjmp_buffer))
	{
		// If we get here, the JPEG code has signaled an error.
		// We need to clean up the JPEG object and return.

		jpeg_destroy_decompress(&cinfo);

		delete [] input;
		// if the row pointer was created, we delete it.
		if (rowPtr)
			delete [] rowPtr;

		// return null pointer
		return 0;
	}

	// Now we can initialize the JPEG decompression object.
	jpeg_create_decompress(&cinfo);

	// specify data source
	jpeg_source_mgr jsrc;

	// Set up data pointer
	jsrc.bytes_in_buffer = file->getSize();
	jsrc.next_input_byte = (JOCTET*)input;
	cinfo.src = &jsrc;

	jsrc.init_source = init_source;
	jsrc.fill_input_buffer = fill_input_buffer;
	jsrc.skip_input_data = skip_input_data;
	jsrc.resync_to_restart = jpeg_resync_to_restart;
	jsrc.term_source = term_source;

	// Decodes JPG input from whatever source
	// Does everything AFTER jpeg_create_decompress
	// and BEFORE jpeg_destroy_decompress
	// Caller is responsible for arranging these + setting up cinfo

	// read file parameters with jpeg_read_header()
	jpeg_read_header(&cinfo, TRUE);

	bool useCMYK=false;
	if (cinfo.jpeg_color_space==JCS_CMYK)
	{
		cinfo.out_color_space=JCS_CMYK;
		cinfo.out_color_components=4;
		useCMYK=true;
	}
	else
	{
		cinfo.out_color_space=JCS_RGB;
		cinfo.out_color_components=3;
	}
	cinfo.output_gamma=2.2;
	cinfo.do_fancy_upsampling=FALSE;

	// Start decompressor
	jpeg_start_decompress(&cinfo);

	// Get image data
	u16 rowspan = cinfo.image_width * cinfo.out_color_components;
	u32 width = cinfo.image_width;
	u32 height = cinfo.image_height;

	// Allocate memory for buffer
	u8* output = new u8[rowspan * height];

	// Here we use the library's state variable cinfo.output_scanline as the
	// loop counter, so that we don't have to keep track ourselves.
	// Create array of row pointers for lib
	rowPtr = new u8* [height];

	for( u32 i = 0; i < height; i++ )
		rowPtr[i] = &output[ i * rowspan ];

	u32 rowsRead = 0;

	while( cinfo.output_scanline < cinfo.output_height )
		rowsRead += jpeg_read_scanlines( &cinfo, &rowPtr[rowsRead], cinfo.output_height - rowsRead );

	delete [] rowPtr;
	// Finish decompression

	jpeg_finish_decompress(&cinfo);

	// Release JPEG decompression object
	// This is an important step since it will release a good deal of memory.
	jpeg_destroy_decompress(&cinfo);

	// convert image
	IImage* image = 0;
	if (useCMYK)
	{
		image = new CImage(ECF_R8G8B8,
				core::dimension2d<u32>(width, height));
		const u32 size = 3*width*height;
		u8* data = (u8*)image->lock();
		if (data)
		{
			for (u32 i=0,j=0; i<size; i+=3, j+=4)
			{
				// Also works without K, but has more contrast with K multiplied in
//				data[i+0] = output[j+2];
//				data[i+1] = output[j+1];
//				data[i+2] = output[j+0];
				data[i+0] = (char)(output[j+2]*(output[j+3]/255.f));
				data[i+1] = (char)(output[j+1]*(output[j+3]/255.f));
				data[i+2] = (char)(output[j+0]*(output[j+3]/255.f));
			}
		}
		image->unlock();
		delete [] output;
	}
	else
		image = new CImage(ECF_R8G8B8,
				core::dimension2d<u32>(width, height), output);

	delete [] input;

	return image;

	#endif
}

bool CImageLoaderJPG::getImageSize(io::IReadFile* file, core::dimension2du* dim) const
{
	#ifndef _IRR_COMPILE_WITH_LIBJPEG_
	return false;
	#else
	if (!dim || !file || !isALoadableFileFormat(file))
		return false;
	file->seek(0);
	scanhead((FILE*)file->getFileHandle(), &dim->Width, &dim->Height);
	file->seek(0);
	return true;
	#endif
}

//! creates a loader which is able to load jpeg images
IImageLoader* createImageLoaderJPG()
{
	return new CImageLoaderJPG();
}

} // end namespace video
} // end namespace irr

#endif

