//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014 Joerg Henrichs
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

#include "states_screens/register_screen.hpp"

#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "audio/sfx_manager.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "online/xml_request.hpp"
#include "states_screens/dialogs/registration_dialog.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/user_screen.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace Online;
using namespace irr;
using namespace core;

DEFINE_SCREEN_SINGLETON( RegisterScreen );

// -----------------------------------------------------------------------------

RegisterScreen::RegisterScreen() : Screen("online/register.stkgui")
{
    m_existing_player = NULL;
}   // RegisterScreen

// -----------------------------------------------------------------------------
void RegisterScreen::init()
{
    Screen::init();

    // If there is no player (i.e. first start of STK), try to pick
    // a good default name
    stringw username = "";
    if(m_existing_player)
    {
        username = m_existing_player->getName();
    }
    else if (PlayerManager::get()->getNumPlayers() == 0)
    {
        if (getenv("USERNAME") != NULL)        // for windows
            username = getenv("USERNAME");
        else if (getenv("USER") != NULL)       // Linux, Macs
            username = getenv("USER");
        else if (getenv("LOGNAME") != NULL)    // Linux, Macs
            username = getenv("LOGNAME");
    }

    getWidget<TextBoxWidget>("local_username")->setText(username);

    TextBoxWidget *password_widget = getWidget<TextBoxWidget>("password");
    password_widget->setPasswordBox(true, L'*');
    password_widget = getWidget<TextBoxWidget>("password_confirm");
    password_widget->setPasswordBox(true, L'*');

    m_info_widget = getWidget<LabelWidget>("info");
    assert(m_info_widget);
    m_info_widget->setDefaultColor();
    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget);

    m_signup_request = NULL;
    m_info_message_shown = false;

    getWidget<CheckBoxWidget>("online")->setVisible(true);
    getWidget<LabelWidget>("label_online")->setVisible(true);
    onDialogClose();
    bool online =    UserConfigParams::m_internet_status
                  != Online::RequestManager::IPERM_NOT_ALLOWED;
    getWidget<CheckBoxWidget>("online")->setState(online);
    makeEntryFieldsVisible(online);
}   // init

// -----------------------------------------------------------------------------
void RegisterScreen::setRename(PlayerProfile *player)
{
    m_existing_player = player;
}   // setRename

// -----------------------------------------------------------------------------
/** Will be called first time STK is started, when the 'internet yes/no' dialog
 *  is closed. Adjust the state of the online checkbox depending on that
 *  answer.
 */
void RegisterScreen::onDialogClose()
{
    bool online =    UserConfigParams::m_internet_status
                  != Online::RequestManager::IPERM_NOT_ALLOWED;
    getWidget<CheckBoxWidget>("online")->setState(online);
    makeEntryFieldsVisible(online);
}   // onDialogClose

// -----------------------------------------------------------------------------
/** Shows or hides the entry fields for online registration, depending on
 *  online mode.
 *  \param online True if an online account should be created.
 */
void RegisterScreen::makeEntryFieldsVisible(bool online)
{
    // In case of a rename, hide all other fields.
    if(m_existing_player)
    {
        m_info_widget->setVisible(false);
        getWidget<CheckBoxWidget>("online")->setVisible(false);
        getWidget<LabelWidget>("label_online")->setVisible(false);
        online = false;
    }

    getWidget<TextBoxWidget>("username")->setVisible(online);
    getWidget<LabelWidget  >("label_username")->setVisible(online);
    getWidget<TextBoxWidget>("password")->setVisible(online);
    getWidget<LabelWidget  >("label_password")->setVisible(online);
    getWidget<TextBoxWidget>("password_confirm")->setVisible(online);
    getWidget<LabelWidget  >("label_password_confirm")->setVisible(online);
    getWidget<TextBoxWidget>("email")->setVisible(online);
    getWidget<LabelWidget  >("label_email")->setVisible(online);
    getWidget<TextBoxWidget>("email_confirm")->setVisible(online);
    getWidget<LabelWidget  >("label_email_confirm")->setVisible(online);
}   // makeEntryFieldsVisible

// -----------------------------------------------------------------------------
/** If necessary creates the local user.
 *  \param local_name Name of the local user.
 */
void RegisterScreen::handleLocalName(const stringw &local_name)
{
    if (local_name.size() == 0)
        return;

    // If a local player with that name does not exist, create one
    if(!PlayerManager::get()->getPlayer(local_name))
    {
        PlayerProfile *player;
        // If it's a rename, change the name of the player
        if(m_existing_player && local_name.size()>0)
        {
            m_existing_player->setName(local_name);
            player = m_existing_player;
        }
        else
        {
            player = PlayerManager::get()->addNewPlayer(local_name);
        }
        PlayerManager::get()->save();
        if (player)
        {
            PlayerManager::get()->setCurrentPlayer(player);
        }
        else
        {
            m_info_widget->setErrorColor();
            m_info_widget->setText(_("Could not create player '%s'.", local_name),
                                   false);
        }
    }
    else
    {
        m_info_widget->setErrorColor();
        m_info_widget->setText(_("Could not create player '%s'.", local_name),
                               false);
    }
}   // handleLocalName

// -----------------------------------------------------------------------------
/** Handles the actual registration process. It does some tests on id, password
 *  and email address, then submits a corresponding request.
 */
