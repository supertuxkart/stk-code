//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2013 SuperTuxKart-Team
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

#ifndef HEADER_PLAYER_HPP
#define HEADER_PLAYER_HPP

#include "challenges/game_slot.hpp"

#include "config/user_config.hpp"
#include "utils/no_copy.hpp"
#include "utils/types.hpp"

#include <irrString.h>
using namespace irr;

#include <string>

class GameSlot;
class UTFWriter;

/**
  * \brief Class for managing player profiles (name, control configuration, etc.)
  * A list of all possible players is stored as PlayerProfiles in the user config.
  * A list of currently playing players will be stored somewhere else (FIXME : complete comment)
  * \ingroup config
  */
class PlayerProfile : public NoCopy
{
private:

    /** The name of the player (wide string, so it can be in native 
     *  language). */
    core::stringw m_name;

    /** True if this account is a guest account. */
    bool m_is_guest_account;

#ifdef DEBUG
    unsigned int m_magic_number;
#endif

    /** Counts how often this player was used (always -1 for guests). */
    int m_use_frequency;

    /** A unique number for this player, used to link it to challenges etc. */
    unsigned int m_unique_id;

    /** True if this is the default (last used) player. */
    bool m_is_default;

    /** The complete challenge state. */
    GameSlot *m_game_slot;

public:

    PlayerProfile(const core::stringw& name, bool is_guest = false);

    PlayerProfile(const XMLNode* node);

    void save(UTFWriter &out);
    void incrementUseFrequency();
    bool operator<(const PlayerProfile &other);
    bool operator>(const PlayerProfile &other);


    // ------------------------------------------------------------------------
    ~PlayerProfile()
    {
        #ifdef DEBUG
        m_magic_number = 0xDEADBEEF;
        #endif
    }   // ~PlayerProfile

    // ------------------------------------------------------------------------
    /** Sets the name of this player. */
    void setName(const core::stringw& name)
    {
        #ifdef DEBUG
        assert(m_magic_number == 0xABCD1234);
        #endif
        m_name = name;
    }   // setName

    // ------------------------------------------------------------------------
    /** Returns the name of this player. */
    core::stringw getName() const
    {
        #ifdef DEBUG
        assert(m_magic_number == 0xABCD1234);
        #endif
        return m_name.c_str();
    }   // getName

    // ------------------------------------------------------------------------
    /** Returns true if this player is a guest account. */
    bool isGuestAccount() const
    {
        #ifdef DEBUG
        assert(m_magic_number == 0xABCD1234);
        #endif
        return m_is_guest_account;
    }   // isGuestAccount

    // ------------------------------------------------------------------------
    /** Returns the unique id of this player. */
    unsigned int getUniqueID() const { return m_unique_id; }
    // -----------------------------------------------------------------------
    /** Returns true if this is the default (last used) player. */
    bool isDefault() const { return m_is_default; }
    // ------------------------------------------------------------------------
    /** Sets if this player is the default player or not. */
    void setDefault(bool is_default) { m_is_default = is_default; }
    // ------------------------------------------------------------------------
    /** Returnes if the feature (kart, track) is locked. */
    bool isLocked(const std::string &feature) const
    {
        return m_game_slot->isLocked(feature); 
    }   // isLocked
    // ------------------------------------------------------------------------
    /** Returns all active challenges. */
    void computeActive() { m_game_slot->computeActive(); }
    // ------------------------------------------------------------------------
    /** Returns the list of recently completed challenges. */
    std::vector<const ChallengeData*> getRecentlyCompletedChallenges() 
    {
        return m_game_slot->getRecentlyCompletedChallenges();
    }   // getRecently Completed Challenges
    // ------------------------------------------------------------------------
    /** Sets the currently active challenge. */
    void setCurrentChallenge(const std::string &name)
    {
        m_game_slot->setCurrentChallenge(name);
    }   // setCurrentChallenge
    // ------------------------------------------------------------------------
    /** Notification of a finished race, which can trigger fulfilling 
     *  challenges. */
    void raceFinished() { m_game_slot->raceFinished(); }
    // ------------------------------------------------------------------------
    /** Callback when a GP is finished (to test if a challenge was
     *  fulfilled). */
    void grandPrixFinished() { m_game_slot->grandPrixFinished(); }
    // ------------------------------------------------------------------------
    unsigned int getPoints() const { return m_game_slot->getPoints(); }
    // ------------------------------------------------------------------------
    void setFirstTime(bool b) { m_game_slot->setFirstTime(b); }
    // ------------------------------------------------------------------------
    bool isFirstTime() const { return m_game_slot->isFirstTime(); }
    // ------------------------------------------------------------------------
    void clearUnlocked() { m_game_slot->clearUnlocked(); }
    // ------------------------------------------------------------------------
    /** Returns the current challenge for this player. */
    const Challenge* getCurrentChallenge() const
    {
        return m_game_slot->getCurrentChallenge();
    }   // getCurrentChallenge
    // ------------------------------------------------------------------------
    const Challenge* getChallenge(const std::string &id)
    {
        return m_game_slot->getChallenge(id);
    }   // getChallenge
    // ------------------------------------------------------------------------
    unsigned int getNumEasyTrophies() const
    {
        return m_game_slot->getNumEasyTrophies(); 
    }   // getNumEasyTrophies
    // ------------------------------------------------------------------------
    unsigned int getNumMediumTrophies() const
    {
        return m_game_slot->getNumMediumTrophies();
    }   // getNumEasyTrophies
    // -----------------------------------------------------------------------
    unsigned int getNumHardTrophies() const
    {
        return m_game_slot->getNumHardTrophies(); 
    }   // getNumHardTropies

};   // class PlayerProfile

#endif

/*EOF*/
