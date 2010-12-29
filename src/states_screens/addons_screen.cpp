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

#include <pthread.h>
#include <sstream>

#include "irrlicht.h"

#include "addons/addons_manager.hpp"
#include "addons/network_http.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/CGUISpriteBank.h"
#include "io/file_manager.hpp"
#include "states_screens/addons_update_screen.hpp"
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
    m_icon_bank->addTextureAsSprite(icon1);
    m_icon_bank->addTextureAsSprite(icon2);
    m_icon_bank->addTextureAsSprite(icon3);
}
// ----------------------------------------------------------------------------
void AddonsScreen::loadList()
{
	std::cout << "load list" << std::endl;
    GUIEngine::ListWidget* w_list = 
        getWidget<GUIEngine::ListWidget>("list_addons");
    w_list->clear();
    for(unsigned int i=0; i<addons_manager->getNumAddons(); i++)
    {
        const AddonsManager::AddonsProp &addons = addons_manager->getAddons(i);

        std::cout << addons.getName()<< std::endl;
        if(addons.isInstalled() && 
            addons.getInstalledVersion() < addons.getVersion())
        {
        	w_list->addItem(addons.getId(), addons.getName().c_str(), 
                            2 /* icon installed */);
        }
	    else if(addons.isInstalled())
        {
        	w_list->addItem(addons.getId(),
        	        addons_manager->getName().c_str(), 0 /* icon installed */);
        }
	    else
        {
        	w_list->addItem(addons.getId(), addons.getName().c_str(),
        	                1 /* icon unsinstalled*/);
        }
    }

	m_can_load_list = false;
	getWidget<GUIEngine::RibbonWidget>("category")->setActivated();
	getWidget<GUIEngine::LabelWidget>("update_status")->setText("");
	if(m_type == "kart")
    	getWidget<GUIEngine::RibbonWidget>("category")->select("tab_kart", 
                                                        PLAYER_ID_GAME_MASTER);
	else if(m_type == "track")
    	getWidget<GUIEngine::RibbonWidget>("category")->select("tab_track", 
                                                        PLAYER_ID_GAME_MASTER);
}
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
        std::string addons = list->getSelectionInternalName();

        addons_manager->selectId(addons);
        m_load = new AddonsLoading(0.8f, 0.8f);
    }
    if (name == "category")
    {
        std::string selection = ((GUIEngine::RibbonWidget*)widget)
                         ->getSelectionIDString(PLAYER_ID_GAME_MASTER).c_str();
        std::cout << selection << std::endl;
        if (selection == "tab_update") 
            StateManager::get()->replaceTopMostScreen(AddonsUpdateScreen::getInstance());
        else if (selection == "tab_track")
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
}

// ----------------------------------------------------------------------------

void AddonsScreen::onUpdate(float delta,  irr::video::IVideoDriver* driver)
{
	pthread_mutex_lock(&m_mutex);
    if(m_can_load_list)
    {
        loadList();
    }
	pthread_mutex_unlock(&m_mutex);
}

// ----------------------------------------------------------------------------

void AddonsScreen::init()
{
    Screen::init();
	getWidget<GUIEngine::RibbonWidget>("category")->setDeactivated();
    m_type = "kart";

    pthread_mutex_init(&m_mutex, NULL);
    std::cout << "[Addons] Using directory <" + file_manager->getAddonsDir() 
              << ">\n";
    GUIEngine::ListWidget* w_list = 
        getWidget<GUIEngine::ListWidget>("list_addons");
    w_list->setIcons(m_icon_bank);
    //w_list->clear();
    std::cout << "icon bank" << std::endl;
	m_can_load_list = false;

    getWidget<GUIEngine::LabelWidget>("update_status")
        ->setText(_("Updating the list..."));
    pthread_t thread;
    pthread_create(&thread, NULL, &AddonsScreen::downloadList, this);
}

// ----------------------------------------------------------------------------

void *AddonsScreen::downloadList( void *pthis)
{
    AddonsScreen *pt = (AddonsScreen*)pthis;	
	pthread_mutex_lock(&(pt->m_mutex));
	pt->m_can_load_list = true;
	pthread_mutex_unlock(&(pt->m_mutex));
	
    return NULL;
}
#endif
