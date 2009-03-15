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

#include "sdldrv.hpp"

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

#include <SDL/SDL.h>

#include "input.hpp"
#include "actionmap.hpp"
#include "user_config.hpp"
#include "material_manager.hpp"
#include "main_loop.hpp"
#include "loader.hpp"
#include "player.hpp"
#include "user_config.hpp"
#include "items/item_manager.hpp"
#include "items/powerup_manager.hpp"
#include "items/attachment_manager.hpp"
#include "items/projectile_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "gui/font.hpp"
#include "gui/race_gui.hpp"
#include "gui/engine.hpp"
#include "gui/state_manager.hpp"

#define DEADZONE_MOUSE        150
#define DEADZONE_MOUSE_SENSE  200
#define DEADZONE_JOYSTICK    2000
#define MULTIPLIER_MOUSE      750

SDLDriver *inputDriver;

//-----------------------------------------------------------------------------
/** Initialise SDL.
 */
SDLDriver::SDLDriver()
    : m_sensed_input(0), m_action_map(0), m_main_surface(0), m_flags(0), m_stick_infos(0),
    m_mode(BOOTSTRAP), m_mouse_val_x(0), m_mouse_val_y(0)
{
    if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_TIMER) < 0)
    {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        exit(1);
    }

    m_flags = SDL_OPENGL | SDL_HWSURFACE;
        
    //detect if previous resolution crashed STK
    if (user_config->m_crashed)
    {
        //STK crashed last time
        user_config->m_crashed = false;  //reset flag
        // set window mode as a precaution
        user_config->m_fullscreen = false;
        // blacklist the res if not already done
        std::ostringstream o;
        o << user_config->m_width << "x" << user_config->m_height;
        std::string res = o.str();
        if (std::find(user_config->m_blacklist_res.begin(),
          user_config->m_blacklist_res.end(),res) == user_config->m_blacklist_res.end())
        {
            user_config->m_blacklist_res.push_back (o.str());
        }
        //use prev screen res settings if available
        if (user_config->m_width != user_config->m_prev_width
            || user_config->m_height != user_config->m_prev_height)
        {
            user_config->m_width = user_config->m_prev_width;
            user_config->m_height = user_config->m_prev_height;
        }
        else //set 'safe' resolution to return to
        {
            user_config->m_width = user_config->m_prev_width = 800;
            user_config->m_height = user_config->m_prev_height = 600;
        }
    }
    
    if(user_config->m_fullscreen)
        m_flags |= SDL_FULLSCREEN;
        
    setVideoMode(false);

    SDL_JoystickEventState(SDL_ENABLE);

    initStickInfos();

    // Get into menu mode initially.
    setMode(MENU);
}


// -----------------------------------------------------------------------------
/** Initialises joystick/gamepad info.
 */
void SDLDriver::initStickInfos()
{
    int nextIndex = 0;
    
    // Prepare a list of connected joysticks.
    const int numSticks = SDL_NumJoysticks();
    m_stick_infos = new StickInfo *[numSticks];
    std::vector<StickInfo *> *si = new std::vector<StickInfo *>;
    for (int i = 0; i < numSticks; i++)
        si->push_back(m_stick_infos[i] = new StickInfo(i));
        
    // Get the list of known configs and make a copy of it.
    std::vector<UserConfig::StickConfig *> *sc
        = new std::vector<UserConfig::StickConfig *>(*user_config->getStickConfigs());
    
    bool match;
    std::vector<StickInfo *>::iterator si_ite = si->begin();

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
}   // initStickInfos

//-----------------------------------------------------------------------------
/** Show cursor.
 */
void SDLDriver::showPointer()
{
  SDL_ShowCursor(SDL_ENABLE);
}   // showPointer

//-----------------------------------------------------------------------------
/** Hide cursor.
 */
void SDLDriver::hidePointer()
{
  SDL_ShowCursor(SDL_DISABLE);
}   // hidePointer

//-----------------------------------------------------------------------------
/** Toggles to fullscreen mode.
 */
