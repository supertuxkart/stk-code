//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
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

#ifndef HEADER_ARENAS_SCREEN_HPP
#define HEADER_ARENAS_SCREEN_HPP

#include "guiengine/screen.hpp"


namespace GUIEngine { class Widget; }

/**
  * \brief Handles the screen where a battle arena choice is offered
  * \ingroup states_screens
  */
class ArenasScreen : public GUIEngine::Screen, public GUIEngine::ScreenSingleton<ArenasScreen>
{
    friend class GUIEngine::ScreenSingleton<ArenasScreen>;

    ArenasScreen();
    void buildTrackList();

private:
    std::set<int> m_unsupported_arena;

public:

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void beforeAddingWidget() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    void setFocusOnTrack(const std::string& trackName);
};

#endif
