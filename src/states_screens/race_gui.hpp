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

#include <irrString.h>
using namespace irr;

#include "states_screens/race_gui_base.hpp"

class AbstractKart;
class InputMap;
class Material;
class RaceSetup;

/**
  * \brief Handles the in-race GUI (messages, mini-map, rankings, timer, etc...)
  * \ingroup states_screens
  */
class RaceGUI : public RaceGUIBase
{
private:

    // Minimap related variables
    // -------------------------
    /** The size of a single marker on the screen for AI karts,
     *  need not be a power of 2. */
    int              m_minimap_ai_size;

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

    /** Distance of map from bottom of screen. */
    int              m_map_bottom;

    /** Maximum lap display length (either 9/9  or 99/99). */
    int              m_lap_width;

    /** Maximum string length for the timer */
    int              m_timer_width;

    /** Maximum string length for a small precise timer
     *  (like the live difference timer under a minute) */
    int              m_small_precise_timer_width;

    /** Maximum string length for a big precise timer
     *  (like the live difference timer over a minute) */
    int              m_big_precise_timer_width;

    int              m_negative_timer_additional_width;

    /** Height of the digit font. */
    int              m_font_height;

    /** Icon textures (stored as variables to not look up
        their location on every frame) */
    irr::video::ITexture *m_red_team;
    irr::video::ITexture *m_blue_team;
    irr::video::ITexture *m_red_flag;
    irr::video::ITexture *m_blue_flag;
    irr::video::ITexture *m_soccer_ball;
    irr::video::ITexture *m_heart_icon;
    irr::video::ITexture *m_basket_ball_icon;
    /** Texture for the hit limit icon*/
    irr::video::ITexture* m_champion;

    /** Texture for speedometer. */
    irr::video::ITexture *m_speed_meter_icon;
    irr::video::ITexture *m_speed_bar_icon;

    /** Animation state: none, getting smaller (old value),
     *  getting bigger (new number). */
    enum AnimationState {AS_NONE, AS_SMALLER, AS_BIGGER};
    std::vector<AnimationState> m_animation_states;

    /** How long the rank animation has been shown. */
    std::vector<float> m_rank_animation_duration;

    /** Stores the previous rank for each kart. Used for the rank animation. */
    std::vector<int> m_last_ranks;

    bool m_is_tutorial;

    /* Display informat for one player on the screen. */
    void drawEnergyMeter       (int x, int y, const AbstractKart *kart,
                                const core::recti &viewport,
                                const core::vector2df &scaling);
    void drawSpeedEnergyRank   (const AbstractKart* kart,
                                const core::recti &viewport,
                                const core::vector2df &scaling, float dt);
    void drawLap               (const AbstractKart* kart,
                                const core::recti &viewport,
                                const core::vector2df &scaling);
    void drawRank              (const AbstractKart *kart,
                                const core::vector2df &offset,
                                float min_ratio, int meter_width,
                                int meter_height, float dt);

    /* Helper functions for drawing meters */

    void drawMeterTexture(video::ITexture *meter_texture, video::S3DVertex vertices[], unsigned int count, bool reverse = false);

    unsigned int computeVerticesForMeter(core::vector2df position[], float threshold[], video::S3DVertex vertices[],
                                         unsigned int vertices_count, float measure, int gauge_width,
                                         int gauge_height, core::vector2df offset);

    /** Display items that are shown once only (for all karts). */
    void drawGlobalMiniMap     ();
    void drawGlobalTimer       ();
    void drawLiveDifference    ();

public:

         RaceGUI();
        ~RaceGUI();
    virtual void init();
    virtual void reset();
    virtual void renderGlobal(float dt);
    virtual void renderPlayerView(const Camera *camera, float dt);

    /** Returns the size of the texture on which to render the minimap to. */
    virtual const core::dimension2du getMiniMapSize() const
                  { return core::dimension2du(m_map_width, m_map_height); }
    virtual void initSize();
    virtual void calculateMinimapSize();
};   // RaceGUI

#endif
