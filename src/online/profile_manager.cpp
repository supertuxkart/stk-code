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
    }

    // ============================================================================

    void ProfileManager::iterateCache(Profile * profile)
    {
        if(m_profiles_cache.size() == m_max_cache_size)
        {
            profile->setCacheBit();
            ProfilesMap::iterator iter;
            for (iter = m_profiles_cache.begin(); iter != m_profiles_cache.end(); ++iter)
            {
               if (!iter->second->getCacheBit())
                   return;
            }
            //All cache bits are one! Set then all to zero except the one currently being visited
            for (iter = m_profiles_cache.begin(); iter != m_profiles_cache.end(); ++iter)
            {
               iter->second->unsetCacheBit();
            }
            profile->setCacheBit();
        }

    }

    // ============================================================================

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
                   m_profiles_cache.erase(iter++);
                   continue;
               }
               else
                   ++iter;
            }
        }
        m_profiles_cache[profile->getID()] = profile;
        assert(m_profiles_cache.size() <= m_max_cache_size);

    }


    void ProfileManager::addToCache(Profile * profile)
    {
        if(cacheHit(profile->getID()))
        {
            //FIXME should do updating of values
            delete profile;
        }
        else
        {
            directToCache(profile);
        }
    }


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
        if(cacheHit(id))
        {
            m_currently_visiting = m_profiles_cache[id];
        }
        else
            m_currently_visiting = NULL;
    }

    // ============================================================================

    Profile * ProfileManager::getProfileByID(const uint32_t id)
    {
        if(cacheHit(id))
            return m_profiles_cache[id];
        return NULL;
    }



} // namespace Online
