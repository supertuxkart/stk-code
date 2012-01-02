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

#ifdef ENABLE_WIIUSE

#include "input/wiimote_manager.hpp"
#include "wiiuse/wiiuse.h"
#include "graphics/irr_driver.hpp"
#include "input/input_manager.hpp"
#include "input/device_manager.hpp"

WiimoteManager*  wiimote_manager;

const int    MAX_WIIMOTES = 2;
const int    WIIMOTE_AXES = 2;
const int    WIIMOTE_BUTTONS = 13;

// -----------------------------------------------------------------------------
WiimoteManager::WiimoteManager()
{
    m_wiimotes = NULL;
    m_nb_wiimotes = 0;
    m_initial_nb_gamepads = 0;
    
    resetEvent(&m_irr_events[0], -1);
    resetEvent(&m_irr_events[1], -1);
    m_shut = false;
    m_write_id = 0;
}

// -----------------------------------------------------------------------------
WiimoteManager::~WiimoteManager()
{
    cleanup();
}

// -----------------------------------------------------------------------------
/**
  * Launch wiimote detection and add the corresponding gamepad devices to the device manager
  * TODO: this should be done in a separate thread, to not block the UI...
  */
void WiimoteManager::launchDetection(int timeout)
{
    cleanup();
    
    m_wiimotes =  wiiuse_init(MAX_WIIMOTES);
    
    // Detect wiimotes
    int nb_found_wiimotes = wiiuse_find(m_wiimotes, MAX_WIIMOTES, timeout);
    
    // Try to connect to all found wiimotes
    m_nb_wiimotes = wiiuse_connect(m_wiimotes, nb_found_wiimotes);
    
    // Set the LEDs and rumble for 0.2s
    int leds[] = {WIIMOTE_LED_1, WIIMOTE_LED_2, WIIMOTE_LED_3, WIIMOTE_LED_4};
    for(int i=0 ; i < m_nb_wiimotes ; i++)
    {
        wiiuse_set_leds(m_wiimotes[i], leds[i]);
        wiiuse_rumble(m_wiimotes[i], 1);
    }
    
    irr_driver->getDevice()->sleep(200);

    for(int i=0 ; i < m_nb_wiimotes ; i++)
        wiiuse_rumble(m_wiimotes[i], 0);
    
    // ---------------------------------------------------
    
    // Create GamePadDevice for each physical gamepad and find a GamepadConfig to match
    DeviceManager* device_manager = input_manager->getDeviceList();
    GamepadConfig* gamepadConfig = NULL;
    GamePadDevice* gamepadDevice = NULL;
    
    m_initial_nb_gamepads = device_manager->getGamePadAmount();
    
    for(int i=0 ; i < m_nb_wiimotes ; i++)
    {
        int id = getGamepadId(i);
        
        core::stringc name = core::stringc("Wiimote ") + StringUtils::toString(i).c_str();
        
        // Returns true if new configuration was created
        if (device_manager->getConfigForGamepad(id, name, &gamepadConfig) == true)
        {
            if(UserConfigParams::logMisc()) 
                printf("creating new configuration.\n");
        }
        else
        {
            if(UserConfigParams::logMisc())
                printf("using existing configuration.\n");
        }

        gamepadConfig->setPlugged();
        gamepadDevice = new GamePadDevice(id, 
                                          name.c_str(),
                                          WIIMOTE_AXES,
                                          WIIMOTE_BUTTONS,
                                          gamepadConfig );
        device_manager->addGamepad(gamepadDevice);
    } // end for
    
    // ---------------------------------------------------
    // Create the update thread
    if(m_nb_wiimotes > 0)
    {
        m_write_id = 0;
        m_shut = false;
        resetEvent(&m_irr_events[0], getGamepadId(0));
        resetEvent(&m_irr_events[1], getGamepadId(0));
        
        pthread_mutex_init(&m_mutex, NULL);
        pthread_create(&m_thread, NULL, &threadFuncWrapper, this);
    }
}

// -----------------------------------------------------------------------------
void WiimoteManager::cleanup()
{
    if(m_nb_wiimotes > 0)
    {
        m_shut = true;
        pthread_join(m_thread, NULL);
        pthread_mutex_destroy(&m_mutex);
        
        wiiuse_cleanup(m_wiimotes, MAX_WIIMOTES);
        m_wiimotes = NULL;
        m_nb_wiimotes = 0;
    }
}

// -----------------------------------------------------------------------------
void WiimoteManager::update()
{
    if(m_nb_wiimotes > 0)
    {
        pthread_mutex_lock(&m_mutex);
        int read_id = !m_write_id;
        irr::SEvent event = m_irr_events[read_id];
        pthread_mutex_unlock(&m_mutex);
        
        input_manager->input(event);
    }
}

