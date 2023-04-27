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

#include "states_screens/options/user_screen.hpp"

#include "addons/news_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/central_settings.hpp"
#include "guiengine/screen_keyboard.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "online/link_helper.hpp"
#include "online/request_manager.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/dialogs/kart_color_slider_dialog.hpp"
#include "states_screens/dialogs/recovery_dialog.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/options/options_screen_audio.hpp"
#include "states_screens/options/options_screen_general.hpp"
#include "states_screens/options/options_screen_input.hpp"
#include "states_screens/options/options_screen_language.hpp"
#include "states_screens/options/options_screen_ui.hpp"
#include "states_screens/options/options_screen_video.hpp"
#include "states_screens/online/register_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;

// ----------------------------------------------------------------------------

BaseUserScreen::BaseUserScreen(const std::string &name) : Screen(name.c_str())
{
    m_online_cb           = NULL;
    m_new_registered_data = false;
    m_auto_login          = false;
}   // BaseUserScreen

// ----------------------------------------------------------------------------

void BaseUserScreen::loadedFromFile()
{
    m_online_cb = getWidget<CheckBoxWidget>("online");
    assert(m_online_cb);
    m_username_tb = getWidget<TextBoxWidget >("username");
    assert(m_username_tb);
    m_password_tb = getWidget<TextBoxWidget >("password");
    assert(m_password_tb);
    m_players = getWidget<DynamicRibbonWidget>("players");
    assert(m_players);
    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget);
    m_info_widget = getWidget<LabelWidget>("message");
    assert(m_info_widget);

}   // loadedFromFile

// ----------------------------------------------------------------------------
/** Stores information from the register screen. It allows this screen to 
 *  use the entered user name and password to prefill fields so that the user
 *  does not have to enter them again.
 *  \param online If the user created an online account.
 *  \param auto-login If the user should be automatically logged in online.
 *         This can not be done for newly created online accounts, since they
 *         need to be confirmed first.
 * \param online_name The online account name.
 *  \param password The password for the online account.
 */
void BaseUserScreen::setNewAccountData(bool online, bool auto_login,
                                       const core::stringw &online_name,
                                       const core::stringw &password)
{
    // Indicate for init that new user data is available.
    m_new_registered_data = true;
    m_auto_login          = auto_login;
    m_online_cb->setState(online);
    m_username_tb->setText(online_name);
    m_password_tb->setText(password);
}   // setOnline

// ----------------------------------------------------------------------------
void BaseUserScreen::beforeAddingWidget()
{
}   // beforeAddingWidget

// ----------------------------------------------------------------------------
/** Initialises the user screen. Searches for all players to fill the 
 *  list of users with their icons, and initialises all widgets for the
 *  current user (e.g. the online flag etc).
 */
