//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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
#include "network/network_manager.hpp"
#include "network/race_state.hpp"

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
    if(network_manager->getMode()==NetworkManager::NW_CLIENT)
    {
        updateClient(dt);
    }
    else
    {
        updateServer(dt);
    }

    // Then check if any projectile hit something
    if(m_something_was_hit)
    {
        Projectiles::iterator p = m_active_projectiles.begin();
        while(p!=m_active_projectiles.end())
        {
            if(! (*p)->hasHit()) { p++; continue; }
            if((*p)->needsExplosion())
            {
                newHitEffect((*p)->getXYZ(), (*p)->getExplosionSound(),
                             /*player_kart_hit*/ false );
            }
            Flyable *f=*p;
            Projectiles::iterator pNext=m_active_projectiles.erase(p);  // returns the next element
            delete f;
            p=pNext;
        }   // while p!=m_active_projectiles.end()
    }

    HitEffects::iterator he = m_active_hit_effects.begin();
    while(he!=m_active_hit_effects.end())
    {
        // Update this hit effect. If it can be removed, remove it.
        if((*he)->update(dt))
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
    // First update all projectiles on the track
    if(network_manager->getMode()!=NetworkManager::NW_NONE)
    {
        race_state->setNumFlyables(m_active_projectiles.size());
    }
    for(Projectiles::iterator i  = m_active_projectiles.begin();
                              i != m_active_projectiles.end();   ++i)
    {
        (*i)->update(dt);
        // Store the state information on the server
        if(network_manager->getMode()!=NetworkManager::NW_NONE)
        {
            race_state->setFlyableInfo(i-m_active_projectiles.begin(),
                                       FlyableInfo((*i)->getXYZ(), 
                                                   (*i)->getRotation(),
                                                   (*i)->hasHit())      );
        }
    }
}   // updateServer

// -----------------------------------------------------------------------------
/** Updates all rockets and hit effects on the client.
 *  updateClient takes the information in race_state and updates all rockets
 *  (i.e. position, hit effects etc)                                            */
void ProjectileManager::updateClient(float dt)
{
    m_something_was_hit = false;
    unsigned int num_projectiles = race_state->getNumFlyables();
    if(num_projectiles != m_active_projectiles.size())
        fprintf(stderr, "Warning: num_projectiles %d active %d\n",num_projectiles,
                (int)m_active_projectiles.size());

    unsigned int indx=0;
    for(Projectiles::iterator i  = m_active_projectiles.begin();
        i != m_active_projectiles.end();   ++i, ++indx)
    {
        const FlyableInfo &f = race_state->getFlyable(indx);
        (*i)->updateFromServer(f, dt);
        if(f.m_exploded) 
        {
            m_something_was_hit = true;
            (*i)->hit(NULL);
        }
    }   // for i in m_active_projectiles

}   // updateClient
// -----------------------------------------------------------------------------
Flyable *ProjectileManager::newProjectile(Kart *kart, 
                                          PowerupManager::PowerupType type)
{
    Flyable *f;
    switch(type) 
    {
        case PowerupManager::POWERUP_BOWLING:    f = new Bowling(kart); break;
        case PowerupManager::POWERUP_PLUNGER:    f = new Plunger(kart); break;
        case PowerupManager::POWERUP_CAKE:       f = new Cake(kart);    break;
        case PowerupManager::POWERUP_RUBBERBALL: f = new RubberBall(kart);break;
        default:              return NULL;
    }
    m_active_projectiles.push_back(f);
    return f;
}   // newProjectile

// -----------------------------------------------------------------------------
/** Creates a new hit effect.
 *  \param coord The coordinates where the hit effect (i.e. sound, graphics)
 *         should be placed).
 *  \param sfx The name of the sound effect to be played.
 *  \param player_kart_hit True of a player kart was hit.
 */
HitEffect* ProjectileManager::newHitEffect(const Vec3& coord, 
                                           const char *sfx,
                                           bool player_kart_hit)
{
    HitEffect *he = new Explosion(coord, sfx, player_kart_hit);
    m_active_hit_effects.push_back(he);
    return he;
}   // newHitEffect

// =============================================================================
/** A general function which is only needed here, but
 *  it's not really a method, so I'll leave it here.
 */
/*
static ssgSelector *find_selector ( ssgBranch *b )
{
    if ( b == NULL )
        return NULL ;

    if ( ! b -> isAKindOf ( ssgTypeBranch () ) )
        return NULL ;

    if ( b -> isAKindOf ( ssgTypeSelector () ) )
        return (ssgSelector *) b ;

    for ( int i = 0 ; i < b -> getNumKids() ; i++ )
    {
        ssgSelector *res = find_selector ( (ssgBranch *)(b ->getKid(i)) ) ;

        if ( res != NULL )
            return res ;
    }

    return NULL ;
}   // find_selector
*/
