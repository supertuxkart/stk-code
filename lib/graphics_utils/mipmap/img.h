/* -----------------------------------------------------------------------------
 *
 * Copyright (c) 2007-2017 Alexis Naveros.
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

#ifndef IMG_H
#define IMG_H


typedef struct
{
  int width;
  int height;
  int type;
  int bytesperpixel;
  int bytesperline;
} imgFormat;

enum
{
  IMG_FORMAT_TYPE_ANY,
  IMG_FORMAT_TYPE_RGB24,
  IMG_FORMAT_TYPE_BGR24,
  IMG_FORMAT_TYPE_RGBX32,
  IMG_FORMAT_TYPE_BGRX32,
  IMG_FORMAT_TYPE_RGBA32,
  IMG_FORMAT_TYPE_BGRA32,
  IMG_FORMAT_TYPE_GRAYSCALE,
  IMG_FORMAT_TYPE_GRAYALPHA
};

typedef struct
{
  imgFormat format;
  void *data;
} imgImage;


////


void imgCopyRect( imgImage *image, int dstx, int dsty, int srcx, int srcy, int sizex, int sizey );

void (*imgBlendGetFunction( imgImage *dstimage, imgImage *srcimage ))( imgImage *dstimage, int dstx, int dsty, imgImage *srcimage );
int imgBlendImage( imgImage *dstimage, int dstx, int dsty, imgImage *srcimage );

void imgAllocCopy( imgImage *dst, imgImage *src );
void imgAllocCopyExtendBorder( imgImage *dstimage, imgImage *srcimage, int extendsize );
void imgAllocExtractChannel( imgImage *dst, imgImage *src, int channelindex );
void imgAllocExtractChannelExtendBorder( imgImage *dstimage, imgImage *srcimage, int channelindex, int extendsize );
void imgAllocCopyChannelToAlpha( imgImage *dstimage, imgImage *srcimage, int channelindex, unsigned char r, unsigned char g, unsigned char b );
void imgAllocAdjustBrightnessContrast( imgImage *dstimage, imgImage *srcimage, float brightness, float contrast );

void imgFree( imgImage *image );


#endif

