//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Marianne Gagnon
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

#include <IGUIEnvironment.h>
#include <IGUIListBox.h>

#include "guiengine/abstract_state_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "input/input_manager.hpp"
#include "states_screens/state_manager.hpp"

using GUIEngine::EventHandler;
using GUIEngine::EventPropagation;

using namespace irr::gui;

// -----------------------------------------------------------------------------

EventHandler::EventHandler()
{
}

// -----------------------------------------------------------------------------

EventHandler::~EventHandler()
{
}

// -----------------------------------------------------------------------------

bool EventHandler::OnEvent (const SEvent &event)
{
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
             event.EventType == EET_KEY_INPUT_EVENT   ||
             event.EventType == EET_JOYSTICK_INPUT_EVENT)
    {
        // FIXME? it may be a bit unclean that all input events go trough the gui module
        const EventPropagation blockPropagation = input_manager->input(event);
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
                printf("The following message will not be printed in release mode:\n");
#else
            return true; // EVENT_BLOCK
#endif
            
            if (event.LogEvent.Level == irr::ELL_WARNING)
            {
                printf("[Irrlicht Warning] %s\n", event.LogEvent.Text);
            }
            else if (event.LogEvent.Level == irr::ELL_ERROR)
            {
                printf("[Irrlicht Error] %s\n", event.LogEvent.Text);
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
    
    if (!pressedDown && type == Input::IT_STICKMOTION) return;
    
    switch (action)
    {
        case PA_STEER_LEFT:
        case PA_MENU_LEFT:
        {
            Widget* w = GUIEngine::getFocusForPlayer(playerID);
            if (w == NULL) break;
                        
            Widget* widget_to_call = w;
            
            /* Find topmost parent. Stop looping if a widget event handler's is itself, to not fall
             in an infinite loop (this can happen e.g. in checkboxes, where they need to be
             notified of clicks onto themselves so they can toggle their state. )
             On the way, also notify everyone in the chain of the left press. */
            while (widget_to_call->m_event_handler != NULL && widget_to_call->m_event_handler != widget_to_call)
            {
                if (widget_to_call->leftPressed(playerID) == EVENT_LET)
                {
                    sendEventToUser(w, w->m_properties[PROP_ID], playerID);
                }
                widget_to_call = widget_to_call->m_event_handler;
            }
            
            
            if (widget_to_call->leftPressed(playerID) == EVENT_LET)
            {
                sendEventToUser(widget_to_call, widget_to_call->m_properties[PROP_ID], playerID);
            }
        }
        break;
            
        case PA_STEER_RIGHT:
        case PA_MENU_RIGHT:
        {
            Widget* w = GUIEngine::getFocusForPlayer(playerID);
            if (w == NULL) break;
            
            Widget* widget_to_call = w;
            /* Find topmost parent. Stop looping if a widget event handler's is itself, to not fall
             in an infinite loop (this can happen e.g. in checkboxes, where they need to be
             notified of clicks onto themselves so they can toggle their state. )
             On the way, also notify everyone in the chain of the right press */
            while (widget_to_call->m_event_handler != NULL && widget_to_call->m_event_handler != widget_to_call)
            {
                if (widget_to_call->rightPressed(playerID) == EVENT_LET)
                {
                    sendEventToUser(widget_to_call, w->m_properties[PROP_ID], playerID);
                }
                widget_to_call = widget_to_call->m_event_handler;
            }
            
            if (widget_to_call->rightPressed(playerID) == EVENT_LET)
            {
                sendEventToUser(widget_to_call, widget_to_call->m_properties[PROP_ID], playerID);
            }
        }
        break;
            
        case PA_ACCEL:
        case PA_MENU_UP:
            navigateUp(playerID, type, pressedDown);
            break;
            
        case PA_BRAKE:
        case PA_MENU_DOWN:
            navigateDown(playerID, type, pressedDown);
            break;
            
        case PA_RESCUE:
        case PA_MENU_CANCEL:
            if (pressedDown) GUIEngine::getStateManager()->escapePressed();
            break;
            
        case PA_FIRE:
        case PA_MENU_SELECT:
            if (pressedDown && !isWithinATextBox())
            {
                Widget* w = GUIEngine::getFocusForPlayer(playerID);
                if (w == NULL) break;
                
                // FIXME : consider returned value?
                onWidgetActivated( w, playerID );
            }
            break;
        default:
            return;
    }
}

// -----------------------------------------------------------------------------

EventHandler* event_handler_singleton = NULL;

EventHandler* EventHandler::get()
{
    if (event_handler_singleton == NULL)
    {
        event_handler_singleton = new EventHandler();
    }
    return event_handler_singleton;
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

const bool NAVIGATION_DEBUG = false;

#if 0
#pragma mark -
#pragma mark Private methods
#endif

void EventHandler::navigateUp(const int playerID, Input::InputType type, const bool pressedDown)
{
    //std::cout << "Naviagte up!\n";
    IGUIElement *el = NULL/*, *first=NULL*/, *closest=NULL;

    if (type == Input::IT_STICKBUTTON && !pressedDown)
        return;
    
    Widget* w = GUIEngine::getFocusForPlayer(playerID);
    if (w != NULL)
    {
        el = w->getIrrlichtElement();
    }

    
    // list widgets are a bit special, because up/down keys are also used
    // to navigate between various list items, not only to navigate between
    // components
    if (w != NULL && w->m_type == WTYPE_LIST)
    {
        ListWidget* list = (ListWidget*)w;
        
        const bool stay_within_list = list->getSelectionID() > 0;
        
        if (stay_within_list)
        {
            list->setSelectionID(list->getSelectionID()-1);
            return;
        }
        else
        {
            list->setSelectionID(-1);
        }
    }
    
    if (w != NULL && w->m_tab_up_root != -1)
    {
        Widget* up = GUIEngine::getWidget( w->m_tab_up_root );
        assert( up != NULL );

        el = up->getIrrlichtElement();
        
        if (el == NULL)
        {
            std::cerr << "WARNING : m_tab_down_root is set to an ID for which I can't find the widget\n";
            return;
        }
    }

    // don't allow navigating to any widget when a dialog is shown; only navigate to widgets in the dialog
    if (ModalDialog::isADialogActive() && !ModalDialog::getCurrent()->isMyIrrChild(el))
    {
        el = NULL;
    }
    
    bool found = false;
    
    // find closest widget
    if (el != NULL && el->getTabGroup() != NULL)
    {
        // if the current widget is e.g. 15, search for widget 14, 13, 12, ... (up to 10 IDs may be missing)
        for (int n=1; n<10 && !found; n++)
        {
            closest = GUIEngine::getGUIEnv()->getRootGUIElement()->getElementFromId(el->getTabOrder() - n, true);
                        
            if (closest != NULL && Widget::isFocusableId(closest->getID()))
            {
                if (NAVIGATION_DEBUG) std::cout << "Navigating up to " << closest->getID() << std::endl;
                Widget* closestWidget = GUIEngine::getWidget( closest->getID() );
                
                if (playerID != PLAYER_ID_GAME_MASTER && !closestWidget->m_supports_multiplayer) return;
                
                // if a dialog is shown, restrict to items in the dialog
                if (ModalDialog::isADialogActive() && !ModalDialog::getCurrent()->isMyChild(closestWidget))
                {
                    continue;
                }
                
                // when focusing a list by going up, select the last item of the list
                assert (closestWidget != NULL);
                
                closestWidget->setFocusForPlayer(playerID);

                if (closestWidget->m_type == WTYPE_LIST)
                {
                    IGUIListBox* list = (IGUIListBox*)(closestWidget->m_element);
                    assert(list != NULL);
                    
                    list->setSelected( list->getItemCount()-1 );
                    return;
                }
                found = true;
                
            }
        } // end for
        
    }
    
    if (!found)
    {    
        if (NAVIGATION_DEBUG)
        {
            std::cout << "EventHandler::navigateUp : warp around, selecting the last widget\n";
        }
        
        // select the last widget
        Widget* lastWidget = NULL;
        
        if (ModalDialog::isADialogActive())
        {
            lastWidget = ModalDialog::getCurrent()->getLastWidget();
        }
        else
        {
            Screen* screen = GUIEngine::getCurrentScreen();
            if (screen == NULL) return;
            lastWidget = screen->getLastWidget();
        }
        
        if (lastWidget != NULL) lastWidget->setFocusForPlayer(playerID);
    }
}

// -----------------------------------------------------------------------------

void EventHandler::navigateDown(const int playerID, Input::InputType type, const bool pressedDown)
{
    //std::cout << "Naviagte down!\n";

    IGUIElement *el = NULL, *closest = NULL;

    if (type == Input::IT_STICKBUTTON && !pressedDown)
        return;
    
    Widget* w = GUIEngine::getFocusForPlayer(playerID);
    if (w != NULL)
    {
        el = w->getIrrlichtElement();
    }
    //std::cout << "!!! Player " << playerID << " navigating down of " << w->m_element->getID() << std::endl;
    
    // list widgets are a bit special, because up/down keys are also used
    // to navigate between various list items, not only to navigate between
    // components
    if (w != NULL && w->m_type == WTYPE_LIST)
    {
        ListWidget* list = (ListWidget*)w;
        
        const bool stay_within_list = list->getSelectionID() < list->getItemCount()-1;
            
        if (stay_within_list)
        {
            list->setSelectionID(list->getSelectionID()+1);
            return;
        }
        else
        {
            list->setSelectionID(-1);
        }
    }
    
    if (w != NULL && w->m_tab_down_root != -1)
    {
        Widget* down = GUIEngine::getWidget( w->m_tab_down_root );
        assert(down != NULL);
        el = down->getIrrlichtElement();
        
        if (el == NULL)
        {
            std::cerr << "WARNING : m_tab_down_root is set to an ID for which I can't find the widget\n";
            return;
        }
    }

    // don't allow navigating to any widget when a dialog is shown; only navigate to widgets in the dialog
    if (ModalDialog::isADialogActive() && !ModalDialog::getCurrent()->isMyIrrChild(el))
    {
        el = NULL;
    }
    
    bool found = false;
    
    if (el != NULL && el->getTabGroup() != NULL)
    {
        // if the current widget is e.g. 5, search for widget 6, 7, 8, 9, ..., 15 (up to 10 IDs may be missing)
        for (int n=1; n<10 && !found; n++)
        {
            closest = GUIEngine::getGUIEnv()->getRootGUIElement()->getElementFromId(el->getTabOrder() + n, true);
            
            if (closest != NULL && Widget::isFocusableId(closest->getID()))
            {
                
                Widget* closestWidget = GUIEngine::getWidget( closest->getID() );
                if (playerID != PLAYER_ID_GAME_MASTER && !closestWidget->m_supports_multiplayer) return;
                
                // if a dialog is shown, restrict to items in the dialog
                if (ModalDialog::isADialogActive() && !ModalDialog::getCurrent()->isMyChild(closestWidget))
                {
                    continue;
                }
                
                if (NAVIGATION_DEBUG)
                {
                    std::cout << "Navigating down to " << closestWidget->getID() << "\n";
                }
                
                assert( closestWidget != NULL );
                closestWidget->setFocusForPlayer(playerID);
                
                // another list exception : when entering a list, select the first item
                if (closestWidget->m_type == WTYPE_LIST)
                {
                    IGUIListBox* list = (IGUIListBox*)(closestWidget->m_element);
                    assert(list != NULL);

                    list->setSelected(0);
                }
                
                found = true;
            }
        } // end for
    }
    
    if (!found)
    {

        if (NAVIGATION_DEBUG) std::cout << "Navigating down : warp around\n";
        
        // select the first widget
        Widget* firstWidget = NULL;
        
        if (ModalDialog::isADialogActive())
        {
            //std::cout <<  "w = ModalDialog::getCurrent()->getFirstWidget();\n";
            firstWidget = ModalDialog::getCurrent()->getFirstWidget();
        }
        else
        {
            Screen* screen = GUIEngine::getCurrentScreen();
            if (screen == NULL) return;
            firstWidget = screen->getFirstWidget();  
        }
        
        if (firstWidget != NULL)  firstWidget->setFocusForPlayer(playerID);
    }
}

// -----------------------------------------------------------------------------

void EventHandler::sendEventToUser(GUIEngine::Widget* widget, std::string& name, const int playerID)
{
    if (ModalDialog::isADialogActive())
    {
        if (ModalDialog::getCurrent()->processEvent(widget->m_properties[PROP_ID]) == EVENT_BLOCK)
        {
            return;
        }
    }

    getCurrentScreen()->eventCallback(widget, name, playerID);
}

// -----------------------------------------------------------------------------

EventPropagation EventHandler::onWidgetActivated(GUIEngine::Widget* w, const int playerID)
{
    if (w->m_deactivated) return EVENT_BLOCK;

    if (ModalDialog::isADialogActive())
    {
        if (ModalDialog::getCurrent()->processEvent(w->m_properties[PROP_ID]) == EVENT_BLOCK)
        {
            return EVENT_BLOCK;
        }
        if (w->m_event_handler == NULL) return EVENT_LET;
    }
    
    //std::cout << "**** widget activated : " << w->m_properties[PROP_ID].c_str() << " ****" << std::endl;
    
    Widget* parent = w->m_event_handler;
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
        
        if (parent->m_deactivated) return EVENT_BLOCK;
        
        /* notify the found event event handler, and also notify the main callback if the
         parent event handler says so */
        if (parent->transmitEvent(w, w->m_properties[PROP_ID], playerID) == EVENT_LET)
        {
            sendEventToUser(parent, parent->m_properties[PROP_ID], playerID);
        }
    }
    else
    {
        if (w->transmitEvent(w, w->m_properties[PROP_ID], playerID) == EVENT_LET)
        {
            sendEventToUser(w, w->m_properties[PROP_ID], playerID);
        }
    }
    
    return EVENT_BLOCK;
}

// -----------------------------------------------------------------------------

EventPropagation EventHandler::onGUIEvent(const SEvent& event)
{    
    if (event.EventType == EET_GUI_EVENT)
    {
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
                
                if (w->m_deactivated) return EVENT_BLOCK;
                
                // These events are only triggered by mouse (or so I hope)
                // The player that owns the mouser receives "game master" priviledges
                return onWidgetActivated(w, PLAYER_ID_GAME_MASTER);
                
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
                                
                if (!w->m_focusable) return GUIEngine::EVENT_BLOCK;
                                
                // When a modal dialog is shown, don't select widgets out of the dialog
                if (ModalDialog::isADialogActive() && !ModalDialog::getCurrent()->isMyChild(w))
                {
                    // check for parents too before discarding event
                    if (w->m_event_handler != NULL)
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
                    const int playerID = PLAYER_ID_GAME_MASTER; //input_manager->getPlayerKeyboardID();
                    
                    if (playerID == -1) break;
                    if (input_manager->masterPlayerOnly() && playerID != PLAYER_ID_GAME_MASTER) break;
                    
                    if (ribbon->mouseHovered(w, playerID) == EVENT_LET) sendEventToUser(ribbon, ribbon->m_properties[PROP_ID], playerID);
                    if (ribbon->m_event_handler != NULL) ribbon->m_event_handler->mouseHovered(w, playerID);
                    ribbon->setFocusForPlayer(playerID);
                }
                else
                {
                    // focus on hover for other widgets
                    // give the mouse "game master" priviledges
                    const int playerID = PLAYER_ID_GAME_MASTER; //input_manager->getPlayerKeyboardID();
                    if (input_manager->masterPlayerOnly() && playerID != PLAYER_ID_GAME_MASTER) break;
                    if (playerID != -1)
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
                if (!w->isFocusedForPlayer(playerID)) w->setFocusForPlayer(playerID);
                
                break;
            }
            case EGET_EDITBOX_ENTER:
            {
                // currently, enter pressed in text ctrl events can only happen in dialogs.
                // FIXME : find a cleaner way to route the event to its proper location
                if (ModalDialog::isADialogActive()) ModalDialog::onEnterPressed();
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

