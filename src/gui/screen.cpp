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


#include <iostream>
#include <sstream>
#include <assert.h>

#include "irrlicht.h"

#include "input/input.hpp"
#include "io/file_manager.hpp"
#include "gui/engine.hpp"
#include "gui/modaldialog.hpp"
#include "gui/screen.hpp"
#include "gui/state_manager.hpp"
#include "gui/widget.hpp"
#include "utils/translation.hpp"

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace GUIEngine;

Screen::Screen(const char* file)
{
    m_mouse_x = 0;
    m_mouse_y = 0;
    this->m_filename = file;
    m_loaded = false;
    loadFromFile();
    m_inited = false;
}

#if 0
#pragma mark -
#pragma mark Load/Init
#endif

// -----------------------------------------------------------------------------
void Screen::loadFromFile()
{
    IrrXMLReader* xml = irr::io::createIrrXMLReader( (file_manager->getGUIDir() + "/" + m_filename).c_str() );
    parseScreenFileDiv(xml, m_widgets);
    m_loaded = true;
    calculateLayout();
}
// -----------------------------------------------------------------------------
/* small shortcut so this method can be called without arguments */
void Screen::calculateLayout()
{
    // build layout
    calculateLayout( m_widgets );
}
// -----------------------------------------------------------------------------
/*
 * Recursive call that lays out children widget within parent (or screen if none)
 * Manages 'horizontal-row' and 'vertical-row' layouts, along with the proportions
 * of the remaining children, as well as absolute sizes and locations.
 */
