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


#include "gui/event_handler.hpp"
#include "gui/engine.hpp"
#include "gui/modaldialog.hpp"
#include "gui/screen.hpp"
#include "gui/state_manager.hpp"
#include "input/input_manager.hpp"

using GUIEngine::EventHandler;

EventHandler::EventHandler()
{
}

EventHandler::~EventHandler()
{
}

bool EventHandler::OnEvent (const SEvent &event)
{
    if(event.EventType == EET_GUI_EVENT ||
       (!StateManager::isGameState() && event.EventType != EET_KEY_INPUT_EVENT && event.EventType != EET_JOYSTICK_INPUT_EVENT)
       )
    {
        return onGUIEvent(event);
    }
    else
    {
        // FIXME : it's a bit unclean that all input events go trough the gui module
        return input_manager->input(event);
    }
}

bool EventHandler::onGUIEvent(const SEvent& event)
{    
    if(event.EventType == EET_GUI_EVENT)
    {
        const s32 id = event.GUIEvent.Caller->getID();
        
        switch(event.GUIEvent.EventType)
        {
            case EGET_BUTTON_CLICKED:
            case EGET_SCROLL_BAR_CHANGED:
            case EGET_CHECKBOX_CHANGED:
            case EGET_LISTBOX_SELECTED_AGAIN:
            {
                Widget* w = GUIEngine::getCurrentScreen()->getWidget(id);
                if(w == NULL) break;
                
                return onWidgetActivated(w);
            }    
            case EGET_ELEMENT_HOVERED:
            {
                Widget* w = GUIEngine::getCurrentScreen()->getWidget(id);
                if(w == NULL) break;
                
                // select ribbons on hover
                if(w->m_event_handler != NULL && w->m_event_handler->m_type == WTYPE_RIBBON)
                {
                    RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(w->m_event_handler);
                    if(ribbon == NULL) break;
                    if(ribbon->mouseHovered(w))
                        transmitEvent(ribbon, ribbon->m_properties[PROP_ID]);
                    if(ribbon->m_event_handler != NULL) ribbon->m_event_handler->mouseHovered(w);
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
                Widget* el = GUIEngine::getCurrentScreen()->getWidget(id);
                if(el == NULL) break;
                
                el->focused();
                
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
    return false;        
}

bool EventHandler::onWidgetActivated(Widget* w)
{
    if(ModalDialog::isADialogActive() && w->m_event_handler == NULL)
    {
        ModalDialog::getCurrent()->processEvent(w->m_properties[PROP_ID]);
        return false;
    }
    
    Widget* parent = w->m_event_handler;
    if(w->m_event_handler != NULL)
    {
        /* Find topmost parent. Stop looping if a widget event handler's is itself, to not fall
         in an infinite loop (this can happen e.g. in checkboxes, where they need to be
         notified of clicks onto themselves so they can toggle their state. ) */
        while(parent->m_event_handler != NULL && parent->m_event_handler != parent)
            parent = parent->m_event_handler;
        
        /* notify the found event event handler, and also notify the main callback if the
         parent event handler says so */
        if(parent->transmitEvent(w, w->m_properties[PROP_ID]))
            transmitEvent(parent, parent->m_properties[PROP_ID]);
    }
    else transmitEvent(w, w->m_properties[PROP_ID]);
    
    return true;
}

/**
 * Called by the input module
 */
void EventHandler::processAction(const int action, const unsigned int value, Input::InputType type)
{
    const bool pressedDown = value > Input::MAX_VALUE*2/3;
    
    if(!pressedDown && type == Input::IT_STICKMOTION) return;
    
    switch(action)
    {
        case PA_LEFT:
        {
            IGUIElement *el = GUIEngine::getGUIEnv()->getFocus();
            if(el == NULL) break;
            
            Widget* w = GUIEngine::getCurrentScreen()->getWidget( el->getID() );
            if(w == NULL) break;
            
            Widget* widget_to_call = w;
            
            /* Find topmost parent. Stop looping if a widget event handler's is itself, to not fall
             in an infinite loop (this can happen e.g. in checkboxes, where they need to be
             notified of clicks onto themselves so they can toggle their state. ) */
            while(widget_to_call->m_event_handler != NULL && widget_to_call->m_event_handler != widget_to_call)
                widget_to_call = widget_to_call->m_event_handler;
            
            if(widget_to_call->leftPressed())
                transmitEvent(w, w->m_properties[PROP_ID]);
        }
            break;
            
        case PA_RIGHT:
        {
            IGUIElement *el = GUIEngine::getGUIEnv()->getFocus();
            if(el == NULL) break;
            Widget* w = GUIEngine::getCurrentScreen()->getWidget( el->getID() );
            if(w == NULL) break;
            
            Widget* widget_to_call = w;
            /* Find topmost parent. Stop looping if a widget event handler's is itself, to not fall
             in an infinite loop (this can happen e.g. in checkboxes, where they need to be
             notified of clicks onto themselves so they can toggle their state. ) */
            while(widget_to_call->m_event_handler != NULL && widget_to_call->m_event_handler != widget_to_call)
                widget_to_call = widget_to_call->m_event_handler;
            
            if(widget_to_call->rightPressed())
                transmitEvent(widget_to_call, w->m_properties[PROP_ID]);
        }
            
            break;
            
        case PA_ACCEL:
        {
            IGUIElement *el, *first=NULL, *closest=NULL;
            el = GUIEngine::getGUIEnv()->getFocus();
            
            Widget* w = (el == NULL) ? NULL : GUIEngine::getCurrentScreen()->getWidget( el->getID() );
            
            // list widgets are a bit special, because up/down keys are also used
            // to navigate between various list items, not only to navigate between
            // components
            if(w != NULL && w->m_type == WTYPE_LIST)
            {
                IGUIListBox* list = dynamic_cast<IGUIListBox*>(w->m_element);
                assert(list != NULL);
                
                const bool stay_within_list = list->getSelected() > 0;
                
                if(type == Input::IT_STICKMOTION)
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
                
                if(stay_within_list) break;
                else list->setSelected(-1);
            }
            
            // find closest widget
            if(el != NULL && el->getTabGroup() != NULL &&
               el->getTabGroup()->getNextElement(el->getTabOrder(), true /* reverse */, false /* group */, first, closest))
            {
                GUIEngine::getGUIEnv()->setFocus(closest);
                
                // when focusing a list by going up, select the last item of the list
                Widget* w = GUIEngine::getCurrentScreen()->getWidget( closest->getID() );
                if(w != NULL && w->m_type == WTYPE_LIST)
                {
                    IGUIListBox* list = dynamic_cast<IGUIListBox*>(w->m_element);
                    assert(list != NULL);
                    
                    list->setSelected( list->getItemCount()-1 );
                    return;
                }
                
                
            }
            else
            {
                std::cout << "Could not find any!\n";
                
                // select the first widget
                Widget* w = GUIEngine::getCurrentScreen()->getLastWidget();
                
                if(w != NULL) GUIEngine::getGUIEnv()->setFocus( w->m_element );
            }
        }
            break;
            
        case PA_BRAKE:
        {
            IGUIElement *el, *first = NULL, *closest = NULL;
            el = GUIEngine::getGUIEnv()->getFocus();
            
            Widget* w = (el == NULL) ? NULL : GUIEngine::getCurrentScreen()->getWidget( el->getID() );
            
            // list widgets are a bit special, because up/down keys are also used
            // to navigate between various list items, not only to navigate between
            // components
            if(w != NULL && w->m_type == WTYPE_LIST)
            {
                IGUIListBox* list = dynamic_cast<IGUIListBox*>(w->m_element);
                assert(list != NULL);
                
                const bool stay_within_list = list->getSelected() < (int)list->getItemCount()-1;
                
                if(type == Input::IT_STICKMOTION)
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
                
                if(stay_within_list) break;
                else list->setSelected(-1);
            }
            
            if(el != NULL && el->getTabGroup() != NULL &&
               el->getTabGroup()->getNextElement(el->getTabOrder(), false, false, first, closest))
            {
                GUIEngine::getGUIEnv()->setFocus(closest);
            }
            else
            {
                // select the first widget
                Widget* w = GUIEngine::getCurrentScreen()->getFirstWidget();                    
                if(w != NULL) GUIEngine::getGUIEnv()->setFocus( w->m_element );
            }
        }
            
            break;
            
        case PA_RESCUE:
            if(pressedDown)
                StateManager::escapePressed();
            break;
            
        case PA_FIRE:
            if(type == Input::IT_STICKBUTTON)
            {
                if (pressedDown)
                {
                    IGUIElement* element = GUIEngine::getGUIEnv()->getFocus();
                    Widget* w = GUIEngine::getCurrentScreen()->getWidget( element->getID() );
                    if(w == NULL) break;
                    onWidgetActivated( w );
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
            }
            break;
        default:
            return;
    }
}


EventHandler* event_handler_singleton = NULL;

EventHandler* EventHandler::get()
{
    if (event_handler_singleton == NULL)
    {
        event_handler_singleton = new EventHandler();
    }
    return event_handler_singleton;
}
