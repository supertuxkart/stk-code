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

class ChallengeData;
class Challenge;
class XMLWriter;

/**
 * \ingroup challenges
 * This class contains the progression through challenges for one game slot
 */

class GameSlot
{
    irr::core::stringw m_player_name;
    std::string m_kart_ident;
    
    /** Contains whether each feature of the challenge is locked or unlocked */
    std::map<std::string, bool>   m_locked_features;
    
    /** Recently unlocked features (they are waiting here
      * until they are shown to the user) */
    std::vector<const ChallengeData*> m_unlocked_features;
    
    std::map<std::string, Challenge*> m_challenges_state;
    
    friend class UnlockManager;
    
    void computeActive();
    
    int m_points;
    
public:
    
    GameSlot(const irr::core::stringw& player_name)
    {
        m_player_name = player_name;
        m_points = 0;
    }
    
    const irr::core::stringw& getPlayerName() const { return m_player_name; }
    const std::string& getKartIdent () const { return m_kart_ident;  }
    void setKartIdent(const std::string& kart_ident) { m_kart_ident = kart_ident; }
    
    
    /** Returns the list of recently unlocked features (e.g. call at the end of a
     race to know if any features were unlocked) */
    const std::vector<const ChallengeData*> 
        getRecentlyUnlockedFeatures() {return m_unlocked_features;}
    
    /** Clear the list of recently unlocked challenges */
    void       clearUnlocked     () {m_unlocked_features.clear(); }
    
    
    /** Returns a complete list of all solved challenges */
    const std::vector<const ChallengeData*>   getUnlockedFeatures();
    
    /** Returns the list of currently inaccessible (locked) challenges */
    const std::vector<const ChallengeData*>   getLockedChallenges();
    
    bool       isLocked          (const std::string& feature);
    
    void       lockFeature       (Challenge *challenge);
    
    void       unlockFeature     (Challenge* c, bool do_save=true);
    
    std::vector<const ChallengeData*> getActiveChallenges();
    
    void       raceFinished      ();
    void       grandPrixFinished ();
    
    void       save              (XMLWriter& file);
    
    int        getPoints          () const { return m_points; }
};

#endif
