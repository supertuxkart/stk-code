//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#include "graphics/camera_fps.hpp"
#include "karts/controller/ghost_controller.hpp"
#include "karts/controller/kart_control.hpp"
#include "modes/world.hpp"

GhostController::GhostController(AbstractKart *kart, core::stringw display_name)
                : Controller(kart)
{
    m_display_name = display_name;
}   // GhostController

//-----------------------------------------------------------------------------
void GhostController::reset()
{
    m_current_index = 0;
    m_current_time = 0.0f;
}   // reset

//-----------------------------------------------------------------------------
void GhostController::update(float dt)
{
    m_current_time = World::getWorld()->getTime();
    // Find (if necessary) the next index to use
    if (m_current_time != 0.0f)
    {
        while (m_current_index + 1 < m_all_times.size() &&
               m_current_time >= m_all_times[m_current_index + 1])
        {
            m_current_index++;
        }
    }

    // Watching replay use only
    for(unsigned int i=0; i<Camera::getNumCameras(); i++)
    {
        Camera *camera = Camera::getCamera(i);
        if(camera->getKart()!=m_kart) continue;
        if (camera->getType() != Camera::CM_TYPE_END)
        {
            if (m_controls->getLookBack())
            {
                camera->setMode(Camera::CM_REVERSE);
            }
            else
            {
                if (camera->getMode() == Camera::CM_REVERSE)
                    camera->setMode(Camera::CM_NORMAL);
            }
        }   // if camera mode != END
    }   // for i in all cameras

}   // update

//-----------------------------------------------------------------------------
void GhostController::addReplayTime(float time)
{
    // FIXME: for now avoid that transforms for the same time are set
    // twice (to avoid division by zero in update). This should be
    // done when saving in replay
    if (m_all_times.size() > 0 && m_all_times.back() == time)
        return;
    m_all_times.push_back(time);

}   // addReplayTime

//-----------------------------------------------------------------------------
void GhostController::action(PlayerAction action, int value)
{
    // Watching replay use only
    if (action == PA_LOOK_BACK)
        m_controls->setLookBack(value!=0);
}   // action
