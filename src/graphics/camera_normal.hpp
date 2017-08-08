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

#ifndef HEADER_CAMERA_NORMAL_HPP
#define HEADER_CAMERA_NORMAL_HPP

#include "graphics/camera.hpp"

#include "utils/cpp2011.hpp"

/**
  * \brief Handles the normal racing camera
  * \ingroup graphics
  */
class CameraNormal : public Camera
{

private:

    /** Current ambient light for this camera. */
    video::SColor   m_ambient_light;

    /** Distance between the camera and the kart. */
    float           m_distance;

    /** The speed at which the camera changes position. */
    float           m_position_speed;

    /** The speed at which the camera target changes position. */
    float           m_target_speed;

    /** Factor of the effects of steering in camera aim. */
    float           m_rotation_range;

    Vec3            m_camera_offset;

    void smoothMoveCamera(float dt);
    void handleEndCamera(float dt);
    void getCameraSettings(float *above_kart, float *cam_angle,
                           float *side_way, float *distance,
                           bool *smoothing);
    void positionCamera(float dt, float above_kart, float cam_angle,
                        float side_way, float distance, float smoothing);

    btVector3 m_kart_position;
    btQuaternion m_kart_rotation;

    // Give a few classes access to the constructor (mostly for inheritance)
    friend class Camera;
    friend class CameraDebug;
    friend class CameraEnd;
             CameraNormal(Camera::CameraType type, int camera_index,
                          AbstractKart* kart);
    virtual ~CameraNormal() {}
public:
    bool isDebug() { return false; }
    // ------------------------------------------------------------------------
    bool isFPS() { return false; }
    // ------------------------------------------------------------------------
    virtual void update(float dt) OVERRIDE;
    // ------------------------------------------------------------------------
    /** Sets the ambient light for this camera. */
    void setAmbientLight(const video::SColor &color) { m_ambient_light=color; }
    // ------------------------------------------------------------------------
    float getDistanceToKart() const { return m_distance; }
    // ------------------------------------------------------------------------
    /** Returns the current ambient light. */
    const video::SColor &getAmbientLight() const {return m_ambient_light; }

};   // class CameraNormal

#endif

/* EOF */
