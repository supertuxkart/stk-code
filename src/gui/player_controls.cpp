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

char *sKartAction2String[KC_FIRE+1] = {_("Left"), _("Right"), _("Accelerate"),
                                       _("Brake"),  _("Wheelie"), _("Jump"),
                                       _("Rescue"), _("Fire")                 };


PlayerControls::PlayerControls(int whichPlayer): m_player_index(whichPlayer),
        m_grab_input(false)
{
    m_menu_id = widgetSet -> vstack(0);
    sprintf(m_heading, _("Choose your controls, %s"), user_config->m_player[m_player_index].getName());
    widgetSet -> label(m_menu_id, m_heading, GUI_LRG, GUI_ALL, 0, 0);

    const int HA        = widgetSet->harray(m_menu_id);
    const int CHANGE_ID = widgetSet->varray(HA);
    const int LABEL_ID  = widgetSet->varray(HA);

    for(int i=KC_LEFT; i<=KC_FIRE; i++)
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
}   // ~PlayerControls

//-----------------------------------------------------------------------------
void PlayerControls::select()
{
    if (m_grab_input) return;

    m_grab_id        = widgetSet -> click();
    const int MENU_CHOICE = widgetSet -> token (m_grab_id);
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
        m_grab_input = false;

        // Do not accept pressing ESC as input.
        if (type != IT_KEYBOARD || id0 != SDLK_ESCAPE)
            user_config->m_player[m_player_index].setInput(m_edit_action, type, id0, id1, id2);

        changeKeyLabel(m_grab_id, m_edit_action);
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
