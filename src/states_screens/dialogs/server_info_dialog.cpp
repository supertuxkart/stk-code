//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Glenn De Jonghe
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

#include "states_screens/dialogs/server_info_dialog.hpp"

#include "audio/sfx_manager.hpp"
#include "guiengine/engine.hpp"
#include "network/protocol_manager.hpp"
#include "network/protocols/connect_to_server.hpp"
#include "online/servers_manager.hpp"
#include "states_screens/dialogs/registration_dialog.hpp"
#include "states_screens/networking_lobby.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <IGUIEnvironment.h>

using namespace GUIEngine;
using namespace irr;
using namespace irr::gui;
using namespace Online;

// -----------------------------------------------------------------------------

ServerInfoDialog::ServerInfoDialog(uint32_t server_id, uint32_t host_id, bool from_server_creation)
        : ModalDialog(0.8f,0.8f), m_server_id(server_id), m_host_id(host_id)
{
    Log::info("ServerInfoDialog", "Server id is %d, Host id is %d", server_id, host_id);
    m_self_destroy = false;
    m_enter_lobby = false;
    m_from_server_creation = from_server_creation;
    m_server_join_request = NULL;

    loadFromFile("online/server_info_dialog.stkgui");

    m_name_widget = getWidget<LabelWidget>("name");
    assert(m_name_widget != NULL);
    const Server * server = ServersManager::get()->getServerByID(m_server_id);
    m_name_widget->setText(server->getName(),false);
    m_info_widget = getWidget<LabelWidget>("info");
    assert(m_info_widget != NULL);
    if (m_from_server_creation)
        m_info_widget->setText(_("Server successfully created. You can now join it."), true);
    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget != NULL);
    m_join_widget = getWidget<IconButtonWidget>("join");
    assert(m_join_widget != NULL);
    m_cancel_widget = getWidget<IconButtonWidget>("cancel");
    assert(m_cancel_widget != NULL);
    m_options_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

}   // ServerInfoDialog

// -----------------------------------------------------------------------------
ServerInfoDialog::~ServerInfoDialog()
{
    if (m_server_join_request)
        delete m_server_join_request;
    m_server_join_request = NULL;
}   // ~ServerInfoDialog

// -----------------------------------------------------------------------------
void ServerInfoDialog::requestJoin()
{
    // FIXME - without this next line, it appears that m_server_join is completely unused.
    //m_server_join_request = Online::CurrentUser::get()->requestServerJoin(m_server_id);
    Online::ServersManager::get()->setJoinedServer(m_server_id);
    ProtocolManager::getInstance()->requestStart(new ConnectToServer(m_server_id, m_host_id));
    ModalDialog::dismiss();
    NetworkingLobby::getInstance()->push();
    //Online::CurrentUser::release();
}   // requestJoin

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation ServerInfoDialog::processEvent(const std::string& eventSource)
{

    if (eventSource == m_options_widget->m_properties[PROP_ID])
    {
        const std::string& selection = m_options_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (selection == m_cancel_widget->m_properties[PROP_ID])
        {
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if(selection == m_join_widget->m_properties[PROP_ID])
        {
            requestJoin();
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}   // processEvent

// -----------------------------------------------------------------------------

void ServerInfoDialog::onEnterPressedInternal()
{

    //If enter was pressed while none of the buttons was focused interpret as join event
    const int playerID = PLAYER_ID_GAME_MASTER;
    if (GUIEngine::isFocusedForPlayer(m_options_widget, playerID))
        return;
    requestJoin();
}   // onEnterPressedInternal

// -----------------------------------------------------------------------------

bool ServerInfoDialog::onEscapePressed()
{
    if (m_cancel_widget->isActivated())
        m_self_destroy = true;
    return false;
}   // onEscapePressed

// -----------------------------------------------------------------------------

void ServerInfoDialog::onUpdate(float dt)
{
    if(m_server_join_request != NULL)
    {
        if(m_server_join_request->isDone())
        {
            if(m_server_join_request->isSuccess())
            {
                m_enter_lobby = true;
            }
            else
            {
                SFXManager::get()->quickSound( "anvil" );
                m_info_widget->setErrorColor();
                m_info_widget->setText(m_server_join_request->getInfo(), false);
            }
            delete m_server_join_request;
            m_server_join_request = NULL;
        }
        else
        {
            m_info_widget->setDefaultColor();
            m_info_widget->setText(StringUtils::loadingDots(_("Joining server")),
                                   false);
        }
    }

    //If we want to open the registration dialog, we need to close this one first
    if (m_enter_lobby) m_self_destroy = true;

    // It's unsafe to delete from inside the event handler so we do it here
    if (m_self_destroy)
    {
        ModalDialog::dismiss();
        if (m_from_server_creation)
            StateManager::get()->popMenu();
        if (m_enter_lobby)
            NetworkingLobby::getInstance()->push();
        return;
    }
}   // onUpdate
