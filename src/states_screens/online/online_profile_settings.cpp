//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Glenn De Jonghe
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

#include "states_screens/online/online_profile_settings.hpp"

#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/dialogs/change_password_dialog.hpp"
#include "states_screens/dialogs/general_text_field_dialog.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "utils/translation.hpp"
#include "online/xml_request.hpp"
#include "config/player_manager.hpp"
#include "audio/sfx_manager.hpp"

#include <IGUIButton.h>

#include <iostream>
#include <sstream>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;
using namespace Online;

// -----------------------------------------------------------------------------

OnlineProfileSettings::OnlineProfileSettings() : OnlineProfileBase("online/profile_settings.stkgui")
{
}   // OnlineProfileSettings

// -----------------------------------------------------------------------------

void OnlineProfileSettings::loadedFromFile()
{
    OnlineProfileBase::loadedFromFile();
    m_change_password_button = this->getWidget<ButtonWidget>("change_password_button");
    m_change_email_button = getWidget<ButtonWidget>("change_email_button");
    assert(m_change_password_button != NULL);
}   // loadedFromFile

// -----------------------------------------------------------------------------

void OnlineProfileSettings::init()
{
    OnlineProfileBase::init();
    m_profile_tabs->select( m_settings_tab->m_properties[PROP_ID], PLAYER_ID_GAME_MASTER );
    m_settings_tab->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
}   // init

// -----------------------------------------------------------------------------

void OnlineProfileSettings::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    OnlineProfileBase::eventCallback( widget, name, playerID);
    if (name == m_change_password_button->m_properties[GUIEngine::PROP_ID])
   {
       new ChangePasswordDialog();
   }
    if (name == m_change_email_button->m_properties[GUIEngine::PROP_ID])
    {
        new GeneralTextFieldDialog(_("Enter new E-mail below"),[](const irr::core::stringw& str){},[&](GUIEngine::LabelWidget* lw, GUIEngine::TextBoxWidget* tb)->bool
        {
            const irr::core::stringw new_email = tb->getText().trim();
            if (new_email.size() < 5 || new_email.size() > 254)
            {
                lw->setText(_("New Email has to be between 5 and 254 characters long!"), false);
                lw->setErrorColor();
                SFXManager::get()->quickSound("anvil");
                return false;
            }
            else if (  new_email.find(L"@")== -1 || new_email.find(L".")== -1 ||
                    (new_email.findLast(L'.') - new_email.findLast(L'@') <= 2 ) ||
                        new_email.findLast(L'@')==0 )
            {
                lw->setText(_("New Email is invalid!"), false);
                lw->setErrorColor();
                SFXManager::get()->quickSound("anvil");
                return false;
            }
            else
            {
                lw->setDefaultColor();
                changeEmail(new_email);
                return true;
            }
            
        });
    }
}   // eventCallback

// -----------------------------------------------------------------------------

void OnlineProfileSettings::changeEmail(const irr::core::stringw &new_email)
{
    class ChangeEmailRequest : public XMLRequest
    {
        virtual void callback()
        {
            if(isSuccess())
                new MessageDialog(_("E-mail changed!"));
            else
                new MessageDialog(_("Failed to change E-mail: %s", getInfo()));
        }   // callback
        public:
            ChangeEmailRequest() : XMLRequest() {}
    };  // ChangeEmailRequest
    auto request = std::make_shared<ChangeEmailRequest>();
    PlayerManager::setUserDetails(request, "change-email");
    request->addParameter("new-email", new_email);
    request->queue();
}   // changeEmail
