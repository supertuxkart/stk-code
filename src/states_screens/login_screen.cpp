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

#include "states_screens/login_screen.hpp"

#include "audio/sfx_manager.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "online/current_user.hpp"
#include "online/messages.hpp"
#include "states_screens/guest_login_screen.hpp"
#include "states_screens/online_screen.hpp"
#include "states_screens/register_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"

#include <irrString.h>

using namespace GUIEngine;
using namespace irr;

DEFINE_SCREEN_SINGLETON( LoginScreen );

// -----------------------------------------------------------------------------

LoginScreen::LoginScreen() : Screen("online/login.stkgui")
{
}   // LoginScreen

// -----------------------------------------------------------------------------
void LoginScreen::init()
{
    Screen::init();
    // Make sure this tab is actually focused.
    RibbonWidget* tabs = this->getWidget<RibbonWidget>("login_tabs");
    if (tabs) tabs->select( "tab_login", PLAYER_ID_GAME_MASTER );

    TextBoxWidget *password_widget = getWidget<TextBoxWidget>("password");
    password_widget->setPasswordBox(true,L'*');
    
    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget);
    m_options_widget->setActivated();

    m_info_widget = getWidget<LabelWidget>("info");
    assert(m_info_widget != NULL);
    m_success = false;

    // As default don't select 'remember'
    getWidget<CheckBoxWidget>("remember")->setState(false);
}   // init

// -----------------------------------------------------------------------------
/** Collects the data entered into the gui and submits a login request.
 *  The login request is processes asynchronously b the ReqeustManager.
 */
void LoginScreen::login()
{
    // Reset any potential error message shown.
    LabelWidget *info_widget = getWidget<LabelWidget>("info");
    info_widget->setDefaultColor();
    info_widget->setText("", false);

    const core::stringw username = getWidget<TextBoxWidget>("username")
                            ->getText().trim();
    const core::stringw password = getWidget<TextBoxWidget>("password")
                            ->getText().trim();

    if (username.size() < 4 || username.size() > 30 || 
        password.size() < 8 || password.size() > 30    )
    {
        sfx_manager->quickSound("anvil");
        info_widget->setErrorColor();
        info_widget->setText(_("Username and/or password too short or too long."),
                             false);
    }
    else
    {
        m_options_widget->setDeactivated();
        info_widget->setDefaultColor();
        bool remember = getWidget<CheckBoxWidget>("remember")->getState();
        Online::CurrentUser::get()->requestSignIn(username,password, 
                                                  remember           );
    }
}   // login

// -----------------------------------------------------------------------------
/** Called from the login request when it was successfully exected.
 */
void LoginScreen::loginSuccessful()
{
    // The callback is done from the main thread, so no need to sync
    // access to m_success
    m_success = true;
}   // loginSuccessful

// -----------------------------------------------------------------------------
/** Called from the login request when the login was not successful.
 *  \param error_message The error message from the server (or curl).
 */
void LoginScreen::loginError(const irr::core::stringw & error_message)
{
    sfx_manager->quickSound("anvil");
    LabelWidget *info_widget = getWidget<LabelWidget>("info");
    info_widget->setErrorColor();
    info_widget->setText(error_message, false);

    m_options_widget->setActivated();
}   // loginError

// -----------------------------------------------------------------------------
/** Called in each frame. If a successful login is detected, the online screen
 *  will be displayed.
 */
void LoginScreen::onUpdate(float dt)
{

    if(!m_options_widget->isActivated())
        m_info_widget->setText(Online::Messages::signingIn(), false);

    // Login was successful, so put the online main menu on the screen
    if(m_success)
    {
        StateManager::get()->replaceTopMostScreen(OnlineScreen::getInstance());    
    }
}   // onUpdate

// -----------------------------------------------------------------------------
/** Called when the user clicks on a widget.
 *  \param widget that was clicked on.
 *  \param name Name of the widget.
 *  \param playerID The id of the player who clicked the item.
 */
void LoginScreen::eventCallback(Widget* widget, const std::string& name, 
                                const int playerID)
{
    if (name == "login_tabs")
    {
        StateManager *sm = StateManager::get();
        const std::string selection = 
            ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "tab_guest_login")
            sm->replaceTopMostScreen(GuestLoginScreen::getInstance()); 
        else if (selection == "tab_register")
            sm->replaceTopMostScreen(RegisterScreen::getInstance());
    }
    else if (name=="options")
    {
        const std::string button = 
             getWidget<RibbonWidget>("options")
             ->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if(button=="sign_in")
        {
            login();
        }
        else if(button=="recovery")
        {
        }
        else if(button=="cancel")
            StateManager::get()->escapePressed();
    }

}   // eventCallback

// -----------------------------------------------------------------------------
