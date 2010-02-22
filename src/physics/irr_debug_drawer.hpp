//  $Id: irr_debug_drawer.hpp 839 2006-10-24 00:01:56Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#ifndef HEADER_IRR_DEBUG_DRAWER_HPP
#define HEADER_IRR_DEBUG_DRAWER_HPP

#include "irrlicht.h"
using namespace irr;

#include "btBulletDynamicsCommon.h"

#include "graphics/irr_driver.hpp"
#include "utils/vec3.hpp"

class IrrDebugDrawer : public btIDebugDraw
{
    /** The drawing mode to use. */
    int                  m_debug_mode;

public:
                    IrrDebugDrawer();
    void            render(float dt);
    /** Draws a line. */
    virtual void    drawLine(const btVector3& from, const btVector3& to,
                             const btVector3& color);
    ///optional debug methods
    virtual void    drawContactPoint(const btVector3& Point_on_b, 
                                     const btVector3& normal_on_b,
                                     btScalar distance,int life_time,
                                     const btVector3& color)      {}
    virtual void    reportErrorWarning(const char* warningString) {}
    virtual void    draw3dText(const btVector3& location, 
                               const char* textString)            {}
    virtual void    setDebugMode(int debug_mode) { m_debug_mode = debug_mode; }
    virtual int     getDebugMode() const         { return m_debug_mode;       }
    void            activate();
    void            deactivate();

};   // IrrDebugDrawer

#endif
/* EOF */

