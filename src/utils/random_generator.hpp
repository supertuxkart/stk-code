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

#ifndef HEADER_RANDOM_GENERATOR_HPP
#define HEADER_RANDOM_GENERATOR_HPP

#include <random>
#include <vector>

/** A random number generator. Each objects that needs a random number uses
    its own number random generator. They are all seeded with number provided
    by the server. This guarantees that in a network game all 'random' values
    are actually identical among all machines.
    The formula used is x(n+1)=(a*x(n)+c) % m, but m is assumed to be 2^32,
    so the modulo operation can be skipped (for 4 byte integers).
 */
class RandomGenerator
{
private:
   static unsigned int m_random_value;
   static constexpr unsigned int default_seed = 3141591;
#ifdef NOT_USED_ATM
   unsigned int m_a = 1103515245;
   unsigned int m_c = 12345;
#endif
//    static std::vector<RandomGenerator*> m_all_random_generators;

public:
    RandomGenerator() = default;
    /** Return a reference to the thread-local generator */
    static std::mt19937& getGenerator();
    //std::vector<int> generateAllSeeds();
    /** Returns a pseudo random number between 0 and n-1 inclusive */
    int get(int n)  {
       std::uniform_int_distribution<int> dist(0, n - 1);
       return dist(getGenerator());
    }
    static void seed(int s) {m_random_value = s;}
};  // RandomGenerator

#endif // HEADER_RANDOM_GENERATOR_HPP
