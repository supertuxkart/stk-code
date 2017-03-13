/* *****************************************************************************
 *
 * Copyright (c) 2007-2016 Alexis Naveros.
 * Portions developed under contract to the SURVICE Engineering Company.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 *
 * *****************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <math.h>


#include "cpusimd.h"

#include "img.h"


#ifndef ADDRESS
 #define ADDRESS(p,o) ((void *)(((char *)p)+(o)))
#endif


////


void imgCopyRect( imgImage *image, int dstx, int dsty, int srcx, int srcy, int sizex, int sizey )
{
  int y;
  void *dst, *src;
  src = ADDRESS( image->data, ( srcx * image->format.bytesperpixel ) + ( srcy * image->format.bytesperline ) );
  dst = ADDRESS( image->data, ( dstx * image->format.bytesperpixel ) + ( dsty * image->format.bytesperline ) );
  for( y = 0 ; y < sizey ; y++ )
  {
    memcpy( dst, src, sizex * image->format.bytesperpixel );
    src = ADDRESS( src, image->format.bytesperline );
    dst = ADDRESS( dst, image->format.bytesperline );
  }
  return;
}


#if CPU_SSE2_SUPPORT
static const uint16_t CPU_ALIGN16 imgBlendRgbMask[8] = { 0xffff, 0xffff, 0xffff, 0x0000, 0xffff, 0xffff, 0xffff, 0x0000 };
static const uint8_t CPU_ALIGN16 imgBlendAlphaTestMask[16] = { 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0xff };
static const uint16_t CPU_ALIGN16 imgBlendRoundBias[8] = { 128, 128, 128, 128, 128, 128, 128, 128 };
 #if CPU_SSSE3_SUPPORT
static const uint8_t CPU_ALIGN16 imgBlendShufMask[16] = { 6,7,6,7,6,7,6,7, 14,15,14,15,14,15,14,15 };
 #endif
#endif

static void imgBlendImageRgba2Rgba( imgImage *dstimage, int dstx, int dsty, imgImage *srcimage )
{
  int x, y;
#if CPU_SSE2_SUPPORT
  int row4size;
  __m128i vsrc01, vsrc23, vdst01, vdst23, vblend01, vblend23;
  __m128i vzero, v255, vrgbmask, valphatest, vroundbias;
 #if CPU_SSSE3_SUPPORT
  __m128i vshufmask;
 #endif
#else
  int32_t dstr, dstg, dstb, dsta;
  int32_t srcr, srcg, srcb, srca;
#endif
  unsigned char *src, *srcrow, *dstrow;
  uint32_t *dst;

  /* TODO: Other function to clamp copy area? */

#if CPU_SSE2_SUPPORT
  row4size = srcimage->format.width & ~3;
  vzero = _mm_setzero_si128();
  v255 = _mm_set1_epi16( 255 );
  vrgbmask = _mm_load_si128( (void *)imgBlendRgbMask );
  valphatest = _mm_load_si128( (void *)imgBlendAlphaTestMask );
  vroundbias = _mm_load_si128( (void *)imgBlendRoundBias );
 #if CPU_SSSE3_SUPPORT
  vshufmask = _mm_load_si128( (void *)imgBlendShufMask );
 #endif
