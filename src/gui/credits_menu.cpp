//  $Id$
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
#include <fstream>
#include <stdexcept>
#include <iostream>

#include "credits_menu.hpp"
#include "loader.hpp"
#include "translation.hpp"
#include "widget_manager.hpp"
#include "menu_manager.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define strdup _strdup
#endif

enum WidgetTokens
{
    WTOK_CREDITS,
    WTOK_QUIT
};


CreditsMenu::CreditsMenu()
{
    std::string filename;
    std::string line;
    std::string credits_text;

    filename = loader->getPath("data/CREDITS");
    std::ifstream file(filename.c_str());

    if( file.is_open() )
    {
        while( !file.eof() )
        {
            getline(file, line);
            credits_text.append(line);
            credits_text.push_back('\n');
        }   // while
        file.close();
    }
    else
    {
        printf(_("Couldn't load '%s'\n"),filename.c_str());
        credits_text.append(_("CREDIT file was not installed properly!!\n"));
        credits_text.append(_("Please check 'data/CREDITS'!!"));
    }


    const bool SHOW_RECT = true;
    const bool SHOW_TEXT = true;
    const WidgetFontSize TEXT_SIZE = WGT_FNT_SML;

    widget_manager->setInitialActivationState( true );
    widget_manager->setInitialRectState(SHOW_RECT, WGT_AREA_ALL, WGT_TRANS_BLACK);
    widget_manager->setInitialTextState(SHOW_TEXT, "", TEXT_SIZE );

    widget_manager->addWgt( WTOK_CREDITS, 100, 93);
    widget_manager->setWgtText( WTOK_CREDITS, credits_text );
    //FIXME: maybe I should make scroll names more consistent
    widget_manager->enableWgtScroll( WTOK_CREDITS );
    widget_manager->setWgtYScrollPos( WTOK_CREDITS, WGT_SCROLL_START_BOTTOM );
    widget_manager->setWgtYScrollSpeed( WTOK_CREDITS, -0.25f );
    widget_manager->breakLine();

    widget_manager->addWgt( WTOK_QUIT, 40, 7);
    widget_manager->setWgtText( WTOK_QUIT, _("Go back to the main menu"));

    widget_manager->layout( WGT_AREA_TOP );
}   // CreditsMenu

//-----------------------------------------------------------------------------
CreditsMenu::~CreditsMenu()
{
    widget_manager->reset() ;
}   // ~CreditsMenu

//-----------------------------------------------------------------------------
void CreditsMenu::select()
{
    switch( widget_manager->getSelectedWgt() )
    {
        case WTOK_QUIT:
            menu_manager->popMenu();
            break;
    }
}
/* EOF */
