//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015  Joerg Henrichs
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

#ifndef HEADER_CONTROLLER_HPP
#define HEADER_CONTROLLER_HPP

#include <irrString.h>
using namespace irr;

class BareNetworkString;

/**
  * \defgroup controller Karts/controller
  * Contains kart controllers, which are either human players or AIs
  * (this module thus contains the AIs)
  */

#include "input/input.hpp"
#include "states_screens/state_manager.hpp"

class AbstractKart;
class BareNetworString;
class ItemState;
class KartControl;
class Material;

/** This is the base class for kart controller - that can be a player
 *  or a a robot.
 * \ingroup controller
 */
class Controller
{
private:

protected:
    /** Pointer to the kart that is controlled by this controller. */
    AbstractKart *m_kart;

    /** A pointer to the main controller, from which the kart takes
     *  it commands. */
    KartControl  *m_controls;

    /** The name of the controller, mainly used for debugging purposes. */
    std::string  m_controller_name;

public:
                  Controller         (AbstractKart *kart);
    virtual      ~Controller         () {};
    virtual void  reset              () = 0;
    virtual void  update             (int ticks) = 0;
    virtual void  handleZipper       (bool play_sound) = 0;
    virtual void  collectedItem      (const ItemState &item,
                                      float previous_energy=0) = 0;
    virtual void  crashed            (const AbstractKart *k) = 0;
    virtual void  crashed            (const Material *m) = 0;
    virtual void  setPosition        (int p) = 0;
    /** This function checks if this is a local player. A local player will get 
     *  special graphical effects enabled, has a camera, and sound effects will
     *  be played with normal volume. */
    virtual bool  isLocalPlayerController () const = 0;
    /** This function checks if this player is not an AI, i.e. it is either a
     *  a local or a remote/networked player. This is tested e.g. by the AI for
     *  rubber-banding. */
    virtual bool  isPlayerController () const = 0;
    virtual bool  disableSlipstreamBonus() const = 0;
    virtual bool  saveState(BareNetworkString *buffer) const = 0;
    virtual void  rewindTo(BareNetworkString *buffer) = 0;
    virtual void rumble(float strength_low, float strength_high, uint16_t duration) {}
    // ---------------------------------------------------------------------------
    /** Sets the controller name for this controller. */
    virtual void setControllerName(const std::string &name)
                                                 { m_controller_name = name; }
    // ---------------------------------------------------------------------------
    /** Returns the name of this controller. */
    const std::string &getControllerName() const { return m_controller_name; }
    // ------------------------------------------------------------------------
    /** Default: ignore actions. Only PlayerController get them. */
    virtual bool action(PlayerAction action, int value, bool dry_run=false) = 0;
    // ------------------------------------------------------------------------
    /** Callback whenever a new lap is triggered. Used by the AI
     *  to trigger a recomputation of the way to use.            */
    virtual void  newLap(int lap) = 0;
    // ------------------------------------------------------------------------
    virtual void  skidBonusTriggered() = 0;
    // ------------------------------------------------------------------------
    /** Called whan this controller's kart finishes the last lap. */
    virtual void  finishedRace(float time) = 0;
    // ------------------------------------------------------------------------
    /** Get a pointer on the kart controls. */
    virtual KartControl* getControls() { return m_controls; }
    // ------------------------------------------------------------------------
    void setControls(KartControl* kc) { m_controls = kc; }
    // ------------------------------------------------------------------------
    /** Only local players can get achievements. */
    virtual bool  canGetAchievements () const { return false; }
    // ------------------------------------------------------------------------
    /** Display name of the controller.
     *  Defaults to kart name; overriden by controller classes
     *  (such as player controllers) to display username. */
    virtual core::stringw getName(bool include_handicap_string = true) const;
    // ------------------------------------------------------------------------
    /** Returns the kart controlled by this controller. */
    AbstractKart *getKart() const { return m_kart; }
};   // Controller

#endif

/* EOF */
