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

#ifndef HEADER_RACE_GUI_HPP
#define HEADER_RACE_GUI_HPP

#include <string>
#include <vector>
#include <set>

#include <irrString.h>
using namespace irr;

#include "config/player.hpp"
#include "states_screens/race_gui_base.hpp"

class InputMap;
class Kart;
class Material;
class RaceSetup;
class ChallengeData;

/** Distance (squared) at which a challenge orb "activates" */
const int CHALLENGE_DISTANCE_SQUARED = 20;

/**
 * \brief Handles the in-race GUI (messages, mini-map, rankings, timer, etc...)
 * \ingroup states_screens
 */
class RaceGUIOverworld : public RaceGUIBase
{
private:
    
    Material        *m_speed_meter_icon;
    Material        *m_speed_bar_icon;
    
    /** Translated string 'lap' displayed every frame. */
    core::stringw    m_string_lap;
    
    /** Translated string 'rank' displayed every frame. */
    core::stringw    m_string_rank;
    
    // Minimap related variables
    // -------------------------
    /** The mini map of the track. */
    video::ITexture *m_mini_map;
    
    video::ITexture *m_trophy;
    video::ITexture *m_lock;
    video::ITexture *m_open_challenge;

    video::ITexture* m_icons[3];
    
    /** The size of a single marker on the screen for AI karts, 
     *  need not be a power of 2. */
    int              m_marker_challenge_size;
    
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
    
    int              m_trophy_points_width;
    
    std::set<const ChallengeData*> m_locked_challenges;

    /* Display informat for one player on the screen. */
    void drawEnergyMeter       (int x, int y, const Kart *kart,
                                const core::recti &viewport, 
                                const core::vector2df &scaling);
    
    /** Display items that are shown once only (for all karts). */
    void drawGlobalMiniMap     ();
    void drawTrophyPoints      ();
    
public:
    
    RaceGUIOverworld();
    ~RaceGUIOverworld();
    virtual void renderGlobal(float dt);
    virtual void renderPlayerView(const Kart *kart);
    
    /** Returns the size of the texture on which to render the minimap to. */
    virtual const core::dimension2du getMiniMapSize() const 
    { return core::dimension2du(m_map_width, m_map_height); }
    
};   // RaceGUI

#endif
