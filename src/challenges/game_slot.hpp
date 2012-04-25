//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 SuperTuxKart-Team
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

#ifndef GAME_SLOT_HPP
#define GAME_SLOT_HPP


#include <string>
#include <map>
#include <vector>
#include <irrString.h>

#include "race/race_manager.hpp"

class ChallengeData;
class Challenge;
class XMLWriter;

/**
 * \ingroup challenges
 * This class contains the progression through challenges for one game slot
 */

class GameSlot
{
    std::string m_kart_ident;
    
    /** Profile names can change, so rather than try to make sure all renames are done everywhere,
     *  assign a unique ID to each profiler. Will save much headaches.
     */
    std::string m_player_unique_id;
    
    /** Contains whether each feature of the challenge is locked or unlocked */
    std::map<std::string, bool>   m_locked_features;
    
    /** Recently unlocked features (they are waiting here
      * until they are shown to the user) */
    std::vector<const ChallengeData*> m_unlocked_features;
    
    std::map<std::string, Challenge*> m_challenges_state;
    
    friend class UnlockManager;
    
    void computeActive();
    
    int m_points;
    
    /** Set to false after the initial stuff (intro, select kart, etc.) */
    bool m_first_time;
    
public:
    
    // do NOT attempt to pass 'player_unique_id' by reference here. I don't know why (compiler bug
    // maybe?) but this screws up everything. Better pass by copy.
    GameSlot(std::string player_unique_id)
    {
        m_player_unique_id = player_unique_id;
        m_points = 0;
        m_first_time = true;
    }
    
    const std::string& getPlayerID() const { return m_player_unique_id; }
    const std::string& getKartIdent () const { return m_kart_ident;  }
    void setKartIdent(const std::string& kart_ident) { m_kart_ident = kart_ident; }
    
    
    /** Returns the list of recently unlocked features (e.g. call at the end of a
     race to know if any features were unlocked) */
    const std::vector<const ChallengeData*> 
        getRecentlyCompletedChallenges() {return m_unlocked_features;}
    
    /** Clear the list of recently unlocked challenges */
    void       clearUnlocked     () {m_unlocked_features.clear(); }
    
    bool       isLocked          (const std::string& feature);
    
    void       lockFeature       (Challenge *challenge);
    
    void       unlockFeature     (Challenge* c, RaceManager::Difficulty d, bool do_save=true);
        
    void       raceFinished      ();
    void       grandPrixFinished ();
    
    void       save              (std::ofstream& file);
    
    int        getPoints          () const { return m_points; }
    
    void       setFirstTime(bool ft) { m_first_time = ft;   }
    bool       isFirstTime() const   { return m_first_time; }
    
    const Challenge* getChallenge(const std::string& id) { return m_challenges_state[id]; }
};

#endif
