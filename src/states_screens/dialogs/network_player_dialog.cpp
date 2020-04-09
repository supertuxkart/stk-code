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

#include "states_screens/dialogs/network_player_dialog.hpp"

#include "config/player_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/dialog_queue.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "online/online_profile.hpp"
#include "network/network_string.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/stk_host.hpp"
#include "states_screens/dialogs/general_text_field_dialog.hpp"
#include "states_screens/dialogs/ranking_callback.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

#include <IGUIEnvironment.h>

using namespace GUIEngine;
using namespace irr;
using namespace irr::gui;
using namespace Online;

// ----------------------------------------------------------------------------
void NetworkPlayerDialog::beforeAddingWidgets()
{
    LabelWidget* header = getWidget<LabelWidget>("title");
    assert(header);
    //I18N: In the network player dialog
    header->setText(_("Player info"), false);

    m_desc_widget = getWidget<LabelWidget>("desc");
    assert(m_desc_widget != NULL);
    //I18N: In the network player dialog
    core::stringw desc = _("Player name: %s", m_name);
#ifndef SERVER_ONLY
    if (!m_country_code.empty())
    {
        core::stringw country_name =
            translations->getLocalizedCountryName(m_country_code);
        desc += L"\n";
        //I18N: In the network player dialog, show the player location with
        //country name (based on IP geolocation)
        desc += _("Player location: %s", country_name);
    }
#endif
    m_desc_widget->setText(desc, false);

    m_info_widget = getWidget<LabelWidget>("info");
    assert(m_info_widget != NULL);
    if (m_online_id != 0)
    {
        m_ranking_callback =
            RankingCallback::getRankingCallback(m_name, m_online_id);
        m_ranking_callback->queue();
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
    m_kick_widget->setImage(file_manager->getAsset(FileManager::GUI_ICON,
        "remove.png"), IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);

    //I18N: In the network player dialog
    m_kick_widget->setText(_("Kick"));
    m_kick_widget->setVisible(STKHost::get()->isAuthorisedToControl()
        && m_host_id != STKHost::get()->getMyHostId());

    m_cancel_widget = getWidget<IconButtonWidget>("cancel");
    assert(m_cancel_widget != NULL);

    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget != NULL);

    m_change_team_widget = NULL;
    if (m_allow_change_team && m_host_id == STKHost::get()->getMyHostId())
    {
        m_change_team_widget = getWidget<IconButtonWidget>("accept");
        m_change_team_widget->setVisible(true);
        //I18N: In the network player dialog
        m_change_team_widget->setText(_("Change team"));
        m_change_team_widget->setImage(file_manager->getAsset(FileManager::GUI_ICON,
            "race_giveup.png"), IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    }
    else
        getWidget<IconButtonWidget>("accept")->setVisible(false);

    m_handicap_widget = NULL;
    if (m_host_id == STKHost::get()->getMyHostId())
    {
        m_handicap_widget = getWidget<IconButtonWidget>("remove");
        m_handicap_widget->setVisible(true);
        if (m_handicap == HANDICAP_NONE)
        {
            //I18N: In the network player dialog
            m_handicap_widget->setText(_("Enable handicap"));
        }
        else
        {
            //I18N: In the network player dialog
            m_handicap_widget->setText(_("Disable handicap"));
        }
        m_handicap_widget->setImage(irr_driver->getTexture(FileManager::GUI_ICON,
            "anchor-icon.png"));
    }
    else
        getWidget<IconButtonWidget>("remove")->setVisible(false);
    m_report_widget = getWidget<IconButtonWidget>("enter");
    assert(m_report_widget != NULL);
    auto cl = LobbyProtocol::get<ClientLobby>();
    if (cl->serverEnabledReportPlayer() &&
        m_host_id != STKHost::get()->getMyHostId())
    {
        // I18N: In the network player dialog,
        // report player about for example abusive behaviour in game
        m_report_widget->setText(_("Report player"));
        m_report_widget->setImage(file_manager->getAsset(FileManager::GUI_ICON,
            "red_mark.png"), IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
    }
    else
        m_report_widget->setVisible(false);
}   // beforeAddingWidgets

// -----------------------------------------------------------------------------
void NetworkPlayerDialog::init()
{
    m_options_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    m_options_widget->select("cancel", PLAYER_ID_GAME_MASTER);
}   // init

// -----------------------------------------------------------------------------
void NetworkPlayerDialog::onUpdate(float dt)
{
    if (m_ranking_callback && !m_ranking_callback->isDone())
    {
        // I18N: In the network player dialog, showing when waiting for
        // the result of the ranking info of a player
        core::stringw msg = _("Fetching ranking info for %s", m_name);
        m_info_widget->setText(StringUtils::loadingDots(msg.c_str()), false);
    }
    else if (m_ranking_callback && m_ranking_callback->isDone())
    {
        m_info_widget->setText(m_ranking_callback->getRankingResult(), false);
        m_ranking_callback = nullptr;
    }

    // It's unsafe to delete from inside the event handler so we do it here
    if (m_open_report_textbox)
    {
        // I18N: In the network player dialog, instruction for reporting player
        core::stringw t = _("Tell server administrator about this player (%s):", m_name);
        uint32_t host_id = m_host_id;
        ModalDialog::dismiss();
        new GeneralTextFieldDialog(t.c_str(),
            [] (const irr::core::stringw& text) {},
            [host_id] (GUIEngine::LabelWidget* lw,
                       GUIEngine::TextBoxWidget* tb)->bool
            {
                core::stringw info = tb->getText();
                if (info.empty())
                    return false;
                NetworkString report(PROTOCOL_LOBBY_ROOM);
                report.addUInt8(LobbyProtocol::LE_REPORT_PLAYER)
                    .addUInt32(host_id).encodeString16(info);
                STKHost::get()->sendToServer(&report, true/*reliable*/);
                return true;
            });
        return;
    }
    if (m_self_destroy)
    {
        ModalDialog::dismiss();
        return;
    }
}   // onUpdate

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation
    NetworkPlayerDialog::processEvent(const std::string& source)
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
        else if (selection == m_report_widget->m_properties[PROP_ID])
        {
            m_open_report_textbox = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == m_friend_widget->m_properties[PROP_ID])
        {
            auto request = std::make_shared<XMLRequest>();
            PlayerManager::setUserDetails(request, "friend-request");
            request->addParameter("friendid", m_online_id);
            request->queue();
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == m_kick_widget->m_properties[PROP_ID])
        {
            NetworkString kick(PROTOCOL_LOBBY_ROOM);
            kick.addUInt8(LobbyProtocol::LE_KICK_HOST).addUInt32(m_host_id);
            STKHost::get()->sendToServer(&kick, true/*reliable*/);
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if (m_change_team_widget &&
            selection == m_change_team_widget->m_properties[PROP_ID])
        {
            NetworkString change_team(PROTOCOL_LOBBY_ROOM);
            change_team.addUInt8(LobbyProtocol::LE_CHANGE_TEAM)
                .addUInt8(m_local_id);
            STKHost::get()->sendToServer(&change_team, true/*reliable*/);
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if (m_handicap_widget &&
            selection == m_handicap_widget->m_properties[PROP_ID])
        {
            HandicapLevel new_handicap = HANDICAP_NONE;
            if (m_handicap == HANDICAP_NONE)
            {
                new_handicap = HANDICAP_MEDIUM;
            }
            NetworkString change_handicap(PROTOCOL_LOBBY_ROOM);
            change_handicap.addUInt8(LobbyProtocol::LE_CHANGE_HANDICAP)
                .addUInt8(m_local_id).addUInt8(new_handicap);
            STKHost::get()->sendToServer(&change_handicap, true/*reliable*/);
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}   // processEvent

// -----------------------------------------------------------------------------
bool NetworkPlayerDialog::onEscapePressed()
{
    if (m_cancel_widget->isActivated())
        m_self_destroy = true;
    return false;
}   // onEscapePressed
