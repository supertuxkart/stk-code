//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Joerg Henrichs
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

#ifndef HEADER_KART_REWINDER_HPP
#define HEADER_KART_REWINDER_HPP

#include "karts/kart.hpp"
#include "network/rewinder.hpp"
#include "utils/cpp2011.hpp"

class AbstractKart;
class BareNetworkString;

class KartRewinder : public Rewinder, public Kart
{
private:
    float m_prev_steering, m_steering_smoothing_dt, m_steering_smoothing_time;

    bool m_has_server_state;
public:
    KartRewinder(const std::string& ident, unsigned int world_kart_id,
                 int position, const btTransform& init_transform,
                 HandicapLevel handicap,
                 std::shared_ptr<GE::GERenderInfo> ri);
    ~KartRewinder() {}
    virtual void saveTransform() OVERRIDE;
    virtual void computeError() OVERRIDE;
    virtual BareNetworkString* saveState(std::vector<std::string>* ru)
        OVERRIDE;
    void reset() OVERRIDE;
    virtual void restoreState(BareNetworkString *p, int count) OVERRIDE;
    virtual void rewindToEvent(BareNetworkString *p) OVERRIDE {}
    virtual void update(int ticks) OVERRIDE;
    // -------------------------------------------------------------------------
    virtual float getSteerPercent() const OVERRIDE
    {
        if (m_steering_smoothing_dt >= 0.0f)
        {
            return m_steering_smoothing_dt * AbstractKart::getSteerPercent() +
                (1.0f - m_steering_smoothing_dt) * m_prev_steering;
        }
        return AbstractKart::getSteerPercent();
    }
    // -------------------------------------------------------------------------
    virtual void updateGraphics(float dt) OVERRIDE
    {
        if (m_steering_smoothing_dt >= 0.0f)
        {
            m_steering_smoothing_dt += dt / m_steering_smoothing_time;
            if (m_steering_smoothing_dt > 1.0f)
                m_steering_smoothing_dt = -1.0f;
        }
        Kart::updateGraphics(dt);
    }
    // -------------------------------------------------------------------------
    virtual void undoState(BareNetworkString *p) OVERRIDE {}
    // -------------------------------------------------------------------------
    virtual void undoEvent(BareNetworkString *p) OVERRIDE {}
    // ------------------------------------------------------------------------
    virtual std::function<void()> getLocalStateRestoreFunction() OVERRIDE;


};   // Rewinder
#endif

