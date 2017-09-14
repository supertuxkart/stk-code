//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 SuperTuxKart-Team
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
#define WCONST

#include "input/wiimote_manager.hpp"

#include "graphics/irr_driver.hpp"
#include "guiengine/modaldialog.hpp"
#include "gamepad_device.hpp"
#include "input/input_manager.hpp"
#include "input/device_manager.hpp"
#include "input/wiimote.hpp"
#include "utils/string_utils.hpp"
#include "utils/time.hpp"
#include "utils/translation.hpp"
#include "utils/vs.hpp"

#include "wiiuse.h"

WiimoteManager*  wiimote_manager;


bool WiimoteManager::m_enabled      = false;

/** Irrlicht device IDs for the wiimotes start at this value */
static const int    WIIMOTE_START_IRR_ID   = 32;

WiimoteManager::WiimoteManager()
{
    m_all_wiimote_handles = NULL;
#ifdef WIIMOTE_THREADING
    m_shut = false;
#endif
}   // WiimoteManager

// -----------------------------------------------------------------------------
WiimoteManager::~WiimoteManager()
{
    cleanup();
}   // ~WiimoteManager

// -----------------------------------------------------------------------------
/**
 * Launch wiimote detection and add the corresponding gamepad devices to the
 * device manager.
 * TODO: this should be done in a separate thread, to not block the UI...
 */
