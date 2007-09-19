//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#include <SDL/SDL.h>

#include "player_controls.hpp"
#include "widget_set.hpp"
#include "user_config.hpp"
#include "menu_manager.hpp"
#include "translation.hpp"

#include <string>

const char *sKartAction2String[KC_LAST+1] = {_("Left"), _("Right"), _("Accelerate"),
                                             _("Brake"),  _("Wheelie"), _("Jump"),
                                             _("Rescue"), _("Fire"), _("Look back") };


PlayerControls::PlayerControls(int whichPlayer): m_player_index(whichPlayer),
        m_grab_input(false)
{
    // We need the unicode character here, so enable the translation
    SDL_EnableUNICODE(1);
    m_menu_id = widgetSet -> vstack(0);
    
    sprintf(m_heading, _("Choose your controls, %s"), 
            user_config->m_player[m_player_index].getName().c_str());
    widgetSet -> label(m_menu_id, m_heading, GUI_LRG, GUI_ALL, 0, 0);

    const int HA        = widgetSet->harray(m_menu_id);
    const int CHANGE_ID = widgetSet->varray(HA);
    const int LABEL_ID  = widgetSet->varray(HA);

    widgetSet->label(LABEL_ID, _("Player name"));
    m_name = user_config->m_player[m_player_index].getName();
    m_name_id = widgetSet->state(CHANGE_ID, m_name.c_str(), GUI_MED, -2);
    for(int i=0; i<=KC_LAST; i++)
    {
        //FIXME: this is temporal, just while the jumping is disabled.
        if(i==KC_JUMP) continue;
        addKeyLabel(CHANGE_ID, (KartActions)i,    i==0 );
        widgetSet->label(LABEL_ID, sKartAction2String[i]);
    }

    widgetSet->state(m_menu_id,_("Press <ESC> to go back"), GUI_SML, -1);
    widgetSet -> layout(m_menu_id, 0, 0);
}   // PlayerControls

//-----------------------------------------------------------------------------
PlayerControls::~PlayerControls()
{
    widgetSet -> delete_widget(m_menu_id) ;
    // The unicode translation is not generally needed, so disable it again.
    SDL_EnableUNICODE(0);
}   // ~PlayerControls

//-----------------------------------------------------------------------------
void PlayerControls::select()
{
    if (m_grab_input) return;

    m_grab_id = widgetSet -> click();
    if(m_grab_id == m_name_id) 
    {
        m_grab_input = true;
        return;
    }
    const int MENU_CHOICE = widgetSet -> get_token (m_grab_id);

    if(MENU_CHOICE == -1)
    {
        menu_manager->popMenu();
        return;
    }
    m_edit_action   = static_cast<KartActions>(MENU_CHOICE);
    m_grab_input = true;
    widgetSet->set_label(m_grab_id, _("Press key"));
}   // select

//-----------------------------------------------------------------------------
void PlayerControls::input(InputType type, int id0, int id1, int id2, int value)
{
    if (m_grab_input && value)
    {
        // Handle input of user name
        // -------------------------
        if(m_grab_id == m_name_id)
        {
            if(type==IT_KEYBOARD)
            {
                // Ignore shift, otherwise shift will disable input
                // (making it impossible to enter upper case characters)
                if(id0==SDLK_RSHIFT || id0==SDLK_LSHIFT) return;
                // Handle backspace
                if(id0==SDLK_BACKSPACE)
                {
                    if(m_name.size()>=1) m_name.erase(m_name.size()-1,1);
                }
                // All other control characters are ignored and will end 
                // entering the name
                else if(id0<32 || id0>255)
                {
                    m_grab_input = false;
                    user_config->m_player[m_player_index].setName(m_name);
                    BaseGUI::input(type, id0, id1, id2, value);
                    return;
                }
                else  // Add the character to the name
                {
                    // For this menu only unicode translation is enabled.
                    // So we use the unicode character here, since this will
                    // take care of upper/lower case etc.
                    m_name = m_name + (char)id1;
                }
                widgetSet->set_label(m_name_id, m_name.c_str());
            } 
            else
            {
                // Ignore all other events, e.g. when pressing the mouse
                // button twice on the input field etc.
            }
        }
        // Handle the definition of a key
        // ------------------------------
        else
        {
            m_grab_input = false;

            // Do not accept pressing ESC as input.
            if (type != IT_KEYBOARD || id0 != SDLK_ESCAPE)
            {
 	        // Since unicode translation is enabled, the value of id1 will
                // be the unicode value. Since unicode is usually not enabled
	        // in the race we have to set this value to zero (unicode 
                // translation is only enabled here to help entering the name),
	        // otherwise the keys will not be recognised in the race!!
  	        if(type==IT_KEYBOARD) id1=0;
                user_config->m_player[m_player_index].setInput(m_edit_action, type, 
                                                               id0, id1, id2);
	    }
            
            changeKeyLabel(m_grab_id, m_edit_action);
        }
    }
    else
        BaseGUI::input(type, id0, id1, id2, value);
}

//-----------------------------------------------------------------------------
void PlayerControls::addKeyLabel(int CHANGE_ID, KartActions control, bool start)
{

    setKeyInfoString(control);

    if (start)
        widgetSet -> start(CHANGE_ID, m_key_names[control].c_str(), GUI_MED, control);
    else
        widgetSet -> state(CHANGE_ID, m_key_names[control].c_str(), GUI_MED, control);
}   // addKeyLabel

//-----------------------------------------------------------------------------
void PlayerControls::changeKeyLabel(int m_grab_id, KartActions control)
{
    setKeyInfoString(control);
    widgetSet -> set_label(m_grab_id, m_key_names[control].c_str());
    //  widgetSet -> layout(m_menu_id, 0, 0);
}   // changeKeyLabel

//-----------------------------------------------------------------------------
void PlayerControls::setKeyInfoString(KartActions control)
{
    m_key_names[control] = user_config->getInputAsString(m_player_index, control);
}   // setKeyInfoString
