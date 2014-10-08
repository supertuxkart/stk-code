//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Glenn De Jonghe
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

#include "online/profile_manager.hpp"

#include "online/online_profile.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"

#include <algorithm>
#include <assert.h>
#include <sstream>
#include <stdlib.h>

using namespace Online;

namespace Online
{

ProfileManager* ProfileManager::m_profile_manager = NULL;

// ------------------------------------------------------------------------
/** Private constructor, used by static create() function.
 */
ProfileManager::ProfileManager()
{
    m_max_cache_size = 100;
    m_currently_visiting = NULL;
}   // ProfileManager

// ------------------------------------------------------------------------
/** Destructor, which frees the persistent and cached data.
 */
ProfileManager::~ProfileManager()
{
    clearPersistent();
    ProfilesMap::iterator it;
    for (it = m_profiles_cache.begin(); it != m_profiles_cache.end(); ++it)
    {
        delete it->second;
    }
}   // ~ProfileManager

// ------------------------------------------------------------------------
/** Makes sure that the cache can store at least max_num entries. This is
 *  used by the online search screen to make sure all results found can
 *  be cached at the same time.
 *  \param min_num Minimum number of entries the chache should be able to
 *         store.
 */
int  ProfileManager::guaranteeCacheSize(unsigned int min_num)
{
    if (m_max_cache_size < min_num)
    {
        // Avoid that the cache can grow too big by setting an
        // upper limit.
        if (min_num > 100)
            min_num = 100;
        m_max_cache_size = min_num;
    }

    return m_max_cache_size;
}   // guaranteeCacheSize

// ------------------------------------------------------------------------
/** Search for a given profile in the set of persistent and cached
 *  entries. If the profile does not exist, a NULL is returned.
 *  FIXME: This should be improved to download the profile is necessary.
 *  \param id The id of the profile to find.
 */
OnlineProfile* ProfileManager::getProfileByID(const uint32_t id)
{
    if (inPersistent(id))
        return m_profiles_persistent[id];
    if (isInCache(id))
        return m_profiles_cache[id];

    // FIXME not able to get! Now this should actually fetch the info from the
    // server, but I haven't come up with a good asynchronous idea yet.
    return NULL;
}   // getProfileByID

// ------------------------------------------------------------------------
/** Adds profile to the cache. If the profile is already persistent, then
*  it merges any new information from this profile to the persistent one.
*  If the entry is already in the cache, the cached entry will be updated
*  with any new information from the given profile. Otherwise, the profile
*  is just added to the cache.
*/
void ProfileManager::addToCache(OnlineProfile * profile)
{
    if (inPersistent(profile->getID()))
        m_profiles_persistent[profile->getID()]->merge(profile);
    else if (isInCache(profile->getID()))
        m_profiles_cache[profile->getID()]->merge(profile);
    else
        addDirectToCache(profile);
}   // addToCache

// ------------------------------------------------------------------------
/** Initialisation before the object is displayed. If necessary this function
 *  will pause the race if it is running (i.e. world exists). While only some
 *  of the screen can be shown during the race (via the in-game menu you
 *  can get the options screen and the help screens only). This is used by
 *  the RaceResultGUI to leave the race running (for the end animation) while
 *  the results are being shown.
 */
void ProfileManager::addDirectToCache(OnlineProfile* profile)
{
    assert(profile != NULL);
    if (m_profiles_cache.size() >= m_max_cache_size)
    {
        // Cache already full, try to find entries that
        // don't have their used bit set
        ProfilesMap::iterator iter;
        for (iter = m_profiles_cache.begin(); iter != m_profiles_cache.end();)
        {
            if (!iter->second->getCacheBit())
            {
                updateAllFriendFlags(iter->second);
                delete iter->second;
                iter = m_profiles_cache.erase(iter);
                // Keep on deleting till enough space is available.
                if(m_profiles_cache.size()<m_max_cache_size)
                    break;
            }
            else
            {
                ++iter;
            }
        } // for profile in cache
    }

    m_profiles_cache[profile->getID()] = profile;
}   // addDirectToCache

// ------------------------------------------------------------------------
/** Checks if a profile is in cache. If so, it updates its usage bit.
*  \param id Identifier for the profile to check.
*/
bool ProfileManager::isInCache(const uint32_t id)
{
    ProfilesMap::const_iterator i = m_profiles_cache.find(id);
    if (i != m_profiles_cache.end())
    {
        updateCacheBits(i->second);
        return true;
    }

    return false;
}   // isInCache

// ------------------------------------------------------------------------
/** This function is called when the specified profile id is removed from
 *  cache. It will search all currently cached profiles that have the
 *  'friends fetched' flag set, and reset that flag if the profile id is
 *  one of their friends. This fixes the problem that friend lists can
 *  get shortened if some of their friends are being pushed out of 
 *  cache. 
 */
void ProfileManager::updateFriendFlagsInCache(const ProfilesMap &cache,
                                              uint32_t profile_id)
{
    ProfilesMap::const_iterator i;
    for(i=cache.begin(); i!=cache.end(); i++)
    {
        // Profile has no friends fetched, no need to test
        if(!(*i).second->hasFetchedFriends()) continue;
        const OnlineProfile::IDList &friend_list = (*i).second->getFriends();
        OnlineProfile::IDList::const_iterator frnd;
        frnd = std::find(friend_list.begin(), friend_list.end(), profile_id);
        if(frnd!=friend_list.end())
            (*i).second->unsetHasFetchedFriends();
    }
}   // updateFriendFlagsInCache

// ------------------------------------------------------------------------
/** This function is called when the specified profile is removed from
 *  cache. It searches through all caches for profiles X, that have the
 *  given profile as friend, and then reset the 'friends fetched' flag
 *  in profile X. Otherwise if a profile is pushed out of the cache,
 *  all friends of this profile in cache will have an incomplete list
 *  of friends.
 */
void ProfileManager::updateAllFriendFlags(const OnlineProfile *profile)
{
    updateFriendFlagsInCache(m_profiles_persistent, profile->getID());
    updateFriendFlagsInCache(m_profiles_cache,      profile->getID());
}   // updateAllFriendFlags

// ------------------------------------------------------------------------
/** This function updates the cache bits of all cached entries. It will
*  set the cache bit of the given profile. Then, if the cachen is full
*  it will check if there are any entries that don't have the cache bit
*  set (i.e. entries that can be discarded because they were not used).
*  If no such entry is found, all usage flags will be reset, and only
*  the one for the given entry will remain set. This results with high
*  probability that most often used entries will remain in the cache,
*  without adding much overhead.
*/
void ProfileManager::updateCacheBits(OnlineProfile * profile)
{
    profile->setCacheBit(true);
    if (m_profiles_cache.size() >= m_max_cache_size)
    {
        ProfilesMap::iterator iter;
        for (iter = m_profiles_cache.begin();
            iter != m_profiles_cache.end(); ++iter)
        {
            if (!iter->second->getCacheBit())
                return;
        }

        // All cache bits are set! Set them all to zero except the one
        // currently being visited
        for (iter = m_profiles_cache.begin();
            iter != m_profiles_cache.end(); ++iter)
        {
            iter->second->setCacheBit(false);
        }
        profile->setCacheBit(true);
    }
}   // updateCacheBits

// ------------------------------------------------------------------------
/** True if the profile with the given id is persistent.
*  \param id The id of the profile to test.
*/
bool ProfileManager::inPersistent(const uint32_t id)
{
    return m_profiles_persistent.find(id) != m_profiles_persistent.end();
}   // inPersistent

// ------------------------------------------------------------------------
/** Adds a profile to the persistent map. If a profile with the same id
 *  is already in there, the profiles are "merged" with the goal to save as
 *  much information (i.e. one profile instance could have already fetched
 *  the friends, while the other could have fetched the achievements.)
 *  \param profile The profile to make persistent.
 */
void ProfileManager::addPersistent(OnlineProfile * profile)
{
    if (inPersistent(profile->getID()))
    {
        m_profiles_persistent[profile->getID()]->merge(profile);
    }
    else
    {
        m_profiles_persistent[profile->getID()] = profile;
    }
}   // addPersistent

// ------------------------------------------------------------------------
/** Removes and deletes an entry from the persistent map.
 *  \param id the id of the profile to be removed.
 */
void ProfileManager::deleteFromPersistent(const uint32_t id)
{
    if (inPersistent(id))
    {
        delete m_profiles_persistent[id];
        m_profiles_persistent.erase(id);
    }
    else
    {
        Log::warn("ProfileManager",
                  "Tried to remove profile with id %d from persistent while "
                  "not present", id);
    }
}   // deleteFromPersistent


// ------------------------------------------------------------------------
/** Deletes all persistent profiles.
 */
void ProfileManager::clearPersistent()
{
    ProfilesMap::iterator it;
    for (it  = m_profiles_persistent.begin();
         it != m_profiles_persistent.end(); ++it)
    {
        delete it->second;
    }
    m_profiles_persistent.clear();
}   // clearPersistent

// ------------------------------------------------------------------------
/** Removes a currently persistent profile to the cache (where it can
 *  be deleted later).
 *  \param id The the id of the profile to be moved.
 */
void ProfileManager::moveToCache(const uint32_t id)
{
    if (inPersistent(id))
    {
        OnlineProfile * profile = getProfileByID(id);
        m_profiles_persistent.erase(id);
        addToCache(profile);
    }
    else
    {
        Log::warn("ProfileManager",
                  "Tried to move profile with id %d from persistent to "
                  "cache while not present", id);
    }
}   // moveToCache

} // namespace Online
