//  $Id: moving_texture.cpp 796 2006-09-27 07:06:34Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

MovingTexture::MovingTexture(core::matrix4 *matrix, const XMLNode &node)
             : m_matrix(matrix)
{
    m_dx = 0.0f;
    m_dy = 0.0f;
    core::vector3df v = m_matrix->getTranslation();
    m_x = v.X;
    m_y = v.Y;

    node.get("dx", &m_dx);
    node.get("dy", &m_dy);
}   // MovingTexture

//-----------------------------------------------------------------------------
MovingTexture::~MovingTexture()
{
}   // ~MovingTexture

//-----------------------------------------------------------------------------
void MovingTexture::update(float dt)
{
    m_x = m_x + dt*m_dx;
    m_y = m_y + dt*m_dy;
    if(m_x>1.0f) m_x = fmod(m_x, 1.0f);
    if(m_y>1.0f) m_y = fmod(m_y, 1.0f);

    m_matrix->setTextureTranslate(m_x, m_y);
}   // update


