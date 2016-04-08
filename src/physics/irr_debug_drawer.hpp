//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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

#include "btBulletDynamicsCommon.h"

#include <SColor.h>
#include "utils/vec3.hpp"
#include <map>
#include <vector>

/**
  * \ingroup physics
  */
class IrrDebugDrawer : public btIDebugDraw
{
public:
    /** The drawing mode to use:
     *  If bit 0 is set, draw the bullet collision shape of karts
     *  If bit 1 is set, don't draw the kart graphics
     */
    enum            DebugModeType { DM_NONE              = 0x00,
                                    DM_KARTS_PHYSICS     = 0x01,
                                    DM_NO_KARTS_GRAPHICS = 0x02
                                  };
    DebugModeType   m_debug_mode;

    std::map<video::SColor, std::vector<float> > m_lines;

    Vec3 m_camera_pos;

protected:
    virtual void    setDebugMode(int debug_mode) {}
    /** Callback for bullet: if debug drawing should be done or not.
     *  Note that getDebugMode is even called when debug_drawing is disabled
     *  (i.e. not via Physics::draw()), but internally from bullet. So
     *  we have to make sure to return nodebug if debugging is disabled. */
    virtual int     getDebugMode() const
                    { return m_debug_mode==DM_NONE ? DBG_NoDebug
                                                   : DBG_DrawWireframe;}
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
    /** Returns true if debug mode is enabled. */
    bool            debugEnabled() const         {return m_debug_mode!=0;}
    void            nextDebugMode();
    void            setDebugMode(DebugModeType mode);

    void            beginNextFrame();
    const std::map<video::SColor, std::vector<float> >& getLines() const { return m_lines; }
};   // IrrDebugDrawer

#endif
/* EOF */

