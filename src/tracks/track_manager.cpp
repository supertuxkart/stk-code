//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 SuperTuxKart-Team
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

#include "tracks/track_manager.hpp"

#include "config/stk_config.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "tracks/track.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <stdio.h>

#include <IFileSystem.h>
#include <ITexture.h>
#include <IVideoDriver.h>

#ifndef SERVER_ONLY
#include <ge_main.hpp>
#endif

TrackManager* track_manager = 0;
std::vector<std::string>  TrackManager::m_track_search_path;

/** Constructor (currently empty). The real work happens in loadTrackList.
 */
TrackManager::TrackManager()
{
    m_current_favorite_status = NULL;
}   // TrackManager

//-----------------------------------------------------------------------------
/** Delete all tracks.
 */
TrackManager::~TrackManager()
{
    for(Tracks::iterator i = m_tracks.begin(); i != m_tracks.end(); ++i)
        delete *i;
}   // ~TrackManager

//-----------------------------------------------------------------------------
void TrackManager::removeTrackSearchDirs()
{
    m_track_search_path.clear();
}   // removeTrackSearchDirs

//-----------------------------------------------------------------------------
/** Adds a directory from which tracks are loaded. The track manager checks if
 *  either this directory itself contains a track, and if any subdirectory
 *  contains a track.
 *  \param dir The directory to add.
 */
void TrackManager::addTrackSearchDir(const std::string &dir)
{
    m_track_search_path.push_back(dir);
}   // addTrackDir

//-----------------------------------------------------------------------------
/** Returns the number of racing tracks. Those are tracks that are not 
 *  internal (like cut scenes), arenas, or soccer fields.
 */
int TrackManager::getNumberOfRaceTracks() const
{
    int n=0;
    for(unsigned int i=0; i<m_tracks.size(); i++)
        if(m_tracks[i]->isRaceTrack())
            n++;
    return n;
}   // getNumberOfRaceTracks

//-----------------------------------------------------------------------------
/** Get TrackData by the track identifier.
 *  \param ident Identifier = basename of the directory the track is in.
 *  \return      The corresponding track object, or NULL if not found
 */
Track* TrackManager::getTrack(const std::string& ident) const
{
    for(Tracks::const_iterator i = m_tracks.begin(); i != m_tracks.end(); ++i)
    {
        if ((*i)->getIdent() == ident)
            return *i;
    }

    return NULL;

}   // getTrack

//-----------------------------------------------------------------------------
/** Removes all cached data from all tracks. This is called when the screen
 *  resolution is changed and all textures need to be bound again.
 */
void TrackManager::removeAllCachedData()
{
    for(Tracks::const_iterator i = m_tracks.begin(); i != m_tracks.end(); ++i)
        (*i)->removeCachedData();
}   // removeAllCachedData
//-----------------------------------------------------------------------------
/** Sets all tracks that are not in the list a to be unavailable. This is used
 *  by the network manager upon receiving the list of available tracks from
 *  a client.
 *  \param tracks List of all track identifiere (available on a client).
 */
void TrackManager::setUnavailableTracks(const std::vector<std::string> &tracks)
{
    for(Tracks::const_iterator i = m_tracks.begin(); i != m_tracks.end(); ++i)
    {
        if(!m_track_avail[i-m_tracks.begin()]) continue;
        const std::string id=(*i)->getIdent();
        if (std::find(tracks.begin(), tracks.end(), id)==tracks.end())
        {
            m_track_avail[i-m_tracks.begin()] = false;
            Log::warn("TrackManager", "Track '%s' not available on all clients, disabled.",
                      id.c_str());
        }   // if id not in tracks
    }   // for all available tracks in track manager

}   // setUnavailableTracks

//-----------------------------------------------------------------------------
/** Returns a list with all track identifiert.
 */
std::vector<std::string> TrackManager::getAllTrackIdentifiers()
{
    std::vector<std::string> all;
    for(Tracks::const_iterator i = m_tracks.begin(); i != m_tracks.end(); ++i)
    {
        all.push_back((*i)->getIdent());
    }
    return all;
}   // getAllTrackNames

//-----------------------------------------------------------------------------
/** Loads all tracks from the track directory (data/track).
 */
