//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Glenn De Jonghe
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

#include "states_screens/dialogs/change_email_dialog.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "config/player_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "online/xml_request.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "utils/translation.hpp"
#include "audio/sfx_manager.hpp"

using namespace GUIEngine;
using namespace Online;

ChangeEmailDialog::ChangeEmailDialog() : ModalDialog(0.8f,0.7f),m_self_destroy(false)
{
    loadFromFile("online/change_email_dialog.stkgui");
    m_options_widget = getWidget<RibbonWidget>("options");
    m_info_widget = getWidget<LabelWidget>("info");
    m_current_email_widget = getWidget<TextBoxWidget>("current_email");
    m_new_email_widget = getWidget<TextBoxWidget>("new_email");
    
}   // ChangeEmailDialog
ChangeEmailDialog::~ChangeEmailDialog()
{

}   // ~ChangeEmailDialog
void ChangeEmailDialog::onEnterPressedInternal()
{

}   // onEnterPressedInternal
GUIEngine::EventPropagation ChangeEmailDialog::processEvent(const std::string& eventSource)
{
    if (eventSource == "options")
    {
        const std::string& selection = m_options_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (selection == "cancel")
        {
            m_self_destroy=true;
            return GUIEngine::EventPropagation::EVENT_BLOCK;
        }
        else if (selection == "submit")
        {
            submit();
            return GUIEngine::EventPropagation::EVENT_BLOCK;
        }
    }
    return GUIEngine::EventPropagation::EVENT_LET;
}   // processEvent
void ChangeEmailDialog::onUpdate(float dt)
{
    if(m_self_destroy)
    {
        ModalDialog::dismiss();
    }
}   // onUpdate
void ChangeEmailDialog::changeEmail(const irr::core::stringw &current_email,const irr::core::stringw &new_email)
{
    class ChangeEmailRequest : public XMLRequest
    {
        virtual void callback()
        {
            if (!GUIEngine::ModalDialog::isADialogActive()) return;
            ChangeEmailDialog * dialog =
                dynamic_cast<ChangeEmailDialog*>(GUIEngine::ModalDialog::getCurrent());
            if (dialog)
            {
                if(isSuccess())
                    dialog->success();
                else
                    dialog->error(getInfo());
            }
        }   // callback
        public:
            ChangeEmailRequest() : XMLRequest() {}
    };  // ChangeEmailRequest
    auto request = std::make_shared<ChangeEmailRequest>();
    PlayerManager::setUserDetails(request, "change-email");
    request->addParameter("current", current_email);
    request->addParameter("new", new_email);
    request->queue();
}   // changeEmail
void ChangeEmailDialog::success()
{
    m_info_widget->setDefaultColor();
    m_info_widget->setText(_("E-Mail successfully changed."), false);
    m_options_widget->setActive(true);
    m_current_email_widget->setText("");
    m_new_email_widget->setText("");
}   // success
void ChangeEmailDialog::error(const irr::core::stringw &info)
{
    SFXManager::get()->quickSound("anvil");
    m_info_widget->setErrorColor();
    m_info_widget->setText(info, false);
    m_options_widget->setActive(true);
    m_current_email_widget->setText("");
    m_new_email_widget->setText("");
}   // error
void ChangeEmailDialog::submit()
{
    const irr::core::stringw current_email = m_current_email_widget->getText().trim();
    const irr::core::stringw new_email = m_new_email_widget->getText().trim();
    if(current_email.size() < 5 || current_email.size() > 254)
    {
        m_info_widget->setText(_("Current Email is invalid!"), false);
        m_info_widget->setErrorColor();
        SFXManager::get()->quickSound("anvil");
    }
    else if (new_email.size() < 5 || new_email.size() > 254)
    {
        m_info_widget->setText(_("New Email has to be between 5 and 254 characters long!"), false);
        m_info_widget->setErrorColor();
        SFXManager::get()->quickSound("anvil");
    }
    else if (  new_email.find(L"@")== -1 || new_email.find(L".")== -1 ||
              (new_email.findLast(L'.') - new_email.findLast(L'@') <= 2 ) ||
                new_email.findLast(L'@')==0 )
    {
       m_info_widget->setText(_("New Email is invalid!"), false);
       m_info_widget->setErrorColor();
       SFXManager::get()->quickSound("anvil");
    }
    else
    {
        m_options_widget->setActive(false);
        m_info_widget->setDefaultColor();
        changeEmail(current_email, new_email);
    }
}   // submit