#endif

  src = srcimage->data;
  dst = ADDRESS( dstimage->data, ( dstx * 4 ) + ( dsty * dstimage->format.bytesperline ) );
  for( y = 0 ; y < srcimage->format.height ; y++ )
  {
    srcrow = src;
    dstrow = (unsigned char *)dst;

#if CPU_SSE2_SUPPORT
    for( x = 0 ; x < row4size ; x += 4, srcrow += 16, dstrow += 16 )
    {
      /* r0g0b0a0,r1g1b1a1,r2g2b2a2,r3g3b3a3 */
      vsrc23 = _mm_loadu_si128( (void *)srcrow );
      if( _mm_movemask_ps( _mm_castsi128_ps( _mm_cmpeq_epi32( _mm_and_si128( valphatest, vsrc23 ), vzero ) ) ) == 0xf )
        continue;
      vdst23 = _mm_loadu_si128( (void *)dstrow );
      /* r0__g0__b0__a0__, r1__g1__b1__a1__ */
      vsrc01 = _mm_unpacklo_epi8( vsrc23, vzero );
      vdst01 = _mm_unpacklo_epi8( vdst23, vzero );
      /* r2__g2__b2__a2__, r3__g3__b3__a3__ */
      vsrc23 = _mm_unpackhi_epi8( vsrc23, vzero );
      vdst23 = _mm_unpackhi_epi8( vdst23, vzero );
 #if CPU_SSSE3_SUPPORT
      /* __a0__a0__a0__a0, __a1__a1__a1__a1 */
      vblend01 = _mm_shuffle_epi8( vsrc01, vshufmask );
      /* __a2__a2__a2__a2, __a3__a3__a3__a3 */
      vblend23 = _mm_shuffle_epi8( vsrc23, vshufmask );
 #else
      vblend01 = _mm_shufflelo_epi16( vsrc01, 0xff );
      vblend01 = _mm_shufflehi_epi16( vblend01, 0xff );
      vblend23 = _mm_shufflelo_epi16( vsrc23, 0xff );
      vblend23 = _mm_shufflehi_epi16( vblend23, 0xff );
 #endif
      vdst01 = _mm_adds_epu16( _mm_adds_epu16( _mm_mullo_epi16( vdst01, _mm_sub_epi16( v255, _mm_and_si128( vblend01, vrgbmask ) ) ), _mm_mullo_epi16( vsrc01, vblend01 ) ), vroundbias );
      vdst23 = _mm_adds_epu16( _mm_adds_epu16( _mm_mullo_epi16( vdst23, _mm_sub_epi16( v255, _mm_and_si128( vblend23, vrgbmask ) ) ), _mm_mullo_epi16( vsrc23, vblend23 ) ), vroundbias );
      /* Correction to divide by 255 instead of 256 */
      vdst01 = _mm_srli_epi16( _mm_adds_epu16( vdst01, _mm_srli_epi16( vdst01, 8 ) ), 8 );
      vdst23 = _mm_srli_epi16( _mm_adds_epu16( vdst23, _mm_srli_epi16( vdst23, 8 ) ), 8 );
      /* Combine interleaved and store */
      _mm_storeu_si128( (void *)dstrow, _mm_packus_epi16( vdst01, vdst23 ) );
    }
    for( ; x < srcimage->format.width ; x++, srcrow += 4, dstrow += 4 )
    {
      if( !( srcrow[3] ) )
        continue;
      vsrc01 = _mm_castps_si128( _mm_load_ss( (void *)srcrow ) );
      vdst01 = _mm_castps_si128( _mm_load_ss( (void *)dstrow ) );
      vsrc01 = _mm_unpacklo_epi8( vsrc01, vzero );
      vdst01 = _mm_unpacklo_epi8( vdst01, vzero );
 #if CPU_SSSE3_SUPPORT
      vblend01 = _mm_shuffle_epi8( vsrc01, vshufmask );
 #else
      vblend01 = _mm_shufflelo_epi16( vsrc01, 0xff );
      vblend01 = _mm_shufflehi_epi16( vblend01, 0xff );
 #endif
      vdst01 = _mm_adds_epu16( _mm_adds_epu16( _mm_mullo_epi16( vdst01, _mm_sub_epi16( v255, _mm_and_si128( vblend01, vrgbmask ) ) ), _mm_mullo_epi16( vsrc01, vblend01 ) ), vroundbias );
      /* Correction to divide by 255 instead of 256 */
      vdst01 = _mm_srli_epi16( _mm_adds_epu16( vdst01, _mm_srli_epi16( vdst01, 8 ) ), 8 );
      _mm_store_ss( (void *)dstrow, _mm_castsi128_ps( _mm_packus_epi16( vdst01, vdst01 ) ) );
    }
#else
    for( x = 0 ; x < srcimage->format.width ; x++, srcrow += 4, dstrow += 4 )
    {
      if( !( srcrow[3] ) )
        continue;
      srcr = (int32_t)srcrow[0];
      srcg = (int32_t)srcrow[1];
      srcb = (int32_t)srcrow[2];
      srca = (int32_t)srcrow[3];
      dstr = (int32_t)dstrow[0];
      dstg = (int32_t)dstrow[1];
      dstb = (int32_t)dstrow[2];
      dsta = (int32_t)dstrow[3];
      dstr = ( ( dstr << 8 ) - dstr + ( srca * ( srcr - dstr ) ) + 128 );
      dstg = ( ( dstg << 8 ) - dstg + ( srca * ( srcg - dstg ) ) + 128 );
      dstb = ( ( dstb << 8 ) - dstb + ( srca * ( srcb - dstb ) ) + 128 );
      dsta = ( ( dsta << 8 ) - dsta + ( srca * srca ) + 128 );
      dstr = ( dstr + ( dstr >> 8 ) ) >> 8;
      dstg = ( dstg + ( dstg >> 8 ) ) >> 8;
      dstb = ( dstb + ( dstb >> 8 ) ) >> 8;
      dsta = ( dsta + ( dsta >> 8 ) ) >> 8;
      if( dsta > 255 )
        dsta = 255;
      dstrow[0] = (unsigned char)dstr;
      dstrow[1] = (unsigned char)dstg;
      dstrow[2] = (unsigned char)dstb;
      dstrow[3] = (unsigned char)dsta;
    }
#endif
    src = ADDRESS( src, srcimage->format.bytesperline );
    dst = ADDRESS( dst, dstimage->format.bytesperline );
  }

  return;
}

