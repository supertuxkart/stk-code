//  $Id: challenge_manager.hpp 1259 2007-09-24 12:28:19Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_UNLOCK_MANAGER_H
#define HEADER_UNLOCK_MANAGER_H

#include <map>

#include "challenges/challenge.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "lisp/writer.hpp"

class UnlockManager
{
private:
    typedef std::map<std::string, Challenge*> AllChallengesType;
    AllChallengesType             m_all_challenges;
    std::map<std::string, bool>   m_locked_features;
    std::vector<const Challenge*> m_unlocked_features;
    Challenge *getChallenge      (const std::string& id);
    void       computeActive     ();
public:
               UnlockManager    ();
    void       load             (const lisp::Lisp*);
    void       save             (lisp::Writer* writer);
    std::vector<const Challenge*> 
               getActiveChallenges();
    const std::vector<const Challenge*> 
               getUnlockedFeatures() {return m_unlocked_features;}

    void       clearUnlocked      () {m_unlocked_features.clear(); }
    void       raceFinished       ();
    void       grandPrixFinished  ();
    void       unlockFeature      (Challenge* c, bool save=true);
    void       lockFeature        (const std::string& feature);
    bool       isLocked           (const std::string& feature);
};   // UnlockManager

extern UnlockManager* unlock_manager;
#endif
