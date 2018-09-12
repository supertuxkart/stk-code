//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 Joerg Henrichs
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

#ifndef HEADER_ABSTRACT_KART_ANIMATION_HPP
#define HEADER_ABSTRACT_KART_ANIMATION_HPP

#include "LinearMath/btTransform.h"

#include "config/stk_config.hpp"
#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"

#include <memory>
#include <string>

class AbstractKart;

/** The base class for all kart animation, like rescue, explosion, or cannon.
 *  Kart animations are done by removing the physics body from the physics
 *  world, and instead modifying the rotation and position of the kart
 *  directly. They are registered with the kart, and only one can be
 *  used at the same time. The memory is handled by the kart object, so
 *  there is no need to manage it. Sample usage:
 *    new ExplosionAnimation(kart);
 *  The object does not need to be stored.
 */
class AbstractKartAnimation: public NoCopy
{
private:
    /** Name of this animation, used for debug prints only. */
    std::string m_name;

    std::shared_ptr<int> m_check_created_ticks;

    int m_created_ticks;

    bool m_confirmed_by_network;

    bool m_ignore_undo;

protected:
   /** A pointer to the kart which is animated by this class. */
    AbstractKart *m_kart;

    /** Timer for the animation. */
    int m_timer;

    btTransform m_end_transform;

    int m_end_ticks;

    // ------------------------------------------------------------------------
    void addNetworkAnimationChecker(bool reset_powerup);
public:
                 AbstractKartAnimation(AbstractKart *kart,
                                       const std::string &name);
    virtual     ~AbstractKartAnimation();
    virtual void update(int ticks);
    // ------------------------------------------------------------------------
    /** Returns the current animation timer. */
    virtual float getAnimationTimer() const
                                    { return stk_config->ticks2Time(m_timer); }
    // ------------------------------------------------------------------------
    /** To easily allow printing the name of the animation being used atm.
     *  Used in AstractKart in case of an incorrect sequence of calls. */
    virtual const std::string &getName() const { return m_name; }
    // ------------------------------------------------------------------------
    /** If true, the end transform can be determined early, used in network for
     *  for client to synchronize with server. */
    virtual bool usePredefinedEndTransform() const { return true; }
    // ------------------------------------------------------------------------
    const btTransform& getEndTransform() const { return m_end_transform; }
    // ------------------------------------------------------------------------
    void setEndTransformTicks(const btTransform& t, int ticks)
    {
        if (!usePredefinedEndTransform() || m_confirmed_by_network)
            return;
        m_confirmed_by_network = true;
        m_end_transform = t;
        m_end_ticks = ticks;
    }
    // ----------------------------------------------------------------------------
    void checkNetworkAnimationCreationSucceed(const btTransform& fallback_trans);
    // ----------------------------------------------------------------------------
    int getEndTicks() const { return m_end_ticks; }
};   // AbstractKartAnimation

#endif
