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

#ifndef HEADER_CHALLENGE_DATA_HPP
#define HEADER_CHALLENGE_DATA_HPP

#include <string>
#include <vector>
#include <stdio.h>
#include <stdexcept>


#include "race/race_manager.hpp"

/**
  * \brief the description of one challenge
  * \ingroup challenges
  */
class ChallengeData
{
public:
    /** The type of reward you get when fulfilling this challenge.
    */
    enum RewardType
    {
        UNLOCK_TRACK,
        UNLOCK_GP,
        UNLOCK_MODE,
        UNLOCK_KART,
        UNLOCK_DIFFICULTY
    };
    // ------------------------------------------------------------------------
    class UnlockableFeature
    {
    public:

        std::string        m_name; // internal name
        irr::core::stringw m_user_name; // not all types of feature have one
        RewardType         m_type;

        const irr::core::stringw getUnlockedMessage() const;
    };   // UnlockableFeature
    // ------------------------------------------------------------------------

    /** The various types of challenges that we support, which esp. determine
     *  when a challenge is tested if it is fulfilled. For now we have GP
     *  (a GP challenge, tested at the end of a GP), Race (tested at the
     *  end of a challenge race), and 'any', which is checked at the end of
     *  each race (maybe even non challenged once). An example for 'any' is
     *  'are all challenges unclocked on highest level', which needs to be
     *  tested after each race (but is itself not a race mode, so it's not
     *  that you can start this challenge, which differentiates it from
     *  a race challenge). */
    enum ChallengeModeType
    {
        CM_GRAND_PRIX,
        CM_SINGLE_RACE,
        CM_ANY
    };


private:

    /** The challenge mode of this challenge. */
    ChallengeModeType              m_mode;

    /** The minor type, used when m_mode is CM_GP or CM_RACE only. */
    RaceManager::MinorRaceModeType m_minor;

    int                            m_num_laps;
    int                            m_position[RaceManager::DIFFICULTY_COUNT];
    int                            m_num_karts[RaceManager::DIFFICULTY_COUNT];
    std::string                    m_ai_kart_ident[RaceManager::DIFFICULTY_COUNT];
    std::string                    m_replay_files[RaceManager::DIFFICULTY_COUNT];
    float                          m_time[RaceManager::DIFFICULTY_COUNT];
    int                            m_energy[RaceManager::DIFFICULTY_COUNT];
    RaceManager::AISuperPower      m_ai_superpower[RaceManager::DIFFICULTY_COUNT];
    std::string                    m_gp_id;
    std::string                    m_track_id;
    std::string                    m_filename;
    /** Version number of the challenge. */
    int                            m_version;
    bool                           m_is_ghost_replay;

    void setUnlocks(const std::string &id,
                    ChallengeData::RewardType reward);
    void error(const char *id) const;

    /** Short, internal name for this challenge. */
    std::string              m_id;

    /** Features to unlock. */
    std::vector<UnlockableFeature> m_feature;

    /** Number of trophies required to access this challenge */
    int m_num_trophies;

public:
                 ChallengeData(const std::string& filename);

    virtual      ~ChallengeData() {}

    /** sets the right parameters in RaceManager to try this challenge */
    void         setRace(RaceManager::Difficulty d) const;

    virtual void check() const;
    virtual bool isChallengeFulfilled() const;
    virtual bool isGPFulfilled() const;
    void  addUnlockTrackReward(const std::string &track_name);
    void  addUnlockModeReward(const std::string &internal_mode_name,
                              const irr::core::stringw &user_mode_name);
    void  addUnlockGPReward(const std::string &gp_name);
    void  addUnlockDifficultyReward(const std::string &internal_name,
                                    const irr::core::stringw &user_name);
    void  addUnlockKartReward(const std::string &internal_name,
                              const irr::core::stringw &user_name);

    // ------------------------------------------------------------------------
    /** Returns the version number of this challenge. */
    int  getVersion() const { return m_version; }

    // ------------------------------------------------------------------------
    /** Returns the list of unlockable features for this challenge.
     */
    const std::vector<UnlockableFeature>&
        getFeatures() const { return m_feature; }

    // ------------------------------------------------------------------------
    /** Returns the id of the challenge. */
    const std::string &getId() const { return m_id; }

    // ------------------------------------------------------------------------
    /** Sets the id of this challenge. */
    void  setId(const std::string& s) { m_id = s; }

    // ------------------------------------------------------------------------
    /** Returns the track associated with this challenge. */
    const std::string& getTrackId() const
    {
        assert(m_mode==CM_SINGLE_RACE);
        return m_track_id;
    }   // getTrackId

    // ------------------------------------------------------------------------
    /** Returns the id of the grand prix associated with this challenge. */
    const std::string& getGPId() const
    {
        assert(m_mode==CM_GRAND_PRIX);
        return m_gp_id;
    }   // getGPId

    // ------------------------------------------------------------------------
    /** Return number of laps. */
    int getNumLaps() const
    {
        assert(m_mode==CM_SINGLE_RACE);
        return m_num_laps;
    }   // getNumLaps

    // ------------------------------------------------------------------------
    /** Get number of required trophies to start this challenge */
    int getNumTrophies() const { return m_num_trophies; }
    // ------------------------------------------------------------------------
    /** Returns if this challenge is a grand prix. */
    bool isGrandPrix() const { return m_mode == CM_GRAND_PRIX; }
    // ------------------------------------------------------------------------
    /** Returns if this challenge is a grand prix. */
    bool isSingleRace() const { return m_mode == CM_SINGLE_RACE; }
    // ------------------------------------------------------------------------
    /** Returns if this challenge is using ghost replay. */
    bool isGhostReplay() const { return m_is_ghost_replay; }
    // ------------------------------------------------------------------------
    /** Returns the challenge mode of this challenge. */
    ChallengeModeType getMode() const { return m_mode; }
    // ------------------------------------------------------------------------
    /** Returns the minor mode of this challenge. */
    RaceManager::MinorRaceModeType getMinorMode()  const { return m_minor; }
    // ------------------------------------------------------------------------
    /** Returns the description of this challenge.
     */
    const irr::core::stringw getChallengeDescription() const;

    // ------------------------------------------------------------------------
    /** Returns the minimum position the player must have in order to win.
     */
    int getPosition(RaceManager::Difficulty difficulty) const
    {
        return m_position[difficulty];
    }   // getPosition

    // ------------------------------------------------------------------------
    /** Returns the number of karts to use.
     */
    int getNumKarts(RaceManager::Difficulty difficulty) const
    {
        return m_num_karts[difficulty];
    }   // getNumKarts
    // ------------------------------------------------------------------------
    /** Returns the maximum time in which the kart must finish.
     */
    float getTime(RaceManager::Difficulty difficulty) const
    {
        return m_time[difficulty];
    }   // getTime
    // ------------------------------------------------------------------------
    /** Return the energy that a kart must at least have at the end of a race.
     */
    int getEnergy(RaceManager::Difficulty difficulty) const
    {
        return m_energy[difficulty];
    }   // getEnergy
    // ------------------------------------------------------------------------
    /** Returns the name of the AI to use (used for boss challenge).
     */
    const std::string& getAIKartIdent(RaceManager::Difficulty difficulty) const
    {
        return m_ai_kart_ident[difficulty];
    }

};   // ChallengeData

#endif   // HEADER_CHALLENGE_DATA_HPP
