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

#include "audio/sfx_manager.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "online/current_user.hpp"
#include "online/messages.hpp"
#include "states_screens/dialogs/registration_dialog.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/guest_login_screen.hpp"
#include "states_screens/login_screen.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace Online;

DEFINE_SCREEN_SINGLETON( RegisterScreen );

// -----------------------------------------------------------------------------

RegisterScreen::RegisterScreen() : Screen("online/register.stkgui")
{
}   // RegisterScreen

// -----------------------------------------------------------------------------
void RegisterScreen::init()
{
    // Make sure this tab is actually focused.
    RibbonWidget* tabs = this->getWidget<RibbonWidget>("login_tabs");
    if (tabs) tabs->select( "tab_register", PLAYER_ID_GAME_MASTER );

    TextBoxWidget *password_widget = getWidget<TextBoxWidget>("password");
    password_widget->setPasswordBox(true,L'*');
    password_widget = getWidget<TextBoxWidget>("password_confirm");
    password_widget->setPasswordBox(true,L'*');

    m_info_widget = getWidget<LabelWidget>("info");
    assert(m_info_widget);
    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget);

    m_signup_request = NULL;
    m_info_message_shown = false;
}   // init

// -----------------------------------------------------------------------------
void RegisterScreen::doRegister()
{
    core::stringw username = getWidget<TextBoxWidget>("username")->getText().trim();
    core::stringw password = getWidget<TextBoxWidget>("password")->getText().trim();
    core::stringw password_confirm =  getWidget<TextBoxWidget>("password_confirm")
                          ->getText().trim();
    core::stringw email = getWidget<TextBoxWidget>("email")->getText().trim();
    core::stringw email_confirm = getWidget<TextBoxWidget>("email_confirm")
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
    else if (username.size() < 4 || username.size() > 30)
    {
        m_info_widget->setText(_("Username has to be between 4 and 30 characters long!"), false);
    }
    else if (password.size() < 8 || password.size() > 30)
    {
        m_info_widget->setText(_("Password has to be between 8 and 30 characters long!"), false);
    }
    else if (email.size() < 4 || email.size() > 50)
    {
        m_info_widget->setText(_("Email has to be between 4 and 50 characters long!"), false);
    }
    else
    {
        m_info_widget->setDefaultColor();
        new RegistrationDialog();
        return;
    }

    sfx_manager->quickSound( "anvil" );

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

    m_signup_request = CurrentUser::get()->requestSignUp(username, password, 
                                                         password_confirm, email);
}   // acceptTerms

// -----------------------------------------------------------------------------

void RegisterScreen::onUpdate(float dt)
{
    if(m_signup_request)
    {
        if(!m_options_widget->isActivated())
            m_info_widget->setText(Messages::validatingInfo(), false);

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
    if (name == "login_tabs")
    {
        const std::string selection = 
            ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        StateManager *sm = StateManager::get();
        if (selection == "tab_login")
            sm->replaceTopMostScreen(LoginScreen::getInstance());
        else if (selection == "tab_guest_login")
            sm->replaceTopMostScreen(GuestLoginScreen::getInstance());    
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
            StateManager::get()->escapePressed();
    }

}   // eventCallback

// -----------------------------------------------------------------------------
