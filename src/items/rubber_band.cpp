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
#include "scene.hpp"
#include "items/plunger.hpp"
#include "modes/world.hpp"
#include "karts/kart.hpp"
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
            m_plunger(plunger), m_kart(kart)
{
#ifdef DEBUG
    setName("rubber_band");
#endif

    // The call to update defines the actual coordinates, only the entries are added for now.
    vertices->add(0, 0, 0); vertices->add(0, 0, 0);
    vertices->add(0, 0, 0); vertices->add(0, 0, 0);
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
    const Vec3 &p=m_plunger->getXYZ();
    const Vec3 &k=m_kart.getXYZ();

    Vec3 diff=p-k;

    // See if anything hits the rubber band, but only if there is a certain 
    // distance between plunger and kart. Otherwise the raycast will either
    // hit the kart or the plunger.
    if(diff.length2()>2.0f)
    {
        diff.normalize();
        const Vec3 from=k+diff;  // this could be made dependent on kart length
        const Vec3 to = p-diff;
        btCollisionWorld::ClosestRayResultCallback ray_callback(from, to);
        RaceManager::getWorld()->getPhysics()->getPhysicsWorld()->rayTest(from, 
                                                              to, ray_callback);
        if(ray_callback.HasHit())
        {
            m_plunger->explode(NULL);
            return;
        }
    }

    // Otherwise set the new coordinates for the plunger and the kart:
    // ---------------------------------------------------------------
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
}   // update

// ----------------------------------------------------------------------------