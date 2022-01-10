//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2012-2015  SuperTuxKart-Team
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

#ifndef HEADER_STARS_HPP
#define HEADER_STARS_HPP

#include "utils/no_copy.hpp"

#include "vector3d.h"

#include <vector>

class AbstractKart;

namespace irr
{
    namespace scene { class ISceneNode; }
}
using namespace irr;

/**
  * \brief This class is used to display rotating stars around a kart's head.
  * \ingroup graphics
  */
class Stars : public NoCopy
{
private:

    /** Vector containing the stars */
    std::vector<scene::ISceneNode*> m_nodes;

    /** The scene node of the kart to which the stars belong. */
    scene::ISceneNode  *m_parent_kart_node;

    /** Center around which stars rotate */
    core::vector3df m_center;

    /** Whether stars are currently enabled */
    bool m_enabled;

    float m_period;
    float m_remaining_time;

 public:
           Stars  (AbstractKart *kart);
          ~Stars  ();
    void   showFor(float time);
    void   reset();
    void   update (float delta_t);
    bool   isEnabled() const { return m_enabled; }
};
#endif

/* EOF */


