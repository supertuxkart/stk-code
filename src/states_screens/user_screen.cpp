//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013 Marianne Gagnon
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

#include "states_screens/user_screen.hpp"

#include "audio/sfx_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "online/messages.hpp"
#include "states_screens/dialogs/enter_player_name_dialog.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/register_screen.hpp"
#include "states_screens/state_manager.hpp"


using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( UserScreen );

// ----------------------------------------------------------------------------

UserScreen::UserScreen() : Screen("user_screen.stkgui")
{
    m_is_popup_window = false;
}   // UserScreen

// ----------------------------------------------------------------------------

void UserScreen::loadedFromFile()
{

}   // loadedFromFile

// ----------------------------------------------------------------------------

void UserScreen::init()
{
    m_online_cb = getWidget<CheckBoxWidget>("online");
    assert(m_online_cb);
    m_username_tb = getWidget<TextBoxWidget >("username");
    assert(m_username_tb);
    m_password_tb = getWidget<TextBoxWidget >("password");
    assert(m_password_tb);
    m_password_tb->setPasswordBox(true, L'*');
    m_players = getWidget<DynamicRibbonWidget>("players");
    assert(m_players);
    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget);
    m_info_widget = getWidget<LabelWidget>("message");
    assert(m_info_widget);

    // Make sure this tab is actually focused.
    RibbonWidget* tabs = getWidget<RibbonWidget>("login_tabs");
    if (tabs) tabs->select( "tab_login", PLAYER_ID_GAME_MASTER );

    // It should always be activated ... but just in case
    m_options_widget->setActivated();
    // Clean any error message still shown
    m_info_widget->setText("", true);
    m_info_widget->setErrorColor();

    Screen::init();
    PlayerProfile *player = PlayerManager::getCurrentPlayer();
    if (player && !m_is_popup_window)
    {
        StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
        return;
    }

    m_players->clearItems();
    std::string current_player_index="";

    for (unsigned int n=0; n<PlayerManager::get()->getNumPlayers(); n++)
    {
        const PlayerProfile *player = PlayerManager::get()->getPlayer(n);
        if (player->isGuestAccount()) continue;
        std::string s = StringUtils::toString(n);
        m_players->addItem(player->getName(), s, "/karts/nolok/nolokicon.png", 0, 
                           IconButtonWidget::ICON_PATH_TYPE_RELATIVE);
        if(player==PlayerManager::getCurrentPlayer())
            current_player_index = s;
    }

    m_players->updateItemDisplay();

    // Select the current player. That can only be done after 
    // updateItemDisplay is called.
    if(current_player_index.size()>0)
            m_players->setSelection(current_player_index, PLAYER_ID_GAME_MASTER,
                                    /*focus*/ true);
    else   // no current player found
    {
        // The first player is the most frequently used, so select it
        if (PlayerManager::get()->getNumPlayers() > 0)
            selectUser(0);
    }

}   // init

// ----------------------------------------------------------------------------
PlayerProfile* UserScreen::getSelectedPlayer()
{
    const std::string &s_id = m_players
                            ->getSelectionIDString(PLAYER_ID_GAME_MASTER);
    unsigned int n_id;
    StringUtils::fromString(s_id, n_id);
    return PlayerManager::get()->getPlayer(n_id);
}   // getSelectedPlayer

// ----------------------------------------------------------------------------

void UserScreen::tearDown()
{
    Screen::tearDown();
}   // tearDown

// ----------------------------------------------------------------------------
/** Called when a user is selected. It updates the online checkbox and
 *  entrye fields.
 */
void UserScreen::selectUser(int index)
{
    PlayerProfile *profile = PlayerManager::get()->getPlayer(index);
    assert(profile);
    PlayerProfile *cp = PlayerManager::getCurrentPlayer();
    // If the current user is logged in, a logout is required now.
    if(profile!=cp && cp && cp->isLoggedIn())
        cp->requestSignOut();

    getWidget<TextBoxWidget >("username")->setText(profile
                                                   ->getLastOnlineName());
    m_players->setSelection(StringUtils::toString(index), 0, /*focusIt*/true);

    // Last game was not online, so make the offline settings the default
    // (i.e. unckeck online checkbox, and make entry fields invisible).
    if (profile->getLastOnlineName() == "")
    {
        m_online_cb->setState(false);
        makeEntryFieldsVisible(false);
        return;
    }

    // Now last use was with online --> Display the saved data
    m_online_cb->setState(true);
    makeEntryFieldsVisible(true);
    m_username_tb->setText(profile->getLastOnlineName());

    // And make the password invisible if the session is saved (i.e
    // the user does not need to enter a password).
    if (profile->hasSavedSession())
    {
        m_password_tb->setVisible(false);
        getWidget<LabelWidget>("label_password")->setVisible(false);
    }

}   // selectUser

