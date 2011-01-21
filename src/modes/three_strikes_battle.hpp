//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 SuperTuxKart-Team
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

#ifndef THREE_STRIKES_HPP
#define THREE_STRIKES_HPP

#include <string>

#include "modes/world_with_rank.hpp"
#include "states_screens/race_gui_base.hpp"

/**
 * \brief An implementation of World, to provide the 3 strikes battle game mode
 * \ingroup modes
 */
class ThreeStrikesBattle : public WorldWithRank
{
private:
    struct BattleInfo
    {
        int m_lives;
    };

    RaceGUIBase::KartIconDisplayInfo* m_kart_display_info;
    
    /** This vector contains an 'BattleInfo' struct for every kart in the race.
    */
    std::vector<BattleInfo> m_kart_info;
    
public:
    
    /** Used to show a nice graph when battle is over */
    struct BattleEvent
    {
        float m_time;
        std::vector<BattleInfo> m_kart_info;
    };
    std::vector<BattleEvent> m_battle_events;
    
    ThreeStrikesBattle();
    virtual ~ThreeStrikesBattle();
    
    virtual void init();
    
    // clock events
    virtual bool isRaceOver();
    virtual void terminateRace();
    
    // overriding World methods
    virtual void restartRace();

    //virtual void getDefaultCollectibles(int& collectible_type, int& amount);
    virtual bool useFastMusicNearEnd() const { return false; }
    virtual RaceGUIBase::KartIconDisplayInfo* getKartsDisplayInfo();
    virtual bool raceHasLaps(){ return false; }
    virtual void moveKartAfterRescue(Kart* kart);
    
    virtual std::string getIdent() const;
    
    virtual void kartHit(const int kart_id);
    virtual void update(float dt);            
    void updateKartRanks();
};   // ThreeStrikesBattles


#endif
