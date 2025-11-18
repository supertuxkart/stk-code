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

#include "states_screens/online/networking_lobby.hpp"

#include <cmath>
#include <algorithm>
#include <string>

#include "addons/addons_manager.hpp"
#include "config/user_config.hpp"
#include "config/player_manager.hpp"
#include "font/font_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/CGUISpriteBank.hpp"
#include "guiengine/emoji_keyboard.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widgets/CGUIEditBox.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "input/device_manager.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "network/game_setup.hpp"
#include "network/race_event_manager.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/protocols/connect_to_server.hpp"
#include "network/protocols/game_protocol.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/server.hpp"
#include "network/stk_host.hpp"
#include "network/network_timer_synchronizer.hpp"
#include "online/link_helper.hpp"
#include "states_screens/dialogs/addons_pack.hpp"
#include "states_screens/dialogs/splitscreen_player_dialog.hpp"
#include "states_screens/dialogs/network_player_dialog.hpp"
#include "states_screens/dialogs/server_configuration_dialog.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <IGUIEnvironment.h>
#include <IGUIStaticText.h>
#include <IGUIScrollBar.h>

using namespace Online;
using namespace GUIEngine;

/** This is the lobby screen that is shown on all clients, but not on the
 *  server. It shows currently connected clients, and allows the 'master'
 *  client (i.e. the stk instance that created the server) to control the
 *  server. This especially means that it can initialise the actual start
 *  of the racing.
 *  This class is responsible for creating the ActivePlayers data structure
 *  for all local players (the ActivePlayer maps device to player, i.e.
 *  controls which device is used by which player). Note that a server
 *  does not create an instance of this class and will create the ActivePlayer
 *  data structure in LobbyProtocol::loadWorld().
 */
// ----------------------------------------------------------------------------
NetworkingLobby::NetworkingLobby() : Screen("online/networking_lobby.stkgui")
{
    m_server_info_height = 0;
    m_header_text_width = 0;

    m_back_widget = NULL;
    //I18N: In the networking lobby
    m_header_text = _("Lobby");
    m_header = NULL;
    m_text_bubble = NULL;
    m_timeout_message = NULL;
    m_start_button = NULL;
    m_player_list = NULL;
    m_chat_box = NULL;
    m_send_button = NULL;
    m_icon_bank = NULL;
    m_reload_server_info = false;
    m_addon_install = NULL;
    
    // Chat scrollbar initialization
    m_chat_scrollbar = NULL;
    m_chat_scroll_pos = 0;
    m_chat_auto_scroll = true;

    // Allows one to update chat and counter even if dialog window is opened
    setUpdateInBackground(true);
}   // NetworkingLobby

// ----------------------------------------------------------------------------

void NetworkingLobby::loadedFromFile()
{
    m_header = getWidget<LabelWidget>("lobby-text");
    assert(m_header != NULL);

    m_back_widget = getWidget<IconButtonWidget>("back");
    assert(m_back_widget != NULL);

    m_start_button = getWidget<IconButtonWidget>("start");
    assert(m_start_button!= NULL);

    m_config_button = getWidget<IconButtonWidget>("config");
    assert(m_config_button!= NULL);

    m_text_bubble = getWidget<LabelWidget>("text");
    assert(m_text_bubble != NULL);

    m_timeout_message = getWidget<LabelWidget>("timeout-message");
    assert(m_timeout_message != NULL);

    m_chat_box = getWidget<TextBoxWidget>("chat");
    assert(m_chat_box != NULL);

    m_send_button = getWidget<ButtonWidget>("send");
    assert(m_send_button != NULL);

    m_emoji_button = getWidget<ButtonWidget>("emoji");
    assert(m_emoji_button != NULL);

    m_icon_bank = new irr::gui::STKModifiedSpriteBank(GUIEngine::getGUIEnv());
    video::ITexture* icon_1 = irr_driver->getTexture
        (file_manager->getAsset(FileManager::GUI_ICON, "crown.png"));
    video::ITexture* icon_2 = irr_driver->getTexture
        (file_manager->getAsset(FileManager::GUI_ICON, "difficulty_medium.png"));
    video::ITexture* icon_3 = irr_driver->getTexture
        (file_manager->getAsset(FileManager::GUI_ICON, "main_help.png"));
    video::ITexture* icon_4 = irr_driver->getTexture
        (file_manager->getAsset(FileManager::GUI_ICON, "hourglass.png"));
    video::ITexture* icon_5 = irr_driver->getTexture
        (file_manager->getAsset(FileManager::GUI_ICON, "green_check.png"));
    m_config_texture = irr_driver->getTexture
        (file_manager->getAsset(FileManager::GUI_ICON, "main_options.png"));
    m_spectate_texture = irr_driver->getTexture
        (file_manager->getAsset(FileManager::GUI_ICON, "screen_other.png"));
    video::ITexture* icon_6 = irr_driver->getTexture
        (file_manager->getAsset(FileManager::GUI_ICON, "robot.png"));
    m_addon_texture = irr_driver->getTexture
        (file_manager->getAsset(FileManager::GUI_ICON, "package-update.png"));
    m_icon_bank->addTextureAsSprite(icon_1);
    m_icon_bank->addTextureAsSprite(icon_2);
    m_icon_bank->addTextureAsSprite(icon_3);
    m_icon_bank->addTextureAsSprite(icon_4);
    m_icon_bank->addTextureAsSprite(icon_5);
    m_icon_bank->addTextureAsSprite(m_spectate_texture);
    m_icon_bank->addTextureAsSprite(icon_6);

    m_icon_bank->setScale(1.0f / 96.0f);
    m_icon_bank->setTargetIconSize(128, 128);
    
    // Initialize chat scrollbar after text bubble is loaded
    initChatScrollbar();
}   // loadedFromFile

// ---------------------------------------------------------------------------
void NetworkingLobby::beforeAddingWidget()
{
} // beforeAddingWidget

// ----------------------------------------------------------------------------
/** This function is a callback from the parent class, and is called each time
 *  this screen is shown to initialise the screen. This class is responsible
 *  for creating the active players and binding them to the right device.
 */
