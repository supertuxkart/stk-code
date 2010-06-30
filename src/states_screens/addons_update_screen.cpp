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

#include "states_screens/addons_screen.hpp"
#include "states_screens/dialogs/addons_loading.hpp"

#include "guiengine/widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets.hpp"
#include "states_screens/state_manager.hpp"
#include "addons/network.hpp"
#include "addons/addons.hpp"
#include "io/file_manager.hpp"

/*pthread aren't supported natively by windows. Here a port: http://sourceware.org/pthreads-win32/ */
#include <pthread.h>
#include <sstream>

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
void AddonsUpdateScreen::download_list()
{
/*
    GUIEngine::ListWidget* w_list = this->getWidget<GUIEngine::ListWidget>("list_karts");
    
	this->addons = new Addons(std::string(file_manager->getConfigDir() + "/list_text"), std::string(file_manager->getConfigDir() + "/installed.xml"));
	//to have the name of the first karts and load it informatins later
	this->addons->Next();
	std::string first_kart = this->addons->GetName();
	std::cout << this->addons->GetName() << std::endl;
	w_list->addItem(std::string("list_karts" + this->addons->GetName()).c_str(), this->addons->GetName().c_str(), 0 /* icon */);
    /*while(this->addons->Next())
    {
        std::cout << this->addons->GetName() << std::endl;
        w_list->addItem(std::string("list_karts" + this->addons->GetName()).c_str(), this->addons->GetName().c_str(), 0 /* icon *//*);
    }
    this->addons->Select(first_kart);
	this->loadInformations();*/
}
// ------------------------------------------------------------------------------------------------------

void AddonsUpdateScreen::eventCallback(GUIEngine::Widget* widget, const std::string& name, const int playerID)
{
    if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if (name == "install")
    {
        this->load = new AddonsLoading(0.4f, 0.4f);
        pthread_t thread;
        pthread_create(&thread, NULL, *startInstall, this);
        //this->addons->Install();
    }
    else if (name.find("list_karts") == 0)
    {
        GUIEngine::ListWidget* list = this->getWidget<GUIEngine::ListWidget>("list_karts");
        std::string kart = list->getSelectionInternalName().replace(0, 10, "");
        this->addons->Select(kart);
        this->loadInformations();
    }
}

// ------------------------------------------------------------------------------------------------------

void AddonsUpdateScreen::init()
{
    pthread_t nThreadID2;
    pthread_create(&nThreadID2, NULL, *download_l, this);
}

// ------------------------------------------------------------------------------------------------------

void AddonsUpdateScreen::tearDown()
{
}
void AddonsUpdateScreen::loadInformations()
{
        std::cout << this->addons->GetName() << std::endl;
        GUIEngine::LabelWidget* w = this->getWidget<GUIEngine::LabelWidget>("name_addons");
        w->setText(std::string("Name: "+ this->addons->GetName()).c_str());
        w = this->getWidget<GUIEngine::LabelWidget>("description_addons");
        w->setText(std::string("Description: " + this->addons->GetDescription()).c_str());
        w = this->getWidget<GUIEngine::LabelWidget>("version_addons");
        std::ostringstream os;
        os << this->addons->GetVersion();
        w->setText(std::string("Version: " + os.str()).c_str());
        w = this->getWidget<GUIEngine::LabelWidget>("install_addons");
        w->setText(std::string("Installed: " + this->addons->IsInstalled()).c_str());
        
        GUIEngine::ButtonWidget* button = this->getWidget<GUIEngine::ButtonWidget>("install");
        if(this->addons->IsInstalled() == "yes")
        {
            button->setLabel(std::string("Uninstall").c_str());
        }
        else
        {
            button->setLabel(std::string("Install").c_str());
        }
}
// ------------------------------------------------------------------------------------------------------
//I dislike this way, it is too dirty but I didn't find another way
void * startInstall(void* pthis)
{
    AddonsUpdateScreen * obj = (AddonsUpdateScreen*)pthis;
    if(obj->addons->IsInstalled() == "yes")
    {
    std::cout << obj->addons->IsInstalled() << std::endl;
        obj->addons->UnInstall();
    }
    else
    {
        obj->addons->Install();
    }
    obj->load->close();
    obj->loadInformations();
    return NULL;
}
