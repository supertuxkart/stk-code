//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2013 Joerg Henrichs
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

#include "items/rubber_band.hpp"

#include <IMeshSceneNode.h>

#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/stkmeshscenenode.hpp"
#include "items/plunger.hpp"
#include "items/projectile_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "karts/max_speed.hpp"
#include "modes/world.hpp"
#include "physics/physics.hpp"
#include "race/race_manager.hpp"
#include "utils/string_utils.hpp"
#include "graphics/glwrap.hpp"

#include "utils/log.hpp" //TODO: remove after debugging is done

#include <IMesh.h>

/** RubberBand constructor. It creates a simple quad and attaches it to the
 *  root(!) of the graph. It's easier this way to get the right coordinates
 *  than attaching it to the plunger or kart, and trying to find the other
 *  coordinate.
 *  \param plunger Pointer to the plunger (non const, since the rubber band
 *                 can trigger an explosion)
 *  \param kart    Reference to the kart.
 */
RubberBand::RubberBand(Plunger *plunger, AbstractKart *kart)
          : m_plunger(plunger), m_owner(kart)
{
    const video::SColor color(77, 179, 0, 0);
    video::SMaterial m;
    m.AmbientColor    = color;
    m.DiffuseColor    = color;
    m.EmissiveColor   = color;
    m.BackfaceCulling = false;
    m_mesh           = irr_driver->createQuadMesh(&m, /*create_one_quad*/ true);
    m_buffer         = m_mesh->getMeshBuffer(0);
    m_attached_state = RB_TO_PLUNGER;
    assert(m_buffer->getVertexType()==video::EVT_STANDARD);

    // Set the vertex colors properly, as the new pipeline doesn't use the old light values
    u32 i;
    scene::IMeshBuffer * const mb = m_mesh->getMeshBuffer(0);
    video::S3DVertex * const verts = (video::S3DVertex *) mb->getVertices();
    const u32 max = mb->getVertexCount();
    for (i = 0; i < max; i++)
    {
        verts[i].Color = color;
    }

    // Color
    mb->getMaterial().setTexture(0, getUnicolorTexture(video::SColor(255, 255, 255, 255)));
    // Gloss
    mb->getMaterial().setTexture(1, getUnicolorTexture(video::SColor(0, 0, 0, 0)));
    updatePosition();
    m_node = irr_driver->addMesh(m_mesh, "rubberband");
    irr_driver->applyObjectPassShader(m_node);
    if (STKMeshSceneNode *stkm = dynamic_cast<STKMeshSceneNode *>(m_node))
        stkm->setReloadEachFrame(true);
#ifdef DEBUG
    std::string debug_name = m_owner->getIdent()+" (rubber-band)";
    m_node->setName(debug_name.c_str());
#endif

}   // RubberBand

// ----------------------------------------------------------------------------
RubberBand::~RubberBand()
{
    irr_driver->removeNode(m_node);
    m_mesh->drop();
}   // RubberBand

// ----------------------------------------------------------------------------
/** Updates the position of the rubber band. It especially sets the
 *  end position of the rubber band, i.e. the side attached to the plunger,
 *  track, or kart hit.
 */
void RubberBand::updatePosition()
{
    const Vec3 &k = m_owner->getXYZ();

    // Get the position to which the band is attached
    // ----------------------------------------------
    switch(m_attached_state)
    {
    case RB_TO_KART:    m_end_position = m_hit_kart->getXYZ(); break;
    case RB_TO_TRACK:   m_end_position = m_hit_position;       break;
    case RB_TO_PLUNGER: m_end_position = m_plunger->getXYZ();
                        checkForHit(k, m_end_position);        break;
    }   // switch(m_attached_state);

    // Update the rubber band positions
    // --------------------------------
    // Todo: make height dependent on length (i.e. rubber band gets
    // thinner). And call explosion if the band is too long.
    const float hh=.1f;  // half height of the band
    const Vec3 &p=m_end_position;  // for shorter typing
    irr::video::S3DVertex* v=(video::S3DVertex*)m_buffer->getVertices();
    v[0].Pos.X = p.getX()-hh; v[0].Pos.Y=p.getY(); v[0].Pos.Z = p.getZ()-hh;
    v[1].Pos.X = p.getX()+hh; v[1].Pos.Y=p.getY(); v[1].Pos.Z = p.getZ()+hh;
    v[2].Pos.X = k.getX()+hh; v[2].Pos.Y=k.getY(); v[2].Pos.Z = k.getZ()+hh;
    v[3].Pos.X = k.getX()-hh; v[3].Pos.Y=k.getY(); v[3].Pos.Z = k.getZ()-hh;
    m_buffer->recalculateBoundingBox();
    m_mesh->setBoundingBox(m_buffer->getBoundingBox());
}   // updatePosition

// ----------------------------------------------------------------------------
/** Updates the rubber band. It takes the new position of the kart and the
 *  plunger, and sets the quad representing the rubber band appropriately.
 *  It then casts a ray along the rubber band to detect if anything is hit. If
 *  so, an explosion is triggered.
 *  \param dt: Time step size.
 */