void NetworkingLobby::init()
{
    Screen::init();

    m_player_list = getWidget<ListWidget>("players");
    assert(m_player_list!= NULL);

    m_server_configurable = false;
    m_player_names.clear();
    m_has_auto_start_in_server = false;
    m_client_live_joinable = false;
    m_assigned_players = false;
    m_addon_install = NULL;
    m_ping_update_timer = 0;
    m_start_timeout = std::numeric_limits<float>::max();
    m_cur_starting_timer = std::numeric_limits<int64_t>::max();
    m_min_start_game_players = 0;
    m_timeout_message->setVisible(false);

    m_start_text = _("Start race");
    //I18N: In the networking lobby, ready button is to allow player to tell
    //server that he is ready for next game for owner less server
    m_ready_text = _("Ready");
    //I18N: Live join is displayed in networking lobby to allow players
    //to join the current started in-progress game
    m_live_join_text = _("Live join");
    //I18N: In networking lobby to configuration server settings
    m_configuration_text = _("Configuration");
    //I18N: Spectate is displayed in networking lobby to allow players
    //to join the current started in-progress game
    m_spectate_text = _("Spectate");
    m_install_addon_text = _("Install addon");

    setHeader(m_header_text);
    m_server_info_height = GUIEngine::getFont()->getDimension(L"X").Height;
    m_start_button->setVisible(false);
    m_config_button->setVisible(false);
    m_state = LS_CONNECTING;
    m_chat_box->setVisible(false);
    m_chat_box->setActive(false);
    m_chat_box->setTextBoxType(TBT_CAP_SENTENCES);
    m_send_button->setVisible(false);
    m_send_button->setActive(false);
    // Unicode enter arrow
    m_send_button->setText(L"\u21B2");
    m_emoji_button->setVisible(false);
    m_emoji_button->setActive(false);
    // Unicode smile emoji
    m_emoji_button->setText(L"\u263A");

    // Connect to server now if we have saved players and not disconnected
    if (!LobbyProtocol::get<LobbyProtocol>() &&
        !NetworkConfig::get()->getNetworkPlayers().empty())
        std::make_shared<ConnectToServer>(m_joined_server)->requestStart();

    if (NetworkConfig::get()->getNetworkPlayers().empty())
    {
        m_state = LS_ADD_PLAYERS;
    }
    else if (NetworkConfig::get()->isClient())
    {
        m_chat_box->clearListeners();
        if (UserConfigParams::m_lobby_chat)
        {
            m_chat_box->addListener(this);
            m_chat_box->setText("");
            m_chat_box->setVisible(true);
            m_chat_box->setActive(true);
            m_send_button->setVisible(true);
            m_send_button->setActive(true);
            m_emoji_button->setVisible(true);
            m_emoji_button->setActive(true);
        }
        else
        {
            m_chat_box->setText(
                _("Chat is disabled, enable in options menu."));
            m_chat_box->setVisible(true);
            m_chat_box->setActive(false);
            m_send_button->setVisible(true);
            m_send_button->setActive(false);
            m_emoji_button->setVisible(true);
            m_emoji_button->setActive(false);
        }
        if (auto cl = LobbyProtocol::get<ClientLobby>())
        {
            if (cl->isLobbyReady())
                updatePlayers();
        }
    }
#ifndef SERVER_ONLY
    if (m_text_bubble && !m_chat_scrollbar)
    {
        initChatScrollbar();
    }
    
    // If we have chat content, restore scroll position (scroll to bottom if auto-scroll enabled)
    // This handles both new scrollbar creation and existing scrollbar restoration
    // This is called when returning from a game, so we want to preserve chat history
    if (m_chat_scrollbar && !m_server_info_full.empty())
    {
        // Re-break layouts with current dimensions (in case window was resized)
        updateChatScrollbar();
        // Always scroll to bottom when returning from game (auto-scroll enabled)
        m_chat_auto_scroll = true;
        scrollChatToBottom();
    }
    // If scrollbar exists but we have no content, ensure it's hidden
    else if (m_chat_scrollbar && m_server_info_full.empty())
    {
        m_chat_scrollbar->setVisible(false);
        m_chat_scroll_pos = 0;
        m_chat_auto_scroll = true;
    }
    
    gui::IGUIStaticText* st =
        m_text_bubble->getIrrlichtElement<gui::IGUIStaticText>();
    NetworkingLobby* lobby = this;
    st->setMouseCallback([lobby](gui::IGUIStaticText* text, SEvent::SMouseInput mouse)->bool
    {
        // Handle mouse wheel scrolling
        if (mouse.Event == EMIE_MOUSE_WHEEL)
        {
            if (lobby->m_chat_scrollbar && lobby->m_chat_scrollbar->isVisible() && 
                lobby->m_text_bubble)
            {
                irr::SEvent scroll_event;
                scroll_event.EventType = irr::EET_MOUSE_INPUT_EVENT;
                scroll_event.MouseInput = mouse;
                lobby->handleChatScrollEvent(scroll_event);
                return true;
            }
            return false;
        }
        
        if (mouse.Event == EMIE_LMOUSE_PRESSED_DOWN)
        {
            std::shared_ptr<std::u32string> s;
            int glyph_idx = -1;
            int cluster = text->getCluster(mouse.X, mouse.Y, &s, &glyph_idx);
            if (cluster == -1 || (unsigned)cluster > s->size())
                return false;
            const std::vector<gui::GlyphLayout>& gls = text->getGlyphLayouts();
            int start_cluster = -1;
            const std::u32string ia = U"/installaddon ";
            std::u32string url = gui::extractURLFromGlyphLayouts(gls,
                glyph_idx, &start_cluster);
            if (url.empty())
                goto handle_player_message_copy;
            if ((unsigned)start_cluster >= ia.size() &&
                s->substr(start_cluster - (int)ia.size(), ia.size()) == ia)
            {
                AddonsPack::install(StringUtils::utf32ToUtf8(url));
            }
            else
            {
                Online::LinkHelper::openURL(StringUtils::utf32ToUtf8(url));
            }
            return true;
handle_player_message_copy:
            size_t start = s->substr(0, cluster).rfind(U'\n');
            size_t end = s->substr(cluster, s->size()).find(U'\n');
            if (start == std::string::npos)
                start = 0;
            else
                start += 1; // Skip newline character
            if (end == std::string::npos)
                end = s->size();
            else
                end += cluster - start;

            std::u32string substr = s->substr(start, end);
            int local_pos = cluster - (int)start;
            if ((size_t)local_pos > substr.size())
                return false;
            for (auto& p : NetworkingLobby::getInstance()->m_player_names)
            {
                size_t colon = substr.substr(0, local_pos).rfind(U": ");
                if (colon == std::string::npos)
                    continue;
                std::u32string player_name = substr.substr(0, colon);
                if (player_name.empty())
                    continue;
                int padding = 2;
                // RTL handling
                if (player_name[0] == 0x200F || player_name[0] == 0x200E)
                {
                    player_name = player_name.substr(1, player_name.size() - 1);
                    padding++;
                }
                if (StringUtils::wideToUtf32(p.second.m_user_name)
                    .find(player_name) != std::string::npos)
                {
                    GUIEngine::getGUIEnv()->getOSOperator()->copyToClipboard(
                        StringUtils::utf32ToUtf8(
                        substr.substr(player_name.size() + padding)).c_str());
                    return true;
                }
            }
            GUIEngine::getGUIEnv()->getOSOperator()->copyToClipboard(
                StringUtils::utf32ToUtf8(substr).c_str());
            return true;
        }
        return false;
    });
#endif
}   // init