void WiimoteManager::launchDetection(int timeout)
{
  // It's only needed on systems with bluez, because wiiuse_find does not find alredy connected wiimotes
#ifdef WIIUSE_BLUEZ
    //Cleans up the config and the disconnected wiimotes
    int number_previous_wiimotes = 0;
    wiimote_t** previous_wiimotes = (wiimote_t**) malloc(sizeof(struct wiimote_t*) * MAX_WIIMOTES);
    memset(previous_wiimotes,0,sizeof(struct wiimote_t*) * MAX_WIIMOTES);
    for (unsigned int i = 0; i < m_wiimotes.size(); i++)
    {
      if (WIIMOTE_IS_CONNECTED(m_all_wiimote_handles[i]))
      {
        previous_wiimotes[i]=m_all_wiimote_handles[i];
        m_all_wiimote_handles[i] = NULL;
        number_previous_wiimotes++;
      }
    }

    //To prevent segmentation fault, have to delete NULLs
    wiimote_t** deletable_wiimotes = (wiimote_t**) malloc(sizeof(struct wiimote_t*) * (m_wiimotes.size()-number_previous_wiimotes));
    memset(deletable_wiimotes,0,sizeof(struct wiimote_t*) * (m_wiimotes.size()-number_previous_wiimotes));
    int number_deletables = 0;
    for (unsigned int i = 0; i < m_wiimotes.size(); i++)
    {
      if (m_all_wiimote_handles[i] != NULL)
      {
        deletable_wiimotes[number_deletables++] = m_all_wiimote_handles[i];
      }
    }
    m_all_wiimote_handles = wiiuse_init(MAX_WIIMOTES);
    wiiuse_cleanup(deletable_wiimotes, number_deletables);

#endif

    // Stop WiiUse, remove wiimotes, gamepads, gamepad configs.
    cleanup();

    m_all_wiimote_handles = wiiuse_init(MAX_WIIMOTES);

    // Detect wiimotes
    int nb_found_wiimotes = wiiuse_find(m_all_wiimote_handles, MAX_WIIMOTES, timeout);

#ifndef WIIUSE_BLUEZ
    // Couldn't find any wiimote?
    if(nb_found_wiimotes == 0)
        return;
#endif

#ifdef WIIUSE_BLUEZ
    // Couldn't find any wiimote?
    if(nb_found_wiimotes + number_previous_wiimotes == 0)
        return;
#endif

    // Try to connect to all found wiimotes
    int nb_wiimotes = wiiuse_connect(m_all_wiimote_handles, nb_found_wiimotes);

#ifndef WIIUSE_BLUEZ
    // Couldn't connect to any wiimote?
    if(nb_wiimotes == 0)
        return;
#endif

#ifdef WIIUSE_BLUEZ
    // Couldn't connect to any wiimote?
    if(nb_wiimotes + number_previous_wiimotes == 0)
        return;

    //Merges previous and new wiimote's list
    int number_merged_wiimotes = 0;
    for (int i = 0; i < number_previous_wiimotes && i + nb_wiimotes < MAX_WIIMOTES; i++)
    {
      m_all_wiimote_handles[i+nb_wiimotes] = previous_wiimotes[i];
      previous_wiimotes[i] = NULL;
      m_all_wiimote_handles[i]->unid = nb_wiimotes+i+1;
      number_merged_wiimotes++;
    }
    nb_wiimotes += number_merged_wiimotes;

    //To prevent segmentation fault, have to delete NULLs
    number_deletables = 0;
    deletable_wiimotes = (wiimote_t**) malloc(sizeof(struct wiimote_t*) * (number_previous_wiimotes-number_merged_wiimotes));
    memset(deletable_wiimotes,0,sizeof(struct wiimote_t*) * (number_previous_wiimotes-number_merged_wiimotes));
    for (int i = 0; i < number_previous_wiimotes; i++)
    {
      if (previous_wiimotes[i] != NULL)
      {
        deletable_wiimotes[number_deletables++] = previous_wiimotes[i];
      }
    }
    // Cleans up wiimotes which ones didn't fit in limit
    wiiuse_cleanup(deletable_wiimotes, number_deletables);

#endif

    // ---------------------------------------------------
    // Create or find a GamepadConfig for all wiimotes
    DeviceManager* device_manager = input_manager->getDeviceManager();
    GamepadConfig* gamepad_config = NULL;

    device_manager->getConfigForGamepad(WIIMOTE_START_IRR_ID, "Wiimote",
                                        &gamepad_config);
    int num_buttons = (int)( log((float)WIIMOTE_BUTTON_ALL) / log((float)2.0f))+1;
    gamepad_config->setNumberOfButtons(num_buttons);
    gamepad_config->setNumberOfAxis(1);

    setWiimoteBindings(gamepad_config);

    // Initialize all Wiimotes, which in turn create their
    // associated GamePadDevices
    for(int i=0 ; i < nb_wiimotes ; i++)
    {
        m_wiimotes.push_back(new Wiimote(m_all_wiimote_handles[i], i,
                                         gamepad_config              ));
    } // end for

    // ---------------------------------------------------
    // Set the LEDs and rumble for 0.2s
    int leds[] = {WIIMOTE_LED_1, WIIMOTE_LED_2, WIIMOTE_LED_3, WIIMOTE_LED_4};
    for(unsigned int i=0 ; i < m_wiimotes.size(); i++)
    {
        wiimote_t*  wiimote_handle = m_wiimotes[i]->getWiimoteHandle();
        wiiuse_set_leds(wiimote_handle, leds[i]);
        wiiuse_rumble(wiimote_handle, 1);
    }

    StkTime::sleep(200);

    for(unsigned int i=0 ; i < m_wiimotes.size(); i++)
    {
        wiimote_t*  wiimote_handle = m_wiimotes[i]->getWiimoteHandle();
        wiiuse_rumble(wiimote_handle, 0);
    }

    // TODO: only enable accelerometer during race
    enableAccelerometer(true);

    // ---------------------------------------------------
    // Launch the update thread
#ifdef WIIMOTE_THREADING
    m_shut = false;
    pthread_create(&m_thread, NULL, &threadFuncWrapper, this);
#endif
}   // launchDetection

// ----------------------------------------------------------------------------
/** Determines the button number based on the bitmask of a button. E.g.
 *  0x0004 (= 00100_2) is converted into 3.
 */
int getButton(int n)
{
    return (int)(log((float)n)/log((float)2.0f));
}   // getButton

