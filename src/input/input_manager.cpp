//  $Id: plibdrv.cpp 757 2006-09-11 22:27:39Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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
#include "input/device_manager.hpp"

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

#include <SDL/SDL.h>

#include "input/input.hpp"
//#include "actionmap.hpp"
#include "user_config.hpp"

#include "main_loop.hpp"
#include "player.hpp"
#include "user_config.hpp"
#include "gui/state_manager.hpp"
#include "gui/engine.hpp"
#include "race_manager.hpp"
#include "modes/world.hpp"
#include "karts/kart.hpp"
#include "history.hpp"
#include "gui/race_gui.hpp"
#include "sdl_manager.hpp"

InputManager *input_manager;

//-----------------------------------------------------------------------------
/** Initialise SDL.
 */
InputManager::InputManager()
: m_sensed_input(0),
m_mode(BOOTSTRAP), m_mouse_val_x(0), m_mouse_val_y(0)
{
    
    m_device_manager = new DeviceManager();
    
    // init keyboard. TODO - load key bindings from file
    KeyboardDevice* default_device = new KeyboardDevice();
    default_device->loadDefaults();
    m_device_manager->add( default_device );
    
    initGamePadDevices();
}


// -----------------------------------------------------------------------------
/** Initialises joystick/gamepad info.
 */
void InputManager::initGamePadDevices()
{
    //int nextIndex = 0;
    
    // Prepare a list of connected joysticks.
    const int numSticks = SDL_NumJoysticks();
    
    std::cout << "SDL detects " << numSticks << " gamepads" << std::endl;
    
    for (int i = 0; i < numSticks; i++)
        m_device_manager->add(new GamePadDevice(i));
    
    // TODO - init gamepad devices from config file
    /*
     m_stick_infos = new GamePadDevice *[numSticks];
     std::vector<GamePadDevice *> *si = new std::vector<GamePadDevice *>;
     for (int i = 0; i < numSticks; i++)
     si->push_back(m_stick_infos[i] = new GamePadDevice(i));
     
     // Get the list of known configs and make a copy of it.
     std::vector<UserConfig::StickConfig *> *sc
     = new std::vector<UserConfig::StickConfig *>(*user_config->getStickConfigs());
     
     bool match;
     std::vector<GamePadDevice *>::iterator si_ite = si->begin();
     
     // FIXME: Visual Studio triggers an exception (in debug mode) when si 
     // becomes empty (incompatible iterators). This is apparently caused
     // by using erase. For now I added a work around by checking for 
     // si->size()>0, which solves the problem for me. But I have only one
     // gamepad, I'd suspect that with more gamepads the problem still exists.
     while (si->size()>0 && si_ite != si->end())
     {
     match = false;
     
     std::vector<UserConfig::StickConfig *>::iterator sc_ite = sc->begin();
     while (sc_ite != sc->end())
     {
     if (nextIndex <= (*sc_ite)->m_preferredIndex)
     nextIndex = (*sc_ite)->m_preferredIndex + 1;
     
     if (!(*si_ite)->m_id.compare((*sc_ite)->m_id))
     {
     // Connected stick matches a stored one.
     
     // Copy important properties.
     
     // Deadzone is taken only if its not null.
     if ((*sc_ite)->m_deadzone)
     (*si_ite)->m_deadzone = (*sc_ite)->m_deadzone;
     
     // Restore former used index and other properties.
     (*si_ite)->m_index = (*sc_ite)->m_preferredIndex;
     
     // Remove matching entries from the list to prevent double
     // allocation.
     sc->erase(sc_ite);
     si->erase(si_ite);
     
     match = true;
     
     break;
     }
     
     sc_ite++;
     }
     
     if (!match)
     si_ite++;
     }
     delete sc;
     
     // si now contains all those stick infos which have no stick config yet
     // and nextIndex is set to the next free index. 
     
     // Now add all those new sticks and generate a config for them.
     si_ite = si->begin();
     while (si_ite != si->end())
     {
     (*si_ite)->m_index = nextIndex;
     
     UserConfig::StickConfig *sc = new UserConfig::StickConfig((*si_ite)->m_id);
     sc->m_preferredIndex = nextIndex;
     sc->m_deadzone = DEADZONE_JOYSTICK;
     
     user_config->addStickConfig(sc);
     
     nextIndex++;
     si_ite++;
     }
     
     delete si;
     */
}   // initGamePadDevices