// ----------------------------------------------------------------------------
void NetworkingLobby::addMoreServerInfo(core::stringw info)
{
#ifndef SERVER_ONLY
    // Reserve space for scrollbar when breaking layouts
    s32 scrollbar_width = getChatScrollbarWidth();
    const unsigned box_width = core::max_(1u, 
        (unsigned)core::max_(1, (s32)m_text_bubble->getDimension().Width - scrollbar_width));
    std::vector<GlyphLayout> cur_info;
    font_manager->initGlyphLayouts(info, cur_info, gui::SF_DISABLE_CACHE |
        gui::SF_ENABLE_CLUSTER_TEST);

    // Highlight addon name from /installaddon
    std::set<int> addon_names;
    const std::u32string ia = U"/installaddon ";
    size_t pos = 0;
    std::shared_ptr<std::u32string> orig_str;
    for (gui::GlyphLayout& gl : cur_info)
    {
        orig_str = gl.orig_string;
        if (orig_str)
            break;
    }
    if (!orig_str)
        goto break_glyph_layouts;

    pos = orig_str->find(ia, 0);
    while (pos != std::u32string::npos)
    {
        size_t newline_pos = orig_str->find(U'\n', pos + ia.size());
        size_t space_pos = orig_str->find(U' ', pos + ia.size());
        size_t end_pos = std::u32string::npos;
        if (newline_pos != std::u32string::npos ||
            space_pos != std::u32string::npos)
        {
            if (space_pos > newline_pos)
                end_pos = newline_pos;
            else
                end_pos = space_pos;
        }
        else
            end_pos = orig_str->size();
        std::u32string name = orig_str->substr(pos + ia.size(),
            end_pos - pos - ia.size());
        if (name.rfind(U"https://", 0) == 0 ||
            name.rfind(U"http://", 0) == 0)
        {
            pos = orig_str->find(ia, pos + ia.size());
            continue;
        }
        for (size_t p = pos + ia.size(); p < end_pos; p++)
            addon_names.insert((int)p);
        pos = orig_str->find(ia, pos + ia.size());
    }

    for (gui::GlyphLayout& gl : cur_info)
    {
        for (int c : gl.cluster)
        {
            if (addon_names.find(c) != addon_names.end())
            {
                gl.flags |= gui::GLF_URL;
                continue;
            }
        }
    }

break_glyph_layouts:
    gui::IGUIFont* font = GUIEngine::getFont();
    gui::breakGlyphLayouts(cur_info, box_width,
        font->getInverseShaping(), font->getScale());
    
    // Add to full history (for scrolling)
    m_server_info_full.insert(m_server_info_full.end(), cur_info.begin(),
        cur_info.end());
    gui::GlyphLayout new_line = { 0 };
    new_line.flags = gui::GLF_NEWLINE;
    m_server_info_full.push_back(new_line);
    
    // Check if we should auto-scroll BEFORE updating scrollbar
    // (we need to check the old position before new content is added)
    bool was_at_bottom = isChatAtBottom();
    
    // Update scrollbar with new content (only if scrollbar exists)
    if (m_chat_scrollbar)
    {
        updateChatScrollbar();
        
        // If auto-scroll is enabled and we were at bottom, scroll to bottom
        // This must happen AFTER updateChatScrollbar() so the max is updated
        if (m_chat_auto_scroll && was_at_bottom)
        {
            scrollChatToBottom();
        }
        else
        {
            // Maintain the current scroll position, clamp to valid range
            s32 max_pos = m_chat_scrollbar->getMax();
            m_chat_scroll_pos = core::clamp(m_chat_scroll_pos, 0, max_pos);
            m_chat_scrollbar->setPos(m_chat_scroll_pos);
        }
    }
    
    filterGlyphLayoutsForScroll();
    
    updateServerInfos();
#endif
}   // addMoreServerInfo

// ----------------------------------------------------------------------------
void NetworkingLobby::onResize()
{
    Screen::onResize();
    
    // Preserve relative scroll position
    float relative_pos = 0.0f;
    if (m_chat_scrollbar && m_chat_scrollbar->getMax() > 0)
    {
        relative_pos = (float)m_chat_scroll_pos / (float)m_chat_scrollbar->getMax();
    }
    
    // Reserve space for scrollbar when breaking layouts
    s32 scrollbar_width = getChatScrollbarWidth();
    const unsigned box_width = core::max_(1u, 
        (unsigned)core::max_(1, (s32)m_text_bubble->getDimension().Width - scrollbar_width));
    gui::IGUIFont* font = GUIEngine::getFont();
    
    if (!font || font->getHeightPerLine() <= 0)
        return;
    
    // Re-break full history with new width
    gui::breakGlyphLayouts(m_server_info_full, box_width,
        font->getInverseShaping(), font->getScale());
    
    // Update scrollbar with new dimensions
    updateChatScrollbar();
    
    // Restore relative scroll position
    if (m_chat_scrollbar && m_chat_scrollbar->getMax() > 0)
    {
        m_chat_scroll_pos = (s32)(relative_pos * m_chat_scrollbar->getMax());
        m_chat_scrollbar->setPos(m_chat_scroll_pos);
    }
    
    // Update scrollbar position
    if (m_chat_scrollbar && m_text_bubble && m_text_bubble->getIrrlichtElement())
    {
        irr::core::rect<s32> text_rect = m_text_bubble->getIrrlichtElement()->getAbsolutePosition();
        irr::gui::IGUISkin* skin = GUIEngine::getGUIEnv()->getSkin();
        s32 scrollbar_width = skin ? skin->getSize(irr::gui::EGDS_SCROLLBAR_SIZE) : 20;
        
        irr::core::rect<s32> scrollbar_rect(
            text_rect.LowerRightCorner.X - scrollbar_width,
            text_rect.UpperLeftCorner.Y,
            text_rect.LowerRightCorner.X,
            text_rect.LowerRightCorner.Y
        );
        m_chat_scrollbar->setRelativePosition(scrollbar_rect);
    }
    
    // Filter glyph layouts for current scroll position
    filterGlyphLayoutsForScroll();
    updateServerInfos();

    int header_text_width =
        GUIEngine::getTitleFont()->getDimension(m_header_text.c_str()).Width;
    if ((m_header->m_w < header_text_width && m_header->getScrollSpeed() == 0.0f) ||
        (m_header->m_w >= header_text_width && m_header->getScrollSpeed() != 0.0f))
    {
        m_header->getIrrlichtElement()->remove();
        GUIEngine::getGUIEnv()->getRootGUIElement()->setChildEnd(m_back_widget->getIrrlichtElement());
        m_header->setScrollSpeed(m_header->m_w < header_text_width ? 0.5f: 0.0f);
        m_header->add();
        m_header->setText(m_header_text, true);
        GUIEngine::getGUIEnv()->getRootGUIElement()->setChildEnd(NULL);
    }
    m_header_text_width = header_text_width;
}   // onResize

// ----------------------------------------------------------------------------
void NetworkingLobby::updateServerInfos()
{
#ifndef SERVER_ONLY
    if (GUIEngine::getCurrentScreen() != this)
        return;

    gui::IGUIStaticText* st =
        m_text_bubble->getIrrlichtElement<gui::IGUIStaticText>();
    st->setUseGlyphLayoutsOnly(true);
    st->setGlyphLayouts(m_server_info);
    
    // Update scrollbar position
    if (m_chat_scrollbar)
    {
        updateChatScrollbar();
    }
#endif
}   // updateServerInfos