void TrackManager::loadTrackList()
{
    m_all_track_dirs.clear();

    m_track_group_names.clear();
    m_track_groups.clear();
    m_arena_group_names.clear();
    m_soccer_arena_group_names.clear();
    m_arena_groups.clear();
    m_soccer_arena_groups.clear();

    m_track_groups_no_custom.clear();
    m_arena_groups_no_custom.clear();
    m_soccer_arena_groups_no_custom.clear();

    m_track_avail.clear();
    // This function is called when install a new addons, delete previous
    // tracks
    for (Track* track : m_tracks)
        delete track;
    m_tracks.clear();

    for(unsigned int i=0; i<m_track_search_path.size(); i++)
    {
        const std::string &dir = m_track_search_path[i];

        // First test if the directory itself contains a track:
        // ----------------------------------------------------
        if(loadTrack(dir)) continue;  // track found, no more tests

        // Then see if a subdir of this dir contains tracks
        // ------------------------------------------------
        std::set<std::string> dirs;
        file_manager->listFiles(dirs, dir);
        for(std::set<std::string>::iterator subdir = dirs.begin();
            subdir != dirs.end(); subdir++)
        {
            if(*subdir=="." || *subdir=="..") continue;
            loadTrack(dir+*subdir+"/");
        }   // for dir in dirs
    }   // for i <m_track_search_path.size()
    updateScreenshotCache();
    onDemandLoadTrackScreenshots();
}  // loadTrackList

// ----------------------------------------------------------------------------
/** Tries to load a track from a single directory. Returns true if a track was
 *  successfully loaded.
 *  \param dirname Name of the directory to load the track from.
 */
bool TrackManager::loadTrack(const std::string& dirname)
{
    std::string config_file = dirname+"track.xml";
    if(!file_manager->fileExists(config_file))
        return false;

    Track *track;

    try
    {
        track = new Track(config_file);
    }
    catch (std::exception& e)
    {
        Log::error("TrackManager", "Cannot load track <%s> : %s\n",
                dirname.c_str(), e.what());
        return false;
    }

    if (track->getVersion()<stk_config->m_min_track_version ||
        track->getVersion()>stk_config->m_max_track_version)
    {
        Log::warn("TrackManager", "Track '%s' is not supported "
                        "by this binary, ignored. (Track is version %i, this "
                        "executable supports from %i to %i).",
                  track->getIdent().c_str(), track->getVersion(),
                  stk_config->m_min_track_version,
                  stk_config->m_max_track_version);
        delete track;
        return false;
    }
    m_all_track_dirs.push_back(dirname);
    m_tracks.push_back(track);
    m_track_avail.push_back(true);
    updateGroups(track);
    return true;
}   // loadTrack

// ----------------------------------------------------------------------------
/** Removes a track.
 *  \param ident Identifier of the track (i.e. the name of the directory).
 */
