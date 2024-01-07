//  SuperTuxKart - a fun racing game with go-kart
//
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

#include "input/input_manager.hpp"

#include "config/user_config.hpp"
#include "graphics/camera_fps.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/shader_based_renderer.hpp"
#include "graphics/sp/sp_base.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/event_handler.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/screen_keyboard.hpp"
#include "input/device_manager.hpp"
#include "input/gamepad_device.hpp"
#include "input/input.hpp"
#include "input/keyboard_device.hpp"
#include "input/multitouch_device.hpp"
#include "input/sdl_controller.hpp"
#include "input/wiimote_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "modes/demo_world.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/rewind_manager.hpp"
#include "physics/physics.hpp"
#include "race/history.hpp"
#include "replay/replay_recorder.hpp"
#include "states_screens/kart_selection.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "states_screens/options/options_screen_device.hpp"
#ifdef MOBILE_STK
#include "states_screens/race_gui_multitouch.hpp"
#endif
#include "states_screens/state_manager.hpp"
#include "utils/debug.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <IrrlichtDevice.h>
#include <ISceneManager.h>
#include <ICameraSceneNode.h>
#include <ISceneNode.h>
#include <ITexture.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#ifndef SERVER_ONLY
#include <SDL.h>
#endif

#ifdef __SWITCH__
extern "C" {
#define Event libnx_Event
#define u64 libnx_u64
#define u32 libnx_u32
#define s64 libnx_s64
#define s32 libnx_s32
#include <switch/runtime/pad.h>
#undef u64
#undef u32
#undef s64
#undef s32
#undef Event
}
#endif

InputManager *input_manager;

using GUIEngine::EventPropagation;
using GUIEngine::EVENT_LET;
using GUIEngine::EVENT_BLOCK;

#define INPUT_MODE_DEBUG 0

//-----------------------------------------------------------------------------
/** Initialise input
 */
InputManager::InputManager() : m_mode(BOOTSTRAP),
                               m_mouse_val_x(-1), m_mouse_val_y(-1)
{
    Log::debug("InputManager", "Initialising InputManager!");
    m_device_manager = new DeviceManager();
    m_device_manager->initialize();

    m_master_player_only = false;
#ifndef SERVER_ONLY
#ifdef __SWITCH__
    padConfigureInput(8, HidNpadStyleSet_NpadStandard);
    // Otherwise we report 'B' as 'A' (like Xbox controller)
    SDL_SetHint(
        SDL_HINT_GAMECONTROLLERCONFIG,
        "53776974636820436F6E74726F6C6C65,Switch Controller,a:b0,b:b1,back:b11,dpdown:b15,dpleft:b12,dpright:b14,dpup:b13,leftshoulder:b6,leftstick:b4,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b5,righttrigger:b9,rightx:a2,righty:a3,start:b10,x:b2,y:b3,\n"
    );
#endif // __SWITCH__
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0)
    {
        Log::error("InputManager", "Failed to init SDL game controller: %s",
            SDL_GetError());
    }

    if (SDL_InitSubSystem(SDL_INIT_HAPTIC) != 0)
    {
        Log::error("InputManager", "Failed to init SDL haptics: %s",
            SDL_GetError());
    }
#endif // SERVER_ONLY
}

// -----------------------------------------------------------------------------
void InputManager::addJoystick()
{
#ifndef SERVER_ONLY
    // When irrlicht device is reinitialized the joystick added event may be
    // lost, we look for them and add it back
    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        try
        {
            SDL_Joystick* joystick = SDL_JoystickOpen(i);
            if (!joystick)
                continue;
            SDL_JoystickID id = SDL_JoystickInstanceID(joystick);
            if (m_sdl_controller.find(id) != m_sdl_controller.end())
                continue;
            std::unique_ptr<SDLController> c(
                new SDLController(i));
            id = c->getInstanceID();
            m_sdl_controller[id] = std::move(c);
        }
        catch (std::exception& e)
        {
            Log::error("SDLController", "Error in addJoystick %s", e.what());
        }
    }
#endif
}

// -----------------------------------------------------------------------------
#ifndef SERVER_ONLY
// For CIrrDeviceSDL
extern "C" void handle_joystick(SDL_Event& event)
{
    if (input_manager)
        input_manager->handleJoystick(event);
}   // handle_joystick

