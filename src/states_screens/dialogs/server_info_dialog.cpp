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

#include "states_screens/dialogs/server_info_dialog.hpp"

#include "guiengine/engine.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "network/server.hpp"
#include "network/stk_host.hpp"
#include "network/network_config.hpp"
#include "states_screens/networking_lobby.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"

#include <IGUIEnvironment.h>

using namespace GUIEngine;
using namespace irr;
using namespace irr::gui;
using namespace Online;

// -----------------------------------------------------------------------------
/** Dialog constructor. 
 *  \param server_id ID of the server of which to display the info.
 *  \param host_id ID of the host.
 *  \param from_server_creation: true if the dialog shows the data of this
 *         server (i.e. while it is being created).
 */
ServerInfoDialog::ServerInfoDialog(std::shared_ptr<Server> server)
                : ModalDialog(0.8f,0.8f), m_server(server), m_password(NULL)
{
    Log::info("ServerInfoDialog", "Server id is %d, owner is %d",
       server->getServerId(), server->getServerOwner());
    m_self_destroy = false;

    loadFromFile("online/server_info_dialog.stkgui");

    GUIEngine::LabelWidget *name = getWidget<LabelWidget>("server_name");
    assert(name);
    name->setText(server->getName(),false);

    core::stringw difficulty = race_manager->getDifficultyName(server->getDifficulty());
    GUIEngine::LabelWidget *lbldifficulty = getWidget<LabelWidget>("server_difficulty");
    lbldifficulty->setText(difficulty, false);

    core::stringw mode = NetworkConfig::get()->getModeName(server->getServerMode());
    GUIEngine::LabelWidget *gamemode = getWidget<LabelWidget>("server_game_mode");
    gamemode->setText(mode, false);

    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget != NULL);
    m_join_widget = getWidget<IconButtonWidget>("join");
    assert(m_join_widget != NULL);
    m_cancel_widget = getWidget<IconButtonWidget>("cancel");
    assert(m_cancel_widget != NULL);
    m_options_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

    if (m_server->isPasswordProtected())
    {
        m_password = getWidget<TextBoxWidget>("password");
        m_password->setPasswordBox(true, L'*');
        assert(m_password != NULL);
    }
    else
    {
        getWidget("label_password")->setVisible(false);
        getWidget("password")->setVisible(false);
    }

}   // ServerInfoDialog

// -----------------------------------------------------------------------------
ServerInfoDialog::~ServerInfoDialog()
{
}   // ~ServerInfoDialog

// -----------------------------------------------------------------------------
void ServerInfoDialog::requestJoin()
{
    if (m_server->isPasswordProtected())
    {
        assert(m_password != NULL);
        NetworkConfig::get()->setPassword(
            StringUtils::wideToUtf8(m_password->getText()));
    }
    else
    {
        NetworkConfig::get()->setPassword("");
    }
    STKHost::create();
    NetworkingLobby::getInstance()->setJoinedServer(m_server);
    ModalDialog::dismiss();
    NetworkingLobby::getInstance()->push();
}   // requestJoin

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation
                 ServerInfoDialog::processEvent(const std::string& eventSource)
{
    if (eventSource == m_options_widget->m_properties[PROP_ID])
    {
        const std::string& selection =
                 m_options_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
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
/** When the player pressed enter, select 'join' as default.
 */
void ServerInfoDialog::onEnterPressedInternal()
{
    // If enter was pressed while none of the buttons was focused interpret
    // as join event
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
    // It's unsafe to delete from inside the event handler so we do it here
    if (m_self_destroy)
    {
        ModalDialog::dismiss();
        return;
    }
}   // onUpdate