static void imgBlendImageRgba2Rgbx( imgImage *dstimage, int dstx, int dsty, imgImage *srcimage )
{
  int x, y;
#if CPU_SSE2_SUPPORT
  int row4size;
  __m128i vsrc01, vsrc23, vdst01, vdst23, vblend01, vblend23;
  __m128i vzero, v255, valphatest, vroundbias;
 #if CPU_SSSE3_SUPPORT
  __m128i vshufmask;
 #endif
#else
  int32_t dstr, dstg, dstb;
  int32_t srcr, srcg, srcb, srca;
#endif
  unsigned char *src, *srcrow, *dstrow;
  uint32_t *dst;

  /* TODO: Other function to clamp copy area? */

#if CPU_SSE2_SUPPORT
  row4size = srcimage->format.width & ~3;
  vzero = _mm_setzero_si128();
  v255 = _mm_set1_epi16( 255 );
  valphatest = _mm_load_si128( (void *)imgBlendAlphaTestMask );
  vroundbias = _mm_load_si128( (void *)imgBlendRoundBias );
 #if CPU_SSSE3_SUPPORT
  vshufmask = _mm_load_si128( (void *)imgBlendShufMask );
 #endif
#endif

  src = srcimage->data;
  dst = ADDRESS( dstimage->data, ( dstx * 4 ) + ( dsty * dstimage->format.bytesperline ) );
  for( y = 0 ; y < srcimage->format.height ; y++ )
  {
    srcrow = src;
    dstrow = (unsigned char *)dst;

#if CPU_SSE2_SUPPORT
    for( x = 0 ; x < row4size ; x += 4, srcrow += 16, dstrow += 16 )
    {
      /* r0g0b0a0,r1g1b1a1,r2g2b2a2,r3g3b3a3 */
      vsrc23 = _mm_loadu_si128( (void *)srcrow );
      if( _mm_movemask_ps( _mm_castsi128_ps( _mm_cmpeq_epi32( _mm_and_si128( valphatest, vsrc23 ), vzero ) ) ) == 0xf )
        continue;
      vdst23 = _mm_loadu_si128( (void *)dstrow );
      /* r0__g0__b0__a0__, r1__g1__b1__a1__ */
      vsrc01 = _mm_unpacklo_epi8( vsrc23, vzero );
      vdst01 = _mm_unpacklo_epi8( vdst23, vzero );
      /* r2__g2__b2__a2__, r3__g3__b3__a3__ */
      vsrc23 = _mm_unpackhi_epi8( vsrc23, vzero );
      vdst23 = _mm_unpackhi_epi8( vdst23, vzero );
 #if CPU_SSSE3_SUPPORT
      /* __a0__a0__a0__a0, __a1__a1__a1__a1 */
      vblend01 = _mm_shuffle_epi8( vsrc01, vshufmask );
      /* __a2__a2__a2__a2, __a3__a3__a3__a3 */
      vblend23 = _mm_shuffle_epi8( vsrc23, vshufmask );
 #else
      vblend01 = _mm_shufflelo_epi16( vsrc01, 0xff );
      vblend01 = _mm_shufflehi_epi16( vblend01, 0xff );
      vblend23 = _mm_shufflelo_epi16( vsrc23, 0xff );
      vblend23 = _mm_shufflehi_epi16( vblend23, 0xff );
 #endif
      vdst01 = _mm_adds_epu16( _mm_adds_epu16( _mm_mullo_epi16( vdst01, _mm_sub_epi16( v255, vblend01 ) ), _mm_mullo_epi16( vsrc01, vblend01 ) ), vroundbias );
      vdst23 = _mm_adds_epu16( _mm_adds_epu16( _mm_mullo_epi16( vdst23, _mm_sub_epi16( v255, vblend23 ) ), _mm_mullo_epi16( vsrc23, vblend23 ) ), vroundbias );
      /* Correction to divide by 255 instead of 256 */
      vdst01 = _mm_srli_epi16( _mm_adds_epu16( vdst01, _mm_srli_epi16( vdst01, 8 ) ), 8 );
      vdst23 = _mm_srli_epi16( _mm_adds_epu16( vdst23, _mm_srli_epi16( vdst23, 8 ) ), 8 );
      /* Combine interleaved and store */
      _mm_storeu_si128( (void *)dstrow, _mm_or_si128( _mm_packus_epi16( vdst01, vdst23 ), valphatest ) );
    }
    for( ; x < srcimage->format.width ; x++, srcrow += 4, dstrow += 4 )
    {
      if( !( srcrow[3] ) )
        continue;
      vsrc01 = _mm_castps_si128( _mm_load_ss( (void *)srcrow ) );
      vdst01 = _mm_castps_si128( _mm_load_ss( (void *)dstrow ) );
      vsrc01 = _mm_unpacklo_epi8( vsrc01, vzero );
      vdst01 = _mm_unpacklo_epi8( vdst01, vzero );
 #if CPU_SSSE3_SUPPORT
      vblend01 = _mm_shuffle_epi8( vsrc01, vshufmask );
 #else
      vblend01 = _mm_shufflelo_epi16( vsrc01, 0xff );
      vblend01 = _mm_shufflehi_epi16( vblend01, 0xff );
 #endif
      vdst01 = _mm_adds_epu16( _mm_adds_epu16( _mm_mullo_epi16( vdst01, _mm_sub_epi16( v255, vblend01 ) ), _mm_mullo_epi16( vsrc01, vblend01 ) ), vroundbias );
      /* Correction to divide by 255 instead of 256 */
      vdst01 = _mm_srli_epi16( _mm_adds_epu16( vdst01, _mm_srli_epi16( vdst01, 8 ) ), 8 );
      _mm_store_ss( (void *)dstrow, _mm_castsi128_ps( _mm_or_si128( _mm_packus_epi16( vdst01, vdst01 ), valphatest ) ) );
    }
#else
    for( x = 0 ; x < srcimage->format.width ; x++, srcrow += 4, dstrow += 4 )
    {
      if( !( srcrow[3] ) )
        continue;
      srcr = (int32_t)srcrow[0];
      srcg = (int32_t)srcrow[1];
      srcb = (int32_t)srcrow[2];
      srca = (int32_t)srcrow[3];
      dstr = (int32_t)dstrow[0];
      dstg = (int32_t)dstrow[1];
      dstb = (int32_t)dstrow[2];
      dstr = ( ( dstr << 8 ) - dstr + ( srca * ( srcr - dstr ) ) + 128 );
      dstg = ( ( dstg << 8 ) - dstg + ( srca * ( srcg - dstg ) ) + 128 );
      dstb = ( ( dstb << 8 ) - dstb + ( srca * ( srcb - dstb ) ) + 128 );
      dstr = ( dstr + ( dstr >> 8 ) ) >> 8;
      dstg = ( dstg + ( dstg >> 8 ) ) >> 8;
      dstb = ( dstb + ( dstb >> 8 ) ) >> 8;
      dstrow[0] = (unsigned char)dstr;
      dstrow[1] = (unsigned char)dstg;
      dstrow[2] = (unsigned char)dstb;
      dstrow[3] = (unsigned char)255;
    }
#endif
    src = ADDRESS( src, srcimage->format.bytesperline );
    dst = ADDRESS( dst, dstimage->format.bytesperline );
  }

  return;
}

