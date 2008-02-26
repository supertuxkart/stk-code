//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#include <stdexcept>
#include "loader.hpp"
#include "string_utils.hpp"
#include "track_manager.hpp"
#include "track.hpp"
#include "translation.hpp"

TrackManager* track_manager = 0;

TrackManager::TrackManager()
{}

//-----------------------------------------------------------------------------
TrackManager::~TrackManager()
{
    for(Tracks::iterator i = m_tracks.begin(); i != m_tracks.end(); ++i)
        delete *i;
}

//-----------------------------------------------------------------------------
Track*
TrackManager::getTrack(const std::string& ident) const
{
    for(Tracks::const_iterator i = m_tracks.begin(); i != m_tracks.end(); ++i)
    {
        if ((*i)->getIdent() == ident)
            return *i;
    }

    char msg[MAX_ERROR_MESSAGE_LENGTH];
    fprintf(stderr, "TrackManager: Couldn't find track: '%s'", ident.c_str() );
    throw std::runtime_error(msg);
}

//-----------------------------------------------------------------------------
Track*
TrackManager::getTrack(size_t id) const
{
    return m_tracks[id];
}

//-----------------------------------------------------------------------------
size_t
TrackManager::getTrackCount() const
{
    return m_tracks.size();
}

//-----------------------------------------------------------------------------
void
TrackManager::loadTrackList ()
{
    // Load up a list of tracks - and their names
    std::set<std::string> files;
    loader->listFiles(files, "data");
    for(std::set<std::string>::iterator i = files.begin(); i != files.end(); ++i)
        {
            if(StringUtils::has_suffix(*i, ".track"))
            {
                std::string track_name= loader->getTrackFile(*i);
                m_tracks.push_back(new Track(track_name.c_str()));
            }
        }
}

