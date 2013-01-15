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

extern const int    WIIMOTE_AXES;
extern const int    WIIMOTE_BUTTONS;

#define MAX_WIIMOTES  4

struct wiimote_t;
class GamePadDevice;
class GamepadConfig;

/** Wiimote device class */
class Wiimote
{
private:
    /** Handle to the corresponding WiiUse wiimote handle */
    wiimote_t*      m_wiimote_handle;
    
    /** Index of this element the arrays of wiimotes */
    int             m_wiimote_id;
    
    /** Corresponding gamepad managed by the DeviceManager */
    GamePadDevice*  m_gamepad_device;
    
    /** Corresponding Irrlicht gamepad event */
    irr::SEvent     m_irr_event;
    
    /** Event used for reading and writing the Irrlicht events */
    pthread_mutex_t m_event_mutex;
    
    /** Whether the wiimote received a "disconnected" event */
    bool            m_connected;
    
public:
    Wiimote();
    ~Wiimote();
    
    /** Resets internal state and creates the corresponding gamepad device */
    void        init(wiimote_t* wiimote_handle, int wiimote_id, GamepadConfig* gamepad_config);
    
    /** To be called when the wiimote becomes unused */
    void        cleanup();
    
    /** Called from the update thread: updates the Irrlicht event from the wiimote state */
    void        updateIrrEvent();
    
    /** Thread-safe reading of the last updated event */
    irr::SEvent getIrrEvent();
    
    wiimote_t*  getWiimoteHandle() const        {return m_wiimote_handle;}
    
    bool        isConnected() const             {return m_connected;}
    void        setConnected(bool connected)    {m_connected=connected;}
};

/** Wiimote manager: handles wiimote connection, disconnection,
 *  gamepad configuration and input handling
 */
class WiimoteManager
{
private:
    Wiimote         m_wiimotes[MAX_WIIMOTES];
    
    /** WiiUse wiimote handles */
    wiimote_t**     m_all_wiimote_handles;
    int             m_nb_wiimotes;
    
    /** Wiimote state update thread */
    pthread_t       m_thread;
    
    /** Shut the update thread? */
    bool            m_shut;

public:
    WiimoteManager();
    ~WiimoteManager();
    
    void launchDetection(int timeout);
    void update();
    void cleanup();
    
    int  getNbWiimotes() const   {return m_nb_wiimotes;}
    
private:
    /** Wiimotes update thread */
    void threadFunc();
    static void* threadFuncWrapper(void* data);
};

extern WiimoteManager* wiimote_manager;

#endif

#endif // WIIMOTE_MANAGER_HPP
