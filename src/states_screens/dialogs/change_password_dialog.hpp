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


#ifndef HEADER_CHANGE_PASSWORD_DIALOG_HPP
#define HEADER_CHANGE_PASSWORD_DIALOG_HPP

#include "guiengine/modaldialog.hpp"

#include <irrString.h>

namespace GUIEngine
{
    class IconButtonWidget;
    class LabelWidget;
    class RibbonWidget;
}

/**
 * \brief Dialog that allows a user to sign in
 * \ingroup states_screens
 */
class ChangePasswordDialog : public GUIEngine::ModalDialog
{

public:

    ChangePasswordDialog();
    ~ChangePasswordDialog();

    virtual void onEnterPressedInternal();

    GUIEngine::EventPropagation processEvent(const std::string& eventSource);

    virtual bool onEscapePressed();
    virtual void onUpdate(float dt);
    void success();
    void error(const irr::core::stringw & error_message);
    void changePassword(const irr::core::stringw &current_password,
                        const irr::core::stringw &new_password);

private:

    bool m_self_destroy;
    bool m_success;

    GUIEngine::TextBoxWidget * m_current_password_widget;
    GUIEngine::TextBoxWidget * m_new_password1_widget;
    GUIEngine::TextBoxWidget * m_new_password2_widget;
    GUIEngine::LabelWidget * m_info_widget;

    GUIEngine::RibbonWidget * m_options_widget;
    GUIEngine::IconButtonWidget * m_submit_widget;
    GUIEngine::IconButtonWidget * m_cancel_widget;
    
    void submit();
};

#endif
