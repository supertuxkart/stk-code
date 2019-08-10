//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Glenn De Jonghe
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

#ifndef HEADER_ONLINE_PROFILE_MANAGER_HPP
#define HEADER_ONLINE_PROFILE_MANAGER_HPP

#include "utils/types.hpp"

#include <irrString.h>

#include <assert.h>
#include <map>
#include <string>

namespace Online
{
    class OnlineProfile;

/** Class that manages all online profiles. Profiles are used for storing
 *  online information from local users, but also to store information about
 *  remote users (e.g. if you want to see the achievements of another user
 *  a Profile for this user is created, the server is then queried for
 *  the information and the result is stored in that profile).
 *  The profile manager has two
 * \ingroup online.
 */
class ProfileManager
{
private:
    /** Singleton pointer. */
    static ProfileManager* m_profile_manager;

    ProfileManager();
    ~ProfileManager();

    /** The mapping of ids to profile. */
    typedef std::map<uint32_t, OnlineProfile*>    ProfilesMap;

    /** A map of profiles that is persistent. (i.e. no automatic
     *  removing happens) Normally used for the current user's profile
     *  and friends. */
    ProfilesMap m_profiles_persistent;

    /** Any profiles that don't go into the persistent map, go here. This
     *  uses a pseudo-LRU algorithm with age bits to remove entries when
     *  the max size is reached. */
    ProfilesMap  m_profiles_cache;

    /** A temporary profile that is currently being 'visited',
     *  e.g. its data is shown in a gui. */
    OnlineProfile* m_currently_visiting;

    /** The max size of the m_profiles cache. Its default size can be
     *  inrceased when necessary (e.g. when too many search results are
     *  loaded, to make sure they can be all stored). */
    unsigned int  m_max_cache_size;

    void updateCacheBits(OnlineProfile *profile);
    void addDirectToCache(OnlineProfile *profile);
    void updateFriendFlagsInCache(const ProfilesMap &cache,
                                  uint32_t profile_id);
    void updateAllFriendFlags(const OnlineProfile *profile);
public:
    /** Create the singleton instance. */
    static void create()
    {
        assert(!m_profile_manager);
        m_profile_manager = new ProfileManager();
    }   // create

    // ----------------------------------------------------------------
    /** Returns the singleton.
     *  \pre create has been called to create the singleton.
     */
    static ProfileManager* get()
    {
        assert(m_profile_manager);
        return m_profile_manager;
    }   // get
    // ----------------------------------------------------------------
    /** Destroys the singleton. */
    static void destroy()
    {
        delete m_profile_manager;
        m_profile_manager = NULL;
    }   // destroy
    // ----------------------------------------------------------------

    void addToCache(OnlineProfile *profile);
    OnlineProfile* addPersistent(OnlineProfile *profile);
    void deleteFromPersistent(const uint32_t id);
    void clearPersistent();
    void moveToCache(const uint32_t id);
    int  guaranteeCacheSize(unsigned int max_num);
    bool isInCache(const uint32_t id);
    bool inPersistent(const uint32_t id);
    OnlineProfile* getProfileByID(const uint32_t id);

    // ----------------------------------------------------------------
    /** Marks a given profile to be the currently visited one. This
     *  is used to mark the profiles that ave its data display (e.g.
     *  to see achievements either of a local or a remote player). */
    void setVisiting(const uint32_t id)
    {
        m_currently_visiting = getProfileByID(id);
    }   // setVisiting

    // ----------------------------------------------------------------
    /** \return the instance of the profile that's currently being
     *  visited */
    OnlineProfile* getVisitingProfile() { return m_currently_visiting; }

};   // class CurrentUser
} // namespace Online
#endif // HEADER_ONLINE_PROFILE_MANAGER_HPP
