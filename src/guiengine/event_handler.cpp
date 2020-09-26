//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Marianne Gagnon
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

#include "guiengine/event_handler.hpp"

#include "audio/music_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "guiengine/abstract_state_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/screen_keyboard.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "input/input_manager.hpp"
#include "modes/demo_world.hpp"
#include "modes/world.hpp"
#include "network/rewind_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/debug.hpp"
#include "utils/profiler.hpp"


#include <IGUIEnvironment.h>
#include <IGUIListBox.h>

#include <iostream>

using GUIEngine::EventHandler;
using GUIEngine::EventPropagation;
using GUIEngine::NavigationDirection;

using namespace irr::gui;

// -----------------------------------------------------------------------------

EventHandler::EventHandler()
{
    m_accept_events = false;
}

// -----------------------------------------------------------------------------

EventHandler::~EventHandler()
{
}

// -----------------------------------------------------------------------------

bool EventHandler::OnEvent (const SEvent &event)
{
    if (!m_accept_events && event.EventType != EET_LOG_TEXT_EVENT) return true;

    if(!Debug::onEvent(event))
        return false;
        
    if (ScreenKeyboard::isActive())
    {
        if (ScreenKeyboard::getCurrent()->onEvent(event))
            return true; // EVENT_BLOCK
    }
    
    // TO DEBUG HATS (when you don't actually have a hat)
    /*
    if (event.EventType == EET_KEY_INPUT_EVENT)
    {
        if (event.KeyInput.Key == 'W')
        {
            printf("Sending hat up event %i\n", event.KeyInput.PressedDown);
            SEvent evt2;
            evt2.EventType = EET_JOYSTICK_INPUT_EVENT;
            for (int n=0; n<SEvent::SJoystickEvent::NUMBER_OF_AXES; n++)
            {
                evt2.JoystickEvent.Axis[n] = 0.0f;
            }
            evt2.JoystickEvent.ButtonStates = 0;
            evt2.JoystickEvent.Joystick = 0;
            evt2.JoystickEvent.POV = (event.KeyInput.PressedDown ? 0 : 65535); // 0 degrees
            OnEvent(evt2);
            return false;
        }
        else if (event.KeyInput.Key == 'D')
        {
            printf("Sending hat right event %i\n", event.KeyInput.PressedDown);
            SEvent evt2;
            evt2.EventType = EET_JOYSTICK_INPUT_EVENT;
            for (int n=0; n<SEvent::SJoystickEvent::NUMBER_OF_AXES; n++)
            {
                evt2.JoystickEvent.Axis[n] = 0.0f;
            }
            evt2.JoystickEvent.ButtonStates = 0;
            evt2.JoystickEvent.Joystick = 0;
            evt2.JoystickEvent.POV = (event.KeyInput.PressedDown ? 9000 : 65535); // 90 degrees
            OnEvent(evt2);
            return false;
        }
        else if (event.KeyInput.Key == 'S')
        {
            printf("Sending hat down event %i\n", event.KeyInput.PressedDown);
            SEvent evt2;
            evt2.EventType = EET_JOYSTICK_INPUT_EVENT;
            for (int n=0; n<SEvent::SJoystickEvent::NUMBER_OF_AXES; n++)
            {
                evt2.JoystickEvent.Axis[n] = 0.0f;
            }
            evt2.JoystickEvent.ButtonStates = 0;
            evt2.JoystickEvent.Joystick = 0;
            evt2.JoystickEvent.POV = (event.KeyInput.PressedDown ? 18000 : 65535); // 180 degrees
            OnEvent(evt2);
            return false;
        }
        else if (event.KeyInput.Key == 'A')
        {
            printf("Sending hat left event %i\n", event.KeyInput.PressedDown);
            SEvent evt2;
            evt2.EventType = EET_JOYSTICK_INPUT_EVENT;
            for (int n=0; n<SEvent::SJoystickEvent::NUMBER_OF_AXES; n++)
            {
                evt2.JoystickEvent.Axis[n] = 0.0f;
            }
            evt2.JoystickEvent.ButtonStates = 0;
            evt2.JoystickEvent.Joystick = 0;
            evt2.JoystickEvent.POV = (event.KeyInput.PressedDown ? 27000 : 65535); // 270 degrees
            OnEvent(evt2);
            return false;
        }
    }
    */

    // We do this (seemingly) overzealously to make sure that:
    //  1. It resets on any GUI events
    //  2. It resets on any mouse/joystick movement
    //  3. It resets on any keyboard presses
    if ((StateManager::get()->getGameState() == MENU)
        && (event.EventType != EET_LOG_TEXT_EVENT   )
        && (event.EventType != EET_USER_EVENT       ))
    {
        DemoWorld::resetIdleTime();
    }

    if (event.EventType == EET_GUI_EVENT)
    {
        return onGUIEvent(event) == EVENT_BLOCK;
    }
    else if (GUIEngine::getStateManager()->getGameState() != GUIEngine::GAME &&
             event.EventType != EET_KEY_INPUT_EVENT && event.EventType != EET_JOYSTICK_INPUT_EVENT &&
             event.EventType != EET_LOG_TEXT_EVENT)
    {
        return false; // EVENT_LET
    }
    else if (event.EventType == EET_MOUSE_INPUT_EVENT ||
             event.EventType == EET_TOUCH_INPUT_EVENT ||
             event.EventType == EET_KEY_INPUT_EVENT   ||
             event.EventType == EET_JOYSTICK_INPUT_EVENT ||
             event.EventType == EET_ACCELEROMETER_EVENT ||
             event.EventType == EET_GYROSCOPE_EVENT)
    {
        // Remember the mouse position
        if (event.EventType == EET_MOUSE_INPUT_EVENT &&
            event.MouseInput.Event == EMIE_MOUSE_MOVED)
        {
            m_mouse_pos.X = event.MouseInput.X;
            m_mouse_pos.Y = event.MouseInput.Y;
        }

        // Notify the profiler of mouse events
        if(UserConfigParams::m_profiler_enabled &&
           event.EventType == EET_MOUSE_INPUT_EVENT &&
           event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP)
        {
            profiler.onClick(m_mouse_pos);
        }

        if (event.EventType == EET_MOUSE_INPUT_EVENT &&
            event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN &&
            StateManager::get()->getGameState() == GAME)
        {
            World::getWorld()->onMouseClick(event.MouseInput.X, event.MouseInput.Y);
        }

        if (UserConfigParams::m_keyboard_debug)
        {
            Log::verbose("keyboard", "char %d key %d ctrl %d down %d shift %d",
                event.KeyInput.Char, event.KeyInput.Key,
                event.KeyInput.Control, event.KeyInput.PressedDown, 
                event.KeyInput.Shift);
        }
        // FIXME? it may be a bit unclean that all input events go trough
        // the gui module
        const EventPropagation blockPropagation = input_manager->input(event);

        if (event.EventType == EET_KEY_INPUT_EVENT &&
            event.KeyInput.Key == irr::IRR_KEY_TAB)
        {
            // block all tab events, if we let them go, irrlicht will try
            // to apply its own focus code
            return true; // EVENT_BLOCK
        }

        return blockPropagation == EVENT_BLOCK;
    }
    else if (event.EventType == EET_LOG_TEXT_EVENT)
    {
        // Ignore 'normal' messages
        if (event.LogEvent.Level > irr:: ELL_INFORMATION)
        {
            // Unfortunatly irrlicht produces some internal error/warnings
            // messages that can't be avoided (see COpenGLTexture where
            // the constructor of COpenGLFBOTexture is used without
            // specifying a color format, so the detault format (ECF_UNKOWNO)
            // is used, which produces this error message). In non-debug
            // mode ignore this error message, but leave it in for debugging.
            if(std::string(event.LogEvent.Text)=="Unsupported texture format")
#ifdef DEBUG
                Log::info("EventHandler", "The following message will not be printed in release mode");
#else
            return true; // EVENT_BLOCK
#endif
            const std::string &error_info = STKTexManager::getInstance()->getTextureErrorMessage();
            if (event.LogEvent.Level == irr::ELL_WARNING)
            {
                if(error_info.size()>0)
                    Log::warn("EventHandler", error_info.c_str());
                Log::warn("Irrlicht", event.LogEvent.Text);
            }
            else if (event.LogEvent.Level == irr::ELL_ERROR)
            {
                if(error_info.size()>0)
                    Log::error("EventHandler", error_info.c_str());
                Log::error("Irrlicht", event.LogEvent.Text);
            }
        }
        return true;
    }


    // nothing to do with other events
    return false;
}

