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


#ifndef HEADER_LOGIN_DIALOG_HPP
#define HEADER_LOGIN_DIALOG_HPP

#include <irrString.h>

#include "online/current_user.hpp"

#include "guiengine/modaldialog.hpp"
#include "guiengine/widgets.hpp"

/**
 * \brief Dialog that allows a user to sign in
 * \ingroup states_screens
 */
class LoginDialog : public GUIEngine::ModalDialog
{

private:
    
    bool m_self_destroy;
    bool m_open_registration_dialog;

    GUIEngine::LabelWidget * m_info_widget;
    GUIEngine::TextBoxWidget * m_username_widget;
    GUIEngine::TextBoxWidget * m_password_widget;
    GUIEngine::CheckBoxWidget * m_remember_widget;
    GUIEngine::LabelWidget * m_message_widget;

    GUIEngine::RibbonWidget * m_options_widget;
    GUIEngine::IconButtonWidget * m_sign_in_widget;
    GUIEngine::IconButtonWidget * m_recovery_widget;
    GUIEngine::IconButtonWidget * m_register_widget;
    GUIEngine::IconButtonWidget * m_as_guest_widget;
    GUIEngine::IconButtonWidget * m_cancel_widget;
    
    void login();

public:
    
    enum Message
    {
        Normal                  = 1,    // If the user presses the sign in button himself
        Signing_In_Required     = 2,    // If the user needs to be signed in
        Registration_Required   = 3     // If the user needs to be registered
    };

    /**
     * Creates a modal dialog with given percentage of screen width and height
     */
    LoginDialog(const Message);
    ~LoginDialog();

    void onEnterPressedInternal();
    GUIEngine::EventPropagation processEvent(const std::string& eventSource);
    
    virtual void onUpdate(float dt);
    //virtual void onTextUpdated();
};

#endif
