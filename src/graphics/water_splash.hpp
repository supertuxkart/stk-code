//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009  Joerg Henrichs
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

#ifndef HEADER_WATER_SPLASH_HPP
#define HEADER_WATER_SPLASH_HPP

#include "utils/no_copy.hpp"

#include "irrlicht.h"
using namespace irr;

class Kart;

/**
  * \brief Handles a water-splash particle effect
  * \ingroup graphics
  */
class WaterSplash : public NoCopy
{
private:
    /** The kart to which this smoke belongs. */
    const Kart                      *m_kart;
    /** Irrlicht's particle systems. */
    scene::IParticleSystemSceneNode *m_node; /* left wheel */
    /** The emitters. Access to these is needed to adjust the number of
     *  particles per second. */
    scene::IParticleEmitter         *m_emitter;
    /** Size of the particles. */
    const float                      m_particle_size;
public:
                 WaterSplash    (Kart* kart);
    virtual     ~WaterSplash();
    virtual void update         (float t);
    void         setCreationRate(float f);
};   // WaterSplash
#endif

