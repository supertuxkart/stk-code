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

#ifndef FREE_FOR_ALL_HPP
#define FREE_FOR_ALL_HPP

#include "modes/world_with_rank.hpp"
#include "states_screens/race_gui_base.hpp"

#include <vector>

class NetworkString;

class FreeForAll : public WorldWithRank
{
protected:
    bool m_count_down_reached_zero;

    std::vector<int> m_scores;
    // ------------------------------------------------------------------------
    void handleScoreInServer(int kart_id, int hitter);

private:
    // ------------------------------------------------------------------------
    virtual video::SColor getColor(unsigned int kart_id) const;

public:
    // ------------------------------------------------------------------------
    FreeForAll();
    // ------------------------------------------------------------------------
    virtual ~FreeForAll();
    // ------------------------------------------------------------------------
    virtual void init() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool isRaceOver() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void reset(bool restart=false) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void getKartsDisplayInfo(
                 std::vector<RaceGUIBase::KartIconDisplayInfo> *info) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool raceHasLaps() OVERRIDE                       { return false; }
    // ------------------------------------------------------------------------
    virtual const std::string& getIdent() const OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool kartHit(int kart_id, int hitter = -1) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void update(int ticks) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void countdownReachedZero() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void terminateRace() OVERRIDE;
    // ------------------------------------------------------------------------
    void setKartScoreFromServer(NetworkString& ns);
    // ------------------------------------------------------------------------
    int getKartScore(int kart_id) const        { return m_scores.at(kart_id); }
    // ------------------------------------------------------------------------
    bool getKartFFAResult(int kart_id) const;
    // ------------------------------------------------------------------------
    virtual std::pair<uint32_t, uint32_t> getGameStartedProgress() const
        OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void addReservedKart(int kart_id) OVERRIDE
    {
        WorldWithRank::addReservedKart(kart_id);
        m_scores.at(kart_id) = 0;
    }
    // ------------------------------------------------------------------------
    virtual void saveCompleteState(BareNetworkString* bns,
                                   STKPeer* peer) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void restoreCompleteState(const BareNetworkString& b) OVERRIDE;
};   // FreeForAll


#endif
