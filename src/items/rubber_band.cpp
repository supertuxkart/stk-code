//  $Id: rubber_band.hpp 2458 2008-11-15 02:12:28Z auria $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
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

#include "rubber_band.hpp"

#include "material_manager.hpp"
#include "race_manager.hpp"
#include "graphics/scene.hpp"
#include "items/plunger.hpp"
#include "items/projectile_manager.hpp"
#include "karts/kart.hpp"
#include "modes/world.hpp"
#include "physics/physics.hpp"

/** RubberBand constructor. It creates a simple quad and attaches it to the
 *  root(!) of the graph. It's easier this way to get the right coordinates
 *  than attaching it to the plunger or kart, and trying to find the other
 *  coordinate.
 *  \param plunger Pointer to the plunger (non const, since the rubber band 
 *                 can trigger an explosion)
 *  \param kart    Reference to the kart.
 */
RubberBand::RubberBand(Plunger *plunger, const Kart &kart)
          : ssgVtxTable(GL_QUADS, new ssgVertexArray,
                        new ssgNormalArray,
                        new ssgTexCoordArray,
                        new ssgColourArray ), 
            m_plunger(plunger), m_owner(kart)
{
#ifdef DEBUG
    setName("rubber_band");
#endif

    // The call to update defines the actual coordinates, only the entries are added for now.
    vertices->add(0, 0, 0); vertices->add(0, 0, 0);
    vertices->add(0, 0, 0); vertices->add(0, 0, 0);
    m_attached_state = RB_TO_PLUNGER;
    update(0);

    sgVec3 norm;
    sgSetVec3(norm, 1/sqrt(2.0f), 0, 1/sqrt(2.0f));
    normals->add(norm);normals->add(norm);
    normals->add(norm);normals->add(norm);

    sgVec4 colour;
    sgSetVec4(colour, 0.7f, 0.0f, 0.0f, 0.3f);
    colours->add(colour);colours->add(colour);
    colours->add(colour);colours->add(colour);
    m_state = new ssgSimpleState();
    m_state->disable(GL_CULL_FACE);
    setState(m_state);
    //setState(material_manager->getMaterial("chrome.rgb")->getState());

    scene->add(this);
}   // RubberBand

// ----------------------------------------------------------------------------
/** Removes the rubber band from the scene. Is called when the plunger 
 *  explodes.
 */
void RubberBand::removeFromScene()
{
    scene->remove(this);
}   // removeFromScene

// ----------------------------------------------------------------------------
/** Updates the rubber band. It takes the new position of the kart and the
 *  plunger, and sets the quad representing the rubber band appropriately.
 *  It then casts a ray along the rubber band to detect if anything is hit. If
 *  so, an explosion is triggered.
 *  \param dt: Time step size.
 */
void RubberBand::update(float dt)
{
    if(m_owner.isEliminated())
    {
        // Rubber band snaps
        m_plunger->hit(NULL);
        // This causes the plunger to be removed at the next update
        m_plunger->setKeepAlive(0.0f);
        return;
    }

    Vec3 p;
    const Vec3 &k = m_owner.getXYZ();

    // Get the position to which the band is attached
    // ----------------------------------------------
    switch(m_attached_state)
    {
    case RB_TO_KART:    p = m_hit_kart->getXYZ(); break;
    case RB_TO_TRACK:   p = m_hit_position;       break;
    case RB_TO_PLUNGER: p = m_plunger->getXYZ();
                        checkForHit(k, p);        break;
    }   // switch(m_attached_state);

    // Draw the rubber band
    // --------------------
    // Todo: make height dependent on length (i.e. rubber band gets
    // thinner). And call explosion if the band is too long.
    const float hh=.1f;  // half height of the band
  
    float *f = vertices->get(0);
    f[0] = p.getX()-hh; f[1] = p.getY(); f[2] = p.getZ()-hh;
    f = vertices->get(1);
    f[0] = p.getX()+hh; f[1] = p.getY(); f[2] = p.getZ()+hh;
    f = vertices->get(2);
    f[0] = k.getX()+hh; f[1] = k.getY(); f[2] = k.getZ()+hh;
    f = vertices->get(3);
    f[0] = k.getX()-hh; f[1] = k.getY(); f[2] = k.getZ()-hh;
    dirtyBSphere();

    // Check for rubber band snapping
    // ------------------------------
    float l = (p-k).length2();
    float max_len = m_owner.getKartProperties()->getRubberBandMaxLength();
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
        float force = m_owner.getKartProperties()->getRubberBandForce();
        Vec3 diff   = p-k;
        
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
        m_owner.getBody()->applyCentralForce(diff*force);
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
    if(m_owner.getBody()->getBroadphaseHandle())
        old_kart_group = m_owner.getBody()->getBroadphaseHandle()->m_collisionFilterGroup;
    m_plunger->getBody()->getBroadphaseHandle()->m_collisionFilterGroup = 0;
    if(m_owner.getBody()->getBroadphaseHandle())
        m_owner.getBody()->getBroadphaseHandle()->m_collisionFilterGroup = 0;

    // Do the raycast
    RaceManager::getWorld()->getPhysics()->getPhysicsWorld()->rayTest(k, p, 
                                                                      ray_callback);
    // Reset collision groups
    m_plunger->getBody()->getBroadphaseHandle()->m_collisionFilterGroup = old_plunger_group;
    if(m_owner.getBody()->getBroadphaseHandle())
        m_owner.getBody()->getBroadphaseHandle()->m_collisionFilterGroup = old_kart_group;
    if(ray_callback.HasHit())
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
void RubberBand::hit(Kart *kart_hit, const Vec3 *track_xyz)
{
    // More than one report of a hit. This can happen if the raycast detects
    // a hit as well as the bullet physics.
    if(m_attached_state!=RB_TO_PLUNGER) return;

    // A kart was hit
    // ==============
    if(kart_hit)
    {
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