// -----------------------------------------------------------------------------
void InputManager::handleJoystick(SDL_Event& event)
{
    try
    {
        switch (event.type)
        {
        case SDL_JOYDEVICEADDED:
        {
            std::unique_ptr<SDLController> c(
                new SDLController(event.jdevice.which));
            SDL_JoystickID id = c->getInstanceID();
            m_sdl_controller[id] = std::move(c);
            break;
        }
        case SDL_JOYDEVICEREMOVED:
        {
            m_gamepads_timer.erase(
                m_sdl_controller.at(event.jdevice.which)->getInstanceID());
            m_sdl_controller.erase(event.jdevice.which);
            break;
        }
        case SDL_JOYAXISMOTION:
        {
            auto& controller = m_sdl_controller.at(event.jaxis.which);
            if (m_mode == INPUT_SENSE_GAMEPAD)
                controller->handleAxisInputSense(event);
            if (controller->handleAxis(event) &&
                !UserConfigParams::m_gamepad_visualisation)
                input(controller->getEvent());
            break;
        }
        case SDL_JOYHATMOTION:
        {
            auto& controller = m_sdl_controller.at(event.jhat.which);
            if (controller->handleHat(event) &&
                !UserConfigParams::m_gamepad_visualisation)
                input(controller->getEvent());
            break;
        }
        case SDL_JOYBUTTONUP:
        case SDL_JOYBUTTONDOWN:
        {
            auto& controller = m_sdl_controller.at(event.jbutton.which);
            if (controller->handleButton(event) &&
                !UserConfigParams::m_gamepad_visualisation)
                input(controller->getEvent());
            break;
        }
        default:
            break;
        }
    }
    catch (std::exception& e)
    {
        Log::error("SDLController", "Error in handleJoystick() %s", e.what());
    }
}   // handleJoystick

SDLController* InputManager::getSDLController(unsigned i) {
    assert(i < m_sdl_controller.size());
    auto it = m_sdl_controller.begin();
    for(unsigned j = 0; j < i; ++j)
            it++;
    return it->second.get();
}
#endif

// -----------------------------------------------------------------------------
void InputManager::update(float dt)
{
#ifdef ENABLE_WIIUSE
    if (wiimote_manager)
        wiimote_manager->update();
#endif
#ifdef __SWITCH__
    hidSetNpadJoyHoldType(HidNpadJoyHoldType_Horizontal);
#endif

    for (auto it = m_gamepads_timer.begin(); it != m_gamepads_timer.end();)
    {
        it->second -= dt;
        if (it->second < 0)
            it = m_gamepads_timer.erase(it);
        else
            it++;
    }

#ifndef SERVER_ONLY
    for (auto& controller : m_sdl_controller)
        controller.second->checkPowerLevel();
#endif
}

//-----------------------------------------------------------------------------
#ifndef SERVER_ONLY
const irr::SEvent& InputManager::getEventForGamePad(unsigned i) const
{
    auto it = m_sdl_controller.begin();
    return std::next(it, i)->second->getEvent();
}
#endif

//-----------------------------------------------------------------------------
/** Destructor. Frees all data structures.
 */
InputManager::~InputManager()
{
#ifndef SERVER_ONLY
    m_sdl_controller.clear();
#endif
    delete m_device_manager;
}   // ~InputManager

