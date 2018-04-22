//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2015 Joerg Henrichs
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

#ifndef HEADER_HIT_SFX_HPP
#define HEADER_HIT_SFX_HPP

#include "graphics/hit_effect.hpp"
#include "utils/cpp2011.hpp"

class SFXBase;

/**
  * \ingroup graphics
  */
class HitSFX : public HitEffect
{
private:
    /** The sfx to play. */
    SFXBase*       m_sfx;

public:
         HitSFX(const Vec3& coord, const char* explosion_sound);
        ~HitSFX();
    virtual bool updateAndDelete(int ticks) OVERRIDE;
    virtual void setLocalPlayerKartHit() OVERRIDE;

};   // HitSFX

#endif

/* EOF */
