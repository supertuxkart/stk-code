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

#include "graphics/moving_texture.hpp"
#include "io/xml_node.hpp"

/** Constructor for an animated texture.
 *  \param matrix The texture matrix to modify.
 *  \param node An XML node containing dx and dy attributes to set the
 *         speed of the animation.
 */
MovingTexture::MovingTexture(core::matrix4 *matrix, const XMLNode &node)
             : m_matrix(matrix)
{
    m_dx = 0.0f;
    m_dy = 0.0f;
    m_dt = 0.0f;
    m_count = 0.0f;

    core::vector3df v = m_matrix->getTranslation();
    m_x = v.X;
    m_y = v.Y;

    // by default the animation by step is disabled
    m_isAnimatedByStep = false;

    node.get("dx", &m_dx);
    node.get("dy", &m_dy);
    node.get("dt", &m_dt);

    node.get("animByStep", &m_isAnimatedByStep);
}   // MovingTexture

//-----------------------------------------------------------------------------
/** Constructor for an animated texture, specifying the speed of the animation
 *  directly.
 *  \param matrix The texture matrix to modify.
 *  \param dx Speed of the animation in X direction.
 *  \param dy Speed of the animation in Y direction.
 */
MovingTexture::MovingTexture(core::matrix4 *matrix, float dx, float dy)
             : m_matrix(matrix)
{
    // by default the animation by step is disabled
    m_isAnimatedByStep = false;

    m_dx = dx;
    m_dy = dy;
    core::vector3df v = m_matrix->getTranslation();
    m_x = v.X;
    m_y = v.Y;
}   // MovingTexture

//-----------------------------------------------------------------------------
MovingTexture::MovingTexture(float dx, float dy)
{
    // by default the animation by step is disabled
    m_isAnimatedByStep = false;

    m_dx     = dx;
    m_dy     = dy;
    m_x      = 0;
    m_y      = 0;
    m_matrix = NULL;
}   // MovingTexture

//-----------------------------------------------------------------------------
/** Destructor for an animated texture.
 */
MovingTexture::~MovingTexture()
{
}   // ~MovingTexture

//-----------------------------------------------------------------------------
/** Resets at (re)start of a race.
 */
void MovingTexture::reset()
{
    m_x = m_y = 0;
    m_matrix->setTextureTranslate(m_x, m_y);
}   // reset

//-----------------------------------------------------------------------------
/** Updates the transform of an animated texture.
 *  \param dt Time step size.
 */
void MovingTexture::update(float dt)
{

    if (m_isAnimatedByStep)
    {
        m_count += dt;
        if(m_count > m_dt)
        {
            m_count -= m_dt;

            m_x = m_x + 1.0f*m_dx;
            m_y = m_y + 1.0f*m_dy;
            if(m_x>1.0f) m_x = fmod(m_x, 1.0f);
            if(m_y>1.0f) m_y = fmod(m_y, 1.0f);

            m_matrix->setTextureTranslate(m_x, m_y);
        }
    }
    else
    {
        m_x = m_x + dt*m_dx;
        m_y = m_y + dt*m_dy;
        if(m_x>1.0f) m_x = fmod(m_x, 1.0f);
        if(m_y>1.0f) m_y = fmod(m_y, 1.0f);
        m_matrix->setTextureTranslate(m_x, m_y);
    }
}   // update


