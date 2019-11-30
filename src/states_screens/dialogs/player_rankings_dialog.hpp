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


#ifndef HEADER_PLAYER_RANKINGS_DIALOG_HPP
#define HEADER_PLAYER_RANKINGS_DIALOG_HPP

#include "guiengine/modaldialog.hpp"
#include "utils/types.hpp"

#include <irrString.h>
#include <memory>
#include <tuple>
#include <vector>

namespace GUIEngine
{
    class IconButtonWidget;
    class LabelWidget;
    class ListWidget;
    class RibbonWidget;
}

class RankingCallback;
/**
 * \brief Dialog that handle user in network lobby
 * \ingroup states_screens
 */
class PlayerRankingsDialog : public GUIEngine::ModalDialog
{
private:
    const uint32_t m_online_id;

    const core::stringw m_name;

    bool m_self_destroy;

    std::shared_ptr<RankingCallback> m_ranking_callback;

    GUIEngine::RibbonWidget* m_options_widget;

    GUIEngine::LabelWidget* m_ranking_info;

    GUIEngine::ListWidget* m_top_ten;

    GUIEngine::IconButtonWidget* m_refresh_widget;

    GUIEngine::IconButtonWidget* m_ok_widget;

    static std::vector<std::tuple</*rank*/int, /*user name*/core::stringw,
        /*scores*/float> > m_rankings;

    // ------------------------------------------------------------------------
    void updateTopTenList();
    // ------------------------------------------------------------------------
    void fillTopTenList();

public:
    PlayerRankingsDialog(uint32_t online_id, const core::stringw& name);
    // ------------------------------------------------------------------------
    ~PlayerRankingsDialog() {}
    // ------------------------------------------------------------------------
    virtual void beforeAddingWidgets();
    // ------------------------------------------------------------------------
    void onEnterPressedInternal()                    { m_self_destroy = true; }
    // ------------------------------------------------------------------------
    GUIEngine::EventPropagation processEvent(const std::string& source);
    // ------------------------------------------------------------------------
    virtual bool onEscapePressed()
    {
        m_self_destroy = true;
        return false;
    }
    // ------------------------------------------------------------------------
    virtual void onUpdate(float dt);
};

#endif