static void imgBlendImageRgba2Rgb( imgImage *dstimage, int dstx, int dsty, imgImage *srcimage )
{
  int x, y;
  int32_t dstr, dstg, dstb;
  int32_t srcr, srcg, srcb, srca;
  unsigned char *src, *srcrow, *dstrow;
  uint32_t *dst;

  /* TODO: Other function to clamp copy area? */

  src = srcimage->data;
  dst = ADDRESS( dstimage->data, ( dstx * 3 ) + ( dsty * dstimage->format.bytesperline ) );
  for( y = 0 ; y < srcimage->format.height ; y++ )
  {
    srcrow = src;
    dstrow = (unsigned char *)dst;
    for( x = 0 ; x < srcimage->format.width ; x++, srcrow += 4, dstrow += 3 )
    {
      if( !( srcrow[3] ) )
        continue;
      srcr = (int32_t)srcrow[0];
      srcg = (int32_t)srcrow[1];
      srcb = (int32_t)srcrow[2];
      srca = (int32_t)srcrow[3];
      dstr = (int32_t)dstrow[0];
      dstg = (int32_t)dstrow[1];
      dstb = (int32_t)dstrow[2];
      dstr = ( ( dstr << 8 ) - dstr + ( srca * ( srcr - dstr ) ) + 128 );
      dstg = ( ( dstg << 8 ) - dstg + ( srca * ( srcg - dstg ) ) + 128 );
      dstb = ( ( dstb << 8 ) - dstb + ( srca * ( srcb - dstb ) ) + 128 );
      dstr = ( dstr + ( dstr >> 8 ) ) >> 8;
      dstg = ( dstg + ( dstg >> 8 ) ) >> 8;
      dstb = ( dstb + ( dstb >> 8 ) ) >> 8;
      dstrow[0] = (unsigned char)dstr;
      dstrow[1] = (unsigned char)dstg;
      dstrow[2] = (unsigned char)dstb;
    }
    src = ADDRESS( src, srcimage->format.bytesperline );
    dst = ADDRESS( dst, dstimage->format.bytesperline );
  }

  return;
}


