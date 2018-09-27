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

    m_info_widget = getWidget<LabelWidget>("info");
    assert(m_info_widget != NULL);
    if (m_online_id != 0)
    {
        updatePlayerRanking(m_name, m_online_id, m_info_widget,
            m_fetched_ranking);
    }
    else
    {
        m_info_widget->setVisible(false);
    }

    m_friend_widget = getWidget<IconButtonWidget>("friend");
    assert(m_friend_widget != NULL);
    m_friend_widget->setVisible(m_online_id != 0 &&
        PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_SIGNED_IN
        && m_online_id != PlayerManager::getCurrentPlayer()->getOnlineId());

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
    m_kick_widget->setVisible(STKHost::get()->isAuthorisedToControl()
        && m_host_id != STKHost::get()->getMyHostId());

    m_cancel_widget = getWidget<IconButtonWidget>("cancel");
    assert(m_cancel_widget != NULL);

    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget != NULL);
    m_options_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    m_options_widget->select("cancel", PLAYER_ID_GAME_MASTER);

    m_change_team_widget = NULL;
    if (m_allow_change_team && m_host_id == STKHost::get()->getMyHostId())
    {
        m_change_team_widget = getWidget<IconButtonWidget>("accept");
        m_change_team_widget->setVisible(true);
        //I18N: In the network user dialog
        m_change_team_widget->setText(_("Change team"));
        m_change_team_widget->setImage(file_manager->getAsset(FileManager::GUI_ICON,
            "race_giveup.png"), IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    }
    else
        getWidget<IconButtonWidget>("accept")->setVisible(false);
    getWidget<IconButtonWidget>("remove")->setVisible(false);
    getWidget<IconButtonWidget>("enter")->setVisible(false);
}   // beforeAddingWidgets

// -----------------------------------------------------------------------------
void NetworkUserDialog::onUpdate(float dt)
{
    if (*m_fetched_ranking == false)
    {
        // I18N: In the network player dialog, showing when waiting for
        // the result of the ranking info of a player
        core::stringw msg = _("Fetching ranking info for %s", m_name);
        m_info_widget->setText(StringUtils::loadingDots(msg.c_str()), false);
    }

    // It's unsafe to delete from inside the event handler so we do it here
    if (m_self_destroy)
    {
        ModalDialog::dismiss();
        return;
    }
}   // onUpdate

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
        else if(selection == m_change_team_widget->m_properties[PROP_ID])
        {
            NetworkString change_team(PROTOCOL_LOBBY_ROOM);
            change_team.addUInt8(LobbyProtocol::LE_CHANGE_TEAM)
                .addUInt8(m_local_id);
            STKHost::get()->sendToServer(&change_team, true/*reliable*/);
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
