//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Glenn De Jonghe
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

#include "states_screens/dialogs/registration_dialog.hpp"

#include <IGUIEnvironment.h>

#include "audio/sfx_manager.hpp"
#include "config/player.hpp"
#include "guiengine/engine.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"
#include "utils/string_utils.hpp"
#include "online/messages.hpp"


using namespace GUIEngine;
using namespace irr;
using namespace irr::gui;
using namespace Online;

// -----------------------------------------------------------------------------

RegistrationDialog::RegistrationDialog() :
        ModalDialog(0.8f,0.9f)
{
    m_sign_up_request = NULL;
    m_self_destroy = false;
    m_show_registration_input = true;
    m_show_registration_terms = false;
    m_show_registration_info = false;
    m_username = "";
    m_email = "";
    m_email_confirm = "";
    m_password = "";
    m_password_confirm = "";
    m_agreement = false;
}

// -----------------------------------------------------------------------------

RegistrationDialog::~RegistrationDialog()
{
    delete m_sign_up_request;
}

// -----------------------------------------------------------------------------

void RegistrationDialog::showRegistrationInput()
{
    m_show_registration_input = false;
    clearWindow();
    m_phase = Input;
    loadFromFile("online/registration_input.stkgui");

    //Password should always be reentered if previous has been clicked, or an error occurred.
    m_password = "";
    m_password_confirm = "";

    m_username_widget = getWidget<TextBoxWidget>("username");
    assert(m_username_widget != NULL);
    m_username_widget->setText(m_username);
    m_username_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

    m_password_widget = getWidget<TextBoxWidget>("password");
    assert(m_password_widget != NULL);
    m_password_widget->setPasswordBox(true,L'*');

    m_password_confirm_widget = getWidget<TextBoxWidget>("password_confirm");
    assert(m_password_confirm_widget != NULL);
    m_password_confirm_widget->setPasswordBox(true,L'*');

    m_email_widget = getWidget<TextBoxWidget>("email");
    assert(m_email_widget != NULL);
    m_email_widget->setText(m_email);

    m_email_confirm_widget = getWidget<TextBoxWidget>("email_confirm");
    assert(m_email_confirm_widget != NULL);
    m_email_confirm_widget->setText(m_email_confirm);

    m_info_widget = getWidget<LabelWidget>("info");
    assert(m_info_widget != NULL);
    m_info_widget->setErrorColor();
    m_info_widget->setText(m_registration_error, false);

    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget != NULL);
    m_next_widget = getWidget<IconButtonWidget>("next");
    assert(m_next_widget != NULL);
    m_cancel_widget = getWidget<IconButtonWidget>("cancel");
    assert(m_cancel_widget != NULL);
}

// -----------------------------------------------------------------------------

void RegistrationDialog::showRegistrationTerms()
{
    m_show_registration_terms = false;
    clearWindow();
    m_phase = Terms;
    loadFromFile("online/registration_terms.stkgui");


    ListWidget * terms_widget = getWidget<ListWidget>("terms");


    terms_widget->addItem("title", "=== STK Terms and Conditions ===", -1 , true );
    terms_widget->addItem("par1", "You must agree to these terms in order to register an account for STK.", -1 , false );
    terms_widget->addItem("par2",
        "Still needs actual content. Preferably in an XML document which can then be parsed to be put here."
        , -1 , false );
    terms_widget->addItem("par3",
        "By checking the box below, you are confirming that you understand these terms."
        "If you have any questions or comments regarding these terms,"
        "one of the members of the development team would gladly assist you."
        , -1 , false );


    m_accept_terms_widget = getWidget<CheckBoxWidget>("accepted");
    assert(m_accept_terms_widget != NULL);
    m_accept_terms_widget->setState(false);
    m_accept_terms_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER); //FIXME set focus on the terms

    m_info_widget = getWidget<LabelWidget>("info");
    assert(m_info_widget != NULL);

    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget != NULL);
    m_previous_widget = getWidget<IconButtonWidget>("previous");
    assert(m_previous_widget != NULL);
    m_next_widget = getWidget<IconButtonWidget>("next");
    assert(m_next_widget != NULL);
    m_cancel_widget = getWidget<IconButtonWidget>("cancel");
    assert(m_cancel_widget != NULL);
    m_next_widget->setDeactivated();
}

// -----------------------------------------------------------------------------

void RegistrationDialog::showRegistrationInfo()
{
    m_show_registration_info = false;
    clearWindow();
    m_phase = Info;
    loadFromFile("online/registration_info.stkgui");

    m_info_widget = getWidget<LabelWidget>("info");
    assert(m_info_widget != NULL);

    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget != NULL);
    m_cancel_widget = getWidget<IconButtonWidget>("cancel");
    assert(m_cancel_widget != NULL);
}

// -----------------------------------------------------------------------------