void (*imgBlendGetFunction( imgImage *dstimage, imgImage *srcimage ))( imgImage *dstimage, int dstx, int dsty, imgImage *srcimage )
{
  void (*blendfunc)( imgImage *dstimage, int dstx, int dsty, imgImage *srcimage );
  blendfunc = 0;
  if( srcimage->format.bytesperpixel == 4 )
  {
    if( dstimage->format.bytesperpixel == 4 )
    {
      if( ( dstimage->format.type == IMG_FORMAT_TYPE_RGBA32 ) || ( dstimage->format.type == IMG_FORMAT_TYPE_BGRA32 ) )
        blendfunc = imgBlendImageRgba2Rgba;
      else
        blendfunc = imgBlendImageRgba2Rgbx;
    }
    else if( dstimage->format.bytesperpixel == 3 )
      blendfunc = imgBlendImageRgba2Rgb;
  }
  return blendfunc;
}


int imgBlendImage( imgImage *dstimage, int dstx, int dsty, imgImage *srcimage )
{
  void (*blendfunc)( imgImage *dstimage, int dstx, int dsty, imgImage *srcimage );
  blendfunc = imgBlendGetFunction( dstimage, srcimage );
  if( blendfunc )
  {
    blendfunc( dstimage, dstx, dsty, srcimage );
    return 1;
  }
  return 0;
}


////


void imgAllocCopy( imgImage *dstimage, imgImage *srcimage )
{
  dstimage->format = srcimage->format;
  dstimage->data = malloc( srcimage->format.height * srcimage->format.bytesperline );
  memcpy( dstimage->data, srcimage->data, srcimage->format.height * srcimage->format.bytesperline );
  return;
}


