//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010  Joerg Henrichs
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

/**
 *  A WorldWithRank is a world where the karts are ranked. This is the base
 *  class for races and battle modes - all of which rank the kart.
 *  A class using this as a subclass must call setKartRank(kart id, rank)
 *  and this class is used to access the ranks from other objects.
 * \ingroup modes
 */
class WorldWithRank : public World
{
private:
    /** This contains a mapping from race position to kart index. */
    std::vector<int> m_position_index;
public:
                  WorldWithRank() : World() {}
    /** call just after instanciating. can't be moved to the contructor as child
        classes must be instanciated, otherwise polymorphism will fail and the
        results will be incorrect */
    virtual void  init();

    void          setKartPosition(unsigned int kart_id,
                                 unsigned int position);

    /** Returns the kart with position p, 1<=p<=num_karts). */
    const Kart*   getKartAtPosition(unsigned int p) const 
                  { return m_karts[m_position_index[p-1]]; }
    
    /** Called by the race result GUI at the end of the race to know the 
     *  final order (fill in the 'order' array) */
    virtual void  getRaceResultOrder(std::vector<int> *order);
};   // WorldWithRank

#endif
