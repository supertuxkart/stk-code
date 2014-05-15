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
#include "guiengine/widgets/text_box_widget.hpp"
#include "online/messages.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/options_screen_audio.hpp"
#include "states_screens/options_screen_input.hpp"
#include "states_screens/options_screen_ui.hpp"
#include "states_screens/options_screen_video.hpp"
#include "states_screens/register_screen.hpp"
#include "states_screens/state_manager.hpp"


using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( UserScreen       );
DEFINE_SCREEN_SINGLETON( TabbedUserScreen );

// ----------------------------------------------------------------------------

BaseUserScreen::BaseUserScreen(const std::string &name) : Screen(name.c_str())
{
}   // BaseUserScreen

// ----------------------------------------------------------------------------

void BaseUserScreen::loadedFromFile()
{

}   // loadedFromFile

// ----------------------------------------------------------------------------

void BaseUserScreen::init()
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

    // It should always be activated ... but just in case
    m_options_widget->setActivated();
    // Clean any error message still shown
    m_info_widget->setText("", true);
    m_info_widget->setErrorColor();

    Screen::init();

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
    {
        m_players->setSelection(current_player_index, PLAYER_ID_GAME_MASTER,
                                /*focus*/ true);
        const stringw &online_name = PlayerManager::getCurrentPlayer()
                                   ->getLastOnlineName();
        m_online_cb->setState(online_name.size()>0);
        m_username_tb->setText(online_name);
        makeEntryFieldsVisible(online_name.size()>0);
    }
    else   // no current player found
    {
        // The first player is the most frequently used, so select it
        if (PlayerManager::get()->getNumPlayers() > 0)
            selectUser(0);
    }

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
/** Called when a user is selected. It updates the online checkbox and
 *  entrye fields.
 */
void BaseUserScreen::selectUser(int index)
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
void BaseUserScreen::makeEntryFieldsVisible(bool online)
{
#ifdef GUEST_ACCOUNTS_ENABLED
    getWidget<LabelWidget>("label_guest")->setVisible(online);
    getWidget<CheckBoxWidget>("guest")->setVisible(online);
#endif
    getWidget<LabelWidget>("label_username")->setVisible(online);
    m_username_tb->setVisible(online);
    getWidget<LabelWidget>("label_password")->setVisible(online);
    m_password_tb->setVisible(online);
}   // makeEntryFieldsVisible

// ----------------------------------------------------------------------------
/** Make sure that a current player is defined when escape is pressed.
 */
bool BaseUserScreen::onEscapePressed()
{
    return Screen::onEscapePressed();
}   // onEscapePressed

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
            login();
        }   // button==ok
        else if (button == "new_user")
        {
            StateManager::get()->pushScreen(RegisterScreen::getInstance());
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
            StateManager::get()->pushScreen(RegisterScreen::getInstance());
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
/** Closes the BaseUserScreen, and makes sure that the right screen is displayed
 *  next.
 */
void BaseUserScreen::closeScreen()
{
    StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
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
    m_options_widget->setDeactivated();

    PlayerProfile *profile = getSelectedPlayer();
    PlayerManager::get()->setCurrentPlayer(profile);
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
            m_options_widget->setActivated();
            return;
        }
        profile->requestSignIn(m_username_tb->getText(),
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
        m_info_widget->setText(Online::Messages::loadingDots( _("Signing in")),
                               false);
}   // onUpdate

// ----------------------------------------------------------------------------
/** Callback from player profile if login was successful.
 */
void BaseUserScreen::loginSuccessful()
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
void BaseUserScreen::loginError(const irr::core::stringw & error_message)
{
    sfx_manager->quickSound("anvil");
    m_info_widget->setErrorColor();
    m_info_widget->setText(error_message, false);
    m_options_widget->setActivated();
}   // loginError

// ----------------------------------------------------------------------------
void BaseUserScreen::newUserAdded(const irr::core::stringw &local_name,
                              const irr::core::stringw &online_name)
{
    PlayerProfile *player = PlayerManager::get()->getPlayer(local_name);
    PlayerManager::get()->setCurrentPlayer(player);
    player->setLastOnlineName(online_name);
}   // newUserAdded

// ----------------------------------------------------------------------------
/** Called when a player will be deleted.
 */
void BaseUserScreen::deletePlayer()
{
    PlayerProfile *player = getSelectedPlayer();
    irr::core::stringw message =
        //I18N: In the player info dialog (when deleting)
        _("Do you really want to delete player '%s' ?", player->getName());

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
    init();
}   // doDeletePlayer

// ----------------------------------------------------------------------------
void BaseUserScreen::unloaded()
{
}   // unloaded


// ----------------------------------------------------------------------------
/** Gets called when a dialog closes. At a first time start of STK the
 *  internet dialog is shown first. Only when this dialog closes is it possible
 *  to open the next dialog, which is the one to create a new player (which
 *  is conventient on a first start).
 */
void BaseUserScreen::onDialogClose()
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
        StateManager::get()->pushScreen(RegisterScreen::getInstance());
    }   // getNumPlayers == 0
}   // onDialogClose


// ============================================================================
/** In the tab version, make sure the right tab is selected.
 */
void TabbedUserScreen::init()
{
    RibbonWidget* tab_bar = this->getWidget<RibbonWidget>("options_choice");
    if (tab_bar != NULL) tab_bar->select("tab_players", PLAYER_ID_GAME_MASTER);
    tab_bar->getRibbonChildren()[0].setTooltip( _("Graphics") );
    tab_bar->getRibbonChildren()[1].setTooltip( _("Audio") );
    tab_bar->getRibbonChildren()[2].setTooltip(_("User Interface"));
    tab_bar->getRibbonChildren()[4].setTooltip( _("Controls") );
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
        const std::string &selection = 
            ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        Screen *s;
        if      (selection=="tab_audio"   ) s = OptionsScreenAudio::getInstance();
        else if (selection=="tab_video"   ) s = OptionsScreenVideo::getInstance();
        else if (selection=="tab_players" ) s = TabbedUserScreen::getInstance();
        else if (selection=="tab_controls") s = OptionsScreenInput::getInstance();
        else if (selection=="tab_ui"      ) s = OptionsScreenUI::getInstance();
        assert(s);
        StateManager::get()->replaceTopMostScreen(s);
    }
    else
        BaseUserScreen::eventCallback(widget, name, player_id);

}   // eventCallback