//-----------------------------------------------------------------------------
void InputManager::handleStaticAction(int key, int value)
{
    static bool control_is_pressed = false;
    static bool shift_is_pressed   = false;

    World *world = World::getWorld();

    // When no players... a cutscene
    if (RaceManager::get() &&
        RaceManager::get()->getNumPlayers() == 0 && world != NULL && value > 0 &&
        (key == IRR_KEY_SPACE || key == IRR_KEY_RETURN ||
        key == IRR_KEY_BUTTON_A))
    {
        world->onFirePressed(NULL);
    }

    if (world != NULL && UserConfigParams::m_artist_debug_mode)
    {
        Debug::handleStaticAction(key, value, control_is_pressed, shift_is_pressed);
    }

    switch (key)
    {
#ifdef DEBUG
        // Special debug options for profile mode: switch the
        // camera to show a different kart.
        case IRR_KEY_1:
        case IRR_KEY_2:
        case IRR_KEY_3:
        case IRR_KEY_4:
        case IRR_KEY_5:
        case IRR_KEY_6:
        case IRR_KEY_7:
        case IRR_KEY_8:
        case IRR_KEY_9:
        {
            if(!ProfileWorld::isProfileMode() || !world) break;
            int kart_id = key - IRR_KEY_1;
            if(kart_id < 0 || kart_id >= (int)world->getNumKarts()) break;
            Camera::getCamera(0)->setKart(world->getKart(kart_id));
            break;
        }
#endif
        case IRR_KEY_CONTROL:
        case IRR_KEY_RCONTROL:
        case IRR_KEY_LCONTROL:
        case IRR_KEY_RMENU:
        case IRR_KEY_LMENU:
        case IRR_KEY_LWIN:
        {
            control_is_pressed = value != 0;
            break;
        }
        case IRR_KEY_LSHIFT:
        case IRR_KEY_RSHIFT:
        case IRR_KEY_SHIFT:
        {
            shift_is_pressed = value != 0;
            break;
        }
        case IRR_KEY_SNAPSHOT:
        case IRR_KEY_PRINT:
        {
            // on windows we don't get a press event, only release.  So
            // save on release only (to avoid saving twice on other platforms)
            if (value == 0)
            {
                if (control_is_pressed)
                {
                    const bool is_recording = irr_driver->isRecording();
                    irr_driver->setRecording(!is_recording);
                }
                else
                {
                    irr_driver->requestScreenshot();
                }
            }
            break;
        }
        case IRR_KEY_F11:
        {
            if (value)
            {
                if (control_is_pressed)
                {
                    history->Save();
                }
                else if (shift_is_pressed)
                {
#ifndef SERVER_ONLY
                    ShaderBasedRenderer* sbr = SP::getRenderer();
                    if (sbr)
                    {
                        sbr->dumpRTT();
                    }
#endif
                }
                else
                {
                    ReplayRecorder::get()->save();
                }
            }
            break;
        }
        case IRR_KEY_F12:
        {
            if (value)
            {
                if (control_is_pressed)
                {
                    UserConfigParams::m_karts_powerup_gui =
                    !UserConfigParams::m_karts_powerup_gui;
                }
                else if (shift_is_pressed)
                {
                    UserConfigParams::m_soccer_player_list =
                    !UserConfigParams::m_soccer_player_list;
                }
                else
                {
                    UserConfigParams::m_display_fps =
                    !UserConfigParams::m_display_fps;
                }
            }
            break;
        }
        default : break;
    }
}   // handleStaticAction

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
            OptionsScreenDevice::getInstance()->gotSensedInput(sensed_input);
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
            sensed_input.m_axis_direction = Input::AD_NEUTRAL;
            OptionsScreenDevice::getInstance()->gotSensedInput(sensed_input);
            return;
        }
        break;
    case Input::IT_STICKMOTION:
        {
        // We have to save the direction in which the axis was moved.
        // This is done by storing it as a sign (and since button can
        // be zero, we add one before changing the sign).
        int input_button_id = value>=0 ? 1+button : -(1+button);
        std::tuple<int, int> input_id(deviceID, input_button_id);
        std::tuple<int, int> input_id_inv(deviceID, -input_button_id);

        bool id_was_high         = m_sensed_input_high_gamepad.find(input_id)
                                   != m_sensed_input_high_gamepad.end();
        bool inverse_id_was_high = m_sensed_input_high_gamepad.find(input_id_inv)
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
                OptionsScreenDevice::getInstance()->gotSensedInput(sensed_input);

            }
            else m_sensed_input_high_gamepad.insert(input_id);
        }
        // At least with xbox controller they can come to a 'rest' with a value of
        // around 6000! So in order to detect that an axis was released, we need to
        // test with a rather high deadzone value
        else if ( abs(value) < Input::MAX_VALUE/3.0f )
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
                OptionsScreenDevice::getInstance()->gotSensedInput(sensed_input);
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
                OptionsScreenDevice::getInstance()->gotSensedInput(sensed_input);
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
 *  to the currently active menu.
 *  It also handles whether the game is currently sensing input. It does so by
 *  suppressing the distribution of the input as a GameAction. Instead the
 *  input is stored in 'm_sensed_input' and GA_SENSE_COMPLETE is distributed. If
 *  however the input in question has resolved to GA_LEAVE this is treated as
 *  an attempt of the user to cancel the sensing. In that case GA_SENSE_CANCEL
 *  is distributed.
 *
 *  Note: It is the obligation of the called menu to switch of the sense mode.
 *
 */