void RegisterScreen::doRegister()
{
    stringw local_name = getWidget<TextBoxWidget>("local_username")
                       ->getText().trim();

    handleLocalName(local_name);

    // If no online account is requested, don't register
    if(!getWidget<CheckBoxWidget>("online")->getState() || m_existing_player)
    {
        StateManager::get()->popMenu();
        m_existing_player = NULL;
        return;
    }

    stringw username = getWidget<TextBoxWidget>("username")->getText().trim();
    stringw password = getWidget<TextBoxWidget>("password")->getText().trim();
    stringw password_confirm =  getWidget<TextBoxWidget>("password_confirm")
                             ->getText().trim();
    stringw email = getWidget<TextBoxWidget>("email")->getText().trim();
    stringw email_confirm = getWidget<TextBoxWidget>("email_confirm")
                           ->getText().trim();

    m_info_widget->setErrorColor();

    if (password != password_confirm)
    {
        m_info_widget->setText(_("Passwords don't match!"), false);
    }
    else if (email != email_confirm)
    {
        m_info_widget->setText(_("Emails don't match!"), false);
    }
    else if (username.size() < 3 || username.size() > 30)
    {
        m_info_widget->setText(_("Online username has to be between 3 and 30 characters long!"), false);
    }
    else if (password.size() < 8 || password.size() > 30)
    {
        m_info_widget->setText(_("Password has to be between 8 and 30 characters long!"), false);
    }
    else if (email.size() < 4 || email.size() > 50)
    {
        m_info_widget->setText(_("Email has to be between 4 and 50 characters long!"), false);
    }
    else if (  email.find(L"@")== -1 || email.find(L".")== -1 ||
              (email.findLast(L'.') - email.findLast(L'@') <= 2 ) ||
                email.findLast(L'@')==0 )
    {
       m_info_widget->setText(_("Email is invalid!"), false);
    }
    else
    {
        m_info_widget->setDefaultColor();
        new RegistrationDialog();
        if (local_name.size() > 0)
        {
            PlayerProfile *player = PlayerManager::get()->getPlayer(local_name);
            if (player)
            {
                player->setLastOnlineName(username);
                player->setWasOnlineLastTime(true);
            }
        }
        return;
    }

    SFXManager::get()->quickSound( "anvil" );
}   // doRegister

// -----------------------------------------------------------------------------
/** Called from the registration info dialog when 'accept' is clicked.
 */
void RegisterScreen::acceptTerms()
{
    m_options_widget->setDeactivated();

    core::stringw username = getWidget<TextBoxWidget>("username")->getText().trim();
    core::stringw password = getWidget<TextBoxWidget>("password")->getText().trim();
    core::stringw password_confirm= getWidget<TextBoxWidget>("password_confirm")->getText().trim();
    core::stringw email = getWidget<TextBoxWidget>("email")->getText().trim();

    m_signup_request = new XMLRequest();
    m_signup_request->setApiURL(API::USER_PATH, "register");
    m_signup_request->addParameter("username",         username        );
    m_signup_request->addParameter("password",         password        );
    m_signup_request->addParameter("password_confirm", password_confirm);
    m_signup_request->addParameter("email",            email           );
    m_signup_request->addParameter("terms",            "on"            );
    m_signup_request->queue();
}   // acceptTerms

// -----------------------------------------------------------------------------

void RegisterScreen::onUpdate(float dt)
{
    if(m_signup_request)
    {
        if(!m_options_widget->isActivated())
            m_info_widget->setText(StringUtils::loadingDots(_("Validating info")),
                                   false);

        if(m_signup_request->isDone())
        {
            if(m_signup_request->isSuccess())
            {
                new MessageDialog(
                    _("You will receive an email with further instructions "
                    "regarding account activation. Please be patient and be "
                    "sure to check your spam folder."),
                    MessageDialog::MESSAGE_DIALOG_OK, NULL, false);
                // Set the flag that the message was shown, which will triger
                // a pop of this menu and so a return to the main menu
                m_info_message_shown = true;
            }
            else
            {
                // Error signing up, display error message
                m_info_widget->setText(m_signup_request->getInfo(), false);
            }
            delete m_signup_request;
            m_signup_request = NULL;
            m_options_widget->setActivated();
        }
    }
    else if(m_info_message_shown && !ModalDialog::isADialogActive())
    {
        // Once the info message was shown (signup was successful), but the
        // message has been gone (user clicked on OK), go back to main menu
        StateManager::get()->popMenu();
    }
}   // onUpdate

// -----------------------------------------------------------------------------

void RegisterScreen::eventCallback(Widget* widget, const std::string& name,
                                const int playerID)
{
    if (name == "online")
    {
        if (UserConfigParams::m_internet_status == Online::RequestManager::IPERM_NOT_ALLOWED)
        {
            m_info_widget->setErrorColor();
            m_info_widget->setText(_("Internet access is disabled, please enable it in the options"), false);
            getWidget<CheckBoxWidget>("online")->setState(false);
        }
        else
            makeEntryFieldsVisible(getWidget<CheckBoxWidget>("online")->getState());
    }
    else if (name=="options")
    {
        const std::string button = m_options_widget
                                 ->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if(button=="next")
        {
            doRegister();
        }
        else if(button=="cancel")
        {
            // We poop this menu, onEscapePress will handle the special case
            // of e.g. a fresh start of stk that is aborted.
            StateManager::get()->popMenu();
            onEscapePressed();
        }
    }
    else if (name == "back")
    {
        m_existing_player = NULL;
        StateManager::get()->escapePressed();
    }

}   // eventCallback

// -----------------------------------------------------------------------------
bool RegisterScreen::onEscapePressed()
{
    m_existing_player = NULL;
    if (PlayerManager::get()->getNumPlayers() == 0)
    {
        // Must be first time start, and player cancelled player creation
        // so quit stk. At this stage there are two menus on the stack:
        // 1) The UserScreen,  2) RegisterStreen
        // Popping them both will trigger STK to close.
        StateManager::get()->popMenu();
        return true;
    }
    return true;
}   // onEscapePressed

