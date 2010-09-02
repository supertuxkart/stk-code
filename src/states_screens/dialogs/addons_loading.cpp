//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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

#include "states_screens/dialogs/addons_loading.hpp"

#include <pthread.h>

#include "guiengine/engine.hpp"
#include "guiengine/widgets.hpp"
#include "input/input_manager.hpp"
#include "utils/translation.hpp"
#include "addons/addons.hpp"
#include "addons/network.hpp"
#include "states_screens/addons_screen.hpp"
#include "utils/string_utils.hpp"
#include "io/file_manager.hpp"

using namespace GUIEngine;
using namespace irr::gui;

// ------------------------------------------------------------------------------------------------------

AddonsLoading::AddonsLoading(Addons * id, const float w, const float h) :
        ModalDialog(w, h)
{
    loadFromFile("addons_view_dialog.stkgui");
    this->addons = id;
    m_can_install = false;
    m_percent_update = false;
    pthread_mutex_init(&m_mutex_can_install, NULL);

    /*Init the icon here to be able to load a single image*/
    icon = this->getWidget<IconButtonWidget>("icon");

    name = this->getWidget<LabelWidget>("name");

    description = this->getWidget<LabelWidget>("description");
    
    version = this->getWidget<LabelWidget>("version");
    
    if(this->addons->IsInstalledAsBool())
        this->getWidget<ButtonWidget>("install")->setLabel(_("Uninstall"));
    
    this->loadInfo();
}
void AddonsLoading::loadInfo()
{


    /*I think we can wait a little to have the icon ?*/
    download("icon/" + this->addons->GetIcon(), this->addons->GetName() + ".png");
    icon->setImage(std::string(file_manager->getConfigDir() + "/" +  this->addons->GetName() + ".png").c_str(), IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);



    name->setText(StringUtils::insertValues(_("Name: %i"), this->addons->GetName().c_str()));
    description->setText(StringUtils::insertValues(_("Description: %i"), this->addons->GetDescription().c_str()));
    version->setText(StringUtils::insertValues(_("Version: %i"), this->addons->GetVersionAsStr().c_str()));



}
// ------------------------------------------------------------------------------------------------------

GUIEngine::EventPropagation AddonsLoading::processEvent(const std::string& eventSource)
{
    if(eventSource == "cancel")
    {
        //input_manager->setMode(InputManager::MENU);
        dismiss();
        //return GUIEngine::EVENT_BLOCK;
    }
    else if(eventSource == "next")
    {
        this->addons->NextType(this->addons->GetType());
        this->loadInfo();
    }
    else if(eventSource == "previous")
    {
        this->addons->PreviousType(this->addons->GetType());
        this->loadInfo();
    }
    if(eventSource == "install")
    {
        m_progress = new ProgressBarWidget();
        m_progress->m_x = 180;
        m_progress->m_y = m_area.getHeight()-45;
        m_progress->m_w = 250;
        m_progress->m_h = 35;
        m_progress->setParent(m_irrlicht_window);

        m_widgets.push_back(m_progress);
        m_progress->add();

        /*This widget will show some text as "downloading..." or "installing".*/
        m_state = new LabelWidget();
        m_state->m_properties[PROP_TEXT_ALIGN] = "center";
        /* Center the widget*/
        m_state->m_x = 10;
        m_state->m_y = getHeight()-125;
        m_state->m_w = getWidth() - 20;
        m_state->m_h = 35;
        m_state->setParent(m_irrlicht_window);

        m_widgets.push_back(m_state);
        m_state->add();

        this->getWidget<ButtonWidget>("back")->setDeactivated();
        //FIXME : re-implement this buttons
        /*
        m_next->setDeactivated();
        m_previous->setDeactivated();
        */
        this->getWidget<ButtonWidget>("install")->setDeactivated();
        m_percent_update = true;
        pthread_t thread;
        pthread_create(&thread, NULL, &AddonsLoading::startInstall, this);
    }
    return GUIEngine::EVENT_LET;
}
// ------------------------------------------------------------------------------------------------------
void AddonsLoading::onUpdate(float delta)
{

    pthread_mutex_lock(&(m_mutex_can_install));
    if(m_can_install)
    {
        this->close();
    }
    if(m_percent_update)
    {
        m_progress->setValue(addons->getDownloadState());
        m_state->setText(addons->getDownloadStateAsStr().c_str());
    }
    pthread_mutex_unlock(&(m_mutex_can_install));
}

// ------------------------------------------------------------------------------------------------------
void AddonsLoading::close()
{

    GUIEngine::Screen* curr_screen = GUIEngine::getCurrentScreen();
	pthread_mutex_lock(&(((AddonsScreen*)curr_screen)->mutex));
	((AddonsScreen*)curr_screen)->can_load_list = true;
	pthread_mutex_unlock(&(((AddonsScreen*)curr_screen)->mutex));
    dismiss();
}
// ------------------------------------------------------------------------------------------------------

void * AddonsLoading::startInstall(void* pthis)
{
    AddonsLoading * obj = (AddonsLoading*)pthis;
    if(obj->addons->IsInstalled() == "yes")
    {
        obj->addons->UnInstall();
    }
    else
    {
        obj->addons->Install();
    }
    pthread_mutex_lock(&(obj->m_mutex_can_install));
    obj->m_can_install = true;
    obj->m_percent_update = false;
    pthread_mutex_unlock(&(obj->m_mutex_can_install));
    return NULL;
}
// ------------------------------------------------------------------------------------------------------

void * AddonsLoading::downloadIcon(void* pthis)
{
/*
    AddonsLoading * obj = (AddonsLoading*)pthis;
    download("icon/" + obj->addons->GetIcon(), obj->addons->GetName() + ".png");
    obj->icon->setImage(std::string(file_manager->getConfigDir() + "/" +  obj->addons->GetName() + ".png").c_str(), IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    obj->icon->setImage(std::string(file_manager->getConfigDir() + "/" +  obj->addons->GetName() + ".png").c_str(), IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    obj->icon->x = 0;
    obj->icon->y = 0;
    obj->icon->w = obj->m_area.getWidth()/2;
    obj->icon->h = obj->m_area.getHeight();
    obj->icon->setParent(obj->m_irrlicht_window);
    obj->m_children.push_back(obj->icon);
    obj->icon->add();*/
    return NULL;
}
#endif
