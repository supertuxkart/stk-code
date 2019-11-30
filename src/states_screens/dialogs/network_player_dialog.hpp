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

#include <string>
#include <irrString.h>
#include <memory>

namespace GUIEngine
{
    class IconButtonWidget;
    class LabelWidget;
    class RibbonWidget;
}

class RankingCallback;
enum HandicapLevel : uint8_t;

/**
 * \brief Dialog that handle player in network lobby
 * \ingroup states_screens
 */
class NetworkPlayerDialog : public GUIEngine::ModalDialog
{
private:
    const uint32_t m_host_id;

    const uint32_t m_online_id;

    const uint8_t m_local_id;

    HandicapLevel m_handicap;

    const core::stringw m_name;

    const std::string m_country_code;

    const bool m_allow_change_team;

    bool m_self_destroy, m_open_report_textbox;

    std::shared_ptr<RankingCallback> m_ranking_callback;

    GUIEngine::RibbonWidget* m_options_widget;

    GUIEngine::LabelWidget* m_desc_widget;

    GUIEngine::LabelWidget* m_info_widget;

    GUIEngine::IconButtonWidget* m_friend_widget;

    GUIEngine::IconButtonWidget* m_kick_widget;

    GUIEngine::IconButtonWidget* m_change_team_widget;

    GUIEngine::IconButtonWidget* m_cancel_widget;

    GUIEngine::IconButtonWidget* m_handicap_widget;

    GUIEngine::IconButtonWidget* m_report_widget;
public:
    NetworkPlayerDialog(uint32_t host_id, uint32_t online_id, uint8_t local_id,
                        const core::stringw& name,
                        const std::string& country_code,
                        bool allow_change_team, HandicapLevel h)
        : ModalDialog(0.8f,0.8f), m_host_id(host_id), m_online_id(online_id),
          m_local_id(local_id), m_handicap(h),
          m_name(name), m_country_code(country_code),
          m_allow_change_team(allow_change_team), m_self_destroy(false),
          m_open_report_textbox(false)
    {
        loadFromFile("online/user_info_dialog.stkgui");
    }
    // ------------------------------------------------------------------------
    ~NetworkPlayerDialog() {}
    // ------------------------------------------------------------------------
    virtual void beforeAddingWidgets();
    // ------------------------------------------------------------------------
    void onEnterPressedInternal()                    { m_self_destroy = true; }
    // ------------------------------------------------------------------------
    GUIEngine::EventPropagation processEvent(const std::string& source);
    // ------------------------------------------------------------------------
    virtual bool onEscapePressed();
    // ------------------------------------------------------------------------
    virtual void onUpdate(float dt);
    // ------------------------------------------------------------------------
    virtual void init();

};

#endif
