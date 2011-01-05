//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011  Joerg Henrichs, Marianne Gagnon
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

#ifndef HEADER_PARTICLE_KIND_HPP
#define HEADER_PARTICLE_KIND_HPP

#include "utils/no_copy.hpp"

#include <string>
#include "irrlicht.h"
using namespace irr;

class Material;

enum EmitterShape
{
    EMITTER_POINT,
    EMITTER_BOX
};

/**
 * \brief type of particles
 * \ingroup graphics
 */
class ParticleKind : public NoCopy
{
private:
    
    /** Size of the particles. */
    //float     m_particle_size;
    float     m_max_size;
    float     m_min_size;
    
    float     m_spread_factor;

    EmitterShape m_shape;
    
    Material* m_material;
    
    /** Minimal emission rate in particles per second */
    int       m_min_rate;
    
    /** Maximal emission rate in particles per second */
    int       m_max_rate;
    
    int       m_lifetime_min;
    int       m_lifetime_max;
    
    int       m_fadeout_time;
    
    video::SColor m_min_start_color;
    video::SColor m_max_start_color;
    
    /** For box emitters only */
    float m_box_x, m_box_y, m_box_z;
    
public:
    
    ParticleKind(const std::string file);
    virtual     ~ParticleKind() {}

    
    //float     getParticleSize() const { return m_particle_size;   }
    float     getMaxSize     () const { return m_max_size;        }
    float     getMinSize     () const { return m_min_size;        }
    
    int       getMinRate     () const { return m_min_rate;        }
    int       getMaxRate     () const { return m_max_rate;        }
    
    float     getSpreadFactor() const { return m_spread_factor;   }
    
    EmitterShape getShape    () const { return m_shape;           }
    
    Material* getMaterial    () const { return m_material;        }

    int       getMaxLifetime () const { return m_lifetime_max;    }
    int       getMinLifetime () const { return m_lifetime_min;    }
    
    int       getFadeoutTime () const { return m_fadeout_time;    }
    
    video::SColor getMinColor() const { return m_min_start_color; }
    video::SColor getMaxColor() const { return m_max_start_color; }
    
    float     getBoxSizeX    () const { return m_box_x;           }
    float     getBoxSizeY    () const { return m_box_y;           }
    float     getBoxSizeZ    () const { return m_box_z;           }
};
#endif


