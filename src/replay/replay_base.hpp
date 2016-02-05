//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 Joerg Henrichs
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

#ifndef HEADER_REPLAY_BASE_HPP
#define HEADER_REPLAY_BASE_HPP

#include "LinearMath/btTransform.h"
#include "utils/no_copy.hpp"

#include <stdio.h>
#include <string>

/**
  * \ingroup race
  */
class ReplayBase : public NoCopy
{
    // Needs access to KartReplayEvent
    friend class GhostKart;
private:
    /** The filename of the replay file. Only defined after calling
     *  openReplayFile. */
    std::string m_filename;
protected:
    /** Stores a transform event, i.e. a position and rotation of a kart
     *  at a certain time. */
    struct TransformEvent
    {
        /** Time at which this event happens. */
        float               m_time;
        /** The transform at a certain time. */
        btTransform         m_transform;
    };   // TransformEvent

    // ------------------------------------------------------------------------
    struct PhysicInfo
    {
        /** The speed at a certain time. */
        float               m_speed;
        /** The steering at a certain time. */
        float               m_steer;
        /** The suspension length of 4 wheels at a certain time. */
        float               m_suspension_length[4];
    };   // PhysicInfo

    // ------------------------------------------------------------------------
    /** Records all other events - atm nitro and zipper handling. */
    struct KartReplayEvent
    {
        /** The type of event. */
        bool        m_on_nitro;
        bool        m_on_zipper;
    };   // KartReplayEvent

    // ------------------------------------------------------------------------
          ReplayBase();
    FILE *openReplayFile(bool writeable);
    // ----------------------------------------------------------------------
    /** Returns the filename that was opened. */
    const std::string &getReplayFilename() const { return m_filename;}
    // ----------------------------------------------------------------------
    /** Returns the version number of the replay file. This is used to check
     *  that a loaded replay file can still be understood by this
     *  executable. */
    unsigned int getReplayVersion() const { return 1; }
};   // ReplayBase

#endif
