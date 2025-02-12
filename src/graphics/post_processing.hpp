//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015 the SuperTuxKart-Team
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

#ifndef HEADER_POST_PROCESSING_HPP
#define HEADER_POST_PROCESSING_HPP

#include "graphics/gl_headers.hpp"

#include <S3DVertex.h>
#include <matrix4.h>
#include <vector>

class FrameBuffer;
class RTT;

namespace irr
{
    namespace video { class IVideoDriver; class ITexture; }
    namespace scene { class ICameraSceneNode; }
}

using namespace irr;

/** \brief   Handles post processing, eg motion blur
 *  \ingroup graphics
 */
class PostProcessing
{
private:
    /** Boost time, how long the boost should be displayed. This also
     *  affects the strength of the effect: longer boost time will
     *  have a stronger effect. */
    std::vector<float>  m_boost_time;

    video::ITexture* m_areamap;

public:
                 PostProcessing();

    void         reset();
    void         update(float dt);

    void renderBloom(GLuint in);
    void renderSSAO(const FrameBuffer& linear_depth_framebuffer,
                    const FrameBuffer& ssao_framebuffer,
                    GLuint depth_stencil_texture);
    /** Blur the in texture */
    void renderGaussian3Blur(const FrameBuffer &in_fbo, const FrameBuffer &auxiliary) const;

    void renderGaussian6Blur(const FrameBuffer &in_fbo, const FrameBuffer &auxiliary,
                              float sigmaV, float sigmaH) const;
	void renderHorizontalBlur(const FrameBuffer &in_fbo, const FrameBuffer &auxiliary) const;

    void renderGaussian17TapBlur(const FrameBuffer &in_fbo,
                                 const FrameBuffer &auxiliary,
                                 const FrameBuffer &linear_depth) const;

    /** Render tex. Used for blit/texture resize */
    void renderPassThrough(unsigned tex, unsigned width, unsigned height) const;
    void renderTextureLayer(unsigned tex, unsigned layer) const;
    
    void renderDoF(const FrameBuffer &framebuffer, GLuint color_texture, GLuint depth_stencil_texture);
    void renderGodRays(scene::ICameraSceneNode * const camnode,
                       const FrameBuffer &in_fbo,
                       const FrameBuffer &tmp_fbo,
                       const FrameBuffer &quarter1_fbo,
                       const FrameBuffer &quarter2_fbo);

    void applyMLAA(const FrameBuffer& mlaa_tmp_framebuffer,
                   const FrameBuffer& mlaa_blend_framebuffer,
                   const FrameBuffer& mlaa_colors_framebuffer);

    void renderMotionBlur(const FrameBuffer &in_fbo,
                          FrameBuffer &out_fbo,
                          GLuint depth_stencil_texture);
    void renderGlow(const FrameBuffer& quarter_framebuffer) const;
    void renderLightning(core::vector3df intensity);

    /** Use motion blur for a short time */
    void         giveBoost(unsigned int cam_index);
    
    /** Render the post-processed scene */
    FrameBuffer *render(scene::ICameraSceneNode * const camnode, bool isRace,
                        RTT *rtts);
};   // class PostProcessing

#endif // HEADER_POST_PROCESSING_HPP
