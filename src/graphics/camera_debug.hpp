//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2015 SuperTuxKart-Team, Steve Baker
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

#ifndef HEADER_CAMERA_DEBUG_HPP
#define HEADER_CAMERA_DEBUG_HPP

#include "graphics/camera_normal.hpp"

class AbstractKart;

/**
  * Handles the debug camera. Inherits from CameraNormal to make use
  * of the smoothing function.
  * \ingroup graphics
  */
class CameraDebug : public CameraNormal
{
public:
    enum CameraDebugType {
        CM_DEBUG_TOP_OF_KART,   //!< Camera hovering over kart
        CM_DEBUG_GROUND,        //!< Camera at ground level, wheel debugging
        CM_DEBUG_BEHIND_KART,   //!< Camera straight behind kart
        CM_DEBUG_SIDE_OF_KART,  //!< Camera to the right of the kart
        CM_DEBUG_INV_SIDE_OF_KART,  //!< Camera to the left of the kart
        CM_DEBUG_FRONT_OF_KART,  //!< Camera to the front of the kart
    };   // CameraDebugType

private:

    static CameraDebugType m_default_debug_Type;

    void getCameraSettings(float *above_kart, float *cam_angle,
                           float *side_way, float *distance    );
    void positionCamera(float dt, float above_kart, float cam_angle,
                        float side_way, float distance);

    friend class Camera;
             CameraDebug(int camera_index, AbstractKart* kart);
    virtual ~CameraDebug();
public:

    void update(float dt);
    // ------------------------------------------------------------------------
    /** Sets the debug type for all cameras. */
    static void setDebugType(CameraDebugType type)
    {
        m_default_debug_Type = type;
    }   // setDebugType

};   // CameraDebug

#endif

/* EOF */
