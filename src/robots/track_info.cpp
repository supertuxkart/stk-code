//  $Id: track_info.cpp 2547 2008-12-02 13:01:39Z hikerstk $
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

#include "constants.hpp"
#include "track.hpp"
#include "robots/track_info.hpp"

TrackInfo::TrackInfo(const Track *track)
{
    m_track = track;
    setupSteerInfo();
}   // TrackInfo

// ----------------------------------------------------------------------------
/** Creates the steer-info array.
 */
void TrackInfo::setupSteerInfo()
{
    // First find the beginning of a new section, i.e. a place where the track
    // direction changes from either straight to curve or the other way round.
    int i = 0;
    int num_drivelines  = m_track->m_driveline.size();
    float current_angle = m_track->m_angle[i];
    while(i<num_drivelines)
    {
        DirectionType dir = computeDirection(i);
        i++;
    }
}   // setupSteerInfo

// ----------------------------------------------------------------------------
TrackInfo::DirectionType TrackInfo::computeDirection(int i)
{
    int   i_prev     = i-1;
    if(i_prev<0) i_prev = m_track->m_angle.size()-1;
    float prev_angle = m_track->m_angle[i_prev];
    float angle      = m_track->m_angle[i];

    float diff       = prev_angle - angle;
    while( diff>  2*M_PI ) diff -= 2*M_PI;
    while( diff < -2*M_PI ) diff += 2*M_PI;

    if( diff > M_PI ) diff -= 2*M_PI;
    else if( diff < -M_PI ) diff+= 2*M_PI;

    // Consider a difference of up to 5 degrees as 'straight'.
    const float curve_degree = 5*M_PI/180.0f;  
    DirectionType t = DIR_STRAIGHT;
    if (diff <-curve_degree) 
        t = DIR_LEFT;
    else if (diff > curve_degree)
        t = DIR_RIGHT;
    return t;
}   // computeDirection

// ----------------------------------------------------------------------------
