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

#include <iostream>
#include <assert.h>

#include "guiengine/modaldialog.hpp"
#include "guiengine/widget.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"
#include "online/current_online_user.hpp"


DEFINE_SCREEN_SINGLETON( ServerSelection );

// ----------------------------------------------------------------------------

ServerSelection::ServerSelection() : Screen("server_selection.stkgui")
{
    m_selected_index = -1;
    m_reload_timer = 0.0f;
}   // ServerSelection

// ----------------------------------------------------------------------------

void ServerSelection::loadedFromFile()
{

    m_back_widget = getWidget<GUIEngine::IconButtonWidget>("back");
    m_reload_widget = getWidget<GUIEngine::IconButtonWidget>("reload");

    m_server_list_widget = getWidget<GUIEngine::ListWidget>("server_list");
    assert(m_server_list_widget != NULL);
    m_server_list_widget->setColumnListener(this);
    
    m_servers = CurrentOnlineUser::get()->getServerList();
}   // loadedFromFile


// ----------------------------------------------------------------------------

void ServerSelection::beforeAddingWidget()
{
    m_server_list_widget->clearColumns();
    m_server_list_widget->addColumn( _("Name"), 4 );
    m_server_list_widget->addColumn( _("Players"), 1 );
    m_server_list_widget->addColumn( _("Max"), 1 );
}
// ----------------------------------------------------------------------------

void ServerSelection::init()
{
    Screen::init();
    m_reloading = false;
    m_sort_desc = true;
    m_reload_widget->setActivated();

    // Set the default sort order
    Server::setSortOrder(Server::SO_NAME);
    loadList();
}   // init

// ----------------------------------------------------------------------------

void ServerSelection::unloaded()
{
}

// ----------------------------------------------------------------------------

void ServerSelection::tearDown()
{
}

// ----------------------------------------------------------------------------
/** Loads the list of all servers. The gui element will be
 *  updated.
 *  \param type Must be 'kart' or 'track'.
 */
void ServerSelection::loadList()
{
    // First create a list of sorted entries

    m_servers->insertionSort(/*start=*/0, m_sort_desc);

    m_server_list_widget->clear();

    for(int i=0; i<m_servers->size(); i++)
    {
        core::stringw table_entry;
        table_entry.append((*m_servers)[i].getName());
        table_entry.append("\t");
        table_entry.append((*m_servers)[i].getCurrentPlayers());
        table_entry.append("\t");
        table_entry.append((*m_servers)[i].getMaxPlayers());
        m_server_list_widget->addItem("server", table_entry);

    }
}   // loadList

// ----------------------------------------------------------------------------
void ServerSelection::onColumnClicked(int column_id)
{
    switch(column_id)
    {
        case 0: Server::setSortOrder(Server::SO_NAME); break;
        case 1: Server::setSortOrder(Server::SO_PLAYERS); break;
        case 2: Server::setSortOrder(Server::SO_PLAYERS); break;
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
        if (!m_reloading)
        {
            m_reloading = true;
            m_server_list_widget->clear();

            m_server_list_widget->addItem("spacer", L"");
            m_server_list_widget->addItem("loading", _("Please wait while the list is being updated."));
        }
    }

    else if (name == m_server_list_widget->m_properties[GUIEngine::PROP_ID])
    {
        m_selected_index = m_server_list_widget->getSelectionID();
        //new ServerInfoDialog(0.8f, 0.8f, id);
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

void ServerSelection::onUpdate(float dt, irr::video::IVideoDriver*)
{
    m_reload_timer += dt;
    if (m_reloading)
    {
        m_reloading = false;
        if(m_reload_timer > 5000.0f)
        {
            m_servers = CurrentOnlineUser::get()->getServerList();
            m_reload_timer = 0.0f;
        }
        loadList();
    }
}   // onUpdate
