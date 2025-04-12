//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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


#include "states_screens/online/online_screen.hpp"

#include "addons/news_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/CGUISpriteBank.hpp"
#include "guiengine/message_queue.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "io/file_manager.hpp"
#include "input/device_manager.hpp"
#include "network/event.hpp"
#include "network/network_config.hpp"
#include "network/server.hpp"
#include "network/server_config.hpp"
#include "network/socket_address.hpp"
#include "network/stk_host.hpp"
#include "network/stk_peer.hpp"
#include "online/link_helper.hpp"
#include "online/profile_manager.hpp"
#include "online/request_manager.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "states_screens/online/online_lan.hpp"
#include "states_screens/online/online_profile_achievements.hpp"
#include "states_screens/online/online_profile_servers.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/options/user_screen.hpp"
#include "states_screens/dialogs/general_text_field_dialog.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"
#include "states_screens/dialogs/enter_address_dialog.hpp"
#include <string>


using namespace GUIEngine;
using namespace Online;

// ----------------------------------------------------------------------------

OnlineScreen::OnlineScreen() : Screen("online/online.stkgui")
{
    m_online_string = _("Your profile");
    //I18N: Used as a verb, appears on the main networking menu (login button)
    m_login_string = _("Login");
}   // OnlineScreen

// ----------------------------------------------------------------------------

void OnlineScreen::loadedFromFile()
{
    video::ITexture* icon1 = irr_driver->getTexture( file_manager->getAsset(FileManager::GUI_ICON,
                                                     "red_dot.png"         ));
    video::ITexture* icon2 = irr_driver->getTexture( file_manager->getAsset(FileManager::GUI_ICON,
                                                     "news_headline.png"      ));
    video::ITexture* icon3 = irr_driver->getTexture( file_manager->getAsset(FileManager::GUI_ICON,
                                                     "news.png"  ));

    m_icon_bank = new irr::gui::STKModifiedSpriteBank( GUIEngine::getGUIEnv());
    m_icon_red_dot       = m_icon_bank->addTextureAsSprite(icon1);
    m_icon_news_headline = m_icon_bank->addTextureAsSprite(icon2);
    m_icon_news          = m_icon_bank->addTextureAsSprite(icon3);

    m_enable_splitscreen = getWidget<CheckBoxWidget>("enable-splitscreen");
    assert(m_enable_splitscreen);

    m_news_list = getWidget<GUIEngine::ListWidget>("news_list");
    assert(m_news_list);
    m_news_list->setColumnListener(this);
}   // loadedFromFile

// ----------------------------------------------------------------------------

void OnlineScreen::unloaded()
{
    delete m_icon_bank;
    m_icon_bank = NULL;
}

// ----------------------------------------------------------------------------

void OnlineScreen::beforeAddingWidget()
{
    m_news_list->clearColumns();
    m_news_list->addColumn( _("News from STK Blog"), 4 );
    m_news_list->addColumn( _("Date"), 1 );
} // beforeAddingWidget

// ----------------------------------------------------------------------------
//
void OnlineScreen::init()
{
    Screen::init();

    m_online = getWidget<IconButtonWidget>("online");
    assert(m_online);

    m_user_id = getWidget<ButtonWidget>("user-id");
    assert(m_user_id);

    m_icon_bank->setScale(1.0f / 72.0f);
    m_icon_bank->setTargetIconSize(128, 128);
    m_news_list->setIcons(m_icon_bank, 2.0f);

    RibbonWidget* r = getWidget<RibbonWidget>("menu_toprow");
    r->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

    m_enable_splitscreen->setState(
        UserConfigParams::m_enable_network_splitscreen);
    // Pre-add a default single player profile in network
    if (!m_enable_splitscreen->getState() &&
        NetworkConfig::get()->getNetworkPlayers().empty())
    {
        NetworkConfig::get()->addNetworkPlayer(
            input_manager->getDeviceManager()->getLatestUsedDevice(),
            PlayerManager::getCurrentPlayer(), HANDICAP_NONE);
        NetworkConfig::get()->doneAddingNetworkPlayers();
    }
    loadList();
}   // init

// ----------------------------------------------------------------------------

void OnlineScreen::loadList()
{
#ifndef SERVER_ONLY

    int news_count = NewsManager::get()->getNewsCount(NewsManager::NTYPE_LIST);

    int last_shown_id = UserConfigParams::m_news_list_shown_id;

    NewsManager::get()->resetNewsPtr(NewsManager::NTYPE_LIST);
    NewsManager::get()->prioritizeNewsAfterID(NewsManager::NTYPE_LIST, last_shown_id);

    m_news_list->clear();
    m_news_links.clear();

    if (news_count == 0)
    {
        std::vector<GUIEngine::ListWidget::ListCell> row;
        row.push_back(GUIEngine::ListWidget::ListCell(
            _("There are currently no news available."), -1, 4, false));
        m_news_list->addItem("-1", row);
    }

    while (news_count--)
    {
        int id = NewsManager::get()->getNextNewsID(NewsManager::NTYPE_LIST);
        std::string id_str = StringUtils::toString(id);
        core::stringw str = NewsManager::get()->getCurrentNewsMessage(NewsManager::NTYPE_LIST);

        if (id == -1) // It's an error message
        {
            std::vector<GUIEngine::ListWidget::ListCell> row;
            row.push_back(GUIEngine::ListWidget::ListCell(str.c_str(), -1, 4, false));
            m_news_list->addItem(id_str.c_str(), row);
            continue;
        }

        std::string date = NewsManager::get()->getCurrentNewsDate(NewsManager::NTYPE_LIST);
        int icon = NewsManager::get()->isCurrentNewsImportant(NewsManager::NTYPE_LIST) ?
            m_icon_news_headline : m_icon_news;

        m_news_links[id_str] = NewsManager::get()->getCurrentNewsLink(NewsManager::NTYPE_LIST);
        
        if (id > UserConfigParams::m_news_list_shown_id)
            icon = m_icon_red_dot;
        
        last_shown_id = std::max(id, last_shown_id);

        // Date format
        int yyyy, mm, dd;
        sscanf(date.c_str(), "%d-%d-%d", &yyyy, &mm, &dd);
        date = StkTime::toString(yyyy, mm, dd);

        std::vector<GUIEngine::ListWidget::ListCell> row;
        row.push_back(GUIEngine::ListWidget::ListCell(str.c_str(), icon, 4, false));
        row.push_back(GUIEngine::ListWidget::ListCell(date.c_str(), -1, 1, true));
        m_news_list->addItem(id_str.c_str(), row);
    }

    UserConfigParams::m_news_list_shown_id = last_shown_id;

#endif
}

