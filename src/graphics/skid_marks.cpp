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

#include "graphics/irr_driver.hpp"
#include "karts/kart.hpp"
#include "physics/btKart.hpp"

float SkidMarks::m_avoid_z_fighting  = 0.0f;
const int SkidMarks::m_start_alpha = 128;
const int SkidMarks::m_start_grey = 32;
//const int start_premul = SkidMarks::m_start_grey * SkidMarks::m_start_alpha / 256; compiler whines about private later
const int start_premul = 32 * 128 / 256;

/** Initialises empty skid marks. */
SkidMarks::SkidMarks(const Kart& kart, float width) : m_kart(kart)
{
    m_width                   = width;
    m_material                = new video::SMaterial();
    m_material->MaterialType  = video::EMT_TRANSPARENT_VERTEX_ALPHA;
    m_material->AmbientColor  = video::SColor(128, 0, 0, 0); // or should be start_premul?
//    m_material->DiffuseColor  = video::SColor(SkidMarks::m_start_alpha, start_premul, start_premul, start_premul); compiler whines about private
    m_material->DiffuseColor  = video::SColor(128, 16, 16, 16);
    m_material->Shininess     = 0;
    m_skid_marking            = false;
    m_current                 = -1;
}   // SkidMark

//-----------------------------------------------------------------------------
/** Removes all skid marks from the scene graph and frees the state. */
SkidMarks::~SkidMarks()
{
    reset();  // remove all skid marks
    delete m_material;
}   // ~SkidMarks

//-----------------------------------------------------------------------------
/** Removes all skid marks, called when a race is restarted.
 */
void SkidMarks::reset()
{
    for(unsigned int i=0; i<m_nodes.size(); i++)
    {
        irr_driver->removeNode(m_nodes[i]);
        m_left[i]->drop();
        m_right[i]->drop();
    }
    m_left.clear();
    m_right.clear();
    m_nodes.clear();
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
    float f = dt/stk_config->m_skid_fadeout_time*m_start_alpha;
    for(unsigned int i=0; i<m_left.size(); i++)
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
            // The vertices and indices will not change anymore (till these
            // skid mark quads are deleted)
            m_left[m_current]->setHardwareMappingHint(scene::EHM_STATIC);
            m_right[m_current]->setHardwareMappingHint(scene::EHM_STATIC);
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
        // Adjust the boundary box of the mesh to include the 
        // adjusted aabb of its buffers.
        core::aabbox3df aabb=m_nodes[m_current]->getMesh()->getBoundingBox();
        aabb.addInternalBox(m_left[m_current]->getAABB());
        aabb.addInternalBox(m_right[m_current]->getAABB());
        m_nodes[m_current]->getMesh()->setBoundingBox(aabb);
        return;
    }

    // Currently no skid marking
    // -------------------------
    if((!m_kart.getControls().m_drift) || 
        (fabsf(m_kart.getControls().m_steer) < 0.001f)) return;   // no skidmarking
        
    // not turning enough, don't draw skidmarks if kart is going straight ahead
    // this is even stricter for Ai karts, since they tend to use LOTS of skidding
    const float min_skid_angle = m_kart.getController()->isPlayerController() ? 0.55f : 0.95f;
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
                                         m_material, m_avoid_z_fighting);
    scene::SMesh *new_mesh = new scene::SMesh();
    new_mesh->addMeshBuffer(smq_left);

    m_avoid_z_fighting += 0.001f;
    if(m_avoid_z_fighting>0.01f) m_avoid_z_fighting = 0.0f;
    SkidMarkQuads *smq_right = new SkidMarkQuads(raycast_right.m_contactPointWS
                                                  - delta,
                                                  raycast_right.m_contactPointWS,
                                                  m_material,
                                                  m_avoid_z_fighting);
    new_mesh->addMeshBuffer(smq_right);
    scene::IMeshSceneNode *new_node = irr_driver->addMesh(new_mesh);
#ifdef DEBUG
    std::string debug_name = m_kart.getIdent()+" (skid-mark)";
    new_node->setName(debug_name.c_str());
