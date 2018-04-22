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
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/modaldialog.hpp"
#include "network/network_config.hpp"
#include "network/server.hpp"
#include "network/servers_manager.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/dialogs/server_info_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"
#include "utils/string_utils.hpp"

#include <algorithm>
#include <cassert>

using namespace Online;

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
    m_servers.clear();
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
    m_private_server = getWidget<GUIEngine::CheckBoxWidget>("private_server");
    assert(m_private_server != NULL);
    m_private_server->setState(false);
}   // loadedFromFile

// ----------------------------------------------------------------------------
/** Clear the server list, which will be reloaded. 
 */
void ServerSelection::beforeAddingWidget()
{
    m_server_list_widget->clearColumns();
    m_server_list_widget->addColumn( _("Name"), 3);
    m_server_list_widget->addColumn( _("Players"), 1);
    m_server_list_widget->addColumn(_("Difficulty"), 1);
    m_server_list_widget->addColumn(_("Game mode"), 2);
    if (NetworkConfig::get()->isWAN())
    {
        // I18N: In server selection screen, owner of server, only displayed
        // if it's localhost or friends'
        m_server_list_widget->addColumn(_("Owner"), 1);
        // I18N: In server selection screen, distance to server
        m_server_list_widget->addColumn(_("Distance (km)"), 1);
    }
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
    std::sort(m_servers.begin(), m_servers.end(), [sort_case, this]
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
                return c->getServerMode() > d->getServerMode();
                break;
            case 4:
                return c->getServerOwnerName() > d->getServerOwnerName();
                break;
            case 5:
                return c->getDistance() > d->getDistance();
                break;
            }   // switch
            assert(false);
            return false;
        });
    for (auto server : m_servers)
    {
        core::stringw num_players;
        num_players.append(StringUtils::toWString(server->getCurrentPlayers()));
        num_players.append("/");
        num_players.append(StringUtils::toWString(server->getMaxPlayers()));
        std::vector<GUIEngine::ListWidget::ListCell> row;
        row.push_back(GUIEngine::ListWidget::ListCell(server->getName(), -1, 3));
        row.push_back(GUIEngine::ListWidget::ListCell(num_players, -1, 1, true));

        core::stringw difficulty =
            race_manager->getDifficultyName(server->getDifficulty());
        row.push_back(GUIEngine::ListWidget::ListCell(difficulty, -1, 1, true));

        core::stringw mode =
            NetworkConfig::get()->getModeName(server->getServerMode());
        row.push_back(GUIEngine::ListWidget::ListCell(mode, -1, 2, true));

        if (NetworkConfig::get()->isWAN())
        {
            row.push_back(GUIEngine::ListWidget::ListCell(StringUtils::
                utf8ToWide(server->getServerOwnerName()), -1, 1, true));
            // I18N: In server selection screen, unknown distance to server
            core::stringw distance = _("Unknown");
            if (!(server->getDistance() < 0.0f))
                distance = StringUtils::toWString(server->getDistance());
            row.push_back(GUIEngine::ListWidget::ListCell(distance, -1, 1,
                true));
        }
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
    else if (name == "private_server")
    {
        copyFromServersManager();
    }
    else if (name == m_server_list_widget->m_properties[GUIEngine::PROP_ID])
    {
        int selected_index = m_server_list_widget->getSelectionID();
        // This can happen e.g. when the list is empty and the user
        // clicks somewhere.
        if (selected_index < 0 || m_refreshing_server ||
            selected_index >= (int)m_servers.size())
        {
            return;
        }
        new ServerInfoDialog(m_servers[selected_index]);
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
        !m_servers.empty())
    {
        ServerInfoDialog *sid = new ServerInfoDialog(m_servers[0]);
        sid->requestJoin();
    }

    if (!m_refreshing_server) return;

    if (ServersManager::get()->listUpdated())
    {
        m_refreshing_server = false;
        if (!ServersManager::get()->getServers().empty())
        {
            int selection = m_server_list_widget->getSelectionID();
            std::string selection_str = m_server_list_widget
                ->getSelectionInternalName();
            copyFromServersManager();
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

// ----------------------------------------------------------------------------
void ServerSelection::copyFromServersManager()
{
    m_servers = ServersManager::get()->getServers();
    if (m_servers.empty())
        return;
    m_servers.erase(std::remove_if(m_servers.begin(), m_servers.end(),
        [this](const std::shared_ptr<Server> a)->bool
        {
            return a->isPasswordProtected() != m_private_server->getState();
        }), m_servers.end());
    loadList(0);
}   // copyFromServersManager