void BaseUserScreen::init()
{
#ifndef SERVER_ONLY
    getWidget<IconButtonWidget>("default_kart_color")
        ->setVisible(CVS->supportsColorization());
#endif

    m_password_tb->setPasswordBox(true, L'*');

    // The behaviour of the screen is slightly different at startup, i.e.
    // when it is the first screen: cancel will exit the game, and in
    // this case no 'back' error should be shown.
    bool is_first_screen = StateManager::get()->getMenuStackSize()==1;
    getWidget<IconButtonWidget>("back")->setVisible(!is_first_screen);
    getWidget<IconButtonWidget>("cancel")->setLabel(is_first_screen 
                                                    ? _("Exit game") 
                                                    : _("Cancel")      );

    m_sign_out_name = "";
    m_sign_in_name  = "";

    // It should always be activated ... but just in case
    m_options_widget->setActive(true);
    // Clean any error message still shown
    m_info_widget->setText("", true);
    m_info_widget->setErrorColor();

    Screen::init();

    m_players->clearItems();
    int current_player_index = -1;

    for (unsigned int n=0; n<PlayerManager::get()->getNumPlayers(); n++)
    {
        const PlayerProfile *player = PlayerManager::get()->getPlayer(n);
        if (player->isGuestAccount()) continue;
        std::string s = StringUtils::toString(n);
        m_players->addItem(player->getName(), s, player->getIconFilename(), 0,
                           IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
        if(player == PlayerManager::getCurrentPlayer())
            current_player_index = n;
    }

    m_players->updateItemDisplay();

    // Select the current player. That can only be done after
    // updateItemDisplay is called.
    if (current_player_index != -1)
        selectUser(current_player_index);
    // no current player found
    // The first player is the most frequently used, so select it
    else if (PlayerManager::get()->getNumPlayers() > 0)
        selectUser(0);

    // Disable changing the user while in game
    bool in_game = StateManager::get()->getGameState() == GUIEngine::INGAME_MENU;
    getWidget<IconButtonWidget>("ok")->setActive(!in_game);
    getWidget<IconButtonWidget>("new_user")->setActive(!in_game);
    getWidget<IconButtonWidget>("rename")->setActive(!in_game);
    getWidget<IconButtonWidget>("delete")->setActive(!in_game);
    if (getWidget<IconButtonWidget>("default_kart_color")->isVisible())
        getWidget<IconButtonWidget>("default_kart_color")->setActive(!in_game);

    m_new_registered_data = false;
    if (m_auto_login)
    {
        m_auto_login = false;
        login();
        return;
    }
    m_auto_login = false;
}   // init

// ----------------------------------------------------------------------------
PlayerProfile* BaseUserScreen::getSelectedPlayer()
{
    const std::string &s_id = m_players
                            ->getSelectionIDString(PLAYER_ID_GAME_MASTER);
    unsigned int n_id;
    StringUtils::fromString(s_id, n_id);
    return PlayerManager::get()->getPlayer(n_id);
}   // getSelectedPlayer

// ----------------------------------------------------------------------------

void BaseUserScreen::tearDown()
{
    Screen::tearDown();
}   // tearDown

// ----------------------------------------------------------------------------

EventPropagation BaseUserScreen::filterActions(PlayerAction action,
    int deviceID,
    const unsigned int value,
    Input::InputType type,
    int playerId)
{
    if (action == PA_MENU_SELECT &&
        (!ScreenKeyboard::shouldUseScreenKeyboard() ||
        GUIEngine::ScreenKeyboard::hasSystemScreenKeyboard()))
    {
        if ((m_username_tb != NULL && m_username_tb->isFocusedForPlayer(PLAYER_ID_GAME_MASTER))
            || (m_password_tb != NULL && m_password_tb->isFocusedForPlayer(PLAYER_ID_GAME_MASTER)))
        {
            login();
            return EVENT_BLOCK;
        }
    }

    return EVENT_LET;
}

// ----------------------------------------------------------------------------
/** Called when a user is selected. It updates the online checkbox and
 *  entry fields.
 */
void BaseUserScreen::selectUser(int index)
{
    PlayerProfile *profile = PlayerManager::get()->getPlayer(index);
    assert(profile);

    // Only set focus in case of non-tabbed version (so that keyboard
    // or gamepad navigation with tabs works as expected, i.e. you can
    // select the next tab without having to go up to the tab list first.
    bool focus_it = !getWidget<RibbonWidget>("options_choice");
    m_players->setSelection(StringUtils::toString(index), PLAYER_ID_GAME_MASTER,
                            focus_it);
    
    if (!m_new_registered_data)
        m_username_tb->setText(profile->getLastOnlineName());

    if (!m_new_registered_data)
    {
        // Delete a password that might have been typed for another user
        m_password_tb->setText("");
    }
    
    getWidget<CheckBoxWidget>("remember-user")->setState(
        profile->rememberPassword());

    // Last game was not online, so make the offline settings the default
    // (i.e. unckeck online checkbox, and make entry fields invisible).
    if (!profile->wasOnlineLastTime() || profile->getLastOnlineName() == "")
    {
        if (!m_new_registered_data)
            m_online_cb->setState(false);
        makeEntryFieldsVisible();
        return;
    }

    // Now last use was with online --> Display the saved data
    if (UserConfigParams::m_internet_status == Online::RequestManager::IPERM_NOT_ALLOWED)
        m_online_cb->setState(false);
    else
        m_online_cb->setState(true);

    makeEntryFieldsVisible();
    m_username_tb->setActive(profile->getLastOnlineName().size() == 0);

    // And make the password invisible if the session is saved (i.e
    // the user does not need to enter a password).
    if (profile->hasSavedSession())
    {
        m_password_tb->setVisible(false);
        getWidget<LabelWidget>("label_password")->setVisible(false);
        getWidget<ButtonWidget>("password_reset")->setVisible(false);
    }

}   // selectUser

// ----------------------------------------------------------------------------
/** Make the entry fields either visible or invisible.
 *  \param online Online state, which dicates if the entry fields are
 *         visible (true) or not.
 */
void BaseUserScreen::makeEntryFieldsVisible()
{
#ifdef GUEST_ACCOUNTS_ENABLED
    getWidget<LabelWidget>("label_guest")->setVisible(online);
    getWidget<CheckBoxWidget>("guest")->setVisible(online);
#endif
    bool online = m_online_cb->getState();
    getWidget<LabelWidget>("label_username")->setVisible(online);
    m_username_tb->setVisible(online);
    getWidget<LabelWidget>("label_remember")->setVisible(online);
    getWidget<CheckBoxWidget>("remember-user")->setVisible(online);
    PlayerProfile *player = getSelectedPlayer();

    // Don't show the password fields if the player wants to be online
    // and either is the current player and logged in (no need to enter a
    // password then) or has a saved session.
    if(player && online  &&
        (player->hasSavedSession() || 
          (player==PlayerManager::getCurrentPlayer() && player->isLoggedIn() ) 
        ) 
      )
    {
        // If we show the online login fields, but the player has a
        // saved session, don't show the password field.
        getWidget<LabelWidget>("label_password")->setVisible(false);
        m_password_tb->setVisible(false);
        getWidget<ButtonWidget>("password_reset")->setVisible(false);
    }
    else
    {
        getWidget<LabelWidget>("label_password")->setVisible(online);
        m_password_tb->setVisible(online);
        getWidget<ButtonWidget>("password_reset")->setVisible(Online::LinkHelper::isSupported() && online);
        // Is user has no online name, make sure the user can enter one
        if (player->getLastOnlineName().empty())
            m_username_tb->setActive(true);

    }
}   // makeEntryFieldsVisible

// ----------------------------------------------------------------------------
/** Called when the user selects anything on the screen.
 */
void BaseUserScreen::eventCallback(Widget* widget,
                               const std::string& name,
                               const int player_id)
{
    // Clean any error message still shown
    m_info_widget->setText("", true);
    m_info_widget->setErrorColor();

    if (name == "players")
    {
        // Clicked on a name --> Find the corresponding online data
        // and display them
        const std::string &s_index = getWidget<DynamicRibbonWidget>("players")
                                   ->getSelectionIDString(player_id);
        if (s_index == "") return;  // can happen if the list is empty

        unsigned int id;
        if (StringUtils::fromString(s_index, id))
            selectUser(id);
    }
    else if (name == "remember-user")
    {
        getSelectedPlayer()->setRememberPassword(
            getWidget<CheckBoxWidget>("remember-user")->getState());
    }
    else if (name == "online")
    {
        // If online access is not allowed,
        // give the player the choice to enable this option.
        if (m_online_cb->getState())
        {
            if (UserConfigParams::m_internet_status ==
                                       Online::RequestManager::IPERM_NOT_ALLOWED)
            {
                irr::core::stringw message =
                    _("Internet access is disabled. Do you want to enable it?");

                class ConfirmInternet : public MessageDialog::IConfirmDialogListener
                {
                    BaseUserScreen *m_parent_screen;
                private:
                    GUIEngine::CheckBoxWidget *m_cb;
                public:
                    virtual void onConfirm()
                    {
                        UserConfigParams::m_internet_status =
                            Online::RequestManager::IPERM_ALLOWED;
#ifndef SERVER_ONLY
                        NewsManager::get()->init(false);
#endif
                        m_parent_screen->makeEntryFieldsVisible();
                        ModalDialog::dismiss();
                    }   // onConfirm
                    virtual void onCancel()
                    {
                        m_cb->setState(false);
                        m_parent_screen->makeEntryFieldsVisible();
                        ModalDialog::dismiss();
                    }   // onCancel
                    // ------------------------------------------------------------
                    ConfirmInternet(BaseUserScreen *parent, GUIEngine::CheckBoxWidget *online_cb)
                    {
                        m_parent_screen = parent;
                        m_cb = online_cb;
                    }
                };   // ConfirmInternet

                SFXManager::get()->quickSound( "anvil" );
                new MessageDialog(message, MessageDialog::MESSAGE_DIALOG_CONFIRM,
                      new ConfirmInternet(this, m_online_cb), true);
            }
        }
        makeEntryFieldsVisible();
    }
    else if (name == "password_reset")
    {
        new RecoveryDialog();
    }
    else if (name == "options")
    {
        const std::string &button =
                             m_options_widget->getSelectionIDString(player_id);
        if (button == "ok")
        {
            login();
        }   // button==ok
        else if (button == "new_user")
        {
            RegisterScreen::getInstance()->push();
            RegisterScreen::getInstance()->setParent(this);
            // Make sure the new user will have an empty online name field
            // that can also be edited.
            m_username_tb->setText("");
            m_username_tb->setActive(true);
        }
        else if (button == "cancel")
        {
            // EscapePressed will pop this screen.
            StateManager::get()->escapePressed();
        }
        else if (button == "rename")
        {
            PlayerProfile *cp = getSelectedPlayer();
            RegisterScreen::getInstance()->setRename(cp);
            RegisterScreen::getInstance()->push();
            RegisterScreen::getInstance()->setParent(this);
            m_new_registered_data = false;
            m_auto_login          = false;
            // Init will automatically be called, which
            // refreshes the player list
        }
        else if (button == "default_kart_color")
        {
            new KartColorSliderDialog(getSelectedPlayer());
        }
        else if (button == "delete")
        {
            deletePlayer();
        }
    }   // options
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }

    return;

}   // eventCallback

