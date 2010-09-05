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
    m_position_used.resize(m_karts.size());
    m_position_setting_initialised = false;
}   // init

//-----------------------------------------------------------------------------
/** Returns the kart with a given position.
 *  \param p The position of the kart, 1<=p<=num_karts).
 */
Kart* WorldWithRank::getKartAtPosition(unsigned int p) const
{
    if(p<1 || p>m_position_index.size())
        return NULL;

    return m_karts[m_position_index[p-1]];
}   // getKartAtPosition

//-----------------------------------------------------------------------------
/** This function must be called before starting to set all kart positions 
 *  again. It's mainly used to add some debug support, i.e. detect if the
 *  same position is set in different karts.
 */
void WorldWithRank::beginSetKartPositions()
{
#ifdef DEBUG
    assert(!m_position_setting_initialised);
    m_position_setting_initialised = true;

    for(unsigned int i=0; i<m_position_used.size(); i++)
        m_position_used[i] = false;
#endif
}   // beginSetKartPositions

//-----------------------------------------------------------------------------
/** Sets the position of a kart. This will be saved in this object to allow
 *  quick lookup of which kart is on a given position, but also in the
 *  kart objects.
 *  \param kart_id The index of the kart to set the position for.
 *  \param position The position of the kart (1<=position<=num karts).
 *  \return false if this position was already set, i.e. an inconsistency in
 *          kart positions has occurred. This is used in debug mode only to
 *          allow the calling function to print debug information.
 */
bool WorldWithRank::setKartPosition(unsigned int kart_id,
                                    unsigned int position)
{
    m_position_index[position-1] = kart_id;
    m_karts[kart_id]->setPosition(position);
#ifdef DEBUG
    assert(m_position_setting_initialised);
    if(m_position_used[position-1])
    {
        std::cerr << "== TWO KARTS ARE BEING GIVEN THE SAME POSITION!! ==\n";
        for (unsigned int j=0; j < m_position_index.size(); j++)
        {
            if (!m_position_used[j])
            {
                std::cout << "    No kart is yet set at position " << j << std::endl;
            }
            else
            {
                std::cout << "    Kart " << m_position_index[j] 
                          << " is at position " << j << std::endl;
            }
        }
        std::cout << "Kart " << kart_id << " is being given position " 
                  << position << ", but this position is already taken\n";
        return false;
    }
    m_position_used[position-1] = true;
#endif
    return true;
}   // setKartPosition

//-----------------------------------------------------------------------------
/** Called once the last position was set. Note that we should not test
 *  if all positions were set, since e.g. for eliminated and finished karts
 *  the position won't be set anymore.
 */
void WorldWithRank::endSetKartPositions()
{
    assert(m_position_setting_initialised);
    m_position_setting_initialised = false;
}   // endSetKartPositions


