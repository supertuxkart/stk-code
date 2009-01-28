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
#include "physics/btKart.hpp"
#include "utils/coord.hpp"

float SkidMarks::m_avoid_z_fighting  = 0.0f;
const float SkidMarks::m_start_alpha = 0.5f;

/** Initialises empty skid marks. */
SkidMarks::SkidMarks(const Kart& kart, float width) : m_kart(kart)
{
    m_width        = width;
    m_skid_state   = new ssgSimpleState();
    m_skid_state->ref();
    m_skid_state->enable(GL_BLEND);
    m_skid_state->setMaterial(GL_SPECULAR, 0.0f, 0.0f, 0.0f, 1.0f);
    m_skid_state->setShininess(0.0f);
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
        stk_scene->remove(m_left[i]);
        stk_scene->remove(m_right[i]);
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
    float f = dt/stk_config->m_skid_fadeout_time;
    for(int i=0; i<m_current; i++)
    {
        m_left[i]->fade(f);
        m_right[i]->fade(f);
    }

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
        if(!raycast_right.m_isInContact || !m_kart.getControls().m_drift ||
            fabsf(m_kart.getControls().m_steer) < 0.001f                 ||
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
    if((!m_kart.getControls().m_drift) || (fabsf(m_kart.getControls().m_steer) < 0.001f)) return;   // no skidmarking
        
    // not turning enough, don't draw skidmarks if kart is going straight ahead
    // this is even stricter for Ai karts, since they tend to use LOTS of skidding
    const float min_skid_angle = m_kart.isPlayerKart() ? 0.55f : 0.95f;
    if( fabsf(m_kart.getSteerPercent()) < min_skid_angle) return;
    
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

    m_avoid_z_fighting += 0.001f;
    if(m_avoid_z_fighting>0.01f) m_avoid_z_fighting = 0.0f;

    SkidMarkQuads *smq_left = new SkidMarkQuads(raycast_left.m_contactPointWS,
                                         raycast_left.m_contactPointWS + delta,
                                         m_skid_state, m_avoid_z_fighting);
    stk_scene->add(smq_left);

    m_avoid_z_fighting += 0.001f;
    if(m_avoid_z_fighting>0.01f) m_avoid_z_fighting = 0.0f;
    SkidMarkQuads *smq_right = new SkidMarkQuads(raycast_right.m_contactPointWS
                                                  - delta,
                                                  raycast_right.m_contactPointWS,
                                                  m_skid_state,
                                                  m_avoid_z_fighting);
    stk_scene->add(smq_right);
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
        stk_scene->remove(m_left [m_current]);
        stk_scene->remove(m_right[m_current]);
        m_left [m_current] = smq_left;
        m_right[m_current] = smq_right;
    }


    m_skid_marking = true;

}   // update

//=============================================================================
SkidMarks::SkidMarkQuads::SkidMarkQuads(const Vec3 &left, const Vec3 &right,
                                        ssgSimpleState *state, float z_offset)
        : ssgVtxTable(GL_QUAD_STRIP,
                      new ssgVertexArray,
                      new ssgNormalArray,
                      new ssgTexCoordArray,
                      new ssgColourArray )
{
    m_z_offset = z_offset;
    m_fade_out = 0.0f;
    // If only one colours and/or normal is specified, it is used for
    // all fertices in the table.
    sgVec4 colour;
    sgSetVec4(colour, 0.00f, 0.00f, 0.00f, SkidMarks::m_start_alpha);
    colours->add(colour);
    sgVec3 normal;
    sgSetVec3(normal, 0, 0, 1);
    normals->add(normal);
    
    setState(state);
    m_aabb_min = Vec3(10000);
    m_aabb_max = Vec3(-10000);
    add(left, right);
}   // SkidMarkQuads


//-----------------------------------------------------------------------------
/** Fades the current skid marks. 
 *  \param f fade factor.
 */
void SkidMarks::SkidMarkQuads::fade(float f)
{
    m_fade_out += f;
    // Only actually change the alpha value every 0.1. Otherwise we can't use
    // display lists, which makes skid marks too slow
    if(m_fade_out>0.1f)
    {
        float *c=colours->get(0);
        c[3] -= m_fade_out;
        if(c[3]<0.0f) c[3]=0.0f;
        makeDList();
        m_fade_out = 0.0f;
    }
}   // fade

//-----------------------------------------------------------------------------
void SkidMarks::SkidMarkQuads::recalcBSphere()
{
    Vec3 diff = m_aabb_max - m_aabb_min;
    bsphere.setRadius(diff.length()*0.5f);
    Vec3 center = (m_aabb_min + m_aabb_max)*0.5f;
    bsphere.setCenter(center.toFloat());
}   // recalcBSphere

//-----------------------------------------------------------------------------
void SkidMarks::SkidMarkQuads::add(const Vec3 &left, const Vec3 &right)
{
    // The skid marks must be raised slightly higher, otherwise it blends
    // too much with the track.
    sgVec3 l;
    sgCopyVec3(l, left.toFloat());
    l[2]+=0.01f+m_z_offset;
    sgVec3 r;
    sgCopyVec3(r, right.toFloat());
    r[2]+=0.01f;
    vertices->add(l);
    vertices->add(r);
    // Adjust the axis-aligned boundary boxes.
    m_aabb_min.min(left);m_aabb_min.min(right);
    m_aabb_max.max(left);m_aabb_max.max(right);

    dirtyBSphere();
}   // add
