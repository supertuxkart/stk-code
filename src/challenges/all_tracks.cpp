//  $Id: all_tracks.cpp 1259 2007-09-24 12:28:19Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be ruseful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <algorithm>
#include "challenges/all_tracks.hpp"
#include "world.hpp"

AllTracks::AllTracks() : Challenge("alltracks", "All Tracks")
{
    setChallengeDescription("Finish one race\nin each track");
    setFeatureDescription("New track: SnowTuxPeak\nnow available");
    setFeature("snowtuxpeak");
}   // AllTracks

//-----------------------------------------------------------------------------
void AllTracks::loadState(const lisp::Lisp* config)
{
    config->getVector("solved-tracks", m_raced_tracks);
}   // loadState

//-----------------------------------------------------------------------------
void AllTracks::saveState(lisp::Writer* writer)
{
    writer->write("solved-tracks\t", m_raced_tracks);
}   // saveState

//-----------------------------------------------------------------------------
bool AllTracks::raceFinished()
{
    // If the current track is not yet in the list of raced tracks, add it:
    std::string track_name = world->getTrack()->getIdent();
    if(std::find(m_raced_tracks.begin(), m_raced_tracks.end(), track_name)
        ==m_raced_tracks.end())
    {
        m_raced_tracks.push_back(track_name);
    }

    // Check if all tracks are finished. If so, unlock feature
    return (m_raced_tracks.size()==13);
}   // raceFinished
//-----------------------------------------------------------------------------
