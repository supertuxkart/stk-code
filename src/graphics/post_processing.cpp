//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2013 the SuperTuxKart team
//  Copyright (C) 2013      Joerg Henrichs
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
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA

#include "post_processing.hpp"

#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "race/race_manager.hpp"
#include "utils/log.hpp"

#include <IGPUProgrammingServices.h>
#include <IMaterialRendererServices.h>

#define MOTION_BLUR_FACTOR (1.0f/15.0f)
#define MOTION_BLUR_OFFSET 20.0f

using namespace video;
using namespace scene;

PostProcessing::PostProcessing(video::IVideoDriver* video_driver)
{
    // Check if post-processing is supported on this hardware
    m_supported = false;
    if( video_driver->queryFeature(video::EVDF_ARB_GLSL) &&
        video_driver->queryFeature(video::EVDF_PIXEL_SHADER_2_0) &&
        video_driver->queryFeature(video::EVDF_RENDER_TO_TARGET))
    {
        m_supported = true;
    }
    
    //Check which texture dimensions are supported on this hardware
    bool nonsquare = video_driver->queryFeature(video::EVDF_TEXTURE_NSQUARE);
    bool nonpower = video_driver->queryFeature(video::EVDF_TEXTURE_NPOT);
    if (!nonpower) {
        Log::warn("PostProcessing", 
                  "Only power of two textures are supported.");
    }
    if (!nonsquare) {
        Log::warn("PostProcessing", "Only square textures are supported.");
    }
    // Initialization
    if(m_supported)
    {
        // Render target
        core::dimension2du opt = video_driver->getScreenSize()
                                .getOptimalSize(!nonpower, !nonsquare);
        m_render_target = 
            video_driver->addRenderTargetTexture(opt, "postprocess");
        if(!m_render_target)
        {
            Log::warn("PostProcessing", "Couldn't create the render target "
                      "for post-processing, disabling it.");
            UserConfigParams::m_postprocess_enabled = false;
        }
        
        // Material and shaders
        IGPUProgrammingServices* gpu = 
            video_driver->getGPUProgrammingServices();
        s32 material_type = gpu->addHighLevelShaderMaterialFromFiles(
                   (file_manager->getShaderDir() + "motion_blur.vert").c_str(),
                   "main", video::EVST_VS_2_0,
                   (file_manager->getShaderDir() + "motion_blur.frag").c_str(),
                   "main", video::EPST_PS_2_0,
                   this, video::EMT_SOLID);
        m_blur_material.MaterialType = (E_MATERIAL_TYPE)material_type;
        m_blur_material.setTexture(0, m_render_target);
        m_blur_material.Wireframe = false;
        m_blur_material.Lighting = false;
        m_blur_material.ZWriteEnable = false;

    }
}   // PostProcessing

// ----------------------------------------------------------------------------
PostProcessing::~PostProcessing()
{
    // TODO: do we have to delete/drop anything?
}   // ~PostProcessing

// ----------------------------------------------------------------------------
/** Initialises post processing at the (re-)start of a race. This sets up
 *  the vertices, normals and texture coordinates for each 
 */
void PostProcessing::reset()
{
    unsigned int n = Camera::getNumCameras();
    m_boost_time.resize(n);
    m_vertices.resize(n);
    m_center.resize(n);
    m_direction.resize(n);

    for(unsigned int i=0; i<n; i++)
    {
        m_boost_time[i] = 0.0f;
        
        const core::recti &vp = Camera::getCamera(i)->getViewport();
        // Map viewport to [-1,1] x [-1,1]. First define the coordinates
        // left, right, top, bottom:
        float right  = vp.LowerRightCorner.X < UserConfigParams::m_width 
                     ? 0.0f : 1.0f;
        float left   = vp.UpperLeftCorner.X  > 0.0f ? 0.0f : -1.0f;
        float top    = vp.UpperLeftCorner.Y  > 0.0f ? 0.0f : 1.0f;
        float bottom = vp.LowerRightCorner.Y < UserConfigParams::m_height
                     ? 0.0f : -1.0f;

        // Use left etc to define 4 vertices on which the rendered screen
        // will be displayed:
        m_vertices[i].v0.Pos = core::vector3df(left,  bottom, 0);
        m_vertices[i].v1.Pos = core::vector3df(left,  top,    0);
        m_vertices[i].v2.Pos = core::vector3df(right, top,    0);
        m_vertices[i].v3.Pos = core::vector3df(right, bottom, 0);
        // Define the texture coordinates of each vertex, which must 
        // be in [0,1]x[0,1]
        m_vertices[i].v0.TCoords  = core::vector2df(left  ==-1.0f ? 0.0f : 0.5f,
                                                    bottom==-1.0f ? 0.0f : 0.5f);
        m_vertices[i].v1.TCoords  = core::vector2df(left  ==-1.0f ? 0.0f : 0.5f,
                                                    top   == 1.0f ? 1.0f : 0.5f);
        m_vertices[i].v2.TCoords  = core::vector2df(right == 0.0f ? 0.5f : 1.0f,
                                                    top   == 1.0f ? 1.0f : 0.5f);
        m_vertices[i].v3.TCoords  = core::vector2df(right == 0.0f ? 0.5f : 1.0f,
                                                    bottom==-1.0f ? 0.0f : 0.5f);
        // Set normal and color:
        core::vector3df normal(0,0,1);
        m_vertices[i].v0.Normal = m_vertices[i].v1.Normal = 
        m_vertices[i].v2.Normal = m_vertices[i].v3.Normal = normal;
        video::SColor white(0xFF, 0xFF, 0xFF, 0xFF);
        m_vertices[i].v0.Color  = m_vertices[i].v1.Color  = 
        m_vertices[i].v2.Color  = m_vertices[i].v3.Color  = white;

        m_center[i].X=(m_vertices[i].v0.TCoords.X
                      +m_vertices[i].v2.TCoords.X) * 0.5f;

        // Center is around 20 percent from bottom of screen:
        float tex_height = m_vertices[i].v1.TCoords.Y
                         - m_vertices[i].v0.TCoords.Y;
        m_center[i].Y=m_vertices[i].v0.TCoords.Y + 0.2f*tex_height;
        m_direction[i].X = m_center[i].X;
        m_direction[i].Y = m_vertices[i].v0.TCoords.Y + 0.7f*tex_height;
    }  // for i <number of cameras
}   // reset

