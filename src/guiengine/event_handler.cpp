//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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

#include "guiengine/abstract_state_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "input/input_manager.hpp"

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
    if(event.EventType == EET_GUI_EVENT ||
       (GUIEngine::getStateManager()->getGameState() != GUIEngine::GAME && event.EventType != EET_KEY_INPUT_EVENT &&
         event.EventType != EET_JOYSTICK_INPUT_EVENT)
       )
    {
        return onGUIEvent(event) == EVENT_BLOCK;
    }
    else
    {
        // FIXME : it's a bit unclean that all input events go trough the gui module
        const EventPropagation blockPropagation = input_manager->input(event);
        return blockPropagation == EVENT_BLOCK;
    }
    
    // to shut up a warning. gcc is too stupid too see the code will never get here
    return false;
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
                
                // FIXME: don't hardcode player 0
                return onWidgetActivated(w, 0);
            }    
            case EGET_ELEMENT_HOVERED:
            {
                Widget* w = GUIEngine::getWidget(id);
                if (w == NULL) break;
                
                // select ribbons on hover
                if (w->m_event_handler != NULL && w->m_event_handler->m_type == WTYPE_RIBBON)
                {
                    RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(w->m_event_handler);
                    if (ribbon == NULL) break;
                    const int playerID = 0; // FIXME : don't hardcode player 0
                    if (ribbon->mouseHovered(w) == EVENT_LET) transmitEvent(ribbon, ribbon->m_properties[PROP_ID], playerID);
                    if (ribbon->m_event_handler != NULL) ribbon->m_event_handler->mouseHovered(w);
                    getGUIEnv()->setFocus(ribbon->m_element);
                }
                else
                {
                    // focus on hover for other widgets
                    getGUIEnv()->setFocus(w->m_element);
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
                
                // FIXME: don't hardcode player 0
                return w->focused(0);
                
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

EventPropagation EventHandler::onWidgetActivated(GUIEngine::Widget* w, const int playerID)
{
    if (ModalDialog::isADialogActive())
    {
        if (ModalDialog::getCurrent()->processEvent(w->m_properties[PROP_ID]) == EVENT_BLOCK) return EVENT_BLOCK;
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
        
        /* notify the found event event handler, and also notify the main callback if the
         parent event handler says so */
        if (parent->transmitEvent(w, w->m_properties[PROP_ID], playerID) == EVENT_LET)
        {
            // notify modal dialog too
            if (ModalDialog::isADialogActive())
            {
                if (ModalDialog::getCurrent()->processEvent(parent->m_properties[PROP_ID]) == EVENT_BLOCK) return EVENT_BLOCK;
            }
            
            transmitEvent(parent, parent->m_properties[PROP_ID], playerID);
        }
    }
    else transmitEvent(w, w->m_properties[PROP_ID], playerID);
    
    return EVENT_BLOCK;
}

// -----------------------------------------------------------------------------

/**
 * Called by the input module
 */
void EventHandler::processAction(const int action, const unsigned int value, Input::InputType type, const int playerID)
{
    const bool pressedDown = value > Input::MAX_VALUE*2/3;
    
    if (!pressedDown && type == Input::IT_STICKMOTION) return;
    
    switch (action)
    {
        case PA_LEFT:
        {
            Widget* w = NULL;
            
            // TODO : unify player 0 and players > 0 focus navigation to eliminate this kind of branching
            if (playerID == 0)
            {
                IGUIElement *el = GUIEngine::getGUIEnv()->getFocus();
                if (el == NULL) break;
                            
                w = GUIEngine::getWidget( el->getID() );
            }
            else
            {
                w = GUIEngine::g_focus_for_player[playerID];
            }
            
            if (w == NULL) break;
                        
            Widget* widget_to_call = w;
            
            /* Find topmost parent. Stop looping if a widget event handler's is itself, to not fall
             in an infinite loop (this can happen e.g. in checkboxes, where they need to be
             notified of clicks onto themselves so they can toggle their state. )
             On the way, also notify everyone in the chain of the left press. */
            while (widget_to_call->m_event_handler != NULL && widget_to_call->m_event_handler != widget_to_call)
            {
                widget_to_call->leftPressed(playerID);
                widget_to_call = widget_to_call->m_event_handler;
            }
            
            
            if (widget_to_call->leftPressed(playerID) == EVENT_LET)
            {
                transmitEvent(w, w->m_properties[PROP_ID], playerID);
            }
        }
        break;
            
        case PA_RIGHT:
        {
            Widget* w = NULL;
            
            // TODO : unify player 0 and players > 0 focus navigation to eliminate this kind of branching
            if (playerID == 0)
            {
                IGUIElement *el = GUIEngine::getGUIEnv()->getFocus();
                if(el == NULL) break;
                w = GUIEngine::getWidget( el->getID() );
            }
            else
            {
                w = GUIEngine::g_focus_for_player[playerID];
            }
            
            if (w == NULL) break;
            
            Widget* widget_to_call = w;
            /* Find topmost parent. Stop looping if a widget event handler's is itself, to not fall
             in an infinite loop (this can happen e.g. in checkboxes, where they need to be
             notified of clicks onto themselves so they can toggle their state. )
             On the way, also notify everyone in the chain of the right press */
            while (widget_to_call->m_event_handler != NULL && widget_to_call->m_event_handler != widget_to_call)
            {
                widget_to_call->rightPressed(playerID);
                widget_to_call = widget_to_call->m_event_handler;
            }
            
            if (widget_to_call->rightPressed(playerID) == EVENT_LET)
            {
                transmitEvent(widget_to_call, w->m_properties[PROP_ID], playerID);
            }
        }
        break;
            
        case PA_ACCEL:
            navigateUp(playerID, type, pressedDown);
            break;
            
        case PA_BRAKE:
            navigateDown(playerID, type, pressedDown);
            break;
            
        case PA_RESCUE:
            if (pressedDown) GUIEngine::getStateManager()->escapePressed();
            break;
            
        case PA_FIRE:
            if (pressedDown)
            {
                IGUIElement* element = GUIEngine::getGUIEnv()->getFocus();
                Widget* w = GUIEngine::getWidget( element->getID() );
                if (w == NULL) break;
                // FIXME : consider returned value?
                onWidgetActivated( w, playerID );
            }
            
            /*
             // simulate a 'enter' key press
             irr::SEvent::SKeyInput evt;
             evt.PressedDown = pressedDown;
             evt.Key = KEY_SPACE;  // FIXME : what if keyboard bindings are not set to use this key?
             evt.Char = 666; // My magic code to know it's a fake event (FIXME : ugly, but irrlicht doesn't seem to offer better)
             irr::SEvent wrapper;
             wrapper.KeyInput = evt;
             
             wrapper.EventType = EET_KEY_INPUT_EVENT;
             GUIEngine::getDevice()->postEventFromUser(wrapper);
             */
            break;
        default:
            return;
    }
}

// -----------------------------------------------------------------------------

void EventHandler::navigateUp(const int playerID, Input::InputType type, const bool pressedDown)
{
    IGUIElement *el = NULL, *first=NULL, *closest=NULL;
    
    // TODO : unify player 0 and players > 0 focus navigation to eliminate this kind of branching
    if (playerID == 0)
    {
        el = GUIEngine::getGUIEnv()->getFocus();
    }
    else
    {
        Widget* widget = g_focus_for_player[playerID];
        if (widget != NULL)
        {
            el = widget->m_element;
        }
    }
    
    Widget* w = (el == NULL) ? NULL : GUIEngine::getWidget( el->getID() );
    
    // list widgets are a bit special, because up/down keys are also used
    // to navigate between various list items, not only to navigate between
    // components
    if (w != NULL && w->m_type == WTYPE_LIST)
    {
        IGUIListBox* list = dynamic_cast<IGUIListBox*>(w->m_element);
        assert(list != NULL);
        
        const bool stay_within_list = list->getSelected() > 0;
        
        if (type == Input::IT_STICKMOTION)
        {
            // simulate a key press
            irr::SEvent::SKeyInput evt;
            evt.PressedDown = pressedDown;
            evt.Key = KEY_UP;
            irr::SEvent wrapper;
            wrapper.KeyInput = evt;
            wrapper.EventType = EET_KEY_INPUT_EVENT;
            GUIEngine::getDevice()->postEventFromUser(wrapper);
        }
        
        if (stay_within_list) return;
        else list->setSelected(-1);
    }
    
    if (w != NULL && w->m_tab_up_root != -1) el = GUIEngine::getWidget( w->m_tab_up_root )->getIrrlichtElement();
    if (el == NULL)
    {
        std::cerr << "WARNING : m_tab_down_root is set to an ID for which I can't find the widget\n";
        return;
    }
    
    // find closest widget
    if (el != NULL && el->getTabGroup() != NULL &&
        el->getTabGroup()->getNextElement(el->getTabOrder(), true /* reverse */, false /* group */, first, closest))
    {
        Widget* w = GUIEngine::getWidget( closest->getID() );

        if (playerID == 0)
        {
            GUIEngine::getGUIEnv()->setFocus(closest);
        }
        else if (w != NULL)
        {
            w->setFocusForPlayer(playerID);
        }
        
        // when focusing a list by going up, select the last item of the list
        if (w != NULL && w->m_type == WTYPE_LIST)
        {
            IGUIListBox* list = dynamic_cast<IGUIListBox*>(w->m_element);
            assert(list != NULL);
            
            list->setSelected( list->getItemCount()-1 );
            return;
        }
        
    }
    else
    {        
        // select the last widget
        Widget* w = NULL;
        
        if (ModalDialog::isADialogActive())
        {
            // TODO : select last widget in modal dialogs
        }
        else
        {
            Screen* screen = GUIEngine::getCurrentScreen();
            if (screen == NULL) return;
            w = screen->getLastWidget();
        }
        
        if (w != NULL)
        {
            if (playerID == 0)
                GUIEngine::getGUIEnv()->setFocus( w->m_element );
            else
                w->setFocusForPlayer(playerID);
        }
    }
}

// -----------------------------------------------------------------------------

void EventHandler::navigateDown(const int playerID, Input::InputType type, const bool pressedDown)
{
    IGUIElement *el = NULL, *first = NULL, *closest = NULL;
    
    // TODO : unify player 0 and players > 0 focus navigation to eliminate this kind of branching
    if (playerID == 0)
    {
        el = GUIEngine::getGUIEnv()->getFocus();
    }
    else
    {
        Widget* widget = GUIEngine::g_focus_for_player[playerID];
        if (widget != NULL) el = widget->m_element;
    }
        
    Widget* w = (el == NULL) ? NULL : GUIEngine::getWidget( el->getID() );
    
    //std::cout << "!!! Player " << playerID << " navigating down of " << w->m_element->getID() << std::endl;
    
    // list widgets are a bit special, because up/down keys are also used
    // to navigate between various list items, not only to navigate between
    // components
    if (w != NULL && w->m_type == WTYPE_LIST)
    {
        IGUIListBox* list = dynamic_cast<IGUIListBox*>(w->m_element);
        assert(list != NULL);
        
        const bool stay_within_list = list->getSelected() < (int)list->getItemCount()-1;
        
        if (type == Input::IT_STICKMOTION)
        {
            // simulate a key press
            irr::SEvent::SKeyInput evt;
            evt.PressedDown = pressedDown;
            evt.Key = KEY_DOWN;
            irr::SEvent wrapper;
            wrapper.KeyInput = evt;
            wrapper.EventType = EET_KEY_INPUT_EVENT;
            GUIEngine::getDevice()->postEventFromUser(wrapper);
        }
        
        if (stay_within_list) return;
        else list->setSelected(-1);
    }
    
    if (w != NULL && w->m_tab_down_root != -1)
    {
        Widget* down = GUIEngine::getWidget( w->m_tab_down_root );
        assert(down != NULL);
        el = down->getIrrlichtElement();
    }
    if (el == NULL)
    {
        std::cerr << "WARNING : m_tab_down_root is set to an ID for which I can't find the widget\n";
        return;
    }
    
    if (el != NULL && el->getTabGroup() != NULL &&
       el->getTabGroup()->getNextElement(el->getTabOrder(), false, false, first, closest))
    {
       // std::cout << "!!! Player " << playerID << " navigating to " << closest->getID() << std::endl;

        if (playerID == 0)
        {
            GUIEngine::getGUIEnv()->setFocus(closest);
        }
        else
        {
            Widget* w = GUIEngine::getWidget( closest->getID() );
            w->setFocusForPlayer(playerID);
        }
    }
    else
    {
        //std::cout << "!!! Player " << playerID << " cannot navigating down, no next widget found;\n";

        // select the first widget
        Widget* w = NULL;
        
        if (ModalDialog::isADialogActive())
        {
            // TODO : select first widget in modal dialogs
        }
        else
        {
            Screen* screen = GUIEngine::getCurrentScreen();
            if (screen == NULL) return;
            w = screen->getFirstWidget();  
        }
        
        if(w != NULL)
        {
            if (playerID == 0)
                GUIEngine::getGUIEnv()->setFocus( w->m_element );
            else
                w->setFocusForPlayer(playerID);
        }
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
