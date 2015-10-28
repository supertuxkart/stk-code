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


#ifndef HEADER_RECOVERY_DIALOG_HPP
#define HEADER_RECOVERY_DIALOG_HPP


#include "guiengine/modaldialog.hpp"
#include "guiengine/widgets.hpp"

namespace Online
{
    class XMLRequest;
}

/**
 * \brief Dialog that allows a user to recover his account
 * \ingroup states_screens
 */
class RecoveryDialog : public GUIEngine::ModalDialog
{
public:
    enum Phase
    {
        Input = 1,
        Info  = 2,
    };
    RecoveryDialog();
    virtual ~RecoveryDialog();

    void onEnterPressedInternal();
    GUIEngine::EventPropagation processEvent(const std::string& eventSource);
    
    virtual void onUpdate(float dt);
    virtual bool onEscapePressed();

private:
    Phase m_phase;
    bool m_self_destroy;
    bool m_show_recovery_input;
    bool m_show_recovery_info;

    Online::XMLRequest * m_recovery_request;

    GUIEngine::TextBoxWidget * m_username_widget;
    GUIEngine::TextBoxWidget * m_email_widget;

    GUIEngine::LabelWidget * m_info_widget;

    GUIEngine::RibbonWidget * m_options_widget;
    GUIEngine::IconButtonWidget * m_submit_widget;
    GUIEngine::IconButtonWidget * m_cancel_widget;

    void showRecoveryInput();
    void showRecoveryInfo();
    void processInput();
};

#endif
