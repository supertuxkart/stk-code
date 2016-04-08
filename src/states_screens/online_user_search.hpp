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

#ifndef HEADER_ONLINE_USER_SEARCH_HPP
#define HEADER_ONLINE_USER_SEARCH_HPP

#include "guiengine/screen.hpp"
#include "guiengine/widgets.hpp"
#include "online/online_profile.hpp"
#include "online/request.hpp"
#include "utils/ptr_vector.hpp"

namespace GUIEngine { class Widget; }

/**
  * \brief
  * \ingroup
  */
class OnlineUserSearch :    public GUIEngine::Screen,
                            public GUIEngine::ScreenSingleton<OnlineUserSearch>
{
    friend class GUIEngine::ScreenSingleton<OnlineUserSearch>;

private:
    OnlineUserSearch();
    ~OnlineUserSearch();

    /** Pointer to the back widget. */
    GUIEngine::IconButtonWidget *               m_back_widget;
    /** Pointer to the search button. */
    GUIEngine::ButtonWidget *                   m_search_button_widget;
    /** Pointer to the search box. */
    GUIEngine::TextBoxWidget *                  m_search_box_widget;
    /** Pointer to the result list. */
    GUIEngine::ListWidget *                     m_user_list_widget;

    /** Seach string entered in the search widget. */
    irr::core::stringw                          m_search_string;
    /** Last search string, used to avoid doing the same search again. */
    irr::core::stringw                          m_last_search_string;

    /** The list of all IDs found. */
    Online::OnlineProfile::IDList               m_users;

    /** The online request to search for users. */
    Online::XMLRequest *m_search_request;

    void parseResult(const XMLNode * input);
    void showList();
    void search();


public:

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void beforeAddingWidget() OVERRIDE;

    virtual void init() OVERRIDE;

    virtual void tearDown() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void onUpdate(float dt) OVERRIDE;

    /** Sets the search string to an initial value. */
    void setSearchString(const irr::core::stringw & search_string)
    {
        m_search_string = search_string;
    }

};

#endif
