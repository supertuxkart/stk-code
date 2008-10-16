//  $Id: world.hpp 2326 2008-10-04 18:50:45Z auria $
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

#include "modes/world.hpp"
#include <string>

struct KartIconDisplayInfo;

struct BattleInfo
{
    int m_lives;
};

class ThreeStrikesBattle : public World
{
    KartIconDisplayInfo* m_kart_display_info;
    
    /** This vector contains an 'BattleInfo' struct for every kart in the race.
    */
    std::vector<BattleInfo> m_kart_info;
    
public:
    ThreeStrikesBattle();
    virtual ~ThreeStrikesBattle();
    
    // clock events
    virtual void onGo();
    virtual void terminateRace();
    
    // overriding World methods
    virtual void update(float delta);
    virtual void restartRace();
    //virtual void getDefaultCollectibles(int& collectible_type, int& amount);
    //virtual bool useRedHerring();
    virtual bool useFastMusicNearEnd() const { return false; }
    virtual KartIconDisplayInfo* getKartsDisplayInfo(const RaceGUI* caller);
    virtual bool raceHasLaps(){ return false; }
    virtual void moveKartAfterRescue(Kart* kart, btRigidBody* body);
    
    virtual std::string getInternalCode() const;
    
    virtual void kartHit(const int kart_id);
    
    /** Called by the race result GUI at the end of the race to know the final order
        (fill in the 'order' array) */
    virtual void raceResultOrder( int* order );
};


#endif