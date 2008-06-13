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

#include <stdio.h>
#include <stdexcept>
#include "file_manager.hpp"
#include "string_utils.hpp"
#include "track_manager.hpp"
#include "track.hpp"
#include "sound_manager.hpp"
#include "translation.hpp"

TrackManager* track_manager = 0;

TrackManager::TrackManager()
{}   // TrackManager

//-----------------------------------------------------------------------------
TrackManager::~TrackManager()
{
    for(Tracks::iterator i = m_tracks.begin(); i != m_tracks.end(); ++i)
        delete *i;
}   // ~TrackManager

//-----------------------------------------------------------------------------
Track* TrackManager::getTrack(const std::string& ident) const
{
    for(Tracks::const_iterator i = m_tracks.begin(); i != m_tracks.end(); ++i)
    {
        if ((*i)->getIdent() == ident)
            return *i;
    }

    char msg[MAX_ERROR_MESSAGE_LENGTH];
    fprintf(stderr, "TrackManager: Couldn't find track: '%s'", ident.c_str() );
    throw std::runtime_error(msg);
}   // getTrack

//-----------------------------------------------------------------------------
Track* TrackManager::getTrack(size_t id) const
{
    return m_tracks[id];
}  // getTrack

//-----------------------------------------------------------------------------
size_t TrackManager::getTrackCount() const
{
    return m_tracks.size();
}   // getTrackCount

//-----------------------------------------------------------------------------
void TrackManager::loadTrackList ()
{
    // Load up a list of tracks - and their names
    std::set<std::string> dirs;
    file_manager->listFiles(dirs, file_manager->getTrackDir(), /*is_full_path*/ true);
    for(std::set<std::string>::iterator dir = dirs.begin(); dir != dirs.end(); dir++)
    {
        if(*dir=="." || *dir=="..") continue;
        std::string config_file;
        try
        {
            // getTrackFile appends dir, so it's opening: *dir/*dir.track
            config_file = file_manager->getTrackFile((*dir)+".track");
        }
        catch (std::exception& e)
        {
            (void)e;   // remove warning about unused variable
            continue;
        }
        FILE *f=fopen(config_file.c_str(),"r");
        if(!f) continue;

        m_tracks.push_back(new Track(config_file));

        // Read music files in that dir as well
        sound_manager->loadMusicFromOneDir(*dir);
    }
}  // loadTrackList
