//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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

#include "items/projectile_manager.hpp"

#include "graphics/explosion.hpp"
#include "graphics/hit_effect.hpp"
#include "items/bowling.hpp"
#include "items/cake.hpp"
#include "items/plunger.hpp"
#include "items/powerup_manager.hpp"
#include "items/powerup.hpp"
#include "items/rubber_ball.hpp"
#include "karts/abstract_kart.hpp"

ProjectileManager *projectile_manager=0;

void ProjectileManager::loadData()
{
}   // loadData

//-----------------------------------------------------------------------------
void ProjectileManager::removeTextures()
{
    cleanup();
    // Only the explosion is here, all other models are actually managed
    // by powerup_manager.
}   // removeTextures

//-----------------------------------------------------------------------------
void ProjectileManager::cleanup()
{
    for(Projectiles::iterator i = m_active_projectiles.begin();
        i != m_active_projectiles.end(); ++i)
    {
        delete *i;
    }

    m_active_projectiles.clear();
    for(HitEffects::iterator i  = m_active_hit_effects.begin();
        i != m_active_hit_effects.end(); ++i)
    {
        delete *i;
    }

    m_active_hit_effects.clear();
}   // cleanup

// -----------------------------------------------------------------------------
/** General projectile update call. */
void ProjectileManager::update(float dt)
{
    updateServer(dt);

    HitEffects::iterator he = m_active_hit_effects.begin();
    while(he!=m_active_hit_effects.end())
    {
        // While this shouldn't happen, we had one crash because of this
        if(!(*he))
        {
            HitEffects::iterator next = m_active_hit_effects.erase(he);
            he = next;
        }
        // Update this hit effect. If it can be removed, remove it.
        else if((*he)->updateAndDelete(dt))
        {
            delete *he;
            HitEffects::iterator next = m_active_hit_effects.erase(he);
            he = next;
        }   // if hit effect finished
        else  // hit effect not finished, go to next one.
            he++;
    }   // while hit effect != end
}   // update

// -----------------------------------------------------------------------------
/** Updates all rockets on the server (or no networking). */
void ProjectileManager::updateServer(float dt)
{
    Projectiles::iterator p = m_active_projectiles.begin();
    while(p!=m_active_projectiles.end())
    {
        bool can_be_deleted = (*p)->updateAndDelete(dt);
        if(can_be_deleted)
        {
            HitEffect *he = (*p)->getHitEffect();
            if(he)
                addHitEffect(he);
            Flyable *f=*p;
            Projectiles::iterator p_next=m_active_projectiles.erase(p);
            delete f;
            p=p_next;
        }
        else
            p++;
    }   // while p!=m_active_projectiles.end()
    
}   // updateServer

// -----------------------------------------------------------------------------
/** Creates a new projectile of the given type.
 *  \param kart The kart which shoots the projectile.
 *  \param type Type of projectile.
 */
Flyable *ProjectileManager::newProjectile(AbstractKart *kart,
                                          PowerupManager::PowerupType type)
{
    Flyable *f;
    switch(type)
    {
        case PowerupManager::POWERUP_BOWLING:    f = new Bowling(kart);  break;
        case PowerupManager::POWERUP_PLUNGER:    f = new Plunger(kart);  break;
        case PowerupManager::POWERUP_CAKE:       f = new Cake(kart);     break;
        case PowerupManager::POWERUP_RUBBERBALL: f = new RubberBall(kart);
                                                                         break;
        default:              return NULL;
    }
    m_active_projectiles.push_back(f);
    return f;
}   // newProjectile

// -----------------------------------------------------------------------------
/** Returns true if a projectile is within the given distance of the specified
 *  kart.
 *  \param kart The kart for which the test is done.
 *  \param radius Distance within which the projectile must be.
*/
bool ProjectileManager::projectileIsClose(const AbstractKart * const kart,
                                         float radius)
{
    float r2 = radius*radius;

    for(Projectiles::iterator i  = m_active_projectiles.begin();
                              i != m_active_projectiles.end();   i++)
    {
        float dist2 = (*i)->getXYZ().distance2(kart->getXYZ());
        if(dist2<r2) return true;
    }
    return false;
}   // projectileIsClose
