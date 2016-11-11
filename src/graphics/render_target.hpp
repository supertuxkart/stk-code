//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef HEADER_RENDER_TARGET_HPP
#define HEADER_RENDER_TARGET_HPP

#include <irrlicht.h>
#include <string>

class FrameBuffer;
class RTT;
class ShaderBasedRenderer;

class RenderTarget
{
public:
    virtual ~RenderTarget() {}

    virtual irr::core::dimension2du  getTextureSize()          const = 0;

    virtual void renderToTexture(irr::scene::ICameraSceneNode* camera, float dt) = 0;
    virtual void draw2DImage(const irr::core::rect<irr::s32>& dest_rect,
                             const irr::core::rect<irr::s32>* clip_rect,
                             const irr::video::SColor &colors,
                             bool use_alpha_channel_of_texture) const = 0;    
};

class GL1RenderTarget: public RenderTarget
{
private:
    /** A pointer to texture on which a scene is rendered. Only used
     *  in between beginRenderToTexture() and endRenderToTexture calls. */
    irr::video::ITexture            *m_render_target_texture;

    /** Main node of the RTT scene */
    irr::scene::ISceneNode          *m_rtt_main_node;

public:
    GL1RenderTarget(const irr::core::dimension2du &dimension,
                    const std::string &name);
    ~GL1RenderTarget();

    irr::core::dimension2du getTextureSize() const;

    void renderToTexture(irr::scene::ICameraSceneNode* camera, float dt);
    void draw2DImage(const irr::core::rect<irr::s32>& dest_rect,
                     const irr::core::rect<irr::s32>* clip_rect,
                     const irr::video::SColor &colors,
                     bool use_alpha_channel_of_texture) const;

};

class GL3RenderTarget: public RenderTarget
{
private:
    ShaderBasedRenderer* m_renderer;
    std::string m_name;
    RTT* m_rtts;
    FrameBuffer* m_frame_buffer;

public:
    GL3RenderTarget(const irr::core::dimension2du &dimension,
                    const std::string &name,
                    ShaderBasedRenderer *renderer);
    ~GL3RenderTarget();
    void draw2DImage(const irr::core::rect<irr::s32>& dest_rect,
                     const irr::core::rect<irr::s32>* clip_rect,
                     const irr::video::SColor &colors,
                     bool use_alpha_channel_of_texture) const;
    irr::core::dimension2du getTextureSize() const;
    void renderToTexture(irr::scene::ICameraSceneNode* camera, float dt);
    void setFrameBuffer(FrameBuffer* fb) { m_frame_buffer = fb; }

};

#endif
