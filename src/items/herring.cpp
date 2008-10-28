//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

#include "vec3.hpp"
#include "items/herring.hpp"
#include "kart.hpp"
#include "scene.hpp"
#include "coord.hpp"

Herring::Herring(herringType type, const Vec3& xyz, ssgEntity* model,
                 unsigned int herring_id) 
        : m_coord(xyz, Vec3(0, 0, 0))
{
    m_herring_id       = herring_id;
    m_type             = type;
    m_eaten            = false;
    m_time_till_return = 0.0f;  // not strictly necessary, see isEaten()
    m_root             = new ssgTransform();
    m_root->ref();
    m_root->setTransform(const_cast<sgCoord*>(&m_coord.toSgCoord()));
    m_root->addKid(model);
    scene->add(m_root);

}   // Herring

//-----------------------------------------------------------------------------
Herring::~Herring()
{
    ssgDeRefDelete(m_root);
}   // ~Herring

//-----------------------------------------------------------------------------
void Herring::reset()
{
    m_eaten            = false;
    m_time_till_return = 0.0f;
    m_root->setTransform(const_cast<sgCoord*>(&m_coord.toSgCoord()));
}   // reset

//-----------------------------------------------------------------------------
int Herring::hitKart(Kart* kart)
{
    return (kart->getXYZ()-m_coord.getXYZ()).length2()<0.8f;
}   // hitKart

//-----------------------------------------------------------------------------
void Herring::update(float delta)
{
    if(m_eaten)
    {
        m_time_till_return -= delta;
        if ( m_time_till_return > 0 )
        {
            Vec3 hell(m_coord.getXYZ());

            hell.setZ( (m_time_till_return>1.0f) ? -1000000.0f 
		       : m_coord.getXYZ().getZ() - m_time_till_return / 2.0f);
            m_root->setTransform(hell.toFloat());
        }
        else
        {
            m_eaten    = false;
            m_coord.setHPR(Vec3(0.0f));
            m_root->setTransform(const_cast<sgCoord*>(&m_coord.toSgCoord()));
        }   // T>0

    }
    else
    {   // not m_eaten
        Vec3 rotation(delta*M_PI, 0, 0);
        m_coord.setHPR(m_coord.getHPR()+rotation);
        m_root->setTransform(const_cast<sgCoord*>(&m_coord.toSgCoord()));
    }
}   // update

//-----------------------------------------------------------------------------
void Herring::isEaten()
{
    m_eaten            = true;
    m_time_till_return = 2.0f;
}

