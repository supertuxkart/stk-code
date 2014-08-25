//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2014 Marianne Gagnon
//                2014      Joerg Henrichs
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


#ifndef HEADER_TRACK_INFO_SCREEN_HPP
#define HEADER_TRACK_INFO_SCREEN_HPP

#include "guiengine/screen.hpp"

//#include "guiengine/widgets/check_box_widget.hpp"


namespace GUIEngine
{
    class CheckBoxWidget;
    class IconButtonWidget;
    class LabelWidget;
    class SpinnerWidget;
    class Widget;
}
class Track;

/**
 * \brief Dialog that shows the information about a given track
 * \ingroup states_screens
 */
class TrackInfoScreen : public GUIEngine::Screen,
                  public GUIEngine::ScreenSingleton<TrackInfoScreen>
{
    static const int HIGHSCORE_COUNT = 3;

    /** A pointer to the track of which the info is shown. */
    const Track *m_track;

    // When there is no need to tab through / click on images/labels, we can add directly
    // irrlicht labels (more complicated uses require the use of our widget set)
    /** Spinner for number of laps. */
    GUIEngine::SpinnerWidget* m_lap_spinner;

    /** Spinner for number of AI karts. */
    GUIEngine::SpinnerWidget* m_ai_kart_spinner;

    /** */
    GUIEngine::CheckBoxWidget* m_checkbox;
    GUIEngine::IconButtonWidget* m_kart_icons[HIGHSCORE_COUNT];
    GUIEngine::LabelWidget* m_highscore_entries[HIGHSCORE_COUNT];
    
    void updateHighScores();
    
public:
    /**
     * \brief Creates a track info modal dialog with given percentage of screen width and height
     * \param ribbonItem identifier name of the ribbon item that was clicked in the track selection
     *        screen to get there (often will be 'trackIdent' but may also be the random item)
     * \param trackIdent identifier name of the track to show information about
     * \param trackName  human-readable, possibly translated, name of the track to show information about
     * \param screenshot screenshot of the track to show information about
     */
    TrackInfoScreen();
    virtual ~TrackInfoScreen();
    
    virtual void init();
    virtual void loadedFromFile() {}
    virtual void eventCallback(GUIEngine::Widget *,const std::string &name ,
                               const int player_id);
    void onEnterPressedInternal();
    void setTrack(const Track *track);
};

#endif
