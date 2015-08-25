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


#ifndef HEADER_USER_INFO_DIALOG_HPP
#define HEADER_USER_INFO_DIALOG_HPP

#include "guiengine/modaldialog.hpp"
#include "guiengine/widgets.hpp"
#include "utils/types.hpp"

#include <irrString.h>

namespace Online
{
    class OnlineProfile;
}

/**
 * \brief Dialog that allows a user to sign in
 * \ingroup states_screens
 */
class UserInfoDialog : public GUIEngine::ModalDialog
{

private:

    bool m_self_destroy;
    bool m_enter_profile;
    bool m_processing;

    bool m_error;
    irr::core::stringw m_info;

    const uint32_t m_showing_id;
    Online::OnlineProfile * m_online_profile;

    GUIEngine::LabelWidget * m_name_widget;
    GUIEngine::LabelWidget * m_info_widget;

    GUIEngine::RibbonWidget * m_options_widget;
    GUIEngine::IconButtonWidget * m_remove_widget;
    GUIEngine::IconButtonWidget * m_friend_widget;
    GUIEngine::IconButtonWidget * m_accept_widget;
    GUIEngine::IconButtonWidget * m_decline_widget;
    GUIEngine::IconButtonWidget * m_enter_widget;
    GUIEngine::IconButtonWidget * m_cancel_widget;

    void requestJoin();
    void activate();
    void deactivate();
    void sendFriendRequest();
    void acceptFriendRequest();
    void declineFriendRequest();
    void removeExistingFriend();
    void removePendingFriend();

public:
    UserInfoDialog(uint32_t showing_id, const core::stringw info = "", bool error = false, bool from_queue = false);
    ~UserInfoDialog();

    virtual void beforeAddingWidgets();
    virtual void load();

    void onEnterPressedInternal();
    GUIEngine::EventPropagation processEvent(const std::string& eventSource);

    virtual bool onEscapePressed();
    virtual void onUpdate(float dt);
};

#endif