// -----------------------------------------------------------------------------

void EventHandler::processGUIAction(const PlayerAction action,
                                    int deviceID,
                                    const unsigned int value,
                                    Input::InputType type,
                                    const int playerID)
{
    Screen* screen = GUIEngine::getCurrentScreen();
    if (screen != NULL)
    {
        EventPropagation propg = screen->filterActions(action, deviceID, value,
                                                       type, playerID);
        if (propg == EVENT_BLOCK) return;
    }

    const bool pressedDown = value > Input::MAX_VALUE*2/3;

    if (!pressedDown) return;

    switch (action)
    {
        case PA_STEER_LEFT:
        case PA_MENU_LEFT:
        {
            sendNavigationEvent(NAV_LEFT, playerID);
            break;
        }

        case PA_STEER_RIGHT:
        case PA_MENU_RIGHT:
        {
            sendNavigationEvent(NAV_RIGHT, playerID);
            break;
        }

        case PA_ACCEL:
        case PA_MENU_UP:
        {
            if (type == Input::IT_STICKBUTTON && !pressedDown)
                break;
            sendNavigationEvent(NAV_UP, playerID);
            break;
        }

        case PA_BRAKE:
        case PA_MENU_DOWN:
        {
            if (type == Input::IT_STICKBUTTON && !pressedDown)
                break;
            sendNavigationEvent(NAV_DOWN, playerID);
            break;
        }

        case PA_RESCUE:
        case PA_MENU_CANCEL:
            if (pressedDown&& !isWithinATextBox())
            {
                GUIEngine::getStateManager()->escapePressed();
            }
            break;

        case PA_FIRE:
        case PA_MENU_SELECT:
            if (pressedDown)
            {
                Widget* w = GUIEngine::getFocusForPlayer(playerID);
                if (w == NULL) break;

                // FIXME : consider returned value?
                onWidgetActivated(w, playerID, type);
            }
            break;
        default:
            return;
    }
}

