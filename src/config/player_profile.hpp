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

#ifndef HEADER_PLAYER_PROFILE_HPP
#define HEADER_PLAYER_PROFILE_HPP

#include "challenges/story_mode_status.hpp"
#include "utils/leak_check.hpp"
#include "utils/no_copy.hpp"
#include "utils/types.hpp"

#include <irrString.h>
using namespace irr;

#include <string>

class AchievementsStatus;
namespace Online
{
    class CurrentUser; 
    class HTTPRequest;
    class OnlineProfile;
    class XMLRequest;
}
class UTFWriter;

/** Class for managing player profiles (name, usage frequency,
 *  etc.). All PlayerProfiles are managed by the PlayerManager.
 *  A PlayerProfile keeps track of the story mode progress using an instance
 *  of StoryModeStatus, and achievements with AchievementsStatus. All data
 *  is saved in the players.xml file.
 * \ingroup config
 */
class PlayerProfile : public NoCopy
{
public:
    /** The online state a player can be in. */
    enum OnlineState
    {
        OS_SIGNED_OUT = 0,
        OS_SIGNED_IN,
        OS_GUEST,
        OS_SIGNING_IN,
        OS_SIGNING_OUT
    };


private:
    LEAK_CHECK()

    /** The name of the player (wide string, so it can be in native
     *  language). */
    core::stringw m_local_name;

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

    /** True if this user has a saved session. */
    bool m_saved_session;

    /** If a session was saved, this will be the online user id to use. */
    int m_saved_user_id;

    /** The token of the saved session. */
    std::string m_saved_token;

    /** The complete challenge state. */
    StoryModeStatus *m_story_mode_status;

    AchievementsStatus *m_achievements_status;

public:

         PlayerProfile(const core::stringw &name, bool is_guest = false);
         PlayerProfile(const XMLNode *node);
    virtual ~PlayerProfile();
    void save(UTFWriter &out);
    void loadRemainingData(const XMLNode *node);
    void initRemainingData();
    void incrementUseFrequency();
    bool operator<(const PlayerProfile &other);
    bool operator>(const PlayerProfile &other);
    void raceFinished();
    void saveSession(int user_id, const std::string &token);
    void clearSession();

    /** Abstract virtual classes, to be implemented by the OnlinePlayer. */
    virtual void setUserDetails(Online::HTTPRequest *request,
                                const std::string &action,
                                const std::string &php_script = "") = 0;
    virtual uint32_t getOnlineId() const = 0;
    virtual PlayerProfile::OnlineState getOnlineState() const = 0;
    virtual Online::OnlineProfile* getProfile() const = 0;
    virtual void requestPoll() const = 0;
    virtual void requestSavedSession() = 0;
    virtual void onSTKQuit() const = 0;
    virtual Online::XMLRequest* requestSignIn(const irr::core::stringw &username,
                                              const irr::core::stringw &password,
                                              bool save_session,
                                              bool request_now = true) = 0;
    virtual void signIn(bool success, const XMLNode * input) = 0;
    virtual void signOut(bool success, const XMLNode * input) = 0;
    virtual void requestSignOut() = 0;
    virtual bool isLoggedIn() const { return false;  }
    // ------------------------------------------------------------------------
    /** Sets the name of this player. */
    void setName(const core::stringw& name)
    {
        #ifdef DEBUG
        assert(m_magic_number == 0xABCD1234);
        #endif
        m_local_name = name;
    }   // setName

    // ------------------------------------------------------------------------
    /** Returns the name of this player. */
    core::stringw getName() const
    {
        assert(m_magic_number == 0xABCD1234);
        return m_local_name.c_str();
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
        return m_story_mode_status->isLocked(feature);
    }   // isLocked
    // ------------------------------------------------------------------------
    /** Returns all active challenges. */
    void computeActive() { m_story_mode_status->computeActive(); }
    // ------------------------------------------------------------------------
    /** Returns the list of recently completed challenges. */
    std::vector<const ChallengeData*> getRecentlyCompletedChallenges()
    {
        return m_story_mode_status->getRecentlyCompletedChallenges();
    }   // getRecently Completed Challenges
    // ------------------------------------------------------------------------
    /** Sets the currently active challenge. */
    void setCurrentChallenge(const std::string &name)
    {
        m_story_mode_status->setCurrentChallenge(name);
    }   // setCurrentChallenge
    // ------------------------------------------------------------------------
    /** Callback when a GP is finished (to test if a challenge was
     *  fulfilled). */
    void grandPrixFinished() { m_story_mode_status->grandPrixFinished(); }
    // ------------------------------------------------------------------------
    unsigned int getPoints() const { return m_story_mode_status->getPoints(); }
    // ------------------------------------------------------------------------
    void setFirstTime(bool b) { m_story_mode_status->setFirstTime(b); }
    // ------------------------------------------------------------------------
    bool isFirstTime() const { return m_story_mode_status->isFirstTime(); }
    // ------------------------------------------------------------------------
    void clearUnlocked() { m_story_mode_status->clearUnlocked(); }
    // ------------------------------------------------------------------------
    /** Returns the current challenge for this player. */
    const ChallengeStatus* getCurrentChallengeStatus() const
    {
        return m_story_mode_status->getCurrentChallengeStatus();
    }   // getCurrentChallengeStatus
    // ------------------------------------------------------------------------
    const ChallengeStatus* getChallengeStatus(const std::string &id)
    {
        return m_story_mode_status->getChallengeStatus(id);
    }   // getChallengeStatus
    // ------------------------------------------------------------------------
    unsigned int getNumEasyTrophies() const
    {
        return m_story_mode_status->getNumEasyTrophies();
    }   // getNumEasyTrophies
    // ------------------------------------------------------------------------
    unsigned int getNumMediumTrophies() const
    {
        return m_story_mode_status->getNumMediumTrophies();
    }   // getNumEasyTrophies
    // -----------------------------------------------------------------------
    unsigned int getNumHardTrophies() const
    {
        return m_story_mode_status->getNumHardTrophies();
    }   // getNumHardTropies
    // ------------------------------------------------------------------------
    AchievementsStatus* getAchievementsStatus()
    {
        return m_achievements_status;
    }   // getAchievementsStatus
    // ------------------------------------------------------------------------
    /** Returns true if a session was saved for this player. */
    bool hasSavedSession() const { return m_saved_session;  }
    // ------------------------------------------------------------------------
    /** If a session was saved, return the id of the saved user. */
    int getSavedUserId() const
    { 
        assert(m_saved_session);
        return m_saved_user_id;
    }   // getSavedUserId
    // ------------------------------------------------------------------------
    /** If a session was saved, return the token to use. */
    const std::string& getSavedToken() const
    {
        assert(m_saved_session);
        return m_saved_token;
    }   // getSavedToken
    // ------------------------------------------------------------------------
};   // class PlayerProfile

#endif

/*EOF*/
