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

#include "states_screens/options/options_screen_general.hpp"

#include "addons/news_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "config/hardware_stats.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "font/bold_face.hpp"
#include "font/font_manager.hpp"
#include "font/regular_face.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widget.hpp"
#include "io/file_manager.hpp"
#include "online/request_manager.hpp"
#include "states_screens/dialogs/download_assets.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/options/options_screen_audio.hpp"
#include "states_screens/options/options_screen_input.hpp"
#include "states_screens/options/options_screen_language.hpp"
#include "states_screens/options/options_screen_ui.hpp"
#include "states_screens/options/options_screen_video.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/options/user_screen.hpp"
#include "utils/extract_mobile_assets.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

#include <IrrlichtDevice.h>

using namespace GUIEngine;
using namespace Online;

// -----------------------------------------------------------------------------

OptionsScreenGeneral::OptionsScreenGeneral() : Screen("options_general.stkgui")
{
    m_resizable = true;
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
    GUIEngine::getDevice()->setResizable(
        StateManager::get()->getGameState() == GUIEngine::MENU);
    Screen::init();
    RibbonWidget* ribbon = getWidget<RibbonWidget>("options_choice");
    assert(ribbon != NULL);
    ribbon->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    ribbon->select( "tab_general", PLAYER_ID_GAME_MASTER );

    CheckBoxWidget* internet_enabled = getWidget<CheckBoxWidget>("enable-internet");
    assert( internet_enabled != NULL );
    internet_enabled->setState( UserConfigParams::m_internet_status
                                     ==RequestManager::IPERM_ALLOWED );

    setInternetCheckboxes(internet_enabled->getState());

    CheckBoxWidget* handicap = getWidget<CheckBoxWidget>("enable-handicap");
    assert( handicap != NULL );
    handicap->setState( UserConfigParams::m_per_player_difficulty );
    // I18N: Tooltip in the UI menu. Use enough linebreaks to make sure the text fits the screen in low resolutions.
    handicap->setTooltip(_("In multiplayer mode, players can select handicapped\n(more difficult) profiles on the kart selection screen"));

    CheckBoxWidget* show_login = getWidget<CheckBoxWidget>("show-login");
    assert( show_login!= NULL );
    show_login->setState( UserConfigParams::m_always_show_login_screen);

#ifdef MOBILE_STK
    if (ExtractMobileAssets::hasFullAssets())
    {
        // I18N: For mobile version for STK, uninstall the downloaded assets
        getWidget("assets_settings")->setText(_("Uninstall full game assets"));
    }
    else
    {
        // I18N: For mobile version for STK, install the full game assets which
        // will download from stk server
        getWidget("assets_settings")->setText(_("Install full game assets"));
    }
    if (UserConfigParams::m_internet_status != RequestManager::IPERM_ALLOWED ||
        StateManager::get()->getGameState() == GUIEngine::INGAME_MENU)
        getWidget("assets_settings")->setActive(false);
    else
        getWidget("assets_settings")->setActive(true);
#else
    getWidget("assets_settings")->setVisible(false);
#endif
}   // init

// -----------------------------------------------------------------------------

