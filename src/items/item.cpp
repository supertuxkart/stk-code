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

#include "items/item.hpp"

#include "graphics/scene.hpp"
#include "karts/kart.hpp"
#include "utils/coord.hpp"
#include "utils/vec3.hpp"

Item::Item(ItemType type, const Vec3& xyz, const Vec3& normal,
           ssgEntity* model, unsigned int item_id, bool rotate)
{
    m_rotate           = rotate;
    m_parent           = NULL;
    m_deactive_time    = 0;
    // Sets heading to 0, and sets pitch and roll depending on the normal. */
    Vec3  hpr          = Vec3(0, normal);
    m_coord            = Coord(xyz, hpr);
    m_item_id          = item_id;
    m_type             = type;
    m_collected        = false;
    m_time_till_return = 0.0f;  // not strictly necessary, see isCollected()
    m_root             = new ssgTransform();
    m_root->ref();
    m_root->setTransform(const_cast<sgCoord*>(&m_coord.toSgCoord()));
    m_root->addKid(model);
    scene->add(m_root);
}   // Item

//-----------------------------------------------------------------------------
Item::~Item()
{
    scene->remove(m_root);
    ssgDeRefDelete(m_root);
}   // ~Item

//-----------------------------------------------------------------------------
void Item::reset()
{
    m_collected        = false;
    m_time_till_return = 0.0f;
    m_deactive_time    = 0.0f;
    m_root->setTransform(const_cast<sgCoord*>(&m_coord.toSgCoord()));
}   // reset
//-----------------------------------------------------------------------------
void Item::setParent(Kart* parent)
{
    m_parent        = parent;
    m_deactive_time = 1.5f;
}

//-----------------------------------------------------------------------------
void Item::update(float delta)
{
    if(m_parent != NULL && m_deactive_time > 0) m_deactive_time -= delta;
    
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
/** Is called when the item is hit by a kart.  It sets the flag that the item
 *  has been collected, and the time to return to the parameter. 
 *  \param t Time till the object reappears (defaults to 2 seconds).
 */
void Item::isCollected(float t)
{
    m_collected        = true;
    // Note if the time is negative, in update the m_collected flag will
    // be automatically set to false again.
    m_time_till_return = t;
}

