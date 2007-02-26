//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#include "world.hpp"
#include "loader.hpp"
#include "projectile_manager.hpp"
#include "projectile.hpp"
#include "explosion.hpp"
#include "collectable_manager.hpp"
#include "collectable.hpp"

static ssgSelector *find_selector ( ssgBranch *b );

ProjectileManager *projectile_manager=0;

void ProjectileManager::loadData()
{

    // Load the explosion model and find the actual selector branch in it.
    // Only the explosion model is loaded here, see collectable_manager.
    m_explosion_model = find_selector((ssgBranch*)loader->load("explode.ac",
                                   CB_EXPLOSION) );
    m_explosion_model->ref();
    if ( m_explosion_model == NULL )
    {
        fprintf ( stderr, "explode.ac doesn't have an 'explosion' object.\n" ) ;
        exit ( 1 ) ;
    }

}   // loadData

//-----------------------------------------------------------------------------
void ProjectileManager::removeTextures()
{
    cleanup();
    ssgDeRefDelete(m_explosion_model);
    // Only the explosion is here, all other models are actualy managed
    // by collectable_manager.
    callback_manager->clear(CB_EXPLOSION);
}   // removeTextures

//-----------------------------------------------------------------------------
void ProjectileManager::cleanup()
{
    for(Projectiles::iterator i = m_active_projectiles.begin();
        i != m_active_projectiles.end(); ++i)
    {
        ssgTransform *m = (*i)->getModel();
        m->removeAllKids();
        delete *i;
    }
    m_active_projectiles.clear();
    for(Explosions::iterator i  = m_active_explosions.begin();
        i != m_active_explosions.end(); ++i)
    {
        world->removeFromScene((ssgTransform*)*i);
        ssgDeRefDelete(*i);
    }
    m_active_explosions.clear();
}   // cleanup

/** General projectile update call. */
void ProjectileManager::update(float dt)
{
    m_something_was_hit=false;
    // First update all projectiles on the track
    for(Projectiles::iterator i  = m_active_projectiles.begin();
        i != m_active_projectiles.end(); ++i)
    {
        (*i)->update(dt);
    }
    // Then check if any projectile hit something
    if(m_something_was_hit)
    {
        Projectiles::iterator p = m_active_projectiles.begin();
        while(p!=m_active_projectiles.end())
        {
            if(! (*p)->hasHit()) { p++; continue; }
            newExplosion((*p)->getCoord());
            // Create a new explosion, move the projectile to the
            // list of deleted projectiles (so that they can be
            // reused later), and remove it from the list of active
            // projectiles.
            m_deleted_projectiles.push_back(*p);
            p=m_active_projectiles.erase(p);  // returns the next element
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
            m_deleted_explosions.push_back(*e);
            e=m_active_explosions.erase(e);
        }   // while e!=m_active_explosions.end()
    }   // if m_explosion_ended

}   // update

/** See if there is an old, unused projectile object available. If so,
 *  reuse this object, otherwise create a new one. */
Projectile *ProjectileManager::newProjectile(Kart *kart, int type)
{
    Projectile *p;
    if(m_deleted_projectiles.size()>0)
    {
        p = m_deleted_projectiles.back();
        m_deleted_projectiles.pop_back();
        p->init(kart, type);
    }
    else
    {
        p=new Projectile(kart, type);
    }
    m_active_projectiles.push_back(p);
    return p;

}   // newProjectile

/** See if there is an old, unused explosion object available. If so,
 *  reuse this object, otherwise create a new one. */
Explosion* ProjectileManager::newExplosion(sgCoord* coord)
{
    Explosion *e;
    if(m_deleted_explosions.size()>0)
    {
        e = m_deleted_explosions.back();
        m_deleted_explosions.pop_back();
        e->init(coord);
    }
    else
    {
        e=new Explosion(coord);
    }
    m_active_explosions.push_back(e);
    return e;
}   // newExplosion

// =============================================================================
/** A general function which is only needed here, but
 *  it's not really a method, so I'll leave it here.
 */
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
