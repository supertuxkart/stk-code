//  $Id: track_info.hpp 2547 2008-12-02 13:01:39Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008  Joerg Henrichs
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

#ifndef HEADER_TRACK_INFO_HPP
#define HEADER_TRACK_INFO_HPP

#include <vector>

class Track;

/** This class is used to pre-compute some track information used by the AI, 
 *  e.g. length of straight sections, turn radius etc.
 *  Each AI share this information, so it's only done once per track.
 */
class TrackInfo
{
    /** The direction of the drivelines: straight ahead, or a turn. */
    enum DirectionType {DIR_STRAIGHT, DIR_LEFT, DIR_RIGHT};

    /** This class stores information for a set of consecutive driveline 
     *  points. E.g points that form a straight line, or a left/right turn.
     */
    class SteerInfo
    {
    public:
        DirectionType m_type;
    };   // SteerInfo

    /** The list of all steer info objects. */
    std::vector<SteerInfo> m_steer_info;
    /** The mapping of driveline points to steer infos. Several driveline
     *  points will have the same steer info object. */
    std::vector<int>       m_driveline_2_steer_info;

    /** Pointer to the track. */
    const Track           *m_track;

    void  setupSteerInfo();

    DirectionType computeDirection(int i);
public:
    TrackInfo(const Track *track);
};   // TrackInfo

#endif