// ----------------------------------------------------------------------------
s32 NetworkingLobby::getChatScrollbarWidth() const
{
    irr::gui::IGUISkin* skin = GUIEngine::getGUIEnv()->getSkin();
    return skin ? skin->getSize(irr::gui::EGDS_SCROLLBAR_SIZE) : 20;
}   // getChatScrollbarWidth

// ----------------------------------------------------------------------------
void NetworkingLobby::filterGlyphLayoutsForScroll()
{
#ifndef SERVER_ONLY
    if (!m_text_bubble || !m_chat_scrollbar || m_server_info_full.empty())
    {
        m_server_info.clear();
        return;
    }
    
    gui::IGUIFont* font = GUIEngine::getFont();
    if (!font)
    {
        m_server_info.clear();
        return;
    }
    
    // Reserve space for scrollbar when breaking layouts
    s32 scrollbar_width = getChatScrollbarWidth();
    const unsigned box_width = core::max_(1u, 
        (unsigned)core::max_(1, (s32)m_text_bubble->getDimension().Width - scrollbar_width));
    const float box_height = m_text_bubble->getDimension().Height;
    s32 line_height = font->getHeightPerLine();
    
    if (box_height <= 0.0f || line_height <= 0)
    {
        m_server_info.clear();
        return;
    }
    
    s32 scroll_offset = m_chat_scroll_pos;
    
    std::vector<gui::GlyphLayout> temp_layouts = m_server_info_full;
    gui::breakGlyphLayouts(temp_layouts, box_width,
        font->getInverseShaping(), font->getScale());
    
    // Group glyphs by line (lines end with NEWLINE flag)
    struct LineInfo {
        size_t start_idx;
        size_t end_idx;  // inclusive
        s32 line_top;
        s32 line_bottom;
    };
    std::vector<LineInfo> lines;
    s32 current_height = 0;
    size_t line_start = 0;
    
    for (size_t i = 0; i < temp_layouts.size(); i++)
    {
        if ((temp_layouts[i].flags & gui::GLF_NEWLINE) != 0)
        {
            // End of line
            LineInfo line;
            line.start_idx = line_start;
            line.end_idx = i;  // inclusive
            line.line_top = current_height;
            line.line_bottom = current_height + line_height;
            lines.push_back(line);
            
            current_height = line.line_bottom;
            line_start = i + 1;
        }
    }
    
    // If there's content after the last newline, add it as a line
    if (line_start < temp_layouts.size())
    {
        LineInfo line;
        line.start_idx = line_start;
        line.end_idx = temp_layouts.size() - 1;
        line.line_top = current_height;
        line.line_bottom = current_height + line_height;
        lines.push_back(line);
    }
    
    // Find visible lines based on scroll offset
    m_server_info.clear();
    s32 visible_top = scroll_offset;
    s32 visible_bottom = scroll_offset + (s32)box_height;
    
    for (const LineInfo& line : lines)
    {
        // Check if line overlaps with visible area
        if (line.line_bottom > visible_top && line.line_top < visible_bottom)
        {
            // Include all glyphs for this line
            for (size_t j = line.start_idx; j <= line.end_idx; j++)
            {
                m_server_info.push_back(temp_layouts[j]);
            }
        }
        // If we've passed the visible area, we can stop
        else if (line.line_top >= visible_bottom)
        {
            break;
        }
    }
#endif
}   // filterGlyphLayoutsForScroll