void imgAllocCopyExtendBorder( imgImage *dstimage, imgImage *srcimage, int extendsize )
{
  int y;
  void *dst, *src, *dstrow;

  dstimage->format.width = srcimage->format.width + ( extendsize << 1 );
  dstimage->format.height = srcimage->format.height + ( extendsize << 1 );
  dstimage->format.type = srcimage->format.type;
  dstimage->format.bytesperpixel = srcimage->format.bytesperpixel;
  dstimage->format.bytesperline = dstimage->format.width * dstimage->format.bytesperpixel;
  dstimage->data = malloc( dstimage->format.height * dstimage->format.bytesperline );

  src = srcimage->data;
  dst = dstimage->data;
  for( y = 0 ; y < extendsize ; y++ )
  {
    memset( dst, 0, dstimage->format.bytesperline );
    dst = ADDRESS( dst, dstimage->format.bytesperline );
  }
  for( y = 0 ; y < srcimage->format.height ; y++ )
  {
    dstrow = dst;
    memset( dstrow, 0, extendsize * dstimage->format.bytesperpixel );
    dstrow = ADDRESS( dstrow, extendsize * dstimage->format.bytesperpixel );
    memcpy( dstrow, src, srcimage->format.width * dstimage->format.bytesperpixel );
    dstrow = ADDRESS( dstrow, srcimage->format.width * dstimage->format.bytesperpixel );
    memset( dstrow, 0, extendsize * dstimage->format.bytesperpixel );
    src = ADDRESS( src, srcimage->format.bytesperline );
    dst = ADDRESS( dst, dstimage->format.bytesperline );
  }
  for( y = 0 ; y < extendsize ; y++ )
  {
    memset( dst, 0, dstimage->format.bytesperline );
    dst = ADDRESS( dst, dstimage->format.bytesperline );
  }

  return;
}


void imgAllocExtractChannel( imgImage *dstimage, imgImage *srcimage, int channelindex )
{
  int x, y;
  unsigned char *dst, *src, *srcrow;

  dstimage->format.width = srcimage->format.width;
  dstimage->format.height = srcimage->format.height;
  dstimage->format.type = IMG_FORMAT_TYPE_GRAYSCALE;
  dstimage->format.bytesperpixel = 1;
  dstimage->format.bytesperline = dstimage->format.width * dstimage->format.bytesperpixel;
  dstimage->data = malloc( dstimage->format.height * dstimage->format.bytesperline );

  src = ADDRESS( srcimage->data, channelindex );
  dst = dstimage->data;
  for( y = 0 ; y < dstimage->format.height ; y++ )
  {
    srcrow = src;
    for( x = 0 ; x < dstimage->format.width ; x++ )
    {
      dst[x] = *srcrow;
      srcrow = ADDRESS( srcrow, srcimage->format.bytesperpixel );
    }
    src = ADDRESS( src, srcimage->format.bytesperline );
    dst = ADDRESS( dst, dstimage->format.bytesperline );
  }

  return;
}


void imgAllocExtractChannelExtendBorder( imgImage *dstimage, imgImage *srcimage, int channelindex, int extendsize )
{
  int x, y;
  unsigned char *src, *dst, *srcrow, *dstrow;

  dstimage->format.width = srcimage->format.width + ( extendsize << 1 );
  dstimage->format.height = srcimage->format.height + ( extendsize << 1 );
  dstimage->format.type = IMG_FORMAT_TYPE_GRAYSCALE;
  dstimage->format.bytesperpixel = 1;
  dstimage->format.bytesperline = dstimage->format.width * dstimage->format.bytesperpixel;
  dstimage->data = malloc( dstimage->format.height * dstimage->format.bytesperline );

  src = ADDRESS( srcimage->data, channelindex );
  dst = dstimage->data;
  for( y = 0 ; y < extendsize ; y++ )
  {
    memset( dst, 0, dstimage->format.bytesperline );
    dst = ADDRESS( dst, dstimage->format.bytesperline );
  }
  for( y = 0 ; y < srcimage->format.height ; y++ )
  {
    srcrow = src;
    dstrow = dst;
    memset( dstrow, 0, extendsize * dstimage->format.bytesperpixel );
    dstrow = ADDRESS( dstrow, extendsize * dstimage->format.bytesperpixel );
    for( x = 0 ; x < srcimage->format.width ; x++ )
    {
      dstrow[x] = *srcrow;
      srcrow = ADDRESS( srcrow, srcimage->format.bytesperpixel );
    }
    dstrow = ADDRESS( dstrow, srcimage->format.width * dstimage->format.bytesperpixel );
    memset( dstrow, 0, extendsize * dstimage->format.bytesperpixel );
    src = ADDRESS( src, srcimage->format.bytesperline );
    dst = ADDRESS( dst, dstimage->format.bytesperline );
  }
  for( y = 0 ; y < extendsize ; y++ )
  {
    memset( dst, 0, dstimage->format.bytesperline );
    dst = ADDRESS( dst, dstimage->format.bytesperline );
  }

  return;
}


