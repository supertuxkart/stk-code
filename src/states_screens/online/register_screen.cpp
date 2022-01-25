//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 Joerg Henrichs
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

#include "states_screens/online/register_screen.hpp"

#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "audio/sfx_manager.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "online/link_helper.hpp"
#include "online/request_manager.hpp"
#include "online/xml_request.hpp"
#include "states_screens/dialogs/registration_dialog.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/options/user_screen.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"

#ifdef __SWITCH__
extern "C" {
  #define u64 uint64_t
  #define u32 uint32_t
  #define s64 int64_t
  #define s32 int32_t
  #include <switch/services/acc.h>
  #undef u64
  #undef u32
  #undef s64
  #undef s32
}
#endif

using namespace GUIEngine;
using namespace Online;
using namespace irr;
using namespace core;

// -----------------------------------------------------------------------------

RegisterScreen::RegisterScreen() : Screen("online/register.stkgui")
{
    m_existing_player = NULL;
    m_account_mode    = ACCOUNT_OFFLINE;
    m_parent_screen   = NULL;
}   // RegisterScreen

// -----------------------------------------------------------------------------
void RegisterScreen::init()
{
    if (m_existing_player)
        getWidget("create_user")->setText(_("Rename"));
    else
        getWidget("create_user")->setText(_("Create User"));

    getWidget<TextBoxWidget>("username")->setText(L"");
    m_info_widget = getWidget<LabelWidget>("info");
    assert(m_info_widget);
    m_info_widget->setDefaultColor();
    m_info_widget->setText(L"", false);
    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget);
    m_password_widget = getWidget<TextBoxWidget>("password");
    assert(m_password_widget);

    RibbonWidget* ribbon = getWidget<RibbonWidget>("mode_tabs");
    assert(ribbon);
    if (UserConfigParams::m_internet_status !=
        Online::RequestManager::IPERM_NOT_ALLOWED)
    {
        m_account_mode = ACCOUNT_NEW_ONLINE;
        ribbon->select("tab_new_online", PLAYER_ID_GAME_MASTER);
    }
    else
    {
        m_account_mode = ACCOUNT_OFFLINE;
        ribbon->select("tab_offline", PLAYER_ID_GAME_MASTER);
    }

    // Hide the tabs in case of a rename
    ribbon->setVisible(m_existing_player == NULL);
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
#ifndef MOBILE_STK
    // For mobile stk always use the name Player as in iOS the following
    // getenv return "mobile" for some reason
#if defined(WIN32)
        std::vector<wchar_t> env;
        // An environment variable has a maximum size limit of 32,767 characters
        env.resize(32767, 0);
        DWORD length = GetEnvironmentVariable(L"USERNAME", env.data(), 32767);
        if (length != 0)
            username = env.data();
#elif defined(__SWITCH__)
        AccountUid uid;
        // It's possible the user is using an app that doesn't need a user selection
        // We try the last opened user as well
        if(R_SUCCEEDED(accountInitialize(AccountServiceType_Application)))
        {
            if(R_SUCCEEDED(accountGetPreselectedUser(&uid)) || R_SUCCEEDED(accountGetLastOpenedUser(&uid)))
            {
                AccountProfile profile;
                if(R_SUCCEEDED(accountGetProfile(&profile, uid)))
                {
                    AccountProfileBase profileBase;
                    if(R_SUCCEEDED(accountProfileGet(&profile, NULL, &profileBase)))
                        username = profileBase.nickname;
                    accountProfileClose(&profile);
                }
            }
            accountExit();
        }
#else
        if (getenv("USER") != NULL)          // Linux, Macs
            username = getenv("USER");
        else if (getenv("LOGNAME") != NULL)  // Linux, Macs
            username = getenv("LOGNAME");
