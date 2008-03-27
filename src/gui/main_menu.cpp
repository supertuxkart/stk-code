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

#include "main_menu.hpp"
#include "widget_manager.hpp"
#include "race_manager.hpp"
#include "menu_manager.hpp"
#include "translation.hpp"
#include "user_config.hpp"
#include "unlock_manager.hpp"

enum WidgetTokens
{
    WTOK_SINGLE,
    WTOK_MULTI,
    WTOK_CHALLENGES,
    WTOK_OPTIONS,
    WTOK_QUIT,
    WTOK_EMPTY,
    WTOK_HELP,
    WTOK_CREDITS,
    WTOK_WARNING
};

MainMenu::MainMenu()
{
    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    widget_manager->setInitialActivationState(true);
    widget_manager->setInitialRectState(SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK);
    widget_manager->setInitialTextState(SHOW_TEXT, "", WGT_FNT_MED,
        WGT_FONT_GUI );

    widget_manager->insertColumn();
    widget_manager->addWgt(WTOK_SINGLE, 25, 7);
    widget_manager->setWgtText( WTOK_SINGLE, _("Single Player") );

    widget_manager->addWgt(WTOK_MULTI, 25, 7);
    widget_manager->setWgtText( WTOK_MULTI, _("Multiplayer") );

    std::vector<const Challenge*> all_challenges=unlock_manager->getActiveChallenges();
    if(all_challenges.size()>0)
    {
        widget_manager->addWgt(WTOK_CHALLENGES, 25, 7);
        widget_manager->setWgtText( WTOK_CHALLENGES, _("Challenges") );
    }
    widget_manager->addWgt(WTOK_OPTIONS, 25, 7);
    widget_manager->setWgtText( WTOK_OPTIONS, _("Options") );

    widget_manager->addWgt(WTOK_QUIT, 25, 7);
    widget_manager->setWgtText( WTOK_QUIT, _("Quit") );

    widget_manager->addWgt(WTOK_EMPTY, 25, 7);
    widget_manager->hideWgtText( WTOK_EMPTY );
    widget_manager->hideWgtRect( WTOK_EMPTY );
    widget_manager->deactivateWgt( WTOK_EMPTY );

    widget_manager->addWgt(WTOK_HELP, 25, 7);
    widget_manager->setWgtText( WTOK_HELP, _("Help") );
    //FIXME: if text size is not set, we get a crash when resizing the rect to the text
    widget_manager->setWgtTextSize( WTOK_HELP, WGT_FNT_SML );

    widget_manager->addWgt(WTOK_CREDITS, 25, 7);
    widget_manager->setWgtText( WTOK_CREDITS, _("Credits") );
    widget_manager->setWgtTextSize( WTOK_CREDITS, WGT_FNT_SML );

    if(user_config->getWarning()!="")
    {
        widget_manager->addWgt( WTOK_WARNING, 80, 8 );
        widget_manager->setWgtText( WTOK_WARNING, user_config->getWarning().c_str() );
        widget_manager->setWgtTextSize( WTOK_WARNING, WGT_FNT_SML );
        widget_manager->deactivateWgt(WTOK_WARNING);
        widget_manager->hideWgtRect(WTOK_WARNING);
    }

    widget_manager->layout(WGT_AREA_ALL);
}

//-----------------------------------------------------------------------------
MainMenu::~MainMenu()
{
    widget_manager->reset();
    user_config->resetWarning();
}

//-----------------------------------------------------------------------------
void MainMenu::select()
{
    switch ( widget_manager->getSelectedWgt() )
    {
    case WTOK_SINGLE:
        race_manager->setNumPlayers(1);
        menu_manager->pushMenu(MENUID_GAMEMODE);
        break;
    case WTOK_MULTI:
        menu_manager->pushMenu(MENUID_NUMPLAYERS);
        break;
    case WTOK_CHALLENGES:
        menu_manager->pushMenu(MENUID_CHALLENGES);
        break;
    case WTOK_OPTIONS:
        menu_manager->pushMenu(MENUID_OPTIONS);
        break;

    case WTOK_QUIT:
        menu_manager->pushMenu(MENUID_EXITGAME);
        break;
    case WTOK_HELP:
        menu_manager->pushMenu(MENUID_HELP1);
        break;
    case WTOK_CREDITS:
        menu_manager->pushMenu(MENUID_CREDITS);
        break;
    }
}

//-----------------------------------------------------------------------------
void MainMenu::handle(GameAction ga, int value)
{
    switch ( ga )
    {
    case GA_LEAVE:
        if(!value) break;
        menu_manager->pushMenu(MENUID_EXITGAME);
        break;

    default:
        BaseGUI::handle(ga, value);
        break;
    }
}

/* EOF */
