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

#include "states_screens/online/server_selection.hpp"

#include "audio/sfx_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/CGUISpriteBank.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/message_queue.hpp"
#include "guiengine/modaldialog.hpp"
#include "io/file_manager.hpp"
#include "network/network_config.hpp"
#include "network/server.hpp"
#include "network/server_config.hpp"
#include "network/servers_manager.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/dialogs/server_info_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"
#include "utils/string_utils.hpp"

#include <algorithm>
#include <cassert>

using namespace Online;
bool g_bookmarks_next = false;
// ----------------------------------------------------------------------------
/** Constructor, which loads the stkgui file.
 */
ServerSelection::ServerSelection() : Screen("online/server_selection.stkgui")
{
    m_refreshing_server = false;
    m_refresh_timer = 0.0f;
    m_ipv6_only_without_nat64 = false;
    m_ip_warning_shown = false;
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
    m_server_list_widget->clear();
    m_server_list = nullptr;
    if (!UserConfigParams::m_server_bookmarks.empty())
        user_config->saveConfig();
}   // tearDown

// ----------------------------------------------------------------------------
/** Requests the servers manager to update its list of servers, and disables
 *  the 'refresh' button (till the refresh was finished).
 */
void ServerSelection::refresh()
{
    // If the request was created (i.e. no error, and not re-requested within
    // 5 seconds), clear the list and display the waiting message:
    if ((int64_t)StkTime::getMonoTimeMs() - m_last_load_time < 5000)
        return;

    m_ip_warning_shown = false;
    m_server_list_widget->clear();
    m_reload_widget->setActive(false);
    m_refreshing_server = true;
    m_refresh_timer = 0.0f;
    m_last_load_time = StkTime::getMonoTimeMs();
    m_server_list = NetworkConfig::get()->isWAN() ?
        ServersManager::get()->getWANRefreshRequest() :
        ServersManager::get()->getLANRefreshRequest();
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
    m_ipv6 = getWidget<GUIEngine::CheckBoxWidget>("ipv6");
    assert(m_ipv6 != NULL);
    m_searcher = getWidget<GUIEngine::TextBoxWidget>("searcher");
    assert(m_searcher != NULL);
    m_ipv6->setState(false);
    m_icon_bank = new irr::gui::STKModifiedSpriteBank(GUIEngine::getGUIEnv());
    m_bookmark_widget = getWidget<GUIEngine::IconButtonWidget>("bookmark");
    assert(m_bookmark_widget != NULL);
    m_bookmark_icon = irr_driver->getTexture
        (file_manager->getAsset(FileManager::GUI_ICON, "story_mode_book.png"));
    m_global_icon = irr_driver->getTexture
        (file_manager->getAsset(FileManager::GUI_ICON, "main_network.png"));
}   // loadedFromFile

// ----------------------------------------------------------------------------
/** Clear the server list, which will be reloaded. 
 */
void ServerSelection::beforeAddingWidget()
{
    m_icon_bank->clear();
    m_server_list_widget->clearColumns();
    m_server_list_widget->addColumn( _C("column_name", "Name"), 7);
    m_server_list_widget->addColumn(_C("column_name", "Game mode"), 3);
    m_server_list_widget->addColumn( _C("column_name", "Players"), 2);
    m_server_list_widget->addColumn(_C("column_name", "Difficulty"), 3);
    if (NetworkConfig::get()->isWAN())
    {
        // I18N: In server selection screen, owner of server, only displayed
        // if it's localhost or friends'
        m_server_list_widget->addColumn(_C("column_name", "Owner"), 3);
        // I18N: In server selection screen, distance to server
        m_server_list_widget->addColumn(_C("column_name", "Distance (km)"), 3);
        m_bookmark_widget->setVisible(true);
    }
    else
        m_bookmark_widget->setVisible(false);
}   // beforeAddingWidget

// ----------------------------------------------------------------------------
/** Triggers a refresh of the server list.
 */