// -----------------------------------------------------------------------------

static EventHandler* event_handler_singleton = NULL;

EventHandler* EventHandler::get()
{
    if (event_handler_singleton == NULL)
    {
        event_handler_singleton = new EventHandler();
    }
    return event_handler_singleton;
}

void EventHandler::deallocate()
{
    delete event_handler_singleton;
    event_handler_singleton = NULL;
}   // deallocate


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#if 0
#pragma mark -
#pragma mark Private methods
#endif

void EventHandler::sendNavigationEvent(const NavigationDirection nav, const int playerID)
{
    Widget* w = GUIEngine::getFocusForPlayer(playerID);
    
    if (w != NULL)
    {
        if (ScreenKeyboard::isActive())
        {
            if (!ScreenKeyboard::getCurrent()->isMyIrrChild(w->getIrrlichtElement()))
            {
                w = NULL;
            }
        }
        else if (ModalDialog::isADialogActive())
        {
            if (!ModalDialog::getCurrent()->isMyIrrChild(w->getIrrlichtElement()))
            {
                w = NULL;
            }
        }
    }
    
    if (w == NULL)
    {
        Widget* defaultWidget = NULL;
        
        if (ScreenKeyboard::isActive())
        {
            defaultWidget = ScreenKeyboard::getCurrent()->getFirstWidget();
        }
        else if (ModalDialog::isADialogActive())
        {
            defaultWidget = ModalDialog::getCurrent()->getFirstWidget();
        }
        else if (GUIEngine::getCurrentScreen() != NULL)
        {
            defaultWidget = GUIEngine::getCurrentScreen()->getFirstWidget();
        }

        if (defaultWidget != NULL)
        {
            if (playerID != PLAYER_ID_GAME_MASTER && !defaultWidget->m_supports_multiplayer)
                return;
            defaultWidget->setFocusForPlayer(playerID);
        }
        return;
    }

    Widget* widget_to_call = w;

    bool handled_by_widget = false;
    EventPropagation propagation_state = EVENT_BLOCK;

    /* Find topmost parent. Stop looping if a widget event handler's is itself, to not fall
     in an infinite loop (this can happen e.g. in checkboxes, where they need to be
     notified of clicks onto themselves so they can toggle their state. )
     On the way, also notify everyone in the chain of the press. */
    do
    {
        if (nav == NAV_LEFT)
            propagation_state = widget_to_call->leftPressed(playerID);
        else if (nav == NAV_RIGHT)
            propagation_state = widget_to_call->rightPressed(playerID);
        else if (nav == NAV_UP)
            propagation_state = widget_to_call->upPressed(playerID);
        else if (nav == NAV_DOWN)
            propagation_state = widget_to_call->downPressed(playerID);

        if (propagation_state == EVENT_LET || propagation_state == EVENT_BLOCK_BUT_HANDLED)
            handled_by_widget = true;

        if (propagation_state == EVENT_LET)
            sendEventToUser(widget_to_call, widget_to_call->m_properties[PROP_ID], playerID);

        if (widget_to_call->m_event_handler == NULL)
            break;

        widget_to_call = widget_to_call->m_event_handler;
    } while (widget_to_call->m_event_handler != widget_to_call);

    if (!handled_by_widget)
    {
        navigate(nav, playerID);
    }
} // sendNavigationEvent


