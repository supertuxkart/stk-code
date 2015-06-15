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

#ifndef HEADER_RTTS_HPP
#define HEADER_RTTS_HPP

#include "graphics/irr_driver.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/leak_check.hpp"

class FrameBuffer;

namespace irr {
    namespace video {
        class ITexture;
    };
    namespace scene {
        class ICameraSceneNode;
    }
};

using irr::video::ITexture;


class RTT
{
public:
    RTT(size_t width, size_t height);
    ~RTT();

    FrameBuffer &getShadowFBO() { return *m_shadow_FBO; }
    FrameBuffer &getRH() { return *m_RH_FBO; }
    FrameBuffer &getRSM() { return *m_RSM; }

    unsigned getDepthStencilTexture() const { return DepthStencilTexture; }
    unsigned getRenderTarget(enum TypeRTT target) const { return RenderTargetTextures[target]; }
    FrameBuffer& getFBO(enum TypeFBO fbo) { return FrameBuffers[fbo]; }

    FrameBuffer* render(irr::scene::ICameraSceneNode* camera, float dt);

    void prepareRender(scene::ICameraSceneNode* camera);

private:
    unsigned RenderTargetTextures[RTT_COUNT];
    PtrVector<FrameBuffer> FrameBuffers;
    unsigned DepthStencilTexture;

    int m_width;
    int m_height;

    unsigned shadowColorTex, shadowNormalTex, shadowDepthTex;
    unsigned RSM_Color, RSM_Normal, RSM_Depth;
    unsigned RH_Red, RH_Green, RH_Blue;
    FrameBuffer* m_shadow_FBO, *m_RSM, *m_RH_FBO;

    LEAK_CHECK();
};

#endif

