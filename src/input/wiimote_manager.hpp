//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012 SuperTuxKart-Team
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

#ifndef WIIMOTE_MANAGER_HPP
#define WIIMOTE_MANAGER_HPP

#ifdef ENABLE_WIIUSE

#include <pthread.h>
#include "IEventReceiver.h"

extern const int    MAX_WIIMOTES;
extern const int    WIIMOTE_AXES;
extern const int    WIIMOTE_BUTTONS;

struct wiimote_t;

class WiimoteManager
{
private:
    wiimote_t**     m_wiimotes;
    int             m_nb_wiimotes;
    
    int             m_initial_nb_gamepads;  // Wiimotes are attributed the IDs following
                                            // the "normal" gamepads - that's a bit of a hack...
    
    pthread_t       m_thread;
    pthread_mutex_t m_mutex;
    int             m_write_id;
    irr::SEvent     m_irr_events[2];
    bool            m_shut;

public:
    WiimoteManager();
    ~WiimoteManager();
    
    void launchDetection(int timeout);
    void update();
    void cleanup();
    
    int  getNbWiimotes() const   {return m_nb_wiimotes;}
    
private:
    void translateEvent(wiimote_t* wm, int gamepad_id, irr::SEvent* event);
    int  getGamepadId(int wiimote_id) const {return wiimote_id + m_initial_nb_gamepads;}
    void resetEvent(irr::SEvent* event, int gamepad_id);
    
    void threadFunc();
    static void* threadFuncWrapper(void* data);
};

extern WiimoteManager* wiimote_manager;

#endif

#endif // WIIMOTE_MANAGER_HPP
