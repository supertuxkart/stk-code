//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011 the SuperTuxKart team
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

#include <IGPUProgrammingServices.h>
#include <IMaterialRendererServices.h>
#include "post_processing.hpp"
#include "irr_driver.hpp"
#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "race/race_manager.hpp"

#define MOTION_BLUR_FACTOR (1.0f/15.0f)
#define MOTION_BLUR_OFFSET 20.0f

using namespace video;
using namespace scene;

PostProcessing::PostProcessing()
{
    m_boost_amount = 0.0f;
}

PostProcessing::~PostProcessing()
{
}

/** Initialization */
void PostProcessing::init(video::IVideoDriver* video_driver)
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
        fprintf(stdout, "WARNING: Only power of two textures are supported.\n");
    }
    if (!nonsquare) {
        fprintf(stdout, "WARNING: Only square textures are supported.\n");
    }
    // Initialization
    if(m_supported)
    {
        // Render target
        m_render_target = video_driver->addRenderTargetTexture(video_driver->getScreenSize().getOptimalSize(!nonpower, !nonsquare), "postprocess");
        if(!m_render_target)
        {
            fprintf(stderr, "Couldn't create the render target for post-processing, disabling it\n");
            UserConfigParams::m_postprocess_enabled = false;
        }
        
        // Material and shaders
        IGPUProgrammingServices* gpu = video_driver->getGPUProgrammingServices();
        s32 material_type = gpu->addHighLevelShaderMaterialFromFiles(
                    (file_manager->getDataDir() + "shaders/motion_blur.vert").c_str(), "main", video::EVST_VS_2_0,
                    (file_manager->getDataDir() + "shaders/motion_blur.frag").c_str(), "main", video::EPST_PS_2_0,
                    this, video::EMT_SOLID);
        m_material.MaterialType = (E_MATERIAL_TYPE)material_type;
        m_material.setTexture(0, m_render_target);
        m_material.Wireframe = false;
        m_material.Lighting = false;
        m_material.ZWriteEnable = false;
    }
}

/** Termination */
void PostProcessing::shut()
{
    if(!m_supported)
        return;
    
    // TODO: do we have to delete/drop anything?
}

/** Setup the render target */
void PostProcessing::beginCapture()
{
    if(!m_supported || !UserConfigParams::m_postprocess_enabled ||
       race_manager->getNumPlayers() > 1)
        return;
    
    // don't capture the input when we have no post-processing to add
    // it will be faster and this ay we won't lose anti-aliasing
    if (m_boost_amount <= 0.0f)
    {
        m_used_pp_this_frame = false;
        return;
    }
    
    m_used_pp_this_frame = true;
    irr_driver->getVideoDriver()->setRenderTarget(m_render_target, true, true);
}

/** Restore the framebuffer render target */
void PostProcessing::endCapture()
{
    if(!m_supported || !UserConfigParams::m_postprocess_enabled ||
       race_manager->getNumPlayers() > 1)
        return;
    
    if (m_used_pp_this_frame)
    {
        irr_driver->getVideoDriver()->setRenderTarget(video::ERT_FRAME_BUFFER, true, true, 0);
    }
}

void PostProcessing::update(float dt)
{
    if (m_boost_amount > 0.0f)
    {
        m_boost_amount -= dt*3.5f;
        if (m_boost_amount < 0.0f) m_boost_amount = 0.0f;
    }
}

/** Render the post-processed scene */
void PostProcessing::render()
{
    if(!m_supported || !UserConfigParams::m_postprocess_enabled ||
       race_manager->getNumPlayers() > 1)
        return;
    
    if (!m_used_pp_this_frame)
    {
        return;
    }
    
    // Draw the fullscreen quad while applying the corresponding post-processing shaders
    video::IVideoDriver*    video_driver = irr_driver->getVideoDriver();
    video::S3DVertex        vertices[6];
    
    video::SColor white(0xFF, 0xFF, 0xFF, 0xFF);
    vertices[0] = irr::video::S3DVertex(-1.0f,-1.0f,0.0f,0,0,1, white, 0.0f,1.0f);
    vertices[1] = irr::video::S3DVertex(-1.0f, 1.0f,0.0f,0,0,1, white, 0.0f,0.0f);
    vertices[2] = irr::video::S3DVertex( 1.0f, 1.0f,0.0f,0,0,1, white, 1.0f,0.0f);
    vertices[3] = irr::video::S3DVertex( 1.0f,-1.0f,0.0f,0,0,1, white, 1.0f,1.0f);
    vertices[4] = irr::video::S3DVertex(-1.0f,-1.0f,0.0f,0,0,1, white, 0.0f,1.0f);
    vertices[5] = irr::video::S3DVertex( 1.0f, 1.0f,0.0f,0,0,1, white, 1.0f,0.0f);
    
    u16 indices[6] = {0, 1, 2, 3, 4, 5};
    
    video_driver->setMaterial(m_material);
    video_driver->drawIndexedTriangleList(&vertices[0], 6, &indices[0], 2);
}

/** Set the boost amount according to the speed of the camera */
void PostProcessing::giveBoost()
{
    m_boost_amount = 2.5f;
    //printf("setCameraSpeed(%f)\n", cam_speed);
    //m_boost_amount = core::clamp(MOTION_BLUR_FACTOR * (cam_speed - MOTION_BLUR_OFFSET), 0.0f, 1.0f);
}

/** Implement IShaderConstantsSetCallback. Shader constants setter for post-processing */
void PostProcessing::OnSetConstants(video::IMaterialRendererServices *services, s32 user_data)
{
    services->setPixelShaderConstant("boost_amount", &m_boost_amount, 1);
    const float texunit = 0.0f;
    services->setPixelShaderConstant("color_buffer", &texunit, 1);  // Irrlicht knows this is actually a GLint and makes
                                                                   // the conversion
}
