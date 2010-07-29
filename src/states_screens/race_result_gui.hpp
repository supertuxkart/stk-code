//  $Id: race_result_gui.hpp 5310 2010-04-28 18:26:23Z auria $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Joerg Henrichs
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

#ifndef HEADER_RACE_RESULT_GUI_HPP
#define HEADER_RACE_RESULT_GUI_HPP

#include "states_screens/race_gui_base.hpp"

#include <assert.h>
#include <vector>

namespace irr
{
    namespace gui
    {
        class ScalableFont;
    }
}

/**
  * \brief Displays the results (while the end animation is shown).
  * \ingroup states_screens
  */
class RaceResultGUI : public RaceGUIBase
{
private:
    /** Timer variable for animations. */
    float                      m_timer;

    /** Finite state machine for the animations:
        INIT:            Set up data structures.
        RACE_RESULT:     The rows scroll into place.
        OLD_GP_TABLE:    Scroll new table into place, sorted by previous
                         GP ranks
        INCREASE_POINTS: The overall points are added up
        RESORT_TABLE:    Resort the table so that it is now sorted by 
                         GP points.
        WAIT_TILL_END    Some delay to wait for end, after a period it
                         wii automatically end. */
    enum                       {RR_INIT,
                                RR_RACE_RESULT,
                                RR_OLD_GP_RESULTS,
                                RR_INCREASE_POINTS,
                                RR_RESORT_TABLE,
                                RR_WAIT_TILL_END}
                               m_animation_state;

    /** Start time for each line of the animation. */
    std::vector<float>         m_start_at;

    /** Currenct X position. */
    std::vector<float>         m_x_pos;

    /** Currenct Y position. */
    std::vector<float>         m_y_pos;

    /** The center point when sorting the entries. */
    std::vector<float>         m_centre_point;

    /** The radius to use when sorting the entries. Positive values
        will rotate downwards, negatives are upwards. */
    std::vector<float>         m_radius;

    /** The names of all karts in the right order. */
    std::vector<core::stringw> m_kart_names;

    /** Points earned in this race. */
    std::vector<int>           m_new_points;

    /** New overall points after this race. */
    std::vector<int>           m_new_overall_points;

    /** When updating the number of points in the display, this is the
        currently displayed number of points, so
        m_new_overall_points-m_new_points <= m_current_displayed_points <=
                                             m_new_overall_points.
        This is a floating point number since it stores the increments
        during increasing the points. */
    std::vector<float>         m_current_displayed_points;

    /** The kart icons. */
    std::vector<video::ITexture*> m_kart_icons;

    /** The times of all karts in the right order. */
    std::vector<core::stringw> m_finish_time_string;

    /** Time to wait till the next row starts to be animated. */
    float                      m_time_between_rows;

    /** The time a single line scrolls into place. */
    float                      m_time_single_scroll;

    /** Time to rotate the GP entries. */
    float                      m_time_rotation;

    /** The time for inreasing the points by one during the
        point update phase. */
    float                      m_time_for_points;

    /** The overall time the first phase (scrolling) is displayed.
        This includes a small waiting time at the end. */
    float                      m_time_overall_scroll;

    /** Distance between each row of the race results */
    unsigned int               m_distance_between_rows;

    /** The size of the kart icons. */
    unsigned int               m_width_icon;

    /** Width of the kart name column. */
    unsigned int               m_width_kart_name;

    /** Width of the finish time column. */
    unsigned int               m_width_finish_time;

    /** Width of the new points columns. */
    unsigned int               m_width_new_points;

    /** Position of left end of table (so that the whole
        table is aligned. */
    unsigned int               m_leftmost_column;

    /** Top-most pixel for first row. */
    unsigned int               m_top;

    /** Size of space between columns. */
    unsigned int               m_width_column_space;
    
    /** The font to use. */
    gui::ScalableFont         *m_font;

    /** The previous monospace state of the font. */
    bool                       m_was_monospace;

    void displayOneEntry(unsigned int x, unsigned int y, 
                         unsigned int n, bool display_points);
    void determineTableLayout();
    void determineGPLayout();

public:

                 RaceResultGUI();
    virtual     ~RaceResultGUI();
    virtual void renderGlobal(float dt);

    /** Should not be called anymore.  */
    const core::dimension2du getMiniMapSize() const 
                  { assert(false); return core::dimension2du(0, 0); }

    /** No kart specific view needs to be rendered in the result gui. */
    virtual void renderPlayerView(const Kart *kart) {}
    
    /** No more messages need to be displayed, so this function shouldn't
     *  be called at all. */
    virtual void addMessage(const irr::core::stringw &m, const Kart *kart, 
                            float time, int fonst_size, 
                            const video::SColor &color=
                                video::SColor(255, 255, 0, 255),
                            bool important=true) { assert(false); }

    /** Should not be called anymore. */
    virtual void clearAllMessages() {assert(false); }
    
    void nextPhase();
};   // RaceResultGUI

#endif
