//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#ifndef HEADER_TRACKS_AND_GP_SCREEN_HPP
#define HEADER_TRACKS_AND_GP_SCREEN_HPP

#include "guiengine/screen.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include <deque>
#include <random>

namespace GUIEngine { class Widget; }

/**
  * \brief screen where the user can select a track or grand prix
  * \ingroup states_screens
  */
class TracksAndGPScreen : public GUIEngine::Screen,
                          public GUIEngine::ScreenSingleton<TracksAndGPScreen>,
                          public GUIEngine::ITextBoxWidgetListener
{
    friend class GUIEngine::ScreenSingleton<TracksAndGPScreen>;

private:
    GUIEngine::TextBoxWidget* m_search_box;

    std::mt19937 m_random_number_generator;

    TracksAndGPScreen() : Screen("tracks_and_gp.stkgui"), m_random_number_generator(std::random_device{}()) {}

    /** adds the tracks from the current track group into the tracks ribbon */
    void buildTrackList();

    std::deque<std::string> m_random_track_list;

public:

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void loadedFromFile() OVERRIDE {};

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void eventCallback(GUIEngine::Widget* widget,
                               const std::string& name,
                               const int playerID) OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void init() OVERRIDE;

    /** \brief implement callback from parent class GUIEngine::Screen */
    virtual void beforeAddingWidget() OVERRIDE;

    /** Rebuild the list of tracks based on search text */
    virtual void onTextUpdated() OVERRIDE
    {
        buildTrackList();
        // After buildTrackList the m_search_box may be unfocused
        m_search_box->focused(0);
    }

    void setFocusOnTrack(const std::string& trackName);
    void setFocusOnGP(const std::string& gpName);

};

#endif
