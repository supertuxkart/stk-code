//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Glenn De Jonghe
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

#ifndef HEADER_SERVER_SELECTION_HPP
#define HEADER_SERVER_SELECTION_HPP

#include "guiengine/screen.hpp"
#include "guiengine/widgets.hpp"

namespace Online { class XMLRequest; }

namespace GUIEngine { class Widget; }

/**
  * \brief ServerSelection
  * \ingroup states_screens
  */
class ServerSelection :  public GUIEngine::Screen,
                         public GUIEngine::ScreenSingleton<ServerSelection>,
                         public GUIEngine::IListWidgetHeaderListener
{
    friend class GUIEngine::ScreenSingleton<ServerSelection>;

private:
    ServerSelection();
    ~ServerSelection();

    GUIEngine::IconButtonWidget *               m_reload_widget;
    GUIEngine::LabelWidget *                    m_update_status;
    GUIEngine::ListWidget *                     m_server_list_widget;

    /** \brief To check (and set) if sort order is descending **/
    bool                                        m_sort_desc;

    /** A pointer to the http request for getting a server list. */
    const Online::XMLRequest *m_refresh_request;


public:

    void refresh();

    /** Load the addons into the main list.*/
    void loadList();

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void beforeAddingWidget() OVERRIDE;

    virtual void onColumnClicked(int columnId) OVERRIDE;

    virtual void init() OVERRIDE;

    virtual void tearDown() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void onUpdate(float dt) OVERRIDE;

};   // ServerSelection

#endif
