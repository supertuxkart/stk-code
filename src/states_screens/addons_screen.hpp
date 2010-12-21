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

#ifndef HEADER_ADDONS_SCREEN_HPP
#define HEADER_ADDONS_SCREEN_HPP

#include <pthread.h>
#include "irrlicht.h"

#include "addons/addons.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "states_screens/dialogs/addons_loading.hpp"

/* used for the installed/unsinstalled icons*/
namespace irr { namespace gui { class STKModifiedSpriteBank; } }

namespace GUIEngine { class Widget; }

/**
  * \brief Addons screen
  * \ingroup states_screens
  */
class AddonsScreen : public GUIEngine::Screen, 
                     public GUIEngine::ScreenSingleton<AddonsScreen>
{
    friend class GUIEngine::ScreenSingleton<AddonsScreen>;

    AddonsScreen();
    Addons          *m_addons;
    AddonsLoading   *m_load;
    void             loadInformations();
    /** For the addons list, a package when it is installed. */
    irr::gui::STKModifiedSpriteBank
                    *m_icon_bank;
    GUIEngine::LabelWidget
                    *m_update_status;

public:

    bool                    m_can_load_list;
    pthread_mutex_t         m_mutex;
    std::string             m_type;

    /** Load the addons into the main list.*/
    void loadList();

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile();

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name, const int playerID);

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init();

    friend void *startInstall(void *);

    static void *downloadList(void *);

    /** This function is used to handle the thread (load the new list, etc...). */
    virtual void onUpdate(float delta,  irr::video::IVideoDriver*);
};

#endif
#endif
