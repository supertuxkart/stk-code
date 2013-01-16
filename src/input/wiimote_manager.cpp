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
#include "wiiuse.h"
#include "graphics/irr_driver.hpp"
#include "input/input_manager.hpp"
#include "input/device_manager.hpp"
#include "utils/string_utils.hpp"

WiimoteManager*  wiimote_manager;

const int    WIIMOTE_AXES        = 1;  // only use one axis, for turning
const int    WIIMOTE_BUTTONS     = 12;  // A, B, left, right, top, bottom, 1, 2, (+), (-), home

/** Irrlicht device IDs for the wiimotes start at this value */
static const int    WIIMOTE_START_IRR_ID = 32;

// ============================ Helper functions ============================
// -----------------------------------------------------------------------------
static int wiimoteIdToIrrId(int wiimote_id)
{
    return wiimote_id + WIIMOTE_START_IRR_ID;
}

// -----------------------------------------------------------------------------
static void resetIrrEvent(irr::SEvent* event, int irr_id)
{
    event->EventType = irr::EET_JOYSTICK_INPUT_EVENT;
    for(int i=0 ; i < SEvent::SJoystickEvent::NUMBER_OF_AXES ; i++)
        event->JoystickEvent.Axis[i] = 0;
    event->JoystickEvent.Joystick = irr_id;
    event->JoystickEvent.POV = 65535;
    event->JoystickEvent.ButtonStates = 0;
}

// -----------------------------------------------------------------------------
struct WiimoteAction
{
    int         button_id;
    int         wiimote_action_id;
    const char* wiimote_action_name;
};

static WiimoteAction wiimote_actions[] = {
    {0,  WIIMOTE_BUTTON_LEFT,   "WIIMOTE_BUTTON_LEFT"},
    {1,  WIIMOTE_BUTTON_RIGHT,  "WIIMOTE_BUTTON_RIGHT"},
    {2,  WIIMOTE_BUTTON_UP,     "WIIMOTE_BUTTON_UP"},
    {3,  WIIMOTE_BUTTON_DOWN,   "WIIMOTE_BUTTON_DOWN"},
    {4,  WIIMOTE_BUTTON_A,      "WIIMOTE_BUTTON_A"},
    {5,  WIIMOTE_BUTTON_B,      "WIIMOTE_BUTTON_B"},
    {6,  WIIMOTE_BUTTON_PLUS,   "WIIMOTE_BUTTON_PLUS"},
    {7,  WIIMOTE_BUTTON_MINUS,  "WIIMOTE_BUTTON_MINUS"},
    {8,  WIIMOTE_BUTTON_ONE,    "WIIMOTE_BUTTON_ONE"},
    {9,  WIIMOTE_BUTTON_TWO,    "WIIMOTE_BUTTON_TWO"},
    {10, WIIMOTE_BUTTON_HOME,   "WIIMOTE_BUTTON_HOME"},
};

static int getButtonId(int wiimote_action_id)
{
    for(unsigned int i=0 ; i < sizeof(wiimote_actions)/sizeof(WiimoteAction) ; i++)
        if(wiimote_actions[i].wiimote_action_id == wiimote_action_id)
            return wiimote_actions[i].button_id;
    assert(false && "shouldn't happen");
    return -1;
}

// -----------------------------------------------------------------------------
static void setWiimoteBindings(GamepadConfig* gamepad_config)
{
    gamepad_config->setBinding(PA_STEER_LEFT,   Input::IT_STICKMOTION, 0, Input::AD_NEGATIVE);
    gamepad_config->setBinding(PA_STEER_RIGHT,  Input::IT_STICKMOTION, 0, Input::AD_POSITIVE);
    gamepad_config->setBinding(PA_ACCEL,        Input::IT_STICKBUTTON, getButtonId(WIIMOTE_BUTTON_TWO));
    gamepad_config->setBinding(PA_BRAKE,        Input::IT_STICKBUTTON, getButtonId(WIIMOTE_BUTTON_ONE));
    gamepad_config->setBinding(PA_FIRE,         Input::IT_STICKBUTTON, getButtonId(WIIMOTE_BUTTON_RIGHT));
    gamepad_config->setBinding(PA_NITRO,        Input::IT_STICKBUTTON, getButtonId(WIIMOTE_BUTTON_UP));
    gamepad_config->setBinding(PA_DRIFT,        Input::IT_STICKBUTTON, getButtonId(WIIMOTE_BUTTON_B));
    gamepad_config->setBinding(PA_RESCUE,       Input::IT_STICKBUTTON, getButtonId(WIIMOTE_BUTTON_PLUS));
    gamepad_config->setBinding(PA_LOOK_BACK,    Input::IT_STICKBUTTON, getButtonId(WIIMOTE_BUTTON_DOWN));
    gamepad_config->setBinding(PA_PAUSE_RACE,   Input::IT_STICKBUTTON, getButtonId(WIIMOTE_BUTTON_MINUS));

    gamepad_config->setBinding(PA_MENU_UP,      Input::IT_STICKBUTTON, getButtonId(WIIMOTE_BUTTON_RIGHT));
    gamepad_config->setBinding(PA_MENU_DOWN,    Input::IT_STICKBUTTON, getButtonId(WIIMOTE_BUTTON_LEFT));
    gamepad_config->setBinding(PA_MENU_LEFT,    Input::IT_STICKBUTTON, getButtonId(WIIMOTE_BUTTON_UP));
    gamepad_config->setBinding(PA_MENU_RIGHT,   Input::IT_STICKBUTTON, getButtonId(WIIMOTE_BUTTON_DOWN));
    gamepad_config->setBinding(PA_MENU_SELECT,  Input::IT_STICKBUTTON, getButtonId(WIIMOTE_BUTTON_TWO));
    gamepad_config->setBinding(PA_MENU_CANCEL,  Input::IT_STICKBUTTON, getButtonId(WIIMOTE_BUTTON_ONE));
}

