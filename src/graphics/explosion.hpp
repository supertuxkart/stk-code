//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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
#include "utils/cpp2011.hpp"
#include "utils/no_copy.hpp"

namespace irr
{
    namespace scene { class IParticleSystemSceneNode;  }
}
using namespace irr;

class Vec3;
class SFXBase;
class ParticleEmitter;

/**
  * \ingroup graphics
  */
class Explosion : public HitSFX
{
private:
    int              m_remaining_ticks;
    int              m_emission_frames;
    ParticleEmitter* m_emitter;
    int              m_explosion_ticks;


public:
         Explosion(const Vec3& coord, const char* explosion_sound, const char * particle_file );
        ~Explosion();
    bool updateAndDelete(int ticks) OVERRIDE;
    bool hasEnded () 
    {
        return  m_remaining_ticks <= -m_explosion_ticks; 
    }

} ;

#endif

/* EOF */
