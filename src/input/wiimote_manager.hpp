//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2013 SuperTuxKart-Team
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

#include "input/wiimote.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "utils/cpp2011.hpp"

#include "IEventReceiver.h"

#include <pthread.h>

#define MAX_WIIMOTES  4

struct wiimote_t;
class GamepadConfig;
class GamePadDevice;
class WiimoteManager;

extern WiimoteManager* wiimote_manager;


/** Wiimote manager: handles wiimote connection, disconnection,
 *  gamepad configuration and input handling
 */
class WiimoteManager
{
private:
    /** List of all connected wiimotes. */
    std::vector<Wiimote*> m_wiimotes;

    /** WiiUse wiimote handles */
    wiimote_t**     m_all_wiimote_handles;
    int             m_number_wiimotes;

    // While the wiimote code can technically work without threading,
    // its too slow (high latency), but it is useful for debugging.
#define WIIMOTE_THREADING
#ifdef WIIMOTE_THREADING
    /** Wiimote state update thread */
    pthread_t       m_thread;

    /** Shut the update thread? */
    bool            m_shut;
#endif

    /** True if wii is enabled via command line option. */
    static bool     m_enabled;

    /** Wiimotes update thread */
    void threadFunc();
    static void* threadFuncWrapper(void* data);
    void            setWiimoteBindings(GamepadConfig* gamepad_config);

public:
    WiimoteManager();
    ~WiimoteManager();

    /** Sets the wiimote to be enabled. */
    static void enable() { m_enabled = true; }

    /** Returns if the wii was enabled on the command line. */
    static bool  isEnabled() { return m_enabled; }

    void launchDetection(int timeout);
    void update();
    void cleanup();

    void enableAccelerometer(bool state);

    /** A simple listener to allow the user to connect wiimotes. It
     *  will display a feedback windows (# wiimotes connected or 'no wiimotes
     *  found').
     */
    class WiimoteDialogListener : public MessageDialog::IConfirmDialogListener
    {
    public:
        virtual void onConfirm() OVERRIDE;
    };   // class WiimoteDialoListener

    /** Shows a dialog allowing the user to connect wiimotes.
     *  \return Number of wiimotes connected.
     */
    int askUserToConnectWiimotes();
    // ------------------------------------------------------------------------
    /** Returns the number of wiimotes connected. */
    unsigned int getNumberOfWiimotes() const  {
        return (unsigned int)m_wiimotes.size();
    }   // getNumberOfWiimotes

};   // class WiimoteManager


#endif

#endif // WIIMOTE_MANAGER_HPP
