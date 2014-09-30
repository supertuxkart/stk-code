//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2013 Steve Baker <sjbaker1@airmail.net>
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

#ifndef HEADER_EXPLOSION_HPP
#define HEADER_EXPLOSION_HPP

#include "graphics/hit_sfx.hpp"
#include "utils/no_copy.hpp"

namespace irr
{
    namespace scene { class IParticleSystemSceneNode;  }
}
using namespace irr;

class Vec3;
class SFXBase;
class ParticleEmitter;

const float explosion_time = 2.0f;

/**
  * \ingroup graphics
  */
class Explosion : public HitSFX
{
private:
    float            m_remaining_time;
    int              m_emission_frames;
    ParticleEmitter* m_emitter;

public:
         Explosion(const Vec3& coord, const char* explosion_sound, const char * particle_file );
        ~Explosion();
    bool updateAndDelete(float delta_t);
    bool hasEnded () { return  m_remaining_time <= -explosion_time;  }

} ;

#endif

/* EOF */
