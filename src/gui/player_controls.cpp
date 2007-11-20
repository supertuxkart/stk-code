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
#include "widget_manager.hpp"
#include "user_config.hpp"
#include "menu_manager.hpp"
#include "translation.hpp"
#include "sdldrv.hpp"

#include <string>

enum WidgetTokens
{
    WTOK_TITLE,
    WTOK_PLYR_NAME0,
    WTOK_PLYR_NAME1,

    WTOK_LEFT,
    WTOK_RIGHT,
    WTOK_ACCEL,
    WTOK_BRAKE,
    WTOK_WHEELIE,
    WTOK_JUMP,
    WTOK_RESCUE,
    WTOK_FIRE,
    WTOK_LOOK_BACK,

    WTOK_KEY0,
    WTOK_KEY1,
    WTOK_KEY2,
    WTOK_KEY3,
    WTOK_KEY4,
    WTOK_KEY5,
    WTOK_KEY6,
    WTOK_KEY7,
    WTOK_KEY8,

    WTOK_QUIT
};

const char *sKartAction2String[KC_LAST+1] = {_("Left"), _("Right"), _("Accelerate"),
                                             _("Brake"),  _("Wheelie"), _("Jump"),
                                             _("Rescue"), _("Fire"), _("Look back") };


PlayerControls::PlayerControls(int whichPlayer):
    m_grab_id(WidgetManager::WGT_NONE), m_player_index(whichPlayer),
    m_grab_input(false)
{
    // We need the unicode character here, so enable the translation
    SDL_EnableUNICODE(1);

    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    widget_manager->set_initial_rect_state(SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK);
    widget_manager->set_initial_text_state(SHOW_TEXT, "", WGT_FNT_MED, Font::ALIGN_CENTER, Font::ALIGN_CENTER );

    widget_manager->add_wgt( WTOK_TITLE, 60, 7);
    sprintf(m_heading, _("Choose your controls, %s"),
            user_config->m_player[m_player_index].getName().c_str());
    widget_manager->set_wgt_text( WTOK_TITLE, m_heading);
    widget_manager->break_line();

    widget_manager->add_wgt( WTOK_PLYR_NAME0, 30, 7);
    widget_manager->set_wgt_text( WTOK_PLYR_NAME0, _("Player name"));

    widget_manager->add_wgt( WTOK_PLYR_NAME1, 30, 7);
    m_name = user_config->m_player[m_player_index].getName();
    widget_manager->set_wgt_text( WTOK_PLYR_NAME1, m_name);
    widget_manager->activate_wgt( WTOK_PLYR_NAME1);
    widget_manager->break_line();

    KartActions control;
    for(int i=0; i<=KC_LAST; i++)
    {
        widget_manager->add_wgt( WTOK_KEY0 + i, 30, 7);
        widget_manager->set_wgt_text( WTOK_KEY0 + i, sKartAction2String[i]);

        control = (KartActions)i;
        m_key_names[control] = user_config->getInputAsString(m_player_index, control);
        widget_manager->add_wgt( WTOK_LEFT + i, 30, 7);
        widget_manager->set_wgt_text( WTOK_LEFT + i, m_key_names[control].c_str());
        widget_manager->activate_wgt( WTOK_LEFT + i);

        widget_manager->break_line();
    }

    widget_manager->add_wgt( WTOK_QUIT, 60, 7);
    widget_manager->set_wgt_text( WTOK_QUIT, _("Press <ESC> to go back"));
    widget_manager->set_wgt_text_size( WTOK_QUIT, WGT_FNT_SML);
    widget_manager->activate_wgt( WTOK_QUIT);

    widget_manager->layout(WGT_AREA_ALL);
}   // PlayerControls

//-----------------------------------------------------------------------------
PlayerControls::~PlayerControls()
{
    widget_manager->delete_wgts();
    // The unicode translation is not generally needed, so disable it again.
    SDL_EnableUNICODE(0);
}   // ~PlayerControls

//-----------------------------------------------------------------------------
void PlayerControls::select()
{
    if (m_grab_input) return;

    m_grab_id = widget_manager->get_selected_wgt();
    if(m_grab_id == WTOK_PLYR_NAME1)
    {
        m_grab_input = true;
        return;
    }

    const int MENU_CHOICE = widget_manager->get_selected_wgt() - WTOK_LEFT;

    if(MENU_CHOICE == WTOK_QUIT)
    {
        menu_manager->popMenu();
        return;
    }
    m_edit_action   = static_cast<KartActions>(MENU_CHOICE);
    m_grab_input = true;
    drv_hidePointer();

    widget_manager->set_wgt_text(m_grab_id, _("Press key"));
}   // select

//-----------------------------------------------------------------------------
void PlayerControls::input(InputType type, int id0, int id1, int id2, int value)
{
    if (m_grab_input && value)
    {
        // Handle input of user name
        // -------------------------
        if(m_grab_id == WTOK_PLYR_NAME1)
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
                    m_grab_id = WidgetManager::WGT_NONE;
                    user_config->m_player[m_player_index].setName(m_name);
//                    BaseGUI::input(type, id0, id1, id2, value);
                    return;
                }
                else  // Add the character to the name
                {
                    // For this menu only unicode translation is enabled.
                    // So we use the unicode character here, since this will
                    // take care of upper/lower case etc.
                    m_name = m_name + (char)id1;
                }
                widget_manager->set_wgt_text(WTOK_PLYR_NAME1, m_name.c_str());
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
            drv_showPointer();
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

            widget_manager->set_wgt_text(m_grab_id, m_key_names[m_edit_action].c_str());
        }
    }
    else
        BaseGUI::input(type, id0, id1, id2, value);
}

//-----------------------------------------------------------------------------
/*void PlayerControls::addKeyLabel(int CHANGE_ID, KartActions control, bool start)
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
}   // setKeyInfoString
*/