void TrackManager::removeTrack(const std::string &ident)
{
    Track *track = getTrack(ident);
    if (track == NULL)
        Log::fatal("TrackManager", "There is no track named '%s'!!", ident.c_str());

    if (track->isInternal()) return;

    std::vector<Track*>::iterator it = std::find(m_tracks.begin(),
                                                 m_tracks.end(), track);
    if (it == m_tracks.end())
        Log::fatal("TrackManager", "Cannot find track '%s' in map!!", ident.c_str());

    int index = int(it - m_tracks.begin());

    // Remove the track from all groups it belongs to
    Group2Indices &group_2_indices =
            (track->isArena() ? m_arena_groups :
             (track->isSoccer() ? m_soccer_arena_groups :
               m_track_groups));
    
    Group2Indices &group_2_indices_no_custom =
            (track->isArena() ? m_arena_groups_no_custom :
             (track->isSoccer() ? m_soccer_arena_groups_no_custom :
               m_track_groups_no_custom));

    std::vector<std::string> &group_names =
            (track->isArena() ? m_arena_group_names :
             (track->isSoccer() ? m_soccer_arena_group_names :
               m_track_group_names));

    std::vector<std::string> groups=track->getGroups();
    if (m_current_favorite_status)
    {
        for (auto it = m_current_favorite_status->m_favorite_tracks.begin();
                it != m_current_favorite_status->m_favorite_tracks.end(); it++)
        { // User-defined groups
            if (it->second.find(ident) != it->second.end())
            {
                groups.push_back(it->first);
            }
        }
    }

    for(unsigned int i=0; i<groups.size(); i++)
    {
        std::vector<int> &indices = group_2_indices_no_custom[groups[i]];
        std::vector<int>::iterator j;
        j = std::find(indices.begin(), indices.end(), index);
        if (j != indices.end())
            indices.erase(j);

        // If the track was the last member of a group,
        // completely remove the group
        if(indices.size()==0)
        {
            group_2_indices_no_custom.erase(groups[i]);
        }   // if complete group must be removed

        indices = group_2_indices[groups[i]];
        j = std::find(indices.begin(), indices.end(), index);
        assert(j!=indices.end());
        indices.erase(j);

        // If the track was the last member of a group,
        // completely remove the group
        if(indices.size()==0)
        {
            group_2_indices.erase(groups[i]);
            std::vector<std::string>::iterator it_g;
            it_g = std::find(group_names.begin(), group_names.end(),
                            groups[i]);
            assert(it_g!=group_names.end());
            group_names.erase(it_g);
        }   // if complete group must be removed
    }   // for i in groups

    // Adjust all indices of tracks with an index number higher than
    // the removed track, since they have been moved down. This must
    // be done for all tracks and all arenas
    for(unsigned int i=0; i<3; i++)  // i=0: soccer arenas, i=1: arenas, i=2: tracks
    {
        Group2Indices &g2i = (i==0 ? m_soccer_arena_groups :
                               (i==1 ? m_arena_groups :
                                 m_track_groups));
        Group2Indices &g2i_nc = (i==0 ? m_soccer_arena_groups_no_custom :
                                (i==1 ? m_arena_groups_no_custom :
                                 m_track_groups_no_custom));
        Group2Indices::iterator j;
        for(j = g2i.begin(); j != g2i.end(); j++)
        {
            for(unsigned int i = 0; i < (*j).second.size(); i++)
                if((*j).second[i] > index) (*j).second[i]--;
        }   // for j in group_2_indices
        for(j = g2i_nc.begin(); j != g2i_nc.end(); j++)
        {
            for(unsigned int i = 0; i < (*j).second.size(); i++)
                if((*j).second[i] > index) (*j).second[i]--;
        }   // for j in group_2_indices
    }   // for i in arenas, tracks

    m_tracks.erase(it);
    m_all_track_dirs.erase(m_all_track_dirs.begin()+index);
    m_track_avail.erase(m_track_avail.begin()+index);
    delete track;
}   // removeTrack

// ----------------------------------------------------------------------------
/** \brief Updates the groups after a track was read in.
  * \param track Pointer to the new track, whose groups are now analysed.
  */
void TrackManager::updateGroups(const Track* track)
{
    if (track->isInternal()) return;

    std::string ident = track->getIdent();
    std::vector<std::string> new_groups = track->getGroups();

    Group2Indices &group_2_indices =
            (track->isArena() ? m_arena_groups :
             (track->isSoccer() ? m_soccer_arena_groups :
               m_track_groups));
    
    Group2Indices &group_2_indices_no_custom =
            (track->isArena() ? m_arena_groups_no_custom :
             (track->isSoccer() ? m_soccer_arena_groups_no_custom :
               m_track_groups_no_custom));

    std::vector<std::string> &group_names =
            (track->isArena() ? m_arena_group_names :
             (track->isSoccer() ? m_soccer_arena_group_names :
               m_track_group_names));

    const unsigned int groups_amount = (unsigned int)new_groups.size();
    for(unsigned int i=0; i<groups_amount; i++)
    {
        bool group_exists = group_2_indices.find(new_groups[i])
                                                      != group_2_indices.end();
        if(!group_exists)
            group_names.push_back(new_groups[i]);
        group_2_indices[new_groups[i]].push_back((int)m_tracks.size()-1);
        group_2_indices_no_custom[new_groups[i]].push_back((int)m_tracks.size()-1);
    }

    if (m_current_favorite_status)
    {
        for (auto it = m_current_favorite_status->m_favorite_tracks.begin();
                it != m_current_favorite_status->m_favorite_tracks.end(); it++)
        { // User-defined groups
            if (it->second.find(ident) != it->second.end())
            {
                bool group_exists = group_2_indices.find(ident)
                                                        != group_2_indices.end();
                if(!group_exists)
                    group_names.push_back(ident);
                group_2_indices[ident].push_back((int)m_tracks.size()-1);
            }
        }
    }
}   // updateGroups

