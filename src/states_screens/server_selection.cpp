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

#include "states_screens/server_selection.hpp"

#include "audio/sfx_manager.hpp"
#include "guiengine/modaldialog.hpp"
#include "network/servers_manager.hpp"
#include "online/xml_request.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/dialogs/server_info_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"
#include "utils/string_utils.hpp"

#include <iostream>
#include <assert.h>

using namespace Online;

DEFINE_SCREEN_SINGLETON( ServerSelection );

// ----------------------------------------------------------------------------
/** Constructor, which loads the stkgui file.
 */
ServerSelection::ServerSelection() : Screen("online/server_selection.stkgui")
{
    m_refresh_request = NULL;
}   // ServerSelection

// ----------------------------------------------------------------------------
/** Destructor.
 */
ServerSelection::~ServerSelection()
{
}   // ServerSelection

// ----------------------------------------------------------------------------
/** Clean up.
 */
void ServerSelection::tearDown()
{
    // If the refresh request is being executed, and stk should exit,
    // then a crash can happen when the refresh request is deleted, but
    // then the request manager tries to activate this request. So only
    // delete this request if it is finished.
    if(m_refresh_request && m_refresh_request->isDone())
        delete m_refresh_request;
}   // tearDown

// ----------------------------------------------------------------------------
/** Requests the servers manager to update its list of servers, and disables
 *  the 'refresh' button (till the refresh was finished).
 */
void ServerSelection::refresh()
{
    m_refresh_request = ServersManager::get()->getRefreshRequest();
    // If the request was created (i.e. no error, and not re-requested within
    // 5 seconds), clear the list and display the waiting message:
    if(m_refresh_request)
    {
        m_server_list_widget->clear();
        m_server_list_widget->addItem("spacer", L"");
        m_server_list_widget->addItem("loading",
                              StringUtils::loadingDots(_("Fetching servers")));
        m_reload_widget->setActive(false);
    }
}   // refresh

// ----------------------------------------------------------------------------
/** Set pointers to the various widgets.
 */
void ServerSelection::loadedFromFile()
{
    m_reload_widget = getWidget<GUIEngine::IconButtonWidget>("reload");
    assert(m_reload_widget != NULL);
    m_server_list_widget = getWidget<GUIEngine::ListWidget>("server_list");
    assert(m_server_list_widget != NULL);
    m_server_list_widget->setColumnListener(this);
}   // loadedFromFile

// ----------------------------------------------------------------------------
/** Clear the server list, which will be reloaded. 
 */
void ServerSelection::beforeAddingWidget()
{
    m_server_list_widget->clearColumns();
    m_server_list_widget->addColumn( _("Name"), 2 );
    m_server_list_widget->addColumn( _("Players"), 1);
    m_server_list_widget->addColumn(_("Difficulty"), 1);
    m_server_list_widget->addColumn(_("Game mode"), 1);
}   // beforeAddingWidget

// ----------------------------------------------------------------------------
/** Triggers a refresh of the server list.
 */
void ServerSelection::init()
{
    Screen::init();
    m_sort_desc = true;

    // Set the default sort order
    Server::setSortOrder(Server::SO_NAME);

    /** Triggers the loading of the server list in the servers manager. */
    refresh();
}   // init

// ----------------------------------------------------------------------------
/** Loads the list of all servers. The gui element will be
 *  updated.
 *  \param type Must be 'kart' or 'track'.
 */
void ServerSelection::loadList()
{
    m_server_list_widget->clear();
    ServersManager *manager = ServersManager::get();
    manager->sort(m_sort_desc);
    for(int i=0; i <  manager->getNumServers(); i++)
    {
        const Server *server = manager->getServerBySort(i);
        core::stringw num_players;
        num_players.append(StringUtils::toWString(server->getCurrentPlayers()));
        num_players.append("/");
        num_players.append(StringUtils::toWString(server->getMaxPlayers()));
        std::vector<GUIEngine::ListWidget::ListCell> row;
        row.push_back(GUIEngine::ListWidget::ListCell(server->getName(),-1,3));
        row.push_back(GUIEngine::ListWidget::ListCell(num_players,-1,1,true));

        core::stringw difficulty = race_manager->getDifficultyName(server->getDifficulty());
        row.push_back(GUIEngine::ListWidget::ListCell(difficulty, -1, 1, true));

        core::stringw mode = RaceManager::getNameOf(server->getRaceMinorMode());
        row.push_back(GUIEngine::ListWidget::ListCell(mode, -1, 1, true));

        m_server_list_widget->addItem("server", row);
    }
}   // loadList

// ----------------------------------------------------------------------------
/** Change the sort order if a column was clicked.
 *  \param column_id ID of the column that was clicked.
 */
void ServerSelection::onColumnClicked(int column_id)
{
    switch(column_id)
    {
        case 0: Server::setSortOrder(Server::SO_NAME); break;
        case 1: Server::setSortOrder(Server::SO_PLAYERS); break;
        default: assert(0); break;
    }   // switch
    /** \brief Toggle the sort order after column click **/
    m_sort_desc = !m_sort_desc;
    loadList();
}   // onColumnClicked

// ----------------------------------------------------------------------------
void ServerSelection::eventCallback( GUIEngine::Widget* widget,
                                     const std::string& name,
                                     const int playerID)
{
    if (name == "back")
    {
        StateManager::get()->escapePressed();
    }

    else if (name == "reload")
    {
        refresh();
    }

    else if (name == m_server_list_widget->m_properties[GUIEngine::PROP_ID])
    {
        int selected_index = m_server_list_widget->getSelectionID();
        // This can happen e.g. when the list is empty and the user
        // clicks somewhere.
        if(selected_index >= ServersManager::get()->getNumServers() ||
           selected_index<0                                           )
        {
            return;
        }

        const Server *server = 
            ServersManager::get()->getServerBySort(selected_index);
        uint32_t server_id = server->getServerId();
        uint32_t host_id = server->getHostId();
        new ServerInfoDialog(server_id, host_id);
    }   // click on server

}   // eventCallback

// ----------------------------------------------------------------------------
/** If a refresh of the server list was requested, check if it is finished and
 *  if so, update the list of servers.
 */
void ServerSelection::onUpdate(float dt)
{
    if (!m_refresh_request) return;

    if (m_refresh_request->isDone())
    {
        if (m_refresh_request->isSuccess())
        {
            loadList();
        }
        else
        {
            SFXManager::get()->quickSound("anvil");
            new MessageDialog(m_refresh_request->getInfo());
			m_server_list_widget->clear();
        }
        delete m_refresh_request;
        m_refresh_request = NULL;
        m_reload_widget->setActive(true);
    }
    else
    {
        m_server_list_widget->renameItem("loading",
                              StringUtils::loadingDots(_("Fetching servers")));
    }
}   // onUpdate