// ============================ Wiimote device implementation ============================
Wiimote::Wiimote()
{
    m_wiimote_handle = NULL;
    m_wiimote_id = -1;
    m_gamepad_device = NULL;
    resetIrrEvent(&m_irr_event, 0);
    m_connected = false;
    
    pthread_mutex_init(&m_event_mutex, NULL);
}

Wiimote::~Wiimote()
{
    pthread_mutex_destroy(&m_event_mutex);
}

// -----------------------------------------------------------------------------
/** Resets internal state and creates the corresponding gamepad device */
void Wiimote::init(wiimote_t* wiimote_handle, int wiimote_id, GamepadConfig* gamepad_config)
{
    m_wiimote_handle    = wiimote_handle;
    m_wiimote_id        = wiimote_id;
    int irr_id = wiimoteIdToIrrId(wiimote_id);
    resetIrrEvent(&m_irr_event, irr_id);
    
    m_connected = true;
    
    // Create the corresponding gamepad device
    
    core::stringc gamepad_name = core::stringc("Wiimote ") + StringUtils::toString(wiimote_id).c_str();

    DeviceManager* device_manager = input_manager->getDeviceList();
    gamepad_config->setPlugged();
    m_gamepad_device = new GamePadDevice(irr_id,
                                         gamepad_name.c_str(),
                                         WIIMOTE_AXES,
                                         WIIMOTE_BUTTONS,
                                         gamepad_config );
    device_manager->addGamepad(m_gamepad_device);
}

// -----------------------------------------------------------------------------
/** Called from the update thread: updates the Irrlicht event from the wiimote state */
void Wiimote::updateIrrEvent()
{
    pthread_mutex_lock(&m_event_mutex);
    
    // Simulate an Irrlicht joystick event
    resetIrrEvent(&m_irr_event, wiimoteIdToIrrId(m_wiimote_id));
    
    // --------------------- Wiimote --------------------
    // Send button states
    for(unsigned int i=0 ; i < sizeof(wiimote_actions)/sizeof(WiimoteAction) ; i++)
    {
        if(IS_PRESSED(m_wiimote_handle, wiimote_actions[i].wiimote_action_id))
        {
            /*
            printf("wiimote %d: pressed button %s -> button id: %d\n",
                   m_wiimote_id,
                   wiimote_actions[i].wiimote_action_name,
                   wiimote_actions[i].button_id);
            */
            m_irr_event.JoystickEvent.ButtonStates |= (1<<(wiimote_actions[i].button_id));
        }
    }
    
    // ------------------ Nunchuk ----------------------
/*    if (m_wiimote_handle->exp.type == EXP_NUNCHUK)
    {
        struct nunchuk_t* nc = (nunchuk_t*)&m_wiimote_handle->exp.nunchuk;

        if (IS_PRESSED(nc, NUNCHUK_BUTTON_C))
        {
            printf("DEBUG: C\n");
            m_irr_event.JoystickEvent.ButtonStates |= (1<<12);
        }
        if (IS_PRESSED(nc, NUNCHUK_BUTTON_Z))
        {
            printf("DEBUG: Z\n");
            m_irr_event.JoystickEvent.ButtonStates |= (1<<13);
        }

        printf("nunchuk roll  = %f\n", nc->orient.roll);
        printf("nunchuk pitch = %f\n", nc->orient.pitch);
        printf("nunchuk yaw   = %f\n", nc->orient.yaw);

        printf("nunchuk joystick angle:     %f\n", nc->js.ang);
        printf("nunchuk joystick magnitude: %f\n", nc->js.mag);
    }
*/
    pthread_mutex_unlock(&m_event_mutex);
}

irr::SEvent Wiimote::getIrrEvent()
{
    irr::SEvent event;
    
    pthread_mutex_lock(&m_event_mutex);
    event = m_irr_event;
    pthread_mutex_unlock(&m_event_mutex);
    
    return event;
}

