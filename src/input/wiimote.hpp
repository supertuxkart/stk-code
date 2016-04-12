//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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

#ifndef WIIMOTE_HPP
#define WIIMOTE_HPP

#ifdef ENABLE_WIIUSE

#include "utils/synchronised.hpp"

#include "IEventReceiver.h"

struct wiimote_t;
class GamepadConfig;
class GamePadDevice;

/** Wiimote device class */
class Wiimote
{
private:
    /** Offset for wiimote irrlicht gamepads ids, to make sure any wiimote
     *  can be identified by having this or a larger id. */
    static const int    WIIMOTE_START_IRR_ID   = 32;

    /** Handle to the corresponding WiiUse wiimote handle */
    wiimote_t*      m_wiimote_handle;

    /** Index of this element the arrays of wiimotes */
    int             m_wiimote_id;

    /** Corresponding gamepad managed by the DeviceManager */
    GamePadDevice*  m_gamepad_device;

    /** Corresponding Irrlicht gamepad event */
    Synchronised<irr::SEvent> m_irr_event;

    /** Whether the wiimote received a "disconnected" event */
    bool            m_connected;

    void resetIrrEvent();
    void printDebugInfo() const;

public:
    /** Resets internal state and creates the corresponding gamepad device */
    Wiimote(wiimote_t* wiimote_handle, int wiimote_id,
                     GamepadConfig* gamepad_config);
    ~Wiimote();

    /** To be called when the wiimote becomes unused */
    void        cleanup();
    void        update();
    irr::SEvent getIrrEvent();

    // -----------------------------------------------------------------------------
    /** Returns the wiiuse handle of this wiimote. */
    wiimote_t*  getWiimoteHandle() const { return m_wiimote_handle; }
    // -----------------------------------------------------------------------------
    /** Returns true if this wiimote is connected. */
    bool isConnected() const { return m_connected; }
    // -----------------------------------------------------------------------------
    /** Sets the connection state of this wiimote. */
    void setConnected(bool connected) { m_connected=connected; }
    // -----------------------------------------------------------------------------
    /** Returns the irrlicht id for this wiimote. The wiimote ids have a higher
    *  index than any gamepad reported by irrlicht.*/
    int getIrrId() { return m_wiimote_id + WIIMOTE_START_IRR_ID; }

};   // class Wiimote

#endif

#endif // WIIMOTE_HPP
