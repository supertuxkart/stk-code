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
        if((*he)->updateAndDelete(dt))
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

    Projectiles::iterator p = m_active_projectiles.begin();
    while(p!=m_active_projectiles.end())
    {
        bool can_be_deleted = (*p)->updateAndDelete(dt);
        if(network_manager->getMode()!=NetworkManager::NW_NONE)
        {
            race_state->setFlyableInfo(p-m_active_projectiles.begin(),
                                       FlyableInfo((*p)->getXYZ(), 
                                                   (*p)->getRotation(),
                                                   can_be_deleted)     );
        }
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
/** Updates all rockets and hit effects on the client.
 *  updateClient takes the information in race_state and updates all rockets
 *  (i.e. position, hit effects etc)                                          */
void ProjectileManager::updateClient(float dt)
{
    unsigned int num_projectiles = race_state->getNumFlyables();
    if(num_projectiles != m_active_projectiles.size())
        fprintf(stderr, "Warning: num_projectiles %d active %d\n",
                num_projectiles, (int)m_active_projectiles.size());

    unsigned int indx=0;
    for(Projectiles::iterator i  = m_active_projectiles.begin();
        i != m_active_projectiles.end();   ++i, ++indx)
    {
        const FlyableInfo &f = race_state->getFlyable(indx);
        (*i)->updateFromServer(f, dt);
        if(f.m_exploded) 
        {
            (*i)->hit(NULL);
        }
    }   // for i in m_active_projectiles

}   // updateClient
// -----------------------------------------------------------------------------
Flyable *ProjectileManager::newProjectile(AbstractKart *kart, Track* track,
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
