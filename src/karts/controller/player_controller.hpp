//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2015 Joerg Henrichs, Steve Baker
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

#ifndef HEADER_PLAYER_CONTROLLER_HPP
#define HEADER_PLAYER_CONTROLLER_HPP

#include "karts/controller/controller.hpp"

class AbstractKart;
class Player;

class PlayerController : public Controller
{
friend class KartRewinder;
protected:
    int            m_steer_val, m_steer_val_l, m_steer_val_r;
    uint16_t       m_prev_accel;
    bool           m_prev_brake;
    bool           m_prev_nitro;

    int            m_penalty_ticks;

    virtual void  steer(int ticks, int steer_val);

public:
                 PlayerController(AbstractKart *kart);
    virtual     ~PlayerController  ();
    virtual void update            (int ticks) OVERRIDE;
    virtual bool action            (PlayerAction action, int value,
                                    bool dry_run = false           ) OVERRIDE;
    virtual void actionFromNetwork(PlayerAction action, int value,
                                   int value_l, int value_r);
    virtual void skidBonusTriggered() OVERRIDE;
    virtual void reset             () OVERRIDE;
    virtual void handleZipper(bool play_sound) OVERRIDE;
    virtual void resetInputState();
    virtual bool saveState(BareNetworkString *buffer) const OVERRIDE;
    virtual void rewindTo(BareNetworkString *buffer) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void  collectedItem(const ItemState &item,
                                float previous_energy=0 ) OVERRIDE { };
    // ------------------------------------------------------------------------
    virtual bool isPlayerController() const OVERRIDE { return true; }
    // ------------------------------------------------------------------------
    virtual bool isLocalPlayerController() const OVERRIDE { return true; }
    // ------------------------------------------------------------------------
    /** Called just before the position of the kart is changed. */
    virtual void setPosition(int p) OVERRIDE
    {
    }   // setPosition
    // ------------------------------------------------------------------------
    virtual void crashed(const AbstractKart *k) OVERRIDE
    {
    }   // crashed(AbstractKart)
    // ------------------------------------------------------------------------
    virtual void crashed(const Material *m) OVERRIDE
    {
    }   // crashed(Material)
    // ------------------------------------------------------------------------
    /** Callback whenever a new lap is triggered. Used by the AI
     *  to trigger a recomputation of the way to use, not used for players. */
    virtual void  newLap(int lap) OVERRIDE
    {
    }
    // ------------------------------------------------------------------------
    /** Player will always be able to get a slipstream bonus. */
    virtual bool  disableSlipstreamBonus() const OVERRIDE
    {
        return false; 
    }
    // ------------------------------------------------------------------------
    /** Called when a race is finished. */
    virtual void finishedRace(float time) OVERRIDE
    {
    }   // finishedRace
    // ------------------------------------------------------------------------
    /** Returns the name of the player profile. */
    core::stringw getName(bool include_handicap_string = true) const OVERRIDE;
    // ------------------------------------------------------------------------
    /** Called when this kart started too early and got a start penalty. */
    virtual void  displayPenaltyWarning();

};   // class PlayerController

#endif // HEADER_PLAYER_CONTROLLER_HPP