// ----------------------------------------------------------------------------
/** Defines the key bindings for a wiimote for the device manager.
 *  \param gamepad_config The configuration to be defined.
 */
void WiimoteManager::setWiimoteBindings(GamepadConfig* gamepad_config)
{
    gamepad_config->setBinding(PA_STEER_LEFT,   Input::IT_STICKMOTION, 0, Input::AD_NEGATIVE);
    gamepad_config->setBinding(PA_STEER_RIGHT,  Input::IT_STICKMOTION, 0, Input::AD_POSITIVE);
    gamepad_config->setBinding(PA_ACCEL,        Input::IT_STICKBUTTON, getButton(WIIMOTE_BUTTON_TWO));
    gamepad_config->setBinding(PA_BRAKE,        Input::IT_STICKBUTTON, getButton(WIIMOTE_BUTTON_ONE));
    gamepad_config->setBinding(PA_FIRE,         Input::IT_STICKBUTTON, getButton(WIIMOTE_BUTTON_RIGHT));
    gamepad_config->setBinding(PA_NITRO,        Input::IT_STICKBUTTON, getButton(WIIMOTE_BUTTON_UP));
    gamepad_config->setBinding(PA_DRIFT,        Input::IT_STICKBUTTON, getButton(WIIMOTE_BUTTON_B));
    gamepad_config->setBinding(PA_RESCUE,       Input::IT_STICKBUTTON, getButton(WIIMOTE_BUTTON_PLUS));
    gamepad_config->setBinding(PA_LOOK_BACK,    Input::IT_STICKBUTTON, getButton(WIIMOTE_BUTTON_DOWN));
    gamepad_config->setBinding(PA_PAUSE_RACE,   Input::IT_STICKBUTTON, getButton(WIIMOTE_BUTTON_HOME));

    gamepad_config->setBinding(PA_MENU_UP,      Input::IT_STICKBUTTON, getButton(WIIMOTE_BUTTON_RIGHT));
    gamepad_config->setBinding(PA_MENU_DOWN,    Input::IT_STICKBUTTON, getButton(WIIMOTE_BUTTON_LEFT));
    gamepad_config->setBinding(PA_MENU_LEFT,    Input::IT_STICKBUTTON, getButton(WIIMOTE_BUTTON_UP));
    gamepad_config->setBinding(PA_MENU_RIGHT,   Input::IT_STICKBUTTON, getButton(WIIMOTE_BUTTON_DOWN));
    gamepad_config->setBinding(PA_MENU_SELECT,  Input::IT_STICKBUTTON, getButton(WIIMOTE_BUTTON_TWO));
    gamepad_config->setBinding(PA_MENU_CANCEL,  Input::IT_STICKBUTTON, getButton(WIIMOTE_BUTTON_ONE));
}   // setWiimoteBindings

// ----------------------------------------------------------------------------
void WiimoteManager::cleanup()
{
    if(m_wiimotes.size() > 0)
    {
        DeviceManager* device_manager = input_manager->getDeviceManager();

        GamePadDevice* first_gamepad_device =
                     device_manager->getGamePadFromIrrID(WIIMOTE_START_IRR_ID);
        assert(first_gamepad_device);

        DeviceConfig*  gamepad_config =
                                      first_gamepad_device->getConfiguration();
        assert(gamepad_config);

        // Remove the wiimote configuration -> automatically removes all
        // linked gamepad devices;
        device_manager->deleteConfig(gamepad_config);

        // Shut the update thread
#ifdef WIIMOTE_THREADING
        m_shut = true;
        pthread_join(m_thread, NULL);
#endif
        // Cleanup WiiUse
        wiiuse_cleanup(m_all_wiimote_handles, MAX_WIIMOTES);
    }

    for(unsigned int i=0; i<m_wiimotes.size(); i++)
        delete m_wiimotes[i];
    m_wiimotes.clear();

    // Reset
    m_all_wiimote_handles = NULL;
#ifdef WIIMOTE_THREADING
    m_shut                = false;
#endif
}   // cleanup

