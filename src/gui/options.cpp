//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#include "gui/options.hpp"

#include "user_config.hpp"
#include "gui/menu_manager.hpp"
#include "gui/widget_manager.hpp"
#include "utils/translation.hpp"

enum WidgetTokens
{
    WTOK_TITLE,

    WTOK_CONTROLS,
    WTOK_DISPLAY,
    WTOK_SOUND,

    WTOK_QUIT
};

Options::Options()
{
    widget_manager->switchOrder();
    widget_manager->addTitleWgt( WTOK_TITLE, 45, 7, _("Options") );
    widget_manager->hideWgtRect(WTOK_TITLE);
    widget_manager->addTextButtonWgt( WTOK_CONTROLS, 45, 7, _("Player Config") );

    // Don't display the fullscreen menu when called from within the race.
    // The fullscreen mode will reload all textures, reload the models,
    // ... basically creating a big mess!!  (and all of this only thanks
    // to windows, who discards all textures, ...)
    if(!menu_manager->isSomewhereOnStack( MENUID_RACE ))
    {
        widget_manager->addTextButtonWgt( WTOK_DISPLAY, 45, 7, _("Display") );
    }

    widget_manager->addTextButtonWgt( WTOK_SOUND, 45, 7, _("Sound") );

    widget_manager->addEmptyWgt( WidgetManager::WGT_NONE, 1, 7 );

    widget_manager->addTextButtonWgt( WTOK_QUIT, 45, 7,
        _("Press <ESC> to go back") );
    widget_manager->setWgtTextSize( WTOK_QUIT, WGT_FNT_SML );

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
    switch ( widget_manager->getSelectedWgt() )
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
    case WTOK_QUIT:
        //FIXME: this shouldn't be in this screen
		// Make config changes permanent.
		user_config->saveConfig();

        menu_manager->popMenu();
        break;
    default:
        break;
    }  // switch
}

void Options::handle(GameAction action, int value)
{
	// Save config on leave.
	if (!value && action == GA_LEAVE)
		user_config->saveConfig();

	BaseGUI::handle(action, value);
}