// ----------------------------------------------------------------------------
/** Closes the BaseUserScreen, and makes sure that the right screen is displayed
 *  next.
 */
void BaseUserScreen::closeScreen()
{
    if (StateManager::get()->getMenuStackSize() > 1)
    {
        StateManager::get()->popMenu();
    }
    else
    {
        StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
    }
}   // closeScreen

// ----------------------------------------------------------------------------
/** Called when OK or OK-and-save is clicked.
 *  This will trigger the actual login (if requested) etc.
 *  \param remember_me True if the login details should be remembered,
 *         so that next time this menu can be skipped.
 */
void BaseUserScreen::login()
{
    // If an error occurs, the callback informing this screen about the
    // problem will activate the widget again.
    m_options_widget->setActive(false);
    m_state = STATE_NONE;

    PlayerProfile *player = getSelectedPlayer();
    PlayerProfile *current = PlayerManager::getCurrentPlayer();
    core::stringw  new_username = m_username_tb->getText();
    // If a different player is connecting, or the same local player with
    // a different online account, log out the current player.
    if(current && current->isLoggedIn() &&
        (player!=current ||
        current->getLastOnlineName()!=new_username) )
    {
        m_sign_out_name = current->getLastOnlineName();
        current->requestSignOut();
        m_state = (UserScreenState)(m_state | STATE_LOGOUT);

        // If the online user name was changed, reset the save data
        // for this user (otherwise later the saved session will be
        // resumed, not logging the user with the new account).
        if(player==current &&
            current->getLastOnlineName()!=new_username)
            current->clearSession();
    }
    PlayerManager::get()->setCurrentPlayer(player);
    assert(player);

    // If no online login requested, log the player out (if necessary)
    // and go to the main menu screen (though logout needs to finish first)
    if(!m_online_cb->getState())
    {
        if(player->isLoggedIn())
        {
            m_sign_out_name =player->getLastOnlineName();
            player->requestSignOut();
            m_state =(UserScreenState)(m_state| STATE_LOGOUT);
        }

        player->setWasOnlineLastTime(false);
        if(m_state==STATE_NONE)
        {
            closeScreen();
        }
        return;
    }

    // Player wants to be online, and is already online - nothing to do
    if(player->isLoggedIn())
    {
        player->setWasOnlineLastTime(true);
        closeScreen();
        return;
    }
    m_state = (UserScreenState) (m_state | STATE_LOGIN);
    // Now we need to start a login request to the server
    // This implies that this screen will wait till the server responds, so
    // that error messages ('invalid password') can be shown, and the user
    // can decide what to do about them.
    if (player->hasSavedSession())
    {
        m_sign_in_name = player->getLastOnlineName();
        // Online login with saved token
        player->requestSavedSession();
    }
    else
    {
        // Online login with password --> we need a valid password
        if (m_password_tb->getText() == "")
        {
            m_info_widget->setText(_("You need to enter a password."), true);
            SFXManager::get()->quickSound("anvil");
            m_options_widget->setActive(true);
            return;
        }
        m_sign_in_name = m_username_tb->getText();
        player->requestSignIn(m_username_tb->getText(),
                               m_password_tb->getText());
    }   // !hasSavedSession

}   // login

