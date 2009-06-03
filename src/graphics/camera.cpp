//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 SuperTuxKart-Team, Steve Baker
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

#include "graphics/camera.hpp"

#include "user_config.hpp"
#include "audio/sound_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "karts/player_kart.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/coord.hpp"

Camera::Camera(int camera_index, const Kart* kart)
{
    m_mode     = CM_NORMAL;
    m_index    = camera_index;
    m_camera   = irr_driver->addCamera();
    m_distance = kart->getKartProperties()->getCameraDistance() * 0.5f;
    m_kart     = kart;
    m_angle_up = 0.0f;
    m_angle_around = 0.0f;

    // TODO: Put these values into a config file
    //       Global or per split screen zone?
    //       Either global or per user (for instance, some users may not like 
    //       the extra camera rotation so they could set m_rotation_range to 
    //       zero to disable it for themselves). 
    m_position_speed = 8.0f;
    m_target_speed   = 10.0f;
    m_rotation_range = 0.4f;

    // TODO: Set fog distance and clipping planes
    setScreenPosition(camera_index);
}   // Camera

// ----------------------------------------------------------------------------
Camera::~Camera()
{
    reset();
}

// ----------------------------------------------------------------------------
void Camera::setScreenPosition(int camera_index)
{
    const int num_players = race_manager->getNumLocalPlayers();
    assert(camera_index >= 0 && camera_index <= 3);

    if (num_players == 1)
    {
//        m_camera->setFOV( DEGREE_TO_RAD(75.0f) ) ;
        m_x = 0.0f; m_y = 0.0f; m_w = 1.0f; m_h = 1.0f ;
    }
    else if (num_players == 2)
    {
//        m_context -> setFOV ( 85.0f, 85.0f*3.0f/8.0f ) ;
        switch ( camera_index )
        {
        case 0 : m_x = 0.0f; m_y = 0.5f; m_w = 1.0f; m_h = 0.5f; break;
        case 1 : m_x = 0.0f; m_y = 0.0f; m_w = 1.0f; m_h = 0.5f; break;
        }
    }
    else if (num_players == 3)
    {
//        m_context -> setFOV ( 50.0f, 0.0f );
        switch ( camera_index )
        {
        case 0 : m_x = 0.0f; m_y = 0.5f; m_w = 0.5f; m_h = 0.5f; break;
        case 1 : m_x = 0.5f; m_y = 0.5f; m_w = 0.5f; m_h = 0.5f; break;
        case 2 : m_x = 0.0f; m_y = 0.0f; m_w = 1.0f; m_h = 0.5f;
//                 m_context -> setFOV ( 85.0f, 85.0f*3.0f/8.0f ); 

                                                                 break;
        }
    }
    else if (num_players == 4)
    {
//        m_context -> setFOV ( 50.0f, 0.0f );

        switch ( camera_index )
        {
        case 0 : m_x = 0.0f; m_y = 0.5f; m_w = 0.5f; m_h = 0.5f; break;
        case 1 : m_x = 0.5f; m_y = 0.5f; m_w = 0.5f; m_h = 0.5f; break;
        case 2 : m_x = 0.0f; m_y = 0.0f; m_w = 0.5f; m_h = 0.5f; break;
        case 3 : m_x = 0.5f; m_y = 0.0f; m_w = 0.5f; m_h = 0.5f; break;
        }
    }
}  // setScreenPosition

//-----------------------------------------------------------------------------
void Camera::setMode(Mode mode)
{
    m_mode = mode;
}   // setMode

// ----------------------------------------------------------------------------
Camera::Mode Camera::getMode()
{
    return m_mode;
}

//-----------------------------------------------------------------------------
/** Reset is called when a new race starts. Make sure that the camera
    is aligned neutral, and not like in the previous race
*/
void Camera::reset()
{
    setMode(CM_NORMAL);
    // m_position, m_target etc. are set when the worlds has computed the right starting
    // position of all karts and calls setInitialTransform for each camera.

}   // reset

//-----------------------------------------------------------------------------
/** Saves the current kart position as initial starting position for the
 *  camera.
 */
void Camera::setInitialTransform()
{
    m_position = Vec3(0,0,0);
    m_target   = m_kart->getXYZ();
    m_temp_position = m_position;
    m_temp_target = m_target;
}   // updateKartPosition

