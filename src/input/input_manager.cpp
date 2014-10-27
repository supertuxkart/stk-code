//  SuperTuxKart - a fun racing game with go-kart
//
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

#include "input/input_manager.hpp"

#include "config/user_config.hpp"
#include "graphics/camera.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/event_handler.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/screen.hpp"
#include "input/device_manager.hpp"
#include "input/gamepad_device.hpp"
#include "input/keyboard_device.hpp"
#include "input/input.hpp"
#include "karts/controller/controller.hpp"
#include "karts/abstract_kart.hpp"
#include "modes/demo_world.hpp"
#include "modes/profile_world.hpp"
#include "modes/world.hpp"
#include "physics/physics.hpp"
#include "race/history.hpp"
#include "replay/replay_recorder.hpp"
#include "states_screens/kart_selection.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/options_screen_input2.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"

#include <ISceneManager.h>
#include <ISceneNode.h>

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>


InputManager *input_manager;

using GUIEngine::EventPropagation;
using GUIEngine::EVENT_LET;
using GUIEngine::EVENT_BLOCK;

#define INPUT_MODE_DEBUG 0

//-----------------------------------------------------------------------------
/** Initialise input
 */
InputManager::InputManager() : m_mode(BOOTSTRAP),
                               m_mouse_val_x(0), m_mouse_val_y(0)
{
    m_device_manager = new DeviceManager();
    m_device_manager->initialize();

    m_timer_in_use = false;
    m_master_player_only = false;
    m_timer = 0;

}
// -----------------------------------------------------------------------------
void InputManager::update(float dt)
{
    if(m_timer_in_use)
    {
        m_timer -= dt;
        if(m_timer < 0) m_timer_in_use = false;
    }
}

//-----------------------------------------------------------------------------
/** Destructor. Frees all data structures.
 */
InputManager::~InputManager()
{
    delete m_device_manager;
}   // ~InputManager

