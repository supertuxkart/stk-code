//  $Id$
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

#include "irrlicht.h"
using namespace irr;

#include "config/player.hpp"
#include "states_screens/race_gui_base.hpp"

class InputMap;
class Kart;
class Material;
class RaceSetup;

/**
  * \brief Handles the in-race GUI (messages, mini-map, rankings, timer, etc...)
  * \ingroup states_screens
  */
class RaceGUI : public RaceGUIBase
{
private:
    class TimedMessage
    {
     public:
        irr::core::stringw m_message;            //!< message to display
        float              m_remaining_time;     //!< time remaining before removing this message from screen
        video::SColor      m_color;              //!< color of message
        int                m_font_size;          //!< size
        const Kart        *m_kart;
        bool               m_important;          //!< Important msgs are displayed in the middle of the screen
        // -----------------------------------------------------
        // std::vector needs standard copy-ctor and std-assignment op.
        // let compiler create defaults .. they'll do the job, no
        // deep copies here ..
        TimedMessage(const irr::core::stringw &message, 
                     const Kart *kart, float time, int size, 
                     const video::SColor &color, const bool important)
        {
            m_message        = message; 
            m_font_size      = size;
            m_kart           = kart;
            m_remaining_time = ( time < 0.0f ) ? -1.0f : time;
            m_color          = color;
            m_important      = important;
        }   // TimedMessage
        // -----------------------------------------------------
        // in follow leader the clock counts backwards
        bool done(const float dt)
        {
            m_remaining_time -= dt;
            return m_remaining_time < 0;
        }   // done
    };   // TimedMessage
    // ---------------------------------------------------------

    Material        *m_speed_meter_icon;
    Material        *m_speed_bar_icon;
    Material        *m_plunger_face;
    typedef          std::vector<TimedMessage> AllMessageType;
    AllMessageType   m_messages;
    
    /** A texture with all mini dots to be displayed in the minimap for all karts. */
    video::ITexture *m_marker;

    /** Musical notes icon (for music description and credits) */
    Material        *m_music_icon;

    /** Translated string of 'finished' message. */
    core::stringw    m_string_finished;

    /** Translated string 'lap' displayed every frame. */
    core::stringw    m_string_lap;

    /** Translated strings 'ready', 'set', 'go'. */
    core::stringw    m_string_ready, m_string_set, m_string_go;
    
    // Minimap related variables
    // -------------------------
    /** The mini map of the track. */
    video::ITexture *m_mini_map;
    
    /** used to render lap count next to the map without overlap */
    int              m_map_right_side_x;
    bool             m_minimap_on_left;
    
    /** The size of a single marker in pixels, must be a power of 2. */
    int              m_marker_rendered_size;
    
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
    int              m_small_font_max_height;
    
    /** Distance on track to begin showing overlap in drawGlobalPlayerIcons */
    float            m_dist_show_overlap;///can be zero
    float            m_icons_inertia;///can be zero
    /** previous position of icons */
    std::vector< core::vector2d<s32> > m_previous_icons_position;
    
    void createMarkerTexture();
    void createRegularPolygon(unsigned int n, float radius, 
                              const core::vector2df &center,
                              const video::SColor &color,
                              video::S3DVertex *v, unsigned short int *index);

    /* Display informat for one player on the screen. */
    void drawEnergyMeter       (const Kart *kart,
                                const core::recti &viewport, 
                                const core::vector2df &scaling);
    void drawPowerupIcons      (const Kart* kart,
                                const core::recti &viewport, 
                                const core::vector2df &scaling);
    void drawAllMessages       (const Kart* kart,
                                const core::recti &viewport, 
                                const core::vector2df &scaling);
    void drawSpeed             (const Kart* kart, const core::recti &viewport, 
                                const core::vector2df &scaling);
    void drawLap               (const KartIconDisplayInfo* info, const Kart* kart,
                                const core::recti &viewport);
    void drawGlobalPlayerIcons (const KartIconDisplayInfo* info);
    /** Display items that are shown once only (for all karts). */
    void drawGlobalMiniMap     ();
    void drawGlobalTimer       ();
    void drawGlobalMusicDescription();
    void cleanupMessages       (const float dt);
    void drawGlobalReadySetGo  ();
    
public:

         RaceGUI();
        ~RaceGUI();
    virtual void renderGlobal(float dt);
    virtual void renderPlayerView(const Kart *kart);
    
    virtual void addMessage(const irr::core::stringw &m, const Kart *kart, 
                            float time, int fonst_size, 
                            const video::SColor &color=
                                video::SColor(255, 255, 0, 255),
                            bool important=true);

    virtual void clearAllMessages() { m_messages.clear(); }
    
    /** Returns the size of the texture on which to render the minimap to. */
    virtual const core::dimension2du getMiniMapSize() const 
                  { return core::dimension2du(m_map_width, m_map_height); }
};   // RaceGUI

#endif
