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

#ifndef HEADER_EASTER_EGG_SCREEN_HPP
#define HEADER_EASTER_EGG_SCREEN_HPP

#include "guiengine/screen.hpp"
#include <deque>
#include <random>

namespace GUIEngine { class Widget; }

/**
  * \brief screen where the user can select a track
  * \ingroup states_screens
  */
class EasterEggScreen : public GUIEngine::Screen, public GUIEngine::ScreenSingleton<EasterEggScreen>
{
    friend class GUIEngine::ScreenSingleton<EasterEggScreen>;

    std::mt19937 m_random_number_generator;

    EasterEggScreen();

    /** adds the tracks from the current track group into the tracks ribbon */
    void buildTrackList();

    std::deque<std::string> m_random_track_list;

public:

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void beforeAddingWidget() OVERRIDE;


    void setFocusOnTrack(const std::string& trackName);

};

#endif
