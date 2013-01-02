//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Lionel Fuentes
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

#include "states_screens/soccer_setup_screen.hpp"

#include "states_screens/state_manager.hpp"
#include "states_screens/arenas_screen.hpp"

using namespace GUIEngine;
DEFINE_SCREEN_SINGLETON( SoccerSetupScreen );

// -----------------------------------------------------------------------------

SoccerSetupScreen::SoccerSetupScreen() : Screen("soccer_setup.stkgui")
{
}

// -----------------------------------------------------------------------------

void SoccerSetupScreen::loadedFromFile()
{
}

// -----------------------------------------------------------------------------
void SoccerSetupScreen::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if(name == "continue")
    {
        StateManager::get()->pushScreen( ArenasScreen::getInstance() );
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
}

// -----------------------------------------------------------------------------
void SoccerSetupScreen::init()
{
    Screen::init();
    
    Widget* players_table = getWidget<Widget>("players-table");
    assert(players_table != NULL);
    
    // BEGIN TODO
    /*
    Widget* div = new Widget(WTYPE_DIV);
    div->m_properties[PROP_LAYOUT] = "horizontal-row";
            //widget.m_properties[prop_flag] = core::stringc(prop_name).c_str(); else widget.m_properties[prop_flag] = ""
    div->setParent(players_table->getIrrlichtElement());
    players_table->getChildren().push_back(div);
    //players_table->add();
    */
    
/*    LabelWidget* lbl = new LabelWidget();
    lbl->m_properties[PROP_PROPORTION] = "1";
    lbl->m_properties[PROP_HEIGHT] = "100%";
    lbl->setText(L"bouYou", false);
    lbl->m_properties[PROP_TEXT_ALIGN] = "left";
            //widget.m_properties[prop_flag] = core::stringc(prop_name).c_str(); else widget.m_properties[prop_flag] = ""
    //lbl->setParent(players_table->getIrrlichtElement());
    players_table->getChildren().push_back(lbl);
    calculateLayout();
*/
    // END TODO
}