//-----------------------------------------------------------------------------
void InputManager::handleStaticAction(int key, int value)
{
    static bool control_is_pressed = false;
    World *world = World::getWorld();

    // When no players... a cutscene
    if (race_manager->getNumPlayers() == 0 && world != NULL && value > 0 &&
        (key == KEY_SPACE || key == KEY_RETURN))
    {
        world->onFirePressed(NULL);
    }

    switch (key)
    {
#ifdef DEBUG
        // Special debug options for profile mode: switch the
        // camera to show a different kart.
        case KEY_KEY_1:
        case KEY_KEY_2:
        case KEY_KEY_3:
        case KEY_KEY_4:
        case KEY_KEY_5:
        case KEY_KEY_6:
        case KEY_KEY_7:
        case KEY_KEY_8:
        case KEY_KEY_9:
        {
            if(!ProfileWorld::isProfileMode() || !world) break;
            int kart_id = key - KEY_KEY_1;
            if(kart_id<0 || kart_id>=(int)world->getNumKarts()) break;
            Camera::getCamera(0)->setKart(world->getKart(kart_id));
            break;
        }
#endif
        case KEY_CONTROL:
        case KEY_RCONTROL:
        case KEY_LCONTROL:
        case KEY_RMENU:
        case KEY_LMENU:
        case KEY_LWIN:
            control_is_pressed = value!=0;
            break;

        case KEY_KEY_I:
        {
            if (!world || !UserConfigParams::m_artist_debug_mode) break;

            AbstractKart* kart = world->getLocalPlayerKart(0);
            if (kart == NULL) break;

            kart->flyUp();
            break;
        }
        case KEY_KEY_K:
        {
            if (!world || !UserConfigParams::m_artist_debug_mode) break;

            AbstractKart* kart = world->getLocalPlayerKart(0);
            if (kart == NULL) break;

            kart->flyDown();
            break;
        }
        case KEY_SNAPSHOT:
        case KEY_PRINT:
            // on windows we don't get a press event, only release.  So
            // save on release only (to avoid saving twice on other platforms)
            if (value ==0 )
                irr_driver->requestScreenshot();
            break;
            /*
        case KEY_F1:
            if (UserConfigParams::m_artist_debug_mode && world)
            {
                AbstractKart* kart = world->getLocalPlayerKart(0);
                
                if (control_is_pressed)
                    kart->setPowerup(PowerupManager::POWERUP_SWATTER, 10000);
                else
                    kart->setPowerup(PowerupManager::POWERUP_RUBBERBALL, 10000);
                    
#ifdef FORCE_RESCUE_ON_FIRST_KART
                // Can be useful for debugging places where the AI gets into
                // a rescue loop: rescue, drive, crash, rescue to same place
                world->getKart(0)->forceRescue();
#endif
            }
            break;
        case KEY_F2:
            if (UserConfigParams::m_artist_debug_mode && world)
            {
                AbstractKart* kart = world->getLocalPlayerKart(0);
                
                kart->setPowerup(PowerupManager::POWERUP_PLUNGER, 10000);
            }
            break;
        case KEY_F3:
            if (UserConfigParams::m_artist_debug_mode && world)
            {
                AbstractKart* kart = world->getLocalPlayerKart(0);
                kart->setPowerup(PowerupManager::POWERUP_CAKE, 10000);
            }
            break;
        case KEY_F4:
            if (UserConfigParams::m_artist_debug_mode && world)
            {
                AbstractKart* kart = world->getLocalPlayerKart(0);
                kart->setPowerup(PowerupManager::POWERUP_SWITCH, 10000);
            }
            break;
        case KEY_F5:
            if (UserConfigParams::m_artist_debug_mode && world)
            {
                AbstractKart* kart = world->getLocalPlayerKart(0);
                kart->setPowerup(PowerupManager::POWERUP_BOWLING, 10000);
            }
            break;
        case KEY_F6:
            if (UserConfigParams::m_artist_debug_mode && world)
            {
                AbstractKart* kart = world->getLocalPlayerKart(0);
                kart->setPowerup(PowerupManager::POWERUP_BUBBLEGUM, 10000);
            }
            break;
        case KEY_F7:
            if (UserConfigParams::m_artist_debug_mode && world)
            {
                AbstractKart* kart = world->getLocalPlayerKart(0);
                kart->setPowerup(PowerupManager::POWERUP_ZIPPER, 10000);
            }
            break;
        case KEY_F8:
            if (UserConfigParams::m_artist_debug_mode && value && world)
            {
                if (control_is_pressed)
                {
                    RaceGUIBase* gui = world->getRaceGUI();
                    if (gui != NULL) gui->m_enabled = !gui->m_enabled;

                    const int count = World::getWorld()->getNumKarts();
                    for (int n=0; n<count; n++)
                    {
                        if(World::getWorld()->getKart(n)->getController()->isPlayerController())
                            World::getWorld()->getKart(n)->getNode()
                                ->setVisible(gui->m_enabled);
                    }
                }
                else
                {
                    AbstractKart* kart = world->getLocalPlayerKart(0);
                    kart->setEnergy(100.0f);
                }
            }
            break;
        case KEY_F9:
            if (UserConfigParams::m_artist_debug_mode && world)
            {
                AbstractKart* kart = world->getLocalPlayerKart(0);
                if(control_is_pressed && race_manager->getMinorMode()!=
                                          RaceManager::MINOR_MODE_3_STRIKES)
                    kart->setPowerup(PowerupManager::POWERUP_RUBBERBALL,
                                     10000);
                else
                    kart->setPowerup(PowerupManager::POWERUP_SWATTER, 10000);
            }
            break;
            */
        case KEY_F10:
            if(world && value)
            {
                if(control_is_pressed && ReplayRecorder::get())
                    ReplayRecorder::get()->Save();
                else
                    history->Save();
            }
            break;
            /*
        case KEY_F11:
            if (UserConfigParams::m_artist_debug_mode && value &&
                control_is_pressed && world)
            {
                world->getPhysics()->nextDebugMode();
            }
            break;
            */
        case KEY_F12:
            if(value)
                UserConfigParams::m_display_fps =
                    !UserConfigParams::m_display_fps;
            break;
        default:
            break;
    } // switch

}

//-----------------------------------------------------------------------------
/**
  *  Handles input when an input sensing mode (when configuring input)
  */