/**
 * Focus the next widget downards, upwards, leftwards or rightwards.
 *
 * \param nav Determine in which direction to navigate
 */
void EventHandler::navigate(const NavigationDirection nav, const int playerID)
{
    Widget* w = GUIEngine::getFocusForPlayer(playerID);

    int next_id = findIDClosestWidget(nav, playerID, w, false);

    if (next_id != -1)
    {
        Widget* closest_widget = GUIEngine::getWidget(next_id);
        closest_widget->setFocusForPlayer(playerID);

        // A list exception : when entering a list by going down/left/right, select the first item
        // when focusing a list by going up, select the last item of the list
        if (closest_widget->m_type == WTYPE_LIST)
        {
            ListWidget* list = (ListWidget*) closest_widget;
            assert(list != NULL);
            list->focusHeader(nav);
        }
        // Similar exception for vertical tabs, only apply when entering with down/up
        if (closest_widget->m_type == GUIEngine::WTYPE_RIBBON && (nav == NAV_UP || nav == NAV_DOWN))
        {
            RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(closest_widget);
            assert(ribbon != NULL);
            if (ribbon->getRibbonType() == GUIEngine::RibbonType::RIBBON_VERTICAL_TABS)
            {
                int new_selection = (nav == NAV_UP) ?
                                   ribbon->getActiveChildrenNumber(playerID) - 1 : 0;
                ribbon->setSelection(new_selection, playerID);
                // The tab selection triggers an action
                sendEventToUser(ribbon, ribbon->m_properties[PROP_ID], playerID);
            }
        }

        // For spinners, select the most intuitive button
        // based on where the navigation came from
        // Right if coming from right by a left press
        // Left for all other directions
        if (closest_widget->getType() == WTYPE_SPINNER)
        {
            SpinnerWidget* spinner = dynamic_cast<SpinnerWidget*>(closest_widget);
            spinner->setSelectedButton(nav == NAV_LEFT);
        }
    }

    return;

} // navigate

/**
 * This function use simple heuristic to find the closest widget
 * in the requested direction,
 * It prioritize widgets close vertically to widget close horizontally,
 * as it is expected behavior in any direction.
 * Several hardcoded values are used, having been found to work well
 * experimentally while keeping simple heuristics.
 */