void imgAllocCopyChannelToAlpha( imgImage *dstimage, imgImage *srcimage, int channelindex, unsigned char r, unsigned char g, unsigned char b )
{
  int x, y;
  unsigned char *dst, *src, *dstrow, *srcrow;

  dstimage->format.width = srcimage->format.width;
  dstimage->format.height = srcimage->format.height;
  dstimage->format.type = IMG_FORMAT_TYPE_RGBA32;
  dstimage->format.bytesperpixel = 4;
  dstimage->format.bytesperline = dstimage->format.width * dstimage->format.bytesperpixel;
  dstimage->data = malloc( dstimage->format.height * dstimage->format.bytesperline );

  src = ADDRESS( srcimage->data, channelindex );
  dst = dstimage->data;
  for( y = 0 ; y < dstimage->format.height ; y++ )
  {
    srcrow = src;
    dstrow = dst;
    for( x = 0 ; x < dstimage->format.width ; x++ )
    {
      dstrow[0] = r;
      dstrow[1] = g;
      dstrow[2] = b;
      dstrow[3] = *srcrow;
      srcrow = ADDRESS( srcrow, srcimage->format.bytesperpixel );
      dstrow = ADDRESS( dstrow, dstimage->format.bytesperpixel );
    }
    src = ADDRESS( src, srcimage->format.bytesperline );
    dst = ADDRESS( dst, dstimage->format.bytesperline );
  }

  return;
}


void imgAllocAdjustBrightnessContrast( imgImage *dstimage, imgImage *srcimage, float brightness, float contrast )
{
  int x, y;
  float r, g, b;
  unsigned char *dst, *src, *dstrow, *srcrow;

  dstimage->format = srcimage->format;
  dstimage->data = malloc( srcimage->format.height * srcimage->format.bytesperline );

  brightness += 0.5f;

  if( dstimage->format.bytesperpixel >= 3 )
  {
    src = srcimage->data;
    dst = dstimage->data;
    for( y = 0 ; y < dstimage->format.height ; y++ )
    {
      srcrow = src;
      dstrow = dst;
      for( x = 0 ; x < dstimage->format.width ; x++ )
      {
        r = (1.0f/255.0f) * (float)srcrow[0];
        g = (1.0f/255.0f) * (float)srcrow[1];
        b = (1.0f/255.0f) * (float)srcrow[2];
        r = ( ( r - 0.5f ) * contrast ) + brightness;
        g = ( ( g - 0.5f ) * contrast ) + brightness;
        b = ( ( b - 0.5f ) * contrast ) + brightness;
        dstrow[0] = (unsigned char)fmaxf( 0.0f, fminf( 255.0f, roundf( r * 255.0f ) ) );
        dstrow[1] = (unsigned char)fmaxf( 0.0f, fminf( 255.0f, roundf( g * 255.0f ) ) );
        dstrow[2] = (unsigned char)fmaxf( 0.0f, fminf( 255.0f, roundf( b * 255.0f ) ) );
        if( dstimage->format.bytesperpixel >= 4 )
          dstrow[3] = srcrow[3];
        srcrow = ADDRESS( srcrow, srcimage->format.bytesperpixel );
        dstrow = ADDRESS( dstrow, dstimage->format.bytesperpixel );
      }
      src = ADDRESS( src, srcimage->format.bytesperline );
      dst = ADDRESS( dst, dstimage->format.bytesperline );
    }
  }
  else if( dstimage->format.bytesperpixel == 1 )
  {
    src = srcimage->data;
    dst = dstimage->data;
    for( y = 0 ; y < dstimage->format.height ; y++ )
    {
      srcrow = src;
      dstrow = dst;
      for( x = 0 ; x < dstimage->format.width ; x++ )
      {
        r = (1.0f/255.0f) * (float)srcrow[0];
        r = ( ( r - 0.5f ) * contrast ) + brightness;
        dstrow[0] = (unsigned char)fmaxf( 0.0f, fminf( 255.0f, roundf( r * 255.0f ) ) );
        srcrow = ADDRESS( srcrow, srcimage->format.bytesperpixel );
        dstrow = ADDRESS( dstrow, dstimage->format.bytesperpixel );
      }
      src = ADDRESS( src, srcimage->format.bytesperline );
      dst = ADDRESS( dst, dstimage->format.bytesperline );
    }
  }

  return;
}


void imgFree( imgImage *image )
{
  free( image->data );
  image->data = 0;
  return;
}


////