void InputManager::inputSensing(Input::InputType type, int deviceID,
                                int button, Input::AxisDirection axisDirection,
                                int value)
{
#if INPUT_MODE_DEBUG
    Log::info("InputManager::inputSensing", "Start sensing input");
#endif

    // don't store if we're trying to do something like bindings keyboard
    // keys on a gamepad
    if (m_mode == INPUT_SENSE_KEYBOARD && type != Input::IT_KEYBOARD)
        return;
    if (m_mode == INPUT_SENSE_GAMEPAD  && type != Input::IT_STICKMOTION &&
        type != Input::IT_STICKBUTTON)
        return;

#if INPUT_MODE_DEBUG
    Log::info("InputManager::inputSensing", store_new ? "storing it" : "ignoring it");
#endif


    switch(type)
    {
    case Input::IT_KEYBOARD:
        if (value > Input::MAX_VALUE/2)
        {
            m_sensed_input_high_kbd.insert(button);
            break;
        }
        if (value != 0) break;   // That shouldn't happen
        // only notify on key release
        if (m_sensed_input_high_kbd.find(button)
            != m_sensed_input_high_kbd.end())
        {
            Input sensed_input;
            sensed_input.m_type           = Input::IT_KEYBOARD;
            sensed_input.m_device_id      = deviceID;
            sensed_input.m_button_id      = button;
            sensed_input.m_character      = deviceID;
            OptionsScreenInput2::getInstance()->gotSensedInput(sensed_input);
            return;
        }
        break;
    case Input::IT_STICKBUTTON:
        if (abs(value) > Input::MAX_VALUE/2.0f)
        {
            Input sensed_input;
            sensed_input.m_type           = Input::IT_STICKBUTTON;
            sensed_input.m_device_id      = deviceID;
            sensed_input.m_button_id      = button;
            sensed_input.m_character      = deviceID;
            OptionsScreenInput2::getInstance()->gotSensedInput(sensed_input);
            return;
        }
        break;
    case Input::IT_STICKMOTION:
        {
        Log::info("InputManager::inputSensing", "Storing new axis binding, value = %d; "
            "deviceID = %d; button = %d; axisDirection = %s", value, deviceID, button,
            axisDirection == Input::AD_NEGATIVE ? "-" : "+");
        // We have to save the direction in which the axis was moved.
        // This is done by storing it as a sign (and since button can
        // be zero, we add one before changing the sign).
        int input_id = value>=0 ? 1+button : -(1+button);

        bool id_was_high         = m_sensed_input_high_gamepad.find(input_id)
                                   != m_sensed_input_high_gamepad.end();
        bool inverse_id_was_high = m_sensed_input_high_gamepad.find(-input_id)
                                   != m_sensed_input_high_gamepad.end();
        bool id_was_zero         = m_sensed_input_zero_gamepad.find(button)
                                   != m_sensed_input_zero_gamepad.end();

        // A stick was pushed far enough (for the first time) to count as
        // 'triggered' - save the axis (coded with direction in the button
        // value) for later, so that it can be registered when the stick is
        // released again.
        // This is mostly legacy behaviour, it is probably good enough
        // to register this as soon as the value is high enough.
        if (!id_was_high && abs(value) > Input::MAX_VALUE*6.0f/7.0f)
        {
            if(inverse_id_was_high && !id_was_zero) {
                Input sensed_input;
                sensed_input.m_type           = type;
                sensed_input.m_device_id      = deviceID;
                sensed_input.m_button_id      = button;
                sensed_input.m_axis_direction = (value>=0) ? Input::AD_POSITIVE
                                                           : Input::AD_NEGATIVE;
                sensed_input.m_axis_range     = Input::AR_FULL;
                sensed_input.m_character      = deviceID;
                OptionsScreenInput2::getInstance()->gotSensedInput(sensed_input);

            }
            else m_sensed_input_high_gamepad.insert(input_id);
        }
        else if ( abs(value) < Input::MAX_VALUE/8.0f )
        {
            if( id_was_high )
            {
                Input sensed_input;
                sensed_input.m_type           = type;
                sensed_input.m_device_id      = deviceID;
                sensed_input.m_button_id      = button;
                sensed_input.m_axis_direction = (value>=0) == id_was_zero
                                                           ? Input::AD_POSITIVE
                                                           : Input::AD_NEGATIVE;
                sensed_input.m_axis_range     = id_was_zero ? Input::AR_HALF
                                                            : Input::AR_FULL;
                sensed_input.m_character      = deviceID;
                OptionsScreenInput2::getInstance()->gotSensedInput(sensed_input);
            }
            else if( inverse_id_was_high )
            {
                Input sensed_input;
                sensed_input.m_type           = type;
                sensed_input.m_device_id      = deviceID;
                sensed_input.m_button_id      = button;
                // Since the inverse direction was high (i.e. stick went from
                // +30000 to -100), we have to inverse the sign
                sensed_input.m_axis_direction = (value>=0) == id_was_zero
                                                           ? Input::AD_NEGATIVE
                                                           : Input::AD_POSITIVE;
                sensed_input.m_axis_range     = id_was_zero ? Input::AR_HALF
                                                            : Input::AR_FULL;
                sensed_input.m_character      = deviceID;
                OptionsScreenInput2::getInstance()->gotSensedInput(sensed_input);
            }
            else
            {
                m_sensed_input_zero_gamepad.insert(button);
            }
        }
        break;
        }

        case Input::IT_NONE:
        case Input::IT_MOUSEMOTION:
        case Input::IT_MOUSEBUTTON:
            // uninteresting (but we keep them here to explicitely state we do
            // nothing with them, and thus to fix warnings)
            break;
    }   // switch
}   // inputSensing