// ----------------------------------------------------------------------------
/** Called once every frame. It will replace this screen with the main menu
 *  screen if a successful login happened.
 */
void BaseUserScreen::onUpdate(float dt)
{
    if (!m_options_widget->isActivated())
    {
        core::stringw message = (m_state & STATE_LOGOUT)
                              ? _(L"Logging out '%s'",m_sign_out_name.c_str())
                              : _(L"Logging in '%s'", m_sign_in_name.c_str());
        m_info_widget->setText(StringUtils::loadingDots(message.c_str()),
                               false                                      );
    }
}   // onUpdate

// ----------------------------------------------------------------------------
/** Callback from player profile if login was successful.
 */
void BaseUserScreen::loginSuccessful()
{
    PlayerProfile *player  = getSelectedPlayer();
    player->setWasOnlineLastTime(true);
    m_options_widget->setActive(true);
    // Clean any error message still shown
    m_info_widget->setText("", true);
    m_info_widget->setErrorColor();
    // The callback is done from the main thread, so no need to sync
    // access to m_success. OnUpdate will check this flag
    closeScreen();
}   // loginSuccessful

// ----------------------------------------------------------------------------
/** Callback from player profile if login was unsuccessful.
 *  \param error_message Contains the error message.
 */
void BaseUserScreen::loginError(const irr::core::stringw & error_message,
                                bool clear_password)
{
    m_state = (UserScreenState) (m_state & ~STATE_LOGIN);
    PlayerProfile *player = getSelectedPlayer();
    // Clear information about saved session in case of a problem,
    // which allows the player to enter a new password.
    // Only if not a download error
    if(clear_password && player && player->hasSavedSession())
        player->clearSession();
    player->setLastOnlineName("");
    makeEntryFieldsVisible();
    SFXManager::get()->quickSound("anvil");
    m_info_widget->setErrorColor();
    m_info_widget->setText(error_message, false);
    m_options_widget->setActive(true);
}   // loginError

