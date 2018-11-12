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


#ifndef HEADER_RANKING_CALLBACK_HPP
#define HEADER_RANKING_CALLBACK_HPP

#include "config/player_manager.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "online/xml_request.hpp"

#include <memory>

class RankingCallback
{
protected:
    // ------------------------------------------------------------------------
    void updatePlayerRanking(const core::stringw& name, uint32_t online_id,
                             GUIEngine::LabelWidget* info,
                             std::shared_ptr<bool> done)
    {
        // --------------------------------------------------------------------
        class UpdatePlayerRankingRequest : public Online::XMLRequest
        {
        private:
            std::weak_ptr<bool> m_done;

            core::stringw m_name;

            GUIEngine::LabelWidget* m_info;
            // ----------------------------------------------------------------
            /** Callback for the request to update rank of a player. Shows his
            *   rank and score.
            */
            virtual void callback()
            {
                auto done = m_done.lock();
                // Dialog deleted
                if (!done)
                    return;
                // I18N: In the network player dialog, indiciating a network
                // player has no ranking
                core::stringw result = _("%s has no ranking yet.", m_name);
                if (isSuccess())
                {
                    int rank = -1;
                    float score = 0.0f;
                    getXMLData()->get("rank", &rank);
                    getXMLData()->get("scores", &score);
                    if (rank > 0)
                    {
                        // I18N: In the network player dialog show rank and
                        // score of a player
                        result = _("%s is number %d in the rankings with a score of %f.",
                            m_name, rank, score);
                    }
                }
                *done = true;
                m_info->setText(result, false);

            }   // callback
        public:
            UpdatePlayerRankingRequest(const core::stringw& name,
                                       uint32_t online_id,
                                       GUIEngine::LabelWidget* info,
                                       std::shared_ptr<bool> done)
                : XMLRequest(true)
            {
                m_name = name;
                m_info = info;
                m_done = done;
            }
        };   // UpdatePlayerRankingRequest

        // --------------------------------------------------------------------
        UpdatePlayerRankingRequest* request =
            new UpdatePlayerRankingRequest(name, online_id, info, done);
        PlayerManager::setUserDetails(request, "get-ranking");
        request->addParameter("id", online_id);
        request->queue();
    }
};
#endif
