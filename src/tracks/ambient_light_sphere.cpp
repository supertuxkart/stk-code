//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015  Joerg Henrichs
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

#include "tracks/ambient_light_sphere.hpp"

#include <string>
#include <stdio.h>

#include "graphics/camera.hpp"
#include "io/xml_node.hpp"
#include "karts/abstract_kart.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"

/** Constructor for a checksphere.
 *  \param node XML node containing the parameters for this checkline.
 */
AmbientLightSphere::AmbientLightSphere(const XMLNode &node,
                                       unsigned int index)
                  : CheckSphere(node, index)
{
    m_ambient_color = video::SColor(255, 0, 255, 0);   // green
    m_inner_radius2 = 1;
    node.get("inner-radius", &m_inner_radius2);
    m_inner_radius2 *= m_inner_radius2;   // store the squared value
    node.get("color", &m_ambient_color);
}   // AmbientLightSphere

// ----------------------------------------------------------------------------
void AmbientLightSphere::update(float dt)
{
    CheckStructure::update(dt);

    for(unsigned int i=0; i<Camera::getNumCameras(); i++)
    {
        Camera *camera = Camera::getCamera(i);
        const AbstractKart *kart=camera->getKart();
        if(!kart) continue;
        if(isInside(i))
        {
            float d2=getDistance2ForKart(i);
            video::SColor color;
            Track *track=Track::getCurrentTrack();
            if(d2<m_inner_radius2)
            {   // Inside inner radius --> use new ambient color
                color = m_ambient_color;
            }
            else   //  Interpolate between default and this ambient color
            {
                float f = (getRadius2()-d2)/(getRadius2()-m_inner_radius2);
                const video::SColor &def = track->getDefaultAmbientColor();
                color = m_ambient_color.getInterpolated(def, f);
            }
            camera->setAmbientLight(color);
        }   // if active
    }   // for i<num_karts
}   // update

// ----------------------------------------------------------------------------
/** Only calls the sphere check if the kart is a player kart, since other
 *  karts won't change the ambient light.
 *  \param old_pos  Position in previous frame.
 *  \param new_pos  Position in current frame.
 *  \param indx     Index of the kart, can be used to store kart specific
 *                  additional data.
 */
bool AmbientLightSphere::isTriggered(const Vec3 &old_pos, const Vec3 &new_pos,
                                     unsigned int indx)
{
    for(unsigned int i=0; i<Camera::getNumCameras(); i++)
    {
        if(Camera::getCamera(i)->getKart()->getWorldKartId()==indx)
            return CheckSphere::isTriggered(old_pos, new_pos, indx);
    }
    return false;
}   // isTriggered
