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

#ifndef SERVER_ONLY // No GUI files in server builds

// Manages includes common to all options screens
#include "states_screens/options/options_common.hpp"

#include "addons/news_manager.hpp"
#include "config/player_manager.hpp"
#include "online/request_manager.hpp"
#include "states_screens/dialogs/download_assets.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "utils/extract_mobile_assets.hpp"

#include <IrrlichtDevice.h>

using namespace GUIEngine;
using namespace Online;

// -----------------------------------------------------------------------------

OptionsScreenGeneral::OptionsScreenGeneral() : Screen("options/options_general.stkgui")
{
    m_inited = false;
}   // OptionsScreenVideo

// -----------------------------------------------------------------------------

void OptionsScreenGeneral::loadedFromFile()
{
    m_inited = false;
}   // loadedFromFile

// -----------------------------------------------------------------------------

void OptionsScreenGeneral::init()
{
    Screen::init();

    // Bind typed widget pointers (one-time lookup)
    m_widgets.bind(this);

    m_widgets.options_choice->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    m_widgets.options_choice->select( "tab_general", PLAYER_ID_GAME_MASTER );

    m_widgets.enable_internet->setState( UserConfigParams::m_internet_status
                                     ==RequestManager::IPERM_ALLOWED );

    setInternetCheckboxes(m_widgets.enable_internet->getState());

    m_widgets.enable_handicap->setState( UserConfigParams::m_per_player_difficulty );
    // I18N: Tooltip in the UI menu. Use enough linebreaks to make sure the text fits the screen in low resolutions.
    m_widgets.enable_handicap->setTooltip(_("In multiplayer mode, players can select handicapped\n(more difficult) profiles on the kart selection screen"));

    m_widgets.show_login->setState( UserConfigParams::m_always_show_login_screen);

    OptionsCommon::setTabStatus();

#ifdef MOBILE_STK
    if (ExtractMobileAssets::hasFullAssets())
    {
        // I18N: For mobile version for STK, uninstall the downloaded assets
        m_widgets.assets_settings->setText(_("Uninstall full game assets"));
    }
    else
    {
        // I18N: For mobile version for STK, install the full game assets which
        // will download from stk server
        m_widgets.assets_settings->setText(_("Install full game assets"));
    }
    if (UserConfigParams::m_internet_status != RequestManager::IPERM_ALLOWED ||
        StateManager::get()->getGameState() == GUIEngine::INGAME_MENU)
        m_widgets.assets_settings->setActive(false);
    else
        m_widgets.assets_settings->setActive(true);
#else
    m_widgets.assets_settings->setVisible(false);
#endif
}   // init

// -----------------------------------------------------------------------------

void OptionsScreenGeneral::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (widget == m_widgets.options_choice)
    {
        std::string selection = m_widgets.options_choice->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection != "tab_general")
            OptionsCommon::switchTab(selection);
    }
    else if (widget == m_widgets.back)
    {
        StateManager::get()->escapePressed();
    }
    else if (widget == m_widgets.enable_internet)
    {
        // If internet is being activated, enable immediately. If it's being disabled,
        // we'll disable later after logout.
        if (m_widgets.enable_internet->getState())
        {
            UserConfigParams::m_internet_status = RequestManager::IPERM_ALLOWED;

            if (!RequestManager::isRunning())
                RequestManager::get()->startNetworkThread();
        }

        // If internet gets enabled, re-initialise the addon manager (which
        // happens in a separate thread) so that news.xml etc can be
        // downloaded if necessary.
        setInternetCheckboxes(m_widgets.enable_internet->getState());
        PlayerProfile* profile = PlayerManager::getCurrentPlayer();
        if(m_widgets.enable_internet->getState())
            NewsManager::get()->init(false);
        else if (profile != NULL && profile->isLoggedIn())
            profile->requestSignOut();

        // Deactivate internet after 'requestSignOut' so that the sign out request is allowed
        if (!m_widgets.enable_internet->getState())
            UserConfigParams::m_internet_status = RequestManager::IPERM_NOT_ALLOWED;
    }
    /*else if (widget == m_widgets.enable_hw_report)
    {
        UserConfigParams::m_hw_report_enable = m_widgets.enable_hw_report->getState();
        if(m_widgets.enable_hw_report->getState())
            HardwareStats::reportHardwareStats();
    }
    */
    else if (widget == m_widgets.enable_lobby_chat)
    {
        UserConfigParams::m_lobby_chat = m_widgets.enable_lobby_chat->getState();
        m_widgets.enable_race_chat->setActive(UserConfigParams::m_lobby_chat);
    }
    else if (widget == m_widgets.enable_race_chat)
    {
        UserConfigParams::m_race_chat = m_widgets.enable_race_chat->getState();
    }
    else if (widget == m_widgets.show_login)
    {
        UserConfigParams::m_always_show_login_screen = m_widgets.show_login->getState();
    }
    else if (widget == m_widgets.enable_handicap)
    {
        UserConfigParams::m_per_player_difficulty = m_widgets.enable_handicap->getState();
    }
#ifdef MOBILE_STK
    else if (widget == m_widgets.assets_settings)
    {
        if (ExtractMobileAssets::hasFullAssets())
        {
            class AssetsDialogListener : public MessageDialog::IConfirmDialogListener
            {
            public:
                virtual void onConfirm() OVERRIDE
                {
                    ModalDialog::dismiss();
                    ExtractMobileAssets::uninstall();
                }
            };   // class AssetsDialogListener
            new MessageDialog(
                _("Are you sure to uninstall full game assets?"),
                MessageDialog::MESSAGE_DIALOG_OK_CANCEL,
                new AssetsDialogListener(), true);
        }
        else
            new DownloadAssets();
    }
#endif
}   // eventCallback

void OptionsScreenGeneral::setInternetCheckboxes(bool activate)
{
    //CheckBoxWidget* stats = getWidget<CheckBoxWidget>("enable-hw-report");

    if (activate)
    {
        //stats->setActive(true);
        //stats->setState(UserConfigParams::m_hw_report_enable);
        m_widgets.enable_lobby_chat->setActive(true);
        m_widgets.enable_lobby_chat->setState(UserConfigParams::m_lobby_chat);
        m_widgets.enable_race_chat->setActive(UserConfigParams::m_lobby_chat);
        m_widgets.enable_race_chat->setState(UserConfigParams::m_race_chat);
#ifdef MOBILE_STK
        m_widgets.assets_settings->setActive(true);
#endif
        }
    else
    {
        m_widgets.enable_lobby_chat->setActive(false);
        //stats->setActive(false);
        m_widgets.enable_race_chat->setActive(false);
#ifdef MOBILE_STK
        m_widgets.assets_settings->setActive(false);
#endif
        // Disable this, so that the user has to re-check this if
        // enabled later (for GDPR compliance).
        //UserConfigParams::m_hw_report_enable = false;
        //stats->setState(false);
    }
} // setInternetCheckboxes

// -----------------------------------------------------------------------------

void OptionsScreenGeneral::tearDown()
{
    Screen::tearDown();
    // save changes when leaving screen
    user_config->saveConfig();
}   // tearDown

// -----------------------------------------------------------------------------

void OptionsScreenGeneral::unloaded()
{
    m_inited = false;
}   // unloaded

#endif // ifndef SERVER_ONLY