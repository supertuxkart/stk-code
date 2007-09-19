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
#include "widget_set.hpp"
#include "menu_manager.hpp"
#include "translation.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif
ConfigControls::ConfigControls()
{
    m_menu_id = widgetSet -> vstack(0);
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

    widgetSet -> layout(m_menu_id, 0, 0);
}

//-----------------------------------------------------------------------------
ConfigControls::~ConfigControls()
{
    widgetSet -> delete_widget(m_menu_id) ;
}

//-----------------------------------------------------------------------------
void ConfigControls::update(float dt)
{
    widgetSet -> timer(m_menu_id, dt) ;
    widgetSet -> paint(m_menu_id) ;
}

//-----------------------------------------------------------------------------
void ConfigControls::select()
{
    switch ( widgetSet -> get_token (widgetSet -> click()) )
    {
    case 1: menu_manager->pushMenu(MENUID_CONFIG_P1); break;
    case 2: menu_manager->pushMenu(MENUID_CONFIG_P2); break;
    case 3: menu_manager->pushMenu(MENUID_CONFIG_P3); break;
    case 4: menu_manager->pushMenu(MENUID_CONFIG_P4); break;
    case 5: menu_manager->popMenu();                  break;
    default:                                          break;
    }
}



