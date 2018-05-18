//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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

#include "states_screens/dialogs/network_user_dialog.hpp"

#include "config/player_manager.hpp"
#include "guiengine/dialog_queue.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "online/online_profile.hpp"
#include "network/protocols/lobby_protocol.hpp"
#include "network/stk_host.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

#include <IGUIEnvironment.h>

using namespace GUIEngine;
using namespace irr;
using namespace irr::gui;
using namespace Online;

// ----------------------------------------------------------------------------
void NetworkUserDialog::beforeAddingWidgets()
{
    m_name_widget = getWidget<LabelWidget>("name");
    assert(m_name_widget != NULL);
    m_name_widget->setText(m_name, false);

    m_friend_widget = getWidget<IconButtonWidget>("friend");
    assert(m_friend_widget != NULL);
    m_friend_widget->setVisible(m_online_id != 0);

    // Hide friend request button if already friend
    Online::OnlineProfile* opp =
        PlayerManager::getCurrentPlayer()->getProfile();
    if (m_online_id != 0 && opp && opp->hasFetchedFriends())
    {
        for (uint32_t user_id : opp->getFriends())
        {
            if (user_id == m_online_id)
            {
                m_friend_widget->setVisible(false);
            }
        }
    }

    m_kick_widget = getWidget<IconButtonWidget>("decline");
    assert(m_kick_widget != NULL);

    //I18N: In the network user dialog
    m_kick_widget->setText(_("Kick"));
    m_kick_widget->setVisible(STKHost::get()->isAuthorisedToControl());

    m_cancel_widget = getWidget<IconButtonWidget>("cancel");
    assert(m_cancel_widget != NULL);

    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget != NULL);
    m_options_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    m_options_widget->select("cancel", PLAYER_ID_GAME_MASTER);

    getWidget<IconButtonWidget>("accept")->setVisible(false);
    getWidget<IconButtonWidget>("remove")->setVisible(false);
    getWidget<IconButtonWidget>("enter")->setVisible(false);
    getWidget<LabelWidget>("info")->setVisible(false);
}   // beforeAddingWidgets

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation
    NetworkUserDialog::processEvent(const std::string& source)
{
    if (source == m_options_widget->m_properties[PROP_ID])
    {
        const std::string& selection = m_options_widget
            ->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (selection == m_cancel_widget->m_properties[PROP_ID])
        {
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if(selection == m_friend_widget->m_properties[PROP_ID])
        {
            XMLRequest *request = new XMLRequest();
            PlayerManager::setUserDetails(request, "friend-request");
            request->addParameter("friendid", m_online_id);
            request->queue();
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if(selection == m_kick_widget->m_properties[PROP_ID])
        {
            NetworkString kick(PROTOCOL_LOBBY_ROOM);
            kick.addUInt8(LobbyProtocol::LE_KICK_HOST).addUInt32(m_host_id);
            STKHost::get()->sendToServer(&kick, true/*reliable*/);
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}   // processEvent

// -----------------------------------------------------------------------------
bool NetworkUserDialog::onEscapePressed()
{
    if (m_cancel_widget->isActivated())
        m_self_destroy = true;
    return false;
}   // onEscapePressed
