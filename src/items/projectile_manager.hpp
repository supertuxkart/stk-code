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

#ifndef HEADER_PROJECTILEMANAGER_HPP
#define HEADER_PROJECTILEMANAGER_HPP

#include <vector>

namespace irr
{
    namespace scene { class IMesh; }
}

#include "items/powerup_manager.hpp"
#include "utils/no_copy.hpp"

class AbstractKart;
class Flyable;
class HitEffect;
class Track;
class Vec3;

/**
  * \ingroup items
  */
class ProjectileManager : public NoCopy
{
private:
    typedef std::vector<Flyable*>   Projectiles;
    typedef std::vector<HitEffect*> HitEffects;

    /** The list of all active projectiles, i.e. projectiles which are
     *  currently moving on the track. */
    Projectiles      m_active_projectiles;

    /** All active hit effects, i.e. hit effects which are currently
     *  being shown or have a sfx playing. */
    HitEffects       m_active_hit_effects;

    void             updateServer(float dt);
public:
                     ProjectileManager() {}
                    ~ProjectileManager() {}
    void             loadData         ();
    void             cleanup          ();
    void             update           (float dt);
    Flyable*         newProjectile    (AbstractKart *kart,
                                       PowerupManager::PowerupType type);
    void             Deactivate       (Flyable *p) {}
    void             removeTextures   ();
    bool             projectileIsClose(const AbstractKart * const kart,
                                       float radius);
    // ------------------------------------------------------------------------
    /** Adds a special hit effect to be shown.
     *  \param hit_effect The hit effect to be added. */
    void             addHitEffect(HitEffect *hit_effect)
                                { m_active_hit_effects.push_back(hit_effect); }
};

extern ProjectileManager *projectile_manager;

#endif

/* EOF */
