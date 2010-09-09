//  $Id: slip_stream.hpp 1681 2008-04-09 13:52:48Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010  Joerg Henrichs
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

#ifndef HEADER_SLIP_STREAM_HPP
#define HEADER_SLIP_STREAM_HPP

#include "irrlicht.h"
using namespace irr;

#include "graphics/moving_texture.hpp"
#include "utils/no_copy.hpp"

class Kart;

/**
  * \ingroup graphics
  */
class SlipStream : public MovingTexture
{
private:
    /** The kart to which this smoke belongs. */
    Kart              *m_kart;

    /** The scene node. */
    scene::ISceneNode *m_node;

    /** The actual mesh. */
    scene::IMesh      *m_mesh;

    /** The texture matrix for the slipstream effect. */
    core::matrix4     *m_matrix;

    /** The length of the slipstream cylinder. This is used to scale
     *  the actual scene node correctly. */
    float              m_length;

    void         createMesh(const video::SMaterial &m);
public:
                 SlipStream  (Kart* kart);
    virtual     ~SlipStream  ();
    virtual void update(float dt);
    void         setIntensity(float f, const Kart* kart);
};   // SlipStream
#endif

