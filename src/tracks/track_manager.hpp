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

#ifndef HEADER_TRACK_MANAGER_HPP
#define HEADER_TRACK_MANAGER_HPP

#include <string>
#include <vector>
#include <map>

class Track;

/**
  * \brief Simple class to load and manage track data, track names and such
  * \ingroup tracks
  */
class TrackManager
{
private:
    /** All directories in which tracks are searched. */
    static std::vector<std::string>          m_track_search_path;

    /** All directories in which tracks were found. */
    std::vector<std::string>                 m_all_track_dirs;

    typedef std::vector<Track*>              Tracks;

    /** All track objects. */
    Tracks                                   m_tracks;

    typedef std::map<std::string, std::vector<int> > Group2Indices;
    /** List of all racing track groups. */
    Group2Indices                            m_track_groups;

    /** List of all arena groups. */
    Group2Indices                            m_arena_groups;

    /** List of all soccer arena groups. */
    Group2Indices                            m_soccer_arena_groups;

    /** List of the names of all groups containing tracks */
    std::vector<std::string>                 m_track_group_names;

    /** List of the names of all groups containing arenas */
    std::vector<std::string>                 m_arena_group_names;

    /** List of the names of all groups containing soccer arenas */
    std::vector<std::string>                 m_soccer_arena_group_names;

    /** Flag if this track is available or not. Tracks are set unavailable
     *  if they are not available on all clients (applies only to network mode)
     */
    std::vector<bool>                        m_track_avail;

    void          updateGroups(const Track* track);

public:
                TrackManager();
               ~TrackManager();

    static void addTrackSearchDir(const std::string &dir);
    /** Returns a list of all track identifiers. */
    std::vector<std::string> getAllTrackIdentifiers();

    /** Load all .track files from all directories */
    void  loadTrackList();
    void  removeTrack(const std::string &ident);
    bool  loadTrack(const std::string& dirname);
    void  removeAllCachedData();
    int   getNumberOfRaceTracks() const;
    Track* getTrack(const std::string& ident) const;
    // ------------------------------------------------------------------------
    /** Sets a list of track as being unavailable (e.g. in network mode the
     *  track is not on all connected machines.
     *  \param tracks List of tracks to mark as unavilable. */
    void setUnavailableTracks(const std::vector<std::string> &tracks);
    // ------------------------------------------------------------------------
    /** \brief Returns a list of all directories that contain a track. */
    const std::vector<std::string>* getAllTrackDirs() const
    {
        return &m_all_track_dirs;
    }   // getAllTrackDirs
    // ------------------------------------------------------------------------
    /** \brief Returns a list of the names of all used track groups. */
    const std::vector<std::string>& getAllTrackGroups() const
    {
        return m_track_group_names;
    }   // getAllTrackGroups
    // ------------------------------------------------------------------------
    /** \brief Returns a list of the names of all used arena groups. */
    const std::vector<std::string>&
                  getAllArenaGroups(bool soccer_arena=false) const
    {
        return soccer_arena ? m_soccer_arena_group_names : m_arena_group_names;
    }   // getAllArenaGroups
    // ------------------------------------------------------------------------
    /** Returns the number of tracks. */
    size_t getNumberOfTracks() const { return m_tracks.size(); }
    // ------------------------------------------------------------------------
    /** Returns the track with a given index number.
     *  \param index The index number of the track. */
    Track* getTrack(unsigned int index) const { return m_tracks[index];}
    // ------------------------------------------------------------------------
    /** Checks if a certain track is available.
     *  \param n Index of the track to check. */
    bool isAvailable(unsigned int n) const {return m_track_avail[n];}
    // ------------------------------------------------------------------------
    /** Returns a list of all tracks in a given group.
     *  \param g Name of the group. */
    const std::vector<int>& getTracksInGroup(const std::string& g)
    {
        return m_track_groups[g];
    }   // getTracksInGroup
    // ------------------------------------------------------------------------
    /** Returns a list of all arenas in a given group.
     *  \param g Name of the group. */
    const std::vector<int>&
        getArenasInGroup(const std::string& g, bool soccer_arena=false)
    {
        return soccer_arena ? m_soccer_arena_groups[g] : m_arena_groups[g];
    }   // getArenasInGroup

};   // TrackManager

extern TrackManager* track_manager;

#endif   // HEADER_TRACK_MANAGER_HPP
