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
#include "items/bowling.hpp"
#include "items/cake.hpp"
#include "items/plunger.hpp"
#include "items/powerup_manager.hpp"
#include "items/powerup.hpp"
#include "network/network_manager.hpp"
#include "network/race_state.hpp"

//static ssgSelector *find_selector ( ssgBranch *b );

ProjectileManager *projectile_manager=0;

void ProjectileManager::loadData()
{
}   // loadData

//-----------------------------------------------------------------------------
void ProjectileManager::removeTextures()
{
    cleanup();
    //ssgDeRefDelete(m_explosion_model);
    // Only the explosion is here, all other models are actually managed
    // by powerup_manager.
    //callback_manager->clear(CB_EXPLOSION);
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
    for(Explosions::iterator i  = m_active_explosions.begin();
        i != m_active_explosions.end(); ++i)
    {
        delete *i;
    }

    m_active_explosions.clear();
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
                newExplosion((*p)->getXYZ(), (*p)->getExplosionSound() );
            }
            Flyable *f=*p;
            Projectiles::iterator pNext=m_active_projectiles.erase(p);  // returns the next element
            delete f;
            p=pNext;
        }   // while p!=m_active_projectiles.end()
    }

    m_explosion_ended=false;
    for(Explosions::iterator i  = m_active_explosions.begin();
        i != m_active_explosions.end(); ++i)
    {
        (*i)->update(dt);
    }
    if(m_explosion_ended)
    {
        Explosions::iterator e;
        e = m_active_explosions.begin();
        while(e!=m_active_explosions.end())
        {
            if(!(*e)->hasEnded()) { e++; continue;}

            delete *e;
            
            Explosions::iterator eNext = m_active_explosions.erase(e);
            e = eNext;
        }   // while e!=m_active_explosions.end()
    }   // if m_explosion_ended
    m_something_was_hit=false;
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
/** Updates all rockets and explosions on the client.
 *  updateClient takes the information in race_state and updates all rockets
 *  (i.e. position, explosion etc)                                            */
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
        case PowerupManager::POWERUP_BOWLING: f = new Bowling(kart); break;
        case PowerupManager::POWERUP_PLUNGER: f = new Plunger(kart); break;
        case PowerupManager::POWERUP_CAKE:    f = new Cake(kart);  break;
        default:              return NULL;
    }
    m_active_projectiles.push_back(f);
    return f;
}   // newProjectile

// -----------------------------------------------------------------------------
/** See if there is an old, unused explosion object available. If so,
 *  reuse this object, otherwise create a new one. */
Explosion* ProjectileManager::newExplosion(const Vec3& coord, 
                                           const char* explosion_sound)
{
    Explosion *e = new Explosion(coord, explosion_sound);
    m_active_explosions.push_back(e);
    return e;
}   // newExplosion

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
