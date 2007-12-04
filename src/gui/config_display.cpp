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

#include "config_display.hpp"
#include "widget_manager.hpp"
#include "user_config.hpp"
#include "menu_manager.hpp"
#include "sdldrv.hpp"
#include "translation.hpp"

enum WidgetTokens {
    WTOK_TITLE,

    WTOK_FULLSCREEN,

    WTOK_EMPTY,

    WTOK_QUIT
};

ConfigDisplay::ConfigDisplay()
{
    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    widget_manager->set_initial_rect_state(SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK);
    widget_manager->set_initial_text_state(SHOW_TEXT, "", WGT_FNT_MED, Font::ALIGN_CENTER, Font::ALIGN_CENTER );

    widget_manager->insert_column();
    widget_manager->add_wgt( WTOK_TITLE, 40, 7);
    widget_manager->set_wgt_text( WTOK_TITLE, _("Display Settings"));

    widget_manager->set_initial_activation_state(true);
    widget_manager->add_wgt( WTOK_FULLSCREEN, 40, 7);
    if(user_config->m_fullscreen)
    {
        widget_manager->set_wgt_text( WTOK_FULLSCREEN, _("Window mode"));
    }
    else
    {
        widget_manager->set_wgt_text( WTOK_FULLSCREEN, _("Fullscreen mode"));
    }

    widget_manager->add_wgt( WTOK_EMPTY, 40, 2);
    widget_manager->deactivate_wgt( WTOK_EMPTY );
    widget_manager->hide_wgt_rect( WTOK_EMPTY );
    widget_manager->hide_wgt_text( WTOK_EMPTY );

    widget_manager->add_wgt( WTOK_QUIT, 40, 7);
    widget_manager->set_wgt_text( WTOK_QUIT, _("Press <ESC> to go back"));
    widget_manager->set_wgt_text_size( WTOK_QUIT, WGT_FNT_SML );

    widget_manager->layout( WGT_AREA_ALL );
}

//-----------------------------------------------------------------------------
ConfigDisplay::~ConfigDisplay()
{
//    widgetSet -> delete_widget(m_menu_id) ;
    widget_manager->reset();
}

//-----------------------------------------------------------------------------
/*void ConfigDisplay::update(float dt)
{
    widgetSet -> timer(m_menu_id, dt) ;
    widgetSet -> paint(m_menu_id) ;
}*/

//-----------------------------------------------------------------------------
void ConfigDisplay::select()
{
    switch ( widget_manager->get_selected_wgt())
    {
    case WTOK_FULLSCREEN:
        drv_toggleFullscreen();
//        widgetSet -> delete_widget(m_menu_id) ;
        // Since changing the video mode in drv_toggleFullscreen deletes all
        // display lists, textures etc., we have to load the menu again.
        // drv_toggleFullscreen takes care of general material, general
        // widgetSet, etc.
//        CreateMenu();
        if(user_config->m_fullscreen)
            widget_manager->set_wgt_text( WTOK_FULLSCREEN, _("Window mode"));
        else
            widget_manager->set_wgt_text( WTOK_FULLSCREEN, _("Fullscreen mode"));
        break;
    case WTOK_QUIT:
        menu_manager->popMenu();
        break;
    default: break;
    }
}



