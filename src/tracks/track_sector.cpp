//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011  Joerg Henrichs
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

#include "tracks/track_sector.hpp"
#include "tracks/quad_graph.hpp"


// ----------------------------------------------------------------------------
/** Initialises the object, and sets the current graph node to be undefined.
 */
TrackSector::TrackSector()
{
    reset();
}   // TrackSector

// ----------------------------------------------------------------------------
void TrackSector::reset()
{
    m_current_graph_node    = QuadGraph::UNKNOWN_SECTOR;
    m_last_valid_graph_node = QuadGraph::UNKNOWN_SECTOR;
    m_on_road               = false;
}   // reset

// ----------------------------------------------------------------------------
/** Updates the current graph node index, and the track coordinates for
 *  the specified point.
 *  \param xyz The new coordinates to search the graph node for.
 */
void TrackSector::update(const Vec3 &xyz)
{
    int prev_sector = m_current_graph_node;

    QuadGraph::get()->findRoadSector(xyz, &m_current_graph_node);
    m_on_road = m_current_graph_node != QuadGraph::UNKNOWN_SECTOR;

    // If m_track_sector == UNKNOWN_SECTOR, then the kart is not on top of
    // the road, so we have to use search for the closest graph node.
    if(m_current_graph_node == QuadGraph::UNKNOWN_SECTOR)
    {
        m_current_graph_node = 
            QuadGraph::get()->findOutOfRoadSector(xyz,
                                                  prev_sector);
    }
    else
        m_last_valid_graph_node = m_current_graph_node;

    // Now determine the 'track' coords, i.e. ow far from the start of the 
    // track, and how far to the left or right of the center driveline.
    QuadGraph::get()->spatialToTrack(&m_current_track_coords, xyz, 
                                     m_current_graph_node);
}   // update

// ----------------------------------------------------------------------------
void TrackSector::rescue()
{
    // If the kart is off road, rescue it to the last valid track position
    // instead of the current one (since the sector might be determined by
    // being closest to it, which allows shortcuts like drive towards another
    // part of the lap, press rescue, and be rescued to this other part of
    // the track (example: math class, drive towards the left after start,
    // when hitting the books, press rescue --> you are rescued to the
    // end of the track).
    if(!isOnRoad())
    {
        m_current_graph_node = m_last_valid_graph_node;
    }

    // Using the predecessor has the additional advantage (besides punishing
    // the player a bit more) that it makes it less likely to fall in a 
    // rescue loop since the kart moves back on each attempt. 
    m_current_graph_node = QuadGraph::get()->getNode(m_current_graph_node)
                                               .getPredecessor();
    m_last_valid_graph_node = QuadGraph::get()->getNode(m_current_graph_node)
                                               .getPredecessor();
}    // rescue
