//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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

#ifndef MOVING_TEXTURE_HPP
#define MOVING_TEXTURE_HPP

#include "utils/no_copy.hpp"
#include <string>
#include <matrix4.h>
using namespace irr;

class XMLNode;

/**
  * \brief Handles animated textures (textures that move)
  * \ingroup graphics
  */
class MovingTexture
{
private:
    /** Translation increment per second. */
    float                m_dx, m_dy;

    /** Delta set by user and count */
    float                m_dt;
    float                m_count;
    bool                 m_isAnimatedByStep;

    /** Current x,y position. */
    float                m_x, m_y;
    /** The texture matrix of this texture. */
    core::matrix4       *m_matrix;

    // For sp
    float* m_sp_tm;

public:
                 MovingTexture(core::matrix4 *matrix, const XMLNode &node);
                 MovingTexture(core::matrix4 *matrix, float dx, float dy);
                 MovingTexture(float dx, float dy, float dt = 0.0f,
                               bool animated_by_step = false);
    virtual     ~MovingTexture();

    /** Sets the speed of the animation. */
    void         setSpeed(float dx, float dy) {m_dx = dx; m_dy = dy;}
    /** Sets the texture matrix. */
    void         setTextureMatrix(core::matrix4 *matrix) {m_matrix=matrix;}
    void setSPTM(float* sp_tm) { m_sp_tm = sp_tm; }
    virtual void update  (float dt);
    virtual void reset   ();
    float        getCurrentX() const { return m_x; }
    float        getCurrentY() const { return m_y; }
}
;   // MovingTexture

#endif