// ----------------------------------------------------------------------------
/** Callback from player profile if logout was successful.
 */
void BaseUserScreen::logoutSuccessful()
{
    m_state = (UserScreenState) (m_state & ~STATE_LOGOUT);
    if(m_state==STATE_NONE)
        closeScreen();
    // Otherwise the screen still has to wait for a login request to finish.
}   // loginSuccessful

// ----------------------------------------------------------------------------
/** Callback from player profile if login was unsuccessful.
 *  \param error_message Contains the error message.
 */
void BaseUserScreen::logoutError(const irr::core::stringw & error_message)
{
    m_state = (UserScreenState) (m_state & ~STATE_LOGOUT);
    PlayerProfile *player = getSelectedPlayer();
    // Clear information about saved session in case of a problem,
    // which allows the player to enter a new password.
    if(player && player->hasSavedSession())
        player->clearSession();
    makeEntryFieldsVisible();
    SFXManager::get()->quickSound("anvil");
    m_info_widget->setErrorColor();
    m_info_widget->setText(error_message, false);
    m_options_widget->setActive(true);
}   // logoutError

// ----------------------------------------------------------------------------
/** Called when a player will be deleted.
 */
void BaseUserScreen::deletePlayer()
{
    // Check that there is at least one player left: we need to have a
    // valid current player, so the last player can not be deleted.
    if(PlayerManager::get()->getNumNonGuestPlayers()==1)
    {
        m_info_widget->setText(_("You can't delete the only player."), true);
        m_info_widget->setErrorColor();
        return;
    }

    PlayerProfile *player = getSelectedPlayer();
    irr::core::stringw message =
        //I18N: In the player info dialog (when deleting)
        _("Do you really want to delete player '%s'?", player->getName());

    class ConfirmServer : public MessageDialog::IConfirmDialogListener
    {
        BaseUserScreen *m_parent_screen;
    public:
        virtual void onConfirm()
        {
            m_parent_screen->doDeletePlayer();
        }   // onConfirm
        // ------------------------------------------------------------
        ConfirmServer(BaseUserScreen *parent)
        {
            m_parent_screen = parent;
        }
    };   // ConfirmServer

    new MessageDialog(message, MessageDialog::MESSAGE_DIALOG_CONFIRM,
                      new ConfirmServer(this), true);
}   // deletePlayer

