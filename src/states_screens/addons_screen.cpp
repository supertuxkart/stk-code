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

#include "states_screens/addons_screen.hpp"
#include "states_screens/addons_update_screen.hpp"
#include "states_screens/dialogs/addons_loading.hpp"

#include "guiengine/widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets.hpp"
#include "states_screens/state_manager.hpp"
#include "addons/network.hpp"
#include "guiengine/CGUISpriteBank.h"
#include "addons/addons.hpp"
#include "io/file_manager.hpp"

#include "irrlicht.h"
/*pthread aren't supported natively by windows. Here a port: http://sourceware.org/pthreads-win32/ */
#include <pthread.h>
#include <sstream>

DEFINE_SCREEN_SINGLETON( AddonsScreen );

// ------------------------------------------------------------------------------------------------------

AddonsScreen::AddonsScreen() : Screen("addons.stkgui")
{
}

// ------------------------------------------------------------------------------------------------------

void AddonsScreen::loadedFromFile()
{

    video::ITexture* icon1 = irr_driver->getTexture( file_manager->getGUIDir()
            + "/package.png" );
    video::ITexture* icon2 = irr_driver->getTexture( file_manager->getGUIDir()
            + "/no-package.png" );

    m_icon_bank = new irr::gui::STKModifiedSpriteBank( GUIEngine::getGUIEnv() );
    m_icon_bank->addTextureAsSprite(icon1);
    m_icon_bank->addTextureAsSprite(icon2);
}
// ------------------------------------------------------------------------------------------------------
void AddonsScreen::loadList()
{
	std::cout << "load list" << std::endl;
    GUIEngine::ListWidget* w_list = this->getWidget<GUIEngine::ListWidget>("list_addons");
    w_list->clear();
    addons_manager->resetIndex();
	//w_list->addItem("kart", _("Karts:"), -1 /* no icon */);
    while(addons_manager->NextType(this->type))
    {
        std::cout << addons_manager->GetName() << std::endl;
	    if(addons_manager->IsInstalledAsBool())
        	w_list->addItem(addons_manager->GetIdAsStr().c_str(),
        	        addons_manager->GetName().c_str(), 0 /* icon installed */);

	    else
        	w_list->addItem(addons_manager->GetIdAsStr().c_str(),
        	        addons_manager->GetName().c_str(), 1 /* icon unsinstalled*/);
    }

	this->can_load_list = false;
	this->getWidget<GUIEngine::RibbonWidget>("category")->setActivated();
	this->getWidget<GUIEngine::LabelWidget>("update_status")->setText("");
	if(type == "kart")
    	this->getWidget<GUIEngine::RibbonWidget>("category")->select("tab_kart", PLAYER_ID_GAME_MASTER);
	else if(type == "track")
    	this->getWidget<GUIEngine::RibbonWidget>("category")->select("tab_track", PLAYER_ID_GAME_MASTER);
}
// ------------------------------------------------------------------------------------------------------

void AddonsScreen::eventCallback(GUIEngine::Widget* widget, const std::string& name, const int playerID)
{
    if (name == "back")
    {
        StateManager::get()->escapePressed();
    }

    else if (name == "list_addons")
    {
        GUIEngine::ListWidget* list = this->getWidget<GUIEngine::ListWidget>("list_addons");
        std::string addons = list->getSelectionInternalName();

        if(addons != "track" && addons != "kart")
        {
            addons_manager->SelectId(addons);
            this->load = new AddonsLoading(addons_manager, 0.8f, 0.8f);
        }
    }
    if (name == "category")
    {
        std::string selection = ((GUIEngine::RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER).c_str();
        std::cout << selection << std::endl;
        if (selection == "tab_update") StateManager::get()->replaceTopMostScreen(AddonsUpdateScreen::getInstance());
        else if (selection == "tab_track")
        {
            this->type = "track";
            loadList();
        }
        else if (selection == "tab_kart")
        {
            this->type = "kart";
            loadList();
        }
    }
}

// ------------------------------------------------------------------------------------------------------
void AddonsScreen::onUpdate(float delta,  irr::video::IVideoDriver* driver)
{
	pthread_mutex_lock(&(this->mutex));
    if(this->can_load_list)
    {
        this->loadList();
    }
	pthread_mutex_unlock(&(this->mutex));
}
void AddonsScreen::init()
{
    Screen::init();
	this->getWidget<GUIEngine::RibbonWidget>("category")->setDeactivated();
    this->type = "kart";

    pthread_mutex_init(&(this->mutex), NULL);
    std::cout << "Addons dir:" + file_manager->getAddonsDir() << std::endl;
    GUIEngine::ListWidget* w_list = this->getWidget<GUIEngine::ListWidget>("list_addons");
    w_list->setIcons(m_icon_bank);
    //w_list->clear();
    std::cout << "icon bank" << std::endl;
	this->can_load_list = false;

    this->getWidget<GUIEngine::LabelWidget>("update_status")->setText(_("Updating the list..."));
    pthread_t thread;
    pthread_create(&thread, NULL, &AddonsScreen::downloadList, this);
}

// ------------------------------------------------------------------------------------------------------
void * AddonsScreen::downloadList( void * pthis)
{
    AddonsScreen * pt = (AddonsScreen*)pthis;	
	pthread_mutex_lock(&(pt->mutex));
	pt->can_load_list = true;
	pthread_mutex_unlock(&(pt->mutex));
	
    return NULL;
}
#endif
