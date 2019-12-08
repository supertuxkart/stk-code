//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Lucas Baudin, Joerg Henrichs
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

#include "states_screens/online/online_user_search.hpp"

#include "audio/sfx_manager.hpp"
#include "config/player_manager.hpp"
#include "guiengine/modaldialog.hpp"
#include "online/profile_manager.hpp"
#include "online/xml_request.hpp"
#include "states_screens/dialogs/user_info_dialog.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"
#include "utils/string_utils.hpp"

using namespace Online;

#include <assert.h>
#include <iostream>

// ----------------------------------------------------------------------------

OnlineUserSearch::OnlineUserSearch() : Screen("online/user_search.stkgui")
{
    m_search_string      = "";
    m_last_search_string = "";
}   // OnlineUserSearch

// ----------------------------------------------------------------------------

OnlineUserSearch::~OnlineUserSearch()
{
}   // OnlineUserSearch

// ----------------------------------------------------------------------------
/** Callback when the xml file was loaded.
 */
void OnlineUserSearch::loadedFromFile()
{
    m_back_widget = getWidget<GUIEngine::IconButtonWidget>("back");
    assert(m_back_widget != NULL);
    m_search_button_widget = getWidget<GUIEngine::ButtonWidget>("search_button");
    assert(m_search_button_widget != NULL);
    m_search_box_widget = getWidget<GUIEngine::TextBoxWidget>("search_box");
    assert(m_search_box_widget != NULL);
    m_user_list_widget = getWidget<GUIEngine::ListWidget>("user_list");
    assert(m_user_list_widget != NULL);
}   // loadedFromFile

// ----------------------------------------------------------------------------
/** Callback before widgets are added. Clears all widgets.
 */
void OnlineUserSearch::beforeAddingWidget()
{
    m_user_list_widget->clearColumns();
    m_user_list_widget->addColumn(_("Username"), 3);
}
// ----------------------------------------------------------------------------
/** Called when entering this menu (before widgets are added).
 */
void OnlineUserSearch::init()
{
    Screen::init();
    search();
    m_search_box_widget->setText(m_search_string);
}   // init

// ----------------------------------------------------------------------------
/** Callback before the screen is removed.
 */
void OnlineUserSearch::tearDown()
{
    // Cancel the work in progress request when leaving the screen
    if (m_search_request)
    {
        if (!m_search_request->isDone())
        {
            m_search_request->cancel();
        }
        m_search_request = nullptr;
    }   // if m_search_request
}   // tearDown


// ----------------------------------------------------------------------------
/** Adds the results of the query to the ProfileManager cache.
 *  \param input The XML node with all user data.
 */
void OnlineUserSearch::parseResult(const XMLNode * input)
{
    m_users.clear();
    const XMLNode * users_xml = input->getNode("users");
    if (users_xml == NULL)
    {
        Log::warn("OnlineSearch", "No users in server response.");
        return;
    }

    // Try to reserve enough cache space for all found entries.
    unsigned int n = ProfileManager::get()
                   ->guaranteeCacheSize(users_xml->getNumNodes());

    if (n >= users_xml->getNumNodes())
    {
        n = users_xml->getNumNodes();
    }
    else
    {
        Log::warn("OnlineSearch",
            "Too many results found, only %d will be displayed.", n);
    }

    for (unsigned int i = 0; i < n; i++)
    {
        OnlineProfile * profile = new OnlineProfile(users_xml->getNode(i));

        // The id must be pushed before adding it to the cache, since
        // the cache might merge the new data with an existing entry
        m_users.push_back(profile->getID());
        ProfileManager::get()->addToCache(profile);
    } // for i = 0 ... number of display users
}   // parseResult

// ----------------------------------------------------------------------------
/** Takes the list of user ids from a query and shows it in the list gui.
 */
void OnlineUserSearch::showList()
{
    m_user_list_widget->clear();

    for (unsigned int i = 0; i < m_users.size(); i++)
    {
        std::vector<GUIEngine::ListWidget::ListCell> row;
        OnlineProfile * profile = ProfileManager::get()->getProfileByID(m_users[i]);

        // This could still happen if something pushed results out of the cache.
        if (!profile)
        {
            Log::warn("OnlineSearch", "User %d not in cache anymore, ignored.", m_users[i]);
            continue;
        }

        row.push_back(GUIEngine::ListWidget::ListCell(profile->getUserName(),-1,3));
        m_user_list_widget->addItem("user", row);
    }
}   // showList

// ----------------------------------------------------------------------------
/** Called when a search is triggered. When it is a new search (and not just
 *  searching for the same string again), a request will be queued to
 *  receive the search results
 */
void OnlineUserSearch::search()
{
    if (m_search_string != "" && m_last_search_string != m_search_string)
    {
        m_search_request = std::make_shared<XMLRequest>();
        PlayerManager::setUserDetails(m_search_request, "user-search");
        m_search_request->addParameter("search-string", m_search_string);
        m_search_request->queue();

        m_user_list_widget->clear();
        m_user_list_widget->addItem("spacer", L"");
        m_user_list_widget->addItem("loading", StringUtils::loadingDots(_("Searching")));
        m_back_widget->setActive(false);
        m_search_box_widget->setActive(false);
        m_search_button_widget->setActive(false);
    }
}   // search


// ----------------------------------------------------------------------------
/** Called when an event occurs (i.e. user clicks on something).
 */
void OnlineUserSearch::eventCallback(GUIEngine::Widget* widget,
                                     const std::string& name,
                                     const int player_id)
{
    if (name == m_back_widget->m_properties[GUIEngine::PROP_ID])
    {
        StateManager::get()->escapePressed();
    }
    else if (name == m_user_list_widget->m_properties[GUIEngine::PROP_ID])
    {
        int selected_index = m_user_list_widget->getSelectionID();
        if (selected_index != -1 && selected_index < (int)m_users.size())
            new UserInfoDialog(m_users[selected_index]);
    }
    else if (name == m_search_button_widget->m_properties[GUIEngine::PROP_ID])
    {
        m_last_search_string = m_search_string;
        m_search_string = m_search_box_widget->getText().trim();
        search();
    }

}   // eventCallback

// ----------------------------------------------------------------------------
/** Called every frame. It queries the search request for results and
 *  displays them if necessary.
 */
void OnlineUserSearch::onUpdate(float dt)
{
    if (m_search_request)
    {
        if(m_search_request->isDone())
        {
            if(m_search_request->isSuccess())
            {
                parseResult(m_search_request->getXMLData());
                showList();
            }
            else
            {
                SFXManager::get()->quickSound( "anvil" );
                new MessageDialog(m_search_request->getInfo());
            }

            m_search_request = nullptr;
            m_back_widget->setActive(true);
            m_search_box_widget->setActive(true);
            m_search_button_widget->setActive(true);
        }
        else
        {
            m_user_list_widget->renameItem("loading",
                                    StringUtils::loadingDots(_("Searching")) );
        }
    }
}   // onUpdate
