//  $Id: sdldrv.hpp 694 2006-08-29 07:42:36Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Joerg Henrichs
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
#ifdef HAVE_IRRLICHT

#include "graphics/irr_driver.hpp"
using namespace core;

#include "user_config.hpp"

IrrDriver *irr_driver = NULL;

IrrDriver::IrrDriver()
{
    // Try different drivers: start with opengl, then DirectX
    for(int driver_type=0; driver_type<3; driver_type++)
    {
        video::E_DRIVER_TYPE type = driver_type==0 
                                  ? video::EDT_OPENGL 
                                  : (driver_type==1 
                                     ? video::EDT_DIRECT3D9 
                                     : video::EDT_DIRECT3D8);
        // Try 32 and 16 bit per pixels
        for(int bits=32; bits>15; bits -=16) 
        {
            m_device = createDevice(type,
                                    dimension2d<irr::s32>(user_config->m_width,
                                                          user_config->m_height ),
                                    bits, //bits per pixel
                                    user_config->m_fullscreen,
                                    false,  // stencil buffers
                                    false,  // vsync
                                    this    // event receiver
                                    );
            if(m_device) break;
        }   // for bits=24, 16
        if(m_device) break;
    }   // for edt_types
    if(!m_device)
    {
        fprintf(stderr, "Couldn't initialise irrlicht device. Quitting.\n");
        exit(-1);
    }

}   // IrrDriver

// ----------------------------------------------------------------------------
IrrDriver::~IrrDriver()
{
    m_device->drop();
}   // ~IrrDriver

// ----------------------------------------------------------------------------
// Irrlicht Event handler.
bool IrrDriver::OnEvent(const irr::SEvent &event)
{
    return false;
}   // OnEvent

// ----------------------------------------------------------------------------

#endif // HAVE_IRRLICHT