//-----------------------------------------------------------------------------
int InputManager::getPlayerKeyboardID() const
{
    // In no-assign mode, just return the GUI player ID (devices not
    // assigned yet)
    if (m_device_manager->getAssignMode() == NO_ASSIGN)
        return PLAYER_ID_GAME_MASTER;

    // Otherwise, after devices are assigned, we can check in more depth
    // Return the first keyboard that is actually being used
    const int amount = m_device_manager->getKeyboardAmount();
    for (int k=0; k<amount; k++)
    {
        if (m_device_manager->getKeyboard(k) != NULL &&
            m_device_manager->getKeyboard(k)->getPlayer() != NULL)
        {
            return m_device_manager->getKeyboard(k)->getPlayer()->getID();
        }
    }

    return -1;
}
//-----------------------------------------------------------------------------
/** Handles the conversion from some input to a GameAction and its distribution
 * to the currently active menu.
 * It also handles whether the game is currently sensing input. It does so by
 * suppressing the distribution of the input as a GameAction. Instead the
 * input is stored in 'm_sensed_input' and GA_SENSE_COMPLETE is distributed. If
 * however the input in question has resolved to GA_LEAVE this is treated as
 * an attempt of the user to cancel the sensing. In that case GA_SENSE_CANCEL
 * is distributed.
 *
 * Note: It is the obligation of the called menu to switch of the sense mode.
 *
 */
