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

#ifndef HEADER_PROJECTILEMANAGER_HPP
#define HEADER_PROJECTILEMANAGER_HPP

#include <vector>

namespace irr
{
    namespace scene { class IMesh; }
}

#include "audio/sfx_manager.hpp"
#include "items/powerup_manager.hpp"
#include "utils/no_copy.hpp"

class Vec3;
class Kart;
class Explosion;
class Flyable;

/**
  * \ingroup items
  */
class ProjectileManager : public NoCopy
{
private:
    typedef std::vector<Flyable*>   Projectiles;
    typedef std::vector<Explosion*> Explosions;

    // The list of all active projectiles, i.e. projectiles
    // which are currently moving on the track
    Projectiles      m_active_projectiles;

    // All active explosions, i.e. explosions which are currently
    // being shown
    Explosions       m_active_explosions;

    scene::IMesh    *m_explosion_model;
    bool             m_something_was_hit;
    bool             m_explosion_ended;
    void             updateClient(float dt);
    void             updateServer(float dt);
public:
                     ProjectileManager() {m_something_was_hit=false;}
                    ~ProjectileManager() {}
    /** Notifies the projectile manager that something needs to be removed. */
    void             notifyRemove     () {m_something_was_hit=true; }
    void             FinishedExplosion() {m_explosion_ended =true;  }
    scene::IMesh*     getExplosionModel()
    {
        return m_explosion_model;
    }
    unsigned int     getNumProjectiles() const {return m_active_explosions.size();}
    int              getProjectileId  (const std::string ident);
    void             loadData         ();
    void             cleanup          ();
    void             update           (float dt);
    Flyable*         newProjectile    (Kart *kart, 
                                       PowerupManager::PowerupType type);
    Explosion*       newExplosion     (const Vec3& coord, 
                                       const char* explosion_sound="explosion",
                                       bool is_player_kart_hit = false);
    void             Deactivate       (Flyable *p) {}
    void             removeTextures   ();
};

extern ProjectileManager *projectile_manager;

#endif

/* EOF */