// ----------------------------------------------------------------------------
/** Setup the render target. First determines if there is any need for post-
 *  processing, and if so, set up render to texture.
 */
void PostProcessing::beginCapture()
{
    if(!m_supported || !UserConfigParams::m_postprocess_enabled)
        return;

    bool any_boost = false;
    for(unsigned int i=0; i<m_boost_time.size(); i++)
        any_boost |= m_boost_time[i]>0.0f;
    
    // Don't capture the input when we have no post-processing to add
    // it will be faster and this ay we won't lose anti-aliasing
    if(!any_boost) 
    {
        m_used_pp_this_frame = false;
        return;
    }
    
    m_used_pp_this_frame = true;
    irr_driver->getVideoDriver()->setRenderTarget(m_render_target, true, true);
}   // beginCapture

// ----------------------------------------------------------------------------
/** Restore the framebuffer render target.
  */
void PostProcessing::endCapture()
{
    if(!m_supported || !UserConfigParams::m_postprocess_enabled ||
        !m_used_pp_this_frame)
        return;
    
    irr_driver->getVideoDriver()->setRenderTarget(video::ERT_FRAME_BUFFER,
                                                  true, true, 0);
}   // endCapture

// ----------------------------------------------------------------------------
/** Set the boost amount according to the speed of the camera */
void PostProcessing::giveBoost(unsigned int camera_index)
{
    m_boost_time[camera_index] = 0.75f;
}   // giveBoost

// ----------------------------------------------------------------------------
/** Updates the boost times for all cameras, called once per frame.
 *  \param dt Time step size.
 */
void PostProcessing::update(float dt)
{
    for(unsigned int i=0; i<m_boost_time.size(); i++)
    {
        if (m_boost_time[i] > 0.0f)
        {
            m_boost_time[i] -= dt;
            if (m_boost_time[i] < 0.0f) m_boost_time[i] = 0.0f;
        }
    }
}   // update

// ----------------------------------------------------------------------------
/** Render the post-processed scene */
void PostProcessing::render()
{
    if(!m_supported || !UserConfigParams::m_postprocess_enabled)
        return;
    
    if (!m_used_pp_this_frame)
    {
        return;
    }

    u16 indices[6] = {0, 1, 2, 3, 0, 2};

    for(m_current_camera=0; m_current_camera<Camera::getNumCameras(); 
        m_current_camera++)
    {
        // Draw the fullscreen quad while applying the corresponding 
        // post-processing shaders
        video::IVideoDriver*    video_driver = irr_driver->getVideoDriver();
        video_driver->setMaterial(m_blur_material);
        video_driver->drawIndexedTriangleList(&(m_vertices[m_current_camera].v0),
                                              4, &indices[0], 2);
    }

}   // render

// ----------------------------------------------------------------------------
/** Implement IShaderConstantsSetCallback. Shader constants setter for 
 *  post-processing */
void PostProcessing::OnSetConstants(video::IMaterialRendererServices *services,
                                    s32 user_data)
{
    // We need the maximum texture coordinates:
    float max_tex_height = m_vertices[m_current_camera].v1.TCoords.Y;
    services->setPixelShaderConstant("max_tex_height", &max_tex_height, 1);

    // Scale the boost time to get a usable boost amount:
    float boost_amount = m_boost_time[m_current_camera] * 0.7f;
    
    // Especially for single screen the top of the screen is less blurred
    // in the fragment shader by multiplying the blurr factor by
    // (max_tex_height - texcoords.t), where max_tex_height is the maximum
    // texture coordinate (1.0 or 0.5). In split screen this factor is too
    // small (half the value compared with non-split screen), so we
    // multiply this by 2.
    if(m_boost_time.size()>1)
        boost_amount *= 2.0f;

    services->setPixelShaderConstant("boost_amount", &boost_amount, 1);
    services->setPixelShaderConstant("center",    
                                     &(m_center[m_current_camera].X), 2);
    services->setPixelShaderConstant("direction", 
                                     &(m_direction[m_current_camera].X), 2);

    // Use a radius of 0.15 when showing a single kart, otherwise (2-4 karts
    // on splitscreen) use only 0.75.
    float radius = Camera::getNumCameras()==1 ? 0.15f : 0.075f;
    services->setPixelShaderConstant("mask_radius", &radius, 1);
    const int texunit = 0;
    services->setPixelShaderConstant("color_buffer", &texunit, 1);
}   // OnSetConstants
