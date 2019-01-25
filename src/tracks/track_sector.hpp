//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015  Joerg Henrichs
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

#ifndef HEADER_TRACK_SECTOR_HPP
#define HEADER_TRACK_SECTOR_HPP

#include "utils/vec3.hpp"

class BareNetworkString;
class Track;

/** This object keeps track of which sector an object is on. A sector is
 *  actually just the graph node (it's called sector to better distinguish
 *  the graph node from say xml node and scene node).
 *  An object that has a track sector can determine how far away it is from
 *  the start line, how far away it is from the center driveline. If the
 *  object is not actually on part of the quad graph, it will determine the
 *  closest sector it is to, and set a flag (!isOnRoad).
 *  This object will also keep track on the last valid sector an object was
 *  on, which is used to reset a kart in case of a rescue.

 */
class TrackSector
{
private:
    /** The graph node the object is on. */
    int  m_current_graph_node;

    /** The index of the estimated valid graph node. Used for distance. */
    int  m_estimated_valid_graph_node;

    /** The index of the last valid graph node. Used for rescue */
    int  m_last_valid_graph_node;

    /** The coordinates of this object on the track, i.e. how far from
     *  the start of the track, and how far to the left or right
     *  of the center driveline. */
    Vec3 m_current_track_coords;

    Vec3 m_estimated_valid_track_coords;

    Vec3 m_latest_valid_track_coords;

    /** True if the object is on the road (driveline), or not. */
    bool m_on_road;

    int m_last_triggered_checkline;

public:
          TrackSector();
    void  reset();
    void  rescue();
    void  update(const Vec3 &xyz, bool ignore_vertical = false);
    float getRelativeDistanceToCenter() const;
    // ------------------------------------------------------------------------
    /** Returns how far the the object is from the start line. */
    float getDistanceFromStart(bool account_for_checklines, bool strict=false) const
    {
        if (account_for_checklines && strict)
            return m_latest_valid_track_coords.getZ();
        else if (account_for_checklines)
            return m_estimated_valid_track_coords.getZ();
        else
            return m_current_track_coords.getZ();
    }
    // ------------------------------------------------------------------------
    /** Returns the distance to the centre driveline. */
    float getDistanceToCenter() const { return m_current_track_coords.getX(); }
    // ------------------------------------------------------------------------
    /** Returns the current graph node. */
    int getCurrentGraphNode() const {return m_current_graph_node;}
    // ------------------------------------------------------------------------
    /** Returns if this object is on the road (driveline). */
    bool isOnRoad() const { return m_on_road; }
    // ------------------------------------------------------------------------
    void setLastTriggeredCheckline(int i) { m_last_triggered_checkline = i; }
    // ------------------------------------------------------------------------
    int getLastTriggeredCheckline() const
                                         { return m_last_triggered_checkline; }
    // ------------------------------------------------------------------------
    int getLastValidGraphNode() const { return m_last_valid_graph_node; }
    // ------------------------------------------------------------------------
    void saveState(BareNetworkString* buffer) const;
    // ------------------------------------------------------------------------
    void rewindTo(BareNetworkString* buffer);
    // ------------------------------------------------------------------------
    void saveCompleteState(BareNetworkString* bns);
    // ------------------------------------------------------------------------
    void restoreCompleteState(const BareNetworkString& b);

};   // TrackSector

#endif

