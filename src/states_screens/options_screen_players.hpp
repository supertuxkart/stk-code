//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Marianne Gagnon
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


#ifndef __HEADER_OPTIONS_SCREEN_PLAYERS_HPP__
#define __HEADER_OPTIONS_SCREEN_PLAYERS_HPP__

#include <string>
#include <irrString.h>

#include "guiengine/screen.hpp"
#include "states_screens/dialogs/enter_player_name_dialog.hpp"
namespace GUIEngine { class Widget; }

struct Input;
class PlayerProfile;

/**
  * \brief Player management options screen
  * \ingroup states_screens
  */
class OptionsScreenPlayers : public GUIEngine::Screen, public EnterPlayerNameDialog::INewPlayerListener,
    public GUIEngine::ScreenSingleton<OptionsScreenPlayers>
{
private:
    OptionsScreenPlayers();
    bool refreshPlayerList();
public:
    friend class GUIEngine::ScreenSingleton<OptionsScreenPlayers>;
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;
    
    /**
     * \brief Adds a new player (if 'player' is NULL) or renames an existing player (if 'player' is not NULL)
     * \return  whether adding was successful (can fail e.g. if trying to add a duplicate)
     */
    bool renamePlayer(const irr::core::stringw& newName,  PlayerProfile* player=NULL);
    void deletePlayer(PlayerProfile* player);
    
    void selectPlayer(const irr::core::stringw& name);
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;
    
    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void tearDown() OVERRIDE;
    
    /** \brief implement callback from EnterPlayerNameDialog::INewPlayerListener */
    virtual void onNewPlayerWithName(const irr::core::stringw& newName);
};

#endif
