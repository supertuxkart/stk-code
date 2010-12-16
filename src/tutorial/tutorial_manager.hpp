//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Alejandro Santiago
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

#ifndef TUTORIALMANAGER_H
#define TUTORIALMANAGER_H

#include <map>

#include "tutorial/tutorial.hpp"
#include "utils/no_copy.hpp"

#include <fstream>

class XMLNode;
class SFXBase;

using namespace std;
/**
  * \brief main class to handle the tutorials list
  * \ingroup tutorial
  */
class TutorialManager : public NoCopy
{
private:
    SFXBase    *m_locked_sound;
    typedef map<string, Tutorial*> AllChallengesType;
    AllChallengesType             m_all_tutorial;
    map<string, bool>   m_locked_tutorials;
    vector<const Tutorial*> m_unlocked_tutorials;
    void       computeActive     ();
    void       load              ();

    void       unlockFeature     (Tutorial* t, bool do_save=true);

public:
               TutorialManager     ();
              ~TutorialManager     ();
    void       addTutorial      (Tutorial *t);
    void       addTutorial      (const string& filename);
    void       save              ();
    vector<const Tutorial*> getActiveTutorials();

    /** Returns the list of recently unlocked features (e.g. call at the end of a
        race to know if any features were unlocked) */
    const vector<const Tutorial*> getRecentlyUnlockedFeatures() {return m_unlocked_tutorials;}

    /** Clear the list of recently unlocked challenges */
    void       clearUnlocked     () {m_unlocked_tutorials.clear(); }

    /** Returns a complete list of all solved challenges */
    const vector<const Tutorial*>   getUnlockedTutorials();

    /** Returns the list of currently inaccessible (locked) challenges */
    const vector<const Tutorial*>   getLockedTutorials();

    const Tutorial *getTutorial     (const string& id);

    void       raceFinished      ();
    void       grandPrixFinished ();
    void       lockFeature       (Tutorial* tutorial);
    bool       isLocked          (const string& feature);

    /** Eye- (or rather ear-) candy. Play a sound when user tries to access a locked area */
    void       playLockSound() const;

};   // UnlockManager

//extern UnlockManager* unlock_manager;
#endif // TUTORIALMANAGER_H