// ----------------------------------------------------------------------------
/** \brief Adds a player's favorite track status to define the custom group
  */
void TrackManager::setFavoriteTrackStatus(FavoriteTrackStatus *status)
{
    clearFavoriteTrackStatus();

    m_current_favorite_status = status;

    if (status == NULL)
    {
        return;
    }
    
    // Add all user-defined groups
    for (auto it = status->m_favorite_tracks.begin(); it != status->m_favorite_tracks.end(); it++)
    {
        for (auto it_name = it->second.begin(); it_name != it->second.end(); it_name++)
        {
            int id = getTrackIndexByIdent(*it_name);
            Track *track = m_tracks[id];

            Group2Indices &group_2_indices =
                    (track->isArena() ? m_arena_groups :
                    (track->isSoccer() ? m_soccer_arena_groups :
                    m_track_groups));
                
            group_2_indices[it->first].push_back(id);
        }
    }
    for (int i = 0; i < 3; i++)
    {
        Group2Indices &g2i = (i==0 ? m_soccer_arena_groups :
                             (i==1 ? m_arena_groups :
                                m_track_groups));
        std::vector<std::string> &gn = (i==0 ? m_soccer_arena_group_names :
                                        (i==1 ? m_arena_group_names :
                                            m_track_group_names));
        gn.clear();
        for (auto it = g2i.begin(); it != g2i.end(); it++)
        {
            std::sort(it->second.begin(), it->second.end());
            auto unique_end = std::unique(it->second.begin(), it->second.end());
            it->second.erase(unique_end, it->second.end());
            gn.push_back(it->first);
        }
        // Make sure the order of groups are right
        std::sort(gn.begin(), gn.end(), [&, g2i](std::string &a, std::string &b)->bool{
            int x = g2i.find(a)->second[0], y = g2i.find(b)->second[0];
            return x == y ? a < b : x < y;
        });
    }
}   // addFavoriteTrack

// ----------------------------------------------------------------------------
/** \brief Clears the list of active favorite tracks, used e.g. when switching
 * between player profiles.
  */
void TrackManager::clearFavoriteTrackStatus()
{
    m_track_groups = m_track_groups_no_custom;
    m_arena_groups = m_arena_groups_no_custom;
    m_soccer_arena_groups = m_soccer_arena_groups_no_custom;

    m_current_favorite_status = NULL;
}   // clearFavoriteTracks

// ----------------------------------------------------------------------------
int TrackManager::getTrackIndexByIdent(const std::string& ident) const
{
    for (unsigned i = 0; i < m_tracks.size(); i++)
    {
        if (m_tracks[i]->getIdent() == ident)
            return i;
    }
    return -1;
}   // getTrackIndexByIdent

// ----------------------------------------------------------------------------
void TrackManager::onDemandLoadTrackScreenshots()
{
    if (irr_driver->getVideoDriver()->getDriverType() != video::EDT_VULKAN)
        return;
    for (unsigned i = 0; i < m_tracks.size(); i++)
    {
        if (m_tracks[i]->isInternal())
            continue;
        irr::video::ITexture* screenshot = irr_driver->getTexture(
            m_tracks[i]->getScreenshotFile());
        if (screenshot && screenshot->useOnDemandLoad())
            screenshot->getTextureHandler();
    }
}   // onDemandLoadTrackScreenshots

// ----------------------------------------------------------------------------
void TrackManager::updateScreenshotCache()
{
    for (unsigned i = 0; i < m_tracks.size(); i++)
    {
        // Populate the texture cache with track screenshots
        // (internal tracks like end cutscene don't have screenshots)
        Track* t = m_tracks[i];
        if (t->isInternal() || t->getScreenshotFile().empty())
            continue;
        std::string full_path = file_manager->getFileSystem()
            ->getAbsolutePath(t->getScreenshotFile().c_str()).c_str();
        if (!file_manager->fileExists(full_path))
            continue;
#ifndef SERVER_ONLY
        if (GE::getDriver()->getDriverType() == video::EDT_VULKAN)
            GE::getGEConfig()->m_ondemand_load_texture_paths.insert(full_path);
#endif
        irr_driver->getTexture(t->getScreenshotFile());
    }
}   // updateScreenshotCache
