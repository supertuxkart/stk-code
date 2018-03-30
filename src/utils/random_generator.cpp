//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2015  Joerg Henrichs
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

#include "utils/random_generator.hpp"

#include <stdlib.h>
#include <ctime>

//std::vector<RandomGenerator*> RandomGenerator::m_all_random_generators;

RandomGenerator::RandomGenerator()
{
    m_a = 1103515245;
    m_c = 12345;
    //m_all_random_generators.push_back(this);
    m_random_value = 3141591;
}   // RandomGenerator

// ----------------------------------------------------------------------------
#if 0
std::vector<int> RandomGenerator::generateAllSeeds()
{
    std::vector<int> all_seeds;
    for(unsigned int i=0; i<m_all_random_generators.size(); i++)
    {
        int seed = rand();
        all_seeds.push_back(seed);
        m_all_random_generators[i]->seed(seed);
    }
    return all_seeds;
}   // generateAllSeeds


// ----------------------------------------------------------------------------
int RandomGenerator::get(int n)
{
    // This generator is (currently) not good enough, i.e. it often gives
    // long sequences of same numbers. And the seeding is not done (the
    // mid term goal is to synchronise all random number generators on
    // client and server to make less communication necessary).
    // For now: just use standard random numbers:
    return rand() % n;
#ifdef NOT_USED_ATM
    m_random_value = m_random_value*m_a+m_c;
    // Note: the lower bits can have a very short cycle, e.g. for n = 4 the
    // cycle length is 4, meaning that the same sequence 1,2,3,4 is repeated
    // over and over again. The higher bits are more random, so the lower
    // 8 bits are discarded.
    return (m_random_value >> 8) % n;
#endif
}   // get

#endif // if 0
