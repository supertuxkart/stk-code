//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015  Joerg Henrichs
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

#ifndef HEADER_WORLD_WITH_RANK_HPP
#define HEADER_WORLD_WITH_RANK_HPP

#include <vector>

#include "modes/world.hpp"

class AbstractKart;
class TrackSector;

/**
 *  A WorldWithRank is a world where the karts are ranked. This is the base
 *  class for races and battle modes - all of which rank the kart.
 *  A class using this as a subclass must call setKartPosition(kart id, position)
 *  and this class is used to access the ranks from other objects. This class
 *  adds a convenient rescue implementation: a kart is rescued to the closest
 *  start point. This is useful for battle, soccer, ... modes. Linear world
 *  defines its own rescue functions and will overwrite this.
 * \ingroup modes
 */
class WorldWithRank : public World
{
protected:
    /** This contains a mapping from race position to kart index. */
    std::vector<int> m_position_index;

    /** Whether to display the rank in the race GUI */
    bool m_display_rank;

    /** The points given to a kart on a given position (index is
     *  0 based, so using race-position - 1. */
    std::vector<int> m_score_for_position;

#ifdef DEBUG
    /** Used for debugging to help detect if the same kart position
     *  is used more than once. */
    std::vector<bool> m_position_used;

    /** True if beginSetKartPositions was called, false after
     *  endSetKartPositions. Used to make sure the sequence of calls
     *  is correct. */
    bool              m_position_setting_initialised;
#endif

    unsigned int getClosestStartPoint(AbstractKart *kart);

    /** Stores the current graph node and track coordinates for each kart. */
    std::vector<TrackSector*> m_kart_track_sector;

    // ------------------------------------------------------------------------
    void updateSectorForKarts();

public:
                  WorldWithRank() : World() {}
    virtual      ~WorldWithRank();
    /** call just after instanciating. can't be moved to the contructor as child
        classes must be instanciated, otherwise polymorphism will fail and the
        results will be incorrect */
    virtual void  init() OVERRIDE;
    virtual void  reset(bool restart=false) OVERRIDE;

    bool          displayRank() const { return m_display_rank; }

    void          beginSetKartPositions();
    bool          setKartPosition(unsigned int kart_id,
                                  unsigned int position);
    void          endSetKartPositions();
    AbstractKart* getKartAtPosition(unsigned int p) const;
    /** Returns the kart at which position (start from 1) to draw race icon
     *  \param p Position of the kart. */
    virtual AbstractKart* getKartAtDrawingPosition(unsigned int p) const
                                               { return getKartAtPosition(p); }
    virtual int   getScoreForPosition(int p);
    virtual unsigned int getRescuePositionIndex(AbstractKart *kart) OVERRIDE;
    // ------------------------------------------------------------------------
    /** Returns the track_sector object for the specified kart.
     *  \param kart_index World index of the kart. */
    TrackSector* getTrackSector(unsigned int kart_index) const
    {
        assert(kart_index < m_kart_track_sector.size());
        return m_kart_track_sector[kart_index];
    }   // getTrackSector
    // ------------------------------------------------------------------------
    bool isOnRoad(unsigned int kart_index) const;
    // ------------------------------------------------------------------------
    int getSectorForKart(const AbstractKart *kart) const;

};   // WorldWithRank

#endif
