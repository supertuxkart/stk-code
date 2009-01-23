//  $Id$
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

#include "credits_menu.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>
#if defined(WIN32) && !defined(__CYGWIN__)
#  define strdup _strdup
#endif

#include "file_manager.hpp"
#include "gui/menu_manager.hpp"
#include "gui/widget_manager.hpp"
#include "utils/translation.hpp"

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

    filename = file_manager->getConfigFile("CREDITS");
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
        printf("Couldn't load '%s'\n",filename.c_str());
        credits_text.append("CREDIT file was not installed properly!!\n");
        credits_text.append("Please check 'data/CREDITS'!!");
    }


    widget_manager->addTextWgt( WTOK_CREDITS, 100, 93, "" );
    widget_manager->setWgtTextSize( WTOK_CREDITS, WGT_FNT_SML );
    //FIXME: maybe I should make scroll names more consistent
    widget_manager->enableWgtScroll( WTOK_CREDITS );
    widget_manager->setWgtYScrollPos( WTOK_CREDITS, WGT_SCROLL_START_BOTTOM );
    widget_manager->setWgtCornerRadius( WTOK_CREDITS, 1);
    widget_manager->showWgtBorder( WTOK_CREDITS );
    widget_manager->setWgtBorderColor( WTOK_CREDITS, WGT_TRANS_GREEN );
    widget_manager->setWgtBorderPercentage( WTOK_CREDITS, 2);
    widget_manager->setWgtYScrollSpeed( WTOK_CREDITS, -80 );
    widget_manager->breakLine();
    
    widget_manager->addTextButtonWgt( WTOK_QUIT, 40, 7,
        _("Go back to the main menu"));

    widget_manager->layout( WGT_AREA_TOP );

    //This is done after layout so the widget doesn't gets resized.
    widget_manager->setWgtText( WTOK_CREDITS, credits_text );
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