void InputManager::dispatchInput(Input::InputType type, int deviceID,
                                 int button,
                                 Input::AxisDirection axisDirection, int value)
{
    // Act different in input sensing mode.
    if (m_mode == INPUT_SENSE_KEYBOARD ||
        m_mode == INPUT_SENSE_GAMEPAD)
    {
        // Do not pick disabled gamepads for input sensing
         if (type == Input::IT_STICKBUTTON || type == Input::IT_STICKMOTION)
        {
             GamePadDevice *gPad = m_device_manager->getGamePadFromIrrID(deviceID);
             DeviceConfig *conf = gPad->getConfiguration();
             if (!conf->isEnabled())
                 return;
         }

        inputSensing(type, deviceID, button, axisDirection,  value);
        return;
    }

    // Abort demo mode if a key is pressed during the race in demo mode
    if(dynamic_cast<DemoWorld*>(World::getWorld()))
    {
        race_manager->exitRace();
        StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
        return;
    }

    StateManager::ActivePlayer*   player = NULL;
    PlayerAction    action;
    bool action_found = m_device_manager->translateInput(type, deviceID,
                                                         button, axisDirection,
                                                         &value, m_mode,
                                                         &player, &action);

    // in menus, some keyboard keys are standard (before each player selected
    // his device). So if a key could not be mapped to any known binding,
    // fall back to check the defaults.
    if (!action_found &&
            StateManager::get()->getGameState() != GUIEngine::GAME &&
            type == Input::IT_KEYBOARD &&
            m_mode == MENU && m_device_manager->getAssignMode() == NO_ASSIGN)
    {
        action = PA_BEFORE_FIRST;

        if      (button == KEY_UP)     action = PA_MENU_UP;
        else if (button == KEY_DOWN)   action = PA_MENU_DOWN;
        else if (button == KEY_LEFT)   action = PA_MENU_LEFT;
        else if (button == KEY_RIGHT)  action = PA_MENU_RIGHT;
        else if (button == KEY_SPACE)  action = PA_MENU_SELECT;
        else if (button == KEY_RETURN) action = PA_MENU_SELECT;
        else if (button == KEY_TAB)    action = PA_MENU_DOWN;

        if (button == KEY_RETURN && GUIEngine::ModalDialog::isADialogActive())
        {
            GUIEngine::ModalDialog::onEnterPressed();
        }

        if (action != PA_BEFORE_FIRST)
        {
            action_found = true;
            player = NULL;
        }
    }

    // do something with the key if it matches a binding
    if (action_found)
    {
        // If we're in the kart menu awaiting new players, do special things
        // when a device presses fire or rescue
        if (m_device_manager->getAssignMode() == DETECT_NEW)
        {
            // Player is unjoining
            if ((player != NULL) && (action == PA_RESCUE ||
                                     action == PA_MENU_CANCEL ) )
            {
                // returns true if the event was handled
                if (KartSelectionScreen::getRunningInstance()->playerQuit( player ))
                {
                    return; // we're done here
                }
            }

            /* The way this is currently structured, any time an event is
               received from an input device that is not associated with a
               player and the device manager is in DETECT_NEW mode, the event
               is ignored, unless it is a PA_FIRE event (a player is joining)

               perhaps it will be good to let unassigned devices back out
               of the kart selection menu?
            */

            else if (player == NULL)
            {
                // New player is joining
                if (action == PA_FIRE || action == PA_MENU_SELECT)
                {
                    InputDevice *device = NULL;
                    if (type == Input::IT_KEYBOARD)
                    {
                        //Log::info("InputManager", "New Player Joining with Key %d", button);
                        device = m_device_manager->getKeyboardFromBtnID(button);
                    }
                    else if (type == Input::IT_STICKBUTTON ||
                             type == Input::IT_STICKMOTION    )
                    {
                        device = m_device_manager->getGamePadFromIrrID(deviceID);
                    }

                    if (device != NULL)
                    {
                        KartSelectionScreen::getRunningInstance()->joinPlayer(device,
                                                                       false );
                    }
                }
                return; // we're done here, ignore devices that aren't
                        // associated with players
            }
        }

        // ... when in-game
        if (StateManager::get()->getGameState() == GUIEngine::GAME &&
             !GUIEngine::ModalDialog::isADialogActive()                  )
        {
            if (player == NULL)
            {
                // Prevent null pointer crash
                return;
            }

            // Find the corresponding PlayerKart from our ActivePlayer instance
            AbstractKart* pk = player->getKart();

            if (pk == NULL)
            {
                Log::error("InputManager::dispatchInput", "Trying to process "
                    "action for an unknown player");
                return;
            }

            Controller* controller = pk->getController();
            if (controller != NULL) controller->action(action, abs(value));
        }
        // ... when in menus
        else
        {

            // reset timer when released
            if (abs(value) == 0 &&  type == Input::IT_STICKBUTTON)
            {
                m_timer_in_use = false;
                m_timer = 0;
            }

            // When in master-only mode, we can safely assume that players
            // are set up, contrarly to early menus where we accept every
            // input because players are not set-up yet
            if (m_master_player_only && player == NULL)
            {
                if (type == Input::IT_STICKMOTION ||
                    type == Input::IT_STICKBUTTON)
                {
                    GamePadDevice* gp =
                        getDeviceManager()->getGamePadFromIrrID(deviceID);

                    if (gp != NULL &&
                        abs(value)>gp->m_deadzone)
                    {
                        //I18N: message shown when an input device is used but
                        // is not associated to any player
                        GUIEngine::showMessage(
                            _("Ignoring '%s', you needed to join earlier to play!",
                            irr::core::stringw(gp->getName().c_str()).c_str())      );
                    }
                }
                return;
            }

            // menu input
            if (!m_timer_in_use)
            {
                if (abs(value) > Input::MAX_VALUE*2/3)
                {
                    m_timer_in_use = true;
                    m_timer = 0.25;
                }

                // player may be NULL in early menus, before player setup has
                // been performed
                int playerID = (player == NULL ? 0 : player->getID());

                // If only the master player can act, and this player is not
                // the master, ignore his input
                if (m_device_manager->getAssignMode() == ASSIGN &&
                    m_master_player_only &&
                    playerID != PLAYER_ID_GAME_MASTER)
                {
                    //I18N: message shown when a player that isn't game master
                    //I18N: tries to modify options that only the game master
                    //I18N: is allowed to
                    GUIEngine::showMessage(
                        _("Only the Game Master may act at this point!"));
                    return;
                }

                // all is good, pass the translated input event on to the
                // event handler
                GUIEngine::EventHandler::get()
                    ->processGUIAction(action, deviceID, abs(value), type,
                                       playerID);
            }
        }
    }
    else if (type == Input::IT_KEYBOARD)
    {
        // keyboard press not handled by device manager / bindings.
        // Check static bindings...
        handleStaticAction( button, value );
    }
}   // input

