//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Lucas Baudin, Joerg Henrichs
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

#include "states_screens/addons_screen.hpp"

#include <sstream>

#include "irrlicht.h"

#include "addons/addons_manager.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/CGUISpriteBank.h"
#include "io/file_manager.hpp"
#include "states_screens/dialogs/addons_loading.hpp"
#include "states_screens/state_manager.hpp"

DEFINE_SCREEN_SINGLETON( AddonsScreen );

// ------------------------------------------------------------------------------------------------------

AddonsScreen::AddonsScreen() : Screen("addons.stkgui")
{
}

// ------------------------------------------------------------------------------------------------------

void AddonsScreen::loadedFromFile()
{

    video::ITexture* icon1 = irr_driver->getTexture( file_manager->getGUIDir()
                                                    + "/package.png"         );
    video::ITexture* icon2 = irr_driver->getTexture( file_manager->getGUIDir()
                                                    + "/no-package.png"      );
    video::ITexture* icon3 = irr_driver->getTexture( file_manager->getGUIDir()
                                                    + "/package-update.png"  );

    m_icon_bank = new irr::gui::STKModifiedSpriteBank( GUIEngine::getGUIEnv());
    m_icon_installed     = m_icon_bank->addTextureAsSprite(icon1);
    m_icon_not_installed = m_icon_bank->addTextureAsSprite(icon2);
    m_icon_needs_update  = m_icon_bank->addTextureAsSprite(icon3);
    
    GUIEngine::ListWidget* w_list = 
        getWidget<GUIEngine::ListWidget>("list_addons");
    w_list->addColumn( L"Add-on name" );
    w_list->addColumn( L"Updated date" );
}   // loadedFromFile

// ----------------------------------------------------------------------------

void AddonsScreen::init()
{
    Screen::init();
	getWidget<GUIEngine::RibbonWidget>("category")->setDeactivated();

    if(UserConfigParams::logAddons())
        std::cout << "[addons] Using directory <" + file_manager->getAddonsDir() 
              << ">\n";
    
    GUIEngine::ListWidget* w_list = 
        getWidget<GUIEngine::ListWidget>("list_addons");
    w_list->setIcons(m_icon_bank);
    
    m_type = "kart";
    loadList();
}   // init

// ----------------------------------------------------------------------------
/** Loads the list of all addons of the given type. The gui element will be
 *  updated.
 *  \param type Must be 'kart' or 'track'.
 */
void AddonsScreen::loadList()
{
    GUIEngine::ListWidget* w_list = 
        getWidget<GUIEngine::ListWidget>("list_addons");
    w_list->clear();
    for(unsigned int i=0; i<addons_manager->getNumAddons(); i++)
    {
        const Addon &addon = addons_manager->getAddon(i);
        // Ignore addons of a different type
        if(addon.getType()!=m_type) continue;
        
        // Get the right icon to display
        int icon;
        if(addon.isInstalled())
            icon = addon.needsUpdate() ? m_icon_needs_update 
                                       : m_icon_installed;
	    else
        	icon = m_icon_not_installed;

        w_list->addItem(addon.getId(), addon.getName().c_str(), 
                        icon);
    }

	getWidget<GUIEngine::RibbonWidget>("category")->setActivated();
	if(m_type == "kart")
    	getWidget<GUIEngine::RibbonWidget>("category")->select("tab_kart", 
                                                        PLAYER_ID_GAME_MASTER);
	else if(m_type == "track")
    	getWidget<GUIEngine::RibbonWidget>("category")->select("tab_track", 
                                                        PLAYER_ID_GAME_MASTER);
    else
    	getWidget<GUIEngine::RibbonWidget>("category")->select("tab_update", 
                                                        PLAYER_ID_GAME_MASTER);
}   // loadList

// ----------------------------------------------------------------------------
void AddonsScreen::eventCallback(GUIEngine::Widget* widget, 
                                 const std::string& name, const int playerID)
{
    if (name == "back")
    {
        StateManager::get()->escapePressed();
    }

    else if (name == "list_addons")
    {
        GUIEngine::ListWidget* list = 
            getWidget<GUIEngine::ListWidget>("list_addons");
        std::string id = list->getSelectionInternalName();

        if (!id.empty())
        {
            new AddonsLoading(0.8f, 0.8f, id);
        }
    }
    if (name == "category")
    {
        std::string selection = ((GUIEngine::RibbonWidget*)widget)
                         ->getSelectionIDString(PLAYER_ID_GAME_MASTER).c_str();
        std::cout << selection << std::endl;
        if (selection == "tab_track")
        {
            m_type = "track";
            loadList();
        }
        else if (selection == "tab_kart")
        {
            m_type = "kart";
            loadList();
        }
    }
}   // eventCallback

// ----------------------------------------------------------------------------

#endif
