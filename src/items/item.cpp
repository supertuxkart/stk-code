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
#include "items/item.hpp"
#include "kart.hpp"
#include "scene.hpp"
#include "coord.hpp"

Item::Item(ItemType type, const Vec3& xyz, ssgEntity* model,
                 unsigned int item_id) 
        : m_coord(xyz, Vec3(0, 0, 0))
{
    m_item_id          = item_id;
    m_type             = type;
    m_collected        = false;
    m_time_till_return = 0.0f;  // not strictly necessary, see isCollected()
    m_root             = new ssgTransform();
    m_root->ref();
    m_root->setTransform(const_cast<sgCoord*>(&m_coord.toSgCoord()));
    m_root->addKid(model);
    scene->add(m_root);
    m_rotate = true;
    
    m_parent = NULL;
    m_immunity_timer = 0;

}   // Item

//-----------------------------------------------------------------------------
Item::~Item()
{
    ssgDeRefDelete(m_root);
}   // ~Item

//-----------------------------------------------------------------------------
void Item::reset()
{
    m_collected        = false;
    m_time_till_return = 0.0f;
    m_root->setTransform(const_cast<sgCoord*>(&m_coord.toSgCoord()));
}   // reset
//-----------------------------------------------------------------------------
void Item::setParent(Kart* parent)
{
    m_parent = parent;
    m_immunity_timer = 1.5f;
}
//-----------------------------------------------------------------------------
bool Item::hitKart(Kart* kart)
{
    if(m_immunity_timer > 0) return false;
    
    return (kart->getXYZ()-m_coord.getXYZ()).length2()<0.8f;
}   // hitKart

//-----------------------------------------------------------------------------
void Item::update(float delta)
{
    if(m_parent != NULL && m_immunity_timer > 0) m_immunity_timer -= delta;
    
    if(m_collected)
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
            m_collected    = false;
            m_coord.setHPR(Vec3(0.0f));
            m_root->setTransform(const_cast<sgCoord*>(&m_coord.toSgCoord()));
        }   // T>0

    }
    else
    {   // not m_collected
        
        if(!m_rotate) return;
        // have it rotate
        Vec3 rotation(delta*M_PI, 0, 0);
        m_coord.setHPR(m_coord.getHPR()+rotation);
        m_root->setTransform(const_cast<sgCoord*>(&m_coord.toSgCoord()));
    }
}   // update

//-----------------------------------------------------------------------------
void Item::isCollected()
{
    m_collected            = true;
    m_time_till_return = 2.0f;
}