void Screen::calculateLayout(ptr_vector<Widget>& widgets, Widget* parent)
{
    const unsigned short widgets_amount = widgets.size();
    
    // ----- read x/y/size parameters
    for(unsigned short n=0; n<widgets_amount; n++)
    {
        widgets[n].readCoords(parent);
    }//next widget        
    
    // ----- manage 'layout's if relevant
    do // i'm using 'while false' here just to be able to 'break' ...
    {
        if(parent == NULL) break;
        
        std::string layout_name = parent->m_properties[PROP_LAYOUT];
        if(layout_name.size() < 1) break;
        
        bool horizontal = false;
        
        if (!strcmp("horizontal-row", layout_name.c_str()))
            horizontal = true;
        else if(!strcmp("vertical-row", layout_name.c_str()))
            horizontal = false;
        else
        {
            std::cerr << "Unknown layout name : " << layout_name.c_str() << std::endl;
            break;
        }
        
        const int w = parent->w, h = parent->h;
        
        // find space left after placing all absolutely-sized widgets in a row
        // (the space left will be divided between remaining widgets later)
        int left_space = (horizontal ? w : h);
        unsigned short total_proportion = 0;
        for(int n=0; n<widgets_amount; n++)
        {
            // relatively-sized widget
            std::string prop = widgets[n].m_properties[ PROP_PROPORTION ];
            if(prop.size() != 0)
            {
                total_proportion += atoi( prop.c_str() );
                continue;
            }
            
            // absolutely-sized widgets
            left_space -= (horizontal ? widgets[n].w : widgets[n].h);
        } // next widget
        
        // lay widgets in row
        int x = parent->x, y = parent->y;
        for(int n=0; n<widgets_amount; n++)
        {
            std::string prop = widgets[n].m_properties[ PROP_PROPORTION ];
            if(prop.size() != 0)
            {
                // proportional size
                int proportion = 1;
                std::istringstream myStream(prop);
                if(!(myStream >> proportion))
                    std::cerr << "/!\\ Warning /!\\ : proportion  '" << prop.c_str() << "' is not a number in widget " << widgets[n].m_properties[PROP_ID].c_str() << std::endl;
                
                const float fraction = (float)proportion/(float)total_proportion;
                
                if(horizontal)
                {
   
                    widgets[n].x = x;
                    
                    if(widgets[n].m_properties[ PROP_Y ].size() > 0)
                        widgets[n].y = atoi(widgets[n].m_properties[ PROP_Y ].c_str());
                    else
                        widgets[n].y = y;
                    
                    widgets[n].w = (int)(left_space*fraction);
                    if(widgets[n].m_properties[PROP_MAX_WIDTH].size() > 0)
                    {
                        const int max_width = atoi( widgets[n].m_properties[PROP_MAX_WIDTH].c_str() );
                        if(widgets[n].w > max_width) widgets[n].w = max_width;
                    }
                    
                    x += widgets[n].w;
                }
                else
                {    
                    widgets[n].h = (int)(left_space*fraction);
                    
                    if(widgets[n].m_properties[PROP_MAX_HEIGHT].size() > 0)
                    {
                        const int max_height = atoi( widgets[n].m_properties[PROP_MAX_HEIGHT].c_str() );
                        if(widgets[n].h > max_height) widgets[n].h = max_height;
                    }
                    
                    std::string align = widgets[n].m_properties[ PROP_ALIGN ];
                    if(align.size() < 1)
                    {                        
                        if(widgets[n].m_properties[ PROP_X ].size() > 0)
                            widgets[n].x = atoi(widgets[n].m_properties[ PROP_X ].c_str());
                        else
                            widgets[n].x = x;
                    }
                    else if(align == "left") widgets[n].x = x;
                    else if(align == "center") widgets[n].x = x + w/2 - widgets[n].w/2;
                    else if(align == "right") widgets[n].x = x + w - widgets[n].w;
                    else std::cerr << "/!\\ Warning /!\\ : alignment  '" << align.c_str() << "' is unknown in widget " << widgets[n].m_properties[PROP_ID].c_str() << std::endl;
                    widgets[n].y = y;
                    
                    y += widgets[n].h;
                }
            }
            else
            {
                // absolute size
                
                if(horizontal)
                {
                    widgets[n].x = x;
                    
                    if(widgets[n].m_properties[ PROP_Y ].size() > 0)
                        widgets[n].y = atoi(widgets[n].m_properties[ PROP_Y ].c_str());
                    else
                        widgets[n].y = y;
                    
                    x += widgets[n].w;
                }
                else
                {
                    //widgets[n].h = abs_var;
                    
                    std::string align = widgets[n].m_properties[ PROP_ALIGN ];
                    
                    if(align.size() < 1)
                    {
                        if(widgets[n].m_properties[ PROP_X ].size() > 0)
                            widgets[n].x = atoi(widgets[n].m_properties[ PROP_X ].c_str());
                        else
                            widgets[n].x = x;
                    }
                    else if(align == "left") widgets[n].x = x;
                    else if(align == "center") widgets[n].x = x + w/2 - widgets[n].w/2;
                    else if(align == "right") widgets[n].x = x + w - widgets[n].w;
                    else std::cerr << "/!\\ Warning /!\\ : alignment  '" << align.c_str() << "' is unknown in widget " << widgets[n].m_properties[PROP_ID].c_str() << std::endl;
                    widgets[n].y = y;
                    
                    y += widgets[n].h;
                }
            } // end if property or absolute size
            
        } // next widget
        
    } while(false);
    
    // ----- also deal with containers' children
    for(int n=0; n<widgets_amount; n++)
    {
        if(widgets[n].m_type == WTYPE_DIV) calculateLayout(widgets[n].m_children, &widgets[n]);
    }
}

#if 0
#pragma mark -
#pragma mark Adding widgets
#endif


void Screen::addWidgets()
{
    if(!m_loaded) loadFromFile();
    
    addWidgetsRecursively( m_widgets );

    // select the first widget
    Widget* w = getFirstWidget();
    if(w != NULL) GUIEngine::getGUIEnv()->setFocus( w->m_element );
}
// -----------------------------------------------------------------------------
void Screen::addWidgetsRecursively(ptr_vector<Widget>& widgets, Widget* parent)
{
    const unsigned short widgets_amount = widgets.size();
    
    // ------- add widgets
    for(int n=0; n<widgets_amount; n++)
    {
        if(widgets[n].m_type == WTYPE_DIV)
        {
            addWidgetsRecursively(widgets[n].m_children, &widgets[n]);
        }
        else
        {
            // warn if widget has no dimensions (except for ribbons and icons, where it is normal since it adjusts to its contents)
            if((widgets[n].w < 1 || widgets[n].h < 1) && widgets[n].m_type != WTYPE_RIBBON  && widgets[n].m_type != WTYPE_ICON_BUTTON)
                std::cerr << "/!\\ Warning /!\\ : widget " << widgets[n].m_properties[PROP_ID].c_str() << " has no dimensions" << std::endl;
            
            if(widgets[n].x == -1 || widgets[n].y == -1)
                std::cerr << "/!\\ Warning /!\\ : widget " << widgets[n].m_properties[PROP_ID].c_str() << " has no position" << std::endl;
            
            widgets[n].add();
        }
        
    } // next widget
    
}

