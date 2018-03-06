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
#include "network/network_config.hpp"
#include "network/server.hpp"
#include "network/servers_manager.hpp"
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
    m_refreshing_server = false;
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
    ServersManager::get()->cleanUpServers();
    m_server_list_widget->clear();
}   // tearDown

// ----------------------------------------------------------------------------
/** Requests the servers manager to update its list of servers, and disables
 *  the 'refresh' button (till the refresh was finished).
 */
void ServerSelection::refresh()
{
    // If the request was created (i.e. no error, and not re-requested within
    // 5 seconds), clear the list and display the waiting message:
    if (ServersManager::get()->refresh())
    {
        m_server_list_widget->clear();
        m_reload_widget->setActive(false);
        m_refreshing_server = true;
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
    /** Triggers the loading of the server list in the servers manager. */
    refresh();
}   // init

// ----------------------------------------------------------------------------
/** Loads the list of all servers. The gui element will be
 *  updated.
 *  \param sort_case what sorting method will be used.
 */
void ServerSelection::loadList(unsigned sort_case)
{
    m_server_list_widget->clear();
    ServersManager::get()->sortServers([sort_case, this]
        (const std::shared_ptr<Server> a,
         const std::shared_ptr<Server> b)->bool
        {
            std::shared_ptr<Server> c = m_sort_desc ? a : b;
            std::shared_ptr<Server> d = m_sort_desc ? b : a;
            switch (sort_case)
            {
            case 0:
                return c->getLowerCaseName() > d->getLowerCaseName();
                break;
            case 1:
                return c->getCurrentPlayers() > d->getCurrentPlayers();
                break;
            case 2:
                return c->getDifficulty() > d->getDifficulty();
                break;
            case 3:
                return c->getRaceMinorMode() > d->getRaceMinorMode();
                break;
            }   // switch
            assert(false);
            return false;
        });
    for (auto server : ServersManager::get()->getServers())
    {
        core::stringw num_players;
        num_players.append(StringUtils::toWString(server->getCurrentPlayers()));
        num_players.append("/");
        num_players.append(StringUtils::toWString(server->getMaxPlayers()));
        std::vector<GUIEngine::ListWidget::ListCell> row;
        row.push_back(GUIEngine::ListWidget::ListCell(server->getName(),-1,3));
        row.push_back(GUIEngine::ListWidget::ListCell(num_players,-1,1,true));

        core::stringw difficulty =
            race_manager->getDifficultyName(server->getDifficulty());
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
    m_sort_desc = !m_sort_desc;
    loadList(column_id);
}   // onColumnClicked

// ----------------------------------------------------------------------------
void ServerSelection::eventCallback(GUIEngine::Widget* widget,
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
        if (selected_index < 0 || m_refreshing_server ||
            selected_index >= (int)ServersManager::get()->getServers().size())
        {
            return;
        }
        new ServerInfoDialog(
            ServersManager::get()->getServers()[selected_index]);
    }   // click on server

}   // eventCallback

// ----------------------------------------------------------------------------
/** If a refresh of the server list was requested, check if it is finished and
 *  if so, update the list of servers.
 */
void ServerSelection::onUpdate(float dt)

{
    // In case of auto-connect command line parameter, select the first server asap
    if (NetworkConfig::get()->isAutoConnect() &&
        m_refreshing_server == false          &&
        !ServersManager::get()->getServers().empty())
    {
        ServerInfoDialog *sid = new ServerInfoDialog(ServersManager::get()->getServers()[0]);
        sid->requestJoin();
    }

    if (!m_refreshing_server) return;

    if (ServersManager::get()->listUpdated())
    {
        m_refreshing_server = false;
        if (!ServersManager::get()->getServers().empty())
        {
            int selection = m_server_list_widget->getSelectionID();
            std::string selection_str = m_server_list_widget->getSelectionInternalName();
            loadList(0);
            // restore previous selection
            if (selection != -1 && selection_str != "loading")
                m_server_list_widget->setSelectionID(selection);
        }
        else
        {
            SFXManager::get()->quickSound("anvil");
            new MessageDialog(_("No server is available."));
            m_server_list_widget->clear();
        }
        m_reload_widget->setActive(true);
    }
    else
    {
        m_server_list_widget->clear();
        m_server_list_widget->addItem("loading",
            StringUtils::loadingDots(_("Fetching servers")));
    }

}   // onUpdate
