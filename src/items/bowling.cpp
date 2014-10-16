//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007-2013 Joerg Henrichs
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

#include "items/bowling.hpp"

#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "graphics/hit_sfx.hpp"
#include "graphics/material.hpp"
#include "io/xml_node.hpp"
#include "karts/abstract_kart.hpp"
#include "utils/random_generator.hpp"

#include "utils/log.hpp" //TODO: remove after debugging is done

float Bowling::m_st_max_distance;   // maximum distance for a bowling ball to be attracted
float Bowling::m_st_max_distance_squared;
float Bowling::m_st_force_to_target;

// -----------------------------------------------------------------------------
Bowling::Bowling(AbstractKart *kart)
        : Flyable(kart, PowerupManager::POWERUP_BOWLING, 50.0f /* mass */)
{
    m_has_hit_kart = false;
    float y_offset = 0.5f*kart->getKartLength() + m_extend.getZ()*0.5f;

    // if the kart is looking backwards, release from the back
    if( kart->getControls().m_look_back )
    {
        y_offset   = -y_offset;
        m_speed    = -m_speed*2;
    }
    else
    {
        float min_speed = m_speed*4.0f;
        /* make it go faster when throwing forward
           so the player doesn't catch up with the ball
           and explode by touching it */
        m_speed = kart->getSpeed() + m_speed;
        if(m_speed < min_speed) m_speed = min_speed;
    }

    createPhysics(y_offset, btVector3(0.0f, 0.0f, m_speed*2),
                  new btSphereShape(0.5f*m_extend.getY()),
                  1.0f /*restitution*/,
                  -70.0f /*gravity*/,
                  true /*rotates*/);
    // Even if the ball is fired backwards, m_speed must be positive,
    // otherwise the ball can start to vibrate when energy is added.
    m_speed = fabsf(m_speed);
    // Do not adjust the up velociy depending on height above terrain, since
    // this would disable gravity.
    setAdjustUpVelocity(false);

    // unset no_contact_response flags, so that the ball
    // will bounce off the track
    int flag = getBody()->getCollisionFlags();
    flag = flag & (~ btCollisionObject::CF_NO_CONTACT_RESPONSE);
    getBody()->setCollisionFlags(flag);

    // should not live forever, auto-destruct after 20 seconds
    m_max_lifespan = 20;

    m_roll_sfx = SFXManager::get()->createSoundSource("bowling_roll");
    m_roll_sfx->play();
    m_roll_sfx->setLoop(true);

}   // Bowling

// ----------------------------------------------------------------------------
/** Destructor, removes any playing sfx.
 */
Bowling::~Bowling()
{
    // This will stop the sfx and delete the object.
    m_roll_sfx->deleteSFX();

}   // ~RubberBall

// -----------------------------------------------------------------------------
/** Initialises this object with data from the power.xml file.
 *  \param node XML Node
 *  \param bowling The bowling ball mesh
 */
void Bowling::init(const XMLNode &node, scene::IMesh *bowling)
{
    Flyable::init(node, bowling, PowerupManager::POWERUP_BOWLING);
    m_st_max_distance         = 20.0f;
    m_st_max_distance_squared = 20.0f * 20.0f;
    m_st_force_to_target      = 10.0f;

    node.get("max-distance",    &m_st_max_distance   );
    m_st_max_distance_squared = m_st_max_distance*m_st_max_distance;

    node.get("force-to-target", &m_st_force_to_target);
}   // init

// ----------------------------------------------------------------------------
/** Updates the bowling ball ineach frame. If this function returns true, the
 *  object will be removed by the projectile manager.
 *  \param dt Time step size.
 *  \returns True of this object should be removed.
 */
bool Bowling::updateAndDelete(float dt)
{
    bool can_be_deleted = Flyable::updateAndDelete(dt);
    if(can_be_deleted)
        return true;

    const AbstractKart *kart=0;
    Vec3        direction;
    float       minDistance;
    getClosestKart(&kart, &minDistance, &direction);
    if(kart && minDistance<m_st_max_distance_squared)   // move bowling towards kart
    {
        // limit angle, so that the bowling ball does not turn
        // around to hit a kart behind
        if(fabs(m_body->getLinearVelocity().angle(direction)) < 1.3)
        {
            direction*=1/direction.length()*m_st_force_to_target;
            m_body->applyCentralForce(direction);
        }
    }

    // Bowling balls lose energy (e.g. when hitting the track), so increase
    // the speed if the ball is too slow, but only if it's not too high (if
    // the ball is too high, it is 'pushed down', which can reduce the
    // speed, which causes the speed to increase, which in turn causes
    // the ball to fly higher and higher.
    //btTransform trans = getTrans();
    float hat         = getXYZ().getY()-getHoT();
    if(hat-0.5f*m_extend.getY()<0.01f)
    {
        const Material *material = getMaterial();
        if(!material || material->isDriveReset())
        {
            hit(NULL);
            return true;
        }
    }
    btVector3 v       = m_body->getLinearVelocity();
    float vlen        = v.length2();
    if (hat<= m_max_height)
    {
        if(vlen<0.8*m_speed*m_speed)
        {   // bowling lost energy (less than 80%), i.e. it's too slow - speed it up:
            if(vlen==0.0f) {
                v = btVector3(.5f, .0, 0.5f);  // avoid 0 div.
            }
 //           m_body->setLinearVelocity(v*(m_speed/sqrt(vlen)));
        }   // vlen < 0.8*m_speed*m_speed
    }   // hat< m_max_height

    if(vlen<0.1)
    {
        hit(NULL);
        return true;
    }

    if (m_roll_sfx->getStatus()==SFXBase::SFX_PLAYING)
        m_roll_sfx->setPosition(getXYZ());

    return false;
}   // updateAndDelete
// -----------------------------------------------------------------------------
/** Callback from the physics in case that a kart or physical object is hit.
 *  The bowling ball triggers an explosion when hit.
 *  \param kart The kart hit (NULL if no kart was hit).
 *  \param object The object that was hit (NULL if none).
 *  \returns True if there was actually a hit (i.e. not owner, and target is
 *           not immune), false otherwise.
 */
bool Bowling::hit(AbstractKart* kart, PhysicalObject* obj)
{
    bool was_real_hit = Flyable::hit(kart, obj);
    if(was_real_hit)
    {
        if(kart && kart->isShielded())
        {
            kart->decreaseShieldTime();
            return true;
        }
        else
        {
            m_has_hit_kart = kart != NULL;
            explode(kart, obj, /*hit_secondary*/false);
        }
    }
    return was_real_hit;
}   // hit

// ----------------------------------------------------------------------------
/** Returns the hit effect object to use when this objects hits something.
 *  \returns The hit effect object, or NULL if no hit effect should be played.
 */
HitEffect* Bowling::getHitEffect() const
{
    if(m_has_hit_kart)
        return new HitSFX(getXYZ(), "strike");
    else
        return new HitSFX(getXYZ(), "crash");
}   // getHitEffect
