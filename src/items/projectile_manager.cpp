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
#include "modes/world.hpp"
#include "network/dummy_rewinder.hpp"
#include "network/rewind_manager.hpp"

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
    m_active_projectiles.clear();
    m_deleted_projectiles.clear();
    for(HitEffects::iterator i  = m_active_hit_effects.begin();
        i != m_active_hit_effects.end(); ++i)
    {
        delete *i;
    }

    m_active_hit_effects.clear();
}   // cleanup

// -----------------------------------------------------------------------------
/** Called once per rendered frame. It is used to only update any graphical
 *  effects, and calls updateGraphics in any flyable objects.
 *  \param dt Time step size (since last call).
 */
void ProjectileManager::updateGraphics(float dt)
{
    for (auto& p : m_active_projectiles)
        p.second->updateGraphics(dt);
}   // updateGraphics

// -----------------------------------------------------------------------------
/** General projectile update call. */
void ProjectileManager::update(int ticks)
{
    updateServer(ticks);

    if (RewindManager::get()->isRewinding())
        return;
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
        else if((*he)->updateAndDelete(ticks))
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
void ProjectileManager::updateServer(int ticks)
{
    auto p = m_active_projectiles.begin();
    while (p != m_active_projectiles.end())
    {
        if (p->second->isUndoCreation())
        {
            p++;
            continue;
        }
        bool can_be_deleted = p->second->updateAndDelete(ticks);
        if (can_be_deleted)
        {
            if (!p->second->hasUndoneDestruction())
            {
                HitEffect *he = p->second->getHitEffect();
                if (he)
                    addHitEffect(he);
            }
            p->second->handleUndoDestruction();
            p = m_active_projectiles.erase(p);
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
std::shared_ptr<Flyable>
    ProjectileManager::newProjectile(AbstractKart *kart,
                                     PowerupManager::PowerupType type)
{
    const std::string& uid = getUniqueIdentity(kart, type);
    auto it = m_active_projectiles.find(uid);
    // Flyable already created during rewind
    if (it != m_active_projectiles.end())
        return it->second;

    std::shared_ptr<Flyable> f;
    switch(type)
    {
        case PowerupManager::POWERUP_BOWLING:
            f = std::make_shared<Bowling>(kart);
            break;
        case PowerupManager::POWERUP_PLUNGER:
            f = std::make_shared<Plunger>(kart);
            break;
        case PowerupManager::POWERUP_CAKE:
            f = std::make_shared<Cake>(kart);
            break;
        case PowerupManager::POWERUP_RUBBERBALL:
            f = std::make_shared<RubberBall>(kart);
            break;
        default:
            return nullptr;
    }
    m_active_projectiles[uid] = f;
    if (RewindManager::get()->isEnabled())
    {
        f->addForRewind(uid);
        f->addRewindInfoEventFunctionAfterFiring();
    }
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
    float r2 = radius * radius;
    for (auto i = m_active_projectiles.begin(); i != m_active_projectiles.end();
        i++)
    {
        if (i->second->isUndoCreation())
            continue;
        float dist2 = i->second->getXYZ().distance2(kart->getXYZ());
        if (dist2 < r2)
            return true;
    }
    return false;
}   // projectileIsClose

// -----------------------------------------------------------------------------
/** Returns an int containing the numbers of a given flyable in a given radius
 *  around the kart
 *  \param kart The kart for which the test is done.
 *  \param radius Distance within which the projectile must be.
 *  \param type The type of projectile checked
*/
int ProjectileManager::getNearbyProjectileCount(const AbstractKart * const kart,
                                         float radius, PowerupManager::PowerupType type)
{
    float r2 = radius * radius;
    int projectile_count = 0;
    for (auto i = m_active_projectiles.begin(); i != m_active_projectiles.end();
         i++)
    {
        if (i->second->isUndoCreation())
            continue;
        if (i->second->getType() == type)
        {
            float dist2 = i->second->getXYZ().distance2(kart->getXYZ());
            if (dist2 < r2)
            {
                projectile_count++;
            }
        }
    }
    return projectile_count;
}   // getNearbyProjectileCount

// -----------------------------------------------------------------------------
std::string ProjectileManager::getUniqueIdentity(AbstractKart* kart,
                                                 PowerupManager::PowerupType t)
{
    switch (t)
    {
        case PowerupManager::POWERUP_BOWLING:
            return std::string("B_") +
                StringUtils::toString(kart->getWorldKartId()) + "_" +
                StringUtils::toString(World::getWorld()->getTicksSinceStart());
        case PowerupManager::POWERUP_PLUNGER:
            return std::string("P_") +
                StringUtils::toString(kart->getWorldKartId()) + "_" +
                StringUtils::toString(World::getWorld()->getTicksSinceStart());
        case PowerupManager::POWERUP_CAKE:
            return std::string("C_") +
                StringUtils::toString(kart->getWorldKartId()) + "_" +
                StringUtils::toString(World::getWorld()->getTicksSinceStart());
        case PowerupManager::POWERUP_RUBBERBALL:
            return std::string("R_") +
                StringUtils::toString(kart->getWorldKartId()) + "_" +
                StringUtils::toString(World::getWorld()->getTicksSinceStart());
        default:
            assert(false);
            return "";
    }
}   // getUniqueIdentity

// -----------------------------------------------------------------------------
std::shared_ptr<Rewinder>
          ProjectileManager::addRewinderFromNetworkState(const std::string& uid)
{
    std::vector<std::string> id = StringUtils::split(uid, '_');
    if (id.size() != 3)
        return nullptr;
    if (!(id[0] == "B" || id[0] == "P" || id[0] == "C" || id[0] == "R"))
        return nullptr;
    int world_id = -1;
    if (!StringUtils::fromString(id[1], world_id))
        return nullptr;
    AbstractKart* kart = World::getWorld()->getKart(world_id);
    char first_id = id[0][0];

    auto it = m_deleted_projectiles.find(uid);
    if (it != m_deleted_projectiles.end())
    {
        Log::debug("ProjectileManager", "Flyable %s locally (early) deleted,"
            " use a dummy rewinder to skip.", uid.c_str());
        return std::make_shared<DummyRewinder>();
    }

    Log::debug("ProjectileManager",
        "Missed a firing event, add the flyable %s manually.", uid.c_str());
    switch (first_id)
    {
        case 'B':
        {
            auto f = std::make_shared<Bowling>(kart);
            f->addForRewind(uid);
            m_active_projectiles[uid] = f;
            return f;
        }
        case 'P':
        {
            auto f = std::make_shared<Plunger>(kart);
            f->addForRewind(uid);
            m_active_projectiles[uid] = f;
            return f;
        }
        case 'C':
        {
            auto f = std::make_shared<Cake>(kart);
            f->addForRewind(uid);
            m_active_projectiles[uid] = f;
            return f;
        }
        case 'R':
        {
            auto f = std::make_shared<RubberBall>(kart);
            f->addForRewind(uid);
            m_active_projectiles[uid] = f;
            return f;
        }
        default:
            assert(false);
            return nullptr;
    }
}   // addProjectileFromNetworkState