// ----------------------------------------------------------------------------
/** Make the entry fields either visible or invisible.
 *  \param online Online state, which dicates if the entry fields are
 *         visible (true) or not.
 */
void UserScreen::makeEntryFieldsVisible(bool online)
{
    getWidget<LabelWidget>("label_username")->setVisible(online);
    m_username_tb->setVisible(online);
    getWidget<LabelWidget>("label_password")->setVisible(online);
    m_password_tb->setVisible(online);
}   // makeEntryFieldsVisible

// ----------------------------------------------------------------------------
/** Called when the user selects anything on the screen.
 */
void UserScreen::eventCallback(Widget* widget,
                                         const std::string& name,
                                         const int player_id)
{
    // Clean any error message still shown
    m_info_widget->setText("", true);
    m_info_widget->setErrorColor();

    if (name == "login_tabs")
    {
        const std::string selection =
            ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (selection == "tab_register")
            StateManager::get()->replaceTopMostScreen(RegisterScreen::getInstance());
    }
    else if (name == "players")
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
    else if (name == "online")
    {
        // If online access is not allowed, do not accept an online account
        // but advice the user where to enable this option.
        if (m_online_cb->getState() && UserConfigParams::m_internet_status ==
                                       Online::RequestManager::IPERM_NOT_ALLOWED)
        {
            m_info_widget->setText(
                "Internet access is disabled, please enable it in the options",
                true);
            sfx_manager->quickSound( "anvil" );
            m_online_cb->setState(false);
        }
        makeEntryFieldsVisible(m_online_cb->getState());
    }
    else if (name == "options")
    {
        const std::string &button = 
                             m_options_widget->getSelectionIDString(player_id);
        if (button == "ok")
        {
            login(UserConfigParams::m_remember_user);
        }   // button==ok
        else if (button == "new_user")
        {
            new EnterPlayerNameDialog(this, 0.5f, 0.4f);
        }
        else if (button == "cancel")
        {
            PlayerProfile *cp = PlayerManager::getCurrentPlayer();
            if(cp && cp->isLoggedIn())
                cp->requestSignOut();
            StateManager::get()->popMenu();
        }
        else if (button == "rename")
        {
            PlayerProfile *cp = getSelectedPlayer();
            new EnterPlayerNameDialog(this, 0.5f, 0.4f, cp->getName());
            // Init will automatically be called, which 
            // refreshes the player list
        }
        else if (button == "delete")
        {
            deletePlayer();
        }
    }   // options

    return;

}   // eventCallback

// ----------------------------------------------------------------------------
/** Closes the UserScreen, and makes sure that the right screen is displayed
 *  next.
 */
void UserScreen::closeScreen()
{
    StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
}   // closeScreen

// ----------------------------------------------------------------------------
/** Called when OK or OK-and-save is clicked.
 *  This will trigger the actual login (if requested) etc.
 *  \param remember_me True if the login details should be remembered,
 *         so that next time this menu can be skipped.
 */
