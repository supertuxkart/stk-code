//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#include <stdio.h>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <iostream>

#include "audio/music_manager.hpp"
#include "config/stk_config.hpp"
#include "io/file_manager.hpp"
#include "tracks/track.hpp"

TrackManager* track_manager = 0;
std::vector<std::string>  TrackManager::m_track_search_path;

/** Constructor (currently empty). The real work happens in loadTrackList.
 */
TrackManager::TrackManager()
{}   // TrackManager

//-----------------------------------------------------------------------------
/** Delete all tracks.
 */
TrackManager::~TrackManager()
{
    for(Tracks::iterator i = m_tracks.begin(); i != m_tracks.end(); ++i)
        delete *i;
}   // ~TrackManager

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
    
    std::cerr << "TrackManager: Couldn't find track: '" << ident << "'" << std::endl;
    return NULL;
    
}   // getTrack

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
            fprintf(stderr, "Track '%s' not available on all clients, disabled.\n",
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
    m_arena_groups.clear();
    m_track_avail.clear();
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
        file_manager->listFiles(dirs, dir, /*is_ileull_path*/ true);
        for(std::set<std::string>::iterator subdir = dirs.begin(); 
            subdir != dirs.end(); subdir++)
        {
            if(*subdir=="." || *subdir=="..") continue;
            loadTrack(dir+"/"+*subdir);
        }   // for dir in dirs
    }   // for i <m_track_search_path.size()
}  // loadTrackList

// ----------------------------------------------------------------------------
/** Tries to load a track from a single directory. Returns true if a track was
 *  successfully loaded.
 *  \param dirname Name of the directory to load the track from.
 */
bool TrackManager::loadTrack(const std::string& dirname)
{
    std::string config_file = dirname+"/track.xml";
    FILE *f=fopen(config_file.c_str(),"r");
    if(!f) return false;
    fclose(f);

    Track *track = new Track(config_file);
    if(track->getVersion()<stk_config->m_min_track_version ||
        track->getVersion()>stk_config->m_max_track_version)
    {
        fprintf(stderr, "[TrackManager] Warning: track '%s' is not supported by this binary, ignored.\n",
                track->getIdent().c_str());
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
/** \brief Updates the groups after a track was read in.
  * \param track Pointer to the new track, whose groups are now analysed.
  */
void TrackManager::updateGroups(const Track* track)
{
    const std::vector<std::string>& new_groups = track->getGroups();
    const bool isArena = track->isArena();
    
    const unsigned int groups_amount = new_groups.size();
    for(unsigned int i=0; i<groups_amount; i++)
    {
        if (isArena)
        {
            // update the list of group names if necessary
            const bool isInArenaGroupsList = (m_arena_groups.find(new_groups[i]) != m_arena_groups.end());
            if (!isInArenaGroupsList) m_arena_group_names.push_back(new_groups[i]);

            // add this track to its group
            m_arena_groups[new_groups[i]].push_back(m_tracks.size()-1);
        }
        else
        {
            // update the list of group names if necessary
            const bool isInTrackGroupsList = (m_track_groups.find(new_groups[i]) != m_track_groups.end());
            if (!isInTrackGroupsList) m_track_group_names.push_back(new_groups[i]);

            // add this track to its group
            m_track_groups[new_groups[i]].push_back(m_tracks.size()-1);
        }
    }
}   // updateGroups

// ----------------------------------------------------------------------------