// -----------------------------------------------------------------------------
void WiimoteManager::translateEvent(wiimote_t *wm, int gamepad_id, irr::SEvent* event)
{
    // Simulate an Irrlicht joystick event;
    resetEvent(event, gamepad_id);
    
    // Send button states
    if(IS_PRESSED(wm, WIIMOTE_BUTTON_A))
    {
        printf("DEBUG: A\n");
        event->JoystickEvent.ButtonStates |= (1<<1);
    }
    if(IS_PRESSED(wm, WIIMOTE_BUTTON_B))
    {
        printf("DEBUG: B\n");
        event->JoystickEvent.ButtonStates |= (1<<2);
    }
    if(IS_PRESSED(wm, WIIMOTE_BUTTON_PLUS))
    {
        printf("DEBUG: +\n");
        event->JoystickEvent.ButtonStates |= (1<<3);
    }
    if(IS_PRESSED(wm, WIIMOTE_BUTTON_MINUS))
    {
        printf("DEBUG: -\n");
        event->JoystickEvent.ButtonStates |= (1<<4);
    }
    if(IS_PRESSED(wm, WIIMOTE_BUTTON_ONE))
    {
        printf("DEBUG: 1\n");
        event->JoystickEvent.ButtonStates |= (1<<5);
    }
    if(IS_PRESSED(wm, WIIMOTE_BUTTON_TWO))
    {
        printf("DEBUG: 2\n");
        event->JoystickEvent.ButtonStates |= (1<<6);
    }
    if(IS_PRESSED(wm, WIIMOTE_BUTTON_HOME))
    {
        printf("DEBUG: Home\n");
        event->JoystickEvent.ButtonStates |= (1<<7);
    }
}

void WiimoteManager::resetEvent(irr::SEvent* event, int gamepad_id)
{
    event->EventType = irr::EET_JOYSTICK_INPUT_EVENT;
    for(int i=0 ; i < SEvent::SJoystickEvent::NUMBER_OF_AXES ; i++)
        event->JoystickEvent.Axis[i] = 0;
    event->JoystickEvent.Joystick = (u8)(gamepad_id);
    event->JoystickEvent.POV = 65535;
    event->JoystickEvent.ButtonStates = 0;
}

void WiimoteManager::threadFunc()
{
    while(!m_shut)
    {
        if(wiiuse_poll(m_wiimotes, MAX_WIIMOTES))
        {
            for (int i=0; i < MAX_WIIMOTES; ++i)
            {
                int gamepad_id = getGamepadId(i);
                        
                switch (m_wiimotes[i]->event)
                {
                case WIIUSE_EVENT:
                    translateEvent(m_wiimotes[i], gamepad_id, &m_irr_events[m_write_id]);
                    //printf("DEBUG: wiimote event\n");
                    break;
    
                case WIIUSE_STATUS:
                    //printf("DEBUG: status event\n");
                    break;
    
                case WIIUSE_DISCONNECT:
                case WIIUSE_UNEXPECTED_DISCONNECT:
                    //printf("DEBUG: wiimote disconnected\n");
                    break;
    
                case WIIUSE_READ_DATA:
                    //printf("DEBUG: WIIUSE_READ_DATA\n");
                    break;
    
                case WIIUSE_NUNCHUK_INSERTED:
                    //printf("DEBUG: Nunchuk inserted.\n");
                    break;
    
                case WIIUSE_CLASSIC_CTRL_INSERTED:
                    //printf("DEBUG: Classic controller inserted.\n");
                    break;
    
                case WIIUSE_GUITAR_HERO_3_CTRL_INSERTED:
                    //printf("DEBUG: Guitar Hero 3 controller inserted.\n");
                    break;
    
                case WIIUSE_NUNCHUK_REMOVED:
                case WIIUSE_CLASSIC_CTRL_REMOVED:
                case WIIUSE_GUITAR_HERO_3_CTRL_REMOVED:
                    //printf("DEBUG: An expansion was removed.\n");
                    break;
                default:
                    break;
                }
            }
            
            pthread_mutex_lock(&m_mutex);
            m_write_id = !m_write_id;   // swap buffers (no need to swap them if wiiuse_poll()
                                        // did not find anything)
            pthread_mutex_unlock(&m_mutex);
        }
        
        irr_driver->getDevice()->sleep(1);  // 'cause come on, the whole CPU is not ours :)
    } // end while
}

void* WiimoteManager::threadFuncWrapper(void *data)
{
    ((WiimoteManager*)data)->threadFunc();
    return NULL;
}

#endif // ENABLE_WIIUSE
