//  $Id: help_menu.cpp 812 2006-10-07 11:43:57Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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
    WTOK_MSG,

    WTOK_IMG1, WTOK_TXT1,
    WTOK_IMG2, WTOK_TXT2,
    WTOK_IMG3, WTOK_TXT3,
    WTOK_IMG4, WTOK_TXT4,
    WTOK_IMG5, WTOK_TXT5,
    WTOK_IMG6, WTOK_TXT6,

    WTOK_FIRST_PAGE,
    WTOK_THIRD_PAGE,
    WTOK_QUIT

};

HelpPageTwo::HelpPageTwo()
{
    //FIXME: instead of using setInitialTextState, the gui & widget manager macros should improve it's design
    widget_manager->setInitialTextState
    (
        false,
        "",
        WGT_FNT_SML,
        WGT_FONT_GUI,
        WGT_WHITE,
        false
    );

    widget_manager->addTextWgt( WTOK_MSG, 100, 7,
        _("To help you win, there are certain collectables you can grab:"));
    widget_manager->breakLine();

    widget_manager->addImgWgt( WTOK_IMG1, 10, 12,
        collectable_manager->getIcon(COLLECT_MISSILE)->getState()->getTextureHandle());

    widget_manager->addTextWgt( WTOK_TXT1, 90, 12,
        _("Missile - fast stopper in a straight line"));
    widget_manager->setWgtRoundCorners( WTOK_TXT1, WGT_AREA_RGT );
    widget_manager->breakLine();

    widget_manager->addImgWgt(WTOK_IMG2, 10, 12,
        collectable_manager->getIcon(COLLECT_HOMING)->getState()->getTextureHandle());

    widget_manager->addTextWgt(WTOK_TXT2, 90, 12,
        _("Cake - thrown at the closest rival,\nbest on short ranges and long straights"));
    widget_manager->setWgtRoundCorners( WTOK_TXT2, WGT_AREA_RGT );
    widget_manager->breakLine();

    widget_manager->addImgWgt(WTOK_IMG3, 10, 12,
        collectable_manager->getIcon(COLLECT_BOWLING)->getState()->getTextureHandle());

    widget_manager->addTextWgt(WTOK_TXT3, 90, 12,
        _("Bowling Ball - bounces off walls. If you are looking back,\nit will be thrown backwards."));
    widget_manager->setWgtRoundCorners( WTOK_TXT3, WGT_AREA_RGT );
    widget_manager->breakLine();

    widget_manager->addImgWgt(WTOK_IMG4, 10, 12,
        collectable_manager->getIcon(COLLECT_ZIPPER)->getState()->getTextureHandle());

    widget_manager->addTextWgt(WTOK_TXT4, 90, 12,
        _("Zipper - speed boost"));
    widget_manager->setWgtRoundCorners(WTOK_TXT4, WGT_AREA_RGT);
    widget_manager->breakLine();

    widget_manager->addImgWgt(WTOK_IMG5, 10, 12,
        collectable_manager->getIcon(COLLECT_PARACHUTE)->getState()->getTextureHandle());

    widget_manager->addTextWgt(WTOK_TXT5, 90, 12,
        _("Parachute - slows down all karts in a better position!"));
    widget_manager->setWgtRoundCorners(WTOK_TXT5, WGT_AREA_RGT);
    widget_manager->breakLine();

    widget_manager->addImgWgt(WTOK_IMG6, 10, 12,
        collectable_manager->getIcon(COLLECT_ANVIL)->getState()->getTextureHandle());

    widget_manager->addTextWgt(WTOK_TXT6, 90, 12,
        _("Anvil - slows down greatly the kart in the first position"));
    widget_manager->setWgtRoundCorners(WTOK_TXT6, WGT_AREA_RGT);
    widget_manager->breakLine();

    /*Buttons at the bottom*/
    widget_manager->addTextButtonWgt(WTOK_FIRST_PAGE, 40, 7,
        _("Previous screen"));
    widget_manager->breakLine();

    widget_manager->addTextButtonWgt(WTOK_THIRD_PAGE, 40, 7,
        _("Next help screen"));
    widget_manager->breakLine();

    widget_manager->addTextButtonWgt(WTOK_QUIT, 40, 7,
        _("Back to the menu"));

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

        case WTOK_THIRD_PAGE:
            menu_manager->popMenu();
            menu_manager->pushMenu(MENUID_HELP3);
            break;

        case WTOK_QUIT:
            menu_manager->popMenu();
            break;
    }
}   // select

/* EOF */