void UserScreen::login(bool remember_me)
{
    // If an error occurs, the callback informing this screen about the
    // problem will activate the widget again.
    m_options_widget->setDeactivated();

    PlayerProfile *profile = getSelectedPlayer();
    PlayerManager::get()->setCurrentPlayer(profile, remember_me);
    assert(profile);

    // If no online login requested, go straight to the main menu screen.
    if(!m_online_cb->getState())
    {
        if(profile->isLoggedIn())
        {
            // The player is logged in, but online is now disabled,
            // so log the player out. There is no error handling of
            // a failed logout request
            profile->requestSignOut();
        }
        closeScreen();
        return;
    }

    // Player wants to be online, and is already online - nothing to do
    if(profile->isLoggedIn())
    {
        closeScreen();
        return;
    }

    // Now we need to start a login request to the server
    // This implies that this screen will wait till the server responds, so
    // that error messages ('invalid password') can be shown, and the user
    // can decide what to do about them.
    if (profile->hasSavedSession())
    {
        // Online login with saved token
        profile->requestSavedSession();
    }
    else
    {
        // Online login with password --> we need a valid password
        if (m_password_tb->getText() == "")
        {
            m_info_widget->setText(_("You need to enter a password."), true);
            sfx_manager->quickSound("anvil");
            return;
        }
        profile->requestSignIn(m_username_tb->getText(),
                               m_password_tb->getText(),
                               remember_me);
    }   // !hasSavedSession

}   // login

// ----------------------------------------------------------------------------
/** Called once every frame. It will replace this screen with the main menu
 *  screen if a successful login happened.
 */
void UserScreen::onUpdate(float dt)
{
    if (!m_options_widget->isActivated())
        m_info_widget->setText(Online::Messages::loadingDots( _("Signing in")),
                               false);
}   // onUpdate

// ----------------------------------------------------------------------------
/** Callback from player profile if login was successful.
 */
void UserScreen::loginSuccessful()
{
    m_options_widget->setActivated();
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
void UserScreen::loginError(const irr::core::stringw & error_message)
{
    sfx_manager->quickSound("anvil");
    m_info_widget->setErrorColor();
    m_info_widget->setText(error_message, false);
    m_options_widget->setActivated();
}   // loginError

// ----------------------------------------------------------------------------
/** Called when a player will be deleted.
 */
void UserScreen::deletePlayer()
{
    PlayerProfile *player = getSelectedPlayer();
    irr::core::stringw message =
        //I18N: In the player info dialog (when deleting)
        _("Do you really want to delete player '%s' ?", player->getName());

    class ConfirmServer : public MessageDialog::IConfirmDialogListener
    {
    public:
        virtual void onConfirm()
        {
            UserScreen::getInstance()->doDeletePlayer();
        }   // onConfirm
    };   // ConfirmServer

    new MessageDialog(message, MessageDialog::MESSAGE_DIALOG_CONFIRM,
                      new ConfirmServer(), true);
}   // deletePlayer

// ----------------------------------------------------------------------------
/** Callback when the user confirms to delete a player. This function actually
 *  deletes the player, discards the dialog, and re-initialised the UserScreen
 *  to display only the available players.
 */
void UserScreen::doDeletePlayer()
{
    PlayerProfile *player = getSelectedPlayer();
    PlayerManager::get()->deletePlayer(player);
    GUIEngine::ModalDialog::dismiss();
    init();
}   // doDeletePlayer

// ----------------------------------------------------------------------------
void UserScreen::unloaded()
{
}   // unloaded


// ----------------------------------------------------------------------------
/** Gets called when a dialog closes. At a first time start of STK the
 *  internet dialog is shown first. Only when this dialog closes is it possible
 *  to open the next dialog, which is the one to create a new player (which
 *  is conventient on a first start).
 */
void UserScreen::onDialogClose()
{
    // To allow players to exit the game without creating a player, we count
    // how often this function was called. The first time is after the 
    // internet allowed dialog, the 2nd time
    static int number_of_calls = 0;
    number_of_calls++;
    if(PlayerManager::get()->getNumPlayers() == 0)
    {
        // Still 0 players after the enter player dialog was shown
        // --> User wanted to abort, so pop this menu, which will
        // trigger the end of STK.
        if (number_of_calls > 1)
        {
            StateManager::get()->popMenu();
            return;
        }
        new EnterPlayerNameDialog(this, 0.5f, 0.4f);
    }   // getNumPlayers == 0
}   // onDialogClose

// ----------------------------------------------------------------------------
/** This is a callback from the new user dialog.
 */
void UserScreen::onNewPlayerWithName(const core::stringw& new_name)
{
    init();
    // Select the newly added player
    selectUser(PlayerManager::get()->getNumPlayers() - 1);
        
    return;
}   // onNewPlayerWithName

// -----------------------------------------------------------------------------

