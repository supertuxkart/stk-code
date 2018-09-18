//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Joerg Henrichs
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

#include "karts/abstract_kart_animation.hpp"

#include "graphics/slip_stream.hpp"
#include "items/powerup.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_model.hpp"
#include "karts/skidding.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/rewind_info.hpp"
#include "network/rewind_manager.hpp"
#include "physics/physics.hpp"

#include <limits>

/** Constructor. Note that kart can be NULL in case that the animation is
 *  used for a basket ball in a cannon animation.
 *  \param kart Pointer to the kart that is animated, or NULL if the
 *         the animation is meant for a basket ball etc.
 */
AbstractKartAnimation::AbstractKartAnimation(AbstractKart *kart,
                                             const std::string &name)
{
    m_timer = 0;
    m_kart  = kart;
    m_name  = name;
    m_end_transform = btTransform(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f));
    m_end_transform.setOrigin(Vec3(std::numeric_limits<float>::max()));
    m_end_ticks = -1;
    m_created_ticks = World::getWorld()->getTicksSinceStart();
    m_check_created_ticks = std::make_shared<int>(-1);
    m_confirmed_by_network = false;
    m_ignore_undo = false;
    // Remove previous animation if there is one
#ifndef DEBUG
    // Use this code in non-debug mode to avoid a memory leak (and messed
    // up animations) if this should happen. In debug mode this condition
    // is caught by setKartAnimation(), and useful error messages are
    // printed
    if (kart && kart->getKartAnimation())
    {
        AbstractKartAnimation* ka = kart->getKartAnimation();
        kart->setKartAnimation(NULL);
        delete ka;
    }
#endif
    // Register this animation with the kart (which will free it
    // later).
    if (kart)
    {
        kart->setKartAnimation(this);
        Physics::getInstance()->removeKart(m_kart);
        kart->getSkidding()->reset();
        kart->getSlipstream()->reset();
        if (kart->isSquashed())
        {
            // A time of 0 reset the squashing
            kart->setSquash(0.0f, 0.0f);
        }

        // Reset the wheels (and any other animation played for that kart)
        // This avoid the effect that some wheels might be way below the kart
        // which is very obvious in the rescue animation.
        m_kart->getKartModel()->resetVisualWheelPosition();
    }
}   // AbstractKartAnimation

// ----------------------------------------------------------------------------
AbstractKartAnimation::~AbstractKartAnimation()
{
    // If m_timer >=0, this object is deleted because the kart
    // is deleted (at the end of a race), which means that
    // world is in the process of being deleted. In this case
    // we can't call getPhysics() anymore.
    if(m_timer < 0 && m_kart)
    {
        m_kart->getBody()->setAngularVelocity(btVector3(0,0,0));
        Vec3 linear_velocity = m_kart->getBody()->getLinearVelocity();
        btTransform transform = m_end_transform.getOrigin().x() ==
            std::numeric_limits<float>::max() ?
            m_kart->getBody()->getWorldTransform() : m_end_transform;
        m_kart->getBody()->setLinearVelocity(linear_velocity);
        m_kart->getBody()->proceedToTransform(transform);
        m_kart->setTrans(transform);
        Physics::getInstance()->addKart(m_kart);

        if (RewindManager::get()->useLocalEvent() && !m_ignore_undo)
        {
            AbstractKart* kart = m_kart;
            Vec3 angular_velocity = kart->getBody()->getAngularVelocity();
            RewindManager::get()->addRewindInfoEventFunction(new
                RewindInfoEventFunction(
                World::getWorld()->getTicksSinceStart(),
                [kart]()
                {
                    Physics::getInstance()->removeKart(kart);
                },
                [kart, linear_velocity, angular_velocity, transform]()
                {
                    kart->getBody()->setAngularVelocity(angular_velocity);
                    kart->getBody()->setLinearVelocity(linear_velocity);
                    kart->getBody()->proceedToTransform(transform);
                    kart->setTrans(transform);
                    Physics::getInstance()->addKart(kart);
                }));
        }
    }
}   // ~AbstractKartAnimation

// ----------------------------------------------------------------------------
void AbstractKartAnimation::addNetworkAnimationChecker(bool reset_powerup)
{
    Powerup* p = NULL;
    if (reset_powerup)
    {
        if (m_kart)
        {
            p = m_kart->getPowerup();
            p->reset();
        }
    }

    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isClient())
    {
        // Prevent access to deleted kart animation object
        std::weak_ptr<int> cct = m_check_created_ticks;
        RewindManager::get()->addRewindInfoEventFunction(new
            RewindInfoEventFunction(m_created_ticks,
            [](){},
            /*replay_function*/[p]()
            {
                if (p)
                    p->reset();
            },
            /*delete_function*/[cct]()
            {
                auto cct_sp = cct.lock();
                if (!cct_sp)
                    return;
                *cct_sp = World::getWorld()->getTicksSinceStart();
            }));
    }
}   // addNetworkAnimationChecker

// ----------------------------------------------------------------------------
void AbstractKartAnimation::
    checkNetworkAnimationCreationSucceed(const btTransform& fallback_trans)
{
    if (!m_confirmed_by_network && *m_check_created_ticks != -1 &&
        World::getWorld()->getTicksSinceStart() > *m_check_created_ticks)
    {
        Log::warn("AbstractKartAnimation",
            "No animation has been created on server, remove locally.");
        m_timer = -1;
        m_end_transform = fallback_trans;
        m_ignore_undo = true;
    }
}   // checkNetworkAnimationCreationSucceed

// ----------------------------------------------------------------------------
/** Updates the timer, and if it expires (<0), the kart animation will be
 *  removed from the kart and this object will be deleted.
 *  NOTE: calling this function must be the last thing done in any kart
 *  animation class, since this object might be deleted, so accessing any
 *  members might be invalid.
 *  \param ticks Number of time steps - should be 1.
 */
void AbstractKartAnimation::update(int ticks)
{
    // Scale the timer according to m_end_ticks told by server if
    // necessary
    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isClient() &&
        World::getWorld()->getPhase() == World::RACE_PHASE &&
        usePredefinedEndTransform() && m_end_ticks != -1)
    {
        int cur_end_ticks = World::getWorld()->getTicksSinceStart() +
            m_timer;

        const int difference = cur_end_ticks - m_end_ticks;
        if (World::getWorld()->getTicksSinceStart() > m_end_ticks)
        {
            // Stop right now
            m_timer = -1;
        }
        else if (difference > 0)
        {
            // Speed up
            m_timer -= ticks;
        }
        else if (difference < 0)
        {
            // Slow down
            return;
        }
    }
    // See if the timer expires, if so return the kart to normal game play
    m_timer -= ticks;
    if (m_timer < 0)
    {
        if(m_kart) m_kart->setKartAnimation(NULL);
        delete this;
    }
}   // update
