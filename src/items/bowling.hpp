//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Joerg Henrichs, Marianne Gagnon
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

/**
  * \ingroup items
  */
class Bowling : public Flyable
{
private:
    static float m_st_max_distance;   // maximum distance for a bowling ball to be attracted
    static float m_st_max_distance_squared;
    static float m_st_force_to_target;
    
public:
    Bowling(Kart* kart);
    static  void init(const XMLNode &node, scene::IMesh *bowling);
    virtual bool updateAndDelete(float dt);
    virtual const core::stringw getHitString(const Kart *kart) const;
    virtual bool hit(Kart* kart, PhysicalObject* obj=NULL);

    /** Returns the sfx to use when the bowling ball explodes. */
    const char* getExplosionSound() const { return "strike"; }
    
};   // Bowling

#endif
