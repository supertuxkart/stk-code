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


#ifndef HEADER_USER_INFO_DIALOG_HPP
#define HEADER_USER_INFO_DIALOG_HPP

#include <irrString.h>

#include "guiengine/modaldialog.hpp"
#include "guiengine/widgets.hpp"
#include "online/profile_manager.hpp"
#include "online/current_user.hpp"
#include "utils/types.hpp"


/**
 * \brief Dialog that allows a user to sign in
 * \ingroup states_screens
 */
class UserInfoDialog : public GUIEngine::ModalDialog
{

private:

    bool m_self_destroy;
    bool m_enter_profile;

    const uint32_t m_visiting_id;
    Online::Profile * m_profile;

    GUIEngine::LabelWidget * m_name_widget;
    GUIEngine::LabelWidget * m_info_widget;

    GUIEngine::RibbonWidget * m_options_widget;
    GUIEngine::IconButtonWidget * m_add_widget;
    GUIEngine::IconButtonWidget * m_enter_widget;
    GUIEngine::IconButtonWidget * m_cancel_widget;

    void requestJoin();

public:
    UserInfoDialog(uint32_t visiting_id);
    ~UserInfoDialog();

    void onEnterPressedInternal();
    GUIEngine::EventPropagation processEvent(const std::string& eventSource);

    virtual bool onEscapePressed();
    virtual void onUpdate(float dt);
};

#endif
