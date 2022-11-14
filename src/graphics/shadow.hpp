//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013  Joerg Henrichs
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

#ifndef HEADER_SHADOW_HPP
#define HEADER_SHADOW_HPP

#include "utils/no_copy.hpp"
#include <memory>

class AbstractKart;
class Material;

namespace SP
{
    class SPDynamicDrawCall;
}

namespace irr
{
    namespace scene
    {
        class IMeshSceneNode;
    }
}

/**
 * \brief This class is used to enable a shadow for a kart.
 * For now it uses a simple texture to simulate the shadow, real time shadows might
 * be added later.
 * \ingroup graphics
 */
class Shadow : public NoCopy
{
private:
    /** The dynamic draw call of the shadow. */
    std::shared_ptr<SP::SPDynamicDrawCall> m_dy_dc;

    irr::scene::IMeshSceneNode* m_node;

    /** If a kart is flying, the shadow is disabled (since it is
     *  stuck to the kart, i.e. the shadow would be flying, too). */
    bool             m_shadow_enabled;

    /** A read-only kart object for accessing suspension length. */
    const AbstractKart& m_kart;

public:
         Shadow(Material* shadow_mat, const AbstractKart& kart);
        ~Shadow();
    void update(bool enabled);
};   // Shadow
#endif

/* EOF */

