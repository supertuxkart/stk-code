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

#include "options.hpp"
#include "widget_manager.hpp"
#include "menu_manager.hpp"
#include "translation.hpp"

enum WidgetTokens {
    WTOK_TITLE,
    WTOK_CONTROLS,
    WTOK_DISPLAY,
    WTOK_SOUND,
    WTOK_BACK
};

Options::Options()
{
/*    m_menu_id = widgetSet -> varray(0);

    widgetSet -> space(m_menu_id);
    widgetSet -> space(m_menu_id);
    widgetSet -> label(m_menu_id, _("Options"),   GUI_LRG, GUI_ALL, 0, 0);
    widgetSet -> start(m_menu_id, _("Player Config"),  GUI_MED, WTOK_CONTROLS);

#ifndef WIN32
    // Don't display the fullscreen menu when called from within the race.
    // (Windows only)
    // The fullscreen mode will reload all textures, reload the models,
    // ... basically creating a big mess!!  (and all of this only thanks
    // to windows, who discards all textures, ...)
    widgetSet -> state(m_menu_id, _("Display"),   GUI_MED, WTOK_DISPLAY);
#endif

    widgetSet -> state(m_menu_id, _("Sound"),     GUI_MED, WTOK_SOUND);
    widgetSet -> space(m_menu_id);
    widgetSet -> state(m_menu_id, _("Press <ESC> to go back"), GUI_SML, WTOK_BACK);

    widgetSet -> layout(m_menu_id, 0, 0);*/
    widget_manager->add_wgt(WTOK_TITLE, 35, 7);
    widget_manager->show_wgt_rect( WTOK_TITLE );
    widget_manager->set_wgt_text( WTOK_TITLE, _("Options") );
    widget_manager->set_wgt_text_size( WTOK_TITLE, WGT_FNT_LRG );
    widget_manager->show_wgt_text( WTOK_TITLE );
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_CONTROLS, 35, 7);
    widget_manager->show_wgt_rect( WTOK_CONTROLS );
    widget_manager->set_wgt_text( WTOK_CONTROLS, _("Player Config") );
    widget_manager->set_wgt_text_size( WTOK_CONTROLS, WGT_FNT_MED );
    widget_manager->show_wgt_text( WTOK_CONTROLS );
    widget_manager->activate_wgt( WTOK_CONTROLS);
    widget_manager->break_line();

    // Don't display the fullscreen menu when called from within the race.
    // The fullscreen mode will reload all textures, reload the models,
    // ... basically creating a big mess!!  (and all of this only thanks
    // to windows, who discards all textures, ...)
    if(!menu_manager->isSomewhereOnStack(MENUID_RACE))
    {
        widget_manager->add_wgt(WTOK_DISPLAY, 35, 7);
        widget_manager->show_wgt_rect( WTOK_DISPLAY );
        widget_manager->set_wgt_text( WTOK_DISPLAY, _("Display") );
        widget_manager->set_wgt_text_size( WTOK_DISPLAY, WGT_FNT_MED );
        widget_manager->show_wgt_text( WTOK_DISPLAY );
        widget_manager->activate_wgt( WTOK_DISPLAY );
        widget_manager->break_line();
    }

    widget_manager->add_wgt(WTOK_SOUND, 35, 7);
    widget_manager->show_wgt_rect( WTOK_SOUND );
    widget_manager->set_wgt_text( WTOK_SOUND, _("Sound") );
    widget_manager->set_wgt_text_size( WTOK_SOUND, WGT_FNT_MED );
    widget_manager->show_wgt_text( WTOK_SOUND );
    widget_manager->activate_wgt( WTOK_SOUND );
    widget_manager->break_line();

    widget_manager->add_wgt(WidgetManager::WGT_NONE, 35, 7);
    widget_manager->break_line();

    widget_manager->add_wgt(WTOK_BACK, 35, 7);
    widget_manager->show_wgt_rect( WTOK_BACK );
    widget_manager->set_wgt_text( WTOK_BACK, _("Press <ESC> to go back") );
    widget_manager->set_wgt_text_size( WTOK_BACK, WGT_FNT_SML );
    widget_manager->show_wgt_text( WTOK_BACK );
    widget_manager->activate_wgt( WTOK_BACK );

    widget_manager->layout(WGT_AREA_ALL);
}

// -----------------------------------------------------------------------------
Options::~Options()
{
    widget_manager->reset() ;
}

// -----------------------------------------------------------------------------
void Options::select()
{
    switch ( widget_manager->get_selected_wgt() )
//    switch ( widgetSet -> get_token (widgetSet -> click()) )
    {
    case WTOK_CONTROLS:
        menu_manager->pushMenu(MENUID_CONFIG_CONTROLS);
        break;
    case WTOK_DISPLAY:
        menu_manager->pushMenu(MENUID_CONFIG_DISPLAY);
        break;
    case WTOK_SOUND:
        menu_manager->pushMenu(MENUID_CONFIG_SOUND);
        break;
    case WTOK_BACK:
        menu_manager->popMenu();
        break;
    default:
        break;
    }  // switch
}
