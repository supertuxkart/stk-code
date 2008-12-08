//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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

#include "graphics/skid_marks.hpp"

#include "scene.hpp"
#include "karts/kart.hpp"
#include "utils/coord.hpp"


/** Initialises empty skid marks. */
SkidMarks::SkidMarks(const Kart& kart, float width) : m_kart(kart)
{
    m_width        = width;
    m_skid_state   = new ssgSimpleState();
    m_skid_state->ref();
    m_skid_state->enable(GL_BLEND);
    m_skid_marking = false;
    m_current      = -1;
}   // SkidMark

//-----------------------------------------------------------------------------
/** Removes all skid marks from the scene graph and frees the state. */
SkidMarks::~SkidMarks()
{
    reset();  // remove all skid marks
    ssgDeRefDelete(m_skid_state);
}   // ~SkidMarks

//-----------------------------------------------------------------------------
/** Removes all skid marks, called when a race is restarted.
 */
void SkidMarks::reset()
{
    for(unsigned int i=0; i<m_left.size(); i++)
    {
        scene->remove(m_left[i]);
        scene->remove(m_right[i]);
    }
    m_left.clear();
    m_right.clear();
    m_skid_marking = false;
    m_current      = -1;
}   // reset

//-----------------------------------------------------------------------------
/** Either adds to an existing skid mark quad, or (if the kart is skidding) 
 *  starts a new skid mark quad.
 *  \param dt Time step.
 */
void SkidMarks::update(float dt)
{
    if(m_skid_marking)
    {
        // Get raycast information
        // -----------------------
        const btWheelInfo::RaycastInfo &raycast_right = 
            m_kart.getVehicle()->getWheelInfo(2).m_raycastInfo;
        const btWheelInfo::RaycastInfo raycast_left = 
            m_kart.getVehicle()->getWheelInfo(3).m_raycastInfo;
        Vec3 delta = raycast_right.m_contactPointWS - raycast_left.m_contactPointWS;

        // We were skid marking, but not anymore (either because the 
        // wheels don't touch the ground, or the kart has stopped 
        // skidding). One special case: the physics force both wheels
        // on one axis to touch the ground. If only one wheel touches 
        // the ground, the 2nd one gets the same raycast result -->
        // delta is 0, and the behaviour is undefined. In this case 
        // just stop doing skid marks as well. 
        // ---------------------------------------------------------
        if(!raycast_right.m_isInContact || !m_kart.getControls().jump ||
            delta.length2()<0.0001)
        {
            m_skid_marking = false;
            m_left[m_current]->makeDList();
            m_right[m_current]->makeDList();
            return;
        }

        // We are still skid marking, so add the latest quad
        // -------------------------------------------------        

        delta.normalize();
        delta *= m_width;
        
        m_left [m_current]->add(raycast_left.m_contactPointWS,
                                raycast_left.m_contactPointWS + delta);
        m_right[m_current]->add(raycast_right.m_contactPointWS-delta,
                                raycast_right.m_contactPointWS);

        return;
    }
    // Currently no skid marking
    // -------------------------
    if(!m_kart.getControls().jump) return;   // no skidmarking
        
    // Start new skid marks
    // --------------------
    const btWheelInfo::RaycastInfo &raycast_right = 
        m_kart.getVehicle()->getWheelInfo(2).m_raycastInfo;  
    // No skidmarking if wheels don't have contact
    if(!raycast_right.m_isInContact) return;

    const btWheelInfo::RaycastInfo raycast_left = 
        m_kart.getVehicle()->getWheelInfo(3).m_raycastInfo;

    Vec3 delta = raycast_right.m_contactPointWS - raycast_left.m_contactPointWS;
    // Special case: only one wheel on one axis touches the ground --> physics 
    // set both wheels to touch the same spot --> delta = 0. In this case,
    // don't start skidmarks.
    if(delta.length2()<0.0001) return;
    delta.normalize();
    delta *= m_width;

    SkidMarkQuads *smq_left = new SkidMarkQuads(raycast_left.m_contactPointWS,
                                         raycast_left.m_contactPointWS + delta,
                                         m_skid_state);
    scene->add(smq_left);
    SkidMarkQuads *smq_right = new SkidMarkQuads(raycast_right.m_contactPointWS
                                                  - delta,
                                                  raycast_right.m_contactPointWS,
                                                  m_skid_state);
    scene->add(smq_right);
    m_current++;
    if(m_current>=stk_config->m_max_skidmarks)
        m_current = 0;
    if(m_current>=(int)m_left.size())
    {
        m_left. push_back(smq_left );
        m_right.push_back(smq_right);
    }
    else
    {
        scene->remove(m_left [m_current]);
        scene->remove(m_right[m_current]);
        m_left [m_current] = smq_left;
        m_right[m_current] = smq_right;
    }


    m_skid_marking = true;

}   // update

//=============================================================================
SkidMarks::SkidMarkQuads::SkidMarkQuads(const Vec3 &left, const Vec3 &right,
                                        ssgSimpleState *state)
        : ssgVtxTable(GL_QUAD_STRIP,
                      new ssgVertexArray,
                      new ssgNormalArray,
                      new ssgTexCoordArray,
                      new ssgColourArray )
{
    setState(state);
    add(left, right);
}   // SkidMarkQuads


//-----------------------------------------------------------------------------
void SkidMarks::SkidMarkQuads::recalcBSphere()
{
    bsphere.setRadius(1000.0f);
    bsphere.setCenter(0, 0, 0);
}   // recalcBSphere

//-----------------------------------------------------------------------------
void SkidMarks::SkidMarkQuads::add(const Vec3 &left, const Vec3 &right)
{
    // The skid marks must be raised slightly higher, otherwise it blends
    // too much with the track.
    sgVec3 l;
    sgCopyVec3(l, left.toFloat());
    l[2]+=0.01f;
    sgVec3 r;
    sgCopyVec3(r, right.toFloat());
    r[2]+=0.01f;
    vertices->add(l);
    vertices->add(r);
    sgVec3 normal;
    sgSetVec3(normal, 0, 0, 1);
    normals->add(normal);normals->add(normal);
    
    sgVec4 colour;
    sgSetVec4(colour, 0.07f, 0.07f, 0.07f, 0.8f);
    colours->add(colour); colours->add(colour);

    dirtyBSphere();
}   // add
