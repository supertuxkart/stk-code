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

#include "io/file_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widgets/CGUISTKListBox.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "network/server.hpp"
#include "network/server_config.hpp"
#include "network/stk_host.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "states_screens/online/server_selection.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <IGUIEnvironment.h>
#include <IGUIScrollBar.h>

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
                : ModalDialog(0.85f,0.85f), m_server(server), m_password(NULL)
{
    Log::info("ServerInfoDialog", "Server id is %d, owner is %d",
       server->getServerId(), server->getServerOwner());
    m_self_destroy = false;
    m_join_server = false;

    m_bookmark_icon = irr_driver->getTexture
        (file_manager->getAsset(FileManager::GUI_ICON, "story_mode_book.png"));
    m_remove_icon = irr_driver->getTexture
        (file_manager->getAsset(FileManager::GUI_ICON, "remove.png"));

    loadFromFile("online/server_info_dialog.stkgui");
    LabelWidget* title = getWidget<LabelWidget>("title");
    title->setText(server->getName(), true);
    // Make sure server name is not clickable for URL
    title->getIrrlichtElement<IGUIStaticText>()->setMouseCallback(nullptr);

    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget != NULL);
    m_join_widget = getWidget<IconButtonWidget>("join");
    assert(m_join_widget != NULL);
    m_cancel_widget = getWidget<IconButtonWidget>("cancel");
    assert(m_cancel_widget != NULL);

    if (m_server->isPasswordProtected())
    {
        m_password = getWidget<TextBoxWidget>("password");
        m_password->setPasswordBox(true, L'*');
        assert(m_password != NULL);
        m_password->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    }
    else
    {
        Widget* password_box = getWidget("password-box");
        password_box->setCollapsed(true); // FIXME Doesn't reuse free space for other widgets
        m_options_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    }

    core::stringw difficulty = RaceManager::get()->getDifficultyName(
        server->getDifficulty());
    //I18N: In server info dialog
    getWidget<LabelWidget>("server-info-1")->setText(_("Difficulty: %s", difficulty), false);

    core::stringw mode = ServerConfig::getModeName(server->getServerMode());
    //I18N: In server info dialog
    getWidget<LabelWidget>("server-info-2")->setText(_("Game mode: %s", mode), false);

#ifndef SERVER_ONLY
    if (!server->getCountryCode().empty())
    {
        core::stringw country_name =
            translations->getLocalizedCountryName(server->getCountryCode());
        //I18N: In the server info dialog, show the server location with
        //country name (based on IP geolocation)
        getWidget<LabelWidget>("server-info-3")->setText(_("Server location: %s", country_name), false);
    }
