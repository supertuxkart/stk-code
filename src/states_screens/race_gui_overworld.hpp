//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2015 Joerg Henrichs, SuperTuxKart-Team, Steve Baker
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

#include "states_screens/race_gui_base.hpp"

class AbstractKart;
class ChallengeData;
struct OverworldChallenge;
class InputMap;
class Material;
class RaceSetup;

/** Distance (squared) at which a challenge orb "activates" */
const int CHALLENGE_DISTANCE_SQUARED = 20;

const int CHALLENGE_HEIGHT = 4;

/**
 * \brief Handles the in-race GUI (messages, mini-map, rankings, timer, etc...)
 * \ingroup states_screens
 */
class RaceGUIOverworld : public RaceGUIBase
{
private:

    Material        *m_speed_meter_icon;
    Material        *m_speed_bar_icon;

    bool             m_close_to_a_challenge;

    /** Translated string 'lap' displayed every frame. */
    core::stringw    m_string_lap;

    /** Translated string 'rank' displayed every frame. */
    core::stringw    m_string_rank;

    // Minimap related variables
    // -------------------------
    video::ITexture *m_trophy1;
    video::ITexture *m_trophy2;
    video::ITexture *m_trophy3;
    video::ITexture *m_lock;
    video::ITexture *m_open_challenge;

    video::ITexture* m_icons[5];

    /** The size of a single marker on the screen for AI karts,
     *  need not be a power of 2. */
    int              m_minimap_challenge_size;

    /** The size of a single marker on the screen or player karts,
     *  need not be a power of 2. */
    int              m_minimap_player_size;

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

    /** True if this is the first time the renderer is called. */
    bool             m_is_first_render_call;

    /** Distance of map from bottom of screen. */
    int              m_map_bottom;

    int              m_trophy_points_width;

    /** The latest challenge approached by the kart */
    const ChallengeData* m_active_challenge;

    core::stringw    m_challenge_description;

    /** The current challenge over which the mouse is hovering. */
    const OverworldChallenge *m_current_challenge;

    /** Display items that are shown once only (for all karts). */
    void drawGlobalMiniMap     ();
    void drawTrophyPoints      ();

public:

    RaceGUIOverworld();
    ~RaceGUIOverworld();
    virtual void renderGlobal(float dt);
    virtual void renderPlayerView(const Camera *camera, float dt);

    // ------------------------------------------------------------------------
    /** Returns the currently selected challenge data (or NULL if no is
     *  selected). */
    const OverworldChallenge *getCurrentChallenge() const
    {
        return m_current_challenge;
    }   // getCurrentChallenge

    // ------------------------------------------------------------------------
    /** Returns the size of the texture on which to render the minimap to. */
    virtual const core::dimension2du getMiniMapSize() const
    { return core::dimension2du(m_map_width, m_map_height); }

};   // RaceGUI

#endif
