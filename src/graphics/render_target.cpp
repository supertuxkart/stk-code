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

#ifndef SERVER_ONLY
#include "graphics/render_target.hpp"

#include "graphics/2dutils.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/rtts.hpp"
#include "graphics/shader_based_renderer.hpp"


//-----------------------------------------------------------------------------
GL1RenderTarget::GL1RenderTarget(const irr::core::dimension2du &dimension,
                                 const std::string &name)
{
    m_render_target_texture =
        irr_driver->getVideoDriver()->addRenderTargetTexture(dimension,
                                                             name.c_str(),
                                                             video::ECF_A8R8G8B8);
    if (m_render_target_texture != NULL)
    {
        irr_driver->getVideoDriver()->setRenderTarget(m_render_target_texture);
    }
    else
    {
        // m_render_target_texture will be NULL if RTT doesn't work on this 
        // computer
        Log::error("GL1RenderTarget", "Cannot render to texture.");
    }

    m_rtt_main_node = NULL;
}

GL1RenderTarget::~GL1RenderTarget()
{
        /*assert(m_old_rtt_mini_map->getReferenceCount() == 1);
        irr_driver->removeTexture(m_old_rtt_mini_map);
        m_old_rtt_mini_map = NULL;*/
}

//-----------------------------------------------------------------------------
irr::core::dimension2du GL1RenderTarget::getTextureSize() const
{
    if (m_render_target_texture == NULL) return irr::core::dimension2du(0, 0);
    return m_render_target_texture->getSize();
}

//-----------------------------------------------------------------------------
void GL1RenderTarget::renderToTexture(irr::scene::ICameraSceneNode* camera, float dt)
{
    if (m_render_target_texture == NULL)
        return;

    irr_driver->getVideoDriver()->setRenderTarget(m_render_target_texture);

    irr::video::SOverrideMaterial &overridemat = irr_driver->getVideoDriver()->getOverrideMaterial();
    overridemat.EnablePasses = scene::ESNRP_SOLID;
    overridemat.EnableFlags = video::EMF_MATERIAL_TYPE;
    overridemat.Material.MaterialType = video::EMT_SOLID;

    if (m_rtt_main_node == NULL)
    {
        irr_driver->getSceneManager()->drawAll();
    }
    else
    {
        m_rtt_main_node->setVisible(true);
        irr_driver->getSceneManager()->drawAll();
        m_rtt_main_node->setVisible(false);
    }

    overridemat.EnablePasses = 0;

    irr_driver->getVideoDriver()->setRenderTarget(0, false, false);
}

//-----------------------------------------------------------------------------
void GL1RenderTarget::draw2DImage(const irr::core::rect<s32>& dest_rect,
                                  const irr::core::rect<s32>* clip_rect,
                                  const irr::video::SColor &colors,
                                  bool use_alpha_channel_of_texture) const
{
    if (m_render_target_texture == NULL) return;
    irr::core::rect<s32> source_rect(irr::core::position2di(0, 0),
                                      m_render_target_texture->getSize());
    ::draw2DImage(m_render_target_texture, dest_rect, source_rect,
                  clip_rect, colors, use_alpha_channel_of_texture);
}

//-----------------------------------------------------------------------------
GL3RenderTarget::GL3RenderTarget(const irr::core::dimension2du &dimension,
                                 const std::string &name,
                                 ShaderBasedRenderer *renderer)
               : m_renderer(renderer), m_name(name)
{
    m_rtts = new RTT(dimension.Width, dimension.Height);
    m_frame_buffer = NULL;
}   // GL3RenderTarget

//-----------------------------------------------------------------------------

GL3RenderTarget::~GL3RenderTarget()
{
    delete m_rtts;
    m_renderer->setRTT(NULL);
}   // ~GL3RenderTarget

//-----------------------------------------------------------------------------
void GL3RenderTarget::renderToTexture(irr::scene::ICameraSceneNode* camera,
                                      float dt)
{
    m_frame_buffer = NULL;
    m_renderer->setRTT(m_rtts);
    m_renderer->renderToTexture(this, camera, dt);

}   // renderToTexture

//-----------------------------------------------------------------------------
irr::core::dimension2du GL3RenderTarget::getTextureSize() const
{
    return irr::core::dimension2du(m_frame_buffer->getWidth(),
                                   m_frame_buffer->getHeight());
}   // getTextureSize

//-----------------------------------------------------------------------------
void GL3RenderTarget::draw2DImage(const irr::core::rect<s32>& dest_rect,
                                  const irr::core::rect<s32>* clip_rect,
                                  const irr::video::SColor &colors,
                                  bool use_alpha_channel_of_texture) const
{
    assert(m_frame_buffer != NULL);
    irr::core::rect<s32> source_rect(0, 0, m_frame_buffer->getWidth(),
                                     m_frame_buffer->getHeight());
#if !defined(USE_GLES2)
    if (CVS->isARBSRGBFramebufferUsable())
        glEnable(GL_FRAMEBUFFER_SRGB);
#endif
    draw2DImageFromRTT(m_frame_buffer->getRTT()[0],
                       m_frame_buffer->getWidth(), m_frame_buffer->getHeight(),
                       dest_rect, source_rect,
                       clip_rect, colors, use_alpha_channel_of_texture);
#if !defined(USE_GLES2)
    if (CVS->isARBSRGBFramebufferUsable())
        glDisable(GL_FRAMEBUFFER_SRGB);
#endif

}   // draw2DImage

#endif   // !SERVER_ONLY
