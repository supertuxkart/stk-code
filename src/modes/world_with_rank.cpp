//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Joerg Henrichs
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

#include "modes/world_with_rank.hpp"

#include "race/history.hpp"

//-----------------------------------------------------------------------------
void WorldWithRank::init()
{
    World::init();
    m_position_index.resize(m_karts.size());
}   // init

//-----------------------------------------------------------------------------
/** Returns the kart with a given position.
 *  \param p The position of the kart, 1<=p<=num_karts).
 */
const Kart* WorldWithRank::getKartAtPosition(unsigned int p) const
{
    if(p<1 || p>m_position_index.size())
        return NULL;

    return m_karts[m_position_index[p-1]];
}   // getKartAtPosition

//-----------------------------------------------------------------------------
void WorldWithRank::setKartPosition(unsigned int kart_id,
                                    unsigned int position)
{
    m_position_index[position-1] = kart_id;
    m_karts[kart_id]->setPosition(position);
}   // setKartPosition

// ----------------------------------------------------------------------------
/** Called by the race result GUI at the end of the race to know the final 
 *  order. 
 * \param[out] order returns the order of karts. order[0] will contain the ID 
 *                   of the first kart, order[1] the ID of the second kart, 
 *                   etc... Array dimension will be adjusted to the number of 
 *                   karts.
 */
void WorldWithRank::getRaceResultOrder(std::vector<int> *order)
{
    const unsigned int num_karts = getNumKarts();
    order->resize(num_karts);
    
#ifndef NDEBUG
    for (unsigned int i=0; i < num_karts; i++) (*order)[i] = -1;
    
    bool positions_ok = true;
#endif
    
    for (unsigned int i=0; i < num_karts; i++)
    {
        const int position = getKart(i)->getPosition()-1;
        
#ifndef NDEBUG
        // sanity checks
        if ((*order)[position] != -1)
        {
            std::cerr << "== TWO KARTS ARE BEING GIVEN THE SAME POSITION!! ==\n";
            for (unsigned int j=0; j < num_karts; j++)
            {
                if ((*order)[j] == -1)      
                {
                    std::cout << "    No kart is yet set at position " << j << std::endl;
                }
                else
                {
                    std::cout << "    Kart " << (*order)[j] << " is at position " << j << std::endl;
                }
            }
            std::cout << "Kart " << i << " is being given posiiton " << (getKart(i)->getPosition()-1)
                      << ", but this position is already taken\n";
            positions_ok = false;
        }
#endif
        
        // actually assign the position
        (*order)[position] = i; // even for eliminated karts
    }
    
#ifndef NDEBUG
    if (!positions_ok) history->Save();
    assert(positions_ok);
#endif
    
}   // getRaceResultOrder

