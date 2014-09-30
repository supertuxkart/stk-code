//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2013 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2013 Joerg Henrichs, Steve Baker
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


#ifndef HEADER_PLAYERKART_HPP
#define HEADER_PLAYERKART_HPP

#include "karts/controller/controller.hpp"

class AbstractKart;
class Camera;
class Player;
class SFXBase;

/** PlayerKart manages control events from the player and moves
  * them to the Kart
  *
  * \ingroup controller
  */
class PlayerController : public Controller
{
private:
    int            m_steer_val, m_steer_val_l, m_steer_val_r;
    int            m_prev_accel;
    bool           m_prev_brake;
    bool           m_prev_nitro;
    bool           m_sound_schedule;

    float          m_penalty_time;

    /** The camera attached to the kart for this controller. The camera
     *  object is managed in the Camera class, so no need to free it. */
    Camera        *m_camera;

    SFXBase       *m_bzzt_sound;
    SFXBase       *m_wee_sound;
    SFXBase       *m_ugh_sound;
    SFXBase       *m_grab_sound;
    SFXBase       *m_full_sound;

    void           steer(float, int);
public:
                   PlayerController  (AbstractKart *kart,
                                      StateManager::ActivePlayer *_player,
                                      unsigned int player_index);
                  ~PlayerController  ();
    void           update            (float);
    void           action            (PlayerAction action, int value);
    void           handleZipper      (bool play_sound);
    void           collectedItem     (const Item &item, int add_info=-1,
                                      float previous_energy=0);
    virtual void   skidBonusTriggered();
    virtual void   setPosition       (int p);
    virtual bool   isPlayerController() const {return true;}
    virtual bool   isNetworkController() const { return false; }
    virtual void   reset             ();
    void           resetInputState   ();
    virtual void   finishedRace      (float time);
    virtual void   crashed           (const AbstractKart *k) {}
    virtual void   crashed           (const Material *m) {}
    // ------------------------------------------------------------------------
    /** Callback whenever a new lap is triggered. Used by the AI
     *  to trigger a recomputation of the way to use, not used for players. */
    virtual void  newLap(int lap) {}
    // ------------------------------------------------------------------------
    /** Player will always be able to get a slipstream bonus. */
    virtual bool  disableSlipstreamBonus() const { return false; }

};   // PlayerController

#endif
