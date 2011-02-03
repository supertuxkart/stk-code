//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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


#ifndef STATE_MANAGER_HPP
#define STATE_MANAGER_HPP

/** \defgroup states_screens */

#include <string>
#include "guiengine/abstract_state_manager.hpp"
#include "utils/ptr_vector.hpp"
#include "config/player.hpp"

struct Input;
class InputDevice;
class Kart;

namespace GUIEngine
{
    class Widget;
}

/**
  * \brief the player ID of the "game master" player
  * the game master is the player that can perform the game setup
  * \ingroup states_screens
  */
const static int PLAYER_ID_GAME_MASTER = 0;

/**
  * \brief A concrete scene manager, derived from GUIEngine's 
  * AbastractSceneManager
  * \ingroup states_screens
  */
class StateManager : public GUIEngine::AbstractStateManager
{
    
    void updateActivePlayerIDs();

    
public:

    /**
      * Represents a player that is currently playing.
      * Ties toghether :
      *   - a player's identity (and thus his/her highscores)
      *   - which input device is used by which player
      *  (we're very flexible on this; ActivePlayer #1
      *   can choose to e.g. use profile #5 and device #2)
      */
    class ActivePlayer
    {
        friend class StateManager;
        
        PlayerProfile *m_player;
        InputDevice   *m_device;
    
        /** Pointer to the kart of this player, only valid during the game. */
        Kart          *m_kart;
        
        /** ID of this player within the list of active players */
        int m_id;
        
        ActivePlayer(PlayerProfile* player, InputDevice* device);
        
#ifdef DEBUG
        unsigned int m_magic_number;
#endif
        
    public:
    
        ~ActivePlayer();
        
#ifdef DEBUG
        bool ok()
        {
            return (m_magic_number == 0xAC1EF1AE);
        }
#endif
        
        /** \return the identity of this active player */
        PlayerProfile* getProfile() { 
#ifdef DEBUG
			assert(m_magic_number == 0xAC1EF1AE); 
#endif
			return m_player; }
        
        /** \return the identity of this active player */
        const PlayerProfile* getConstProfile() const { 
#ifdef DEBUG
			assert(m_magic_number == 0xAC1EF1AE); 
#endif
			return m_player; }

        /** Call to change the identity of this player (useful when player is
         *  selecting his identity) */
        void setPlayerProfile(PlayerProfile* player);
        
        /** ID of this player within the list of active players */
        int getID() const {
#ifdef DEBUG
			assert(m_magic_number == 0xAC1EF1AE); 
#endif
			return m_id; }
        
        /** \return Which input device this player is using, or NULL if none 
         *  is set yet */
        InputDevice* getDevice() const { 
#ifdef DEBUG
			assert(m_magic_number == 0xAC1EF1AE);
#endif
			return m_device; }
         
        void setDevice(InputDevice* device);
        
        /** Sets the kart for this player. */
        void setKart(Kart *kart) { 
#ifdef DEBUG
			assert(m_magic_number == 0xAC1EF1AE);
#endif
			m_kart = kart; }
        
        /** \return the kart of this player. Only valid while world exists. */
        Kart* getKart()          { 
#ifdef DEBUG
			assert(m_magic_number == 0xAC1EF1AE); 
#endif
			assert(m_kart != NULL); return m_kart; }
    };


    const PtrVector<ActivePlayer, HOLD>& getActivePlayers() 
                                 { return m_active_players; }
    ActivePlayer* getActivePlayer(const int id);
    
    /** \return    the PlayerProfile of a given ActivePlayer.
      * \param id  the ID of the active player for whichyou want the profile
      */
    const PlayerProfile* getActivePlayerProfile(const int id);
    
    int createActivePlayer(PlayerProfile *profile, InputDevice *device);
    void removeActivePlayer(int id);

    int activePlayerCount();
    void resetActivePlayers();
    
    /** \return whether to reduce FPS at the moment
      * \note   this can be useful to avoid being too CPU/GPU intensive in 
      *         parts of the game that don't require high framerates, like 
      *         menus
      */
    bool throttleFPS();
    
    /** \brief implementing callback from base class AbstractStateManager */
    void escapePressed();

    /** \brief implementing callback from base class AbstractStateManager */
    virtual void onGameStateChange(GUIEngine::GameState new_state);

    /** \brief implementing callback from base class AbstractStateManager */
    virtual void onStackEmptied();
    
    /** \brief implementing callback from base class AbstractStateManager */
    virtual void onTopMostScreenChanged();
    
    // singleton
    static StateManager* get();
    
private:
    /**
     * A list of all currently playing players.
     */
    PtrVector<ActivePlayer, HOLD> m_active_players;
};

#endif
