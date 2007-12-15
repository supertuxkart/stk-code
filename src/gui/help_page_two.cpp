//  $Id: help_menu.cpp 812 2006-10-07 11:43:57Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#include "help_page_two.hpp"
#include "widget_manager.hpp"
#include "race_manager.hpp"
#include "menu_manager.hpp"
#include "user_config.hpp"
#include "player.hpp"
#include "collectable_manager.hpp"
#include "material.hpp"
#include "translation.hpp"

enum WidgetTokens
{
    WTOK_MSG6,

    WTOK_ITEMIMG1, WTOK_ITEMTXT1,
    WTOK_ITEMIMG2, WTOK_ITEMTXT2,
    WTOK_ITEMIMG3, WTOK_ITEMTXT3,
    WTOK_ITEMIMG4, WTOK_ITEMTXT4,
    WTOK_ITEMIMG5, WTOK_ITEMTXT5,
    WTOK_ITEMIMG6, WTOK_ITEMTXT6,

    WTOK_FIRST_PAGE,
    WTOK_QUIT
};

HelpPageTwo::HelpPageTwo()
{

    /* Add the widgets */
    const bool SHOW_RECT = true;
    const WidgetFontSize TEXT_SIZE = WGT_FNT_SML;
    widget_manager->setInitialRectState( SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK );
    widget_manager->setInitialTextState( false, "", TEXT_SIZE );

    widget_manager->addWgt(WTOK_MSG6, 100, 8);
    widget_manager->setWgtText(WTOK_MSG6,
        _("To help you win, there are certain collectables you can grab:"));
    widget_manager->showWgtText( WTOK_MSG6 );
    widget_manager->breakLine();

    /* Collectable images and descriptions */
    widget_manager->addWgt(WTOK_ITEMIMG1, 10, 13);
    widget_manager->setWgtTexture(WTOK_ITEMIMG1,
        collectable_manager->getIcon(COLLECT_MISSILE)->getState()->getTextureHandle());
    widget_manager->setWgtColor(WTOK_ITEMIMG1, WGT_WHITE);
    widget_manager->showWgtTexture(WTOK_ITEMIMG1);
    widget_manager->setWgtRoundCorners(WTOK_ITEMIMG1, WGT_AREA_NONE);

    widget_manager->addWgt(WTOK_ITEMTXT1, 90, 13);
    widget_manager->setWgtText( WTOK_ITEMTXT1,
        _("Missile - fast stopper in a straight line"));
    widget_manager->showWgtText( WTOK_ITEMTXT1 );
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_ITEMIMG2, 10, 13);
    widget_manager->setWgtTexture(WTOK_ITEMIMG2,
        collectable_manager->getIcon(COLLECT_HOMING)->getState()->getTextureHandle());
    widget_manager->setWgtColor(WTOK_ITEMIMG2, WGT_WHITE);
    widget_manager->showWgtTexture( WTOK_ITEMIMG2 );
    widget_manager->setWgtRoundCorners(WTOK_ITEMIMG2, WGT_AREA_NONE);

    widget_manager->addWgt(WTOK_ITEMTXT2, 90, 13);
    widget_manager->setWgtText( WTOK_ITEMTXT2,
        _("Homing missile - follows rivals, but is slower than the missile"));
    widget_manager->showWgtText( WTOK_ITEMTXT2 );
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_ITEMIMG3, 10, 13);
    widget_manager->setWgtTexture(WTOK_ITEMIMG3,
        collectable_manager->getIcon(COLLECT_SPARK)->getState()->getTextureHandle());
    widget_manager->setWgtColor(WTOK_ITEMIMG3, WGT_WHITE);
    widget_manager->showWgtTexture( WTOK_ITEMIMG3 );
    widget_manager->setWgtRoundCorners(WTOK_ITEMIMG3, WGT_AREA_NONE);

    widget_manager->addWgt(WTOK_ITEMTXT3, 90, 13);
    widget_manager->setWgtText( WTOK_ITEMTXT3,
        _("Fuzzy blob/Spark - very slow, but bounces from walls"));
    widget_manager->showWgtText( WTOK_ITEMTXT3 );
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_ITEMIMG4, 10, 13);
    widget_manager->setWgtTexture(WTOK_ITEMIMG4,
        collectable_manager->getIcon(COLLECT_ZIPPER)->getState()->getTextureHandle());
    widget_manager->setWgtColor(WTOK_ITEMIMG4, WGT_WHITE);
    widget_manager->showWgtTexture( WTOK_ITEMIMG4 );
    widget_manager->setWgtRoundCorners(WTOK_ITEMIMG4, WGT_AREA_NONE);

    widget_manager->addWgt(WTOK_ITEMTXT4, 90, 13);
    widget_manager->setWgtText( WTOK_ITEMTXT4,
        _("Zipper - speed boost"));
    widget_manager->showWgtText( WTOK_ITEMTXT4 );
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_ITEMIMG5, 10, 13);
    widget_manager->setWgtTexture(WTOK_ITEMIMG5,
        collectable_manager->getIcon(COLLECT_PARACHUTE)->getState()->getTextureHandle());
    widget_manager->setWgtColor(WTOK_ITEMIMG5, WGT_WHITE);
    widget_manager->showWgtTexture( WTOK_ITEMIMG5 );
    widget_manager->setWgtRoundCorners(WTOK_ITEMIMG5, WGT_AREA_NONE);

    widget_manager->addWgt(WTOK_ITEMTXT5, 90, 13);
    widget_manager->setWgtText( WTOK_ITEMTXT5,
        _("Parachute - slows down all karts in a better position!"));
    widget_manager->showWgtText( WTOK_ITEMTXT5 );
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_ITEMIMG6, 10, 13);
    widget_manager->setWgtTexture(WTOK_ITEMIMG6,
        collectable_manager->getIcon(COLLECT_ANVIL)->getState()->getTextureHandle());
    widget_manager->setWgtColor(WTOK_ITEMIMG6, WGT_WHITE);
    widget_manager->showWgtTexture( WTOK_ITEMIMG6 );
    widget_manager->setWgtRoundCorners(WTOK_ITEMIMG6, WGT_AREA_NONE);

    widget_manager->addWgt(WTOK_ITEMTXT6, 90, 13);
    widget_manager->setWgtText( WTOK_ITEMTXT6,
        _("Anvil - slows down greatly the kart in the first position"));
    widget_manager->showWgtText( WTOK_ITEMTXT6 );
    widget_manager->breakLine();

