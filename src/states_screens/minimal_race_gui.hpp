//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006 Joerg Henrichs, SuperTuxKart-Team, Steve Baker
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

#ifndef HEADER_MINIMAL_RACE_GUI_HPP
#define HEADER_MINIMAL_RACE_GUI_HPP

#include <string>
#include <vector>

#include <vector2d.h>
#include <irrString.h>
#include <dimension2d.h>
namespace irr
{
    namespace video { class ITexture; }
}
using namespace irr;

#include "config/player.hpp"
#include "states_screens/race_gui_base.hpp"

class AbstractKart;
class InputMap;
class Material;
class RaceSetup;

/**
  * \brief Handles the in-race GUI (messages, mini-map, rankings, timer, etc...)
  * \ingroup states_screens
  */
class MinimalRaceGUI : public RaceGUIBase
{
private:

    /** Translated string 'lap' displayed every frame. */
    core::stringw    m_string_lap;

    /** Length of 'Lap 99/99' */
    int              m_lap_width;

    /** Translated string 'rank' displayed every frame. */
    core::stringw    m_string_rank;

    /** Maximum string length for the timer */
    int              m_timer_width;

    /** The width of the 'X/Y' rank display. */
    int              m_rank_width;

    /** A scaling factor for the font for time, rank, and lap display. */
    float            m_font_scale;

    // Minimap related variables
    // -------------------------
    /** The mini map of the track. */
    video::ITexture *m_mini_map;
    
    /** The size of a single marker on the screen for AI karts, 
     *  need not be a power of 2. */
    int              m_marker_ai_size;

    /** The size of a single marker on the screen or player karts, 
     *  need not be a power of 2. */
    int              m_marker_player_size;

    /** The width of the rendered mini map in pixels, must be a power of 2. */
    int              m_map_rendered_width;

    /** The height of the rendered mini map in pixels, must be a power of 2. */
    int              m_map_rendered_height;
    
    /** Width of the map in pixels on the screen, need not be a power of 2. */
    int              m_map_width;

    /** Height of the map in pixels on the screen, need not be a power of 2. */
    int              m_map_height;

    /** Distance of map from left side of screen. */
    int              m_map_left;

    /** Distance of map from bottom of screen. */
    int              m_map_bottom;

    /** Used to display messages without overlapping */
    int              m_max_font_height;
    
    /** previous position of icons */
    std::vector< core::vector2d<s32> > m_previous_icons_position;
    
    /* Display informat for one player on the screen. */
    void drawEnergyMeter       (const AbstractKart *kart,
                                const core::recti &viewport, 
                                const core::vector2df &scaling);
    void drawRankLap           (const KartIconDisplayInfo* info, 
                                const AbstractKart* kart,
                                const core::recti &viewport);
    /** Display items that are shown once only (for all karts). */
    void drawGlobalMiniMap     ();
    void drawGlobalTimer       ();
    
public:

         MinimalRaceGUI();
        ~MinimalRaceGUI();
    virtual void renderGlobal(float dt);
    virtual void renderPlayerView(const AbstractKart *kart, float dt);
        
    /** Returns the size of the texture on which to render the minimap to. */
    virtual const core::dimension2du getMiniMapSize() const 
                  { return core::dimension2du(m_map_width, m_map_height); }
};   // MinimalRaceGUI

#endif
