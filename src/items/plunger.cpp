//  $Id: missile.cpp 1284 2007-11-08 12:31:54Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Joerg Henrichs
//
//  Physics improvements and linear intersection algorithm by
//  by David Mikos. Copyright (C) 2009.
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

#include "items/plunger.hpp"

#include "graphics/irr_driver.hpp"
#include "graphics/scene.hpp"
#include "items/rubber_band.hpp"
#include "items/projectile_manager.hpp"
#include "karts/player_kart.hpp"
#include "modes/world.hpp"
#include "physics/physical_object.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"

// -----------------------------------------------------------------------------
Plunger::Plunger(Kart *kart) : Flyable(kart, POWERUP_PLUNGER)
{
    const float gravity = 0.0f;

    float y_offset = 0.5f*kart->getKartLength()+0.5f*m_extend.getY();
    float up_velocity = 0.0f;
    float plunger_speed = 2 * m_speed;

    // if the kart is looking backwards, release from the back
    m_reverse_mode = kart->getControls().m_look_back;

    // find closest kart in front of the current one
    const Kart *closest_kart=0;   btVector3 direction;   float kartDistSquared;
    getClosestKart(&closest_kart, &kartDistSquared, &direction, kart /* search in front of this kart */, m_reverse_mode);

    btTransform trans = kart->getTrans();

    btMatrix3x3 thisKartDirMatrix = kart->getKartHeading().getBasis();
    btVector3 thisKartDirVector(thisKartDirMatrix[0][1],
                                thisKartDirMatrix[1][1],
                                thisKartDirMatrix[2][1]);

    float heading=atan2f(-thisKartDirVector.getX(), thisKartDirVector.getY());
    float pitch = kart->getTerrainPitch(heading);

    // aim at this kart if it's not too far
    if(closest_kart != NULL && kartDistSquared < 30*30)
    {
        float fire_angle     = 0.0f;
        float time_estimated = 0.0f;
        getLinearKartItemIntersection (kart->getTrans().getOrigin(), closest_kart,
                                       plunger_speed, gravity,
                                       &fire_angle, &up_velocity, &time_estimated);

        // apply transformation to the bullet object (without pitch)
        btMatrix3x3 m;
        m.setEulerZYX(0.0f, 0.0f, fire_angle);
        trans.setBasis(m);

        m_initial_velocity = btVector3(0.0f, plunger_speed, up_velocity);

        createPhysics(y_offset, m_initial_velocity,
                      new btCylinderShape(0.5f*m_extend), gravity, false /* rotates */, false, &trans );
    }
    else
    {
        trans = kart->getKartHeading();

        createPhysics(y_offset, btVector3(pitch, plunger_speed, 0.0f),
                      new btCylinderShape(0.5f*m_extend), gravity, false /* rotates */, m_reverse_mode, &trans );
    }

    // pulling back makes no sense in battle mode, since this mode is not a race.
    // so in battle mode, always hide view
    if( m_reverse_mode || race_manager->isBattleMode(race_manager->getMinorMode()) )
        m_rubber_band = NULL;
    else
    {
        m_rubber_band = new RubberBand(this, *kart);
    }
    m_keep_alive = -1;
}   // Plunger

// -----------------------------------------------------------------------------
Plunger::~Plunger()
{
    if(m_rubber_band)
        m_rubber_band->removeFromScene();
}   // ~Plunger

// -----------------------------------------------------------------------------
void Plunger::init(const lisp::Lisp* lisp, scene::IMesh *plunger_model)
{
    Flyable::init(lisp, plunger_model, POWERUP_PLUNGER);
}   // init

// -----------------------------------------------------------------------------
void Plunger::update(float dt)
{
    // In keep-alive mode, just update the rubber band
    if(m_keep_alive >= 0)
    {
        m_keep_alive -= dt;
        if(m_keep_alive<=0)
        {
            setHasHit();
            projectile_manager->notifyRemove();
        }
        if(m_rubber_band != NULL) m_rubber_band->update(dt);
        return;
    }

    // Else: update the flyable and rubber band
    Flyable::update(dt);
    if(m_rubber_band != NULL) m_rubber_band->update(dt);
    
    if(getHoT()==Track::NOHIT) return;
    float hat = getTrans().getOrigin().getZ()-getHoT();

    // Use the Height Above Terrain to set the Z velocity.
    // HAT is clamped by min/max height. This might be somewhat
    // unphysical, but feels right in the game.

    float delta = m_average_height - std::max(std::min(hat, m_max_height), m_min_height);
    btVector3 v = getVelocity();
    float heading = atan2f(-v.getX(), v.getY());
    float pitch   = getTerrainPitch (heading);
    float vel_z = m_force_updown*(delta);
    if (hat < m_max_height) // take into account pitch of surface
        vel_z += hypotf(v.getX(), v.getY())*tanf(pitch);
    v.setZ(vel_z);
    setVelocity(v);
}   // update

// -----------------------------------------------------------------------------
/** Virtual function called when the plunger hits something.
 *  The plunger is special in that it is not deleted when hitting an object.
 *  Instead it stays around (though not as a graphical or physical object)
 *  till the rubber band expires.
 *  \param kart Pointer to the kart hit (NULL if not a kart).
 *  \param obj  Pointer to PhysicalObject object if hit (NULL otherwise).
 */
void Plunger::hit(Kart *kart, PhysicalObject *obj)
{
    if(isOwnerImmunity(kart)) return;

    // pulling back makes no sense in battle mode, since this mode is not a race.
    // so in battle mode, always hide view
    if( m_reverse_mode || race_manager->isBattleMode(race_manager->getMinorMode()) )
    {
        if(kart) kart->blockViewWithPlunger();

        m_keep_alive = 0;
        // Make this object invisible by placing it faaar down. Not that if this
        // objects is simply removed from the scene graph, it might be auto-deleted
        // because the ref count reaches zero.
        Vec3 hell(0, 0, -10000);
        getNode()->setPosition(hell.toIrrVector());
        RaceManager::getWorld()->getPhysics()->removeBody(getBody());
    }
    else
    {
        m_keep_alive = m_owner->getKartProperties()->getRubberBandDuration();

        // Make this object invisible by placing it faaar down. Not that if this
        // objects is simply removed from the scene graph, it might be auto-deleted
        // because the ref count reaches zero.
        scene::ISceneNode *node = getNode();
        if(node)
        {
            Vec3 hell(0, 0, -10000);
            getNode()->setPosition(hell.toIrrVector());
        }
        RaceManager::getWorld()->getPhysics()->removeBody(getBody());
        
        if(kart)
        {
            m_rubber_band->hit(kart);
            return;
        }
        else if(obj)
        {
            Vec3 pos(obj->getBody()->getWorldTransform().getOrigin());
            m_rubber_band->hit(NULL, &pos);
        }
        else
        {
            m_rubber_band->hit(NULL, &(getXYZ()));
        }
    }
}   // hit

// -----------------------------------------------------------------------------
/** Called when the plunger hits the track. In this case, notify the rubber
 *  band, and remove the plunger (but keep it alive). 
 */
void Plunger::hitTrack()
{
    hit(NULL, NULL);
}   // hitTrack

// -----------------------------------------------------------------------------