int EventHandler::findIDClosestWidget(const NavigationDirection nav, const int playerID,
                                      GUIEngine::Widget* w, bool ignore_disabled, int recursion_counter)
{
    int closest_widget_id = -1;
    int distance = 0;
    // So that the UI behavior don't change when it is upscaled
    const int BIG_DISTANCE = irr_driver->getActualScreenSize().Width*100;
    int smallest_distance = BIG_DISTANCE;
    // Used when there is no suitable widget in the requested direction
    int closest_wrapping_widget_id = -1;
    int wrapping_distance = 0;
    int smallest_wrapping_distance = BIG_DISTANCE;

    // In theory, it's better to look recursively in m_widgets
    // In practice, this is much much simpler and work equally well
    // Usual widget IDs begin at 100 and there is rarely more
    // than a few dozen of them - but it's very cheap to have some margin,
    for (int i=0;i<1000;i++)
    {
        Widget* w_test = GUIEngine::getWidget(i);

        // The widget id is invalid if :
        // - it doesn't match a widget
        // - it doesn't match a focusable widget
        // - it corresponds to the current widget
        // - it corresponds to an invisible or disabled widget
        // - the player is not allowed to select it
        if (w_test == NULL || !Widget::isFocusableId(i) || w == w_test ||
            (!w_test->isVisible()   && ignore_disabled) ||
            (!w_test->isActivated() && ignore_disabled) ||
            (playerID != PLAYER_ID_GAME_MASTER && !w_test->m_supports_multiplayer))
            continue;

        // Ignore empty ribbon widgets and lists
        if (w_test->m_type == GUIEngine::WTYPE_RIBBON)
        {
            RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(w_test);
            assert(ribbon != NULL);
            if (ribbon->getActiveChildrenNumber(playerID) == 0)
                continue;
        }
        else if (w_test->m_type == WTYPE_LIST)
        {
            ListWidget* list = (ListWidget*) w_test;
            assert(list != NULL);
            if (list->getItemCount() == 0)
                continue;
        }

        // if a dialog is shown, restrict to items in the dialog
        if (ScreenKeyboard::isActive())
        {
            if (!ScreenKeyboard::getCurrent()->isMyChild(w_test))
                continue;
        }
        else if (ModalDialog::isADialogActive())
        {
            if (!ModalDialog::getCurrent()->isMyChild(w_test))
                continue;
        }

        int offset = 0;
        int rightmost = w_test->m_x + w_test->m_w;

        if (nav == NAV_UP || nav == NAV_DOWN)
        {
            if (nav == NAV_UP)
            {
                // Compare current top point with other widget lowest point
                distance = w->m_y - (w_test->m_y + w_test->m_h);
            }
            else
            {
                // compare current lowest point with other widget top point
                distance = w_test->m_y - (w->m_y + w->m_h);
            }

            // Better select an item on the side that one much higher,
            // so make the vertical distance matter much more
            // than the horizontal offset.
            // The multiplicator of 100 is meant so that the offset will matter
            // only if there are two or more widget with a (nearly) equal vertical height.
            distance *= 100;

            wrapping_distance = distance;
            // if the two widgets are not well aligned, consider them farther
            // If w's left/right-mosts points are between w_test's,
            // right_offset and left_offset will be 0.
            // If w_test's are between w's,
            // we subtract the smaller from the bigger
            // else, the smaller is 0 and we keep the bigger
            int right_offset = std::max(0, w_test->m_x - w->m_x);
            int left_offset  = std::max(0, (w->m_x + w->m_w) - rightmost);
            offset = std::max (right_offset - left_offset, left_offset - right_offset);
        }
        else if (nav == NAV_LEFT || nav == NAV_RIGHT)
        {
            if (nav == NAV_LEFT)
            {
                // compare current leftmost point with other widget rightmost
                distance = w->m_x - rightmost;
            }
            else
            {
                // compare current rightmost point with other widget leftmost
                distance = w_test->m_x - (w->m_x + w->m_w);
            }
            wrapping_distance = distance;

            int down_offset = std::max(0, w_test->m_y - w->m_y);
            int up_offset  = std::max(0, (w->m_y + w->m_h) - (w_test->m_y + w_test->m_h));
            offset = std::max (down_offset - up_offset, up_offset - down_offset);

            // Special case for vertical tabs : select the top element of the body
            if (w->m_type == GUIEngine::WTYPE_RIBBON)
            {
                RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(w);
                if (ribbon->getRibbonType() == GUIEngine::RibbonType::RIBBON_VERTICAL_TABS)
                {
                    offset = w_test->m_y;
                    offset *= 100;

                    // Don't count elements above the tabs or too high as valid
                    if (!(w_test->m_x > (w->m_x + w->m_w) || rightmost < w->m_x) ||
                         (w_test->m_y + w_test->m_h) < w->m_y)
                    {
                        distance = BIG_DISTANCE;
                        wrapping_distance = BIG_DISTANCE;
                    }
                }
            }

            // No lateral selection if there is not at least partial alignement
            // >= because we don't want it to trigger if two widgets touch each other
            else if (offset >= w->m_h)
            {
                distance = BIG_DISTANCE;
                wrapping_distance = BIG_DISTANCE;
            }
        }

        // This happens if the tested widget is in the opposite direction
        if (distance < 0)
            distance = BIG_DISTANCE;
        distance += offset;
        wrapping_distance += offset;

        if (distance < smallest_distance)
        {
            smallest_distance = distance;
            closest_widget_id = i;
        }
        if (wrapping_distance < smallest_wrapping_distance)
        {
            smallest_wrapping_distance = wrapping_distance;
            closest_wrapping_widget_id = i;
        }
    } // for i < 1000

    int closest_id = (smallest_distance < BIG_DISTANCE) ? closest_widget_id :
                                                          closest_wrapping_widget_id;
    Widget* w_test = GUIEngine::getWidget(closest_id);
    
    if (w_test == NULL)
        return -1;

    // If the newly found focus target is invisible, or not activated,
    // it is not a good target, search again
    // This allows to skip over disabled/invisible widgets in a grid
    if (!w_test->isVisible() || !w_test->isActivated())
    {
        // Can skip over at most 3 consecutive disabled/invisible widget
        if (recursion_counter <=2)
        {
            recursion_counter++;
            return findIDClosestWidget(nav, playerID, w_test, /*ignore disabled*/ false, recursion_counter);
        }
        // If nothing has been found, do a search ignoring disabled/invisible widgets,
        // restarting from the initial focused widget (otherwise, it could lead to weird results) 
        else if (recursion_counter == 3)
        {
            Widget* w_focus = GUIEngine::getFocusForPlayer(playerID);
            return findIDClosestWidget(nav, playerID, w_focus, /*ignore disabled*/ true, recursion_counter);
        }
    }

    return closest_id;
} // findIDClosestWidget

