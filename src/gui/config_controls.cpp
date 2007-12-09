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

#include "config_controls.hpp"
#include "widget_manager.hpp"
#include "menu_manager.hpp"
#include "translation.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

enum WidgetTokens {
    WTOK_TITLE,

    WTOK_PLYR1,
    WTOK_PLYR2,
    WTOK_PLYR3,
    WTOK_PLYR4,

    WTOK_SPACE,

    WTOK_QUIT
};

ConfigControls::ConfigControls()
{
    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    widget_manager->set_initial_rect_state(SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK);
    widget_manager->set_initial_text_state(SHOW_TEXT, "", WGT_FNT_MED );

    widget_manager->insert_column();
    widget_manager->add_wgt( WTOK_TITLE, 60, 7 );
    widget_manager->set_wgt_text( WTOK_TITLE, _("Edit controls for which player?"));

    widget_manager->set_initial_activation_state(true);
    widget_manager->add_wgt( WTOK_PLYR1 , 60, 7 );
    widget_manager->set_wgt_text( WTOK_PLYR1, _("Player 1"));

    widget_manager->add_wgt( WTOK_PLYR2 , 60, 7 );
    widget_manager->set_wgt_text( WTOK_PLYR2, _("Player 2"));

    widget_manager->add_wgt( WTOK_PLYR3 , 60, 7 );
    widget_manager->set_wgt_text( WTOK_PLYR3, _("Player 3"));

    widget_manager->add_wgt( WTOK_PLYR4 , 60, 7 );
    widget_manager->set_wgt_text( WTOK_PLYR4, _("Player 4"));

    widget_manager->add_wgt( WTOK_SPACE, 60, 5);
    widget_manager->deactivate_wgt( WTOK_SPACE );
    widget_manager->hide_wgt_rect( WTOK_SPACE );
    widget_manager->hide_wgt_text( WTOK_SPACE );

    widget_manager->add_wgt( WTOK_QUIT , 60, 7 );
    widget_manager->set_wgt_text( WTOK_QUIT, _("Press <ESC> to go back"));
    widget_manager->set_wgt_text_size( WTOK_QUIT, WGT_FNT_SML );

    widget_manager->layout( WGT_AREA_ALL );
/*
 * m_menu_id = widgetSet -> vstack(0);
    widgetSet -> label(m_menu_id, _("Edit controls for which player?"), GUI_LRG);

    const int VA = widgetSet -> varray(m_menu_id);

    static char playerN[4][MAX_MESSAGE_LENGTH];
    for(int i=1; i<=4; i++)
    {
        snprintf(playerN[i-1], MAX_MESSAGE_LENGTH,
                 _("Player %d"), i);
        if (i == 1)
          widgetSet -> start(VA, playerN[i-1],  GUI_MED, i);
        else
          widgetSet -> state(VA, playerN[i-1],  GUI_MED, i);
    }

    widgetSet -> space(VA);
    widgetSet -> state(VA, _("Press <ESC> to go back"), GUI_SML, 5);

    widgetSet -> layout(m_menu_id, 0, 0);*/
}

//-----------------------------------------------------------------------------
ConfigControls::~ConfigControls()
{
    widget_manager->reset();
//    widgetSet -> delete_widget(m_menu_id) ;
}

//-----------------------------------------------------------------------------
/*void ConfigControls::update(float dt)
{
    widgetSet -> timer(m_menu_id, dt) ;
    widgetSet -> paint(m_menu_id) ;
}
*/
//-----------------------------------------------------------------------------
void ConfigControls::select()
{
    switch ( widget_manager->get_selected_wgt() )
    {
    case WTOK_PLYR1: menu_manager->pushMenu(MENUID_CONFIG_P1); break;
    case WTOK_PLYR2: menu_manager->pushMenu(MENUID_CONFIG_P2); break;
    case WTOK_PLYR3: menu_manager->pushMenu(MENUID_CONFIG_P3); break;
    case WTOK_PLYR4: menu_manager->pushMenu(MENUID_CONFIG_P4); break;
    case WTOK_QUIT: menu_manager->popMenu();                   break;
    }
}



