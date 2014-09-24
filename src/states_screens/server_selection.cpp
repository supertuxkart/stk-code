//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Lucas Baudin, Joerg Henrichs
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

ServerSelection::ServerSelection() : Screen("online/server_selection.stkgui")
{
    m_selected_index = -1;
    m_refresh_request = NULL;

}   // ServerSelection

// ----------------------------------------------------------------------------

ServerSelection::~ServerSelection()
{
}   // ServerSelection

// ----------------------------------------------------------------------------

void ServerSelection::tearDown()
{
    delete m_refresh_request;
}   // tearDown


// ----------------------------------------------------------------------------

void ServerSelection::refresh()
{
    m_refresh_request = ServersManager::get()->refreshRequest();
    m_fake_refresh = (m_refresh_request == NULL ? true : false);
    m_server_list_widget->clear();
    m_server_list_widget->addItem("spacer", L"");
    m_server_list_widget->addItem("loading",
                              StringUtils::loadingDots(_("Fetching servers")));
    m_reload_widget->setDeactivated();
}


// ----------------------------------------------------------------------------

void ServerSelection::loadedFromFile()
{
    m_back_widget = getWidget<GUIEngine::IconButtonWidget>("back");
    assert(m_back_widget != NULL);
    m_reload_widget = getWidget<GUIEngine::IconButtonWidget>("reload");
    assert(m_reload_widget != NULL);
    m_server_list_widget = getWidget<GUIEngine::ListWidget>("server_list");
    assert(m_server_list_widget != NULL);
    m_server_list_widget->setColumnListener(this);
}   // loadedFromFile


// ----------------------------------------------------------------------------

void ServerSelection::beforeAddingWidget()
{
    m_server_list_widget->clearColumns();
    m_server_list_widget->addColumn( _("Name"), 3 );
    m_server_list_widget->addColumn( _("Players"), 1);
}
// ----------------------------------------------------------------------------

void ServerSelection::init()
{
    Screen::init();
    m_sort_desc = true;


    // Set the default sort order
    Server::setSortOrder(Server::SO_NAME);
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
    ServersManager * manager = ServersManager::get();
    manager->sort(m_sort_desc);
    for(int i=0; i <  manager->getNumServers(); i++)
    {
        const Server * server = manager->getServerBySort(i);
        core::stringw num_players;
        num_players.append(StringUtils::toWString(server->getCurrentPlayers()));
        num_players.append("/");
        num_players.append(StringUtils::toWString(server->getMaxPlayers()));
        std::vector<GUIEngine::ListWidget::ListCell> row;
        row.push_back(GUIEngine::ListWidget::ListCell(server->getName(),-1,3));
        row.push_back(GUIEngine::ListWidget::ListCell(num_players,-1,1,true));
        m_server_list_widget->addItem("server", row);
    }
}   // loadList

// ----------------------------------------------------------------------------
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
        m_selected_index = m_server_list_widget->getSelectionID();
        uint32_t server_id = ServersManager::get()->getServerBySort(m_selected_index)->getServerId();
        uint32_t host_id = ServersManager::get()->getServerBySort(m_selected_index)->getHostId();
        new ServerInfoDialog(server_id, host_id);
    }

}   // eventCallback

// ----------------------------------------------------------------------------
/** Selects the last selected item on the list (which is the item that
 *  is just being installed) again. This function is used from the
 *  addons_loading screen: when it is closed, it will reset the
 *  select item so that people can keep on installing from that
 *  point on.
*/
void ServerSelection::setLastSelected()
{
    if(m_selected_index>-1)
    {
        m_server_list_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
        m_server_list_widget->setSelectionID(m_selected_index);
    }
}   // setLastSelected

// ----------------------------------------------------------------------------

void ServerSelection::onUpdate(float dt)
{
    if(m_refresh_request != NULL)
    {
        if(m_refresh_request->isDone())
        {
            if(m_refresh_request->isSuccess())
            {
                loadList();
            }
            else
            {
                SFXManager::get()->quickSound( "anvil" );
                new MessageDialog(m_refresh_request->getInfo());
            }
            delete m_refresh_request;
            m_refresh_request = NULL;
            m_reload_widget->setActivated();
        }
        else
        {
            m_server_list_widget->renameItem("loading",
                              StringUtils::loadingDots(_("Fetching servers")));
        }
    }
    else if(m_fake_refresh)
    {
        loadList();
        m_fake_refresh = false;
        m_reload_widget->setActivated();
    }
}   // onUpdate