// ----------------------------------------------------------------------------
void NetworkingLobby::onUpdate(float delta)
{
    m_addon_install = NULL;
    if (NetworkConfig::get()->isServer() || !STKHost::existHost())
        return;

    if (m_header->getText() != m_header_text)
    {
        m_header_text_width =
            GUIEngine::getTitleFont()->getDimension(m_header_text.c_str()).Width;
        m_header->getIrrlichtElement()->remove();
        if (m_header_text_width > m_header->m_w)
        {
            m_header->setScrollSpeed(0.5f);
            m_header->add();
            m_header->setText(m_header_text, true);
        }
        else
        {
            m_header->setScrollSpeed(0);
            m_header->add();
            m_header->setText(m_header_text, true);
        }
        // Make sure server name is not clickable for URL
        m_header->getIrrlichtElement<IGUIStaticText>()->setMouseCallback(nullptr);
    }

    if (m_header_text_width > m_header->m_w)
    {
        m_header->update(delta);
        if (m_header->scrolledOff())
            m_header->setText(m_header->getText(), true);
    }

    if (m_reload_server_info)
    {
        m_reload_server_info = false;
        updateServerInfos();
    }
    
    // Check if scrollbar position changed
    if (m_chat_scrollbar && m_chat_scrollbar->isVisible())
    {
        s32 current_pos = m_chat_scrollbar->getPos();
        if (current_pos != m_chat_scroll_pos)
        {
            m_chat_scroll_pos = current_pos;
            m_chat_auto_scroll = false;
            updateChatScrollPosition();
        }
    }

    if (m_has_auto_start_in_server)
    {
        m_start_button->setLabel(m_ready_text);
    }
    else
        m_start_button->setLabel(m_start_text);

    m_start_button->setVisible(false);

    m_config_button->setLabel(m_configuration_text);
    m_config_button->setVisible(false);
    m_config_button->setImage(m_config_texture);
    m_client_live_joinable = false;

    if (m_player_list && StkTime::getMonoTimeMs() > m_ping_update_timer)
    {
        m_ping_update_timer = StkTime::getMonoTimeMs() + 2000;
        updatePlayerPings();
    }

    auto cl = LobbyProtocol::get<ClientLobby>();
    if (cl && UserConfigParams::m_lobby_chat)
    {
        if (cl->serverEnabledChat() && !m_send_button->isActivated())
        {
            m_chat_box->setActive(true);
            m_send_button->setActive(true);
            m_emoji_button->setActive(true);
        }
        else if (!cl->serverEnabledChat() && m_send_button->isActivated())
        {
            m_chat_box->setActive(false);
            m_send_button->setActive(false);
            m_emoji_button->setActive(false);
        }
    }
    if (cl && cl->isWaitingForGame())
    {
        m_start_button->setVisible(false);
        m_timeout_message->setVisible(true);
        auto progress = cl->getGameStartedProgress();
        core::stringw msg;
        core::stringw current_track;
        Track* t = cl->getPlayingTrack();
        if (t)
            current_track = t->getName();
        std::string missing_addon_track_id;
        // Show addon identity so player can install it live in lobby
        if (current_track.empty())
        {
            std::string track_id = cl->getPlayingTrackIdent();
            if (StringUtils::startsWith(track_id, "addon_"))
            {
                missing_addon_track_id = track_id.substr(6).c_str();
                current_track = missing_addon_track_id.c_str();
            }
        }
        if (progress.first != std::numeric_limits<uint32_t>::max())
        {
            if (!current_track.empty())
            {
                //I18N: In the networking lobby, show when player is required to
                //wait before the current game finish with remaining time,
                //showing the current track name inside bracket
                msg = _("Please wait for the current game's (%s) end, "
                    "estimated remaining time: %s.", current_track,
                    StringUtils::timeToString((float)progress.first).c_str());
            }
            else
            {
                //I18N: In the networking lobby, show when player is required
                //to wait before the current game finish with remaining time
                msg = _("Please wait for the current game's end, "
                    "estimated remaining time: %s.",
                    StringUtils::timeToString((float)progress.first).c_str());
            }
        }
        else if (progress.second != std::numeric_limits<uint32_t>::max())
        {
            if (!current_track.empty())
            {
                //I18N: In the networking lobby, show when player is required
                //to wait before the current game finish with progress in
                //percent, showing the current track name inside bracket
                msg = _("Please wait for the current game's (%s) end, "
                    "estimated progress: %s%.", current_track,
                    progress.second);
            }
            else
            {
                //I18N: In the networking lobby, show when player is required
                //to wait before the current game finish with progress in
                //percent
                msg = _("Please wait for the current game's end, "
                    "estimated progress: %d%.", progress.second);
            }
        }
        else
        {
            //I18N: In the networking lobby, show when player is required to
            //wait before the current game finish
            msg = _("Please wait for the current game's end.");
        }

        // You can live join or spectator if u have the current play track
        // and network timer is synchronized, and no game protocols exist
        bool no_gep = !RaceEventManager::get() ||
            RaceEventManager::get()->protocolStopped();
        bool no_gp = GameProtocol::emptyInstance();
        if (t &&
            STKHost::get()->getNetworkTimerSynchronizer()->isSynchronised() &&
            cl->isServerLiveJoinable() && no_gep && no_gp)
        {
            m_client_live_joinable = true;
        }
        else
            m_client_live_joinable = false;

        m_timeout_message->setText(msg, false);
        m_cur_starting_timer = std::numeric_limits<int64_t>::max();
#ifndef SERVER_ONLY
        if (!GUIEngine::ModalDialog::isADialogActive() &&
            !ScreenKeyboard::isActive())
        {
            m_addon_install = addons_manager->getAddon(
                Addon::createAddonId(missing_addon_track_id));
            if (m_addon_install)
            {
                m_config_button->setLabel(m_install_addon_text);
                m_config_button->setImage(m_addon_texture);
                m_config_button->setVisible(true);
                return;
            }
        }
#endif

        if (m_client_live_joinable)
        {
            if (RaceManager::get()->supportsLiveJoining())
            {
                m_start_button->setVisible(true);
                m_start_button->setLabel(m_live_join_text);
            }
            else
                m_start_button->setVisible(false);
            m_config_button->setLabel(m_spectate_text);
            m_config_button->setImage(m_spectate_texture);
            m_config_button->setVisible(true);
        }
        return;
    }

    if (m_has_auto_start_in_server && m_player_list)
    {
        m_timeout_message->setVisible(true);
        unsigned cur_player = m_player_list->getItemCount();
        if (cur_player >= m_min_start_game_players &&
            m_cur_starting_timer == std::numeric_limits<int64_t>::max())
        {
            m_cur_starting_timer = (int64_t)StkTime::getMonoTimeMs() +
                (int64_t)(m_start_timeout * 1000.0);
        }
        else if (cur_player < m_min_start_game_players)
        {
            m_cur_starting_timer = std::numeric_limits<int64_t>::max();
            //I18N: In the networking lobby, display the number of players
            //required to start a game for owner-less server
            core::stringw msg =
                _P("Game will start if there is more than %d player.",
               "Game will start if there are more than %d players.",
               (int)(m_min_start_game_players - 1));
            m_timeout_message->setText(msg, false);
        }

        if (m_cur_starting_timer != std::numeric_limits<int64_t>::max())
        {
            int64_t remain = (m_cur_starting_timer -
                (int64_t)StkTime::getMonoTimeMs()) / 1000;
            if (remain < 0)
                remain = 0;
            //I18N: In the networking lobby, display the starting timeout
            //for owner-less server to begin a game
            core::stringw msg = _P("Starting after %d second, "
                "or once everyone has pressed the 'Ready' button.",
                "Starting after %d seconds, "
                "or once everyone has pressed the 'Ready' button.",
                (int)remain);
            m_timeout_message->setText(msg, false);
        }
    }
    else
    {
        m_timeout_message->setVisible(false);
    }

    if (m_state == LS_ADD_PLAYERS)
    {
        m_text_bubble->getIrrlichtElement<gui::IGUIStaticText>()
            ->setUseGlyphLayoutsOnly(false);
        m_text_bubble->setText(_("Everyone:\nPress the 'Select' button to "
                                          "join the game"), false);
        m_start_button->setVisible(false);
        if (!GUIEngine::ModalDialog::isADialogActive())
        {
            input_manager->getDeviceManager()->setAssignMode(DETECT_NEW);
            input_manager->getDeviceManager()->mapFireToSelect(true);
        }
        return;
    }

    m_start_button->setVisible(false);
    if (!cl || !cl->isLobbyReady())
    {
        m_text_bubble->getIrrlichtElement<gui::IGUIStaticText>()
            ->setUseGlyphLayoutsOnly(false);
        core::stringw connect_msg;
        if (m_joined_server)
        {
            connect_msg = StringUtils::loadingDots(
                _("Connecting to server %s", m_joined_server->getName()));
        }
        else
        {
            connect_msg =
                StringUtils::loadingDots(_("Finding a quick play server"));
        }
        m_text_bubble->setText(connect_msg, false);
        m_start_button->setVisible(false);
    }

    m_config_button->setVisible(STKHost::get()->isAuthorisedToControl() &&
        m_server_configurable);

    if (STKHost::get()->isAuthorisedToControl() ||
        (m_has_auto_start_in_server &&
        m_cur_starting_timer != std::numeric_limits<int64_t>::max()))
    {
        m_start_button->setVisible(true);
    }

}   // onUpdate

// ----------------------------------------------------------------------------
void NetworkingLobby::updatePlayerPings()
{
    auto peer_pings = STKHost::get()->getPeerPings();
    for (auto& p : m_player_names)
    {
        core::stringw name_with_ping = p.second.m_user_name;
        const core::stringw& flag = StringUtils::getCountryFlag(
            p.second.m_country_code);
        if (!flag.empty())
        {
            name_with_ping += L" ";
            name_with_ping += flag;
        }
        auto host_online_ids = StringUtils::splitToUInt(p.first, '_');
        if (host_online_ids.size() != 3)
            continue;
        unsigned ping = 0;
        uint32_t host_id = host_online_ids[0];
        if (peer_pings.find(host_id) != peer_pings.end())
            ping = peer_pings.at(host_id);
        if (ping != 0)
        {
            name_with_ping = StringUtils::insertValues(L"%s (%dms)",
                name_with_ping, ping);
        }
        else
            continue;
        int id = m_player_list->getItemID(p.first);
        if (id != -1)
        {
            m_player_list->renameItem(id, name_with_ping, p.second.m_icon_id);
            // Don't show chosen team color for spectator
            if (p.second.isSpectator())
                m_player_list->markItemRed(id, false/*red*/);
            else if (p.second.m_kart_team == KART_TEAM_RED)
                m_player_list->markItemRed(id);
            else if (p.second.m_kart_team == KART_TEAM_BLUE)
                m_player_list->markItemBlue(id);
        }
    }
}   // updatePlayerPings