// -----------------------------------------------------------------------------
/**
 * Called when screen is removed. This means all irrlicht widgets this object has pointers
 * to are now gone. Set all references to NULL to avoid problems.
 */
void Screen::elementsWereDeleted(ptr_vector<Widget>* within_vector)
{
    if(within_vector == NULL) within_vector = &m_widgets;
    const unsigned short widgets_amount = within_vector->size();
    
    for(int n=0; n<widgets_amount; n++)
    {
        Widget& widget = (*within_vector)[n];
        
        widget.m_element = NULL;
        
        if(widget.m_children.size() > 0)
        {
            elementsWereDeleted( &(widget.m_children) );
        }
    }
}
// -----------------------------------------------------------------------------
void Screen::manualAddWidget(Widget* w)
{
    m_widgets.push_back(w);
}

#if 0
#pragma mark -
#pragma mark Getting widgets
#endif

Widget* Screen::getWidget(const char* name)
{
    return getWidget(name, NULL);
}
// -----------------------------------------------------------------------------
Widget* Screen::getWidget(const char* name, ptr_vector<Widget>* within_vector)
{
    if(within_vector == NULL)
    {
       within_vector = &m_widgets;
     
        // if a modal dialog is shown, search within it too
        if(ModalDialog::isADialogActive())
        {
            Widget* widgetWithinDialog = getWidget(name, &(ModalDialog::getCurrent()->m_children));
            if(widgetWithinDialog != NULL) return widgetWithinDialog;
        }
    }
    
    const unsigned short widgets_amount = within_vector->size();
    
    for(int n=0; n<widgets_amount; n++)
    {
        Widget& widget = (*within_vector)[n];
        
        if(widget.m_properties[PROP_ID] == name) return &widget;
        
        if(widget.m_type == WTYPE_DIV)
        {
            Widget* el = getWidget(name, &(widget.m_children));
            if(el != NULL) return el;
        }
    } // next
    
    return NULL;
}
// -----------------------------------------------------------------------------
Widget* Screen::getWidget(const int id, ptr_vector<Widget>* within_vector)
{
    if(within_vector == NULL)
    {
        within_vector = &m_widgets;
    
        // if a modal dialog is shown, search within it too
        if(ModalDialog::isADialogActive())
        {
            Widget* widgetWithinDialog = getWidget(id, &(ModalDialog::getCurrent()->m_children));
            if(widgetWithinDialog != NULL) return widgetWithinDialog;
        }
    }
    
    const unsigned short widgets_amount = within_vector->size();
    
    for(int n=0; n<widgets_amount; n++)
    {
        Widget& widget = (*within_vector)[n];
        
        if(widget.m_element != NULL && widget.m_element->getID() == id) return &widget;
        
        if(widget.m_children.size() > 0)
        {
            // std::cout << "widget = <" << widget.m_properties[PROP_ID].c_str() << ">  widget.m_children.size()=" << widget.m_children.size() << std::endl;
            Widget* el = getWidget(id, &(widget.m_children));
            if(el != NULL) return el;
        }
    } // next
    
    return NULL;
}
// -----------------------------------------------------------------------------
Widget* Screen::getFirstWidget(ptr_vector<Widget>* within_vector)
{
    if(within_vector == NULL) within_vector = &m_widgets;
    
    for(int i = 0; i < within_vector->size(); i++)
    {
        // if container, also checks children
        if(within_vector->get(i)->m_children.size() > 0 &&
           within_vector->get(i)->m_type != WTYPE_RIBBON &&
           within_vector->get(i)->m_type != WTYPE_SPINNER)
        {
            Widget* w = getFirstWidget(&within_vector->get(i)->m_children);
            if(w != NULL) return w;
        }
        
        if(within_vector->get(i)->m_element == NULL || within_vector->get(i)->m_element->getTabOrder() == -1) continue;
        
        return within_vector->get(i);
    }
    return NULL;
}
// -----------------------------------------------------------------------------
Widget* Screen::getLastWidget(ptr_vector<Widget>* within_vector)
{
    if(within_vector == NULL) within_vector = &m_widgets;
    
    for(int i = within_vector->size()-1; i >= 0; i--)
    {
        // if container, also checks children
        if(within_vector->get(i)->m_children.size() > 0 &&
           within_vector->get(i)->m_type != WTYPE_RIBBON &&
           within_vector->get(i)->m_type != WTYPE_SPINNER)
        {
            Widget* w = getLastWidget(&within_vector->get(i)->m_children);
            if(w != NULL) return w;
        }
        
        if(within_vector->get(i)->m_element == NULL || within_vector->get(i)->m_element->getTabOrder() == -1) continue;
        
        return within_vector->get(i);
    }
    return NULL;
}

