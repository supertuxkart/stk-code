//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2015 Joerg Henrichs
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

#ifndef HEADER_CHALLENGE_STATUS_HPP
#define HEADER_CHALLENGE_STATUS_HPP

/**
  * \defgroup challenges
  * This module handles the challenge system, which locks features (tracks, karts
  * modes, etc.) until the user completes some task.
  */

#include <string>
#include <vector>
#include <fstream>
#include <irrString.h>

#include "race/race_manager.hpp"
#include "utils/no_copy.hpp"
#include "utils/translation.hpp"

class ChallengeData;
class UTFWriter;
class XMLNode;

/**
  * \brief The state of a challenge for one player.
  *  Each ChallengeStatus has one ChallengeData associcated, which stores
  *  the actual data about the challenge. The ChallengeStatus stores if the
  *  challenge is not possible yet (inactive), active (i.e. user can try to
  *  solve it), or solved. This status is stored for each difficulty level.
  *  This data is saved to and loaded from the players.xml file.
  *  A StoryModeStatus instance will store an array of ChallengeStatuses,
  *  one for each Challenge in STK.
  *
  * \ingroup challenges
  */
class ChallengeStatus : public NoCopy
{
private:
    /** The different states the challenge can be in. */
    enum {CH_INACTIVE,                 // challenge not yet possible
          CH_ACTIVE,                   // challenge possible, but not yet solved
          CH_SOLVED}                   // challenge was solved
    m_state[RaceManager::DIFFICULTY_COUNT];

    /** Pointer to the original challenge data. */
    const ChallengeData* m_data;

public:
    ChallengeStatus(const ChallengeData* data)
    {
        m_data = data;
        m_state[RaceManager::DIFFICULTY_EASY]   = CH_INACTIVE;
        m_state[RaceManager::DIFFICULTY_MEDIUM] = CH_INACTIVE;
        m_state[RaceManager::DIFFICULTY_HARD]   = CH_INACTIVE;
    }
    virtual ~ChallengeStatus() {};
    void load(const XMLNode* config);
    void save(UTFWriter& writer);
    void setSolved(RaceManager::Difficulty d);

    // ------------------------------------------------------------------------
    /** Returns if this challenge was solved at the specified difficulty.
     */
    bool isSolved(RaceManager::Difficulty d) const
    {
        return m_state[d]==CH_SOLVED;
    }   // isSolved
    // ------------------------------------------------------------------------
    /** Returns true if this challenge was solved at any difficult.
     */
    bool isSolvedAtAnyDifficulty() const
    {
        return m_state[0]==CH_SOLVED || m_state[1]==CH_SOLVED ||
                m_state[2]==CH_SOLVED;
    }   // isSolvedAtAnyDifficulty
    // ------------------------------------------------------------------------
    /** True if this challenge is active at the given difficulty.
     */
    bool isActive(RaceManager::Difficulty d) const
    {
        return m_state[d]==CH_ACTIVE;
    }   // isActive
    // ------------------------------------------------------------------------
    /** Sets this challenge to be active.
     */
    void setActive(RaceManager::Difficulty d)
    {
        m_state[d] = CH_ACTIVE;
    }   // setActive
    // ------------------------------------------------------------------------
    /** Returns a pointer to the actual Challenge data.
     */
    const ChallengeData* getData() const { return m_data; }
};   // ChallengeStatus
#endif
