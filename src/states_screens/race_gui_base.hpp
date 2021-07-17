//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Joerg Henrichs
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
    namespace video { class ITexture; struct S3DVertex; }
    namespace scene { class IAnimatedMeshSceneNode; }
}
using namespace irr;

#include "utils/vec3.hpp"

class AbstractKart;
class Camera;
class Material;
class Referee;
class RaceGUIMultitouch;

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
        /** Text to display next to icon, if any. */
        core::stringw m_text;

        /** Text color, if any text. */
        video::SColor m_color;

        /** If this kart has a special title, e.g. "leader" in follow-the-leader. */
        core::stringw special_title;

        /** Current lap of this kart, or -1 if irrelevant. */
        int lap;

        bool m_outlined_font = false;
    };   // KartIconDisplayInfo

private:
    /** True if unimportant messags (like item messages) should not
     *  be displayed. */
    bool  m_ignore_unimportant_messages;

    class TimedMessage
    {
     public:
         /** Message to display. */
        irr::core::stringw  m_message;
        /** Time remaining before removing this message from screen. */
        float               m_remaining_time;
        /** Color of message. */
        video::SColor       m_color;

        const AbstractKart *m_kart;
        /** Important msgs are displayed in the middle of the screen. */
        bool                m_important;
        bool                m_big_font;

        bool                m_outline;

        // -----------------------------------------------------
        // std::vector needs standard copy-ctor and std-assignment op.
        // let compiler create defaults .. they'll do the job, no
        // deep copies here ..
        TimedMessage(const irr::core::stringw &message,
                     const AbstractKart *kart, float time,
                     const video::SColor &color, const bool important,
                     bool big_font, bool outline)
        {
            m_message        = message;
            m_kart           = kart;
            m_remaining_time = ( time < 0.0f ) ? -1.0f : time;
            m_color          = color;
            m_important      = important;
            m_big_font       = big_font;
            m_outline        = outline;
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
        video::ITexture* m_music_icon;

    /** Texture for the 'plunger in the face' texture. */
    video::ITexture* m_plunger_face;
    
    /** Translated strings 'ready', 'set', 'go'. */
    core::stringw    m_string_ready, m_string_set, m_string_go, m_string_goal,
        m_string_waiting_for_others, m_string_waiting_for_the_server;

    /** The position of the referee for all karts. */
    std::vector<Vec3> m_referee_pos;

    /** The actual rotation to use for the referee for each kart. */
    std::vector<Vec3> m_referee_rotation;

    /** The height of the referee. This is used to make the referee fly
     *  into view. This is the same Y-offset for all karts, so only a
     *  single value needs to be used. */
    float m_referee_height;

    /** The referee scene node. */
    Referee *m_referee;

    /* True if spectating is possible in current GUI (when local player
     * finished). */
    bool m_enabled_network_spectator;

    /* this flag is set to true if we show at least one custom color for other karts
     * in that case we want to draw a bigger circle around the player's own kart
     * to make it easier for the player to identify */
    bool m_showing_kart_colors;
protected:

    /** State of the plunger: From the 'init' states the plunger switches
     *  between two slow moving states ('shakily moving') till the end of
     *  the plunger time is nearly reached, then it goes to a very fast
     *  moving state ('plunger blown off'). */
    enum PlungerState {PLUNGER_STATE_INIT,   PLUNGER_STATE_SLOW_1,
                       PLUNGER_STATE_SLOW_2, PLUNGER_STATE_FAST}
                        m_plunger_state;

    /** How long the plunger should stay in the current state. */
    float m_plunger_move_time;

    /** Offset of the plunger. */
    core::vector2di m_plunger_offset;

    /* Speed of the plunger. This gets changed depending on state (not moving,
     *  slow moving, fast moving). */
    core::vector2df m_plunger_speed;

    /** The size of a single marker in pixels, must be a power of 2. */
    //int              m_marker_rendered_size;

    /** A texture with all mini dots to be displayed in the minimap for all karts. */
    //video::ITexture *m_marker;
    video::ITexture *m_gauge_empty;
    /** Default texture for nitro gauge. */
    video::ITexture *m_gauge_full;
    /** Highlight gauge, used when a kart uses nitro. */
    video::ITexture *m_gauge_full_bright;

    video::ITexture *m_gauge_goal;

    /** The frame around player karts in the mini map. */
    video::ITexture* m_icons_frame;

    /** The frame around player karts in the kart list. */
    video::ITexture* m_icons_kart_list;

    /** Texture for the lap icon*/
    video::ITexture* m_lap_flag;
    
    RaceGUIMultitouch* m_multitouch_gui;

    //void createMarkerTexture();
    void createRegularPolygon(unsigned int n, float radius,
                              const core::vector2df &center,
                              const video::SColor &color,
                              video::S3DVertex *v, unsigned short int *index);
    void drawAllMessages       (const AbstractKart* kart,
                                const core::recti &viewport,
                                const core::vector2df &scaling);
    void drawPowerupIcons      (const AbstractKart* kart,
                                const core::recti &viewport,
                                const core::vector2df &scaling);
    void drawGlobalMusicDescription();
    void drawGlobalReadySetGo();
    void drawGlobalGoal();
    void drawPlungerInFace(const Camera *camera, float dt);
    /** Instructs the base gui to ignore unimportant messages (like
     *  item messages).
     */
    void ignoreUnimportantMessages() { m_ignore_unimportant_messages = true; }

    /** Distance on track to begin showing overlap in drawGlobalPlayerIcons */
    float            m_dist_show_overlap;///can be zero
    float            m_icons_inertia;///can be zero

    /** previous position of icons */
    std::vector< core::vector2d<s32> > m_previous_icons_position;

    /** This vector is passed to world to be filled with the current
     *  race data information. */
    std::vector<KartIconDisplayInfo> m_kart_display_infos;

public:

    bool m_enabled;

                  RaceGUIBase();
    virtual      ~RaceGUIBase();
    virtual void renderGlobal(float dt);
    virtual void init();
    virtual void reset();
    virtual void renderPlayerView(const Camera *camera, float dt);
    virtual void addMessage(const irr::core::stringw &m,
                            const AbstractKart *kart, float time,
                            const video::SColor &color=
                                video::SColor(255, 255, 0, 255),
                            bool important=true,
                            bool big_font=false, bool outline=false);
    virtual void update(float dt);
    virtual void preRenderCallback(const Camera *camera);
    // ------------------------------------------------------------------------
    /** Returns the size of the texture on which to render the minimap to. */
    virtual const core::dimension2du
                  getMiniMapSize() const = 0;
    virtual void calculateMinimapSize() {};
    // ------------------------------------------------------------------------
    virtual void clearAllMessages() { m_messages.clear(); }

    void drawGlobalPlayerIcons(int bottom_margin);
    void drawPlayerIcon(AbstractKart *kart, int x, int y, int w,
                        bool is_local);
    
    virtual void drawEnergyMeter(int x, int y, const AbstractKart *kart,
                                 const core::recti &viewport,
                                 const core::vector2df &scaling) {};

    void cleanupMessages(const float dt);
    void removeReferee();
    
    RaceGUIMultitouch* getMultitouchGUI() {return m_multitouch_gui;}
    void recreateGUI();
    virtual void initSize();

};   // RaceGUIBase

#endif
