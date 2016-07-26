//
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


#ifndef HEADER_LOCAL_PLAYER_CONTROLLER_HPP
#define HEADER_LOCAL_PLAYER_CONTROLLER_HPP

#include "karts/controller/player_controller.hpp"

class AbstractKart;
class Player;
class SFXBase;

/** PlayerKart manages control events from the player and moves
  * them to the Kart
  *
  * \ingroup controller
  */
class LocalPlayerController : public PlayerController
{
private:

    /** Stores the active player data structure. */
    StateManager::ActivePlayer *m_player;

    bool           m_sound_schedule;


    /** The index of the camera attached to the kart for this controller. The
     *  camera object is managed in the Camera class, so no need to free it. */
    int  m_camera_index;

    SFXBase       *m_bzzt_sound;
    SFXBase       *m_wee_sound;
    SFXBase       *m_ugh_sound;
    SFXBase       *m_grab_sound;
    SFXBase       *m_full_sound;

    virtual void steer(float, int) OVERRIDE;
    virtual void displayPenaltyWarning() OVERRIDE;
public:
                 LocalPlayerController(AbstractKart *kart,
                                       StateManager::ActivePlayer *player);
                ~LocalPlayerController();
    void         update            (float) OVERRIDE;
    void         action            (PlayerAction action, int value) OVERRIDE;
    virtual void handleZipper      (bool play_sound) OVERRIDE;
    void         collectedItem     (const Item &item, int add_info=-1,
                                    float previous_energy=0) OVERRIDE;
    virtual void setPosition       (int p) OVERRIDE;
    virtual void reset             () OVERRIDE;
    virtual void finishedRace      (float time) OVERRIDE;
    virtual void resetInputState   () OVERRIDE;
    virtual bool canGetAchievements() const OVERRIDE;

    // ------------------------------------------------------------------------
    virtual bool isPlayerController() const OVERRIDE {return true;}
    // ------------------------------------------------------------------------
    virtual bool isLocalPlayerController() const OVERRIDE {return true;}
    // ------------------------------------------------------------------------
    /** Returns the name of the player profile. */
    core::stringw getName() const OVERRIDE { return m_player->getProfile()->getName(); }


};   // LocalPlayerController

#endif
