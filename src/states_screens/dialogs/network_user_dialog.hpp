//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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


#ifndef HEADER_NETWORK_USER_DIALOG_HPP
#define HEADER_NETWORK_USER_DIALOG_HPP

#include "guiengine/modaldialog.hpp"
#include "utils/types.hpp"

#include <irrString.h>

namespace GUIEngine
{
    class IconButtonWidget;
    class LabelWidget;
    class RibbonWidget;
}

/**
 * \brief Dialog that handle user in network lobby
 * \ingroup states_screens
 */
class NetworkUserDialog : public GUIEngine::ModalDialog
{
private:
    const uint32_t m_host_id;

    const uint32_t m_online_id;

    const core::stringw m_name;

    bool m_self_destroy;

    GUIEngine::RibbonWidget * m_options_widget;

    GUIEngine::LabelWidget * m_name_widget;

    GUIEngine::IconButtonWidget * m_friend_widget;

    GUIEngine::IconButtonWidget * m_kick_widget;

    GUIEngine::IconButtonWidget * m_cancel_widget;

public:
    NetworkUserDialog(uint32_t host_id, uint32_t online_id,
                      const core::stringw& name)
        : ModalDialog(0.8f,0.8f), m_host_id(host_id), m_online_id(online_id),
          m_name(name), m_self_destroy(false)
    {
        loadFromFile("online/user_info_dialog.stkgui");
    }
    // ------------------------------------------------------------------------
    ~NetworkUserDialog() {}
    // ------------------------------------------------------------------------
    virtual void beforeAddingWidgets();
    // ------------------------------------------------------------------------
    void onEnterPressedInternal()                    { m_self_destroy = true; }
    // ------------------------------------------------------------------------
    GUIEngine::EventPropagation processEvent(const std::string& source);
    // ------------------------------------------------------------------------
    virtual bool onEscapePressed();
    // ------------------------------------------------------------------------
    virtual void onUpdate(float dt)
    {
        // It's unsafe to delete from inside the event handler so we do it here
        if (m_self_destroy)
        {
            ModalDialog::dismiss();
            return;
        }
    }
};

#endif
