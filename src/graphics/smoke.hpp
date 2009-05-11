//  $Id: dust_cloud.hpp 1681 2008-04-09 13:52:48Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#ifndef HEADER_SMOKE_H
#define HEADER_SMOKE_H

#include "particle_system.hpp"

class Kart;

class Smoke : public ParticleSystem
{
private:
    /** The kart to which this smoke belongs. */
    Kart           *m_kart;
    /** The texture to use. */
    //ssgSimpleState *m_smokepuff;
public:
                 Smoke          (Kart* kart);
                ~Smoke          ();
    virtual void update         (float t                                        );
    virtual void particle_create(int index, Particle* p                         );
    virtual void particle_update(float deltaTime, int index, Particle *p        );
};
#endif