//-----------------------------------------------------------------------------

void InputManager::setMasterPlayerOnly(bool enabled)
{
#if INPUT_MODE_DEBUG
    Log::info("InputManager::setMasterPlayerOnly", enabled ? "enabled" : "disabled");
#endif
    m_master_player_only = enabled;
}

//-----------------------------------------------------------------------------

/** Returns whether only the master player should be allowed to perform changes
 *  in menus */
bool InputManager::masterPlayerOnly() const
{
    return m_device_manager->getAssignMode() == ASSIGN && m_master_player_only;
}

//-----------------------------------------------------------------------------

/**
 * Called on keyboard events [indirectly] by irrLicht
 *
 * Analog axes can have any value from [-32768, 32767].
 *
 * There are no negative values. Instead this is reported as an axis with a
 * negative direction. This simplifies input configuration and allows greater
 * flexibility (= treat 4 directions as four buttons).
 *
 * Returns whether to halt the event's propagation here
 */
EventPropagation InputManager::input(const SEvent& event)
{
    if (event.EventType == EET_JOYSTICK_INPUT_EVENT)
    {
        // Axes - FIXME, instead of checking all of them, ask the bindings
        // which ones to poll
        for (int axis_id=0; axis_id<SEvent::SJoystickEvent::NUMBER_OF_AXES ;
              axis_id++)
        {
            int value = event.JoystickEvent.Axis[axis_id];

            if (UserConfigParams::m_gamepad_debug)
            {
                Log::info("InputManager",
                          "axis motion: gamepad_id=%d axis=%d value=%d",
                          event.JoystickEvent.Joystick, axis_id, value);
            }

            dispatchInput(Input::IT_STICKMOTION, event.JoystickEvent.Joystick,
                          axis_id, Input::AD_NEUTRAL, value);
        }

        if (event.JoystickEvent.POV == 65535)
        {
            dispatchInput(Input::IT_STICKMOTION, event.JoystickEvent.Joystick,
                          Input::HAT_H_ID, Input::AD_NEUTRAL, 0);
            dispatchInput(Input::IT_STICKMOTION, event.JoystickEvent.Joystick,
                          Input::HAT_V_ID, Input::AD_NEUTRAL, 0);
        }
        else
        {
            // *0.017453925f is to convert degrees to radians
            dispatchInput(Input::IT_STICKMOTION, event.JoystickEvent.Joystick,
                          Input::HAT_H_ID, Input::AD_NEUTRAL,
                          (int)(cos(event.JoystickEvent.POV*0.017453925f/100.0f)
                                *Input::MAX_VALUE));
            dispatchInput(Input::IT_STICKMOTION, event.JoystickEvent.Joystick,
                          Input::HAT_V_ID, Input::AD_NEUTRAL,
                          (int)(sin(event.JoystickEvent.POV*0.017453925f/100.0f)
                                *Input::MAX_VALUE));
        }

        GamePadDevice* gp =
            getDeviceManager()->getGamePadFromIrrID(event.JoystickEvent.Joystick);

        if (gp == NULL)
        {
            // Prevent null pointer crash
            return EVENT_BLOCK;
        }

        for(int i=0; i<gp->m_button_count; i++)
        {
            const bool isButtonPressed = event.JoystickEvent.IsButtonPressed(i);

            // Only report button events when the state of the button changes
            if ((!gp->isButtonPressed(i) &&  isButtonPressed) ||
                 (gp->isButtonPressed(i) && !isButtonPressed)    )
            {
                if (UserConfigParams::m_gamepad_debug)
                {
                    Log::info("InputManager", "button %i, status=%i",
                              i, isButtonPressed);
                }

                dispatchInput(Input::IT_STICKBUTTON,
                              event.JoystickEvent.Joystick, i,
                              Input::AD_POSITIVE,
                              isButtonPressed ? Input::MAX_VALUE : 0);
            }
            gp->setButtonPressed(i, isButtonPressed);
        }

    }
    else if (event.EventType == EET_KEY_INPUT_EVENT)
    {
        // On some systems (linux esp.) certain keys (e.g. [] ) have a 0
        // Key value, but do have a value defined in the Char field.
        // So to distinguish them (otherwise [] would both be mapped to
        // the same value 0, which means we can't distinguish which key
        // was actually pressed anymore), we set bit 10 which should
        // allow us to distinguish those artifical keys from the
        // 'real' keys.
        const int key = event.KeyInput.Key ? event.KeyInput.Key
                                           : event.KeyInput.Char+1024;

        if (event.KeyInput.PressedDown)
        {
            // escape is a little special
            if (key == KEY_ESCAPE)
            {
                StateManager::get()->escapePressed();
                return EVENT_BLOCK;
            }
            // 'backspace' in a text control must never be mapped, since user
            // can be in a text area trying to erase text (and if it's mapped
            // to rescue that would dismiss the dialog instead of erasing a
            // single letter). Same for spacebar. Same for letters.
            if (GUIEngine::isWithinATextBox())
            {
                if (key == KEY_BACK || key == KEY_SPACE || key == KEY_SHIFT)
                {
                    return EVENT_LET;
                }
                if (key >= KEY_KEY_0 && key <= KEY_KEY_Z)
                {
                    return EVENT_LET;
                }
            }

            const bool wasInTextBox = GUIEngine::isWithinATextBox();

            dispatchInput(Input::IT_KEYBOARD, event.KeyInput.Char, key,
                          Input::AD_POSITIVE, Input::MAX_VALUE);

            // if this action took us into a text box, don't let event continue
            // (FIXME not the cleanest solution)
            if (!wasInTextBox && GUIEngine::isWithinATextBox())
            {
                return EVENT_BLOCK;
            }

        }
        else
        {
            // 'backspace' in a text control must never be mapped, since user
            // can be in a text area trying to erase text (and if it's mapped
            // to rescue that would dismiss the dialog instead of erasing a
            // single letter). Same for spacebar. Same for letters.
            if (GUIEngine::isWithinATextBox())
            {
                if (key == KEY_BACK || key == KEY_SPACE || key == KEY_SHIFT)
                {
                    return EVENT_LET;
                }
                if (key >= KEY_KEY_0 && key <= KEY_KEY_Z)
                {
                    return EVENT_LET;
                }
            }

            dispatchInput(Input::IT_KEYBOARD, event.KeyInput.Char, key,
                          Input::AD_POSITIVE, 0);
            return EVENT_BLOCK; // Don't propagate key up events
        }
    }
#if 0 // in case we ever use mouse in-game...
    else if(event.EventType == EET_MOUSE_INPUT_EVENT)
    {
        const int type = event.MouseInput.Event;

        if(type == EMIE_MOUSE_MOVED)
        {
            // m_mouse_x = event.MouseInput.X;
            // m_mouse_y = event.MouseInput.Y;
            //const int wheel = event.MouseInput.Wheel;
        }

        /*
        EMIE_LMOUSE_PRESSED_DOWN    Left mouse button was pressed down.
        EMIE_RMOUSE_PRESSED_DOWN    Right mouse button was pressed down.
        EMIE_MMOUSE_PRESSED_DOWN    Middle mouse button was pressed down.
        EMIE_LMOUSE_LEFT_UP     Left mouse button was left up.
        EMIE_RMOUSE_LEFT_UP     Right mouse button was left up.
        EMIE_MMOUSE_LEFT_UP     Middle mouse button was left up.
        EMIE_MOUSE_MOVED    The mouse cursor changed its position.
        EMIE_MOUSE_WHEEL    The mouse wheel was moved. Use Wheel value in
                            event data to find out in what direction and
                            how fast.
         */
    }
#endif

    // block events in all modes but initial menus (except in text boxes to
    // allow typing, and except in modal dialogs in-game)
    // FIXME: 1) that's awful logic 2) that's not what the code below does,
    // events are never blocked in menus
    if (getDeviceManager()->getAssignMode() != NO_ASSIGN &&
        !GUIEngine::isWithinATextBox() &&
        (!GUIEngine::ModalDialog::isADialogActive() &&
        StateManager::get()->getGameState() == GUIEngine::GAME))
    {
        return EVENT_BLOCK;
    }
    else
    {
        return EVENT_LET;
    }
}

