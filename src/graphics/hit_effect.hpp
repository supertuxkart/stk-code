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

#ifndef HEADER_HIT_EFFECT_HPP
#define HEADER_HIT_EFFECT_HPP

#include "utils/no_copy.hpp"

class Vec3;

/**
 *  \ingroup graphics
 *  A small interface for effects to be used when  a kart is hit. That
 *  includes a sound effect only, or a graphical effect (like an
 *  explosion).
 */
class HitEffect: public NoCopy
{
private:
    /** True if this effect affected a player kart. Used to play certain SFX
     *  less loud if only an AI is hit. */
    bool m_local_player_kart_hit;

public:
                 /** Constructor for a hit effect. */
                 HitEffect() {m_local_player_kart_hit = false; }
    virtual     ~HitEffect() {}
    /** Updates a hit effect. Called once per frame.
     *  \param dt Time step size.
     *  \return True if the hit effect is finished and can be removed. */
    virtual bool updateAndDelete(int ticks) = 0;

    // ------------------------------------------------------------------------
    /** Sets that this SFX affects a player kart, which can be used to
     *  make certain sfx louder/less loud. Default is that the affect
     *  does not affect a player kart. */
    virtual void setLocalPlayerKartHit() { m_local_player_kart_hit = true; }
    // ------------------------------------------------------------------------
    /** Returns if this effect affects a player kart. */
    bool getLocalPlayerKartHit() const { return m_local_player_kart_hit; }
};   // HitEffect

#endif

/* EOF */
