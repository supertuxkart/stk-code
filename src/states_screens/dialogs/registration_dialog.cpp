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
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"
#include "utils/string_utils.hpp"
#include "online/current_online_user.hpp"


using namespace GUIEngine;
using namespace irr;
using namespace irr::gui;

// -----------------------------------------------------------------------------

RegistrationDialog::RegistrationDialog(const float w, const float h, const Phase phase) :
        ModalDialog(w,h)
{
    m_self_destroy = false;
    m_username = "";
    m_email = "";
    m_email_confirm = "";
    m_password = "";
    m_password_confirm = "";
    m_agreement = false;
    //Left the agreement phase out as an option, defaults to the first phase.
    switch (phase)
    {
        case Info:
            showRegistrationInfo();
            break;
        case Activation:
            showRegistrationActivation();
            break;
        default:
            showRegistrationInfo();
            break;
    }
}

// -----------------------------------------------------------------------------

RegistrationDialog::~RegistrationDialog()
{
}

// -----------------------------------------------------------------------------

void RegistrationDialog::showRegistrationInfo(){
    clearWindow();
    m_phase = Info;
    loadFromFile("online/registration_info.stkgui");

    //Password should always be reentered if previous has been clicked, or an error occurred.
    m_password = "";
    m_password_confirm = "";

    TextBoxWidget* textBox = getWidget<TextBoxWidget>("username");
    assert(textBox != NULL);
    textBox->setText(m_username);
    textBox->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

    textBox = getWidget<TextBoxWidget>("password");
    assert(textBox != NULL);
    textBox->setPasswordBox(true,L'*');

    textBox = getWidget<TextBoxWidget>("password_confirm");
    assert(textBox != NULL);
    textBox->setPasswordBox(true,L'*');

    textBox = getWidget<TextBoxWidget>("email");
    assert(textBox != NULL);
    textBox->setText(m_email);

    textBox = getWidget<TextBoxWidget>("email_confirm");
    assert(textBox != NULL);
    textBox->setText(m_email_confirm);

    LabelWidget * label = getWidget<LabelWidget>("info");
    assert(label != NULL);
    label->setColor(irr::video::SColor(255, 255, 0, 0));
}

// -----------------------------------------------------------------------------

void RegistrationDialog::showRegistrationTerms(){
    clearWindow();
    m_phase = Terms;
    loadFromFile("online/registration_terms.stkgui");
    CheckBoxWidget * checkbox = getWidget<CheckBoxWidget>("accepted");
    assert(checkbox != NULL);
    checkbox->setState(false);
    ButtonWidget * submitButton = getWidget<ButtonWidget>("submit");
    assert(submitButton != NULL);
    submitButton->setDeactivated();
}

// -----------------------------------------------------------------------------

void RegistrationDialog::showRegistrationActivation(){
    clearWindow();
    m_phase = Activation;
    loadFromFile("online/registration_activation.stkgui");
}

// -----------------------------------------------------------------------------

bool RegistrationDialog::processInfoEvent(const std::string& eventSource){
    if (m_phase == Info)
    {
        if (eventSource == "next")
        {
            m_username = getWidget<TextBoxWidget>("username")->getText().trim();
            m_password = getWidget<TextBoxWidget>("password")->getText().trim();
            m_password_confirm =  getWidget<TextBoxWidget>("password_confirm")->getText().trim();
            m_email = getWidget<TextBoxWidget>("email")->getText().trim();
            m_email_confirm = getWidget<TextBoxWidget>("email_confirm")->getText().trim();

            //FIXME More validation of registration information (Though all validation should happen at the server too!)
            if (m_password != m_password_confirm)
            {
                getWidget<LabelWidget>("info")->setText(_("Passwords don't match!"), false);
                sfx_manager->quickSound( "anvil" );
            }
            else if (m_email != m_email_confirm)
            {
                getWidget<LabelWidget>("info")->setText(_("Emails don't match!"), false);
                sfx_manager->quickSound( "anvil" );
            }
            else if (m_username.size() < 5 || m_username.size() > 30)
            {
                getWidget<LabelWidget>("info")->setText(_("Username has to be between 5 and 30 characters long!"), false);
                sfx_manager->quickSound( "anvil" );
            }
            else if (m_password.size() < 5 || m_password.size() > 30)
            {
                getWidget<LabelWidget>("info")->setText(_("Password has to be between 5 and 30 characters long!"), false);
                sfx_manager->quickSound( "anvil" );
            }
            else if (m_email.size() < 5 || m_email.size() > 50)
            {
                getWidget<LabelWidget>("info")->setText(_("Email has to be between 5 and 50 characters long!"), false);
                sfx_manager->quickSound( "anvil" );
            }
            else
            {
                showRegistrationTerms();
            }
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------

bool RegistrationDialog::processTermsEvent(const std::string& eventSource){
    if (m_phase == Terms)
    {
        if (eventSource == "submit")
        {
            if (getWidget<CheckBoxWidget>("accepted")->getState())
            {
                m_agreement = true;
                showRegistrationActivation();
            }
            return true;
        }
        else if (eventSource == "accepted")
        {
            CheckBoxWidget * checkbox = getWidget<CheckBoxWidget>("accepted");
            bool new_state = !checkbox->getState();
            checkbox->setState(new_state);
            ButtonWidget * submitButton = getWidget<ButtonWidget>("submit");
            if(new_state)
                submitButton->setActivated();
            else
                submitButton->setDeactivated();
            return true;
        }
        else if (eventSource == "previous")
        {
            showRegistrationInfo();
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------

bool RegistrationDialog::processActivationEvent(const std::string& eventSource){
    if (m_phase == Activation)
    {
        if (eventSource == "activate")
        {
            //FIXME : activate
            m_self_destroy = true;
            return true;
        }
    }
    return false;
}


// -----------------------------------------------------------------------------

GUIEngine::EventPropagation RegistrationDialog::processEvent(const std::string& eventSource)
{
    if (eventSource == "cancel")
    {
        m_self_destroy = true;
        return GUIEngine::EVENT_BLOCK;
    }
    else if (processInfoEvent(eventSource) or processTermsEvent(eventSource) or processActivationEvent(eventSource))
    {
        return GUIEngine::EVENT_BLOCK;
    }
    return GUIEngine::EVENT_LET;
}

// -----------------------------------------------------------------------------

void RegistrationDialog::onEnterPressedInternal()
{
    // ---- Cancel button pressed
    const int playerID = PLAYER_ID_GAME_MASTER;
    ButtonWidget* cancelButton = getWidget<ButtonWidget>("cancel");
    if (GUIEngine::isFocusedForPlayer(cancelButton, playerID))
    {
        std::string fakeEvent = "cancel";
        processEvent(fakeEvent);
        return;
    }


}

// -----------------------------------------------------------------------------

void RegistrationDialog::onUpdate(float dt)
{
    // It's unsafe to delete from inside the event handler so we do it here
    if (m_self_destroy)
    {
        GUIEngine::getGUIEnv()->removeFocus( m_irrlicht_window );
        ModalDialog::dismiss();
    }
}
