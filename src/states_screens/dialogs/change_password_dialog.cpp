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

#include "states_screens/dialogs/change_password_dialog.hpp"

#include "audio/sfx_manager.hpp"
#include "guiengine/engine.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"
#include "utils/string_utils.hpp"
#include "online/messages.hpp"

#include <IGUIEnvironment.h>
#include <irrString.h>

using namespace GUIEngine;
using namespace irr;
using namespace irr::core;
using namespace irr::gui;
using namespace Online;

// -----------------------------------------------------------------------------

ChangePasswordDialog::ChangePasswordDialog() :
        ModalDialog(0.8f,0.7f)
{
    m_self_destroy = false;
    m_success = false;

    loadFromFile("online/change_password.stkgui");

    m_current_password_widget = getWidget<TextBoxWidget>("current_password");
    assert(m_current_password_widget != NULL);
    m_current_password_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    m_current_password_widget->setPasswordBox(true,L'*');

    m_new_password1_widget = getWidget<TextBoxWidget>("new_password1");
    assert(m_new_password1_widget != NULL);
    m_new_password1_widget->setPasswordBox(true,L'*');

    m_new_password2_widget = getWidget<TextBoxWidget>("new_password2");
    assert(m_new_password2_widget != NULL);
    m_new_password2_widget->setPasswordBox(true,L'*');

    m_info_widget = getWidget<LabelWidget>("info");
    assert(m_info_widget != NULL);

    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget != NULL);
    m_submit_widget = getWidget<IconButtonWidget>("submit");
    assert(m_submit_widget != NULL);
    m_cancel_widget = getWidget<IconButtonWidget>("cancel");
    assert(m_cancel_widget != NULL);
}

// -----------------------------------------------------------------------------

ChangePasswordDialog::~ChangePasswordDialog()
{
}

// -----------------------------------------------------------------------------
void ChangePasswordDialog::submit()
{
    const stringw current_password = m_current_password_widget->getText().trim();
    const stringw new_password1 = m_new_password1_widget->getText().trim();
    const stringw new_password2 = m_new_password2_widget->getText().trim();
    if (current_password.size() < 8 || current_password.size() > 30)
    {
        sfx_manager->quickSound("anvil");
        m_info_widget->setErrorColor();
        m_info_widget->setText(_("Current password invalid."), false);
    }
    else if (new_password1.size() < 8 || new_password1.size() > 30)
    {
        sfx_manager->quickSound("anvil");
        m_info_widget->setErrorColor();
        m_info_widget->setText(_("Password has to be between 8 and 30 characters long!"), false);
    }
    else if (new_password1 != new_password2)
    {
        sfx_manager->quickSound("anvil");
        m_info_widget->setErrorColor();
        m_info_widget->setText(_("Passwords don't match!"), false);
    }
    else
    {
        m_options_widget->setDeactivated();
        m_info_widget->setDefaultColor();
        Online::CurrentUser::get()->requestPasswordChange(current_password, new_password1, new_password2);
    }
}

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation ChangePasswordDialog::processEvent(const std::string& eventSource)
{

    if (eventSource == m_options_widget->m_properties[PROP_ID])
    {
        const std::string& selection = m_options_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (selection == m_cancel_widget->m_properties[PROP_ID])
        {
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if(selection == m_submit_widget->m_properties[PROP_ID])
        {
            submit();
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}

// -----------------------------------------------------------------------------

void ChangePasswordDialog::onEnterPressedInternal()
{
    const int playerID = PLAYER_ID_GAME_MASTER;
    if (GUIEngine::isFocusedForPlayer(m_options_widget, playerID))
        return;
    if (m_submit_widget->isActivated())
        submit();
}

// -----------------------------------------------------------------------------

bool ChangePasswordDialog::onEscapePressed()
{
    if (m_cancel_widget->isActivated())
        m_self_destroy = true;
    return false;
}

// -----------------------------------------------------------------------------
void ChangePasswordDialog::success()
{
    m_info_widget->setDefaultColor();
    m_info_widget->setText(_("Password successfully changed."), false);
    m_options_widget->setActivated();
    m_current_password_widget->setText("");
    m_new_password1_widget->setText("");
    m_new_password2_widget->setText("");
}

// -----------------------------------------------------------------------------
void ChangePasswordDialog::error(const irr::core::stringw & error)
{
    sfx_manager->quickSound("anvil");
    m_info_widget->setErrorColor();
    m_info_widget->setText(error, false);
    m_options_widget->setActivated();
    m_current_password_widget->setText("");
    m_new_password1_widget->setText("");
    m_new_password2_widget->setText("");
}

// -----------------------------------------------------------------------------

void ChangePasswordDialog::onUpdate(float dt)
{
    if(!m_options_widget->isActivated())
        m_info_widget->setText(Online::Messages::validatingInfo(), false);

    // It's unsafe to delete from inside the event handler so we do it here
    if (m_self_destroy)
    {
        ModalDialog::dismiss();
    }
}
