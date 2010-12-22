//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2010 Marianne Gagnon, Joerg Henrichs
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

#include "addons/addons_manager.hpp"
#include "addons/network_http.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widgets.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "states_screens/addons_screen.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::gui;

// ------------------------------------------------------------------------------------------------------

AddonsLoading::AddonsLoading(const float w, const float h) :
        ModalDialog(w, h)
{
    loadFromFile("addons_view_dialog.stkgui");
    m_can_install = false;
	m_can_load_icon = false;
    m_percent_update = false;
    pthread_mutex_init(&m_mutex_can_install, NULL);

    /*Init the icon here to be able to load a single image*/
    icon = getWidget<IconButtonWidget>("icon");

    name = getWidget<LabelWidget>("name");

    description = getWidget<LabelWidget>("description");
    
    version = getWidget<LabelWidget>("version");
    
    if(addons_manager->isInstalledAsBool())
    {
        if(addons_manager->getInstalledVersion() < addons_manager->getVersion())
            getWidget<ButtonWidget>("install")->setLabel(_("Update"));
        else
            getWidget<ButtonWidget>("install")->setLabel(_("Uninstall"));
    }
    
    loadInfo();
}
void AddonsLoading::loadInfo()
{
    name->setText(StringUtils::insertValues(_("Name: %i"),
                                            addons_manager->getName().c_str()));
    description->setText(StringUtils::insertValues(_("Description: %i"),
                                            addons_manager->getDescription().c_str()));
    version->setText(StringUtils::insertValues(_("Version: %i"),
                                            addons_manager->getVersionAsStr().c_str()));
    pthread_t thread;
    pthread_create(&thread, NULL, &AddonsLoading::downloadIcon, this);
}

// ------------------------------------------------------------------------------------------------------
void * AddonsLoading::downloadIcon( void * pthis)
{
    AddonsLoading * pt = (AddonsLoading*)pthis;
    
    std::string iconPath = "icon/" + addons_manager->getIcon();
	if (download(iconPath, addons_manager->getName() + ".png"))
    {
        pthread_mutex_lock(&(pt->m_mutex_can_install));
        pt->m_can_load_icon = true;
        pthread_mutex_unlock(&(pt->m_mutex_can_install));
	}
    else
    {
        fprintf(stderr, "[Addons] Download icon '%s' failed\n", iconPath.c_str());
    }
    return NULL;
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
        addons_manager->nextType(addons_manager->getType());
        loadInfo();
    }
    else if(eventSource == "previous")
    {
        addons_manager->previousType(addons_manager->getType());
        loadInfo();
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

        getWidget<ButtonWidget>("cancel")->setDeactivated();
        //FIXME : re-implement this buttons
        /*
        m_next->setDeactivated();
        m_previous->setDeactivated();
        */
        getWidget<ButtonWidget>("install")->setDeactivated();
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
        close();
    }
    if(m_percent_update)
    {
        m_progress->setValue(addons_manager->getDownloadState());
        m_state->setText(addons_manager->getDownloadStateAsStr().c_str());
    }
    if(m_can_load_icon)
    {
		icon->setImage(  (file_manager->getConfigDir() + "/" 
                          +  addons_manager->getName() + ".png").c_str(),
                       IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    }
    pthread_mutex_unlock(&(m_mutex_can_install));
}

// ------------------------------------------------------------------------------------------------------
void AddonsLoading::close()
{

    AddonsScreen* curr_screen = AddonsScreen::getInstance();
	pthread_mutex_lock(&(((AddonsScreen*)curr_screen)->m_mutex));
	((AddonsScreen*)curr_screen)->m_can_load_list = true;
	pthread_mutex_unlock(&(((AddonsScreen*)curr_screen)->m_mutex));
    dismiss();
}
// ------------------------------------------------------------------------------------------------------

void * AddonsLoading::startInstall(void* pthis)
{
    AddonsLoading * obj = (AddonsLoading*)pthis;
    if(!addons_manager->isInstalledAsBool() || addons_manager->needUpdate())
    {
        addons_manager->install();
    }
    else
    {
        addons_manager->uninstall();
    }
    pthread_mutex_lock(&(obj->m_mutex_can_install));
    obj->m_can_install = true;
    obj->m_percent_update = false;
    pthread_mutex_unlock(&(obj->m_mutex_can_install));
    return NULL;
}
#endif