// ----------------------------------------------------------------------------
bool NetworkingLobby::onEnterPressed(const irr::core::stringw& text)
{
    if (auto cl = LobbyProtocol::get<ClientLobby>())
    {
        if (!text.empty())
        {
            if (text[0] == L'/' && text.size() > 1)
            {
                std::string cmd = StringUtils::wideToUtf8(text);
                cl->handleClientCommand(cmd.erase(0, 1));
            }
            else
                cl->sendChat(text, KART_TEAM_NONE);
        }
    }
    return true;
}   // onEnterPressed

// ----------------------------------------------------------------------------
void NetworkingLobby::eventCallback(Widget* widget, const std::string& name,
                                    const int playerID)
{
    if (name == m_back_widget->m_properties[PROP_ID])
    {
        StateManager::get()->escapePressed();
        return;
    }
    else if (name == m_player_list->m_properties[GUIEngine::PROP_ID])
    {
        auto host_online_local_ids = StringUtils::splitToUInt
            (m_player_list->getSelectionInternalName(), '_');
        if (host_online_local_ids.size() != 3)
        {
            return;
        }
        LobbyPlayer& lp =
            m_player_names.at(m_player_list->getSelectionInternalName());
        // For client server AI it doesn't make any sense to open the dialog
        // There is no way to kick or add handicap to them
        if (STKHost::get()->isClientServer() > 0 && lp.isAI())
            return;
        new NetworkPlayerDialog(host_online_local_ids[0],
            host_online_local_ids[1], host_online_local_ids[2],
            lp.m_user_name, lp.m_country_code, lp.m_kart_team != KART_TEAM_NONE,
            lp.m_handicap);
    }   // click on a user
    else if (name == m_send_button->m_properties[PROP_ID])
    {
        onEnterPressed(m_chat_box->getText());
        m_chat_box->setText("");
        m_chat_box->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    }   // send chat message
    else if (name == m_emoji_button->m_properties[PROP_ID] &&
        !ScreenKeyboard::isActive())
    {
        EmojiKeyboard* ek = new EmojiKeyboard(1.0f, 0.40f,
            m_chat_box->getIrrlichtElement<CGUIEditBox>());
        ek->init();
    }
    else if (name == m_start_button->m_properties[PROP_ID])
    {
        if (m_client_live_joinable)
        {
            auto cl = LobbyProtocol::get<ClientLobby>();
            if (cl)
                cl->startLiveJoinKartSelection();
        }
        else
        {
            // Send a message to the server to start
            NetworkString start(PROTOCOL_LOBBY_ROOM);
            start.addUInt8(LobbyProtocol::LE_REQUEST_BEGIN);
            STKHost::get()->sendToServer(&start, true);
        }
    }
    else if (name == m_config_button->m_properties[PROP_ID])
    {
#ifndef SERVER_ONLY
        if (m_addon_install)
        {
            AddonsPack::install(m_addon_install->getDirName());
            return;
        }
#endif
        auto cl = LobbyProtocol::get<ClientLobby>();
        if (m_client_live_joinable && cl)
        {
            NetworkString start(PROTOCOL_LOBBY_ROOM);
            start.setSynchronous(true);
            start.addUInt8(LobbyProtocol::LE_LIVE_JOIN)
                // is spectating
                .addUInt8(1);
            STKHost::get()->sendToServer(&start, true);
            return;
        }
        if (cl)
        {
            new ServerConfigurationDialog(
                RaceManager::get()->isSoccerMode() &&
                cl->getGameSetup()->isSoccerGoalTarget());
        }
    }
}   // eventCallback

// ----------------------------------------------------------------------------
void NetworkingLobby::unloaded()
{
    delete m_icon_bank;
    m_icon_bank = NULL;
}   // unloaded

// ----------------------------------------------------------------------------
void NetworkingLobby::tearDown()
{
    if (m_state == LS_ADD_PLAYERS)
    {
        UserConfigParams::m_enable_network_splitscreen = false;
        NetworkConfig::get()->cleanNetworkPlayers();
        NetworkConfig::get()->addNetworkPlayer(
            input_manager->getDeviceManager()->getLatestUsedDevice(),
            PlayerManager::getCurrentPlayer(), HANDICAP_NONE);
        NetworkConfig::get()->doneAddingNetworkPlayers();
    }

    gui::IGUIStaticText* st =
        m_text_bubble->getIrrlichtElement<gui::IGUIStaticText>();
    st->setMouseCallback(nullptr);
    m_player_list = NULL;
    m_joined_server.reset();
    m_header_text = _("Lobby");
    if (m_header)
        m_header->setText(m_header_text, true);
    m_header_text_width = 0;
    
    // Cleanup scrollbar
    if (m_chat_scrollbar)
    {
        m_chat_scrollbar->remove();
        m_chat_scrollbar->drop();
        m_chat_scrollbar = NULL;
    }
    m_chat_scroll_pos = 0;
    m_chat_auto_scroll = true;
    
    // Server has a dummy network lobby too
    if (!NetworkConfig::get()->isClient())
        return;
    input_manager->getDeviceManager()->mapFireToSelect(false);
    if (!m_assigned_players)
        input_manager->getDeviceManager()->setAssignMode(NO_ASSIGN);
}   // tearDown

// ----------------------------------------------------------------------------
bool NetworkingLobby::onEscapePressed()
{
    if (NetworkConfig::get()->isAddingNetworkPlayers())
        NetworkConfig::get()->cleanNetworkPlayers();
    m_joined_server.reset();
    m_header_text = _("Lobby");
    m_header_text_width = 0;
    
    // Reset chat scroll on disconnect
    m_server_info_full.clear();
    m_chat_scroll_pos = 0;
    m_chat_auto_scroll = true;
    if (m_chat_scrollbar)
        m_chat_scrollbar->setPos(0);
    
    input_manager->getDeviceManager()->mapFireToSelect(false);
    input_manager->getDeviceManager()->setAssignMode(NO_ASSIGN);
    STKHost::get()->shutdown();
    return true; // close the screen
}   // onEscapePressed

