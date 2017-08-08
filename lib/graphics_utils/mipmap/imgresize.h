/* -----------------------------------------------------------------------------
 *
 * Copyright (c) 2014-2017 Alexis Naveros.
 * Portions developed under contract to the SURVICE Engineering Company.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * -----------------------------------------------------------------------------
 */


#ifndef IMGRESIZE_H
#define IMGRESIZE_H


typedef struct
{
  /* Specify filter type, from the IM_REDUCE_FILTER_* list */
  int filter;
  /* High quality, a little slow: hopcount=3; */
  /* Good quality, much faster: hopcount=2; */
  int hopcount;
  /* Strong preservation/amplification of details: alpha=2.0f; */
  /* Mild preservation/amplification of details: alpha=6.0f; */
  float alpha;
  /* NORMALMAP filters: factor to amyplify normals on X and Y before normalization */
  float amplifynormal;
  /* NORMALMAP_SUSTAIN filters: Preserve a factor of deviation "energy" as calculated by sqrtf(x*x+y*y) */
  float normalsustainfactor;
} imReduceOptions;

static inline void imReduceSetOptions( imReduceOptions *options, int filter, int hopcount, float alpha, float amplifynormal, float normalsustainfactor )
{
  options->filter = filter;
  options->hopcount = hopcount;
  options->alpha = alpha;
  options->amplifynormal = amplifynormal;
  options->normalsustainfactor = normalsustainfactor;
  return;
}


/* Reduce the image's dimensions by an integer divisor ~ this is fairly fast */
int imReduceImageKaiserDataDivisor( unsigned char *dstdata, unsigned char *srcdata, int width, int height, int bytesperpixel, int bytesperline, int sizedivisor, imReduceOptions *options );
/* Same as imReduceImageKaiserDataDivisor(), but imgdst is allocated */
int imReduceImageKaiserDivisor( imgImage *imgdst, imgImage *imgsrc, int sizedivisor, imReduceOptions *options );


/* Reduce the image's dimensions to match the newwidth and newheight ~ this is a little slower */
int imReduceImageKaiserData( unsigned char *dstdata, unsigned char *srcdata, int width, int height, int bytesperpixel, int bytesperline, int newwidth, int newheight, imReduceOptions *options );
/* Same as imReduceImageKaiserData(), but imgdst is allocated */
int imReduceImageKaiser( imgImage *imgdst, imgImage *imgsrc, int newwidth, int newheight, imReduceOptions *options );


/* Resize by half with a dumb box filter ~ don't use that except for the smallest mipmaps */
/* Filters with ALPHANORM and/or SUSTAIN keywords are processed as the regular base filter only */
int imReduceImageHalfBoxData( unsigned char *dstdata, unsigned char *srcdata, int width, int height, int bytesperpixel, int bytesperline, imReduceOptions *options );
int imReduceImageHalfBox( imgImage *imgdst, imgImage *imgsrc, imReduceOptions *options );


/*
Keywords for image reduction filters

LINEAR: Data is linear, note that this is *not* the format of typical diffuse textures
SRGB: Color is in sRGB space, any alpha is presumed linear
NORMALMAP: RGB represents a XYZ vector as (2.0*RGB)-1.0f, any alpha is presumed linear

ALPHANORM: Alpha normalization, the weight of pixels is proportional to their alpha values
           (do you have "black" fully transparent pixels? please use an ALPHANORM filter)
SUSTAIN: The "energy" of the normal map is sustained, amplified to preserve the level of details
         Note that this filter is rather slow (set options->normalsustainfactor to 0.75 or so)
*/

enum
{
  /* Linear space */
  IM_REDUCE_FILTER_LINEAR,
  IM_REDUCE_FILTER_LINEAR_ALPHANORM,

  /* sRGB space (probably what you want for diffuse textures) */
  IM_REDUCE_FILTER_SRGB,
  IM_REDUCE_FILTER_SRGB_ALPHANORM,

  /* RGB represents a XYZ vector as (2.0*RGB)-1.0f, any alpha is presumed linear */
  IM_REDUCE_FILTER_NORMALMAP,
  IM_REDUCE_FILTER_NORMALMAP_ALPHANORM,
  IM_REDUCE_FILTER_NORMALMAP_SUSTAIN,
  IM_REDUCE_FILTER_NORMALMAP_SUSTAIN_ALPHANORM,

  /* Custom specialized filters */
  IM_REDUCE_FILTER_WATERMAP,
  IM_REDUCE_FILTER_PLANTMAP,
  IM_REDUCE_FILTER_FOLLIAGE,
  IM_REDUCE_FILTER_SKY,
  IM_REDUCE_FILTER_FOG
};


////


#define IM_MIPMAP_CASCADE_MAX (16)

typedef struct
{
  int width;
  int height;
  int layercount;
  int bytesperpixel;
  int bytesperline;
  imReduceOptions *options;
  void *mipmap[IM_MIPMAP_CASCADE_MAX];
} imMipmapCascade;


int imBuildMipmapCascade( imMipmapCascade *cascade, void *imagedata, int width, int height, int layercount, int bytesperpixel, int bytesperline, imReduceOptions *options, int cascadeflags );

void imFreeMipmapCascade( imMipmapCascade *cascade );

/* For base texture, propagate RGB channels to neighbors if they are fully transparent (ignored if bytesperpixel != 4 ) */
#define IM_CASCADE_FLAGS_COLOR_BORDER_BASE (0x1)
/* For generated mipmaps, propagate RGB channels to neighbors if they are fully transparent (ignored if bytesperpixel != 4 ) */
#define IM_CASCADE_FLAGS_COLOR_BORDER_MIPMAPS (0x2)


////


void imPropagateAlphaBorder( unsigned char *imagedata, int width, int height, int bytesperpixel, int bytesperline );


////


#endif

