//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007-2015 Joerg Henrichs, Marianne Gagnon
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

#ifndef HEADER_BOWLING_HPP
#define HEADER_BOWLING_HPP

namespace irr
{
    namespace scene { class IMesh; }
}
#include <irrString.h>
using namespace irr;

#include "items/flyable.hpp"

class XMLNode;
class SFXBase;

/**
  * \ingroup items
  */
class Bowling : public Flyable
{
private:
    static float m_st_max_distance;   // maximum distance for a bowling ball to be attracted
    static float m_st_max_distance_squared;
    static float m_st_force_to_target;

    /** If a bowling ball has hit something, this flag is set to indicate
     *  if a kart was hit or not. The sound effect is only played if a
     *  kart was hit. */
    bool m_has_hit_kart;

    /** A sound effect for rolling ball. */
    SFXBase     *m_roll_sfx;
    void removeRollSfx();

public:
             Bowling(AbstractKart* kart);
    virtual ~Bowling();
    static  void init(const XMLNode &node, scene::IMesh *bowling);
    virtual bool updateAndDelete(int ticks) OVERRIDE;
    virtual bool hit(AbstractKart* kart, PhysicalObject* obj=NULL) OVERRIDE;
    virtual HitEffect *getHitEffect() const OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void onFireFlyable() OVERRIDE;

};   // Bowling

#endif