#endif
#endif
    }

    TextBoxWidget* local_username = getWidget<TextBoxWidget>("local_username");
    local_username->setText(username);

    m_password_widget->setPasswordBox(true, L'*');
    getWidget<TextBoxWidget>("password_confirm")->setPasswordBox(true, L'*');

    m_signup_request = nullptr;
    m_info_message_shown = false;

    onDialogClose();
    makeEntryFieldsVisible();

    // The behaviour of the screen is slightly different at startup, i.e.
    // when it is the first screen: cancel will exit the game, and in
    // this case no 'back' error should be shown.
    bool has_player_profile = (PlayerManager::get()->getNumPlayers() > 0);
    getWidget<IconButtonWidget>("back")->setVisible(has_player_profile);
    getWidget<IconButtonWidget>("cancel")->setLabel(has_player_profile
        ? _("Cancel")
        : _("Exit game"));
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
    m_account_mode = online ? ACCOUNT_NEW_ONLINE : ACCOUNT_OFFLINE;

    RibbonWidget* ribbon = getWidget<RibbonWidget>("mode_tabs");
    assert(ribbon);
    if (m_account_mode == ACCOUNT_NEW_ONLINE)
    {
        ribbon->select("tab_new_online", PLAYER_ID_GAME_MASTER);
    }
    else
    {
        m_account_mode = ACCOUNT_OFFLINE;
        ribbon->select("tab_offline", PLAYER_ID_GAME_MASTER);
    }
    makeEntryFieldsVisible();
}   // onDialogClose

// -----------------------------------------------------------------------------
void RegisterScreen::onFocusChanged(GUIEngine::Widget* previous, 
                                    GUIEngine::Widget* focus,  int playerID)
{
    TextBoxWidget *online_name = getWidget<TextBoxWidget>("username");
    if (focus == online_name)
    {
        TextBoxWidget *local_name = getWidget<TextBoxWidget>("local_username");
        if (online_name->getText() == "")
            online_name->setText(local_name->getText());
    }
}   // onFocusChanged

// -----------------------------------------------------------------------------
/** Shows or hides the entry fields for online registration, depending on
 *  online mode.
 *  \param online True if an online account should be created.
 */