void RubberBand::update(float dt)
{
    if(m_owner->isEliminated())
    {
        // Rubber band snaps
        m_plunger->hit(NULL);
        // This causes the plunger to be removed at the next update
        m_plunger->setKeepAlive(0.0f);
        return;
    }

    updatePosition();
    const Vec3 &k = m_owner->getXYZ();

    // Check for rubber band snapping
    // ------------------------------
    float l = (m_end_position-k).length2();
    float max_len = m_owner->getKartProperties()->getRubberBandMaxLength();
    if(l>max_len*max_len)
    {
        // Rubber band snaps
        m_plunger->hit(NULL);
        // This causes the plunger to be removed at the next update
        m_plunger->setKeepAlive(0.0f);
    }

    // Apply forces (if applicable)
    // ----------------------------
    if(m_attached_state!=RB_TO_PLUNGER)
    {
        float force = m_owner->getKartProperties()->getRubberBandForce();
        Vec3 diff   = m_end_position-k;

        // detach rubber band if kart gets very close to hit point
        if(m_attached_state==RB_TO_TRACK && diff.length2() < 10*10)
        {
            // Rubber band snaps
            m_plunger->hit(NULL);
            // This causes the plunger to be removed at the next update
            m_plunger->setKeepAlive(0.0f);
            return;
        }

        diff.normalize();   // diff can't be zero here
        m_owner->getBody()->applyCentralForce(diff*force);
        m_owner->increaseMaxSpeed(MaxSpeed::MS_INCREASE_RUBBER,
            m_owner->getKartProperties()->getRubberBandSpeedIncrease(),
            /*engine_force*/ 0.0f,
            /*duration*/0.1f,
            m_owner->getKartProperties()->getRubberBandFadeOutTime());
        if(m_attached_state==RB_TO_KART)
            m_hit_kart->getBody()->applyCentralForce(diff*(-force));
    }
}   // update

// ----------------------------------------------------------------------------
/** Uses a raycast to see if anything has hit the rubber band.
 *  \param k Position of the kart = one end of the rubber band
 *  \param p Position of the plunger = other end of the rubber band.
 */
void RubberBand::checkForHit(const Vec3 &k, const Vec3 &p)
{
    btCollisionWorld::ClosestRayResultCallback ray_callback(k, p);
    // Disable raycast collision detection for this plunger and this kart!
    short int old_plunger_group = m_plunger->getBody()->getBroadphaseHandle()->m_collisionFilterGroup;
    short int old_kart_group=0;

    // If the owner is being rescued, the broadphase handle does not exist!
    if(m_owner->getBody()->getBroadphaseHandle())
        old_kart_group = m_owner->getBody()->getBroadphaseHandle()->m_collisionFilterGroup;
    m_plunger->getBody()->getBroadphaseHandle()->m_collisionFilterGroup = 0;
    if(m_owner->getBody()->getBroadphaseHandle())
        m_owner->getBody()->getBroadphaseHandle()->m_collisionFilterGroup = 0;

    // Do the raycast
    World::getWorld()->getPhysics()->getPhysicsWorld()->rayTest(k, p,
                                                                ray_callback);
    // Reset collision groups
    m_plunger->getBody()->getBroadphaseHandle()->m_collisionFilterGroup = old_plunger_group;
    if(m_owner->getBody()->getBroadphaseHandle())
        m_owner->getBody()->getBroadphaseHandle()->m_collisionFilterGroup = old_kart_group;
    if(ray_callback.hasHit())
    {
        Vec3 pos(ray_callback.m_hitPointWorld);
        UserPointer *up = (UserPointer*)ray_callback.m_collisionObject->getUserPointer();
        if(up && up->is(UserPointer::UP_KART))
            hit(up->getPointerKart(), &pos);
        else
            hit(NULL, &pos);
    }  // if raycast hast hit

}   // checkForHit

// ----------------------------------------------------------------------------
/** The plunger hit a kart or the track.
 *  \param kart_hit The kart hit, or NULL if the track was hit.
 *  \param track _xyz The coordinated where the track was hit (NULL if a kart
 *                    was hit.
 */
void RubberBand::hit(AbstractKart *kart_hit, const Vec3 *track_xyz)
{
    // More than one report of a hit. This can happen if the raycast detects
    // a hit as well as the bullet physics.
    if(m_attached_state!=RB_TO_PLUNGER) return;


    // A kart was hit
    // ==============
    if(kart_hit)
    {
        if(kart_hit->isShielded())
        {
            kart_hit->decreaseShieldTime();
            m_plunger->setKeepAlive(0.0f);

            return;
        }

        m_hit_kart       = kart_hit;
        m_attached_state = RB_TO_KART;
        return;
    }

    // The track was hit
    // =================
    m_hit_position   = *track_xyz;
    m_attached_state = RB_TO_TRACK;
}   // hit

// ----------------------------------------------------------------------------