void ServerSelection::init()
{
    Screen::init();

    m_last_load_time = -5000;
    m_sort_default = true;
    updateHeader();

#ifndef ENABLE_IPV6
    m_ipv6->setState(false);
    m_ipv6->setActive(false);
#endif
    if (NetworkConfig::get()->isLAN())
    {
        m_ipv6->setState(false);
        m_ipv6->setVisible(false);
        getWidget("ipv6_text")->setVisible(false);
    }
    else
        getWidget("ipv6_text")->setVisible(true);

    m_current_column = 5/*distance*/;
    m_searcher->clearListeners();
    m_searcher->addListener(this);

    m_icon_bank->setScale((float)GUIEngine::getFontHeight() / 72.0f);
    m_icon_bank->setTargetIconSize(128, 128);

    video::ITexture* icon1 = irr_driver->getTexture(
        file_manager->getAsset(FileManager::GUI_ICON, "green_check.png"));
    video::ITexture* icon2 = irr_driver->getTexture(
        file_manager->getAsset(FileManager::GUI_ICON, "hourglass.png"));
    m_icon_bank->addTextureAsSprite(icon1);
    m_icon_bank->addTextureAsSprite(icon2);
    for (unsigned i = 0; i < track_manager->getNumberOfTracks(); i++)
    {
        Track* t = track_manager->getTrack(i);
        video::ITexture* tex = NULL;
        if (t->isInternal())
        {
            tex = irr_driver->getTexture(file_manager
                ->getAsset(FileManager::GUI_ICON, "main_help.png"));
        }
        else
        {
            tex = irr_driver->getTexture(t->getScreenshotFile());
        }
        if (!tex)
        {
            tex = irr_driver->getTexture(file_manager
                ->getAsset(FileManager::GUI_ICON, "main_help.png"));
        }
        assert(tex);
        m_icon_bank->addTextureAsSprite(tex);
    }

    int row_height = GUIEngine::getFontHeight() * 2;
    
    m_server_list_widget->setIcons(m_icon_bank, row_height);
    m_sort_desc = false;
    refresh();
    m_ipv6_only_without_nat64 = false;
    m_ip_warning_shown = false;
}   // init

// ----------------------------------------------------------------------------
/** Loads the list of all servers. The gui element will be
 *  updated.
 */
void ServerSelection::loadList()
{
    m_server_list_widget->clear();

    std::stable_sort(m_servers.begin(), m_servers.end(), [this]
        (const std::shared_ptr<Server> a,
         const std::shared_ptr<Server> b)->bool
        {
            std::shared_ptr<Server> c = m_sort_desc ? a : b;
            std::shared_ptr<Server> d = m_sort_desc ? b : a;
            if (g_bookmarks_next && m_sort_default)
                return c->getBookmarkID() > d->getBookmarkID();
            switch (m_current_column)
            {
            case 0:
            {
                return c->getLowerCaseName() > d->getLowerCaseName();
                break;
            }
            case 1:
            {
                return c->getServerMode() > d->getServerMode();
                break;
            }
            case 2:
            {
                int c_players = c->getCurrentPlayers() - c->getCurrentAI();
                if (c_players < 0)
                    c_players = 0;
                int d_players = d->getCurrentPlayers() - d->getCurrentAI();
                if (d_players < 0)
                    d_players = 0;
                return c_players > d_players;
                break;
            }
            case 3:
            {
                return c->getDifficulty() > d->getDifficulty();
                break;
            }
            case 4:
            {
                return c->getServerOwnerLowerCaseName() >
                    d->getServerOwnerLowerCaseName();
                break;
            }
            case 5:
            {
                return c->getDistance() > d->getDistance();
                break;
            }
            }   // switch
            assert(false);
            return false;
        });
    for (auto& server : m_servers)
    {
        int icon = server->isGameStarted() ? 1 : 0;
        Track* t = server->getCurrentTrack();
        if (t)
            icon = track_manager->getTrackIndexByIdent(t->getIdent()) + 2;
        core::stringw num_players;
        int current_players = server->getCurrentPlayers();
        int current_ai = server->getCurrentAI();
        current_players -= current_ai;
        if (current_players < 0)
            current_players = 0;
        num_players.append(StringUtils::toWString(current_players));
        if (current_ai > 0)
        {
            num_players.append("(");
            num_players.append(StringUtils::toWString(current_ai));
            num_players.append(")");
        }
        num_players.append("/");
        num_players.append(StringUtils::toWString(server->getMaxPlayers()));
        std::vector<GUIEngine::ListWidget::ListCell> row;
        row.push_back(GUIEngine::ListWidget::ListCell(server->getName(), icon,
            7));

        core::stringw mode =
            ServerConfig::getModeName(server->getServerMode());
        row.push_back(GUIEngine::ListWidget::ListCell(mode, -1, 3, true));

        row.push_back(GUIEngine::ListWidget::ListCell(num_players, -1, 2,
            true));

        core::stringw difficulty =
            RaceManager::get()->getDifficultyName(server->getDifficulty());
        row.push_back(GUIEngine::ListWidget::ListCell(difficulty, -1, 3,
            true));

        if (NetworkConfig::get()->isWAN())
        {
            row.push_back(GUIEngine::ListWidget::ListCell(
                server->getServerOwnerName(), -1, 3, true));
            // I18N: In server selection screen, unknown distance to server
            core::stringw distance = _("Unknown");
            if (!(server->getDistance() < 0.0f))
                distance = StringUtils::toWString(server->getDistance());
            const core::stringw& flag = StringUtils::getCountryFlag(
                server->getCountryCode());
            if (!flag.empty())
            {
                distance += L" ";
                distance += flag;
            }
            row.push_back(GUIEngine::ListWidget::ListCell(distance, -1, 3,
                true));
        }
        m_server_list_widget->addItem("server", row);
    }
}   // loadList

