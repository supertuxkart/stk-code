#ifndef UTILS2D_HPP
#define UTILS2D_HPP

#include "gl_headers.hpp"
#include <rect.h>
#include <SColor.h>
#include <S3DVertex.h>
#include <SVertexIndex.h>
#include <EPrimitiveTypes.h>
#include <ITexture.h>
#include <irrTypes.h>

void draw2DImageFromRTT(GLuint texture, size_t texture_w, size_t texture_h,
    const irr::core::rect<irr::s32>& destRect,
    const irr::core::rect<irr::s32>& sourceRect, const irr::core::rect<irr::s32>* clipRect,
    const irr::video::SColor &colors, bool useAlphaChannelOfTexture);

void draw2DImage(const irr::video::ITexture* texture, const irr::core::rect<irr::s32>& destRect,
    const irr::core::rect<irr::s32>& sourceRect, const irr::core::rect<irr::s32>* clipRect,
    const irr::video::SColor &color, bool useAlphaChannelOfTexture);

void draw2DImage(const irr::video::ITexture* texture, const irr::core::rect<irr::s32>& destRect,
    const irr::core::rect<irr::s32>& sourceRect, const irr::core::rect<irr::s32>* clipRect,
    const irr::video::SColor* const colors, bool useAlphaChannelOfTexture);

void draw2DVertexPrimitiveList(irr::video::ITexture *t, const void* vertices,
    irr::u32 vertexCount, const void* indexList, irr::u32 primitiveCount,
    irr::video::E_VERTEX_TYPE vType = irr::video::EVT_STANDARD, irr::scene::E_PRIMITIVE_TYPE pType = irr::scene::EPT_TRIANGLES, irr::video::E_INDEX_TYPE iType = irr::video::EIT_16BIT);

void GL32_draw2DRectangle(irr::video::SColor color, const irr::core::rect<irr::s32>& position,
    const irr::core::rect<irr::s32>* clip = 0);

#endif