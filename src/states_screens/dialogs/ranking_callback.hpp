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
#include "io/xml_node.hpp"
#include "online/xml_request.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <memory>

class RankingCallback : public Online::XMLRequest
{
private:
    core::stringw m_name;

    core::stringw m_ranking_result;
    // ----------------------------------------------------------------
    /** Callback for the request to update rank of a player. Shows his
    *   rank and score.
    */
    virtual void callback()
    {
        // I18N: In the network player dialog, indiciating a network
        // player has no ranking
        m_ranking_result = _("%s has no ranking yet.", m_name);
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
                m_ranking_result =
                    _("%s is number %d in the rankings with a score of %f.",
                     m_name, rank, score);
            }
        }   // callback
    }
public:
    RankingCallback(const core::stringw& name, uint32_t online_id)
    : XMLRequest()
    {
        m_name = name;
    }
    const core::stringw& getRankingResult() const { return m_ranking_result; }
    static std::shared_ptr<RankingCallback> getRankingCallback(
        const core::stringw& name, uint32_t online_id)
    {
        auto rc = std::make_shared<RankingCallback>(name, online_id);
        PlayerManager::setUserDetails(rc, "get-ranking");
        rc->addParameter("id", online_id);
        return rc;
    }
};   // RankingCallback

#endif
