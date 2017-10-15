//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015  Joerg Henrichs, Marianne Gagnon
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
#include <SColor.h>
using namespace irr;

class Material;

enum EmitterShape
{
    EMITTER_POINT,
    EMITTER_SPHERE,
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

    int       m_angle_spread;

    float     m_velocity_x;
    float     m_velocity_y;
    float     m_velocity_z;

    EmitterShape m_shape;

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

    /** For sphere emitters only */
    float m_sphere_radius;

    int m_emission_decay_rate;

    bool m_flips;

    std::string m_name;

    std::string m_material_file;

    bool m_has_scale_affector;
    float m_scale_affector_factor_x;
    float m_scale_affector_factor_y;

    /** The particle's billboards should face the
        player by rotating around the Y axis only */
    bool m_vertical_particles;

    /** Used mainly for weather, like snow */
    bool m_randomize_initial_y;

public:

    /**
      * @brief Load a XML file describing a type of particles
      * @param file Name of the file to load (no full path)
      * @throw std::runtime_error If the file cannot be found or is heavily malformed
      */
    ParticleKind(const std::string &file);
    virtual     ~ParticleKind() {}


    float     getMaxSize     () const { return m_max_size;        }
    float     getMinSize     () const { return m_min_size;        }

    int       getMinRate     () const { return m_min_rate;        }
    int       getMaxRate     () const { return m_max_rate;        }

    EmitterShape getShape    () const { return m_shape;           }

    Material* getMaterial    () const;

    int       getMaxLifetime () const { return m_lifetime_max;    }
    int       getMinLifetime () const { return m_lifetime_min;    }

    int       getFadeoutTime () const { return m_fadeout_time;    }

    video::SColor getMinColor() const { return m_min_start_color; }
    video::SColor getMaxColor() const { return m_max_start_color; }

    float     getBoxSizeX    () const { return m_box_x;           }
    float     getBoxSizeY    () const { return m_box_y;           }
    float     getBoxSizeZ    () const { return m_box_z;           }

    float     getSphereRadius() const { return m_sphere_radius;    }

    int       getAngleSpread () const { return m_angle_spread;    }

    float     getVelocityX   () const { return m_velocity_x;      }
    float     getVelocityY   () const { return m_velocity_y;      }
    float     getVelocityZ   () const { return m_velocity_z;      }

    void      setBoxSizeXZ    (float x, float z) { m_box_x = x; m_box_z = z;   }

    int       getEmissionDecayRate() const { return m_emission_decay_rate; }

    bool      hasScaleAffector() const { return m_has_scale_affector; }
    float     getScaleAffectorFactorX() const { return m_scale_affector_factor_x; }
    float     getScaleAffectorFactorY() const { return m_scale_affector_factor_y; };

    bool      getFlips() const { return m_flips; }

    bool      isVerticalParticles() const { return m_vertical_particles; }

    bool      randomizeInitialY() const { return m_randomize_initial_y; }

    const std::string& getName() const { return m_name; }
};

#endif


