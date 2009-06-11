//  $Id: plibdrv.cpp 757 2006-09-11 22:27:39Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
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

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "main_loop.hpp"
#include "config/player.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "gui/options_screen.hpp"
#include "gui/state_manager.hpp"
#include "gui/engine.hpp"
#include "gui/race_gui.hpp"
#include "gui/screen.hpp"
#include "input/device_manager.hpp"
#include "input/input.hpp"
#include "items/projectile_manager.hpp"
#include "karts/kart.hpp"
#include "modes/world.hpp"
#include "race/history.hpp"
#include "race/race_manager.hpp"

InputManager *input_manager;

//-----------------------------------------------------------------------------
/** Initialise input
 */
InputManager::InputManager() : m_sensed_input(0), m_mode(BOOTSTRAP),
                               m_mouse_val_x(0), m_mouse_val_y(0)
{
    m_device_manager = new DeviceManager();

    m_timer_in_use = false;
    m_timer = 0;

    bool something_new_to_write = false;

    if(!m_device_manager->deserialize())
    {
        std::cerr << "Failed to read input config file, using defaults\n";

        // could not read config file so use defaults
        KeyboardDevice* default_device = new KeyboardDevice();
        default_device->loadDefaults();
        m_device_manager->add( default_device );

        something_new_to_write = true;
    }

    if(m_device_manager->initGamePadSupport() /* returns whether a new gamepad was detected */)
    {
        something_new_to_write = true;
    }

    // write config file if necessary
    if(something_new_to_write) m_device_manager->serialize();

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


#define MAX_VALUE 32768

void InputManager::handleStaticAction(int key, int value)
{
    static int isWireframe = false;

	switch (key)
	{
#ifdef DEBUG
		case KEY_F1:
			if (race_manager->getNumPlayers() ==1 )
			{
				Kart* kart = RaceManager::getWorld()->getLocalPlayerKart(0);
                kart->setPowerup(POWERUP_BOWLING, 10000);
                projectile_manager->newExplosion(Vec3(0, 8, 0.5));
			}
			break;
		case KEY_F2:
            if (race_manager->getNumPlayers() ==1 )
			{
				Kart* kart = RaceManager::getPlayerKart(0);
				kart->setPowerup(POWERUP_PLUNGER, 10000);
			}
			break;
		case KEY_F3:
			if (race_manager->getNumPlayers() ==1 )
			{
				Kart* kart = RaceManager::getPlayerKart(0);
				kart->setPowerup(POWERUP_CAKE, 10000);
			}
			break;
#endif
		case KEY_F12:
			user_config->m_display_fps = !user_config->m_display_fps;
			if(user_config->m_display_fps)
			{
                getRaceGUI()->resetFPSCounter();
            }
			break;
		case KEY_F11:
			glPolygonMode(GL_FRONT_AND_BACK, isWireframe ? GL_FILL : GL_LINE);
			isWireframe = ! isWireframe;
			break;
#ifndef WIN32
            // For now disable F9 toggling fullscreen, since windows requires
            // to reload all textures, display lists etc. Fullscreen can
            // be toggled from the main menu (options->display).
		case KEY_F9:
            // TODO
            //irrDriver->toggleFullscreen(false);   // 0: do not reset textures
			// Fall through to put the game into pause mode.
#endif
		case KEY_ESCAPE:
            // TODO - show race menu
            // RaceManager::getWorld()->pause();
             //menu_manager->pushMenu(MENUID_RACEMENU);
            break;
		case KEY_F10:
			history->Save();
			break;
		default:
			break;
	} // switch

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
void InputManager::input(Input::InputType type, int deviceID, int btnID, int axisDirection,  int value)
{
    int player;
    PlayerAction action;

    bool action_found = m_device_manager->mapInputToPlayerAndAction( type, deviceID, btnID, axisDirection, value, &player, &action );


    // in menus, some keyboard keys are standard
    if(!StateManager::isGameState() && type == Input::IT_KEYBOARD && m_mode == MENU)
    {
        action = PA_FIRST;

        if(btnID == KEY_UP)         action = PA_ACCEL;
        else if(btnID == KEY_DOWN)  action = PA_BRAKE;
        else if(btnID == KEY_LEFT)  action = PA_LEFT;
        else if(btnID == KEY_RIGHT) action = PA_RIGHT;
        else if(btnID == KEY_SPACE) action = PA_FIRE;

        if(action != PA_FIRST)
        {
            action_found = true;
            player = 0;
        }
    }

    // Act different in input sensing mode.
    if (m_mode == INPUT_SENSE_KEYBOARD ||
        m_mode == INPUT_SENSE_GAMEPAD)
    {

        //if(type == Input::IT_STICKMOTION)
        //    std::cout << "sensing input detected an axis event\n";
        //if(type == Input::IT_STICKBUTTON)
        //    std::cout << "sensing input detected a button event\n";

        // Input sensing should be canceled. (TODO)
        //if (ga == GA_LEAVE && m_sensed_input->type==Input::IT_KEYBOARD)
        //{
        //   StateManager::gotSensedInput(NULL);
        //}
        // Stores the sensed input when the button/key/axes/<whatever> is
        // released only and is not used in a fixed mapping.
        //else
        //if (!user_config->isFixedInput(type, id0, id1, id2) ) // ignore static actions (TODO)
        {
            // See if the new input should be stored. This happens if:
            // 1) the value is larger
            // 2) nothing has been saved yet
            // 3) the new event has the preferred type : TODO - reimplement
            // The latter is necessary since some gamepads have analog
            // buttons that can return two different events when pressed
            bool store_new = abs(value) > m_max_sensed_input         ||
                             m_max_sensed_type  == Input::IT_NONE;

                        //    (m_mode  == INPUT_SENSE_GAMEPAD &&  (type == Input::IT_STICKMOTION ||
                        //                                         type == Input::IT_STICKBUTTON) &&
                        //    abs(value) > m_max_sensed_input);

            // don't store if we're trying to do something like bindings keyboard keys on a gamepad
            if(m_mode == INPUT_SENSE_KEYBOARD && type != Input::IT_KEYBOARD) store_new = false;
            if(m_mode == INPUT_SENSE_GAMEPAD && type != Input::IT_STICKMOTION && type != Input::IT_STICKBUTTON) store_new = false;

            // only store axes when they're pushed quite far
            if(m_mode == INPUT_SENSE_GAMEPAD && type == Input::IT_STICKMOTION && abs(value) < MAX_VALUE *2/3) store_new = false;

            if(store_new)
            {
                m_sensed_input->type = type;
                if(type == Input::IT_STICKMOTION)
                    std::cout << "storing new axis binding, value=" << value <<
                                 " deviceID=" << deviceID << " btnID=" << btnID << "\n";
                if(type == Input::IT_STICKBUTTON)
                    std::cout << "storing new button binding\n";

                m_sensed_input->deviceID = deviceID;
                m_sensed_input->btnID = btnID;
                m_sensed_input->axisDirection = axisDirection;

                m_max_sensed_input   = abs(value);
                m_max_sensed_type    = type;
            }

            // Notify the completion of the input sensing if the key/stick/
            // ... is released.
            if(value==0)
            {
                StateManager::gotSensedInput(m_sensed_input);
            }
        }
    }
    else if (action_found)
    {
        if(StateManager::isGameState())
        {
            RaceManager::getWorld()->getLocalPlayerKart(player)->action(action, abs(value));
        }
        else
        {
            // reset timer when released
            if( abs(value) == 0 && (/*type == Input::IT_KEYBOARD ||*/ type == Input::IT_STICKBUTTON) )
            {
                if(type == Input::IT_STICKBUTTON) std::cout << "resetting because type == Input::IT_STICKBUTTON\n";
                else std::cout << "resetting for another reason\n";

                m_timer_in_use = false;
                m_timer = 0;
            }

            // menu input
            if(!m_timer_in_use)
            {
                if(abs(value) > MAX_VALUE*2/3)
                {
                    m_timer_in_use = true;
                    m_timer = 0.25;
                }
                GUIEngine::getCurrentScreen()->processAction(action, abs(value), type);
            }
        }
    }
    else if(type == Input::IT_KEYBOARD)
    {
        // keyboard press not handled by device manager / bindings. Check static bindings...
        handleStaticAction( btnID, value );
    }
}   // input

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
 */
bool InputManager::input(const SEvent& event)
{

    if(event.EventType == EET_JOYSTICK_INPUT_EVENT)
    {
        // Axes - FIXME, instead of checking all of them, ask the bindings which ones to poll
        for(int axis_id=0; axis_id<SEvent::SJoystickEvent::NUMBER_OF_AXES ; axis_id++)
        {
            int value = event.JoystickEvent.Axis[axis_id];

#ifdef __APPLE__
            // work around irrLicht bug. FIXME - get it fixed and remove this
            if(value == -32768) continue; // ignore bogus values given by irrlicht
#endif

            if(user_config->m_gamepad_debug)
            {
                printf("axis motion: gamepad_id=%d axis=%d value=%d\n",
                       event.JoystickEvent.Joystick, axis_id, value);
            }

            // FIXME - AD_NEGATIVE/AD_POSITIVE are probably useless since value contains that info too
            if(value < 0)
                input(Input::IT_STICKMOTION, event.JoystickEvent.Joystick , axis_id, Input::AD_NEGATIVE, value);
            else
                input(Input::IT_STICKMOTION, event.JoystickEvent.Joystick, axis_id, Input::AD_POSITIVE, value);
        }

        GamePadDevice* gp = getDeviceList()->getGamePadFromIrrID(event.JoystickEvent.Joystick);

        for(int i=0; i<gp->m_button_count; i++)
        {
            const bool isButtonPressed = event.JoystickEvent.IsButtonPressed(i);

            if(gp->isButtonPressed(i) || isButtonPressed)
                input(Input::IT_STICKBUTTON, event.JoystickEvent.Joystick, i, 0, isButtonPressed ? MAX_VALUE : 0);
            gp->setButtonPressed(i, isButtonPressed);
        }

    }
    else if(event.EventType == EET_LOG_TEXT_EVENT)
    {
        // Ignore 'normal' messages
        if(event.LogEvent.Level>0)
        {
            printf("Level %d: %s\n",
                event.LogEvent.Level,event.LogEvent.Text);
        }
        return true;
    }
    else if(event.EventType == EET_KEY_INPUT_EVENT)
    {
        const int key = event.KeyInput.Key;

        if(event.KeyInput.PressedDown)
        {
            // escape is a little special
            if(key == KEY_ESCAPE)
            {
                StateManager::escapePressed();
                return true;
            }

            input(Input::IT_KEYBOARD, 0, key,
#ifdef HAVE_IRRLICHT
                  // FIXME: not sure why this happens: with plib the unicode
                  // value is 0. Since all values defined in user_config
                  // assume that the unicode value is 0, it does not work
                  // with irrlicht, which has proper unicode values defined
                  // (keydown is not recognised, but keyup is). So for now
                  // (till user_config is migrated to full irrlicht support)
                  // we pass the 0 here artifically so that keyboard handling
                  // works.
                  0,
#else
                  ev.key.keysym.unicode,
#endif
                  MAX_VALUE);

        }
        else
        {
            input(Input::IT_KEYBOARD, 0, key, 0, 0);
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
         EMIE_LMOUSE_PRESSED_DOWN 	Left mouse button was pressed down.
         EMIE_RMOUSE_PRESSED_DOWN 	Right mouse button was pressed down.
         EMIE_MMOUSE_PRESSED_DOWN 	Middle mouse button was pressed down.
         EMIE_LMOUSE_LEFT_UP 	Left mouse button was left up.
         EMIE_RMOUSE_LEFT_UP 	Right mouse button was left up.
         EMIE_MMOUSE_LEFT_UP 	Middle mouse button was left up.
         EMIE_MOUSE_MOVED 	The mouse cursor changed its position.
         EMIE_MOUSE_WHEEL 	The mouse wheel was moved. Use Wheel value in event data to find out in what direction and how fast.
         */
    }
#endif
    // FIXME: to restore the original behaviour (engine returns false in the
    //        case the input_manager::input is called)
    return false;
}

//-----------------------------------------------------------------------------
/** Retrieves the Input instance that has been prepared in the input sense
 * mode.
 *
 *
 * It is wrong to call it when not in input sensing mode anymore.
 */
Input &InputManager::getSensedInput()
{
    assert (m_mode == INPUT_SENSE_KEYBOARD ||
            m_mode == INPUT_SENSE_GAMEPAD   );

    // m_sensed_input should be available in input sense mode.
    assert (m_sensed_input);

    return *m_sensed_input;
}   // getSensedInput

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
    switch (new_mode)
    {
        case MENU:
            switch (m_mode)
        {
            case INGAME:
                // Leaving ingame mode.

                //if (m_action_map)
                //    delete m_action_map;

                // Reset the helper values for the relative mouse movement
                // supresses to the notification of them as an input.
                m_mouse_val_x = m_mouse_val_y = 0;

                irr_driver->showPointer();

                // Fall through expected.
            case BOOTSTRAP:
                // Leaving boot strap mode.

                // Installs the action map for the menu.
                //  m_action_map = user_config->newMenuActionMap();

                m_mode = MENU;

                break;
            case INPUT_SENSE_KEYBOARD:
            case INPUT_SENSE_GAMEPAD:
                // Leaving input sense mode.

                irr_driver->showPointer();

                // The order is deliberate just in case someone starts to make
                // STK multithreaded: m_sensed_input must not be 0 when
                // mode == INPUT_SENSE_PREFER_{AXIS,BUTTON}.
                m_mode = MENU;

                delete m_sensed_input;
                m_sensed_input = 0;

                break;
            case LOWLEVEL:
                // Leaving lowlevel mode.
                irr_driver->showPointer();

                m_mode = MENU;

                break;
            default:
                ;
                // Something is broken.
                //assert (false);
        }

            break;
        case INGAME:
            // We must be in menu mode now in order to switch.
            assert (m_mode == MENU);

            //if (m_action_map)
            //   delete m_action_map;

            // Installs the action map for the ingame mode.
            // m_action_map = user_config->newIngameActionMap();

            irr_driver->hidePointer();

            m_mode = INGAME;

            break;
        case INPUT_SENSE_KEYBOARD:
        case INPUT_SENSE_GAMEPAD:
            // We must be in menu mode now in order to switch.
            assert (m_mode == MENU);

            // Reset the helper values for the relative mouse movement supresses to
            // the notification of them as an input.
            m_mouse_val_x      = m_mouse_val_y = 0;
            m_max_sensed_input = 0;
            m_max_sensed_type  = Input::IT_NONE;
            m_sensed_input     = new Input();

            irr_driver->hidePointer();

            m_mode = new_mode;

            break;
        case LOWLEVEL:
            // We must be in menu mode now in order to switch.
            assert (m_mode == MENU);

            irr_driver->hidePointer();

            m_mode = LOWLEVEL;

            break;
        default:
            // Invalid mode.
            assert(false);
    }
}

