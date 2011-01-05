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

#include <pthread.h>

#include "addons/addon.hpp"
#include "addons/addons_manager.hpp"
#include "guiengine/widgets.hpp"
#include "guiengine/modaldialog.hpp"
#include "utils/synchronised.hpp"

class AddonsLoading : public GUIEngine::ModalDialog
{
//virtual void escapePressed() {};
private:
    GUIEngine::LabelWidget       *m_author;
    GUIEngine::LabelWidget       *m_state;
    GUIEngine::ProgressBarWidget *m_progress;
    GUIEngine::ButtonWidget      *m_back_button;
    GUIEngine::ButtonWidget      *m_install_button;
    GUIEngine::IconButtonWidget  *m_icon;
    GUIEngine::IconButtonWidget  *m_next;
    GUIEngine::IconButtonWidget  *m_previous;
    
    /** The addon to load. */
    Addon                         m_addon;
    void startInstall();
    void endInstall();

    /**
     * This function handle the downllading of the addons icon.
     * It is started using a thread. When it is ended, it change the flag
     * 'm_can_load_icon' and the onUpdate function reload the icon
     * */
    static void * downloadIcon(void*);
    
    /* These three bool are some flags.
     * m_can_install : when the installation is finidhed, onUpdate close the
     * dialog.
     * m_percent_update : to reload the download percent */
    bool m_can_install;
    bool m_percent_update;

    /** True if the icon was successfully downloaded. */
    enum IconState {ICON_NOT_LOADED, ICON_LOADED, ICON_SHOWN};
	Synchronised<IconState> m_icon_loaded;

public:
    /**
     * Creates a modal dialog with given percentage of screen width and height
     */
    AddonsLoading(const float percent_width, const float percent_height,
                  const std::string &addon_name);
    GUIEngine::EventPropagation processEvent(const std::string& event_source);
    
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
