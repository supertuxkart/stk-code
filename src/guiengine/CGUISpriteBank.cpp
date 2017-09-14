// Copyright (C) 2002-2015 Nikolaus Gebhardt, modified by Marianne Gagnon
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "guiengine/CGUISpriteBank.hpp"
#ifdef _IRR_COMPILE_WITH_GUI_

#include "IGUIEnvironment.h"
#include "IVideoDriver.h"
#include "ITexture.h"
#include <cassert>
#include "graphics/2dutils.hpp"

namespace irr
{
namespace gui
{

STKModifiedSpriteBank::STKModifiedSpriteBank(IGUIEnvironment* env) :
    Environment(env), Driver(0)
{
    m_magic_number = 0xCAFEC001;

    #ifdef _DEBUG
    setDebugName("STKModifiedSpriteBank");
    #endif

    m_scale = 1.0f;
    m_height = 0;

    if (Environment)
    {
        Driver = Environment->getVideoDriver();
        if (Driver)
            Driver->grab();
    }
}   // STKModifiedSpriteBank

// ----------------------------------------------------------------------------
STKModifiedSpriteBank::~STKModifiedSpriteBank()
{
    // drop textures
    for (u32 i=0; i<Textures.size(); ++i)
        if (Textures[i])
            Textures[i]->drop();

    // drop video driver
    if (Driver)
        Driver->drop();

    m_magic_number = 0xDEADBEEF;
}   // ~STKModifiedSpriteBank

// ----------------------------------------------------------------------------
core::array< core::rect<s32> >& STKModifiedSpriteBank::getPositions()
{
    assert( m_magic_number == 0xCAFEC001 );
    copy.clear();

    //FIXME: terribly unefficient, CGUIListBox will call this once for every
    //       item xD but I have no choice, short of re-implementing
    //       IGUIListBox too

    for (int n=0; n<(int)Rectangles.size(); n++)
    {
        const int h = getScaledHeight(Rectangles[n].getHeight());
        const int w = getScaledWidth(Rectangles[n].getWidth());
        copy.push_back( core::rect<s32>(Rectangles[n].UpperLeftCorner,
                                       core::dimension2d<s32>(w,h) )
                       );
    }

    return copy;
}   // getPositions

// ----------------------------------------------------------------------------
core::array< SGUISprite >& STKModifiedSpriteBank::getSprites()
{
    assert( m_magic_number == 0xCAFEC001 );
    return Sprites;
}   // getSprites

// ----------------------------------------------------------------------------
u32 STKModifiedSpriteBank::getTextureCount() const
{
    assert( m_magic_number == 0xCAFEC001 );
    return Textures.size();
}   // getTextureCount

// ----------------------------------------------------------------------------
video::ITexture* STKModifiedSpriteBank::getTexture(u32 index) const
{
    assert( m_magic_number == 0xCAFEC001 );
    if (index < Textures.size())
        return Textures[index];
    else
        return 0;
}   // getTexture

// ----------------------------------------------------------------------------
void STKModifiedSpriteBank::addTexture(video::ITexture* texture)
{
    assert( m_magic_number == 0xCAFEC001 );
    if (texture)
        texture->grab();

    Textures.push_back(texture);
}   // addTexture

// ----------------------------------------------------------------------------
void STKModifiedSpriteBank::setTexture(u32 index, video::ITexture* texture)
{
    assert( m_magic_number == 0xCAFEC001 );
    while (index >= Textures.size())
        Textures.push_back(0);

    if (texture)
        texture->grab();

    if (Textures[index])
        Textures[index]->drop();

    Textures[index] = texture;
}   // setTexture

// ----------------------------------------------------------------------------
//! clear everything
void STKModifiedSpriteBank::clear()
{
    assert( m_magic_number == 0xCAFEC001 );
    // drop textures
    for (u32 i=0; i<Textures.size(); ++i)
        if (Textures[i])
            Textures[i]->drop();
    Textures.clear();
    Sprites.clear();
    Rectangles.clear();
}   // clear

// ----------------------------------------------------------------------------
/** Add the texture and use it for a single non-animated sprite.
 */
s32 STKModifiedSpriteBank::addTextureAsSprite(video::ITexture* texture)
{
    assert( m_magic_number == 0xCAFEC001 );
    if ( !texture )
        return -1;

    addTexture(texture);
    u32 textureIndex = getTextureCount() - 1;

    u32 rectangleIndex = Rectangles.size();
    Rectangles.push_back( core::rect<s32>(0,0,
                                          texture->getSize().Width,
                                          texture->getSize().Height) );

    SGUISprite sprite;
    sprite.frameTime = 0;

    SGUISpriteFrame frame;
    frame.textureNumber = textureIndex;
    frame.rectNumber = rectangleIndex;
    sprite.Frames.push_back( frame );

    Sprites.push_back( sprite );

    return Sprites.size() - 1;
}   // addTextureAsSprite

// ----------------------------------------------------------------------------
//! draws a sprite in 2d with scale and color
void STKModifiedSpriteBank::draw2DSprite(u32 index,
        const core::position2di& pos,
        const core::rect<s32>* clip, const video::SColor& color,
        u32 starttime, u32 currenttime, bool loop, bool center)
{
#ifndef SERVER_ONLY
    assert( m_magic_number == 0xCAFEC001 );
    if (index >= Sprites.size() || Sprites[index].Frames.empty() )
        return;

    // work out frame number
    u32 frame = 0;
    if (Sprites[index].frameTime)
    {
        u32 f = ((currenttime - starttime) / Sprites[index].frameTime);
        if (loop)
            frame = f % Sprites[index].Frames.size();
        else
            frame = (f >= Sprites[index].Frames.size())
                  ? Sprites[index].Frames.size()-1 : f;
    }

    const video::ITexture* tex =
        Textures[Sprites[index].Frames[frame].textureNumber];
    if (!tex)
        return;

    const u32 rn = Sprites[index].Frames[frame].rectNumber;
    if (rn >= Rectangles.size())
        return;

    const core::rect<s32>& r = Rectangles[rn];

    const core::dimension2d<s32>& dim = r.getSize();

    core::rect<s32> dest( pos,
                          core::dimension2d<s32>(getScaledWidth(dim.Width),
                                                 getScaledHeight(dim.Height)));
    if (center)
    {
        dest -= dest.getSize() / 2;
    }

    /*
        draw2DImage  (const video::ITexture  *texture,
                      const core::rect<  s32  > &destRect,
                      const core::rect<  s32  > &sourceRect,
                      const core::rect<  s32  > *clipRect=0,
                      const video::SColor  *const colors=0,
                      bool useAlphaChannelOfTexture=false)=0
     */
    draw2DImage(tex, dest, r /* source rect */, clip,
                        NULL /* colors */, true);
#endif
}   // draw2DSprite

// ----------------------------------------------------------------------------
void STKModifiedSpriteBank::draw2DSpriteBatch(const core::array<u32>& indices,
                            const core::array<core::position2di>& pos,
                            const core::rect<s32>* clip,
                            const video::SColor& color,
                            u32 starttime, u32 currenttime,
                            bool loop, bool center)
{
    assert( m_magic_number == 0xCAFEC001 );
    const irr::u32 drawCount = core::min_<u32>(indices.size(), pos.size());

    core::array<SDrawBatch> drawBatches(Textures.size());
    for(u32 i = 0;i < Textures.size();i++)
    {
        drawBatches.push_back(SDrawBatch());
        drawBatches[i].positions.reallocate(drawCount);
        drawBatches[i].sourceRects.reallocate(drawCount);
    }

    for(u32 i = 0;i < drawCount;i++)
    {
        const u32 index = indices[i];

        if (index >= Sprites.size() || Sprites[index].Frames.empty() )
            continue;

        // work out frame number
        u32 frame = 0;
        if (Sprites[index].frameTime)
        {
            u32 f = ((currenttime - starttime) / Sprites[index].frameTime);
            if (loop)
                frame = f % Sprites[index].Frames.size();
            else
                frame = (f >= Sprites[index].Frames.size())
                      ? Sprites[index].Frames.size()-1 : f;
        }

        const u32 texNum = Sprites[index].Frames[frame].textureNumber;

        SDrawBatch& currentBatch = drawBatches[texNum];

        const u32 rn = Sprites[index].Frames[frame].rectNumber;
        if (rn >= Rectangles.size())
            return;

        const core::rect<s32>& r = Rectangles[rn];

        if (center)
        {
            core::position2di p = pos[i];
            p -= r.getSize() / 2;

            currentBatch.positions.push_back(p);
            currentBatch.sourceRects.push_back(r);
        }
        else
        {
            currentBatch.positions.push_back(pos[i]);
            currentBatch.sourceRects.push_back(r);
        }
    }

    for(u32 i = 0;i < drawBatches.size();i++)
    {
        if(!drawBatches[i].positions.empty() &&
            !drawBatches[i].sourceRects.empty())
            Driver->draw2DImageBatch(Textures[i], drawBatches[i].positions,
                drawBatches[i].sourceRects, clip, color, true);
    }
}   // draw2DSpriteBatch

// ----------------------------------------------------------------------------
void STKModifiedSpriteBank::scaleToHeight(int height)
{
    m_height = height;
}

// ----------------------------------------------------------------------------
s32 STKModifiedSpriteBank::getScaledWidth(s32 width) const
{
    if (m_height == 0)
        return (s32)((float)width * m_scale);
    else
        return m_height;
}

// ----------------------------------------------------------------------------
s32 STKModifiedSpriteBank::getScaledHeight(s32 height) const
{
    if (m_height == 0)
        return (s32)((float)height * m_scale);
    else
        return m_height;
}

// ----------------------------------------------------------------------------
} // namespace gui
} // namespace irr

#endif // _IRR_COMPILE_WITH_GUI_