// ----------------------------------------------------------------------------
void NetworkingLobby::updatePlayers()
{
    // In GUI-less server this function will be called without proper
    // initialisation
    if (!m_player_list)
        return;

    std::string selected_name = m_player_list->getSelectionInternalName();
    m_player_list->clear();
    m_player_names.clear();

    auto cl = LobbyProtocol::get<ClientLobby>();
    if (!cl)
        return;

    const auto& players = cl->getLobbyPlayers();
    if (players.empty())
        return;

    irr::gui::STKModifiedSpriteBank* icon_bank = m_icon_bank;
    for (unsigned i = 0; i < players.size(); i++)
    {
        const LobbyPlayer& player = players[i];
        if (icon_bank)
        {
            m_player_list->setIcons(icon_bank);
            icon_bank = NULL;
        }
        KartTeam cur_team = player.m_kart_team;
        const std::string internal_name =
            StringUtils::toString(player.m_host_id) + "_" +
            StringUtils::toString(player.m_online_id) + "_" +
            StringUtils::toString(player.m_local_player_id);
        core::stringw player_name = player.m_user_name;
        const core::stringw& flag = StringUtils::getCountryFlag(
            player.m_country_code);
        if (!flag.empty())
        {
            player_name += L" ";
            player_name += flag;
        }
        m_player_list->addItem(internal_name, player_name,
            player.m_icon_id);
        // Don't show chosen team color for spectator
        if (player.isSpectator())
            m_player_list->markItemRed(i, false/*red*/);
        else if (cur_team == KART_TEAM_RED)
            m_player_list->markItemRed(i);
        else if (cur_team == KART_TEAM_BLUE)
            m_player_list->markItemBlue(i);
        m_player_names[internal_name] = player;
    }
    updatePlayerPings();
    if (!selected_name.empty())
    {
        int id = m_player_list->getItemID(selected_name);
        if (id != -1)
            m_player_list->setSelectionID(id);
    }
}   // updatePlayers

// ----------------------------------------------------------------------------
void NetworkingLobby::openSplitscreenDialog(InputDevice* device)
{
    new SplitscreenPlayerDialog(device);
}   // openSplitscreenDialog

// ----------------------------------------------------------------------------
void NetworkingLobby::addSplitscreenPlayer(irr::core::stringw name)
{
    if (!m_player_list)
        return;
    m_player_list->setIcons(m_icon_bank);
    m_player_list->addItem(StringUtils::wideToUtf8(name), name, 1);
}   // addSplitscreenPlayer

// ----------------------------------------------------------------------------
void NetworkingLobby::finishAddingPlayers()
{
    m_state = LS_CONNECTING;
    std::make_shared<ConnectToServer>(m_joined_server)->requestStart();
    m_start_button->setVisible(false);
    m_chat_box->clearListeners();
    if (UserConfigParams::m_lobby_chat)
    {
        m_chat_box->addListener(this);
        m_chat_box->setVisible(true);
        m_chat_box->setActive(true);
        m_send_button->setVisible(true);
        m_send_button->setActive(true);
        m_emoji_button->setVisible(true);
        m_emoji_button->setActive(true);
    }
    else
    {
        m_chat_box->setText(_("Chat is disabled, enable in options menu."));
        m_chat_box->setVisible(true);
        m_chat_box->setActive(false);
        m_send_button->setVisible(true);
        m_send_button->setActive(false);
        m_emoji_button->setVisible(true);
        m_emoji_button->setActive(false);
    }
}   // finishAddingPlayers

// ----------------------------------------------------------------------------
void NetworkingLobby::cleanAddedPlayers()
{
    if (!m_player_list)
        return;
    m_player_list->clear();
    m_player_names.clear();
}   // cleanAddedPlayers

// ----------------------------------------------------------------------------
void NetworkingLobby::initAutoStartTimer(bool grand_prix_started,
                                         unsigned min_players,
                                         float start_timeout,
                                         unsigned server_max_player)
{
    if (min_players == 0 || start_timeout == 0.0f)
        return;

    m_has_auto_start_in_server = true;
    m_min_start_game_players = grand_prix_started ? 0 : min_players;
    m_start_timeout = start_timeout;
}   // initAutoStartTimer

// ----------------------------------------------------------------------------
void NetworkingLobby::setStartingTimerTo(float t)
{
    m_cur_starting_timer =
        (int64_t)StkTime::getMonoTimeMs() + (int64_t)(t * 1000.0f);
}   // setStartingTimerTo

// ----------------------------------------------------------------------------
void NetworkingLobby::setJoinedServer(std::shared_ptr<Server> server)
{
    m_joined_server = server;
    m_server_info.clear();
    m_server_info_full.clear();
    m_header_text = _("Lobby");
    // Reset scroll position when joining new server
    m_chat_scroll_pos = 0;
    m_chat_auto_scroll = true;
    if (m_chat_scrollbar)
        m_chat_scrollbar->setPos(0);
}   // setJoinedServer

// ----------------------------------------------------------------------------
void NetworkingLobby::initChatScrollbar()
{
#ifndef SERVER_ONLY
    if (!m_text_bubble)
        return;
    
    if (m_chat_scrollbar)
    {
        m_chat_scrollbar->remove();
        m_chat_scrollbar->drop();
    }
    
    if (!m_text_bubble->getIrrlichtElement())
        return;
    
    irr::core::rect<s32> text_rect = m_text_bubble->getIrrlichtElement()->getAbsolutePosition();
    s32 scrollbar_width = getChatScrollbarWidth();
    
    // Position scrollbar on the right side of text bubble
    irr::core::rect<s32> scrollbar_rect(
        text_rect.LowerRightCorner.X - scrollbar_width,
        text_rect.UpperLeftCorner.Y,
        text_rect.LowerRightCorner.X,
        text_rect.LowerRightCorner.Y
    );
    
    // Use a unique ID for the chat scrollbar so we can identify it in events
    const s32 CHAT_SCROLLBAR_ID = -1000;
    irr::gui::IGUIElement* parent = m_text_bubble->getIrrlichtElement()->getParent();
    if (!parent)
        return;
    
    m_chat_scrollbar = GUIEngine::getGUIEnv()->addScrollBar(false, scrollbar_rect,
        parent, CHAT_SCROLLBAR_ID);
    if (m_chat_scrollbar)
    {
        m_chat_scrollbar->grab();
        m_chat_scrollbar->setSubElement(true);
        m_chat_scrollbar->setTabStop(false);
        m_chat_scrollbar->setVisible(false);
        // Restore scroll position if we have content, otherwise start at top
        // After updateChatScrollbar() is called, the position will be properly set
        m_chat_scrollbar->setPos(0);
    }
#endif
}   // initChatScrollbar

// ----------------------------------------------------------------------------
void NetworkingLobby::updateChatScrollbar()
{
#ifndef SERVER_ONLY
    if (!m_chat_scrollbar || !m_text_bubble)
        return;
    
    gui::IGUIFont* font = GUIEngine::getFont();
    if (!font)
        return;
    
    // Reserve space for scrollbar when breaking layouts
    s32 scrollbar_width = getChatScrollbarWidth();
    const unsigned box_width = core::max_(1u, 
        (unsigned)core::max_(1, (s32)m_text_bubble->getDimension().Width - scrollbar_width));
    const float box_height = m_text_bubble->getDimension().Height;
    s32 line_height = font->getHeightPerLine();
    
    if (box_height <= 0.0f || line_height <= 0)
        return;
    
    // Re-break layouts with current width to get accurate dimensions
    std::vector<gui::GlyphLayout> temp_layouts = m_server_info_full;
    gui::breakGlyphLayouts(temp_layouts, box_width,
        font->getInverseShaping(), font->getScale());
    
    // Calculate total height of all chat messages with correct line breaks
    core::dimension2du total_dim = gui::getGlyphLayoutsDimension(
        temp_layouts, line_height,
        font->getInverseShaping(), font->getScale());
    
    s32 total_height = total_dim.Height;
    s32 visible_height = (s32)box_height;
    
    // Update scrollbar max and visibility
    s32 max_scroll = core::max_(0, total_height - visible_height);
    m_chat_scrollbar->setMax(max_scroll);
    
    if (max_scroll > 0)
    {
        m_chat_scrollbar->setVisible(true);
        m_chat_scrollbar->setSmallStep(line_height * 3);
        m_chat_scrollbar->setLargeStep((s32)(visible_height * 0.1f));
    }
    else
    {
        m_chat_scrollbar->setVisible(false);
    }
#endif
}   // updateChatScrollbar