// ============================ Wiimote manager implementation ============================
WiimoteManager::WiimoteManager()
{
    m_all_wiimote_handles = NULL;
    m_nb_wiimotes = 0;
    
    m_shut = false;
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
    // Stop WiiUse, remove wiimotes, gamepads, gamepad configs.
    cleanup();
    
    m_all_wiimote_handles =  wiiuse_init(MAX_WIIMOTES);
    
    // Detect wiimotes
    int nb_found_wiimotes = wiiuse_find(m_all_wiimote_handles, MAX_WIIMOTES, timeout);
    
    // Couldn't find any wiimote?
    if(nb_found_wiimotes == 0)
        return;
    
    // Try to connect to all found wiimotes
    m_nb_wiimotes = wiiuse_connect(m_all_wiimote_handles, nb_found_wiimotes);
    
    // Couldn't connect to any wiimote?
    if(m_nb_wiimotes == 0)
        return;
    
    // ---------------------------------------------------
    // Create or find a GamepadConfig for all wiimotes
    DeviceManager* device_manager = input_manager->getDeviceList();
    GamepadConfig* gamepad_config = NULL;
    
    device_manager->getConfigForGamepad(WIIMOTE_START_IRR_ID, "Wiimote", &gamepad_config);
    setWiimoteBindings(gamepad_config);
    
    // Initialize all Wiimotes, which in turn create their associated GamePadDevices
    for(int i=0 ; i < m_nb_wiimotes ; i++)
    {
        m_wiimotes[i].init(m_all_wiimote_handles[i], i, gamepad_config);
    } // end for
    
    // ---------------------------------------------------
    // Set the LEDs and rumble for 0.2s
    int leds[] = {WIIMOTE_LED_1, WIIMOTE_LED_2, WIIMOTE_LED_3, WIIMOTE_LED_4};
    for(int i=0 ; i < m_nb_wiimotes ; i++)
    {
        wiimote_t*  wiimote_handle = m_wiimotes[i].getWiimoteHandle();
        wiiuse_set_leds(wiimote_handle, leds[i]);
        wiiuse_rumble(wiimote_handle, 1);
    }
    
    irr_driver->getDevice()->sleep(200);

    for(int i=0 ; i < m_nb_wiimotes ; i++)
    {
        wiimote_t*  wiimote_handle = m_wiimotes[i].getWiimoteHandle();
        wiiuse_rumble(wiimote_handle, 0);
    }
    
    // ---------------------------------------------------
    // Launch the update thread
    m_shut = false;
    
    pthread_create(&m_thread, NULL, &threadFuncWrapper, this);
}

// -----------------------------------------------------------------------------
void WiimoteManager::cleanup()
{
    if(m_nb_wiimotes > 0)
    {
        // Remove all configs associated to the wiimotes (linked gamepad devices are removed automatically)
        DeviceManager* device_manager = input_manager->getDeviceList();
        for(int i=0 ; i < m_nb_wiimotes ; i++)
        {
            int irr_id = wiimoteIdToIrrId(i);
            GamePadDevice*  gamepad_device = device_manager->getGamePadFromIrrID(irr_id);
            assert(gamepad_device);
            
            DeviceConfig*  gamepad_config = gamepad_device->getConfiguration();
            assert(gamepad_config);
            
            device_manager->deleteConfig(gamepad_config);
        }
        
        // Shut the update thread
        m_shut = true;
        pthread_join(m_thread, NULL);
        
        // Cleanup WiiUse
        wiiuse_cleanup(m_all_wiimote_handles, MAX_WIIMOTES);
    }
    
    // Reset
    m_all_wiimote_handles = NULL;
    m_nb_wiimotes  = 0;
    m_shut         = false;
}

// -----------------------------------------------------------------------------
void WiimoteManager::update()
{
    for(int i=0 ; i < MAX_WIIMOTES ; i++)
    {
        if(m_wiimotes[i].isConnected())
        {
            irr::SEvent event = m_wiimotes[i].getIrrEvent();
            input_manager->input(event);
        }
    }
}

// -----------------------------------------------------------------------------
/** Thread update method - wiimotes state is updated in another thread to avoid latency problems */
void WiimoteManager::threadFunc()
{
    while(!m_shut)
    {
        if(wiiuse_poll(m_all_wiimote_handles, MAX_WIIMOTES))
        {
            for (int i=0; i < MAX_WIIMOTES; ++i)
            {
                if(!m_wiimotes[i].isConnected())
                    continue;
                
                switch (m_all_wiimote_handles[i]->event)
                {
                case WIIUSE_EVENT:
                    m_wiimotes[i].updateIrrEvent();
                    //translateEvent(m_all_wiimote_handles[i], &m_irr_events[m_write_id]);  // TODO
                    //printf("DEBUG: wiimote event\n");
                    break;
    
                case WIIUSE_STATUS:
                    //printf("DEBUG: status event\n");
                    break;
    
                case WIIUSE_DISCONNECT:
                case WIIUSE_UNEXPECTED_DISCONNECT:
                    //printf("DEBUG: wiimote disconnected\n");
                    m_wiimotes[i].setConnected(false);
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