// ----------------------------------------------------------------------------

void OnlineScreen::onUpdate(float delta)
{
    PlayerProfile *player = PlayerManager::getCurrentPlayer();
    if (PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_GUEST  ||
        PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_SIGNED_IN)
    {
        m_online->setActive(true);
        m_online->setLabel(m_online_string);
        m_user_id->setText(player->getLastOnlineName() + "@stk");
    }
    else if (PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_SIGNED_OUT)
    {
        m_online->setActive(true);
        m_online->setLabel(m_login_string);
        m_user_id->setText(player->getName());
    }
    else
    {
        // now must be either logging in or logging out
        m_online->setActive(false);
        m_user_id->setText(player->getName());
    }

    m_online->setLabel(PlayerManager::getCurrentOnlineId() ? m_online_string
                                                           : m_login_string);
    // In case for entering server address finished
    if (m_entered_server)
    {
        NetworkConfig::get()->setIsLAN();
        NetworkConfig::get()->setIsServer(false);
        ServerConfig::m_private_server_password = "";
        STKHost::create();
        NetworkingLobby::getInstance()->setJoinedServer(m_entered_server);
        m_entered_server = nullptr;
        StateManager::get()->resetAndSetStack(
            NetworkConfig::get()->getResetScreens(true/*lobby*/).data());
    }
}   // onUpdate

// ----------------------------------------------------------------------------

void OnlineScreen::eventCallback(Widget* widget, const std::string& name,
                                   const int playerID)
{
    if (name == "user-id")
    {
        NetworkConfig::get()->cleanNetworkPlayers();
        UserScreen::getInstance()->push();
        return;
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
        return;
    }
    else if (name == "news_list")
    {
        std::string id = m_news_list->getSelectionInternalName();

        if (!m_news_links[id].empty())
        {
            Online::LinkHelper::openURL(m_news_links[id]);
        }
    }
    else if (name == "enable-splitscreen")
    {
        CheckBoxWidget* splitscreen = dynamic_cast<CheckBoxWidget*>(widget);
        assert(splitscreen);
        if (!splitscreen->getState())
        {
            // Default single player
            NetworkConfig::get()->cleanNetworkPlayers();
            NetworkConfig::get()->addNetworkPlayer(
                input_manager->getDeviceManager()->getLatestUsedDevice(),
                PlayerManager::getCurrentPlayer(), HANDICAP_NONE);
            NetworkConfig::get()->doneAddingNetworkPlayers();
        }
        else
        {
            // Let lobby add the players
            NetworkConfig::get()->cleanNetworkPlayers();
        }
        UserConfigParams::m_enable_network_splitscreen = splitscreen->getState();
        return;
    }

    RibbonWidget* ribbon = dynamic_cast<RibbonWidget*>(widget);
    if (ribbon == NULL) return; // what's that event??

    // ---- A ribbon icon was clicked
    std::string selection =
        ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);

    if (selection == "lan")
    {
        OnlineLanScreen::getInstance()->push();
    }
    else if (selection == "wan")
    {
        if (PlayerManager::getCurrentOnlineState() != PlayerProfile::OS_SIGNED_IN)
        {
            //I18N: Shown to players when he is not is not logged in
            new MessageDialog(_("You must be logged in to play Global "
                "networking. Click your username above."));
        }
        else
            OnlineProfileServers::getInstance()->push();
    }
    else if (selection == "online")
    {
        if (UserConfigParams::m_internet_status != RequestManager::IPERM_ALLOWED)
        {
            new MessageDialog(_("You can not play online without internet access. "
                                "If you want to play online, go in the options menu, "
                                "and check \"Connect to the Internet\"."));
            return;
        }

        if (PlayerManager::getCurrentOnlineId())
        {
            ProfileManager::get()->setVisiting(PlayerManager::getCurrentOnlineId());
            TabOnlineProfileAchievements::getInstance()->push();
        }
        else
        {
            UserScreen::getInstance()->push();
        }
    }
    else if (selection == "enter-address")
    {
        m_entered_server = nullptr;
        new EnterAddressDialog(&m_entered_server);
    }
}   // eventCallback

// ----------------------------------------------------------------------------
void OnlineScreen::onColumnClicked(int column_id, bool sort_desc, bool sort_default)
{

}

// ----------------------------------------------------------------------------
/** Also called when pressing the back button. It resets the flags to indicate
 *  a networked game.
 */
bool OnlineScreen::onEscapePressed()
{
    NetworkConfig::get()->cleanNetworkPlayers();
    NetworkConfig::get()->unsetNetworking();
    return true;
}   // onEscapePressed