#endif

    Track* t = server->getCurrentTrack();
    if (t)
    {
        core::stringw track_name = t->getName();
        //I18N: In server info dialog, showing the current track playing in server
        getWidget<LabelWidget>("server-info-4")->setText(_("Current track: %s", track_name), false);
    }

    auto& players = m_server->getPlayers();
    if (!players.empty())
    {
        // I18N: Show above the player list in server info dialog to
        // indicate the row meanings
        ListWidget* player_list = getWidget<ListWidget>("player-list");
        std::vector<ListWidget::ListCell> row;
        row.push_back(ListWidget::ListCell(_("Rank"), -1, 1, true));
        // I18N: Show above the player list in server info dialog, tell
        // the user name on server
        row.push_back(ListWidget::ListCell(_("Player"), -1, 2, true));
        // I18N: Show above the player list in server info dialog, tell
        // the scores of user calculated by player rankings
        row.push_back(ListWidget::ListCell(_("Scores"), -1, 1, true));
        // I18N: Show above the player list in server info dialog, tell
        // the user time played on server
        row.push_back(ListWidget::ListCell(_("Time played"),
            -1, 1, true));
        player_list->addItem("player", row);
        for (auto& r : players)
        {
            row.clear();
            row.push_back(ListWidget::ListCell(
                std::get<0>(r) == -1 ? L"-" :
                StringUtils::toWString(std::get<0>(r)), -1, 1, true));
            row.push_back(ListWidget::ListCell(std::get<1>(r), -1,
                2, true));
            row.push_back(ListWidget::ListCell(
                std::get<0>(r) == -1 ? L"-" :
                StringUtils::toWString(std::get<2>(r)), -1, 1, true));
            row.push_back(ListWidget::ListCell(
                StringUtils::toWString(std::get<3>(r)), -1, 1, true));
            player_list->addItem("player", row);
        }
    }
    else
    {
        getWidget("player-list")->setVisible(false);
    }
    if (m_server->getServerOwner() != 0)
        updateBookmarkStatus(false);
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
        if (m_password->getText().empty())
            return;
        ServerConfig::m_private_server_password =
            StringUtils::wideToUtf8(m_password->getText());
    }
    else
    {
        ServerConfig::m_private_server_password = "";
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
        else if (selection == m_join_widget->m_properties[PROP_ID])
        {
            m_join_server = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == m_bookmark_widget->m_properties[PROP_ID])
        {
            updateBookmarkStatus(true);
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
    m_join_server = true;
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
    if (m_password && m_password->getText().empty())
        m_join_widget->setActive(false);
    else if (!m_join_widget->isActivated())
        m_join_widget->setActive(true);

    // It's unsafe to delete from inside the event handler so we do it here
    if (m_self_destroy)
    {
        ModalDialog::dismiss();
        return;
    }
    if (m_join_server)
        requestJoin();
}   // onUpdate

// -----------------------------------------------------------------------------
void ServerInfoDialog::updateBookmarkStatus(bool change_bookmark)
{
    const std::string& key = m_server->getBookmarkKey();
    std::map<std::string, uint32_t>& bookmarks =
        UserConfigParams::m_server_bookmarks;
    std::map<std::string, uint32_t>& bookmarks_order =
        UserConfigParams::m_server_bookmarks_order;
    auto it = bookmarks.find(key);
    if (it == bookmarks.end())
    {
        if (change_bookmark)
        {
            bookmarks[key] = StkTime::getTimeSinceEpoch();
            uint32_t max_id = 0;
            for (auto& order : bookmarks_order)
            {
                if (order.second > max_id)
                    max_id = order.second;
            }
            max_id += 1;
            bookmarks_order[key] = max_id;
            m_server->setBookmarkID(max_id);
            m_bookmark_widget->setLabel(_("Remove from bookmarks"));
            m_bookmark_widget->setImage(m_remove_icon);
        }
    }
    else
    {
        if (change_bookmark)
        {
            bookmarks.erase(key);
            bookmarks_order.erase(key);
            m_bookmark_widget->setLabel(_("Bookmark this server"));
            m_bookmark_widget->setImage(m_bookmark_icon);
        }
        else
        {
            m_bookmark_widget->setLabel(_("Remove from bookmarks"));
            m_bookmark_widget->setImage(m_remove_icon);
        }
    }
    GUIEngine::ListWidget* w =
        ServerSelection::getInstance()->getServerList();
    CGUISTKListBox* box = NULL;
    if (w)
        box = w->getIrrlichtElement<CGUISTKListBox>();
    int old_pos = -1;
    if (box)
        old_pos = box->getScrollBar()->getPos();
    ServerSelection::getInstance()->copyFromServerList();
    if (box)
        box->getScrollBar()->setPos(old_pos);
}   // updateBookmarkStatus

// -----------------------------------------------------------------------------
void ServerInfoDialog::beforeAddingWidgets()
{
    m_bookmark_widget = getWidget<IconButtonWidget>("bookmark");
    assert(m_bookmark_widget != NULL);
    if (m_server->getServerOwner() == 0)
        m_bookmark_widget->setVisible(false);
}