// -----------------------------------------------------------------------------

void EventHandler::sendEventToUser(GUIEngine::Widget* widget, std::string& name, const int playerID)
{
    if (ScreenKeyboard::isActive())
    {
        if (ScreenKeyboard::getCurrent()->processEvent(widget->m_properties[PROP_ID]) != EVENT_LET)
        {
            return;
        }
    }
    
    if (ModalDialog::isADialogActive())
    {
        if (ModalDialog::getCurrent()->processEvent(widget->m_properties[PROP_ID]) != EVENT_LET)
        {
            return;
        }
    }

    if (getCurrentScreen() != NULL)
        getCurrentScreen()->eventCallback(widget, name, playerID);
}

// -----------------------------------------------------------------------------

EventPropagation EventHandler::onWidgetActivated(GUIEngine::Widget* w, const int playerID, Input::InputType type)
{
    if (w->onActivationInput(playerID) == EVENT_BLOCK)
        return EVENT_BLOCK;

    Widget* parent = w->m_event_handler;
    
    //FIXME : sendEventToUser do the same screen keyboard and modal dialog checks, so they are done twice
    if (ScreenKeyboard::isActive())
    {
        if (ScreenKeyboard::getCurrent()->processEvent(w->m_properties[PROP_ID]) == EVENT_BLOCK)
        {
            return EVENT_BLOCK;
        }
    }

    if (ModalDialog::isADialogActive() && (parent == NULL || parent->m_type != GUIEngine::WTYPE_RIBBON))
    {
        if (ModalDialog::getCurrent()->processEvent(w->m_properties[PROP_ID]) == EVENT_BLOCK)
        {
            return EVENT_BLOCK;
        }
    }

    //Log::info("EventHandler", "Widget activated: %s", w->m_properties[PROP_ID].c_str());

    // For spinners, also trigger activation
    if (w->getType() == WTYPE_SPINNER)
    {
        SpinnerWidget* spinner = dynamic_cast<SpinnerWidget*>(w);
        spinner->activateSelectedButton();
    }

    if (w->m_event_handler != NULL)
    {
        /* Find all parents. Stop looping if a widget event handler's is itself, to not fall
         in an infinite loop (this can happen e.g. in checkboxes, where they need to be
         notified of clicks onto themselves so they can toggle their state. ) */
        while (parent->m_event_handler != NULL && parent->m_event_handler != parent)
        {
            parent->transmitEvent(w, w->m_properties[PROP_ID], playerID);

            parent = parent->m_event_handler;
        }

        if (!parent->isActivated()) return EVENT_BLOCK;

        /* notify the found event event handler, and also notify the main callback if the
         parent event handler says so */
        if (parent->transmitEvent(w, w->m_properties[PROP_ID], playerID) == EVENT_LET)
        {
            if (parent->isEventCallbackActive(type))
                sendEventToUser(parent, parent->m_properties[PROP_ID], playerID);
        }
    }
    else
    {
        std::string id = w->m_properties[PROP_ID];
        assert(w->ok());
        if (w->transmitEvent(w, id, playerID) == EVENT_LET)
        {
            assert(w->ok());
            if (w->isEventCallbackActive(type))
                sendEventToUser(w, id, playerID);
        }
    }

    return EVENT_BLOCK;
}

