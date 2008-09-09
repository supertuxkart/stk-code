//  $Id: randum_number.cpp 2163 2008-07-14 03:40:58Z hikerstk $
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

#include "random_generator.hpp"

#include <stdlib.h>

std::vector<RandomGenerator*> RandomGenerator::m_all_random_generators;

RandomGenerator::RandomGenerator()
{
    m_a = 1103515245;
    m_c = 12345;    
    m_all_random_generators.push_back(this);
}   // RandomGenerator

// ----------------------------------------------------------------------------
std::vector<int> RandomGenerator::generateAllSeeds()
{
    std::vector<int> all_seeds;
    for(unsigned int i=0; i<m_all_random_generators.size(); i++)
    {
        int seed = rand();
        all_seeds.push_back(seed);
    }
    seedAll(all_seeds);
    return all_seeds;

}   // generateAllSeeds
// ----------------------------------------------------------------------------
void RandomGenerator::seedAll(std::vector<int> all_seeds)
{
    for(unsigned int i=0; i<all_seeds.size(); i++)
    {
        m_all_random_generators[i]->seed(all_seeds[i]);
    }
}   // seed

// -------------------q---------------------------------------------------------
void RandomGenerator::seed(int s)
{
    m_random_value = s;
}   // seed

// ----------------------------------------------------------------------------
int RandomGenerator::get(int n)
{
    m_random_value = m_random_value*m_a+m_c;
    return m_random_value % n;
}   // get

// ----------------------------------------------------------------------------