void OptionsScreenGeneral::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
#ifndef SERVER_ONLY
    if (name == "options_choice")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        Screen *screen = NULL;
        if (selection == "tab_audio")
            screen = OptionsScreenAudio::getInstance();
        else if (selection == "tab_video")
            screen = OptionsScreenVideo::getInstance();
        else if (selection == "tab_players")
            screen = TabbedUserScreen::getInstance();
        else if (selection == "tab_controls")
            screen = OptionsScreenInput::getInstance();
        else if (selection == "tab_ui")
            screen = OptionsScreenUI::getInstance();
        //else if (selection == "tab_general")
        //    screen = OptionsScreenGeneral::getInstance();
        else if (selection == "tab_language")
            screen = OptionsScreenLanguage::getInstance();
        if(screen)
            StateManager::get()->replaceTopMostScreen(screen);
    }
    else if(name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if (name=="enable-internet")
    {
        CheckBoxWidget* internet = getWidget<CheckBoxWidget>("enable-internet");
        assert( internet != NULL );

        // If internet is being activated, enable immediately. If it's being disabled,
        // we'll disable later after logout.
        if (internet->getState())
        {
            UserConfigParams::m_internet_status = RequestManager::IPERM_ALLOWED;

            if (!RequestManager::isRunning())
                RequestManager::get()->startNetworkThread();
        }

        // If internet gets enabled, re-initialise the addon manager (which
        // happens in a separate thread) so that news.xml etc can be
        // downloaded if necessary.
        setInternetCheckboxes(internet->getState());
        PlayerProfile* profile = PlayerManager::getCurrentPlayer();
        if(internet->getState())
            NewsManager::get()->init(false);
        else if (profile != NULL && profile->isLoggedIn())
            profile->requestSignOut();

        // Deactivate internet after 'requestSignOut' so that the sign out request is allowed
        if (!internet->getState())
            UserConfigParams::m_internet_status = RequestManager::IPERM_NOT_ALLOWED;
    }
    /*else if (name=="enable-hw-report")
    {
        CheckBoxWidget* stats = getWidget<CheckBoxWidget>("enable-hw-report");
        UserConfigParams::m_hw_report_enable = stats->getState();
        if(stats->getState())
            HardwareStats::reportHardwareStats();
    }
    */
    else if (name=="enable-lobby-chat")
    {
        CheckBoxWidget* chat = getWidget<CheckBoxWidget>("enable-lobby-chat");
        UserConfigParams::m_lobby_chat = chat->getState();
        CheckBoxWidget* race_chat = getWidget<CheckBoxWidget>("enable-race-chat");
        race_chat->setActive(UserConfigParams::m_lobby_chat);
    }
    else if (name=="enable-race-chat")
    {
        CheckBoxWidget* chat = getWidget<CheckBoxWidget>("enable-race-chat");
        UserConfigParams::m_race_chat = chat->getState();
    }
    else if (name=="show-login")
    {
        CheckBoxWidget* show_login = getWidget<CheckBoxWidget>("show-login");
        assert( show_login != NULL );
        UserConfigParams::m_always_show_login_screen = show_login->getState();
    }
    else if (name=="enable-handicap")
    {
        CheckBoxWidget* handicap = getWidget<CheckBoxWidget>("enable-handicap");
        assert( handicap != NULL );
        UserConfigParams::m_per_player_difficulty = handicap->getState();
    }
#ifdef MOBILE_STK
    else if (name=="assets_settings")
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
#endif
}   // eventCallback

void OptionsScreenGeneral::setInternetCheckboxes(bool activate)
{
    //CheckBoxWidget* stats = getWidget<CheckBoxWidget>("enable-hw-report");
    CheckBoxWidget* chat = getWidget<CheckBoxWidget>("enable-lobby-chat");
    CheckBoxWidget* race_chat = getWidget<CheckBoxWidget>("enable-race-chat");

    if (activate)
    {
        //stats->setActive(true);
        //stats->setState(UserConfigParams::m_hw_report_enable);
        chat->setActive(true);
        chat->setState(UserConfigParams::m_lobby_chat);
        race_chat->setActive(UserConfigParams::m_lobby_chat);
        race_chat->setState(UserConfigParams::m_race_chat);
#ifdef MOBILE_STK
        getWidget("assets_settings")->setActive(true);
#endif
        }
    else
    {
        chat->setActive(false);
        //stats->setActive(false);
        race_chat->setActive(false);
#ifdef MOBILE_STK
        getWidget("assets_settings")->setActive(false);
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
    GUIEngine::getDevice()->setResizable(false);
    Screen::tearDown();
    // save changes when leaving screen
    user_config->saveConfig();
}   // tearDown

// -----------------------------------------------------------------------------

void OptionsScreenGeneral::unloaded()
{
    m_inited = false;
}   // unloaded

// -----------------------------------------------------------------------------
