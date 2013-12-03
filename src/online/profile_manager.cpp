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

#include "online/current_user.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"

#include <sstream>
#include <stdlib.h>
#include <assert.h>

using namespace Online;

namespace Online{
    static ProfileManager* profile_manager_singleton(NULL);

    ProfileManager* ProfileManager::get()
    {
        if (profile_manager_singleton == NULL)
            profile_manager_singleton = new ProfileManager();
        return profile_manager_singleton;
    }

    void ProfileManager::deallocate()
    {
        delete profile_manager_singleton;
        profile_manager_singleton = NULL;
    }   // deallocate

    // ============================================================================
    ProfileManager::ProfileManager()
    {
        assert(m_max_cache_size > 1);
        m_currently_visiting = NULL;
    }

    // ============================================================================
    ProfileManager::~ProfileManager()
    {
        clearPersistent();
        ProfilesMap::iterator it;
        for ( it = m_profiles_cache.begin(); it != m_profiles_cache.end(); ++it ) {
            delete it->second;
        }
    }

    // ============================================================================

    void ProfileManager::iterateCache(Profile * profile)
    {
        if(m_profiles_cache.size() == m_max_cache_size)
        {
            profile->setCacheBit(true);
            ProfilesMap::iterator iter;
            for (iter = m_profiles_cache.begin(); iter != m_profiles_cache.end(); ++iter)
            {
               if (!iter->second->getCacheBit())
                   return;
            }
            //All cache bits are one! Set them all to zero except the one currently being visited
            for (iter = m_profiles_cache.begin(); iter != m_profiles_cache.end(); ++iter)
            {
               iter->second->setCacheBit(false);
            }
            profile->setCacheBit(true);
        }

    }

    // ============================================================================
    /** Initialisation before the object is displayed. If necessary this function
     *  will pause the race if it is running (i.e. world exists). While only some
     *  of the screen can be shown during the race (via the in-game menu you
     *  can get the options screen and the help screens only). This is used by
     *  the RaceResultGUI to leave the race running (for the end animation) while
     *  the results are being shown.
     */
    void ProfileManager::directToCache(Profile * profile)
    {
        assert(profile != NULL);
        if(m_profiles_cache.size() == m_max_cache_size)
        {
            ProfilesMap::iterator iter;
            for (iter = m_profiles_cache.begin(); iter != m_profiles_cache.end();)
            {
               if (!iter->second->getCacheBit())
               {
                   delete iter->second;
                   m_profiles_cache.erase(iter);
                   break;
               }
               else
                   ++iter;
            }
        }
        m_profiles_cache[profile->getID()] = profile;
        assert(m_profiles_cache.size() <= m_max_cache_size);

    }

    // ============================================================================
    /**
     * Adds a profile to the persistent map.
     * If a profile with the same id is already in there, the profiles are "merged" with as goal saving as much information.
     * (i.e. one profile instance could have already fetched the friends, while the other could have fetched the achievements.)
     */
    void ProfileManager::addPersistent(Profile * profile)
    {
        if(inPersistent(profile->getID()))
        {
            m_profiles_persistent[profile->getID()]->merge(profile);
        }
        else
        {
            m_profiles_persistent[profile->getID()] = profile;
        }
    }
    // ============================================================================
    /**
     * Removes and deletes an entry from the persistent map.
     */
    void ProfileManager::deleteFromPersistent(const uint32_t id)
    {
        if (inPersistent(id))
        {
            delete m_profiles_persistent[id];
            m_profiles_persistent.erase(id);
        }
        else
            Log::warn("ProfileManager::removePersistent", "Tried to remove profile with id %d from persistent while not present", id);
    }

    // ============================================================================

    void ProfileManager::clearPersistent()
    {
        ProfilesMap::iterator it;
        for ( it = m_profiles_persistent.begin(); it != m_profiles_persistent.end(); ++it ) {
            delete it->second;
        }
        m_profiles_persistent.clear();
    }

    // ============================================================================

    void ProfileManager::moveToCache(const uint32_t id)
    {
        if (inPersistent(id))
        {
            Profile * profile = getProfileByID(id);
            m_profiles_persistent.erase(id);
            addToCache(profile);
        }
        else
            Log::warn("ProfileManager::moveToCache", "Tried to move profile with id %d from persistent to cache while not present", id);
    }

    // ============================================================================

    void ProfileManager::addToCache(Profile * profile)
    {
        if(inPersistent(profile->getID()))
            m_profiles_persistent[profile->getID()]->merge(profile);
        else if(cacheHit(profile->getID()))
            m_profiles_cache[profile->getID()]->merge(profile);
        else
            directToCache(profile);
    }

    // ============================================================================

    bool ProfileManager::inPersistent(const uint32_t id)
    {
        if (m_profiles_persistent.find(id) != m_profiles_persistent.end())
            return true;
        return false;
    }

    // ============================================================================

    bool ProfileManager::cacheHit(const uint32_t id)
    {
        if (m_profiles_cache.find(id) != m_profiles_cache.end())
        {
            iterateCache(m_profiles_cache[id]);
            return true;
        }
        return false;
    }

    // ============================================================================
    void ProfileManager::setVisiting(const uint32_t id)
    {
        m_currently_visiting = getProfileByID(id);
    }

    // ============================================================================

    Profile * ProfileManager::getProfileByID(const uint32_t id)
    {

        if(inPersistent(id))
            return m_profiles_persistent[id];
        if(cacheHit(id))
            return m_profiles_cache[id];
        //FIXME not able to get! Now this should actually fetch the info from the server, but I haven't come up with a good asynchronous idea yet.
        return NULL;
    }



} // namespace Online