//-----------------------------------------------------------------------------
void Camera::update(float dt)
{
    const Track* track=RaceManager::getTrack();
    float steering;
    float dampened_steer;

    // Each case should set m_target and m_position according to what is needed for that mode.
    // Yes, there is a lot of duplicate code but it is (IMHO) much easier to follow this way.
    switch(m_mode)
    {
    case CM_NORMAL:
        // This first line moves the camera around behind the kart, pointing it 
        // towards where the kart is turning (and turning even more while skidding).
        steering = m_kart->getSteerPercent() * (1.0f + (m_kart->getSkidding() - 1.0f)/2.3f ); // dampen skidding effect
        dampened_steer =  fabsf(steering) * steering; // quadratically to dampen small variations (but keep sign)
        m_angle_around = m_kart->getHPR().getX() + m_rotation_range * dampened_steer * 0.5f;
        m_angle_up     = m_kart->getHPR().getY() - DEGREE_TO_RAD(30.0f);      

        m_target = m_kart->getXYZ();
        m_target.setZ(m_target.getZ()+0.75f);

        m_position.setX( sin(m_angle_around));
        m_position.setY(-cos(m_angle_around));
        m_position.setZ(-sin(m_angle_up));
        m_position *= m_distance;
        m_position += m_target;

        break;
    case CM_REVERSE: // Same as CM_NORMAL except it looks backwards
        m_angle_around = m_kart->getHPR().getX() - m_rotation_range * m_kart->getSteerPercent() * m_kart->getSkidding();
        m_angle_up     = m_kart->getHPR().getY() + DEGREE_TO_RAD(30.0f);      

        m_target = m_kart->getXYZ();
        m_target.setZ(m_target.getZ()+0.75f);

        m_position.setX(-sin(m_angle_around));
        m_position.setY( cos(m_angle_around));
        m_position.setZ( sin(m_angle_up));
        m_position *= m_distance * 2.0f;
        m_position += m_target;

        break;
    case CM_CLOSEUP: // Lower to the ground and closer to the kart
        m_angle_around = m_kart->getHPR().getX() + m_rotation_range * m_kart->getSteerPercent() * m_kart->getSkidding();
        m_angle_up     = m_kart->getHPR().getY() - DEGREE_TO_RAD(20.0f);

        m_target = m_kart->getXYZ();
        m_target.setZ(m_target.getZ()+0.75f);

        m_position.setX( sin(m_angle_around));
        m_position.setY(-cos(m_angle_around));
        m_position.setZ(-sin(m_angle_up));
        m_position *= m_distance * 0.5f;
        m_position += m_target;

        break;
    case CM_LEADER_MODE: // Follows the leader kart, higher off of the ground, further from the kart,
                         // and turns in the opposite direction from the kart for a nice effect. :)
        m_angle_around = RaceManager::getKart(0)->getHPR().getX();
        m_angle_up     = RaceManager::getKart(0)->getHPR().getY() + DEGREE_TO_RAD(40.0f);

        m_target = RaceManager::getKart(0)->getXYZ();

        m_position.setX(sin(m_angle_around));
        m_position.setY(cos(m_angle_around));
        m_position.setZ(sin(m_angle_up));
        m_position *= m_distance * 2.0f;
        m_position += m_target;

        break;
    case CM_FINAL:
        if(!track->hasFinalCamera())
        {
            m_mode = CM_NORMAL;
            break;
        }
        m_position = track->getCameraPosition();
        m_target.setX(-sin( track->getCameraHPR().getX() ) );
        m_target.setY( cos( track->getCameraHPR().getX() ) );
        m_target.setZ( sin( track->getCameraHPR().getY() ) );
        m_target *= 10.0f;
        m_target += m_position;
        break;
    case CM_SIMPLE_REPLAY:
        // TODO: Implement
        break;
    }

    // Smoothly interpolate towards the position and target
    m_temp_target += ((m_target - m_temp_target) * m_target_speed) * dt;
    m_temp_position += ((m_position - m_temp_position) * m_position_speed) * dt;

    m_camera->setPosition(m_temp_position.toIrrVector());
    m_camera->setTarget(m_temp_target.toIrrVector());

    if(race_manager->getNumLocalPlayers() < 2)
        sound_manager->positionListener(m_temp_position, m_temp_target - m_temp_position);

}   // update

//-----------------------------------------------------------------------------
void Camera::apply ()
{
    
}   // apply