// ----------------------------------------------------------------------------
/** Change the sort order if a column was clicked.
 *  \param column_id ID of the column that was clicked.
 */
void ServerSelection::onColumnClicked(int column_id, bool sort_desc, bool sort_default)
{
    if (sort_default)
    {
        m_sort_desc = false;
        m_sort_default = true;
        m_current_column = 5/*distance*/;
        loadList();
    }
    else
    {
        m_sort_desc = sort_desc;
        m_sort_default = false;
        m_current_column = column_id;
        loadList();
    }
}   // onColumnClicked

// ----------------------------------------------------------------------------
void ServerSelection::eventCallback(GUIEngine::Widget* widget,
                                    const std::string& name,
                                    const int playerID)
{
    //I18N: Message shown to user if no IPv4 detected by STK
    auto v4 = _("No IPv4 detected, you may not be able to join any servers.");
    //I18N: Message shown to user if no IPv6 detected by STK
    auto v6 = _("No IPv6 detected, you may not be able to join any servers.");
    if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if (name == "reload")
    {
        refresh();
    }
    else if (name == "bookmark")
    {
        g_bookmarks_next = !g_bookmarks_next;
        updateHeader();
        copyFromServerList();
    }
    else if (name == "private_server" || name == "ipv6")
    {
        if (!m_ip_warning_shown && m_ipv6->getState() &&
            NetworkConfig::get()->getIPType() == NetworkConfig::IP_V4)
        {
            m_ip_warning_shown = true;
            MessageQueue::add(MessageQueue::MT_ERROR, v6);
        }
        else if (!m_ip_warning_shown && !m_ipv6->getState() &&
            NetworkConfig::get()->getIPType() == NetworkConfig::IP_V6)
        {
            m_ip_warning_shown = true;
            MessageQueue::add(MessageQueue::MT_ERROR, v4);
        }
        copyFromServerList();
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
#ifdef ENABLE_IPV6
        // Lan server is set IPv6 during broadcasting
        if (NetworkConfig::get()->isWAN() &&
            m_servers[selected_index]->getIPV6Address())
            m_servers[selected_index]->setIPV6Connection(m_ipv6->getState());
#endif
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
    
    if (m_server_list && m_server_list->m_list_updated && !m_refreshing_server &&
        !NetworkConfig::get()->isWAN())
    {
        m_refresh_timer += dt;
        
        if (m_refresh_timer > 10.0f)
        {
            refresh();
        }
    }

    if (!m_ipv6_only_without_nat64 &&
        NetworkConfig::get()->getIPType() == NetworkConfig::IP_V6)
    {
        // Enable IPv6 button if IPv6 only without NAT64
        m_ipv6_only_without_nat64 = true;
        m_ipv6->setState(true);
    }

    if (!m_refreshing_server) return;

    if (m_server_list && m_server_list->m_list_updated)
    {
        m_refreshing_server = false;
        if (!m_server_list->m_servers.empty())
        {
            std::set<std::string> all_possible_keys;
            if (NetworkConfig::get()->isWAN())
            {
                // Remove offline server from bookmarks if inactive for 3 days
                for (auto& server : m_server_list->m_servers)
                    all_possible_keys.insert(server->getBookmarkKey());
                std::map<std::string, uint32_t>& bookmarks =
                    UserConfigParams::m_server_bookmarks;
                std::map<std::string, uint32_t>& bookmarks_order =
                    UserConfigParams::m_server_bookmarks_order;
                auto it = bookmarks.begin();
                while (it != bookmarks.end())
                {
                    uint64_t three_days = 60 * 60 * 24 * 3;
                    uint64_t limit = StkTime::getTimeSinceEpoch() - three_days;
                    if (all_possible_keys.find(it->first) ==
                        all_possible_keys.end())
                    {
                        if (it->second < limit)
                        {
                            it = bookmarks.erase(it);
                            bookmarks_order.erase(it->first);
                        }
                        else
                            it++;
                    }
                    else
                    {
                        it->second = StkTime::getTimeSinceEpoch();
                        it++;
                    }
                }
            }
            int selection = m_server_list_widget->getSelectionID();
            std::string selection_str = m_server_list_widget
                ->getSelectionInternalName();
            copyFromServerList();
            // restore previous selection
            if (selection != -1 && selection_str != "loading")
                m_server_list_widget->setSelectionID(selection);
        }
        else
        {
            SFXManager::get()->quickSound("anvil");
            m_server_list_widget->clear();
            m_server_list_widget->addItem("loading",
                                          _("No server is available."));
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
void ServerSelection::copyFromServerList()
{
    if (!m_server_list)
        return;
    m_servers = m_server_list->m_servers;
    if (m_servers.empty())
        return;
    m_servers.erase(std::remove_if(m_servers.begin(), m_servers.end(),
        [this](const std::shared_ptr<Server>& a)->bool
        {
            return a->isPasswordProtected() != m_private_server->getState();
        }), m_servers.end());
    m_servers.erase(std::remove_if(m_servers.begin(), m_servers.end(),
        [this](const std::shared_ptr<Server>& a)->bool
        {
            if (m_ipv6->getState() && !a->getIPV6Address())
                return true;
            return false;
        }), m_servers.end());
    const core::stringw& search = m_searcher->getText();
    const std::string search_word_lc = StringUtils::toLowerCase(
        StringUtils::wideToUtf8(search));
    if (!search_word_lc.empty())
    {
        m_servers.erase(std::remove_if(m_servers.begin(), m_servers.end(),
            [search_word_lc](const std::shared_ptr<Server>& a)->bool
            {
                if (!a->searchByName(search_word_lc))
                    return true;
                return false;
            }), m_servers.end());
    }
    if (g_bookmarks_next)
    {
        m_servers.erase(std::remove_if(m_servers.begin(), m_servers.end(),
            [](const std::shared_ptr<Server>& a)->bool
            {
                const std::string& key = a->getBookmarkKey();
                auto it = UserConfigParams::m_server_bookmarks.find(key);
                if (it == UserConfigParams::m_server_bookmarks.end())
                    return true;
                return false;
            }), m_servers.end());
    }
    loadList();
}   // copyFromServerList

// ----------------------------------------------------------------------------
void ServerSelection::unloaded()
{
    delete m_icon_bank;
    m_icon_bank = NULL;
}   // unloaded

// ----------------------------------------------------------------------------
void ServerSelection::updateHeader()
{
    m_bookmark_widget->setImage(g_bookmarks_next ?
        m_global_icon : m_bookmark_icon);
    if (g_bookmarks_next && NetworkConfig::get()->isWAN())
        getWidget("title_header")->setText(_("Server Bookmarks"));
    else
        getWidget("title_header")->setText(_("Server Selection"));
}   // updateHeader