// ----------------------------------------------------------------------------
/** Callback when the user confirms to delete a player. This function actually
 *  deletes the player, discards the dialog, and re-initialised the BaseUserScreen
 *  to display only the available players.
 */
void BaseUserScreen::doDeletePlayer()
{
    PlayerProfile *player = getSelectedPlayer();
    PlayerManager::get()->deletePlayer(player);
    GUIEngine::ModalDialog::dismiss();

    // Special case: the current player was deleted. We have to make sure
    // that there is still a current player (all of STK depends on that).
    if(!PlayerManager::getCurrentPlayer())
    {
        for(unsigned int i=0; i<PlayerManager::get()->getNumPlayers(); i++)
        {
            PlayerProfile *player = PlayerManager::get()->getPlayer(i);
            if(!player->isGuestAccount())
            {
                PlayerManager::get()->setCurrentPlayer(player);
                break;
            }
        }
    }
    init();
}   // doDeletePlayer

// ----------------------------------------------------------------------------
void BaseUserScreen::unloaded()
{
}   // unloaded



// ============================================================================
/** In the tab version, make sure the right tab is selected.
 */
void TabbedUserScreen::init()
{
    RibbonWidget* tab_bar = getWidget<RibbonWidget>("options_choice");
    assert(tab_bar != NULL);
    tab_bar->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    tab_bar->select("tab_players", PLAYER_ID_GAME_MASTER);
    BaseUserScreen::init();
}   // init

// ----------------------------------------------------------------------------
/** Switch to the correct tab.
 */
void TabbedUserScreen::eventCallback(GUIEngine::Widget* widget,
                                     const std::string& name,
                                     const int player_id)
{
    if (name == "options_choice")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        Screen *screen = NULL;
        if (selection == "tab_audio")
            screen = OptionsScreenAudio::getInstance();
        else if (selection == "tab_video")
            screen = OptionsScreenVideo::getInstance();
        //else if (selection == "tab_players")
        //    screen = TabbedUserScreen::getInstance();
        else if (selection == "tab_controls")
            screen = OptionsScreenInput::getInstance();
        else if (selection == "tab_ui")
            screen = OptionsScreenUI::getInstance();
        else if (selection == "tab_general")
            screen = OptionsScreenGeneral::getInstance();
        else if (selection == "tab_language")
            screen = OptionsScreenLanguage::getInstance();
        if(screen)
            StateManager::get()->replaceTopMostScreen(screen);
    }
    else
        BaseUserScreen::eventCallback(widget, name, player_id);

}   // eventCallback
