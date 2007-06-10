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
#include "widget_set.hpp"
#include "menu_manager.hpp"
#include "translation.hpp"

enum WidgetTokens {
    WTOK_CONTROLS,
    WTOK_DISPLAY,
    WTOK_SOUND,
    WTOK_BACK
};

Options::Options()
{
    m_menu_id = widgetSet -> varray(0);

    widgetSet -> space(m_menu_id);
    widgetSet -> space(m_menu_id);
    widgetSet -> label(m_menu_id, _("Options"),   GUI_LRG, GUI_ALL, 0, 0);
    widgetSet -> start(m_menu_id, _("Player Config"),  GUI_MED, WTOK_CONTROLS);
    // Don't display the fullscreen menu when called from within the race.
    // The fullscreen mode will reload all textures, reload the models,
    // ... basically creating a big mess!!  (and all of this only thanks
    // to windows, who discards all textures, ...)
    if(!menu_manager->isSomewhereOnStack(MENUID_RACE))
    {
        widgetSet -> state(m_menu_id, _("Display"),   GUI_MED, WTOK_DISPLAY);
    }
    widgetSet -> state(m_menu_id, _("Sound"),     GUI_MED, WTOK_SOUND);
    widgetSet -> space(m_menu_id);
    widgetSet -> state(m_menu_id, _("Press <ESC> to go back"), GUI_SML, WTOK_BACK);

    widgetSet -> layout(m_menu_id, 0, 0);
}

// -----------------------------------------------------------------------------
Options::~Options()
{
    widgetSet -> delete_widget(m_menu_id) ;
}

// -----------------------------------------------------------------------------
void Options::update(float dt)
{
    widgetSet -> timer(m_menu_id, dt) ;
    widgetSet -> paint(m_menu_id) ;
}

// -----------------------------------------------------------------------------
void Options::select()
{
    switch ( widgetSet -> get_token (widgetSet -> click()) )
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