void InputManager::dispatchInput(Input::InputType type, int deviceID,
                                 int button,
                                 Input::AxisDirection axisDirection, int value,
                                 bool shift_mask)
{
    // Updated in rendering from main loop
    if (!irr_driver->getDevice()->getEventReceiver())
        return;

    // Act different in input sensing mode.
    if (m_mode == INPUT_SENSE_KEYBOARD ||
        m_mode == INPUT_SENSE_GAMEPAD)
    {
        // Do not pick disabled gamepads for input sensing
         if (type == Input::IT_STICKBUTTON || type == Input::IT_STICKMOTION)
        {
             GamePadDevice *gPad = m_device_manager->getGamePadFromIrrID(deviceID);
             // This can happen in case of automatically ignored accelerator
             // devices, which are not part of stk's gamepad mapping.
             if (!gPad) return;
             DeviceConfig *conf = gPad->getConfiguration();
             if (!conf->isEnabled())
                 return;
         }

        inputSensing(type, deviceID, button, axisDirection,  value);
        return;
    }

    // Abort demo mode if a key is pressed during the race in demo mode,
    // if a dialog is not active
    if(dynamic_cast<DemoWorld*>(World::getWorld()) &&
       !GUIEngine::ModalDialog::isADialogActive() && value)
    {
        RaceManager::get()->exitRace();
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
    // their device). So if a key could not be mapped to any known binding,
    // fall back to check the defaults.
    if (!action_found &&
            StateManager::get()->getGameState() != GUIEngine::GAME &&
            type == Input::IT_KEYBOARD &&
            m_mode == MENU && m_device_manager->getAssignMode() == NO_ASSIGN)
    {
        action = PA_BEFORE_FIRST;

        if      (button == IRR_KEY_UP)            action = PA_MENU_UP;
        else if (button == IRR_KEY_DOWN)          action = PA_MENU_DOWN;
        else if (button == IRR_KEY_LEFT)          action = PA_MENU_LEFT;
        else if (button == IRR_KEY_RIGHT)         action = PA_MENU_RIGHT;
        else if (button == IRR_KEY_SPACE)         action = PA_MENU_SELECT;
        else if (button == IRR_KEY_RETURN)        action = PA_MENU_SELECT;
        else if (button == IRR_KEY_BUTTON_UP)     action = PA_MENU_DOWN;
        else if (button == IRR_KEY_BUTTON_DOWN)   action = PA_MENU_UP;
        else if (button == IRR_KEY_BUTTON_LEFT)   action = PA_MENU_LEFT;
        else if (button == IRR_KEY_BUTTON_RIGHT)  action = PA_MENU_RIGHT;
        else if (button == IRR_KEY_BUTTON_A)      action = PA_MENU_SELECT;
        else if (button == IRR_KEY_TAB)
        {
            if (shift_mask)
            {
                action = PA_MENU_UP;
            }
            else
            {
                action = PA_MENU_DOWN;
            }
        }

        if (button == IRR_KEY_RETURN || button == IRR_KEY_BUTTON_A)
        {
            if (GUIEngine::ModalDialog::isADialogActive() &&
                !GUIEngine::ScreenKeyboard::isActive())
            {
                GUIEngine::ModalDialog::onEnterPressed();
            }
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
            if (NetworkConfig::get()->isNetworking() &&
                NetworkConfig::get()->isAddingNetworkPlayers())
            {
                // Ignore release event
                if (value == 0)
                    return;
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
                if (device && (action == PA_FIRE || action == PA_MENU_SELECT) &&
                    !GUIEngine::ModalDialog::isADialogActive())
                {
                    GUIEngine::Screen* screen = GUIEngine::getCurrentScreen();
                    NetworkingLobby* lobby = dynamic_cast<NetworkingLobby*>(screen);
                    if (lobby!=NULL)
                    {
                        lobby->openSplitscreenDialog(device);
                    }
                    return;
                }
            }

            // Player is unjoining
            if ((player != NULL) && (action == PA_RESCUE ||
                                     action == PA_MENU_CANCEL ) )
            {
                // returns true if the event was handled
                // Ignore release event (otherwise it exit the kart selection screen
                // when back from race setup screen)
                if (value != 0 &&
                    KartSelectionScreen::getRunningInstance()->playerQuit( player ))
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
                if (value != 0 && (action == PA_FIRE || action == PA_MENU_SELECT))
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
                        KartSelectionScreen::getRunningInstance()->joinPlayer(device, NULL/*player profile*/);
                    }
                }
                else if (value != 0 && (action == PA_MENU_CANCEL))
                {
                    StateManager::get()->escapePressed();
                }
                return; // we're done here, ignore devices that aren't
                        // associated with players
            }
        }

        auto cl = LobbyProtocol::get<ClientLobby>();
        bool is_nw_spectator = cl && cl->isSpectator() &&
             StateManager::get()->getGameState() == GUIEngine::GAME &&
             !GUIEngine::ModalDialog::isADialogActive();

        // ... when in-game
        if (StateManager::get()->getGameState() == GUIEngine::GAME &&
             !GUIEngine::ModalDialog::isADialogActive()            &&
             !GUIEngine::ScreenKeyboard::isActive()                &&
             !RaceManager::get()->isWatchingReplay() && !is_nw_spectator)
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
#ifdef MOBILE_STK
            if (type == Input::IT_STICKBUTTON || type == Input::IT_STICKMOTION)
            {
                if (UserConfigParams::m_multitouch_draw_gui &&
                    irr_driver->getDevice()->isAccelerometerAvailable() &&
                    World::getWorld() && World::getWorld()->getRaceGUI() &&
                    World::getWorld()->getRaceGUI()->getMultitouchGUI() &&
                    !World::getWorld()->getRaceGUI()->getMultitouchGUI()->isSpectatorMode() &&
                    UserConfigParams::m_multitouch_controls != MULTITOUCH_CONTROLS_STEERING_WHEEL)
                {
                    // Disable accelerometer or gyroscope control if gamepad events trigger, see #4705
                    UserConfigParams::m_multitouch_controls = MULTITOUCH_CONTROLS_STEERING_WHEEL;
                    World::getWorld()->getRaceGUI()->recreateGUI();
                }
            }
#endif
        }
        else if (RaceManager::get() &&
            RaceManager::get()->isWatchingReplay() && !GUIEngine::ModalDialog::isADialogActive() &&
            StateManager::get()->getGameState() == GUIEngine::GAME)
        {
            // Get the first ghost kart
            World::getWorld()->getKart(0)
                ->getController()->action(action, abs(value));
        }
        // ... when in menus
        else
        {
            // reset timer when released
            if (abs(value) == 0 && type == Input::IT_STICKMOTION)
                m_gamepads_timer.erase(deviceID);

            // When in master-only mode, we can safely assume that players
            // are set up, contrarly to early menus where we accept every
            // input because players are not set-up yet
            if (m_master_player_only && player == NULL && !is_nw_spectator)
            {
                if (type == Input::IT_STICKMOTION ||
                    type == Input::IT_STICKBUTTON)
                {
                    GamePadDevice* gp =
                        getDeviceManager()->getGamePadFromIrrID(deviceID);

                    // Check for deadzone
                    if (gp != NULL && gp->moved(value))
                    {
                        //I18N: message shown when an input device is used but
                        // is not associated to any player
                        GUIEngine::showMessage(
                            _("Ignoring '%s'. You needed to join earlier to play!",
                            core::stringw(gp->getName().c_str())));
                    }
                }
                return;
            }

            // menu input
            if (m_gamepads_timer.find(deviceID) == m_gamepads_timer.end())
            {
                if (type == Input::IT_STICKMOTION &&
                    abs(value) > Input::MAX_VALUE * 2 / 3)
                {
                    m_gamepads_timer[deviceID] = 0.25f;
                }

                if (is_nw_spectator)
                {
                    cl->changeSpectateTarget(action, abs(value), type);
                    return;
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
    const float ORIENTATION_MULTIPLIER = 10.0f;
    if (event.EventType == EET_JOYSTICK_INPUT_EVENT)
    {
        for (int axis_id=0; axis_id<SEvent::SJoystickEvent::NUMBER_OF_AXES ;
              axis_id++)
        {
            if (!event.JoystickEvent.IsAxisChanged(axis_id))
                continue;
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

        GamePadDevice* gp =
            getDeviceManager()->getGamePadFromIrrID(event.JoystickEvent.Joystick);

        if (gp == NULL)
        {
            // Prevent null pointer crash
            return EVENT_BLOCK;
        }

        for(int i=0; i<gp->getNumberOfButtons(); i++)
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
            if (key == IRR_KEY_ESCAPE)
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
                if (key == IRR_KEY_BACK || key == IRR_KEY_SPACE ||
                    key == IRR_KEY_SHIFT)
                {
                    return EVENT_LET;
                }
                if (key >= IRR_KEY_0 && key <= IRR_KEY_Z)
                {
                    return EVENT_LET;
                }
            }

            const bool wasInTextBox = GUIEngine::isWithinATextBox();

            dispatchInput(Input::IT_KEYBOARD, event.KeyInput.Char, key,
                          Input::AD_POSITIVE, Input::MAX_VALUE,
                          event.KeyInput.Shift);

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
                if (key == IRR_KEY_BACK || key == IRR_KEY_SPACE ||
                    key == IRR_KEY_SHIFT)
                {
                    return EVENT_LET;
                }
                if (key >= IRR_KEY_0 && key <= IRR_KEY_Z)
                {
                    return EVENT_LET;
                }
            }

            dispatchInput(Input::IT_KEYBOARD, event.KeyInput.Char, key,
                          Input::AD_POSITIVE, 0, event.KeyInput.Shift);
            return EVENT_BLOCK; // Don't propagate key up events
        }
    }
    else if (event.EventType == EET_TOUCH_INPUT_EVENT)
    {
        MultitouchDevice* device = m_device_manager->getMultitouchDevice();
        unsigned int id = (unsigned int)event.TouchInput.ID;

        if (device != NULL && id < device->m_events.size())
        {
            device->m_events[id].id = id;
            device->m_events[id].x = event.TouchInput.X;
            device->m_events[id].y = event.TouchInput.Y;

            if (event.TouchInput.Event == ETIE_PRESSED_DOWN)
            {
                device->m_events[id].touched = true;
            }
            else if (event.TouchInput.Event == ETIE_LEFT_UP)
            {
                device->m_events[id].touched = false;
            }

            m_device_manager->updateMultitouchDevice();
            device->updateDeviceState(id);
        }
    }
    // Use the mouse to change the looking direction when first person view is activated
    else if (event.EventType == EET_MOUSE_INPUT_EVENT)
    {
        const int type = event.MouseInput.Event;

        if (type == EMIE_MOUSE_MOVED)
        {
            CameraFPS *cam = dynamic_cast<CameraFPS*>(Camera::getActiveCamera());
            if (cam)
            {
                // Center of the screen
                core::dimension2du screen_size = irr_driver->getActualScreenSize();
                int mid_x = (int) screen_size.Width / 2;
                int mid_y = (int) screen_size.Height / 2;
                // Relative mouse movement
                int diff_x = event.MouseInput.X - m_mouse_val_x;
                int diff_y = event.MouseInput.Y - m_mouse_val_y;
                float mouse_x = ((float) diff_x) *
                    UserConfigParams::m_fpscam_direction_speed;
                float mouse_y = ((float) diff_y) *
                    -UserConfigParams::m_fpscam_direction_speed;

                // No movement the first time it's used
                // At the moment there's also a hard limit because the mouse
                // gets reset to the middle of the screen and sometimes there
                // are more events fired than expected.
                if (m_mouse_val_x != -1 &&
                   (diff_x + diff_y) < 100 && (diff_x + diff_y) > -100)
                {
                    // Rotate camera
                    cam->applyMouseMovement(mouse_x, mouse_y);

                    // Reset mouse position to the middle of the screen when
                    // the mouse is far away
                    if (event.MouseInput.X < mid_x / 2 ||
                        event.MouseInput.X > (mid_x + mid_x / 2) ||
                        event.MouseInput.Y < mid_y / 2 ||
                        event.MouseInput.Y > (mid_y + mid_y / 2))
                    {
                        irr_driver->getDevice()->getCursorControl()->setPosition(mid_x, mid_y);
                        m_mouse_val_x = mid_x;
                        m_mouse_val_y = mid_y;
                    }
                    else
                    {
                        m_mouse_val_x = event.MouseInput.X;
                        m_mouse_val_y = event.MouseInput.Y;
                    }
                }
                else
                {
                    m_mouse_val_x = event.MouseInput.X;
                    m_mouse_val_y = event.MouseInput.Y;
                }
                return EVENT_BLOCK;
            }
            else
                // Reset mouse position
                m_mouse_val_x = m_mouse_val_y = -1;
        }
        else if (type == EMIE_MOUSE_WHEEL)
        {
            CameraFPS *cam = dynamic_cast<CameraFPS*>(Camera::getActiveCamera());
            if (cam)
            {
                // Use scrolling to change the maximum speed
                // Only test if it's more or less than 0 as it seems to be not
                // reliable accross more platforms.
                if (event.MouseInput.Wheel < 0)
                {
                    float vel = cam->getMaximumVelocity() - 3;
                    if (vel < 0.0f)
                        vel = 0.0f;
                    cam->setMaximumVelocity(vel);
                }
                else if (event.MouseInput.Wheel > 0)
                {
                    cam->setMaximumVelocity(cam->getMaximumVelocity() + 3);
                }
            }
        }

        // Simulate touch events if there is no real device
        if (UserConfigParams::m_multitouch_active > 1 &&
            !irr_driver->getDevice()->supportsTouchDevice())
        {
            MultitouchDevice* device = m_device_manager->getMultitouchDevice();

            if (device != NULL && (type == EMIE_LMOUSE_PRESSED_DOWN ||
                type == EMIE_LMOUSE_LEFT_UP || type == EMIE_MOUSE_MOVED))
            {
                device->m_events[0].id = 0;
                device->m_events[0].x = event.MouseInput.X;
                device->m_events[0].y = event.MouseInput.Y;

                if (type == EMIE_LMOUSE_PRESSED_DOWN)
                {
                    device->m_events[0].touched = true;
                }
                else if (type == EMIE_LMOUSE_LEFT_UP)
                {
                    device->m_events[0].touched = false;
                }

                m_device_manager->updateMultitouchDevice();
                device->updateDeviceState(0);
            }
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
    else if (event.EventType == EET_ACCELEROMETER_EVENT)
    {
        MultitouchDevice* device = m_device_manager->getMultitouchDevice();

        if (device && device->isAccelerometerActive())
        {
            m_device_manager->updateMultitouchDevice();

            float factor = UserConfigParams::m_multitouch_tilt_factor;
            factor = std::max(factor, 0.1f);
            if (UserConfigParams::m_multitouch_controls == MULTITOUCH_CONTROLS_GYROSCOPE)
            {
                device->updateOrientationFromAccelerometer((float)event.AccelerometerEvent.X,
                                                           (float)event.AccelerometerEvent.Y);
                device->updateAxisX(device->getOrientation() * ORIENTATION_MULTIPLIER / factor);
            }
            else
            {
                device->updateAxisX(float(event.AccelerometerEvent.Y) / factor);
            }
        }
    }
    else if (event.EventType == EET_GYROSCOPE_EVENT)
    {
        MultitouchDevice* device = m_device_manager->getMultitouchDevice();

        if (device && device->isGyroscopeActive())
        {
            m_device_manager->updateMultitouchDevice();

            float factor = UserConfigParams::m_multitouch_tilt_factor;
            factor = std::max(factor, 0.1f);
            device->updateOrientationFromGyroscope((float)event.GyroscopeEvent.Z);
            device->updateAxisX(device->getOrientation() * ORIENTATION_MULTIPLIER / factor);
        }
    }

    // block events in all modes but initial menus (except in text boxes to
    // allow typing, and except in modal dialogs in-game)
    // FIXME: 1) that's awful logic 2) that's not what the code below does,
    // events are never blocked in menus
    if (getDeviceManager()->getAssignMode() != NO_ASSIGN &&
        !GUIEngine::isWithinATextBox() &&
        (!GUIEngine::ModalDialog::isADialogActive() &&
        !GUIEngine::ScreenKeyboard::isActive() &&
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
                    m_mouse_val_x = m_mouse_val_y = -1;

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
            m_mouse_val_x = m_mouse_val_y = -1;
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