void RegisterScreen::makeEntryFieldsVisible()
{
    // In case of a rename, hide all other fields.
    if(m_existing_player)
    {
        m_info_widget->setVisible(false);
        m_account_mode = ACCOUNT_OFFLINE;
    }

    bool online = m_account_mode != ACCOUNT_OFFLINE;
    getWidget<TextBoxWidget>("username")->setVisible(online);
    getWidget<LabelWidget  >("label_username")->setVisible(online);
    m_password_widget->setVisible(online);
    getWidget<LabelWidget  >("label_password")->setVisible(online);

    bool new_account = online && (m_account_mode == ACCOUNT_NEW_ONLINE);
    getWidget<TextBoxWidget>("password_confirm")->setVisible(new_account);
    getWidget<LabelWidget  >("label_password_confirm")->setVisible(new_account);
    getWidget<TextBoxWidget>("email")->setVisible(new_account);
    getWidget<TextBoxWidget>("email")->setTextBoxType(TBT_EMAIL);
    getWidget<LabelWidget  >("label_email")->setVisible(new_account);
    if(getWidget<TextBoxWidget>("email_confirm"))
    {
        getWidget<TextBoxWidget>("email_confirm")->setVisible(new_account);
        getWidget<LabelWidget  >("label_email_confirm")->setVisible(new_account);
    }

    getWidget<ButtonWidget >("password_reset")->setVisible(LinkHelper::isSupported() && (online && !new_account));
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
                       
    if (local_name.empty())
    {
        m_info_widget->setErrorColor();
        m_info_widget->setText(_("User name cannot be empty."), false);
        return;
    }

    handleLocalName(local_name);

    // If no online account is requested, don't register
    if(m_account_mode==ACCOUNT_EXISTING_ONLINE)
    {
        core::stringw password = m_password_widget->getText();
        core::stringw online_name = getWidget<TextBoxWidget>("username")->getText().trim();
        m_parent_screen->setNewAccountData(true, /*auto login*/true,
                                           online_name, password);
        StateManager::get()->popMenu();
        return;
    }
    else if(m_account_mode==ACCOUNT_OFFLINE)
    {
        m_existing_player = NULL;
        StateManager::get()->popMenu();
        return;
    }

    stringw username = getWidget<TextBoxWidget>("username")->getText().trim();
    stringw password = m_password_widget->getText().trim();
    stringw password_confirm =  getWidget<TextBoxWidget>("password_confirm")
                             ->getText().trim();
    stringw email = getWidget<TextBoxWidget>("email")->getText().trim();

    // If there is an email_confirm field, use it and check if the email
    // address is correct. If there is no such field, set the confirm email
    // address to email address (so the test below will be passed).
    stringw email_confirm = getWidget<TextBoxWidget>("email_confirm") 
                          ? getWidget<TextBoxWidget>("email_confirm")->getText()
                          : getWidget<TextBoxWidget>("email")->getText();
    email_confirm.trim();
    m_info_widget->setErrorColor();

    bool namecheck = false;
    for (unsigned i = 0; i < username.size(); i++)
    {
        if (!((username[i] >= '0' && username[i] <= '9') ||
            (username[i] >= 'a' && username[i] <= 'z') ||
            (username[i] >= 'A' && username[i] <= 'Z') ||
            username[i] == '.' || username[i] == '-' || username[i] == '_'))
        {
            namecheck = true;
            break;
        }
    }
    if (password != password_confirm)
    {
        m_info_widget->setText(_("Passwords don't match!"), false);
    }
    else if (username == password)
    {
        m_info_widget->setText(_("Online username and password must not be the same!"), false);
    }
    else if (email != email_confirm)
    {
        m_info_widget->setText(_("Emails don't match!"), false);
    }
    else if (namecheck)
    {
        m_info_widget->setText(_("Online username can only contain alphanumeric (ASCII) characters, periods, dashes and underscores!"), false);
    }
    else if (username.size() < 3 || username.size() > 30)
    {
        m_info_widget->setText(_("Online username has to be between 3 and 30 characters long!"), false);
    }
    else if (username[0]>='0' && username[0]<='9')
    {
        m_info_widget->setText(_("Online username must not start with a number!"), false);
    }
    else if (password.size() < 8 || password.size() > 30)
    {
        m_info_widget->setText(_("Password has to be between 8 and 30 characters long!"), false);
    }
    else if (email.size() < 5 || email.size() > 254)
    {
        m_info_widget->setText(_("Email has to be between 5 and 254 characters long!"), false);
    }
    else if (  email.find(L"@")== -1 || email.find(L".")== -1 ||
              (email.findLast(L'.') - email.findLast(L'@') <= 1 ) ||
                email.findLast(L'@')==0 || email[(email.size())-1]=='.')
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
                core::stringw online_name = getWidget<TextBoxWidget>("username")->getText().trim();
                m_parent_screen->setNewAccountData(/*online*/true, 
                                                   /*auto_login*/false,
                                                   username, password);

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
    m_options_widget->setActive(false);

    core::stringw username = getWidget<TextBoxWidget>("username")->getText().trim();
    core::stringw password = m_password_widget->getText().trim();
    core::stringw password_confirm= getWidget<TextBoxWidget>("password_confirm")->getText().trim();
    core::stringw email = getWidget<TextBoxWidget>("email")->getText().trim();

    m_signup_request = std::make_shared<XMLRequest>();
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
                m_info_widget->setErrorColor();
                m_info_widget->setText(m_signup_request->getInfo(), false);
            }
            m_signup_request = nullptr;
            m_options_widget->setActive(true);
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
    if (name == "mode_tabs")
    {
        RibbonWidget *ribbon = static_cast<RibbonWidget*>(widget);
        std::string selection = ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if ( (selection == "tab_new_online" || selection == "tab_existing_online")
            && (UserConfigParams::m_internet_status == Online::RequestManager::IPERM_NOT_ALLOWED) )
        {
            m_info_widget->setErrorColor();
            m_info_widget->setText(_("Internet access is disabled, please enable it in the options"), false);
            return;
        }
        if (selection == "tab_new_online")
            m_account_mode = ACCOUNT_NEW_ONLINE;
        else if (selection == "tab_existing_online")
            m_account_mode = ACCOUNT_EXISTING_ONLINE;
        else if (selection == "tab_offline")
            m_account_mode = ACCOUNT_OFFLINE;

        makeEntryFieldsVisible();
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
    else if (name == "password_reset")
    {
        // Open password reset page
        Online::LinkHelper::openURL(stk_config->m_password_reset_url);
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