#endif
    
    // We don't keep a reference to the mesh here, so we have to decrement
    // the reference count (which is set to 1 when doing "new SMesh()".
    // The scene node will keep the mesh alive.
    new_mesh->drop();
    m_current++;
    if(m_current>=stk_config->m_max_skidmarks)
        m_current = 0;
    if(m_current>=(int)m_left.size())
    {
        m_left. push_back (smq_left );
        m_right.push_back (smq_right);
        m_nodes.push_back (new_node);
    }
    else
    {
        irr_driver->removeNode(m_nodes[m_current]);
        // Not necessary to delete m_nodes: removeNode
        // deletes the node since its refcount reaches zero.
        m_left[m_current]->drop();
        m_right[m_current]->drop();

        m_left  [m_current] = smq_left;
        m_right [m_current] = smq_right;
        m_nodes [m_current] = new_node;
    }

    m_skid_marking = true;
    // More triangles are added each frame, so for now leave it
    // to stream.
    m_left[m_current]->setHardwareMappingHint(scene::EHM_STREAM);
    m_right[m_current]->setHardwareMappingHint(scene::EHM_STREAM);
}   // update

//=============================================================================
SkidMarks::SkidMarkQuads::SkidMarkQuads(const Vec3 &left, const Vec3 &right,
                                        video::SMaterial *material, 
                                        float z_offset)
                         : scene::SMeshBuffer()
{
    m_z_offset = z_offset;
    m_fade_out = 0.0f;

    Material   = *material;
    m_aabb     = core::aabbox3df(left.toIrrVector());
    add(left, right);

}   // SkidMarkQuads

//-----------------------------------------------------------------------------
void SkidMarks::SkidMarkQuads::add(const Vec3 &left, const Vec3 &right)
{
    // The skid marks must be raised slightly higher, otherwise it blends
    // too much with the track.
    int n = Vertices.size();

    video::S3DVertex v;
    v.Color = Material.DiffuseColor;
    v.Color.setAlpha(m_start_alpha);
    v.Pos = left.toIrrVector();
    v.Pos.Y += m_z_offset;
    v.Normal = core::vector3df(0, 1, 0);
    Vertices.push_back(v);
    v.Pos = right.toIrrVector();
    v.Pos.Y += m_z_offset;
    Vertices.push_back(v);
    // Out of the box Irrlicht only supports triangle meshes and not
    // triangle strips. Since this is a strip it would be more efficient
    // to use a special triangle strip scene node.
    if(n>=2)
    {
        Indices.push_back(n-2);
        Indices.push_back(n  );
        Indices.push_back(n-1);
        Indices.push_back(n-1);
        Indices.push_back(n  );
        Indices.push_back(n+1);
    }
    // Adjust the axis-aligned boundary boxes.
    m_aabb.addInternalPoint(left.toIrrVector() );
    m_aabb.addInternalPoint(right.toIrrVector());
    setBoundingBox(m_aabb);
    
    setDirty();
}   // add

// ----------------------------------------------------------------------------
/** Fades the current skid marks. 
 *  \param f fade factor.
 */
void SkidMarks::SkidMarkQuads::fade(float f)
{
    m_fade_out += f;
    // Changing the alpha value is quite expensive, so it's only done
    // about 10 times till 0 is reached.
    if(m_fade_out*10>SkidMarks::m_start_alpha)
    {
        video::SColor &c=Material.DiffuseColor;
        int a=c.getAlpha();
        a -= a<m_fade_out ? a : (int)m_fade_out;
        int premul_grey = a * SkidMarks::m_start_grey / 255;
        c.set(a, premul_grey, premul_grey, premul_grey);
        for(unsigned int i=0; i<Vertices.size(); i++)
        {
            Vertices[i].Color.set(a, premul_grey, premul_grey, premul_grey);
        }
        m_fade_out = 0.0f;
    }
}   // fade

// ----------------------------------------------------------------------------

void SkidMarks::adjustFog(bool enabled)
{
    m_material->FogEnable = enabled; 
}