void SDLDriver::toggleFullscreen(bool resetTextures)
{
    user_config->m_fullscreen = !user_config->m_fullscreen;

    m_flags = SDL_OPENGL | SDL_HWSURFACE;

    if(user_config->m_fullscreen)
    {
        m_flags |= SDL_FULLSCREEN;

        if(StateManager::isGameState())
          showPointer();
          
        // Store settings in user config file in case new video mode
        // causes a crash
        user_config->m_crashed = true; //set flag. 
        user_config->saveConfig();
    }
    else if(StateManager::isGameState())
        hidePointer();
            
    setVideoMode(resetTextures);
}   // toggleFullscreen

// -----------------------------------------------------------------------------
/** Sets the video mode. If 8 bit colours are not supported, 5 bits are used;
 *  and if this doesn't work, alpha is disabled, too - before giving up. So
 *  STK should now work with 16 bit windows.
 *  \param resetTextures Forces all textures to be reloaded after a change of 
 *                       resolution. Necessary with windows and Macs OpenGL 
 *                       versions.
 */
void SDLDriver::setVideoMode(bool resetTextures)
{
#if defined(WIN32) || defined(__APPLE__)
    if(resetTextures)
    {
        // Clear plib internal texture cache
        loader->endLoad();

        // Windows needs to reload all textures, display lists, ... which means
        // that all models have to be reloaded. So first, free all textures,
        // models, then reload the textures from materials.dat, then reload
        // all models, textures etc.

        // startScreen             -> removeTextures();
        attachment_manager      -> removeTextures();
        projectile_manager      -> removeTextures();
        item_manager            -> removeTextures();
        kart_properties_manager -> removeTextures();
        powerup_manager         -> removeTextures();

        material_manager->reInit();


        powerup_manager         -> loadPowerups();
        kart_properties_manager -> loadKartData();
        item_manager            -> loadDefaultItems();
        projectile_manager      -> loadData();
        attachment_manager      -> loadModels();

    //        startScreen             -> installMaterial();

        //FIXME: the font reinit funcs should be inside the font class
        //Reinit fonts
        delete_fonts();
        init_fonts();

        //TODO: this function probably will get deleted in the future; if
        //so, the widget_manager.hpp include has no other reason to be here.
        //widget_manager->reloadFonts();
    }
#endif
}   // setVideoMode

//-----------------------------------------------------------------------------
/** Destructor. Frees all data structures.
 */
SDLDriver::~SDLDriver()
{
    const int NUM_STICKS = SDL_NumJoysticks();
    for (int i = 0; i < NUM_STICKS; i++)
        delete m_stick_infos[i];
    
    delete [] m_stick_infos;

    // FIXME LEAK: delete m_action_map if defined
    SDL_FreeSurface(m_main_surface);

}   // ~SDLDriver

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
#define MAX_VALUE 32768

void postIrrLichtMouseEvent(irr::EMOUSE_INPUT_EVENT type, const int x, const int y)
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