// ----------------------------------------------------------------------------
void NetworkingLobby::updateChatScrollPosition()
{
#ifndef SERVER_ONLY
    if (!m_chat_scrollbar || !m_text_bubble)
        return;
    
    m_chat_scroll_pos = m_chat_scrollbar->getPos();
    
    // If user manually scrolled to bottom, re-enable auto-scroll
    if (isChatAtBottom())
        m_chat_auto_scroll = true;
    
    filterGlyphLayoutsForScroll();
    updateServerInfos();
#endif
}   // updateChatScrollPosition

// ----------------------------------------------------------------------------
bool NetworkingLobby::isChatAtBottom() const
{
    if (!m_chat_scrollbar)
        return true;
    
    s32 max_pos = m_chat_scrollbar->getMax();
    if (max_pos <= 0)
        return true;
    
    s32 current_pos = m_chat_scrollbar->getPos();
    // Consider at bottom if within 5 pixels of bottom
    return (max_pos - current_pos) <= 5;
}   // isChatAtBottom

// ----------------------------------------------------------------------------
void NetworkingLobby::scrollChatToBottom()
{
    if (m_chat_scrollbar)
    {
        m_chat_scrollbar->setPos(m_chat_scrollbar->getMax());
        m_chat_scroll_pos = m_chat_scrollbar->getPos();
        m_chat_auto_scroll = true;
        // Update the display to show the bottom content
        filterGlyphLayoutsForScroll();
        updateServerInfos();
    }
}   // scrollChatToBottom

// ----------------------------------------------------------------------------
bool NetworkingLobby::handleChatScrollEvent(const irr::SEvent& event)
{
#ifndef SERVER_ONLY
    if (!m_chat_scrollbar || !m_text_bubble)
        return false;
    
    irr::core::rect<s32> text_rect = m_text_bubble->getIrrlichtElement()->getAbsolutePosition();
    
    // Check if mouse is over chat area
    if (event.EventType == irr::EET_MOUSE_INPUT_EVENT)
    {
        irr::core::position2di mouse_pos(event.MouseInput.X, event.MouseInput.Y);
        
        if (text_rect.isPointInside(mouse_pos))
        {
            if (event.MouseInput.Event == irr::EMIE_MOUSE_WHEEL)
            {
                // Scroll with mouse wheel
                s32 line_height = GUIEngine::getFont()->getHeightPerLine();
                s32 scroll_delta = (s32)(event.MouseInput.Wheel * -line_height * 3);
                s32 new_pos = core::clamp(m_chat_scrollbar->getPos() + scroll_delta,
                    m_chat_scrollbar->getMin(), m_chat_scrollbar->getMax());
                m_chat_scrollbar->setPos(new_pos);
                m_chat_scroll_pos = m_chat_scrollbar->getPos();
                // Disable auto-scroll, but updateChatScrollPosition will re-enable if at bottom
                m_chat_auto_scroll = false;
                updateChatScrollPosition();
                return true;
            }
        }
    }
    else if (event.EventType == irr::EET_KEY_INPUT_EVENT && event.KeyInput.PressedDown)
    {
        // Handle Page Up/Down when chat box has focus
        if (m_chat_box && (GUIEngine::getGUIEnv()->getFocus() == m_chat_box->getIrrlichtElement() ||
            m_chat_box->isFocusedForPlayer(PLAYER_ID_GAME_MASTER)))
        {
            if (event.KeyInput.Key == irr::IRR_KEY_PRIOR) // Page Up
            {
                s32 large_step = m_chat_scrollbar->getLargeStep();
                s32 new_pos = core::clamp(m_chat_scrollbar->getPos() - large_step,
                    m_chat_scrollbar->getMin(), m_chat_scrollbar->getMax());
                m_chat_scrollbar->setPos(new_pos);
                m_chat_scroll_pos = m_chat_scrollbar->getPos();
                // Disable auto-scroll, but updateChatScrollPosition will re-enable if at bottom
                m_chat_auto_scroll = false;
                updateChatScrollPosition();
                return true;
            }
            else if (event.KeyInput.Key == irr::IRR_KEY_NEXT) // Page Down
            {
                s32 large_step = m_chat_scrollbar->getLargeStep();
                s32 new_pos = core::clamp(m_chat_scrollbar->getPos() + large_step,
                    m_chat_scrollbar->getMin(), m_chat_scrollbar->getMax());
                m_chat_scrollbar->setPos(new_pos);
                m_chat_scroll_pos = m_chat_scrollbar->getPos();
                // Disable auto-scroll, but updateChatScrollPosition will re-enable if at bottom
                m_chat_auto_scroll = false;
                updateChatScrollPosition();
                return true;
            }
        }
    }
    else if (event.EventType == irr::EET_GUI_EVENT)
    {
        if (event.GUIEvent.EventType == irr::gui::EGET_SCROLL_BAR_CHANGED)
        {
            if (event.GUIEvent.Caller == m_chat_scrollbar)
            {
                m_chat_scroll_pos = m_chat_scrollbar->getPos();
                // Disable auto-scroll, but updateChatScrollPosition will re-enable if at bottom
                m_chat_auto_scroll = false;
                updateChatScrollPosition();
                return true;
            }
        }
    }
#endif
    return false;
}   // handleChatScrollEvent

// ----------------------------------------------------------------------------
void NetworkingLobby::filterInput(Input::InputType type,
                                  int deviceID,
                                  int btnID,
                                  int axisDir,
                                  int value)
{
#ifndef SERVER_ONLY
    // Handle Page Up/Down keys for chat scrolling
    if (type == Input::IT_KEYBOARD && m_chat_box && 
        (GUIEngine::getGUIEnv()->getFocus() == m_chat_box->getIrrlichtElement() ||
         m_chat_box->isFocusedForPlayer(PLAYER_ID_GAME_MASTER)))
    {
        irr::SEvent event;
        event.EventType = irr::EET_KEY_INPUT_EVENT;
        event.KeyInput.PressedDown = (value != 0);
        event.KeyInput.Char = 0;
        event.KeyInput.Control = false;
        event.KeyInput.Shift = false;
        
        if (btnID == irr::IRR_KEY_PRIOR || btnID == irr::IRR_KEY_NEXT)
        {
            event.KeyInput.Key = (irr::EKEY_CODE)btnID;
            if (handleChatScrollEvent(event))
                return; // Event handled
        }
    }
#endif
    Screen::filterInput(type, deviceID, btnID, axisDir, value);
}   // filterInput