// ----------------------------------------------------------------------------
void WiimoteManager::update()
{
#ifndef WIIMOTE_THREADING
    threadFunc();
#endif
    for(unsigned int i=0 ; i < m_wiimotes.size(); i++)
    {
        irr::SEvent event = m_wiimotes[i]->getIrrEvent();
        input_manager->input(event);
    }
}   // update

// ----------------------------------------------------------------------------
/** Enables or disables the accelerometer in wiimotes (to save battery life).
 *  \param state True if the accelerometer should be enabled.
 */
void WiimoteManager::enableAccelerometer(bool state)
{
    for (unsigned int i=0; i < m_wiimotes.size(); ++i)
    {
        wiiuse_motion_sensing(m_wiimotes[i]->getWiimoteHandle(), state ? 1 : 0);
    }
}   // enableAccelerometer

// ----------------------------------------------------------------------------
/** Thread update method - wiimotes state is updated in another thread to
 *  avoid latency problems */
void WiimoteManager::threadFunc()
{
#ifdef WIIMOTE_THREADING
    while(!m_shut)
#endif
    {
        if(wiiuse_poll(m_all_wiimote_handles, MAX_WIIMOTES))
        {
            for (unsigned int i=0; i < m_wiimotes.size(); ++i)
            {
                switch (m_all_wiimote_handles[i]->event)
                {
                case WIIUSE_EVENT:
                    m_wiimotes[i]->update();
                    //printf("DEBUG: wiimote event\n");
                    break;

                case WIIUSE_STATUS:
                    //printf("DEBUG: status event\n");
                    break;

                case WIIUSE_DISCONNECT:
                case WIIUSE_UNEXPECTED_DISCONNECT:
                    //printf("DEBUG: wiimote disconnected\n");
                    m_wiimotes[i]->setConnected(false);
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

        StkTime::sleep(1);  // 'cause come on, the whole CPU is not ours :)
    } // end while
}   // threadFunc

// ----------------------------------------------------------------------------
/** This is the start function of a separate thread used to poll the wiimotes.
 *  It receives the wiimote manager as parameter when the thread is created.
 *  \param data Pointer to the wiimote manager.
 */
void* WiimoteManager::threadFuncWrapper(void *data)
{
    VS::setThreadName("WiimoteManager");
    ((WiimoteManager*)data)->threadFunc();
    return NULL;
}   // threadFuncWrapper

// ----------------------------------------------------------------------------
/** Shows a simple popup menu asking the user to connect all wiimotes.
 */
int WiimoteManager::askUserToConnectWiimotes()
{
    new MessageDialog(
#ifdef WIN32
        _("Connect your wiimote to the Bluetooth manager, then click on Ok. "
                  "Detailed instructions at supertuxkart.net/Wiimote"),
#else
        _("Press the buttons 1+2 simultaneously on your wiimote to put "
          "it in discovery mode, then click on Ok. "
                  "Detailed instructions at supertuxkart.net/Wiimote"),
#endif
        MessageDialog::MESSAGE_DIALOG_OK_CANCEL,
        new WiimoteDialogListener(), true);

    return getNumberOfWiimotes();
}   // askUserToConnectWiimotes

// ============================================================================
/** Calles when the user clicks on OK, i.e. all wiimotes are in discovery
 *  mode.
 */
void WiimoteManager::WiimoteDialogListener::onConfirm()
{
    GUIEngine::ModalDialog::dismiss();

    wiimote_manager->launchDetection(5);

    int nb_wiimotes = wiimote_manager->getNumberOfWiimotes();
    if(nb_wiimotes > 0)
    {
        new MessageDialog(_P("Found %d wiimote", "Found %d wiimotes",
                             nb_wiimotes));
    }
    else
    {
        new MessageDialog( _("Could not detect any wiimote :/") );
    }
}   // WiimoteDialogListeneronConfirm
#endif // ENABLE_WIIUSE
