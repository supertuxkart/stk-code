//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015 SuperTuxKart-Team
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

#ifndef HEADER_SOLID_FIRST_PASS_HPP
#define HEADER_SOLID_FIRST_PASS_HPP

#include "graphics/draw_calls.hpp"
#include <irrlicht.h>

class SolidFirstPass
{
public:
    virtual ~SolidFirstPass(){}
    virtual void render(const DrawCalls& draw_calls, irr::core::vector3df wind_dir) = 0;
};

/** This class only uses OpenGL3.x functions */
/*class GL3SolidFirstPass: public SolidFirstPass
{
    void render(const DrawCalls& draw_calls, irr::core::vector3df wind_dir);
};*/

/** Require GL_ARB_base_instance and GL_ARB_draw_indirect extensions */
/*class IndirectInstancedSolidFirstPass: public SolidFirstPass
{
    void render(const DrawCalls& draw_calls, irr::core::vector3df wind_dir);
};*/

/** AZDO: Approaching Zero Driver Overhead 
 * Require GL_ARB_base_instance, GL_ARB_draw_indirect,
 * GL_ARB_bindless_texture and GL_ARB_multi_draw_indirect extensions */
/*class AZDOSolidFirstPass: public SolidFirstPass
{
    void render(const DrawCalls& draw_calls, irr::core::vector3df wind_dir);    
};*/


#endif //HEADER_SOLID_FIRST_PASS_HPP
