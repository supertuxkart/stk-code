//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2024 SuperTuxKart-Team
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

#ifndef HEADER_ROCKET_CONTROLLER_HPP
#define HEADER_ROCKET_CONTROLLER_HPP

#include "karts/controller/ai_base_lap_controller.hpp"

class LinearWorld;

/**
 * \brief Controller that provides autopilot rocket boost for catch-up.
 *
 * When activated, this controller takes over from the player controller,
 * uses simple AI pathfinding to drive the kart (no drifting), and applies
 * a 300% speed boost. Terminates after minimum 8 seconds, or up to 12
 * seconds if still in bottom half of positions.
 *
 * \ingroup controller
 */
class RocketController : public AIBaseLapController
{
private:
    /** The original controller that will be restored when rocket ends. */
    Controller* m_original_controller;

    /** Time in ticks when the rocket boost started. */
    int m_start_ticks;

    /** Minimum duration in ticks (8 seconds). */
    static int MIN_DURATION_TICKS;

    /** Maximum duration in ticks (12 seconds). */
    static int MAX_DURATION_TICKS;

    /** Speed multiplier (300%). */
    static constexpr float SPEED_MULTIPLIER = 3.0f;

    /** Whether the rocket effect is still active. */
    bool m_is_active;

    /** Check if termination conditions are met. */
    bool shouldTerminate() const;

    /** Terminate the rocket boost and restore original controller. */
    void terminate();

    /** Handle steering - simple pathfinding without drifting. */
    void handleSteering(float dt);

    /** Find a point to steer toward that won't crash. */
    void findNonCrashingPoint(Vec3 *result);

public:
    RocketController(AbstractKart *kart, Controller* original_controller);
    virtual ~RocketController();

    virtual void update(int ticks) override;
    virtual void reset() override;

    virtual bool isLocalPlayerController() const override;
    virtual bool isPlayerController() const override;
    virtual bool disableSlipstreamBonus() const override { return false; }
    virtual bool saveState(BareNetworkString *buffer) const override;
    virtual void rewindTo(BareNetworkString *buffer) override;
    virtual bool action(PlayerAction action, int value,
                        bool dry_run = false) override;
    virtual void newLap(int lap) override;
    virtual void skidBonusTriggered() override;
    virtual void finishedRace(float time) override;
    virtual void crashed(const AbstractKart *k) override;
    virtual void crashed(const Material *m) override;
    virtual void handleZipper(bool play_sound) override;
    virtual void collectedItem(const ItemState &item,
                               float previous_energy = 0) override;
    virtual void setPosition(int p) override;

    /** Returns whether the rocket is still active. */
    bool isActive() const { return m_is_active; }

    /** Returns the original controller to restore. */
    Controller* getOriginalController() const { return m_original_controller; }

protected:
    /** Disable skidding during rocket boost for stability. */
    virtual bool canSkid(float steer_fraction) override { return false; }
};

#endif // HEADER_ROCKET_CONTROLLER_HPP