void SDLDriver::input(Input::InputType type, int id0, int id1, int id2, 
                      int value)
{
    if(!StateManager::isGameState())
    {
        //http://irrlicht.sourceforge.net/docu/classirr_1_1_irrlicht_device.html#bf859e39f017b0403c6ed331e48e01df
        if(type == Input::IT_KEYBOARD)
        {
            irr::SEvent::SKeyInput evt;
            
            if(id0 == 9)
                evt.Key = irr::KEY_TAB;
            else if(id0 == 13)
                evt.Key = irr::KEY_RETURN;
            else if(id0 == 273)
                evt.Key = irr::KEY_UP;
            else if(id0 == 274)
                evt.Key = irr::KEY_DOWN;
            else if(id0 == 275)
                evt.Key = irr::KEY_RIGHT;            
            else if(id0 == 276)
                evt.Key = irr::KEY_LEFT;
            else
                evt.Key = (irr::EKEY_CODE) id0; // FIXME - probably won't work, need better input handling
            
            
            evt.PressedDown = value > MAX_VALUE/2;
            
            irr::SEvent wrapper;
            wrapper.KeyInput = evt;
            wrapper.EventType = EET_KEY_INPUT_EVENT;
            
            GUIEngine::getDevice()->postEventFromUser(wrapper);
        }

    }
    else
    {
        RaceGUI* menu = getRaceGUI(); // FIXME - input is handled in menu class??
        if(menu == NULL) return;
        
        GameAction ga = m_action_map->getEntry(type, id0, id1, id2);
        // Act different in input sensing mode.
        if (m_mode >= INPUT_SENSE_PREFER_AXIS && 
            m_mode <= INPUT_SENSE_PREFER_BUTTON)
        {
            // Input sensing should be canceled.
            if (ga == GA_LEAVE && m_sensed_input->type==Input::IT_KEYBOARD)
            {
                menu->handle(GA_SENSE_CANCEL, value);
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
                    menu->handle(GA_SENSE_COMPLETE, 0);
            }
        }   // if m_mode==INPUT_SENSE_PREFER_{AXIS,BUTTON}
        else if (ga != GA_NULL)
        {
            if(type==Input::IT_MOUSEBUTTON)
            {
                // If a mouse button is pressed, make sure that the
                // widget the mouse is on is actually highlighted (since
                // the highlighted widget is selected!)
                int x, y;
                SDL_GetMouseState( &x, &y );
                y = SDL_GetVideoSurface()->h - y;
                //menu->inputPointer( x, y );
            }
            
            // Lets the currently active menu handle the GameAction.
            menu->handle(ga, value);
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
void SDLDriver::input()
{
    SDL_Event ev;
    /* Logical joystick index that is reported to the higher game APIs which
     * may not be equal to SDL's joystick index.
     */
    int stickIndex;

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
           // if (m_mode == LOWLEVEL)
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
            if(user_config->m_gamepad_debug)
            {
                printf("axis motion: which %d axis %d value %d\n",
                    ev.jaxis.which, ev.jaxis.axis, ev.jaxis.value);
            }
            stickIndex = m_stick_infos[ev.jaxis.which]->m_index;
            // If the joystick axis exceeds the deadzone report the input.
            // In menu mode (mode = MENU = 0) the joystick number is reported
            // to be zero in all cases. This has the neat effect that all
            // joysticks can be used to control the menu.
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
                
                m_stick_infos[ev.jaxis.which]->m_prevAxisDirections[ev.jaxis.axis]
                    = Input::AD_NEUTRAL;
            }

            break;
        case SDL_JOYBUTTONUP:
            stickIndex = m_stick_infos[ev.jbutton.which]->m_index;
                    
            // See the SDL_JOYAXISMOTION case label because of !m_mode thingie.
            input(Input::IT_STICKBUTTON, !m_mode ? 0 : stickIndex, 
                  ev.jbutton.button, 0, 0);
            break;
        case SDL_JOYBUTTONDOWN:
            stickIndex = m_stick_infos[ev.jbutton.which]->m_index;

            // See the SDL_JOYAXISMOTION case label because of !m_mode thingie.
            input(Input::IT_STICKBUTTON, !m_mode ? 0 : stickIndex, 
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
  * The Instance has valid values of the last input sensing operation *only*
  * if called immediately after a BaseGUI::handle() implementation received
  * GA_SENSE_COMPLETE.
  *
  * It is wrong to call it when not in input sensing mode anymore.
  */
Input &SDLDriver::getSensedInput()
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
bool SDLDriver::isInMode(InputDriverMode expMode)
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
  * In menu mode the pointer is visible (and reports absolute values through
  * BaseGUI::inputKeyboard()) and the BaseGUI::handle() implementations can
  * receive GameAction values from GA_FIRST_MENU to GA_LAST_MENU.
  *
  * In ingame mode the pointer is invisible (and reports relative values)
  * and the BaseGUI::handle() implementations can receive GameAction values
  * from GA_FIRST_INGAME to GA_LAST_INGAME.
  *
  * In input sense mode the pointer is invisible (any movement reports are
  * suppressed). If an input happens it is stored internally and can be
  * retrieved through drv_getm_sensed_input() *after* GA_SENSE_COMPLETE has been
  * distributed to a menu (Normally the menu that received GA_SENSE_COMPLETE
  * will request the sensed input ...). If GA_SENSE_CANCEL is received instead
  * the user decided to cancel input sensing. No other game action values are
  * distributed in this mode.
  * 
  * In lowlevel mode the pointer is invisible (and reports relative values - 
  * this is just a side effect). BaseGUI::handle() can receive GameAction
  * values from GA_FIRST_MENU to GA_LAST_MENU. Additionally each key press is
  * distributed through BaseGUI::inputKeyboard(). This happens *before* the
  * same keypress is processed to be distributed as a GameAction. This was done
  * to make the effects of changing the input driver's mode from
  * BaseGUI::handle() implementations less strange. The way it is implemented
  * makes sure that such a change only affects the next keypress or keyrelease.
  * The same is not true for mode changes from within a BaseGUI::inputKeyboard()
  * implementation. It is therefore discouraged.
  *
  * And there is the bootstrap mode. You cannot switch to it and only leave it
  * once per application instance. It is the state the input driver is first.
  *
  */
void SDLDriver::setMode(InputDriverMode new_mode)
{
    switch (new_mode)
    {
    case MENU:
        switch (m_mode)
        {
        case INGAME:
            // Leaving ingame mode.
                
            if (m_action_map)
                delete m_action_map;
    
            // Reset the helper values for the relative mouse movement
            // supresses to the notification of them as an input.
            m_mouse_val_x = m_mouse_val_y = 0;
            
            showPointer();
            
            // Fall through expected.
        case BOOTSTRAP:
            // Leaving boot strap mode.
                
            // Installs the action map for the menu.
            m_action_map = user_config->newMenuActionMap();
            
            m_mode = MENU;

            break;
        case INPUT_SENSE_PREFER_AXIS:
        case INPUT_SENSE_PREFER_BUTTON:
            // Leaving input sense mode.
                
            showPointer();
            
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

            showPointer();

            m_mode = MENU;
            
            break;
        default:
            // Something is broken.
            assert (false);
        }
        
        break;
    case INGAME:
        // We must be in menu mode now in order to switch.
        assert (m_mode == MENU);
    
        if (m_action_map)
            delete m_action_map;

        // Installs the action map for the ingame mode.
        m_action_map = user_config->newIngameActionMap();

        hidePointer();

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
    
        hidePointer();
    
        m_mode = new_mode;
        
        break;
    case LOWLEVEL:
        // We must be in menu mode now in order to switch.
        assert (m_mode == MENU);
        
        SDL_EnableUNICODE(SDL_ENABLE);

        hidePointer();

        m_mode = LOWLEVEL;

        break;
    default:
        // Invalid mode.
        assert(false);
    }
}

// -----------------------------------------------------------------------------
/** Constructor for StickInfo.
 *  \param sdlIndex Index of stick.
 */
SDLDriver::StickInfo::StickInfo(int sdlIndex)
{
    m_sdlJoystick = SDL_JoystickOpen(sdlIndex);
    
    m_id = SDL_JoystickName(sdlIndex);
    
    const int count = SDL_JoystickNumAxes(m_sdlJoystick);
    m_prevAxisDirections = new Input::AxisDirection[count];
    
    for (int i = 0; i < count; i++)
        m_prevAxisDirections[i] = Input::AD_NEUTRAL;
    
    m_deadzone = DEADZONE_JOYSTICK;
    
    m_index = -1;
}   // StickInfo

// -----------------------------------------------------------------------------
/** Destructor for StickInfo.
 */
SDLDriver::StickInfo::~StickInfo()
{
    delete m_prevAxisDirections;
    
    SDL_JoystickClose(m_sdlJoystick);
}   // ~StickInfo
