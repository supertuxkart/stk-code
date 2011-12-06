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

#ifndef HEADER_BILLBOARD_ANIMATION_HPP
#define HEADER_BILLBOARD_ANIMATION_HPP

#include <string>

#include "animations/animation_base.hpp"

class XMLNode;

/**
  * \brief A 2d billboard animation.
  * \ingroup animations
  */
class BillboardAnimation : public AnimationBase
{
    /** To create halo-like effects where the halo disapears when you get
        close to it. Requested by samuncle */
    bool m_fade_out_when_close;
    float m_fade_out_start;
    float m_fade_out_end;
    
public:
             BillboardAnimation(const XMLNode &node);
    virtual ~BillboardAnimation() {};
    virtual void update(float dt);

};   // BillboardAnimation

#endif