//-----------------------------------------------------------------------------
/** Queries the input driver whether it is in the given expected mode.
 */
bool InputManager::isInMode(InputDriverMode expMode)
{
    return m_mode == expMode;
}   // isInMode

//-----------------------------------------------------------------------------
/** Sets the mode of the input driver.
 *
 * Switching of the input driver's modes is only needed for special menus
 * (those who need typing or input sensing) and the MenuManager (switch to
 * ingame/menu mode). Therefore there is a limited amount of legal combinations
 * of current and next input driver modes: From the menu mode you can switch
 * to any other mode and from any other mode only back to the menu mode.
 *
 * In input sense mode the pointer is invisible (any movement reports are
 * suppressed). If an input happens it is stored internally and can be
 * retrieved through drv_getm_sensed_input() *after* GA_SENSE_COMPLETE has been
 * distributed to a menu (Normally the menu that received GA_SENSE_COMPLETE
 * will request the sensed input ...). If GA_SENSE_CANCEL is received instead
 * the user decided to cancel input sensing. No other game action values are
 * distributed in this mode.
 *
 * And there is the bootstrap mode. You cannot switch to it and only leave it
 * once per application instance. It is the state the input driver is first.
 *
 */
void InputManager::setMode(InputDriverMode new_mode)
{
    if (new_mode == m_mode) return; // no change

    switch (new_mode)
    {
        case MENU:
#if INPUT_MODE_DEBUG
            Log::info("InputManager::setMode", "MENU");
#endif
            switch (m_mode)
            {
                case INGAME:
                    // Leaving ingame mode.

                    //if (m_action_map)
                    //    delete m_action_map;

                    // Reset the helper values for the relative mouse movement
                    // supresses to the notification of them as an input.
                    m_mouse_val_x = m_mouse_val_y = 0;

                    //irr_driver->showPointer();
                    m_mode = MENU;
                    break;

                case BOOTSTRAP:
                    // Leaving boot strap mode.

                    // Installs the action map for the menu.
                    //  m_action_map = UserConfigParams::newMenuActionMap();

                    m_mode = MENU;
                    m_device_manager->setAssignMode(NO_ASSIGN);

                    break;
                case INPUT_SENSE_KEYBOARD:
                case INPUT_SENSE_GAMEPAD:
                    // Leaving input sense mode.

                    //irr_driver->showPointer();
                    m_sensed_input_high_gamepad.clear();
                    m_sensed_input_zero_gamepad.clear();
                    m_sensed_input_high_kbd.clear();

                    // The order is deliberate just in case someone starts
                    // to make STK multithreaded: m_sensed_input must not be
                    // 0 when mode == INPUT_SENSE_PREFER_{AXIS,BUTTON}.
                    m_mode = MENU;

                    break;

                    /*
                case LOWLEVEL:
                    // Leaving lowlevel mode.
                    //irr_driver->showPointer();

                    m_mode = MENU;

                    break;
                     */
                default:
                    ;
                    // Something is broken.
                    //assert (false);
            }

            break;
        case INGAME:
#if INPUT_MODE_DEBUG
            Log::info("InputManager::setMode", "INGAME");
#endif
            // We must be in menu mode now in order to switch.
            assert (m_mode == MENU);

            //if (m_action_map)
            //   delete m_action_map;

            // Installs the action map for the ingame mode.
            // m_action_map = UserConfigParams::newIngameActionMap();

            //irr_driver->hidePointer();

            m_mode = INGAME;

            break;
        case INPUT_SENSE_KEYBOARD:
        case INPUT_SENSE_GAMEPAD:
#if INPUT_MODE_DEBUG
            Log::info("InputManager::setMode", "INPUT_SENSE_*");
#endif
            // We must be in menu mode now in order to switch.
            assert (m_mode == MENU);

            // Reset the helper values for the relative mouse movement
            // supresses to the notification of them as an input.
            m_mouse_val_x = m_mouse_val_y = 0;
            m_mode        = new_mode;

            break;
            /*
        case LOWLEVEL:
#if INPUT_MODE_DEBUG
            Log::info("InputManager::setMode", "LOWLEVEL");
#endif
            // We must be in menu mode now in order to switch.
            assert (m_mode == MENU);

            //irr_driver->hidePointer();

            m_mode = LOWLEVEL;

            break;
             */
        default:
            // Invalid mode.
            assert(false);
    }
}