#ifdef USE_MAGNETS
    //Magnets are currently disabled.
#endif

    /*Buttons at the bottom*/
    widget_manager->addWgt(WTOK_FIRST_PAGE, 25, 7);
    widget_manager->setWgtText(WTOK_FIRST_PAGE, _("Previous screen"));
    widget_manager->showWgtText( WTOK_FIRST_PAGE );
    widget_manager->activateWgt(WTOK_FIRST_PAGE);
    widget_manager->breakLine();

    widget_manager->addWgt(WTOK_QUIT, 40, 7);
    widget_manager->setWgtText(WTOK_QUIT, _("Go back to the main menu"));
    widget_manager->showWgtText( WTOK_QUIT );
    widget_manager->activateWgt(WTOK_QUIT);

    widget_manager->layout( WGT_AREA_TOP );
}   // HelpMenu

//-----------------------------------------------------------------------------
HelpPageTwo::~HelpPageTwo()
{
    widget_manager->reset();
}   // ~HelpMenu

//-----------------------------------------------------------------------------
void HelpPageTwo::select()
{
    switch ( widget_manager->getSelectedWgt() )
    {
        case WTOK_FIRST_PAGE:
            menu_manager->popMenu();
            menu_manager->pushMenu(MENUID_HELP1);
            break;

        case WTOK_QUIT:
            menu_manager->popMenu();
            break;
    }
}   // select

/* EOF */
