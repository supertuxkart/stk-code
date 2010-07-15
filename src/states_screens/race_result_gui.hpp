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

#include <assert.h>
#include <vector>

#include "states_screens/race_gui_base.hpp"

/**
  * \brief Displays the results (while the end animation is shown).
  * \ingroup states_screens
  */
class RaceResultGUI : public RaceGUIBase
{
private:
    float                      m_timer;
    std::vector<float>         m_start_at;
    std::vector<float>         m_x_pos;
    std::vector<int>           m_y_pos;
    std::vector<core::stringw> m_entry;

    /** Time to wait till the next row starts to be animated. */
    float                      m_time_between_rows;

    /** Distance between each row of the race results */
    unsigned int               m_distance_between_rows;

    /** The size of the kart icons. */
    unsigned int               m_icon_width;

    /** The width of the time column. */
    unsigned int               m_time_width;

    /** The width of the point column. */
    unsigned int               m_column_width;

    /** The order in which to display the karts. */
    std::vector<int>           m_order;

    /** The width of the largest digit (not all digits 
     *  have the same font size) */
    unsigned int m_max_digit_width ;

    /** Size of space between columns. */
    unsigned int               m_column_space_size;
    
    /** The font to use. */
    gui::IGUIFont* m_font;

    void displayOneEntry(unsigned int x, unsigned int y, 
                         unsigned int n, bool display_points);
    void determineLayout();

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
