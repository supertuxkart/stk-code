//  $Id$
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

#ifndef HEADER_RACE_GUI_BASE_HPP
#define HEADER_RACE_GUI_BASE_HPP

#include <vector>

#include <irrString.h>
#include <vector2d.h>
#include <rect.h>
#include <dimension2d.h>
#include <SColor.h>
namespace irr
{
    namespace video { class ITexture; class S3DVertex; }
}
using namespace irr;


class Kart;
class Material;

/**
  * \brief An abstract base class for the two race guis (race_gui and 
  *  race_result gui)
  * \ingroup states_screens
  */
class RaceGUIBase
{
public:
    /**
      * Used to display the list of karts and their times or
      * whatever other info is relevant to the current mode.
      */
    struct KartIconDisplayInfo
    {
        /** text to display next to icon, if any */
        core::stringw m_text;
        
        /** text color, if any text */
        float r, g, b;
        
        /** if this kart has a special title, e.g. "leader" in follow-the-leader */
        core::stringw special_title;
        
        /** Current lap of this kart, or -1 if irrelevant */
        int lap;
    };   // KartIconDisplayInfo

private:
    /** Delight in seconds between lightnings. */
    float m_lightning;

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

    typedef          std::vector<TimedMessage> AllMessageType;
    AllMessageType   m_messages;
    int              m_small_font_max_height;

    /** Used to display messages without overlapping */
    int              m_max_font_height;

    /** Musical notes icon (for music description and credits) */
    Material        *m_music_icon;

    /** Translated strings 'ready', 'set', 'go'. */
    core::stringw    m_string_ready, m_string_set, m_string_go;

protected:
    /** Material for the 'plunger in the face' texture. */
    Material        *m_plunger_face;

    /** The size of a single marker in pixels, must be a power of 2. */
    int              m_marker_rendered_size;

    /** A texture with all mini dots to be displayed in the minimap for all karts. */
    video::ITexture *m_marker;
    video::ITexture *m_gauge_empty;
    video::ITexture *m_gauge_full;
    video::ITexture *m_gauge_goal;

    /** The frame around player karts in the mini map. */
    Material         *m_icons_frame;
    
    void cleanupMessages(const float dt);
    void createMarkerTexture();
    void createRegularPolygon(unsigned int n, float radius, 
                              const core::vector2df &center,
                              const video::SColor &color,
                              video::S3DVertex *v, unsigned short int *index);
    void drawAllMessages       (const Kart* kart,
                                const core::recti &viewport, 
                                const core::vector2df &scaling);
    void drawPowerupIcons      (const Kart* kart,
                                const core::recti &viewport, 
                                const core::vector2df &scaling);

    void drawGlobalMusicDescription();
    void drawGlobalReadySetGo  ();

public:
    
    bool m_enabled;

                  RaceGUIBase();
    virtual      ~RaceGUIBase();
    virtual void renderGlobal(float dt);
    virtual void renderPlayerView(const Kart *kart);
    virtual void addMessage(const irr::core::stringw &m, const Kart *kart, 
                            float time, int fonst_size, 
                            const video::SColor &color=
                                video::SColor(255, 255, 0, 255),
                            bool important=true);
    /** Returns the size of the texture on which to render the minimap to. */
    virtual const core::dimension2du 
                  getMiniMapSize() const = 0;
    virtual void clearAllMessages() { m_messages.clear(); }

    /** Set the flag that a lightning should be shown. */
    void doLightning() { m_lightning = 1.0f; }

};   // RaceGUIBase

#endif
