//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Lucas Baudin
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

#ifdef ADDONS_MANAGER

#include "states_screens/addons_update_screen.hpp"

#include <pthread.h>
#include <sstream>

#include "addons/addons_manager.hpp"
#include "addons/network_http.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets.hpp"
#include "io/file_manager.hpp"
#include "states_screens/addons_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/dialogs/addons_loading.hpp"

DEFINE_SCREEN_SINGLETON( AddonsUpdateScreen );

// ------------------------------------------------------------------------------------------------------

AddonsUpdateScreen::AddonsUpdateScreen() : Screen("addons_update.stkgui")
{
}

// ------------------------------------------------------------------------------------------------------

void AddonsUpdateScreen::loadedFromFile()
{
}
// ------------------------------------------------------------------------------------------------------

void AddonsUpdateScreen::eventCallback(GUIEngine::Widget* widget, 
                                       const std::string& name, 
                                       const int playerID)
{
    if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if (name == "list_addons")
    {
        GUIEngine::ListWidget* list = getWidget<GUIEngine::ListWidget>("list_addons");
        std::string addons = list->getSelectionInternalName();

        m_load = new AddonsLoading(0.8f, 0.8f, addons);
    }
    else if (name == "category")
    {
        std::string selection = 
            ((GUIEngine::RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        
        if (selection == "tab_track")
        {
            StateManager::get()->replaceTopMostScreen(AddonsScreen::getInstance());
            AddonsScreen::getInstance()->m_type = "track";
            AddonsScreen::getInstance()->loadList("track");
        }
        else if (selection == "tab_kart")
        {
            StateManager::get()->replaceTopMostScreen(AddonsScreen::getInstance());
            AddonsScreen::getInstance()->m_type = "kart";
            AddonsScreen::getInstance()->loadList("kart");
        }
    }
}

// ------------------------------------------------------------------------------------------------------

void AddonsUpdateScreen::init()
{
    Screen::init();
    getWidget<GUIEngine::RibbonWidget>("category")->select("tab_update", PLAYER_ID_GAME_MASTER);

    GUIEngine::ListWidget* w_list = this->getWidget<GUIEngine::ListWidget>("list_addons");
    w_list->clear();

    for(unsigned int i=0; i<addons_manager->getNumAddons(); i++)
    {
        const Addon &addon = addons_manager->getAddon(i);
        if(addon.isInstalled() && addon.needsUpdate())
		{
			std::cout << addon.getName() << std::endl;
			w_list->addItem(addon.getId(), addon.getName().c_str(), 0);
		}
    }   // for i<getNumAddons
}

#endif
