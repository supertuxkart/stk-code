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

#include "states_screens/dialogs/player_rankings_dialog.hpp"

#include "config/player_manager.hpp"
#include "guiengine/dialog_queue.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "online/online_profile.hpp"
#include "states_screens/dialogs/ranking_callback.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/translation.hpp"

#include <IGUIEnvironment.h>

using namespace GUIEngine;
using namespace irr;
using namespace irr::gui;
using namespace Online;

// ============================================================================
std::vector<std::tuple<int, core::stringw, float> >
    PlayerRankingsDialog::m_rankings;
// ----------------------------------------------------------------------------
PlayerRankingsDialog::PlayerRankingsDialog(uint32_t online_id,
                                           const core::stringw& name)
                    : ModalDialog(0.8f,0.9f), m_online_id(online_id),
                      m_name(name), m_self_destroy(false)
{
    loadFromFile("online/player_rankings_dialog.stkgui");
    m_top_ten = getWidget<ListWidget>("top-ten");
    assert(m_top_ten != NULL);

    if (m_rankings.empty())
        updateTopTenList();
    else
        fillTopTenList();
}   // PlayerRankingsDialog

// -----------------------------------------------------------------------------
void PlayerRankingsDialog::beforeAddingWidgets()
{
    m_options_widget = getWidget<RibbonWidget>("options");
    assert(m_options_widget != NULL);
    m_ok_widget = getWidget<IconButtonWidget>("ok");
    assert(m_ok_widget != NULL);
    m_refresh_widget = getWidget<IconButtonWidget>("refresh");
    assert(m_refresh_widget != NULL);
    m_options_widget->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

    m_ranking_info = getWidget<LabelWidget>("cur-rank");
    assert(m_ranking_info != NULL);
    m_ranking_callback =
        RankingCallback::getRankingCallback(m_name, m_online_id);
    m_ranking_callback->queue();
}   // beforeAddingWidgets

// ----------------------------------------------------------------------------
void PlayerRankingsDialog::updateTopTenList()
{
    // ------------------------------------------------------------------------
    class UpdateTopTenRequest : public XMLRequest
    {
        /** Callback for the request to update top 10 players and update the
         *  list.
        */
        virtual void callback()
        {
            if (isSuccess())
            {
                PlayerRankingsDialog* prd = dynamic_cast<PlayerRankingsDialog*>
                    (getCurrent());
                if (prd == NULL)
                    return;
                prd->m_rankings.clear();
                const XMLNode* players = getXMLData()->getNode("players");
                for (unsigned i = 0; i < players->getNumNodes(); i++)
                {
                    int rank;
                    core::stringw user;
                    float score;
                    players->getNode(i)->get("rank", &rank);
                    players->getNode(i)->get("username", &user);
                    players->getNode(i)->get("scores", &score);
                    prd->m_rankings.emplace_back(rank, user, score);
                }
                prd->fillTopTenList();
            }
        }   // callback
    public:
        UpdateTopTenRequest() : XMLRequest() {}
    };   // UpdateTopTenRequest
    // ------------------------------------------------------------------------

    auto request = std::make_shared<UpdateTopTenRequest>();
    PlayerManager::setUserDetails(request, "top-players");
    request->addParameter("ntop", 10);
    request->queue();
}   // updateTopTenList

// -----------------------------------------------------------------------------
void PlayerRankingsDialog::fillTopTenList()
{
    m_top_ten->clear();
    for (auto& r : m_rankings)
    {
        std::vector<GUIEngine::ListWidget::ListCell> row;
        row.push_back(GUIEngine::ListWidget::ListCell(
            StringUtils::toWString(std::get<0>(r)), -1, 1, true));
        row.push_back(GUIEngine::ListWidget::ListCell(std::get<1>(r), -1, 3,
            true));
        row.push_back(GUIEngine::ListWidget::ListCell(
            StringUtils::toWString(std::get<2>(r)), -1, 1, true));
        m_top_ten->addItem("rank", row);
    }
}   // fillTopTenList

// -----------------------------------------------------------------------------
void PlayerRankingsDialog::onUpdate(float dt)
{
    if (m_ranking_callback && !m_ranking_callback->isDone())
    {
        core::stringw msg = _("Fetching ranking info for %s", m_name);
        m_ranking_info->setText(StringUtils::loadingDots(msg.c_str()), false);
    }
    else if (m_ranking_callback && m_ranking_callback->isDone())
    {
        m_ranking_info->setText(m_ranking_callback->getRankingResult(), false);
        m_ranking_callback = nullptr;
    }

    // It's unsafe to delete from inside the event handler so we do it here
    if (m_self_destroy)
    {
        ModalDialog::dismiss();
        return;
    }
}   // onUpdate

// -----------------------------------------------------------------------------
GUIEngine::EventPropagation
    PlayerRankingsDialog::processEvent(const std::string& source)
{

    if (source == m_options_widget->m_properties[PROP_ID])
    {
        const std::string& selection =
            m_options_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (selection == m_ok_widget->m_properties[PROP_ID])
        {
            m_self_destroy = true;
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == m_refresh_widget->m_properties[PROP_ID])
        {
            static uint64_t timer = StkTime::getMonoTimeMs();
            // 1 minute per refresh
            if (StkTime::getMonoTimeMs() < timer + 60000)
                return GUIEngine::EVENT_BLOCK;

            timer = StkTime::getMonoTimeMs();
            m_ranking_callback =
                RankingCallback::getRankingCallback(m_name, m_online_id);
            m_ranking_callback->queue();
            updateTopTenList();
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}   // processEvent
