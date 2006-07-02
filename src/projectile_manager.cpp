//  $Id: ProjectileManager.cxx,v 1.5 2005/08/17 22:36:30 joh Exp $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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

#include "loader.hpp"
#include "projectile_manager.hpp"
#include "projectile.hpp"
#include "explosion.hpp"
#include "collectable_manager.hpp"
#include "collectable.hpp"

static ssgSelector *find_selector ( ssgBranch *b );

ProjectileManager *projectile_manager=0;

// -----------------------------------------------------------------------------
void ProjectileManager::loadData() {

  // Load the explosion model and find the actual selector branch in int
  explosionModel = find_selector((ssgBranch*)ssgLoad("explode.ac", loader));
  if ( explosionModel == NULL ) {
    fprintf ( stderr, "explode.ac doesn't have an 'explosion' object.\n" ) ;
    exit ( 1 ) ;
  }
  
}   // loadData

// -----------------------------------------------------------------------------
void ProjectileManager::cleanup() {
  for(Projectiles::iterator i = activeProjectiles.begin();
                            i != activeProjectiles.end(); ++i) {
    delete *i;
  }
  activeProjectiles.clear();
  for(Explosions::iterator i  = activeExplosions.begin(); 
                           i != activeExplosions.end(); ++i) {
    delete *i;
  }
  activeExplosions.clear();
}   // cleanup

// -----------------------------------------------------------------------------
// General update call
void ProjectileManager::update(float dt) {
  somethingWasHit=false;
  // First update all projectiles on the track
  for(Projectiles::iterator i  = activeProjectiles.begin(); 
                            i != activeProjectiles.end(); ++i) {
    (*i)->update(dt);
  }
  // Then check if any projectile hit something
  if(somethingWasHit) {
    Projectiles::iterator p;
    p = activeProjectiles.begin();
    while(p!=activeProjectiles.end()) {
      if(! (*p)->hasHit()) { p++; continue; }
      newExplosion(*p);
      // Create a new explosion, move the projectile to the
      // list of deleted projectiles (so that they can be 
      // reused later), and remove it from the list of active
      // projectiles.
      deletedProjectiles.push_back(*p);
      p=activeProjectiles.erase(p);  // returns the next element
    }   // while p!=activeProjectiles.end()
  }
  
  explosionEnded=false;
  for(Explosions::iterator i  = activeExplosions.begin(); 
                           i != activeExplosions.end(); ++i) {
    (*i)->update(dt);
  }
  if(explosionEnded) {
    Explosions::iterator e;
    e = activeExplosions.begin();
    while(e!=activeExplosions.end()) {
      if(!(*e)->hasEnded()) { e++; continue;}
      deletedExplosions.push_back(*e);
      e=activeExplosions.erase(e);
    }   // while e!=activeExplosions.end()
  }   // if explosionEnded
  
}   // update

// -----------------------------------------------------------------------------
// See if there is an old, unused projectile object available. If so,
// reuse this object, otherwise create a new one.
Projectile *ProjectileManager::newProjectile(Kart *kart, int type) {
  Projectile *p;
  if(deletedProjectiles.size()>0) {
    p = deletedProjectiles.back();
    deletedProjectiles.pop_back();
    p->init(kart, type);
  } else {
    p=new Projectile(kart, type);
  }
  activeProjectiles.push_back(p);
  return p;
	      
}   // newProjectile

// -----------------------------------------------------------------------------
// See if there is an old, unused explosion object available. If so,
// reuse this object, otherwise create a new one.
Explosion* ProjectileManager::newExplosion(Projectile* p) {
  Explosion *e;
  if(deletedExplosions.size()>0) {
    e = deletedExplosions.back();
    deletedExplosions.pop_back();
    e->init(p);
  } else {
    e=new Explosion(p);
  }
  activeExplosions.push_back(e);
  return e;
}   // newExplosion

// =============================================================================
// A general function which is only neede here, but
// it's not really a method, so I'll leave it here.
static ssgSelector *find_selector ( ssgBranch *b ) {
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
