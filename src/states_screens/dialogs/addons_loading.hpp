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

#ifndef HEADER_ADDONS_LOADING_HPP
#define HEADER_ADDONS_LOADING_HPP

#include "guiengine/modaldialog.hpp"

#include "addons/addons.hpp"
#include "guiengine/widgets.hpp"
#include <pthread.h>
class AddonsLoading : public GUIEngine::ModalDialog
{
//virtual void escapePressed() {};
private:
    GUIEngine::LabelWidget *            name;
    GUIEngine::LabelWidget *            description;
    GUIEngine::LabelWidget *            version;
    GUIEngine::LabelWidget *            author;
    GUIEngine::LabelWidget *            m_state;
    GUIEngine::ProgressBarWidget *      m_progress;
    GUIEngine::ButtonWidget *           m_back_button;
    GUIEngine::ButtonWidget *           install_button;
    GUIEngine::IconButtonWidget *       icon;
    GUIEngine::IconButtonWidget *       m_next;
    GUIEngine::IconButtonWidget *       m_previous;
    
    /**
     * This function is called when the user click on 'Install', 'Uninstall', or
     * 'Update'. It is started using a thread.
     * */
    static void * startInstall(void*);
    /**
     * This function handle the downllading of the addons icon.
     * It is started using a thread. When it is ended, it change the flag
     * 'm_can_load_icon' and the onUpdate function reload the icon
     * */
    static void * downloadIcon(void*);
    void loadInfo();
    
    /* These three bool are some flags.
     * m_can_install : when the installation is finidhed, onUpdate close the
     * dialog.
     * m_percent_update : to reload the download percent
     * m_can_load_icon see above (function downloadIcon)*/
    bool m_can_install;
    bool m_percent_update;
	bool m_can_load_icon;

public:
    /**
     * Creates a modal dialog with given percentage of screen width and height
     */
    pthread_mutex_t m_mutex_can_install;
    AddonsLoading(const float percentWidth, const float percentHeight);
    GUIEngine::EventPropagation processEvent(const std::string& eventSource);
    
    /** This function is called by the GUI, all the frame (or somthing like
     * that). It checks the flags (m_can_install, m_can_load_icon and
     * m_percent_update) and do the necessary.
     * */
    void onUpdate(float delta);
    
    /** To close the dialog when the (un)installation is finished.*/
    void close();
};

#endif
#endif
