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
#include "karts/controller/controller.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/rewind_manager.hpp"
#include "utils/stk_process.hpp"
#include "utils/string_utils.hpp"

#include <typeinfo>

//=============================================================================================
ProjectileManager* g_projectile_manager[PT_COUNT];
//---------------------------------------------------------------------------------------------
ProjectileManager* ProjectileManager::get()
{
    ProcessType type = STKProcess::getType();
    return g_projectile_manager[type];
}   // get

//---------------------------------------------------------------------------------------------
void ProjectileManager::create()
{
    ProcessType type = STKProcess::getType();
    g_projectile_manager[type] = new ProjectileManager();
}   // create

//---------------------------------------------------------------------------------------------
void ProjectileManager::destroy()
{
    ProcessType type = STKProcess::getType();
    delete g_projectile_manager[type];
    g_projectile_manager[type] = NULL;
}   // destroy

//---------------------------------------------------------------------------------------------
void ProjectileManager::clear()
{
    memset(g_projectile_manager, 0, sizeof(g_projectile_manager));
}   // clear

//---------------------------------------------------------------------------------------------
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
        if (!p->second->hasServerState())
        {
            p++;
            continue;
        }
        bool can_be_deleted = p->second->updateAndDelete(ticks);
        if (can_be_deleted)
        {
            HitEffect* he = p->second->getHitEffect();
            if (he)
                addHitEffect(he);

            p->second->onDeleteFlyable();
            // Flyables will be deleted by computeError in client
            if (!NetworkConfig::get()->isNetworking() ||
                NetworkConfig::get()->isServer())
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
    // Flyable has already created before and now rewinding, re-fire it
    if (it != m_active_projectiles.end())
    {
        it->second->onFireFlyable();
        return it->second;
    }

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
    // This cannot be done in constructor because of virtual function
    f->onFireFlyable();
    m_active_projectiles[uid] = f;
    if (RewindManager::get()->isEnabled())
        f->addForRewind(uid);

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
    for (auto i = m_active_projectiles.begin(); i != m_active_projectiles.end(); i++)
    {
        if (!i->second->hasServerState())
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
                                         float radius, PowerupManager::PowerupType type,
                                         bool exclude_owned)
{
    float r2 = radius * radius;
    int projectile_count = 0;
    for (auto i = m_active_projectiles.begin(); i != m_active_projectiles.end(); i++)
    {
        if (!i->second->hasServerState())
            continue;
        if (i->second->getType() == type)
        {
            if (exclude_owned && (i->second->getOwner() == kart))
                continue;

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
std::vector<Vec3> ProjectileManager::getBasketballPositions()
{
    std::vector<Vec3> positions;
    for (auto i = m_active_projectiles.begin(); i != m_active_projectiles.end(); i++)
    {
        if (!i->second->hasServerState())
            continue;
        if (i->second->getType() == PowerupManager::POWERUP_RUBBERBALL)
            positions.emplace_back(i->second->getXYZ());
    } // loop over projectiles

    return positions;
} // getBasketballPositions
// -----------------------------------------------------------------------------
std::string ProjectileManager::getUniqueIdentity(AbstractKart* kart,
                                                 PowerupManager::PowerupType t)
{
    BareNetworkString uid;
    switch (t)
    {
        case PowerupManager::POWERUP_BOWLING:
        {
            uid.addUInt8(RN_BOWLING);
            break;
        }
        case PowerupManager::POWERUP_PLUNGER:
        {
            uid.addUInt8(RN_PLUNGER);
            break;
        }
        case PowerupManager::POWERUP_CAKE:
        {
            uid.addUInt8(RN_CAKE);
            break;
        }
        case PowerupManager::POWERUP_RUBBERBALL:
        {
            uid.addUInt8(RN_RUBBERBALL);
            break;
        }
        default:
            assert(false);
            return "";
    }
    uid.addUInt8((uint8_t)kart->getWorldKartId())
        .addUInt32(World::getWorld()->getTicksSinceStart());
    return std::string((char*)uid.getBuffer().data(), uid.getBuffer().size());
}   // getUniqueIdentity

// -----------------------------------------------------------------------------
/* If any flyable is not found in current game state, create it with respect to
 * its uid as below. */
std::shared_ptr<Rewinder>
          ProjectileManager::addRewinderFromNetworkState(const std::string& uid)
{
    if (uid.size() != 6)
        return nullptr;
    BareNetworkString data(uid.data(), (int)uid.size());

    RewinderName rn = (RewinderName)data.getUInt8();
    if (!(rn == RN_BOWLING || rn == RN_PLUNGER ||
        rn == RN_CAKE || rn == RN_RUBBERBALL))
        return nullptr;

    AbstractKart* kart = World::getWorld()->getKart(data.getUInt8());
    int created_ticks = data.getUInt32();
    std::shared_ptr<Flyable> f;
    switch (rn)
    {
        case RN_BOWLING:
        {
            f = std::make_shared<Bowling>(kart);
            break;
        }
        case RN_PLUNGER:
        {
            f = std::make_shared<Plunger>(kart);
            break;
        }
        case RN_CAKE:
        {
            f = std::make_shared<Cake>(kart);
            break;
        }
        case RN_RUBBERBALL:
        {
            f = std::make_shared<RubberBall>(kart);
            break;
        }
        default:
        {
            break;
        }
    }
    assert(f);
    f->setCreatedTicks(created_ticks);
    f->onFireFlyable();
    f->addForRewind(uid);
    Flyable* flyable = f.get();
    Log::debug("ProjectileManager", "Missed a firing event, "
        "add the flyable %s by %s created at %d manually.",
        typeid(*flyable).name(),
        StringUtils::wideToUtf8(kart->getController()->getName()).c_str(),
        created_ticks);

    m_active_projectiles[uid] = f;
    return f;
}   // addProjectileFromNetworkState