#if 0
#pragma mark -
#pragma mark irrLicht events
#endif

bool Screen::onWidgetActivated(Widget* w)
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

void Screen::processAction(const int action, const unsigned int value, Input::InputType type)
{
    const bool pressedDown = value > Input::MAX_VALUE*2/3;
    
    if(!pressedDown && type == Input::IT_STICKMOTION) return;
    
    switch(action)
    {
        case PA_LEFT:
            /*
        if(type == Input::IT_STICKMOTION)
        {
            // simulate a key press
            irr::SEvent::SKeyInput evt;
            evt.PressedDown = pressedDown;
            evt.Key = KEY_LEFT;  // FIXME : what if keyboard bindings are not set to use this key?
            irr::SEvent wrapper;
            wrapper.KeyInput = evt;
            wrapper.EventType = EET_KEY_INPUT_EVENT;
            GUIEngine::getDevice()->postEventFromUser(wrapper);
        }
             */
        {
            IGUIElement *el = GUIEngine::getGUIEnv()->getFocus();
            if(el == NULL) break;
            
            Widget* w = getWidget( el->getID() );
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
            /*
        if(type == Input::IT_STICKMOTION)
        {
            // simulate a key press
            irr::SEvent::SKeyInput evt;
            evt.PressedDown = pressedDown;
            evt.Key = KEY_RIGHT;  // FIXME : what if keyboard bindings are not set to use this key?
            irr::SEvent wrapper;
            wrapper.KeyInput = evt;
            wrapper.EventType = EET_KEY_INPUT_EVENT;
            GUIEngine::getDevice()->postEventFromUser(wrapper);
        }*/
        {
            IGUIElement *el = GUIEngine::getGUIEnv()->getFocus();
            if(el == NULL) break;
            Widget* w = getWidget( el->getID() );
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
            
            Widget* w = (el == NULL) ? NULL : getWidget( el->getID() );
            
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
                Widget* w = getWidget( closest->getID() );
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
                Widget* w = getLastWidget();
                
                if(w != NULL) GUIEngine::getGUIEnv()->setFocus( w->m_element );
            }
        }
            break;
            
        case PA_BRAKE:
        {
            IGUIElement *el, *first = NULL, *closest = NULL;
            el = GUIEngine::getGUIEnv()->getFocus();
            
            Widget* w = (el == NULL) ? NULL : getWidget( el->getID() );
            
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
                Widget* w = getFirstWidget();                    
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
                    Widget* w = getWidget( element->getID() );
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

bool Screen::OnEvent(const SEvent& event)
{
    assert(transmitEvent != NULL);

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
                Widget* w = getWidget(id);
                if(w == NULL) break;
                
                return onWidgetActivated(w);
            }    
            case EGET_ELEMENT_HOVERED:
            {
                Widget* w = getWidget(id);
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
                Widget* el = getWidget(id);
                if(el == NULL) break;

                el->focused();
                
                break;
            }
            case EGET_EDITBOX_ENTER:
            {
                // currently, enter pressed in text ctrl events can only happen in dialogs.
                // FIXME : find a cleaner way to route the event to its proper location
                if(ModalDialog::isADialogActive()) ModalDialog::onEnterPressed();
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