void RegistrationDialog::processInput()
{
    m_username = m_username_widget->getText().trim();
    m_password = m_password_widget->getText().trim();
    m_password_confirm =  m_password_confirm_widget->getText().trim();
    m_email = m_email_widget->getText().trim();
    m_email_confirm = m_email_confirm_widget->getText().trim();
    //FIXME More validation of registration information
    m_info_widget->setErrorColor();
    if (m_password != m_password_confirm)
    {
        m_info_widget->setText(_("Passwords don't match!"), false);
    }
    else if (m_email != m_email_confirm)
    {
        m_info_widget->setText(_("Emails don't match!"), false);
    }
    else if (m_username.size() < 4 || m_username.size() > 30)
    {
        m_info_widget->setText(_("Username has to be between 4 and 30 characters long!"), false);
    }
    else if (m_password.size() < 8 || m_password.size() > 30)
    {
        m_info_widget->setText(_("Password has to be between 8 and 30 characters long!"), false);
    }
    else if (m_email.size() < 4 || m_email.size() > 50)
    {
        m_info_widget->setText(_("Email has to be between 4 and 50 characters long!"), false);
    }
    else
    {
        m_show_registration_terms = true;
        return;
    }
    sfx_manager->quickSound( "anvil" );
}


// -----------------------------------------------------------------------------

bool RegistrationDialog::processInputEvent(const std::string& eventSource)
{
    if (m_phase == Input)
    {
        if (eventSource == m_next_widget->m_properties[PROP_ID])
        {
            processInput();
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------

bool RegistrationDialog::processTermsEvent(const std::string& eventSource)
{
    if (m_phase == Terms)
    {
        if (eventSource == m_next_widget->m_properties[PROP_ID])
        {
            assert(m_accept_terms_widget->getState());
            m_options_widget->setDeactivated();
            m_info_widget->setDefaultColor();
            m_info_widget->setText(Messages::signingUp(), false);
            m_sign_up_request = CurrentUser::get()->requestSignUp(m_username, m_password, m_password_confirm, m_email, true);
            return true;
        }
        else if (eventSource == m_accept_terms_widget->m_properties[PROP_ID])
        {
            bool new_state = !m_accept_terms_widget->getState();
            m_accept_terms_widget->setState(new_state);
            if(new_state)
                m_next_widget->setActivated();
            else
                m_next_widget->setDeactivated();
            return true;
        }
        else if (eventSource == m_previous_widget->m_properties[PROP_ID])
        {
            m_show_registration_info = true;
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------

bool RegistrationDialog::processInfoEvent(const std::string& eventSource)
{
    return false;
}

// -----------------------------------------------------------------------------

bool RegistrationDialog::onEscapePressed()
{
    return m_cancel_widget->isActivated();
}

// -----------------------------------------------------------------------------

GUIEngine::EventPropagation RegistrationDialog::processEvent(const std::string& eventSource)
{
    std::string selection;
    if (eventSource == m_options_widget->m_properties[PROP_ID])
        selection = m_options_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
    else
        selection = eventSource;
    if (selection == m_cancel_widget->m_properties[PROP_ID])
    {
        m_self_destroy = true;
        return GUIEngine::EVENT_BLOCK;
    }
    else if (processInputEvent(selection) || processTermsEvent(selection) || processInfoEvent(selection))
    {
        return GUIEngine::EVENT_BLOCK;
    }
    return GUIEngine::EVENT_LET;
}

// -----------------------------------------------------------------------------

void RegistrationDialog::onEnterPressedInternal()
{

    if (GUIEngine::isFocusedForPlayer(m_options_widget, PLAYER_ID_GAME_MASTER))
        return;
    if (m_next_widget->isActivated())
        processInput();
}

// -----------------------------------------------------------------------------

void RegistrationDialog::onUpdate(float dt)
{
    if (m_phase == Terms)
    {
        if(m_sign_up_request  != NULL)
        {
            if(m_sign_up_request->isDone())
            {
                if(m_sign_up_request->isSuccess())
                {
                    m_show_registration_info = true;
                }
                else
                {
                    sfx_manager->quickSound( "anvil" );
                    m_show_registration_input = true;
                    m_registration_error = m_sign_up_request->getInfo();
                }
                delete m_sign_up_request;
                m_sign_up_request = NULL;
                //FIXME m_options_widget->setActivated();
            }
            else
            {
                m_info_widget->setDefaultColor();
                m_info_widget->setText(Messages::signingUp(), false);
            }
        }
    }
    // It's unsafe to delete from inside the event handler so we do it here
    if (m_self_destroy)
        ModalDialog::dismiss();
    else if (m_show_registration_input)
        showRegistrationInput();
    else if (m_show_registration_terms)
        showRegistrationTerms();
    else if (m_show_registration_info)
        showRegistrationInfo();
}
