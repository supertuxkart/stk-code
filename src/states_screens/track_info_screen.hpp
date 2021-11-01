//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
//            (C) 2014-2015 Joerg Henrichs
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
#include "guiengine/widgets.hpp"

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
    static const int HIGHSCORE_COUNT = 5;

    /** A pointer to the track of which the info is shown. */
    Track *m_track;

    bool m_record_this_race;

    bool m_is_soccer;

    bool m_is_lap_trial;

    bool m_show_ffa_spinner;

    // When there is no need to tab through / click on images/labels, we can add directly
    // irrlicht labels (more complicated uses require the use of our widget set)
        
    /** Spinner for target types. */
    GUIEngine::SpinnerWidget* m_target_type_spinner;

    /** The label besides the target types spinner. */
    GUIEngine::LabelWidget* m_target_type_label;

    /** Spinner for number of blue AI karts. */
    GUIEngine::SpinnerWidget* m_ai_blue_spinner;

    /** The label besides the blue AI karts spinner. */
    GUIEngine::LabelWidget* m_ai_blue_label;

    /* The div that contains the blue ai spinner and label */
    GUIEngine::Widget* m_ai_blue_div;

    /* The div that contains the target type spinner and label */
    GUIEngine::Widget* m_target_type_div;

    /** Spinner for target value e.g. number of laps or goals to score. */
    GUIEngine::SpinnerWidget* m_target_value_spinner;

    /** The label besides the target value spinner. */
    GUIEngine::LabelWidget* m_target_value_label;

    /** Spinner for number of AI karts. */
    GUIEngine::SpinnerWidget* m_ai_kart_spinner;

    /** The label besides the AI karts spinner. */
    GUIEngine::LabelWidget* m_ai_kart_label;

    /** Check box for reverse mode or random item in arena. */
    GUIEngine::CheckBoxWidget* m_option;

    /** Check box for record race. */
    GUIEngine::CheckBoxWidget* m_record_race;

    /** The label of the highscore list. */
    GUIEngine::LabelWidget* m_highscore_label;

    /** The actual highscore text values shown. */
    GUIEngine::ListWidget* m_highscore_entries;
    
    irr::gui::STKModifiedSpriteBank* m_icon_bank;
    
    int m_icon_unknown_kart;

    void updateHighScores();
    void setSoccerWidgets(bool has_AI);
    void setSoccerTarget(bool time_limit);
    void soccerSpinnerUpdate(bool blue_spinner);

public:
    TrackInfoScreen();
    virtual ~TrackInfoScreen();

    virtual void init() OVERRIDE;
    virtual void beforeAddingWidget() OVERRIDE;
    virtual void loadedFromFile() OVERRIDE;
    virtual void tearDown() OVERRIDE;
    virtual void unloaded() OVERRIDE;
    virtual void eventCallback(GUIEngine::Widget *,const std::string &name ,
                               const int player_id) OVERRIDE;

    void onEnterPressedInternal();
    void setTrack(Track *track);
};

#endif