//-----------------------------------------------------------------------------
/** Destructor. Frees all data structures.
 */
InputManager::~InputManager()
{
    
    delete m_device_manager;
    
    //const int NUM_STICKS = SDL_NumJoysticks();
    //for (int i = 0; i < NUM_STICKS; i++)
    //    delete m_stick_infos[i];
    
    //delete [] m_stick_infos;
    
    // FIXME LEAK: delete m_action_map if defined
    
}   // ~InputManager


#define MAX_VALUE 32768

void InputManager::postIrrLichtMouseEvent(irr::EMOUSE_INPUT_EVENT type, const int x, const int y)
{
    irr::SEvent::SMouseInput evt;
    
    evt.Event = type;
    evt.X = x;
    evt.Y = y;
    
    irr::SEvent wrapper;
    wrapper.MouseInput = evt;
    wrapper.EventType = EET_MOUSE_INPUT_EVENT;
    
    GUIEngine::getDevice()->postEventFromUser(wrapper);
}

// TODO - make this do something
void InputManager::handleStaticAction(int key, int value)
{
    //if (value) return;
	
    static int isWireframe = false;
    
	switch (key)
	{
#ifdef DEBUG
		case SDLK_F1:
			if (race_manager->getNumPlayers() ==1 )
			{
				Kart* kart = RaceManager::getWorld()->getLocalPlayerKart(0);
                //				kart->setPowerup(POWERUP_BUBBLEGUM, 10000);
                kart->attach(ATTACH_ANVIL, 5);
			}
			break;
		case SDLK_F2:
            if (race_manager->getNumPlayers() ==1 )
			{
				Kart* kart = RaceManager::getPlayerKart(0);
				kart->setPowerup(POWERUP_PLUNGER, 10000);
			}
			break;
		case SDLK_F3:
			if (race_manager->getNumPlayers() ==1 )
			{
				Kart* kart = RaceManager::getPlayerKart(0);
				kart->setPowerup(POWERUP_CAKE, 10000);
			}
			break;
#endif
		case SDLK_F12:
			user_config->m_display_fps = !user_config->m_display_fps;
			if(user_config->m_display_fps)
			{
                getRaceGUI()->resetFPSCounter();
            }
			break;
		case SDLK_F11:
			glPolygonMode(GL_FRONT_AND_BACK, isWireframe ? GL_FILL : GL_LINE);
			isWireframe = ! isWireframe;
			break;
#ifndef WIN32
            // For now disable F9 toggling fullscreen, since windows requires
            // to reload all textures, display lists etc. Fullscreen can
            // be toggled from the main menu (options->display).
		case SDLK_F9:
            SDLManager::toggleFullscreen(false);   // 0: do not reset textures
			// Fall through to put the game into pause mode.
#endif
		case SDLK_ESCAPE:
            // TODO - show race menu
            // RaceManager::getWorld()->pause();
             //menu_manager->pushMenu(MENUID_RACEMENU);
            break;
		case SDLK_F10:
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
void InputManager::input(Input::InputType type, int id0, int id1, int id2, 
                         int value)
{
    
    // menu navigation. TODO : enable navigation with gamepads
    if(!StateManager::isGameState())
    {
        if(type == Input::IT_KEYBOARD)
        {
            irr::SEvent::SKeyInput evt;
            
            // std::cout << id0 << std::endl;
            
            if(id0 == SDLK_RETURN)
                evt.Key = irr::KEY_RETURN;
            else if(id0 == SDLK_UP)
                evt.Key = irr::KEY_UP;
            else if(id0 == SDLK_DOWN)
                evt.Key = irr::KEY_DOWN;
            else if(id0 == SDLK_RIGHT)
                evt.Key = irr::KEY_RIGHT;            
            else if(id0 == SDLK_LEFT)
                evt.Key = irr::KEY_LEFT;
            else
                return; // only those keys are accepted in menus for now.
            
            evt.PressedDown = value > MAX_VALUE/2;
            
            irr::SEvent wrapper;
            wrapper.KeyInput = evt;
            wrapper.EventType = EET_KEY_INPUT_EVENT;
            
            GUIEngine::getDevice()->postEventFromUser(wrapper);
        }
        
    }
    // in-game event handling
    else
    {        
        int player;
        PlayerAction action;
        
        const bool action_found = m_device_manager->mapInputToPlayerAndAction( type, id0, id1, id2, value, &player, &action );
        
        //GameAction ga = m_action_map->getEntry(type, id0, id1, id2);
#if 0 // TODO - input sensing
        // Act different in input sensing mode.
        if (m_mode >= INPUT_SENSE_PREFER_AXIS && 
            m_mode <= INPUT_SENSE_PREFER_BUTTON)
        {
            // Input sensing should be canceled.
            if (ga == GA_LEAVE && m_sensed_input->type==Input::IT_KEYBOARD)
            {
                handleGameAction(GA_SENSE_CANCEL, value);
            }
            // Stores the sensed input when the button/key/axes/<whatever> is
            // released only and is not used in a fixed mapping.
            else if (!user_config->isFixedInput(type, id0, id1, id2) )
            {
                // See if the new input should be stored. This happens if:
                // 1) the value is larger
                // 2) nothing has been saved yet
                // 3) the new event has the preferred type
                // The latter is necessary since some gamepads have analog
                // buttons that can return two different events when pressed
                bool store_new = abs(value) > m_max_sensed_input         ||
                m_max_sensed_type   == Input::IT_NONE                ||
                ( m_mode            == INPUT_SENSE_PREFER_AXIS && 
                 type              == Input::IT_STICKMOTION && 
                 m_max_sensed_type != Input::IT_STICKMOTION      )  ||
                ( m_mode            == INPUT_SENSE_PREFER_BUTTON && 
                 type              == Input::IT_STICKBUTTON && 
                 m_max_sensed_type != Input::IT_STICKBUTTON      );
                if(store_new)
                {
                    m_sensed_input->type = type;
                    m_sensed_input->id0  = id0;
                    m_sensed_input->id1  = id1;
                    m_sensed_input->id2  = id2;
                    m_max_sensed_input   = abs(value);
                    m_max_sensed_type    = type;
                }
                // Notify the completion of the input sensing if the key/stick/
                // ... is released.
                if(value==0)
                    handleGameAction(GA_SENSE_COMPLETE, 0);
            }
        }   // if m_mode==INPUT_SENSE_PREFER_{AXIS,BUTTON}
        else
#endif
        if (action_found)
        {
            RaceManager::getWorld()->getLocalPlayerKart(player)->action(action, abs(value));
        }
        else if(type == Input::IT_KEYBOARD)
        {
            // keyboard press not handled by device manager / bindings. Check static bindings...
            handleStaticAction( id0, value );
        }
    }
}   // input

//-----------------------------------------------------------------------------
/** Reads the SDL event loop, does some tricks with certain values and calls
 * input() if appropriate.
 *
 * Digital inputs get the value of 32768 when pressed (key/button press,
 * digital axis) because this is what SDL provides. Relative mouse inputs
 * which do not fit into this scheme are converted to match. This is done to
 * relieve the KartAction implementor from the need to think about different
 * input devices and how SDL treats them. The same input gets the value of 0
 * when released.
 *
 * Analog axes can have any value from [0, 32768].
 *
 * There are no negative values. Instead this is reported as an axis with a
 * negative direction. This simplifies input configuration and allows greater
 * flexibility (= treat 4 directions as four buttons).
 *
 */
void InputManager::input()
{
    SDL_Event ev;

    while(SDL_PollEvent(&ev))
    {
        switch(ev.type)
        {
            case SDL_QUIT:
                main_loop->abort();
                break;
                
            case SDL_KEYUP:
                input(Input::IT_KEYBOARD, ev.key.keysym.sym, 0, 0, 0);
                break;
            case SDL_KEYDOWN:
                
                // escape is a little special
                if(ev.key.keysym.sym == 27)
                {
                    StateManager::escapePressed();
                    return;
                }
                
                
                input(Input::IT_KEYBOARD, ev.key.keysym.sym,
#ifdef HAVE_IRRLICHT
                      // FIXME: not sure why this happens: with plib the unicode
                      // value is 0. Since all values defined in user_config 
                      // assume that the unicode value is 0, it does not work 
                      // with irrlicht, which has proper unicode values defined
                      // (keydown is not recognised, but keyup is). So for now
                      // (till user_config is migrated to ful lirrlicht support)
                      // we pass the 0 here artifically so that keyboard handling
                      // works.
                      0,
#else
                      ev.key.keysym.unicode, 
#endif
                      0, MAX_VALUE);
                
                break;
                
            case SDL_MOUSEMOTION:
                // Reports absolute pointer values on a separate path to the menu
                // system to avoid the trouble that arises because all other input
                // methods have only one value to inspect (pressed/release,
                // axis value) while the pointer has two.
                if (!m_mode)
                {
                    postIrrLichtMouseEvent(irr::EMIE_MOUSE_MOVED, ev.motion.x, ev.motion.y);
                }
                // If sensing input mouse movements are made less sensitive in order
                // to avoid it being detected unwantedly.
                else if (m_mode >= INPUT_SENSE_PREFER_AXIS &&
                         m_mode <= INPUT_SENSE_PREFER_BUTTON  )
                {
                    if (ev.motion.xrel <= -DEADZONE_MOUSE_SENSE)
                        input(Input::IT_MOUSEMOTION, 0, Input::AD_NEGATIVE, 0, 0);
                    else if (ev.motion.xrel >= DEADZONE_MOUSE_SENSE)
                        input(Input::IT_MOUSEMOTION, 0, Input::AD_POSITIVE, 0, 0);
                    
                    if (ev.motion.yrel <= -DEADZONE_MOUSE_SENSE)
                        input(Input::IT_MOUSEMOTION, 1, Input::AD_NEGATIVE, 0, 0);
                    else if (ev.motion.yrel >= DEADZONE_MOUSE_SENSE)
                        input(Input::IT_MOUSEMOTION, 1, Input::AD_POSITIVE, 0, 0);
                }
                else
                {
                    // Calculates new values for the mouse helper variables. It
                    // keeps them in the [-32768, 32768] range. The same values are
                    // used by SDL for stick axes. 
                    m_mouse_val_x = std::max(-32768, std::min(32768,
                                                              m_mouse_val_x + ev.motion.xrel
                                                              * MULTIPLIER_MOUSE));
                    m_mouse_val_y = std::max(-32768,
                                             std::min(32768, m_mouse_val_y + ev.motion.yrel
                                                      * MULTIPLIER_MOUSE));
                }
                break;
            case SDL_MOUSEBUTTONUP:
                postIrrLichtMouseEvent(irr::EMIE_LMOUSE_LEFT_UP, ev.motion.x, ev.motion.y);
                //input(Input::IT_MOUSEBUTTON, ev.button.button, 0, 0, 0);
                break;
                
            case SDL_MOUSEBUTTONDOWN:
                postIrrLichtMouseEvent(irr::EMIE_LMOUSE_PRESSED_DOWN, ev.motion.x, ev.motion.y);
                //input(Input::IT_MOUSEBUTTON, ev.button.button, 0, 0, 32768);
                break;
                
            case SDL_JOYAXISMOTION:
            {
                const int value = ev.jaxis.value;
                
                //if(user_config->m_gamepad_debug)
                {
                    printf("axis motion: which=%d axis=%d value=%d\n",
                           ev.jaxis.which, ev.jaxis.axis, value);
                }
                
                if(value < 0)
                {
                    /* // TODO - bring back those weird axis tricks. would be cool if
                     // they could happen inside the GamePadDevice class, for encapsulation
                    if (m_stick_infos[ev.jaxis.which]->m_prevAxisDirections[ev.jaxis.axis] == Input::AD_POSITIVE)
                    {
                        input(Input::IT_STICKMOTION, !m_mode ? 0 : stickIndex, ev.jaxis.axis, Input::AD_POSITIVE, 0);
                    }
                     */
                    input(Input::IT_STICKMOTION, ev.jaxis.which, ev.jaxis.axis, Input::AD_NEGATIVE, value);
                    //m_stick_infos[ev.jaxis.which]->m_prevAxisDirections[ev.jaxis.axis] = Input::AD_NEGATIVE;
                }
                else
                {
                    /* TODO - bring back those weird axis tricks. would be cool if
                     // they could happen inside the GamePadDevice class, for encapsulation
                    if (m_stick_infos[ev.jaxis.which]->m_prevAxisDirections[ev.jaxis.axis] == Input::AD_NEGATIVE)
                    {
                        input(Input::IT_STICKMOTION, !m_mode ? 0 : stickIndex, ev.jaxis.axis, Input::AD_NEGATIVE, 0);
                    }
                     */
                    // TODO - set stickIndex
                    input(Input::IT_STICKMOTION, ev.jaxis.which, ev.jaxis.axis, Input::AD_POSITIVE, value);
                    //m_stick_infos[ev.jaxis.which]->m_prevAxisDirections[ev.jaxis.axis] = Input::AD_POSITIVE;
                }
                
                /* TODO - bring gamepad back in, with new InputDevice interface
                 stickIndex = m_stick_infos[ev.jaxis.which]->m_index;
                 // If the joystick axis exceeds the deadzone report the input.
                 // In menu mode (mode = MENU = 0) the joystick number is reported
                 // to be zero in all cases. This has the neat effect that all
                 // joysticks can be used to control the menu.*/
#if 0
                if(ev.jaxis.value <= -m_stick_infos[ev.jaxis.which]->m_deadzone)
                {
                    if (m_stick_infos[ev.jaxis.which]
                        ->m_prevAxisDirections[ev.jaxis.axis] == Input::AD_POSITIVE)
                    {
                        input(Input::IT_STICKMOTION, !m_mode ? 0 : stickIndex,
                              ev.jaxis.axis, Input::AD_POSITIVE, 0);
                    }
                    input(Input::IT_STICKMOTION, !m_mode ? 0 : stickIndex,
                          ev.jaxis.axis, Input::AD_NEGATIVE, -ev.jaxis.value);
                    m_stick_infos[ev.jaxis.which]->m_prevAxisDirections[ev.jaxis.axis]
                    = Input::AD_NEGATIVE;
                }
                else if(ev.jaxis.value >= m_stick_infos[ev.jaxis.which]->m_deadzone)
                {
                    if (m_stick_infos[ev.jaxis.which]
                        ->m_prevAxisDirections[ev.jaxis.axis] == Input::AD_NEGATIVE)
                    {
                        input(Input::IT_STICKMOTION, !m_mode ? 0 : stickIndex,
                              ev.jaxis.axis, Input::AD_NEGATIVE, 0);
                    }
                    input(Input::IT_STICKMOTION, !m_mode ? 0 : stickIndex,
                          ev.jaxis.axis, Input::AD_POSITIVE, ev.jaxis.value);
                    m_stick_infos[ev.jaxis.which]->m_prevAxisDirections[ev.jaxis.axis]
                    = Input::AD_POSITIVE;
                }
                else
                {
                    // Axis stands still: This is reported once for digital axes and
                    // can be called multipled times for analog ones. Uses the
                    // previous direction in which the axis was triggered to
                    // determine which one has to be brought into the released
                    // state. This allows us to regard two directions of an axis
                    // as completely independent input variants (as if they where
                    // two buttons).
                    if (m_stick_infos[ev.jaxis.which]
                        ->m_prevAxisDirections[ev.jaxis.axis] == Input::AD_NEGATIVE)
                        input(Input::IT_STICKMOTION, !m_mode ? 0 : stickIndex,
                              ev.jaxis.axis, Input::AD_NEGATIVE, 0);
                    else if (m_stick_infos[ev.jaxis.which]
                             ->m_prevAxisDirections[ev.jaxis.axis] == Input::AD_POSITIVE)
                        input(Input::IT_STICKMOTION, !m_mode ? 0 : stickIndex,
                              ev.jaxis.axis, Input::AD_POSITIVE, 0);
                    
                    m_stick_infos[ev.jaxis.which]->m_prevAxisDirections[ev.jaxis.axis] = Input::AD_NEUTRAL;
                }
#endif
            }
                break;
            case SDL_JOYBUTTONUP:
                /* TODO - bring gamepad back in, with new InputDevice interface
                 stickIndex = m_stick_infos[ev.jbutton.which]->m_index;
                 */       
                
                // See the SDL_JOYAXISMOTION case label because of !m_mode thingie.
                input(Input::IT_STICKBUTTON, ev.jbutton.which, 
                      ev.jbutton.button, 0, 0);
                break;
            case SDL_JOYBUTTONDOWN:
                /* TODO - bring gamepad back in, with new InputDevice interface
                 stickIndex = m_stick_infos[ev.jbutton.which]->m_index;
                 */
                
                // See the SDL_JOYAXISMOTION case label because of !m_mode thingie.
                input(Input::IT_STICKBUTTON, ev.jbutton.which, 
                      ev.jbutton.button, 0, 32768);
                break;
            case SDL_USEREVENT:
                // TODO - GUI countdown
                break;
                // used in display_res_confirm for the countdown timer
                // (menu_manager->getCurrentMenu())->countdown();
                
        }  // switch
    }   // while (SDL_PollEvent())
    
    // Makes mouse behave like an analog axis.
    if (m_mouse_val_x <= -DEADZONE_MOUSE)
        input(Input::IT_MOUSEMOTION, 0, Input::AD_NEGATIVE, 0, -m_mouse_val_x);
    else if (m_mouse_val_x >= DEADZONE_MOUSE)
        input(Input::IT_MOUSEMOTION, 0, Input::AD_POSITIVE, 0, m_mouse_val_x);
    else
        m_mouse_val_x = 0;
    
    if (m_mouse_val_y <= -DEADZONE_MOUSE)
        input(Input::IT_MOUSEMOTION, 1, Input::AD_NEGATIVE, 0, -m_mouse_val_y);
    else if (m_mouse_val_y >= DEADZONE_MOUSE)
        input(Input::IT_MOUSEMOTION, 1, Input::AD_POSITIVE, 0, m_mouse_val_y);
    else
        m_mouse_val_y = 0;
    
}   // input

//-----------------------------------------------------------------------------
/** Retrieves the Input instance that has been prepared in the input sense
 * mode.
 *
 *
 * It is wrong to call it when not in input sensing mode anymore.
 */
Input &InputManager::getSensedInput()
{
    assert (m_mode >= INPUT_SENSE_PREFER_AXIS &&
            m_mode <= INPUT_SENSE_PREFER_BUTTON   );
    
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
                
                SDLManager::showPointer();
                
                // Fall through expected.
            case BOOTSTRAP:
                // Leaving boot strap mode.
                
                // Installs the action map for the menu.
                //  m_action_map = user_config->newMenuActionMap();
                
                m_mode = MENU;
                
                break;
            case INPUT_SENSE_PREFER_AXIS:
            case INPUT_SENSE_PREFER_BUTTON:
                // Leaving input sense mode.
                
                SDLManager::showPointer();
                
                // The order is deliberate just in case someone starts to make
                // STK multithreaded: m_sensed_input must not be 0 when
                // mode == INPUT_SENSE_PREFER_{AXIS,BUTTON}.
                m_mode = MENU;
                
                delete m_sensed_input;
                m_sensed_input = 0;
                
                break;
            case LOWLEVEL:
                // Leaving lowlevel mode.
                
                SDL_EnableUNICODE(SDL_DISABLE);
                
                SDLManager::showPointer();
                
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
            
            SDLManager::hidePointer();
            
            m_mode = INGAME;
            
            break;
        case INPUT_SENSE_PREFER_AXIS:
        case INPUT_SENSE_PREFER_BUTTON:
            // We must be in menu mode now in order to switch.
            assert (m_mode == MENU);
            
            // Reset the helper values for the relative mouse movement supresses to
            // the notification of them as an input.
            m_mouse_val_x      = m_mouse_val_y = 0;
            m_max_sensed_input = 0;
            m_max_sensed_type  = Input::IT_NONE;
            m_sensed_input     = new Input();
            
            SDLManager::hidePointer();
            
            m_mode = new_mode;
            
            break;
        case LOWLEVEL:
            // We must be in menu mode now in order to switch.
            assert (m_mode == MENU);
            
            SDL_EnableUNICODE(SDL_ENABLE);
            
            SDLManager::hidePointer();
            
            m_mode = LOWLEVEL;
            
            break;
        default:
            // Invalid mode.
            assert(false);
    }
}