// -----------------------------------------------------------------------------

EventPropagation EventHandler::onGUIEvent(const SEvent& event)
{
    if (event.EventType == EET_GUI_EVENT)
    {
        if (event.GUIEvent.Caller == NULL) return EVENT_LET;
        const s32 id = event.GUIEvent.Caller->getID();

        switch (event.GUIEvent.EventType)
        {
            case EGET_BUTTON_CLICKED:
            case EGET_SCROLL_BAR_CHANGED:
            case EGET_CHECKBOX_CHANGED:
            case EGET_LISTBOX_SELECTED_AGAIN:
            {
                Widget* w = GUIEngine::getWidget(id);
                if (w == NULL) break;
                if (!w->isActivated())
                {
                    // Some dialog in overworld could have deactivated widget, and no current screen in overworld
                    if (GUIEngine::getCurrentScreen())
                        GUIEngine::getCurrentScreen()->onDisabledItemClicked(w->m_properties[PROP_ID].c_str());
                    return EVENT_BLOCK;
                }

                EventPropagation result = w->onClick();
                
                if (result == EVENT_BLOCK)
                    return result;

                // These events are only triggered by mouse (or so I hope)
                // The player that owns the mouser receives "game master" priviledges
                return onWidgetActivated(w, PLAYER_ID_GAME_MASTER, Input::IT_MOUSEBUTTON);

                // These events are only triggered by keyboard/mouse (or so I hope...)
                //const int playerID = input_manager->getPlayerKeyboardID();
                //if (input_manager->masterPlayerOnly() && playerID != PLAYER_ID_GAME_MASTER) break;
                //else if (playerID != -1) return onWidgetActivated(w, playerID);
                //else break;
            }
            case EGET_ELEMENT_HOVERED:
            {
                Widget* w = GUIEngine::getWidget(id);

                if (w == NULL) break;

                if (!w->isFocusable() || !w->isActivated()) return GUIEngine::EVENT_BLOCK;

                // When a modal dialog is shown, don't select widgets out of the dialog
                if (ScreenKeyboard::isActive())
                {
                    // check for parents too before discarding event
                    if (!ScreenKeyboard::getCurrent()->isMyChild(w) && 
                        w->m_event_handler != NULL)
                    {
                        if (!ScreenKeyboard::getCurrent()->isMyChild(w->m_event_handler))
                        {
                            break;
                        }
                    }
                }
                else if (ModalDialog::isADialogActive())
                {
                    // check for parents too before discarding event
                    if (!ModalDialog::getCurrent()->isMyChild(w) && 
                        w->m_event_handler != NULL)
                    {
                        if (!ModalDialog::getCurrent()->isMyChild(w->m_event_handler))
                        {
                            break;
                        }
                    }
                }

                // select ribbons on hover
                if (w->m_event_handler != NULL && w->m_event_handler->m_type == WTYPE_RIBBON)
                {
                    // FIXME: don't make a special case for ribbon here, there should be a generic callback
                    //        that all widgets may hook onto
                    RibbonWidget* ribbon = (RibbonWidget*)(w->m_event_handler);
                    if (ribbon == NULL) break;

                    // give the mouse "game master" priviledges
                    const int playerID = PLAYER_ID_GAME_MASTER;

                    ribbon->mouseHovered(w, playerID);
                    if (ribbon->m_event_handler != NULL) ribbon->m_event_handler->mouseHovered(w, playerID);
                    ribbon->setFocusForPlayer(playerID);
                }
                else
                {
                    // focus on hover for other widgets
                    // give the mouse "game master" priviledges
                    const int playerID = PLAYER_ID_GAME_MASTER;
                    {
                        // lists don't like that combined with scrollbars
                        // (FIXME: find why instead of working around)
                        if (w->getType() != WTYPE_LIST)
                        {
                            w->setFocusForPlayer(playerID);
                        }
                    }
                }

                break;
            }
                /*
                 case EGET_ELEMENT_LEFT:
                 {
                 Widget* el = getWidget(id);
                 if(el == NULL) break;

                 break;
                 }
                 */

            case EGET_ELEMENT_FOCUSED:
            {
                Widget* w = GUIEngine::getWidget(id);
                if (w == NULL) break;

                // forbid list for gaining "irrLicht focus", then they will process key events and
                // we don't want that since we do our own custom processing for keys
                if (w->m_type == WTYPE_LIST)
                {
                    // FIXME: fix that better
                    // cheap way to remove the focus from the element (nope, IGUIEnv::removeFocus doesn't work)
                    // Obviously will not work if the list is the first item of the screen.
                    IGUIElement* elem = getCurrentScreen()->getFirstWidget()->getIrrlichtElement();
                    if (elem->getType() == EGUIET_LIST_BOX)
                    {
                        elem = getCurrentScreen()->getLastWidget()->getIrrlichtElement();
                        assert(elem->getType() != EGUIET_LIST_BOX);
                    }
                    GUIEngine::getGUIEnv()->setFocus( elem );
                    return EVENT_BLOCK; // confirms to irrLicht that we processed it
                }


                break;
            }

            case EGET_LISTBOX_CHANGED:
            {
                Widget* w = GUIEngine::getWidget(id);
                if (w == NULL) break;
                assert(w->getType() == WTYPE_LIST);

                const int playerID = input_manager->getPlayerKeyboardID();
                if (input_manager->masterPlayerOnly() && playerID != PLAYER_ID_GAME_MASTER) break;
                if (playerID != -1 && !w->isFocusedForPlayer(playerID)) w->setFocusForPlayer(playerID);

                break;
            }
            case EGET_EDITBOX_ENTER:
            {
                // currently, enter pressed in text ctrl events can only happen in dialogs.
                // FIXME : find a cleaner way to route the event to its proper location
                if (!ScreenKeyboard::isActive() && ModalDialog::isADialogActive()) 
                    ModalDialog::onEnterPressed();
                break;
            }
            default:
                break;
        } // end switch
    }

    /*
     EGET_BUTTON_CLICKED, EGET_SCROLL_BAR_CHANGED, EGET_CHECKBOX_CHANGED, EGET_TAB_CHANGED,
     EGET_MENU_ITEM_SELECTED, EGET_COMBO_BOX_CHANGED, EGET_SPINBOX_CHANGED, EGET_EDITBOX_ENTER,

     EGET_LISTBOX_CHANGED, EGET_LISTBOX_SELECTED_AGAIN,
     EGET_FILE_SELECTED, EGET_FILE_CHOOSE_DIALOG_CANCELLED,
     EGET_MESSAGEBOX_YES, EGET_MESSAGEBOX_NO, EGET_MESSAGEBOX_OK, EGET_MESSAGEBOX_CANCEL,
     EGET_TABLE_CHANGED, EGET_TABLE_HEADER_CHANGED, EGET_TABLE_SELECTED_AGAIN
     EGET_ELEMENT_FOCUS_LOST, EGET_ELEMENT_FOCUSED, EGET_ELEMENT_HOVERED, EGET_ELEMENT_LEFT,
     EGET_ELEMENT_CLOSED,
     */
    return EVENT_LET;
}

// -----------------------------------------------------------------------------
