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

// ----------------------------------------------------------------------------
AddonsLoading::AddonsLoading(const float w, const float h,
                             const std::string &id)
             : ModalDialog(w, h)
             , m_icon_loaded(ICON_NOT_LOADED)
{
    loadFromFile("addons_view_dialog.stkgui");

    m_addon          = *(addons_manager->getAddon(id));
    m_progress       = NULL;
    m_can_install    = false;
    m_percent_update = false;
    pthread_mutex_init(&m_mutex_can_install, NULL);

    /*Init the icon here to be able to load a single image*/
    m_icon = getWidget<IconButtonWidget>("icon");
    
    if(m_addon.isInstalled())
    {
        if(m_addon.needsUpdate())
            getWidget<ButtonWidget>("install")->setLabel(_("Update"));
        else
            getWidget<ButtonWidget>("install")->setLabel(_("Uninstall"));
    }
    
    core::stringw name = StringUtils::insertValues(_("Name: %i"),
                                                   m_addon.getName().c_str() );
    getWidget<LabelWidget>("name")->setText(name);

    core::stringw desc = StringUtils::insertValues(_("Description: %i"),
                                             m_addon.getDescription().c_str());
    getWidget<LabelWidget>("description")->setText(desc);

    core::stringw version = StringUtils::insertValues(_("Version: %d"),
                                            m_addon.getVersion());
    getWidget<LabelWidget>("version")->setText(version);

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, &AddonsLoading::downloadIcon, this);
}   // AddonsLoading

// ----------------------------------------------------------------------------
void * AddonsLoading::downloadIcon( void * pthis)
{
    AddonsLoading *me     = (AddonsLoading*)pthis;
    std::string icon_name = StringUtils::getBasename(me->m_addon.getIcon());
    std::string icon_path = "icon/" + me->m_addon.getIcon();

	if(network_http->downloadFileSynchron(icon_path, icon_name))
    {
        me->m_icon_loaded.set(ICON_LOADED);
    }
    else
    {
        fprintf(stderr, "[Addons] Download icon '%s' failed\n", 
                icon_path.c_str());
    }
    return NULL;
}   // downloadIcon

// ----------------------------------------------------------------------------

GUIEngine::EventPropagation 
                    AddonsLoading::processEvent(const std::string& eventSource)
{
    if(eventSource == "cancel")
    {
        dismiss();
        return GUIEngine::EVENT_BLOCK;
    }
    else if(eventSource == "next")
    {
        // addons_manager->nextType(addons_manager->getType());
        assert(false);
        //loadInfo();
    }
    else if(eventSource == "previous")
    {
        // FIXME: addons_manager->previousType(addons_manager->getType());
        assert(false);
        //loadInfo();
    }
    if(eventSource == "install")
    {
        assert(m_progress==NULL);
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
        startInstall();
    }
    return GUIEngine::EVENT_LET;
}   // processEvent

// ----------------------------------------------------------------------------
void AddonsLoading::onUpdate(float delta)
{
    if(m_progress)
    {
        float progress = network_http->getProgress();
        m_progress->setValue((int)(progress*100.0f));
        if(progress<0)
        {
            // TODO: show a message in the interface
            fprintf(stderr, "[Addons] Failed to download '%s'\n", 
                    m_addon.getFile().c_str());
            dismiss();
            return;
        }
        else if(progress>=1.0f)
        {
            printf("Download finished.\n");
            endInstall();
            dismiss();
            return;
        }
    }

    // See if the icon is loaded (but not yet displayed)
    if(m_icon_loaded.get()==ICON_LOADED)
    {
        const std::string icon = StringUtils::getBasename(m_addon.getIcon());
        m_icon->setImage( file_manager->getAddonsFile(icon).c_str(),
                          IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE  );
        m_icon_loaded.set(ICON_SHOWN);
    }

    return;

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
    pthread_mutex_unlock(&(m_mutex_can_install));
}   // onUpdate

// ----------------------------------------------------------------------------
void AddonsLoading::close()
{
    AddonsScreen* curr_screen = AddonsScreen::getInstance();
	((AddonsScreen*)curr_screen)->m_can_load_list = true;
    dismiss();
}   // close

// ----------------------------------------------------------------------------
/** This function is called when the user click on 'Install', 'Uninstall', or
 *  'Update'.
 **/
void AddonsLoading::startInstall()
{
    std::string file = "file/" + m_addon.getFile();
    std::string save = StringUtils::getBasename(m_addon.getFile());
    network_http->downloadFileAsynchron(file, save);
}   // startInstall

// ----------------------------------------------------------------------------
void AddonsLoading::endInstall()
{
    if(!m_addon.isInstalled() || m_addon.needsUpdate())
    {
        addons_manager->install(m_addon);
    }
    else
    {
        addons_manager->uninstall(m_addon);
    }
}   // endInstall
#endif
