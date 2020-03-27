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

#include "states_screens/race_gui.hpp"

using namespace irr;

#include <algorithm>
#include <limits>

#include "challenges/story_mode_timer.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/camera.hpp"
#include "graphics/2dutils.hpp"
#ifndef SERVER_ONLY
#include "graphics/glwrap.hpp"
#endif
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/scalable_font.hpp"
#include "io/file_manager.hpp"
#include "items/powerup_manager.hpp"
#include "items/powerup.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "karts/controller/spare_tire_ai.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/capture_the_flag.hpp"
#include "modes/follow_the_leader.hpp"
#include "modes/linear_world.hpp"
#include "modes/world.hpp"
#include "modes/soccer_world.hpp"
#include "network/protocols/client_lobby.hpp"
#include "race/race_manager.hpp"
#include "states_screens/race_gui_multitouch.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object_manager.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

/** The constructor is called before anything is attached to the scene node.
 *  So rendering to a texture can be done here. But world is not yet fully
 *  created, so only the race manager can be accessed safely.
 */
RaceGUI::RaceGUI()
{
    m_enabled = true;
    
    if (UserConfigParams::m_artist_debug_mode && UserConfigParams::m_hide_gui)
        m_enabled = false;

    // Determine maximum length of the rank/lap text, in order to
    // align those texts properly on the right side of the viewport.
    gui::ScalableFont* font = GUIEngine::getHighresDigitFont();
    core::dimension2du area = font->getDimension(L"99:99.999");
    m_timer_width = area.Width;
    m_font_height = area.Height;

    area = font->getDimension(L"99.999");
    m_small_precise_timer_width = area.Width;

    area = font->getDimension(L"99:99.999");
    m_big_precise_timer_width = area.Width;

    area = font->getDimension(L"-");
    m_negative_timer_additional_width = area.Width;

    if (RaceManager::get()->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER ||
        RaceManager::get()->isBattleMode()     ||
        RaceManager::get()->getNumLaps() > 9)
        m_lap_width = font->getDimension(L"99/99").Width;
    else
        m_lap_width = font->getDimension(L"9/9").Width;

    bool multitouch_enabled = (UserConfigParams::m_multitouch_active == 1 && 
                               irr_driver->getDevice()->supportsTouchDevice()) ||
                               UserConfigParams::m_multitouch_active > 1;
    
    if (multitouch_enabled && UserConfigParams::m_multitouch_draw_gui &&
        RaceManager::get()->getNumLocalPlayers() == 1)
    {
        m_multitouch_gui = new RaceGUIMultitouch(this);
    }
    
    calculateMinimapSize();

    m_is_tutorial = (RaceManager::get()->getTrackName() == "tutorial");

    // Load speedmeter texture before rendering the first frame
    m_speed_meter_icon = material_manager->getMaterial("speedback.png");
    m_speed_meter_icon->getTexture(false,false);
    m_speed_bar_icon   = material_manager->getMaterial("speedfore.png");
    m_speed_bar_icon->getTexture(false,false);
    //createMarkerTexture();

    // Load icon textures for later reuse
    m_red_team = irr_driver->getTexture(FileManager::GUI_ICON, "soccer_ball_red.png");
    m_blue_team = irr_driver->getTexture(FileManager::GUI_ICON, "soccer_ball_blue.png");
    m_red_flag = irr_driver->getTexture(FileManager::GUI_ICON, "red_flag.png");
    m_blue_flag = irr_driver->getTexture(FileManager::GUI_ICON, "blue_flag.png");
    m_soccer_ball = irr_driver->getTexture(FileManager::GUI_ICON, "soccer_ball_normal.png");
    m_heart_icon = irr_driver->getTexture(FileManager::GUI_ICON, "heart.png");
    m_champion = irr_driver->getTexture(FileManager::GUI_ICON, "cup_gold.png");
}   // RaceGUI

//-----------------------------------------------------------------------------
RaceGUI::~RaceGUI()
{
    delete m_multitouch_gui;
}   // ~Racegui


//-----------------------------------------------------------------------------
void RaceGUI::init()
{
    RaceGUIBase::init();
    // Technically we only need getNumLocalPlayers, but using the
    // global kart id to find the data for a specific kart.
    int n = RaceManager::get()->getNumberOfKarts();

    m_animation_states.resize(n);
    m_rank_animation_duration.resize(n);
    m_last_ranks.resize(n);
}   // init

//-----------------------------------------------------------------------------
/** Reset the gui before a race. It initialised all rank animation related
 *  values back to the default.
 */
void RaceGUI::reset()
{
    RaceGUIBase::reset();
    for(unsigned int i=0; i<RaceManager::get()->getNumberOfKarts(); i++)
    {
        m_animation_states[i] = AS_NONE;
        m_last_ranks[i]       = i+1;
    }
}  // reset

//-----------------------------------------------------------------------------
void RaceGUI::calculateMinimapSize()
{
    float map_size_splitscreen = 1.0f;

    // If there are four players or more in splitscreen
    // and the map is in a player view, scale down the map
    if (RaceManager::get()->getNumLocalPlayers() >= 4 && !RaceManager::get()->getIfEmptyScreenSpaceExists())
    {
        // If the resolution is wider than 4:3, we don't have to scaledown the minimap as much
        // Uses some margin, in case the game's screen is not exactly 4:3
        if ( ((float) irr_driver->getFrameSize().Width / (float) irr_driver->getFrameSize().Height) >
             (4.1f/3.0f))
        {
            if (RaceManager::get()->getNumLocalPlayers() == 4)
                map_size_splitscreen = 0.75f;
            else
                map_size_splitscreen = 0.5f;
        }
        else
            map_size_splitscreen = 0.5f;
    }

    // Originally m_map_height was 100, and we take 480 as minimum res
    float scaling = std::min(irr_driver->getFrameSize().Height,  
                             irr_driver->getFrameSize().Width) / 480.0f;
    const float map_size = stk_config->m_minimap_size * map_size_splitscreen;
    const float top_margin = 3.5f * m_font_height;
    
    // Check if we have enough space for minimap when touch steering is enabled
    if (m_multitouch_gui != NULL  && !m_multitouch_gui->isSpectatorMode())
    {
        const float map_bottom = (float)(irr_driver->getActualScreenSize().Height - 
                                         m_multitouch_gui->getHeight());
        
        if ((map_size + 20.0f) * scaling > map_bottom - top_margin)
        {
            scaling = (map_bottom - top_margin) / (map_size + 20.0f);
        }
        
        // Use some reasonable minimum scale, because minimap size can be 
        // changed during the race
        scaling = std::max(scaling,
                           irr_driver->getActualScreenSize().Height * 0.15f / 
                           (map_size + 20.0f));
    }
    
    // Marker texture has to be power-of-two for (old) OpenGL compliance
    //m_marker_rendered_size  =  2 << ((int) ceil(1.0 + log(32.0 * scaling)));
    m_minimap_ai_size       = (int)( stk_config->m_minimap_ai_icon     * scaling);
    m_minimap_player_size   = (int)( stk_config->m_minimap_player_icon * scaling);
    m_map_width             = (int)(map_size * scaling);
    m_map_height            = (int)(map_size * scaling);

    if ((UserConfigParams::m_minimap_display == 1 && /*map on the right side*/
       RaceManager::get()->getNumLocalPlayers() == 1) || m_multitouch_gui)
    {
        m_map_left          = (int)(irr_driver->getActualScreenSize().Width - 
                                                        m_map_width - 10.0f*scaling);
        m_map_bottom        = (int)(3*irr_driver->getActualScreenSize().Height/4 - 
                                                        m_map_height);
    }
    else if ((UserConfigParams::m_minimap_display == 3 && /*map on the center of the screen*/
       RaceManager::get()->getNumLocalPlayers() == 1) || m_multitouch_gui)
    {
        m_map_left          = (int)(irr_driver->getActualScreenSize().Width / 2);
        if (m_map_left + m_map_width > (int)irr_driver->getActualScreenSize().Width)
          m_map_left        = (int)(irr_driver->getActualScreenSize().Width - m_map_width);
        m_map_bottom        = (int)( 10.0f * scaling);
    }
    else // default, map in the bottom-left corner
    {
        m_map_left          = (int)( 10.0f * scaling);
        m_map_bottom        = (int)( 10.0f * scaling);
    }

    // Minimap is also rendered bigger via OpenGL, so find power-of-two again
    const int map_texture   = 2 << ((int) ceil(1.0 + log(128.0 * scaling)));
    m_map_rendered_width    = map_texture;
    m_map_rendered_height   = map_texture;


    // special case : when 3 players play, use available 4th space for such things
    if (RaceManager::get()->getIfEmptyScreenSpaceExists())
    {
        m_map_left = irr_driver->getActualScreenSize().Width -
                     m_map_width - (int)( 10.0f * scaling);
        m_map_bottom        = (int)( 10.0f * scaling);
    }
    else if (m_multitouch_gui != NULL  && !m_multitouch_gui->isSpectatorMode())
    {
        m_map_left = (int)((irr_driver->getActualScreenSize().Width - 
                                                        m_map_width) * 0.95f);
        m_map_bottom = (int)(irr_driver->getActualScreenSize().Height - 
                                                    top_margin - m_map_height);
    }
}  // calculateMinimapSize

//-----------------------------------------------------------------------------
/** Render all global parts of the race gui, i.e. things that are only
 *  displayed once even in splitscreen.
 *  \param dt Timestep sized.
 */
void RaceGUI::renderGlobal(float dt)
{
#ifndef SERVER_ONLY
    RaceGUIBase::renderGlobal(dt);
    cleanupMessages(dt);

    // Special case : when 3 players play, use 4th window to display such
    // stuff (but we must clear it)
    if (RaceManager::get()->getIfEmptyScreenSpaceExists() &&
        !GUIEngine::ModalDialog::isADialogActive())
    {
        static video::SColor black = video::SColor(255,0,0,0);

        GL32_draw2DRectangle(black, irr_driver->getSplitscreenWindow(
            RaceManager::get()->getNumLocalPlayers()));
    }

    World *world = World::getWorld();
    assert(world != NULL);
    if(world->getPhase() >= WorldStatus::WAIT_FOR_SERVER_PHASE &&
       world->getPhase() <= WorldStatus::GO_PHASE      )
    {
        drawGlobalReadySetGo();
    }
    else if (world->isGoalPhase())
        drawGlobalGoal();

    if (!m_enabled) return;

    // Display the story mode timer if not in speedrun mode
    // If in speedrun mode, it is taken care of in GUI engine
    // as it must be displayed in all the game's screens
    if (UserConfigParams::m_display_story_mode_timer &&
        !UserConfigParams::m_speedrun_mode &&
        RaceManager::get()->raceWasStartedFromOverworld())
        irr_driver->displayStoryModeTimer();

    // MiniMap is drawn when the players wait for the start countdown to end
    drawGlobalMiniMap();

    // Timer etc. are not displayed unless the game is actually started.
    if(!world->isRacePhase()) return;

    //drawGlobalTimer checks if it should display in the current phase/mode
    drawGlobalTimer();

    if (!m_is_tutorial)
    {
        if (RaceManager::get()->isLinearRaceMode() &&
            RaceManager::get()->hasGhostKarts() &&
            RaceManager::get()->getNumberOfKarts() >= 2 )
            drawLiveDifference();

        if(world->getPhase() == WorldStatus::GO_PHASE ||
           world->getPhase() == WorldStatus::MUSIC_PHASE)
        {
            drawGlobalMusicDescription();
        }
    }

    if (!m_is_tutorial)
    {
        if (m_multitouch_gui != NULL)
        {
            drawGlobalPlayerIcons(m_multitouch_gui->getHeight());
        }
        else if (UserConfigParams::m_minimap_display == 0 || /*map in the bottom-left*/
                (UserConfigParams::m_minimap_display == 1 &&
                RaceManager::get()->getNumLocalPlayers() >= 2))
        {
            drawGlobalPlayerIcons(m_map_height + m_map_bottom);
        }
        else // map hidden or on the right side
        {
            drawGlobalPlayerIcons(0);
        }
    }
#endif
}   // renderGlobal

//-----------------------------------------------------------------------------
/** Render the details for a single player, i.e. speed, energy,
 *  collectibles, ...
 *  \param kart Pointer to the kart for which to render the view.
 */
void RaceGUI::renderPlayerView(const Camera *camera, float dt)
{
    if (!m_enabled) return;

    RaceGUIBase::renderPlayerView(camera, dt);
    
    const core::recti &viewport = camera->getViewport();

    core::vector2df scaling = camera->getScaling();
    const AbstractKart *kart = camera->getKart();
    if(!kart) return;
    
    drawPlungerInFace(camera, dt);

    if (viewport.getWidth() != (int)irr_driver->getActualScreenSize().Width)
    {
        scaling *= float(viewport.getWidth()) / float(irr_driver->getActualScreenSize().Width); // scale race GUI along screen size
    }
    else
    {
        scaling *= float(viewport.getWidth()) / 800.0f; // scale race GUI along screen size
    }
    
    drawAllMessages(kart, viewport, scaling);

    if(!World::getWorld()->isRacePhase()) return;

    if (m_multitouch_gui == NULL || m_multitouch_gui->isSpectatorMode())
    {
        drawPowerupIcons(kart, viewport, scaling);
        drawSpeedEnergyRank(kart, viewport, scaling, dt);
    }

    if (!m_is_tutorial)
        drawLap(kart, viewport, scaling);

    // Radar (experimental)
    if (UserConfigParams::m_radar_enabled) 
        RaceGUI::drawRadar(kart);
    
}   // renderPlayerView

//-----------------------------------------------------------------------------
/** Displays the racing time on the screen.
 */
void RaceGUI::drawGlobalTimer()
{
    assert(World::getWorld() != NULL);

    if (!World::getWorld()->shouldDrawTimer())
    {
        return;
    }

    core::stringw sw;
    video::SColor time_color = video::SColor(255, 255, 255, 255);
    int dist_from_right = 10 + m_timer_width;

    bool use_digit_font = true;

    float elapsed_time = World::getWorld()->getTime();
    if (!RaceManager::get()->hasTimeTarget() ||
        RaceManager::get()->getMinorMode() ==RaceManager::MINOR_MODE_SOCCER ||
        RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_FREE_FOR_ALL ||
        RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_CAPTURE_THE_FLAG)
    {
        sw = core::stringw (
            StringUtils::timeToString(elapsed_time).c_str() );
    }
    else
    {
        float time_target = RaceManager::get()->getTimeTarget();
        if (elapsed_time < time_target)
        {
            sw = core::stringw (
              StringUtils::timeToString(time_target - elapsed_time).c_str());
        }
        else
        {
            sw = _("Challenge Failed");
            int string_width =
                GUIEngine::getFont()->getDimension(sw.c_str()).Width;
            dist_from_right = 10 + string_width;
            time_color = video::SColor(255,255,0,0);
            use_digit_font = false;
        }
    }

    core::rect<s32> pos(irr_driver->getActualScreenSize().Width - dist_from_right,
                        irr_driver->getActualScreenSize().Height*2/100,
                        irr_driver->getActualScreenSize().Width,
                        irr_driver->getActualScreenSize().Height*6/100);

    // special case : when 3 players play, use available 4th space for such things
    if (RaceManager::get()->getIfEmptyScreenSpaceExists())
    {
        pos -= core::vector2d<s32>(0, pos.LowerRightCorner.Y / 2);
        pos += core::vector2d<s32>(0, irr_driver->getActualScreenSize().Height - irr_driver->getSplitscreenWindow(0).getHeight());
    }

    gui::ScalableFont* font = (use_digit_font ? GUIEngine::getHighresDigitFont() : GUIEngine::getFont());
    if (use_digit_font)
        font->setShadow(video::SColor(255, 128, 0, 0));
    font->setScale(1.0f);
    font->setBlackBorder(true);
    font->draw(sw, pos, time_color, false, false, NULL,
               true /* ignore RTL */);
    font->setBlackBorder(false);

}   // drawGlobalTimer


//-----------------------------------------------------------------------------
/** Displays the live difference with a ghost on screen.
 */
void RaceGUI::drawLiveDifference()
{
    assert(World::getWorld() != NULL);

    if (!World::getWorld()->shouldDrawTimer())
    {
        return;
    }

    const LinearWorld *linearworld = dynamic_cast<LinearWorld*>(World::getWorld());
    assert(linearworld != NULL);

    // Don't display the live difference timer if its time is wrong
    // (before crossing the start line at start or after crossing it at end)
    if (!linearworld->hasValidTimeDifference())
        return;

    float live_difference = linearworld->getLiveTimeDifference();

    int timer_width = m_small_precise_timer_width;
    
    // 59.9995 is the smallest number of seconds that could get rounded to 1 minute
    // when rounding at the closest ms
    if (fabsf(live_difference) >= 59.9995f)
        timer_width = m_big_precise_timer_width;

    if (live_difference < 0.0f)
        timer_width += m_negative_timer_additional_width;

    core::stringw sw;
    video::SColor time_color;

    // Change color depending on value
    if (live_difference > 1.0f)
        time_color = video::SColor(255, 255, 0, 0);
    else if (live_difference > 0.0f)
        time_color = video::SColor(255, 255, 160, 0);
    else if (live_difference > -1.0f)
        time_color = video::SColor(255, 160, 255, 0);
    else
        time_color = video::SColor(255, 0, 255, 0);

    int dist_from_right = 10 + timer_width;

    sw = core::stringw (StringUtils::timeToString(live_difference,3,
                        /* display_minutes_if_zero */ false).c_str() );

    core::rect<s32> pos(irr_driver->getActualScreenSize().Width - dist_from_right,
                        irr_driver->getActualScreenSize().Height*7/100,
                        irr_driver->getActualScreenSize().Width,
                        irr_driver->getActualScreenSize().Height*11/100);

    gui::ScalableFont* font = GUIEngine::getHighresDigitFont();
    font->setShadow(video::SColor(255, 128, 0, 0));
    font->setScale(1.0f);
    font->setBlackBorder(true);
    font->draw(sw.c_str(), pos, time_color, false, false, NULL,
               true /* ignore RTL */);
    font->setBlackBorder(false);
}   // drawLiveDifference

//-----------------------------------------------------------------------------
/** Draws extra infos
 */


#define DISTANCE(a, b)  float((a-b).length())

#define TRIANGLE_AREA(e1, e2, e3) std::abs( \
    e1.getX() * (e3.getZ() - e2.getZ()) + \
    e2.getX() * (e1.getZ() - e3.getZ()) + \
    e3.getX() * (e2.getZ() - e1.getZ()))

typedef struct {
  float angle;
  int type;
  video::SColor arrow_color;
  float distance;
  int angle_deg_max;  // angle of part to highlight on circle
  video::SColor angle_deg_color;
  float angle_deg;  // angle of part to highlight on circle
} RadarArrow;

//-----------------------------------------------------------------------------
/** Draws the radar to represent targets in the current mode
*/
void RaceGUI::drawRadar(const AbstractKart* target_kart)
{
#ifndef SERVER_ONLY
    if (target_kart == NULL  || target_kart->hasFinishedRace()) return;
    World* world = World::getWorld();
    if (world->isGoalPhase()) return ;
    CaptureTheFlag *ctf_world = dynamic_cast<CaptureTheFlag*>(world);
    SoccerWorld *soccer_world = dynamic_cast<SoccerWorld*>(world);
    World::KartList karts = world->getKarts();

    std::vector<RadarArrow> radar_arrows;

    Vec3 tx;
    Vec3 ty;
    Vec3 th;
    Vec3 r1;
    Vec3 r2;
    Vec3 p;
    Vec3 d;
    Vec3 to_target;

#define DRAWLINE(a, b, color) draw3DLine( a.toIrrVector(), b.toIrrVector(), color);

#define SETPLAN(h) \
    p = Vec3(-(h.getZ()), h.getY(), h.getX()).normalize();

#define DRAWCIRCLE(size, color, definition){ \
    ty = Vec3(0, size, 0); \
    r2 = radar_circle_pos + ty; \
    for (int i=0; i<definition; i++) { \
      ty = ty.rotate(cam_direction, (360/definition) * DEGREE_TO_RAD); \
      r1 = radar_circle_pos + ty; \
      DRAWLINE(r1, r2, color); \
      r2 = r1; \
    } \
}

    // ty = Vec3(0, size, 0).rotate(cam_direction, angle * DEGREE_TO_RAD); \

#define DRAWCENTEREDSEMIARROW(size, color, angle){ \
    ty = Vec3(0, (((radar_circle_size/2)>size) ? ((radar_circle_size/2)-(size)):0.1), 0).rotate(cam_direction, angle * DEGREE_TO_RAD); \
    r2 = radar_circle_pos + ty; \
    tx = ty.rotate(cam_direction, 140 * DEGREE_TO_RAD); \
    ty = ty.rotate(cam_direction, -140 * DEGREE_TO_RAD); \
    r1 = radar_circle_pos + tx; \
    DRAWLINE(r1, r2, color); \
    ty = radar_circle_pos + ty; \
    DRAWLINE(ty, r2, color); \
}

#define DRAWCENTEREDARROW(size, color, angle){ \
    DRAWCENTEREDSEMIARROW(size, color, angle); \
    DRAWLINE(r1, radar_circle_pos, color); \
    DRAWLINE(ty, radar_circle_pos, color); \
}

#define DRAWCENTEREDARROW2(size, color, angle)\
  DRAWCENTEREDARROW(size, color, angle); \
  DRAWCENTEREDSEMIARROW(size + 0.01, color, angle);

#define DRAWCIRCLE2(size, color, definition) \
  DRAWCIRCLE(size, color, definition); \
  DRAWCIRCLE(size + 0.01, color, definition);

#define DRAWCIRCLE3(size, color, definition) \
  DRAWCIRCLE(size, color, definition); \
  DRAWCIRCLE(size + 0.01, color, definition); \
  DRAWCIRCLE(size + 0.02, color, definition);


#define DRAWSQUARE(a, color, rect_size) \
    tx = p * rect_size; \
    ty = Vec3(0, rect_size, 0); \
    r1 = a-tx+ty; \
    r2 = a+tx+ty; \
    DRAWLINE(r1, r2, color); \
    th = a-tx-ty; \
    DRAWLINE(r1, th, color); \
    tx = a+tx-ty; \
    DRAWLINE(r2, tx, color); \
    DRAWLINE(tx, th, color);

    // Get the point 2 objects will meet
    // a + >a * t = b + >b * t
    // t = (a - b) / (>b - >a)
#define CROSSPOINT(a, va, b, vb) a + va * ((a-b).length() / (vb-va).length());

    float angle;
    float deg_angle = 0;
#define GETANGLE(a,b) \
    angle = atan2f((a.getX()*b.getZ()-b.getX()*a.getZ()),(a.getX()*b.getX())+(a.getZ()*b.getZ())); \
    deg_angle = 360.0f+(angle*RAD_TO_DEGREE);


    Vec3 kart_velocity;
    Vec3 cur_kart_velocity = target_kart->getBody()->getLinearVelocity();
    Vec3 cur_kart_pos = target_kart->getSmoothedXYZ();
    Vec3 cur_kart_axis = quatRotate(target_kart->getVisualRotation(), Vec3(0,0,1));
    Vec3 cur_kart_front_pos = cur_kart_pos + cur_kart_axis;

    const bool backwards = (target_kart->getControls()).getLookBack();
    btTransform trans = target_kart->getTrans();
    // get heading=trans.getBasis*(0,0,1) ... so save the multiplication:
    Vec3 direction(trans.getBasis().getColumn(2));
    direction = direction.normalize();
    Vec3  v = backwards ? -direction : direction;

    Camera* cam = Camera::getActiveCamera();
    Vec3 cam_direction = (Vec3((cam->getCameraSceneNode())->getTarget()) - cam->getXYZ()).normalize();
    Vec3 radar_offset = Vec3(0, UserConfigParams::m_radar_offset_y, 0);
    float radar_circle_size = UserConfigParams::m_radar_size;
    float radar_incircle_max_size = UserConfigParams::m_radar_size*0.5f;
    float radar_incircle_high_size = UserConfigParams::m_radar_size*0.75f;
    float radar_incircle_mid_size = UserConfigParams::m_radar_size*0.85f;
    float radar_incircle_low_size = UserConfigParams::m_radar_size*0.90f;
    float radar_incircle_zero_size = UserConfigParams::m_radar_size*0.96f;
    Vec3 radar_circle_pos=cur_kart_pos + radar_offset + (cam_direction * Vec3(1,0,1));

    Vec3 kart_pos ;
    float tick_angle;
    float distance = 0;

    video::SColor blue_color = video::SColor(255, 0, 0, 200);
    video::SColor red_color = video::SColor(255, 200, 0, 0);
    video::SColor ok_color = video::SColor(255, 0, 255, 0);
    video::SColor kart_line_color = video::SColor(220, 73, 73, 0);
    video::SColor target_align_color = video::SColor(255, 0, 255, 0);
    video::SColor kart_tunnel_color = video::SColor(220, 109, 109, 109);
    video::SColor radar_arrow_color = video::SColor(255,0,0,0);
    video::SColor radar_circle_color = video::SColor(220,255,255,255);
    video::SColor radar_circle_color_muted = video::SColor(100,255,255,255);
    video::SColor radar_highlight_color = video::SColor(240,255,255,255);
    video::SColor radar_pointer_color = video::SColor(255,255,255,255);
    video::SColor radar_inside_color = video::SColor(220, 73, 73, 73);
    video::SColor radar_inside_color2 = radar_inside_color;
    video::SColor color = radar_inside_color;

    KartTeam cur_team = world->getKartTeam(target_kart->getWorldKartId());
    KartTeam other_team = (cur_team == KART_TEAM_BLUE ? KART_TEAM_RED: KART_TEAM_BLUE);

#define TOLOG(a) ((std::log10(a+1) / 2) - 0.2)
#define DISTANCELOG(a,b) (TOLOG(DISTANCE(a, b)))

    Vec3 delta_pos;
    float powerup_speed=25.0f;
    float bowling_speed;
    float slowingdown;
    // evaluate the crosspoint of the target with the ball
    //  speed factor of the ball is smoothed between 1 and 4
    //  given the distance
#define BOWLING_CHECK(a, va, max_distance) \
      if (powerup_type == PowerupManager::POWERUP_BOWLING){ \
        distance = DISTANCE(a, cur_kart_pos); \
        if (distance < max_distance){ \
        /* check future alignement from kart to target */ \
        slowingdown = ( 1 - std::pow(0.8f, (int) (distance/10)) ); \
        ty = va * slowingdown; \
        bowling_speed = cur_kart_velocity.length() + (powerup_speed * 4.0f * slowingdown); \
        tx = cur_kart_velocity.normalize() * bowling_speed; \
        delta_pos = CROSSPOINT(a, ty, cur_kart_pos, tx); \
        distance = TRIANGLE_AREA(delta_pos, cur_kart_pos, cur_kart_front_pos)*50; \
        if (distance < 255) { \
          d = a - cur_kart_pos; \
          SETPLAN(d); \
          if (distance < 10) { \
            radar_inside_color = target_align_color; \
            DRAWSQUARE(a, radar_inside_color, 1.0f); \
          } \
          else \
          { \
            radar_inside_color = COMPUTE_RADAR_COLOR(distance); \
          } \
          DRAWSQUARE( a, radar_inside_color, 0.55f); \
          DRAWSQUARE( a, radar_inside_color, 0.5f); \
          DRAWSQUARE( delta_pos, radar_pointer_color, 0.25f); \
        } \
      } \
    }

#define COMPUTE_RADAR_COLOR(distance) \
      video::SColor(220, 255-distance, 120+(distance/2), distance);

    Vec3 cur_kart_pos1 = cur_kart_pos + cur_kart_velocity/4;
    if (cur_kart_velocity.length() > 1){
      GETANGLE(v, cur_kart_velocity);
      angle *= (backwards ? -8 : 8);
      deg_angle *= (backwards ? -8 : 8);
      distance = TOLOG(cur_kart_velocity.length());
      radar_arrows.push_back({angle, 2, kart_line_color, distance, 10, radar_highlight_color, deg_angle});
    }
    // tick_angle = angle * 2;
    // Vec3 rotate_axis = Vec3(0,1,0);
    /* 
     * ty = cur_kart_velocity.rotate(rotate_axis, tick_angle); \
     * Vec3 cur_kart_pos2 = cur_kart_pos1 + ty/8;
     * tick_angle = angle * 4;
     * ty = cur_kart_velocity.rotate(rotate_axis, tick_angle); \
     * Vec3 cur_kart_pos3 = cur_kart_pos2 + ty/4;
     * tick_angle = angle * 8;
     * ty = cur_kart_velocity.rotate(rotate_axis, tick_angle); \
     * Vec3 cur_kart_pos4 = cur_kart_pos3 + ty/2;
     * tick_angle = angle * 16;
     * ty = cur_kart_velocity.rotate(rotate_axis, tick_angle); \
     * Vec3 cur_kart_pos5 = cur_kart_pos4 + ty;
     */

    // draw a vector representing the axis of the kart
    // DRAWLINE( cur_kart_front_pos, r1, kart_line_color);

    /* draw player kart trajectory */
    /* 
     * DRAWLINE( cur_kart_pos1, cur_kart_pos2, kart_line_color);
     * DRAWLINE( cur_kart_pos2, cur_kart_pos3, kart_line_color);
     * DRAWLINE( cur_kart_pos3, cur_kart_pos4, kart_line_color);
     * DRAWLINE( cur_kart_pos4, cur_kart_pos5, kart_line_color);
     * SETPLAN(v);
     * DRAWSQUARE( cur_kart_pos1, kart_tunnel_color, 0.05f);
     * DRAWSQUARE( cur_kart_pos2, kart_tunnel_color, 0.05f);
     * DRAWSQUARE( cur_kart_pos3, kart_tunnel_color, 0.05f);
     * DRAWSQUARE( cur_kart_pos4, kart_tunnel_color, 0.05f);
     * DRAWSQUARE( cur_kart_pos5, kart_tunnel_color, 0.05f);
     */

    PowerupManager::PowerupType powerup_type = PowerupManager::POWERUP_NOTHING;
    if (target_kart->getNumPowerup() > 0){
      const Powerup* powerup = target_kart->getPowerup();
      powerup_type = powerup->getType();
      /* how to get powerup_speed here ? */
    }

    Vec3 ball_pos1;
    if (soccer_world){
      video::SColor ball_line_color = video::SColor(240, 0, 255, 0);
      video::SColor ball_line_color2 = video::SColor(150, 0, 255, 0);
      video::SColor ball_tunnel_color = video::SColor(220, 0, 0, 0);

      Vec3 ball_pos = soccer_world->getBallPosition();
      Vec3 ball_pos_delta = ball_pos;
      float ball_radius = (soccer_world->getBallDiameter()*0.4);

      to_target = ball_pos - kart_pos;

      ball_pos1 = ball_pos;
      Vec3 ball_pos2 = ball_pos;
      Vec3 ball_pos3 = ball_pos;
      Vec3 ball_pos4 = ball_pos;
      Vec3 linear_velocity = Vec3(0, 0, 0);

      color=radar_circle_color;
      if (!soccer_world->ballNotMoving()){
         /* draw ball trajectory */
         linear_velocity = soccer_world->getBallLinearVelocity();
         ball_pos_delta = CROSSPOINT(ball_pos, linear_velocity, cur_kart_pos, cur_kart_velocity);

         ball_pos1 = ball_pos + linear_velocity/4;
         ball_pos2 = ball_pos + linear_velocity/2;
         ball_pos3 = ball_pos + linear_velocity;
         ball_pos4 = ball_pos + linear_velocity*2;
         SETPLAN(linear_velocity);
         DRAWSQUARE(ball_pos1, ball_tunnel_color, 0.25f);
         DRAWSQUARE(ball_pos2, ball_tunnel_color, 0.25f);
         DRAWSQUARE(ball_pos3, ball_tunnel_color, 0.25f);
         DRAWSQUARE(ball_pos4, ball_tunnel_color, 0.25f);
         DRAWLINE( ball_pos, ball_pos4, ball_line_color);
         tx = ball_pos - (p.normalize() * ball_radius) ;
         ty = ball_pos4 - (p.normalize() * ball_radius) ;
         DRAWLINE( tx, ty, ball_line_color2);
         tx = ball_pos - (p.normalize() * ball_radius) ;
         ty = ball_pos + (p.normalize() * ball_radius) ;
         DRAWLINE( tx, ty, ball_line_color2);
         tx = ball_pos4 - (p.normalize() * ball_radius) ;
         ty = ball_pos4 + (p.normalize() * ball_radius) ;
         DRAWLINE( tx, ty, ball_line_color2);
         tx = ball_pos1 - (p.normalize() * ball_radius) ;
         ty = ball_pos1 + (p.normalize() * ball_radius) ;
         DRAWLINE( tx, ty, ball_line_color2);
         tx = ball_pos2 - (p.normalize() * ball_radius) ;
         ty = ball_pos2 + (p.normalize() * ball_radius) ;
         DRAWLINE( tx, ty, ball_line_color2);
         tx = ball_pos3 - (p.normalize() * ball_radius) ;
         ty = ball_pos3 + (p.normalize() * ball_radius) ;
         DRAWLINE( tx, ty, ball_line_color2);
         tx = ball_pos - (p.normalize() * ball_radius) ;
         ty = ball_pos4 - (p.normalize() * ball_radius) ;
         DRAWLINE( tx, ty, ball_line_color2);
         tx = ball_pos + (p.normalize() * ball_radius) ;
         ty = ball_pos4 +(p.normalize() * ball_radius) ;
         DRAWLINE( tx, ty, ball_line_color2);
         tx = ball_pos + (p.normalize() * ball_radius) ;
         ty = ball_pos4 + (p.normalize() * ball_radius) ;
         DRAWLINE( tx, ty, ball_line_color2);

      }
      /* 
       * DRAWCIRCLE((TOLOG(25)*radar_circle_size), radar_circle_color_muted, 36); 
       * DRAWCIRCLE((TOLOG(5)*radar_circle_size), radar_circle_color_muted, 36); 
       */
      distance = DISTANCELOG(ball_pos, cur_kart_pos);
      if (distance < 0.5) tx = (ball_pos_delta-cur_kart_pos).normalize();
      else tx = (ball_pos - cur_kart_pos).normalize();
      distance = (distance*(radar_circle_size/2));
      GETANGLE(v, tx); 
      DRAWCENTEREDARROW2(distance, color, deg_angle); 

      // Vec3 ball_aim_pos = soccer_world->getBallAimPosition(other_team, backwards);
      // ball_aim_pos = ball_pos + ((ball_aim_pos-ball_pos).normalize() * (ball_radius));
      // SETPLAN(linear_velocity);
      // DRAWSQUARE(ball_aim_pos, ok_color, 0.1f);
      // DRAWLINE(ball_aim_pos, ball_pos, ok_color);

      /* draw ball a rectangle representing the shock if the kart touch the ball */
      // distance = DISTANCE(linear_velocity, cur_kart_velocity)/100 + 0.1f;
      // DRAWSQUARE(ball_pos, ball_line_color, 0.1f); 
      // DRAWSQUARE(ball_pos, ball_line_color, distance); 


      Vec3 cur_kart_front_pos_delta;

      /* check future alignement from kart to ball */
      /* with color tips for alignement (blue = cold, red=hot, green=perfect)*/
      distance = TRIANGLE_AREA(ball_pos_delta, cur_kart_pos, cur_kart_front_pos)*40;
      if (distance > 255)  distance=255;
      radar_inside_color2 = COMPUTE_RADAR_COLOR(distance);
      d = ball_pos_delta - cur_kart_pos;
      SETPLAN(d);
      if (distance < 50) {
        DRAWSQUARE(ball_pos_delta, radar_inside_color2, 0.2f);
      }
      /* check current alignement from kart to ball */
      distance = TRIANGLE_AREA(ball_pos, cur_kart_pos, cur_kart_front_pos);
      if (distance < 0.015f)
      {
        radar_inside_color = target_align_color;
        SETPLAN(to_target);
        DRAWSQUARE( ball_pos, radar_inside_color, 0.2f);
      } else {
        radar_inside_color = radar_inside_color2;
      }

      BOWLING_CHECK(ball_pos, linear_velocity, 300.0f);

      tx = (ball_pos-cur_kart_pos).normalize();
      GETANGLE(v, tx); 
      distance = DISTANCELOG(cur_kart_pos1, ball_pos1) * radar_circle_size; 
      radar_arrows.push_back({angle, 1, radar_arrow_color, distance, 10, radar_highlight_color, deg_angle});

    } /* end soccer case */
    else if (ctf_world)
    {
        Vec3 base_pos;
        bool is_flag_in_base=true;
        bool is_red_holder=false;
        bool is_blue_holder=false;

        /* if the flag is hold draw an arrow to the base */
        if (cur_team == KART_TEAM_BLUE && (ctf_world->getRedHolder()  != -1)){
          base_pos = ctf_world->getBlueFlag();
          is_red_holder=true;
          is_flag_in_base = ctf_world->isBlueFlagInBase();
        }
        if (cur_team == KART_TEAM_RED && (ctf_world->getBlueHolder()  != -1)){
          base_pos = ctf_world->getRedFlag();
          is_blue_holder=true;
          is_flag_in_base = ctf_world->isRedFlagInBase();
        }
        if ((is_blue_holder || is_red_holder) && is_flag_in_base) {
          distance = DISTANCELOG(cur_kart_pos, base_pos) * radar_circle_size;
          tx = (base_pos-cur_kart_pos).normalize();
          GETANGLE(v, tx); 
          radar_arrows.push_back({angle, 1, radar_arrow_color, distance, 10, radar_highlight_color, deg_angle});
        }
        /* draw an arrow to find the flag you need to get */
        if (cur_team == KART_TEAM_BLUE ? !is_red_holder : !is_flag_in_base)
        {
          Vec3 red_flag_pos = ctf_world->getRedFlag();
          distance = DISTANCELOG(cur_kart_pos, red_flag_pos) * radar_circle_size;
          tx = (red_flag_pos-cur_kart_pos).normalize();
          GETANGLE(v, tx); 
          radar_arrows.push_back({angle, 1, red_color, distance, 10, red_color, deg_angle});
        }
        if (cur_team == KART_TEAM_RED ? !is_blue_holder : !is_flag_in_base)
        {
          Vec3 blue_flag_pos =  ctf_world->getBlueFlag();
          distance = DISTANCELOG(cur_kart_pos, blue_flag_pos) * radar_circle_size;
          tx = (blue_flag_pos-cur_kart_pos).normalize();
          GETANGLE(v, tx); 
          radar_arrows.push_back({angle, 1, blue_color, distance, 10, blue_color, deg_angle});
        }
        // distance = distance;
        DRAWCENTEREDARROW2(distance, radar_circle_color, deg_angle); 
    } /* end ctf case */

    bool has_teams = (ctf_world || soccer_world);
    KartTeam team = KART_TEAM_NONE;
    btTransform trans_projectile = target_kart->getTrans();

    float minDistSquared = 999999.9f;
    float minDistance = 999999.9f;
    Vec3 minKart_pos = Vec3(0,0,0);
    const AbstractKart *minKart = NULL;
    bool in_front = false;
    for (unsigned int i = 0; i < karts.size(); i++)
    {
      const AbstractKart *kart = karts[i].get();
      if (kart == target_kart || kart -> isEliminated() || !kart->isVisible() || kart->getKartAnimation())
        continue;
      Vec3 kart_pos = kart->getSmoothedXYZ();
      kart_velocity = kart->getBody()->getLinearVelocity();
      Vec3 kart_pos1 = kart_pos + kart_velocity / 4;
      if (has_teams) team = world->getKartTeam(kart->getWorldKartId());
      distance = DISTANCE(cur_kart_pos1, kart_pos1);

      to_target  = kart_pos - cur_kart_pos;
      // Originally it used angle = to_target.angle( backwards ? -direction : direction );
      // but sometimes due to rounding errors we get an acos(x) with x>1, causing
      // an assertion failure. So we remove the whole acos() test here and copy the
      // code from to_target.angle(...)
      in_front = ((to_target.dot(v)/sqrt(v.length2() * to_target.length2()))>=0.54);

      if (!kart->isInvulnerable() && !(has_teams && cur_team == team) && (distance <= 50) && in_front){

        btTransform t=kart->getTrans();

        Vec3 delta      = t.getOrigin()-trans_projectile.getOrigin();
        float distance2 = delta.length2() + std::abs(t.getOrigin().getY()
            - trans_projectile.getOrigin().getY())*2;

        // Original test was: fabsf(acos(c))>1,  which is the same as
        // c<cos(1) (acos returns values in [0, pi] anyway)
        if(distance2 < minDistSquared)
        {
            minDistance = distance;
            minKart  = kart;
            minKart_pos  = kart_pos;
        }
      }

      if (soccer_world) {
        /* draw a cursor to represent each kart on the way to touch the ball */ 
        distance = DISTANCE(kart_pos1, ball_pos1);
        if (distance <= 25.0f) {
          if (team == KART_TEAM_RED) color=video::SColor(200, 255-distance,distance, distance);
          else if (team == KART_TEAM_BLUE)  color=video::SColor(200, distance,distance, 255-distance);
          distance = DISTANCELOG(cur_kart_pos, kart_pos) * radar_circle_size;
          tx = (kart_pos1-cur_kart_pos).normalize();
          GETANGLE(v, tx);
          radar_arrows.push_back({angle, 0, color, distance, 0, color, deg_angle});
        }
      } else {
        /* draw a cursor to represent each kart on the way to hit your kart */ 
        if (distance <= 25.0f || RaceManager::get()->isBattleMode()) {
          distance = TOLOG(distance) * radar_circle_size;
          tx = (kart_pos1-cur_kart_pos).normalize();
          GETANGLE(v, tx);
          color = kart->getKartProperties()->getColor();
          radar_arrows.push_back({angle, 1, color, distance, 5, color, deg_angle});
        }
        BOWLING_CHECK(kart_pos, kart_velocity, 100.0f);
      }
    }
    if (minKart) {
      if (!soccer_world) {
        distance = minDistance;
        if (distance > 255)  distance=255;
        radar_inside_color = COMPUTE_RADAR_COLOR(distance);
        distance = TOLOG(minDistance)*radar_circle_size;
        tx = (minKart_pos - cur_kart_pos).normalize();
        GETANGLE(v, tx);
        DRAWCENTEREDARROW2(distance, radar_circle_color, deg_angle); 
      }
      if (powerup_type == PowerupManager::POWERUP_CAKE && minDistance <= 25.0f){
        to_target = minKart_pos - cur_kart_pos;
        SETPLAN(to_target);
        DRAWSQUARE(minKart_pos, radar_pointer_color, 0.4f);
        DRAWSQUARE(minKart_pos, radar_pointer_color, 0.5f);
      }
    }

#define DRAWTICKAROUNDRADAR(angle, color, p1, p2) \
    ty = Vec3(0, 1, 0); \
    ty = ty.rotate(cam_direction, angle); \
    r1 = radar_circle_pos + (ty*p1); \
    r2 = radar_circle_pos + (ty*p2); \
    DRAWLINE(r1, r2, color);

#define DRAW_RADAR_ARROW(angle, angletri, color, p1, p2, d) \
    ty = Vec3(0, 1, 0); \
    r1 = ty.rotate(cam_direction, angle); \
    tx = radar_circle_pos + (r1*(p1+d)); \
    th = radar_circle_pos + (r1*p2); \
    DRAWLINE(tx, th, color);\
    r1 = radar_circle_pos + (r1*p1); \
    r2 = (tx - r1) ; \
    angle-=angletri;\
    r1 = ty.rotate(cam_direction, angle); \
    tx = radar_circle_pos + (r1*p1); \
    DRAWLINE(tx, th, color); \
    angle+=2*angletri;\
    r1 = ty.rotate(cam_direction, angle); \
    ty = radar_circle_pos + (r1*p1); \
    DRAWLINE(ty, th, color);\
    DRAWLINE(ty, tx, color);\
    r1 = ty + r2;\
    r2 = tx + r2;\
    DRAWLINE(r1, r2, color);

    int nb_arrows = (int)radar_arrows.size();

    float size;
    float radar_circle_radius=radar_circle_size/2;
    float radar_incircle_high_radius=radar_incircle_high_size/2;
    float radar_incircle_mid_radius=radar_incircle_mid_size/2;
    float radar_incircle_low_radius=radar_incircle_low_size/2;
    float radar_incircle_zero_radius=radar_incircle_zero_size/2;
    float radar_incircle_max_radius=radar_incircle_max_size/2;
    bool ortho;
    // DRAW RADAR
    for (int i=0;i<36;i++)
    {
      ortho = false;
      if (i % 9 == 0) {
        color = radar_inside_color;
        size = radar_incircle_high_radius;
        ortho = true;
      } else if (i % 3 == 0) {
        color = radar_inside_color;
        size = radar_incircle_mid_radius;
        ortho = true;
      } else {
        size = radar_circle_radius;
      }
      for (int n=0; n<nb_arrows ; n++) {
        if (radar_arrows[n].angle_deg != 0) {
          distance = radar_arrows[n].angle_deg-(i*10);
          if (distance<0) distance+=360;
          distance = (distance > 360) ? (distance - 360): (distance > 180 ? 360 - distance: distance);

          if (distance < radar_arrows[n].angle_deg_max) {
            if (ortho) {
              color = radar_arrows[n].angle_deg_color;    
              size = radar_incircle_max_radius;
            }
            break;
          }
        }
      }
      tick_angle = i * 10 * DEGREE_TO_RAD;
      DRAWTICKAROUNDRADAR(tick_angle, color, radar_circle_radius, size);


      if (i>0) DRAWLINE(th, r1, radar_inside_color)
      else tx = r1; 
      th = r1;
    }
    DRAWLINE(th, tx, radar_inside_color);
    DRAWLINE(th, r1, radar_inside_color);

    angle=4*DEGREE_TO_RAD;
    float angle2=1.5*DEGREE_TO_RAD;

    for (int n=0; n<nb_arrows ; n++) {
      if (radar_arrows[n].type == 1) {
        DRAW_RADAR_ARROW(radar_arrows[n].angle, angle, radar_arrows[n].arrow_color, radar_incircle_low_radius, radar_circle_radius, radar_arrows[n].distance);
      } else if (radar_arrows[n].type == 2) {
        DRAWTICKAROUNDRADAR(radar_arrows[n].angle, radar_arrows[n].arrow_color, radar_circle_radius, (radar_circle_radius + radar_arrows[n].distance));
      } else if (radar_arrows[n].type == 0){
        DRAW_RADAR_ARROW(radar_arrows[n].angle, angle2, radar_arrows[n].arrow_color, radar_incircle_low_radius, radar_incircle_zero_radius, radar_arrows[n].distance);
      }
    }

#endif
} // drawRadar

//-----------------------------------------------------------------------------
/** Draws the mini map and the position of all karts on it.
*/
void RaceGUI::drawGlobalMiniMap()
{
#ifndef SERVER_ONLY
    //TODO : exception for some game modes ? Another option "Hidden in race, shown in battle ?"
    if (UserConfigParams::m_minimap_display == 2) /*map hidden*/
        return;

    if (m_multitouch_gui != NULL && !m_multitouch_gui->isSpectatorMode())
    {
        float max_scale = 1.3f;

        if (UserConfigParams::m_multitouch_scale > max_scale)
            return;
    }

    // draw a map when arena has a navigation mesh.
    Track *track = Track::getCurrentTrack();
    if ( (track->isArena() || track->isSoccer()) && !(track->hasNavMesh()) )
        return;

    int upper_y = irr_driver->getActualScreenSize().Height - m_map_bottom - m_map_height;
    int lower_y = irr_driver->getActualScreenSize().Height - m_map_bottom;

    core::rect<s32> dest(m_map_left, upper_y,
                         m_map_left + m_map_width, lower_y);

    track->drawMiniMap(dest);

    World* world = World::getWorld();
    CaptureTheFlag *ctf_world = dynamic_cast<CaptureTheFlag*>(World::getWorld());
    SoccerWorld *soccer_world = dynamic_cast<SoccerWorld*>(World::getWorld());

    if (ctf_world)
    {
        Vec3 draw_at;
        if (!ctf_world->isRedFlagInBase())
        {
            track->mapPoint2MiniMap(Track::getCurrentTrack()->getRedFlag().getOrigin(),
                &draw_at);
            core::rect<s32> rs(core::position2di(0, 0), m_red_flag->getSize());
            core::rect<s32> rp(m_map_left+(int)(draw_at.getX()-(m_minimap_player_size/1.4f)),
                lower_y   -(int)(draw_at.getY()+(m_minimap_player_size/2.2f)),
                m_map_left+(int)(draw_at.getX()+(m_minimap_player_size/1.4f)),
                lower_y   -(int)(draw_at.getY()-(m_minimap_player_size/2.2f)));
            draw2DImage(m_red_flag, rp, rs, NULL, NULL, true, true);
        }
        Vec3 pos = ctf_world->getRedHolder() == -1 ? ctf_world->getRedFlag() :
            ctf_world->getKart(ctf_world->getRedHolder())->getSmoothedTrans().getOrigin();

        track->mapPoint2MiniMap(pos, &draw_at);
        core::rect<s32> rs(core::position2di(0, 0), m_red_flag->getSize());
        core::rect<s32> rp(m_map_left+(int)(draw_at.getX()-(m_minimap_player_size/1.4f)),
                                 lower_y   -(int)(draw_at.getY()+(m_minimap_player_size/2.2f)),
                                 m_map_left+(int)(draw_at.getX()+(m_minimap_player_size/1.4f)),
                                 lower_y   -(int)(draw_at.getY()-(m_minimap_player_size/2.2f)));
        draw2DImage(m_red_flag, rp, rs, NULL, NULL, true);

        if (!ctf_world->isBlueFlagInBase())
        {
            track->mapPoint2MiniMap(Track::getCurrentTrack()->getBlueFlag().getOrigin(),
                &draw_at);
            core::rect<s32> rs(core::position2di(0, 0), m_blue_flag->getSize());
            core::rect<s32> rp(m_map_left+(int)(draw_at.getX()-(m_minimap_player_size/1.4f)),
                lower_y   -(int)(draw_at.getY()+(m_minimap_player_size/2.2f)),
                m_map_left+(int)(draw_at.getX()+(m_minimap_player_size/1.4f)),
                lower_y   -(int)(draw_at.getY()-(m_minimap_player_size/2.2f)));
            draw2DImage(m_blue_flag, rp, rs, NULL, NULL, true, true);
        }

        pos = ctf_world->getBlueHolder() == -1 ? ctf_world->getBlueFlag() :
            ctf_world->getKart(ctf_world->getBlueHolder())->getSmoothedTrans().getOrigin();

        track->mapPoint2MiniMap(pos, &draw_at);
        core::rect<s32> bs(core::position2di(0, 0), m_blue_flag->getSize());
        core::rect<s32> bp(m_map_left+(int)(draw_at.getX()-(m_minimap_player_size/1.4f)),
                                 lower_y   -(int)(draw_at.getY()+(m_minimap_player_size/2.2f)),
                                 m_map_left+(int)(draw_at.getX()+(m_minimap_player_size/1.4f)),
                                 lower_y   -(int)(draw_at.getY()-(m_minimap_player_size/2.2f)));
        draw2DImage(m_blue_flag, bp, bs, NULL, NULL, true);
    }

    AbstractKart* target_kart = NULL;
    Camera* cam = Camera::getActiveCamera();
    auto cl = LobbyProtocol::get<ClientLobby>();
    bool is_nw_spectate = cl && cl->isSpectator();
    // For network spectator highlight
    if (RaceManager::get()->getNumLocalPlayers() == 1 && cam && is_nw_spectate)
        target_kart = cam->getKart();

    // Move AI/remote players to the beginning, so that local players icons
    // are drawn above them
    World::KartList karts = world->getKarts();
    std::partition(karts.begin(), karts.end(), [target_kart, is_nw_spectate]
        (const std::shared_ptr<AbstractKart>& k)->bool
    {
        if (is_nw_spectate)
            return k.get() != target_kart;
        else
            return !k->getController()->isLocalPlayerController();
    });

    for (unsigned int i = 0; i < karts.size(); i++)
    {
        const AbstractKart *kart = karts[i].get();
        const SpareTireAI* sta =
            dynamic_cast<const SpareTireAI*>(kart->getController());

        // don't draw eliminated kart
        if (kart->isEliminated() && !(sta && sta->isMoving())) 
            continue;
        if (!kart->isVisible())
            continue;
        const Vec3& xyz = kart->getSmoothedTrans().getOrigin();
        Vec3 draw_at;
        track->mapPoint2MiniMap(xyz, &draw_at);

        video::ITexture* icon = sta ? m_heart_icon :
            kart->getKartProperties()->getMinimapIcon();
        if (icon == NULL)
        {
            continue;
        }
        bool is_local = is_nw_spectate ? kart == target_kart :
            kart->getController()->isLocalPlayerController();
        // int marker_height = m_marker->getSize().Height;
        core::rect<s32> source(core::position2di(0, 0), icon->getSize());
        int marker_half_size = (is_local
                                ? m_minimap_player_size
                                : m_minimap_ai_size                        )>>1;
        core::rect<s32> position(m_map_left+(int)(draw_at.getX()-marker_half_size),
                                 lower_y   -(int)(draw_at.getY()+marker_half_size),
                                 m_map_left+(int)(draw_at.getX()+marker_half_size),
                                 lower_y   -(int)(draw_at.getY()-marker_half_size));

        bool has_teams = (ctf_world || soccer_world);
        
        // Highlight the player icons with some backgorund image.
        if ((has_teams || is_local) && m_icons_frame != NULL)
        {
            video::SColor color = kart->getKartProperties()->getColor();
            
            if (has_teams)
            {
                KartTeam team = world->getKartTeam(kart->getWorldKartId());
                
                if (team == KART_TEAM_RED)
                {
                    color = video::SColor(255, 200, 0, 0);
                }
                else if (team == KART_TEAM_BLUE)
                {
                    color = video::SColor(255, 0, 0, 200);
                }
            }
                                  
            video::SColor colors[4] = {color, color, color, color};

            const core::rect<s32> rect(core::position2d<s32>(0,0),
                                        m_icons_frame->getSize());

            // show kart direction in soccer
            if (soccer_world)
            {
                // Find the direction a kart is moving in
                btTransform trans = kart->getTrans();
                Vec3 direction(trans.getBasis().getColumn(2));
                // Get the rotation to rotate the icon frame
                float rotation = atan2f(direction.getZ(),direction.getX());
                if (track->getMinimapInvert())
                {   // correct the direction due to invert minimap for blue
                    rotation = rotation + M_PI;
                }
                rotation = -1.0f * rotation + 0.25f * M_PI; // icons-frame_arrow.png was rotated by 45 degrees
                draw2DImage(m_icons_frame, position, rect, NULL, colors, true, false, rotation);
            }
            else
            {
                draw2DImage(m_icons_frame, position, rect, NULL, colors, true);
            }
        }   // if isPlayerController

        draw2DImage(icon, position, source, NULL, NULL, true);

    }   // for i<getNumKarts

    if (soccer_world)
    {
        Vec3 draw_at;
        track->mapPoint2MiniMap(soccer_world->getBallPosition(), &draw_at);

        core::rect<s32> source(core::position2di(0, 0), m_soccer_ball->getSize());
        core::rect<s32> position(m_map_left+(int)(draw_at.getX()-(m_minimap_player_size/2.5f)),
                                 lower_y   -(int)(draw_at.getY()+(m_minimap_player_size/2.5f)),
                                 m_map_left+(int)(draw_at.getX()+(m_minimap_player_size/2.5f)),
                                 lower_y   -(int)(draw_at.getY()-(m_minimap_player_size/2.5f)));
        draw2DImage(m_soccer_ball, position, source, NULL, NULL, true);
    }
#endif
}   // drawGlobalMiniMap

//-----------------------------------------------------------------------------
/** Energy meter that gets filled with nitro. This function is called from
 *  drawSpeedEnergyRank, which defines the correct position of the energy
 *  meter.
 *  \param x X position of the meter.
 *  \param y Y position of the meter.
 *  \param kart Kart to display the data for.
 *  \param scaling Scaling applied (in case of split screen)
 */
void RaceGUI::drawEnergyMeter(int x, int y, const AbstractKart *kart,
                              const core::recti &viewport,
                              const core::vector2df &scaling)
{
#ifndef SERVER_ONLY
    float min_ratio        = std::min(scaling.X, scaling.Y);
    const int GAUGEWIDTH   = 94;//same inner radius as the inner speedometer circle
    int gauge_width        = (int)(GAUGEWIDTH*min_ratio);
    int gauge_height       = (int)(GAUGEWIDTH*min_ratio);

    float state = (float)(kart->getEnergy())
                / kart->getKartProperties()->getNitroMax();
    if (state < 0.0f) state = 0.0f;
    else if (state > 1.0f) state = 1.0f;

    core::vector2df offset;
    offset.X = (float)(x-gauge_width) - 9.5f*scaling.X;
    offset.Y = (float)y-11.5f*scaling.Y;


    // Background
    draw2DImage(m_gauge_empty, core::rect<s32>((int)offset.X,
                                               (int)offset.Y-gauge_height,
                                               (int)offset.X + gauge_width,
                                               (int)offset.Y) /* dest rect */,
                core::rect<s32>(core::position2d<s32>(0,0),
                                m_gauge_empty->getSize()) /* source rect */,
                NULL /* clip rect */, NULL /* colors */,
                true /* alpha */);

    // The positions for A to G are defined here.
    // They are calculated from gauge_full.png
    // They are further than the nitrometer farther position because
    // the lines between them would otherwise cut through the outside circle.
    
    const int vertices_count = 9;

    core::vector2df position[vertices_count];
    position[0].X = 0.324f;//A
    position[0].Y = 0.35f;//A
    position[1].X = 0.01f;//B1 (margin for gauge goal)
    position[1].Y = 0.88f;//B1
    position[2].X = 0.029f;//B2
    position[2].Y = 0.918f;//B2
    position[3].X = 0.307f;//C
    position[3].Y = 0.99f;//C
    position[4].X = 0.589f;//D
    position[4].Y = 0.932f;//D
    position[5].X = 0.818f;//E
    position[5].Y = 0.755f;//E
    position[6].X = 0.945f;//F
    position[6].Y = 0.497f;//F
    position[7].X = 0.948f;//G1
    position[7].Y = 0.211f;//G1
    position[8].X = 0.94f;//G2 (margin for gauge goal)
    position[8].Y = 0.17f;//G2

    // The states at which different polygons must be used.

    float threshold[vertices_count-2];
    threshold[0] = 0.0001f; //for gauge drawing
    threshold[1] = 0.2f;
    threshold[2] = 0.4f;
    threshold[3] = 0.6f;
    threshold[4] = 0.8f;
    threshold[5] = 0.9999f;
    threshold[6] = 1.0f;

    // Filling (current state)

    if (state > 0.0f)
    {
        video::S3DVertex vertices[vertices_count];

        //3D effect : wait for the full border to appear before drawing
        for (int i=0;i<5;i++)
        {
            if ((state-0.2f*i < 0.006f && state-0.2f*i >= 0.0f) || (0.2f*i-state < 0.003f && 0.2f*i-state >= 0.0f) )
            {
                state = 0.2f*i-0.003f;
                break;
            }
        }

        unsigned int count = computeVerticesForMeter(position, threshold, vertices, vertices_count,
                                                     state, gauge_width, gauge_height, offset);

        if(kart->getControls().getNitro() || kart->isOnMinNitroTime())
            drawMeterTexture(m_gauge_full_bright, vertices, count);
        else
            drawMeterTexture(m_gauge_full, vertices, count);
    }

    // Target

    if (RaceManager::get()->getCoinTarget() > 0)
    {
        float coin_target = (float)RaceManager::get()->getCoinTarget()
                          / kart->getKartProperties()->getNitroMax();

        video::S3DVertex vertices[vertices_count];

        unsigned int count = computeVerticesForMeter(position, threshold, vertices, vertices_count, 
                                                     coin_target, gauge_width, gauge_height, offset);

        drawMeterTexture(m_gauge_goal, vertices, count);
    }
#endif
}   // drawEnergyMeter

//-----------------------------------------------------------------------------
/** Draws the rank of a player.
 *  \param kart The kart of the player.
 *  \param offset Offset of top left corner for this display (for splitscreen).
 *  \param min_ratio Scaling of the screen (for splitscreen).
 *  \param meter_width Width of the meter (inside which the rank is shown).
 *  \param meter_height Height of the meter (inside which the rank is shown).
 *  \param dt Time step size.
 */
void RaceGUI::drawRank(const AbstractKart *kart,
                      const core::vector2df &offset,
                      float min_ratio, int meter_width,
                      int meter_height, float dt)
{
    static video::SColor color = video::SColor(255, 255, 255, 255);

    // Draw rank
    WorldWithRank *world = dynamic_cast<WorldWithRank*>(World::getWorld());
    if (!world || !world->displayRank())
        return;

    int id = kart->getWorldKartId();

    if (m_animation_states[id] == AS_NONE)
    {
        if (m_last_ranks[id] != kart->getPosition())
        {
            m_rank_animation_duration[id] = 0.0f;
            m_animation_states[id] = AS_SMALLER;
        }
    }
    else
    {
        m_rank_animation_duration[id] += dt;
    }

    float scale = 1.0f;
    int rank = kart->getPosition();
    const float DURATION = 0.4f;
    const float MIN_SHRINK = 0.3f;
    if (m_animation_states[id] == AS_SMALLER)
    {
        scale = 1.0f - m_rank_animation_duration[id]/ DURATION;
        rank = m_last_ranks[id];
        if (scale < MIN_SHRINK)
        {
            m_animation_states[id] = AS_BIGGER;
            m_rank_animation_duration[id] = 0.0f;
            // Store the new rank
            m_last_ranks[id] = kart->getPosition();
            scale = MIN_SHRINK;
        }
    }
    else if (m_animation_states[id] == AS_BIGGER)
    {
        scale = m_rank_animation_duration[id] / DURATION + MIN_SHRINK;
        rank = m_last_ranks[id];
        if (scale > 1.0f)
        {
            m_animation_states[id] = AS_NONE;
            scale = 1.0f;
        }

    }
    else
    {
        m_last_ranks[id] = kart->getPosition();
    }

    gui::ScalableFont* font = GUIEngine::getHighresDigitFont();
    
    int font_height = font->getDimension(L"X").Height;
    font->setScale((float)meter_height / font_height * 0.4f * scale);
    font->setShadow(video::SColor(255, 128, 0, 0));
    std::ostringstream oss;
    oss << rank; // the current font has no . :(   << ".";

    core::recti pos;
    pos.LowerRightCorner = core::vector2di(int(offset.X + 0.64f*meter_width),
                                           int(offset.Y - 0.49f*meter_height));
    pos.UpperLeftCorner = core::vector2di(int(offset.X + 0.64f*meter_width),
                                          int(offset.Y - 0.49f*meter_height));

    font->setBlackBorder(true);
    font->draw(oss.str().c_str(), pos, color, true, true);
    font->setBlackBorder(false);
    font->setScale(1.0f);
}   // drawRank

//-----------------------------------------------------------------------------
/** Draws the speedometer, the display of available nitro, and
 *  the rank of the kart (inside the speedometer).
 *  \param kart The kart for which to show the data.
 *  \param viewport The viewport to use.
 *  \param scaling Which scaling to apply to the speedometer.
 *  \param dt Time step size.
 */
void RaceGUI::drawSpeedEnergyRank(const AbstractKart* kart,
                                 const core::recti &viewport,
                                 const core::vector2df &scaling,
                                 float dt)
{
#ifndef SERVER_ONLY
    float min_ratio         = std::min(scaling.X, scaling.Y);
    const int SPEEDWIDTH   = 128;
    int meter_width        = (int)(SPEEDWIDTH*min_ratio);
    int meter_height       = (int)(SPEEDWIDTH*min_ratio);

    drawEnergyMeter(viewport.LowerRightCorner.X ,
                    (int)(viewport.LowerRightCorner.Y),
                    kart, viewport, scaling);

    // First draw the meter (i.e. the background )
    // -------------------------------------------------------------------------
    core::vector2df offset;
    offset.X = (float)(viewport.LowerRightCorner.X-meter_width) - 24.0f*scaling.X;
    offset.Y = viewport.LowerRightCorner.Y-10.0f*scaling.Y;

    const core::rect<s32> meter_pos((int)offset.X,
                                    (int)(offset.Y-meter_height),
                                    (int)(offset.X+meter_width),
                                    (int)offset.Y);
    video::ITexture *meter_texture = m_speed_meter_icon->getTexture();
    const core::rect<s32> meter_texture_coords(core::position2d<s32>(0,0),
                                               meter_texture->getSize());
    draw2DImage(meter_texture, meter_pos, meter_texture_coords, NULL,
                       NULL, true);
    // TODO: temporary workaround, shouldn't have to use
    // draw2DVertexPrimitiveList to render a simple rectangle

    const float speed =  kart->getSpeed();

    drawRank(kart, offset, min_ratio, meter_width, meter_height, dt);


    if(speed <=0) return;  // Nothing to do if speed is negative.

    // Draw the actual speed bar (if the speed is >0)
    // ----------------------------------------------
    float speed_ratio = speed/40.0f; //max displayed speed of 40
    if(speed_ratio>1) speed_ratio = 1;

    // see computeVerticesForMeter for the detail of the drawing
    // If increasing this, update drawMeterTexture

    const int vertices_count = 12;

    video::S3DVertex vertices[vertices_count];

    // The positions for A to J2 are defined here.

    // They are calculated from speedometer.png
    // A is the center of the speedometer's circle
    // B2, C, D, E, F, G, H, I and J1 are points on the line
    // from A to their respective 1/8th threshold division
    // B2 is 36,9° clockwise from the vertical (on bottom-left)
    // J1 s 70,7° clockwise from the vertical (on upper-right)
    // B1 and J2 are used for correct display of the 3D effect
    // They are 1,13* further than the speedometer farther position because
    // the lines between them would otherwise cut through the outside circle.

    core::vector2df position[vertices_count];

    position[0].X = 0.546f;//A
    position[0].Y = 0.566f;//A
    position[1].X = 0.216f;//B1
    position[1].Y = 1.036f;//B1
    position[2].X = 0.201f;//B2
    position[2].Y = 1.023f;//B2
    position[3].X = 0.036f;//C
    position[3].Y = 0.831f;//C
    position[4].X = -0.029f;//D
    position[4].Y = 0.589f;//D
    position[5].X = 0.018f;//E
    position[5].Y = 0.337f;//E
    position[6].X = 0.169f;//F
    position[6].Y = 0.134f;//F
    position[7].X = 0.391f;//G
    position[7].Y = 0.014f;//G
    position[8].X = 0.642f;//H
    position[8].Y = 0.0f;//H
    position[9].X = 0.878f;//I
    position[9].Y = 0.098f;//I
    position[10].X = 1.046f;//J1
    position[10].Y = 0.285f;//J1
    position[11].X = 1.052f;//J2
    position[11].Y = 0.297f;//J2

    // The speed ratios at which different triangles must be used.

    float threshold[vertices_count-2];
    threshold[0] = 0.00001f;//for the 3D margin
    threshold[1] = 0.125f;
    threshold[2] = 0.25f;
    threshold[3] = 0.375f;
    threshold[4] = 0.50f;
    threshold[5] = 0.625f;
    threshold[6] = 0.750f;
    threshold[7] = 0.875f;
    threshold[8] = 0.99999f;//for the 3D margin
    threshold[9] = 1.0f;

    //3D effect : wait for the full border to appear before drawing
    for (int i=0;i<8;i++)
    {
        if ((speed_ratio-0.125f*i < 0.00625f && speed_ratio-0.125f*i >= 0.0f) || (0.125f*i-speed_ratio < 0.0045f && 0.125f*i-speed_ratio >= 0.0f) )
        {
            speed_ratio = 0.125f*i-0.0045f;
            break;
        }
    }

    unsigned int count = computeVerticesForMeter(position, threshold, vertices, vertices_count, 
                                                     speed_ratio, meter_width, meter_height, offset);


    drawMeterTexture(m_speed_bar_icon->getTexture(), vertices, count);
#endif
}   // drawSpeedEnergyRank

void RaceGUI::drawMeterTexture(video::ITexture *meter_texture, video::S3DVertex vertices[], unsigned int count)
{
#ifndef SERVER_ONLY
    //Should be greater or equal than the greatest vertices_count used by the meter functions
    if (count < 2)
        return;
    short int index[12];
    for(unsigned int i=0; i<count; i++)
    {
        index[i]=i;
        vertices[i].Color = video::SColor(255, 255, 255, 255);
    }

    video::SMaterial m;
    m.setTexture(0, meter_texture);
    m.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
    irr_driver->getVideoDriver()->setMaterial(m);
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    draw2DVertexPrimitiveList(m.getTexture(0), vertices, count,
        index, count-2, video::EVT_STANDARD, scene::EPT_TRIANGLE_FAN);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);

#endif
}   // drawMeterTexture



//-----------------------------------------------------------------------------
/** This function computes a polygon used for drawing the measure for a meter (speedometer, etc.)
 *  The variable measured by the meter is compared to the thresholds, and is then used to
 *  compute a point between the two points associated with the lower and upper threshold
 *  Then, a polygon is calculated linking all the previous points and the variable point
 *  which link back to the first point. This polygon is used for drawing.
 *
 *  Consider the following example :
 *
 *      A                E
 *                      -|
 *                      x
 *                      |
 *                   -D-|
 *                -w-|
 *           |-C--|
 *     -B--v-|
 *
 *  If the measure is inferior to the first threshold, the function will create a triangle ABv
 *  with the position of v varying proportionally on a line between B and C ;
 *  at B with 0 and at C when it reaches the first threshold.
 *  If the measure is between the first and second thresholds, the function will create a quad ABCw,
 *  with w varying in the same way than v.
 *  If the measure exceds the higher threshold, the function will return the poly ABCDE.
 *  
 *  \param position The relative positions of the vertices.
 *  \param threshold The thresholds at which the variable point switch from a segment to the next.
 *                   The size of this array should be smaller by two than the position array.
 *                   The last threshold determines the measure over which the meter is full
 *  \param vertices Where the results of the computation are put, for use by the calling function.
 *  \param vertices_count The maximum number of vertices to use. Should be superior or equal to the
 *                       size of the arrays.
 *  \param measure The value of the variable measured by the meter.
 *  \param gauge_width The width of the meter
 *  \param gauge_height The height of the meter
 *  \param offset The offset to position the meter
 */
unsigned int RaceGUI::computeVerticesForMeter(core::vector2df position[], float threshold[], video::S3DVertex vertices[], unsigned int vertices_count,
                                     float measure, int gauge_width, int gauge_height, core::vector2df offset)
{
    //Nothing to draw ; we need at least three points to draw a triangle
    if (vertices_count <= 2 || measure < 0)
    {
        return 0;
    }

    unsigned int count=2;
    float f = 1.0f;

    for (unsigned int i=2 ; i < vertices_count ; i++)
    {
        count++;

        //Stop when we have found between which thresholds the measure is
        if (measure < threshold[i-2])
        {
            if (i-2 == 0)
            {
                f = measure/threshold[i-2];
            }
            else
            {
                f = (measure - threshold[i-3])/(threshold[i-2]-threshold[i-3]);
            }

            break;
        }
    }

    for (unsigned int i=0 ; i < count ; i++)
    {
        //if the measure don't fall in this segment, use the next predefined point
        if (i<count-1 || (count == vertices_count && f == 1.0f))
        {
            vertices[i].TCoords = core::vector2df(position[i].X, position[i].Y);
            vertices[i].Pos     = core::vector3df(offset.X+position[i].X*gauge_width,
                                  offset.Y-(1-position[i].Y)*gauge_height, 0);
        }
        //if the measure fall in this segment, compute the variable position
        else
        {
            //f : the proportion of the next point. 1-f : the proportion of the previous point
            vertices[i].TCoords = core::vector2df(position[i].X*(f)+position[i-1].X*(1.0f-f),
                                                  position[i].Y*(f)+position[i-1].Y*(1.0f-f));
            vertices[i].Pos = core::vector3df(offset.X+ ((position[i].X*(f)+position[i-1].X*(1.0f-f))*gauge_width),
                                              offset.Y-(((1-position[i].Y)*(f)+(1-position[i-1].Y)*(1.0f-f))*gauge_height),0);
        }
    }

    //the count is used in the drawing functions
    return count;
} //computeVerticesForMeter

//-----------------------------------------------------------------------------
/** Displays the lap of the kart.
 *  \param info Info object c
*/
void RaceGUI::drawLap(const AbstractKart* kart,
                      const core::recti &viewport,
                      const core::vector2df &scaling)
{
#ifndef SERVER_ONLY
    // Don't display laps if the kart has already finished the race.
    if (kart->hasFinishedRace()) return;

    World *world = World::getWorld();

    core::recti pos;
    
    pos.UpperLeftCorner.Y = viewport.UpperLeftCorner.Y + m_font_height;

    // If the time display in the top right is in this viewport,
    // move the lap/rank display down a little bit so that it is
    // displayed under the time.
    if (viewport.UpperLeftCorner.Y == 0 &&
        viewport.LowerRightCorner.X == (int)(irr_driver->getActualScreenSize().Width) &&
        !RaceManager::get()->getIfEmptyScreenSpaceExists()) 
    {
        pos.UpperLeftCorner.Y = irr_driver->getActualScreenSize().Height*12/100;
    }
    pos.LowerRightCorner.Y  = viewport.LowerRightCorner.Y+20;
    pos.UpperLeftCorner.X   = viewport.LowerRightCorner.X
                            - m_lap_width - 10;
    pos.LowerRightCorner.X  = viewport.LowerRightCorner.X;

    // Draw CTF / soccer scores with red score - blue score (score limit)
    CaptureTheFlag* ctf = dynamic_cast<CaptureTheFlag*>(World::getWorld());
    SoccerWorld* sw = dynamic_cast<SoccerWorld*>(World::getWorld());
    FreeForAll* ffa = dynamic_cast<FreeForAll*>(World::getWorld());

    static video::SColor color = video::SColor(255, 255, 255, 255);
    int hit_capture_limit =
        (RaceManager::get()->getHitCaptureLimit() != std::numeric_limits<int>::max()
         && RaceManager::get()->getHitCaptureLimit() != 0)
        ? RaceManager::get()->getHitCaptureLimit() : -1;
    int score_limit = sw && !RaceManager::get()->hasTimeTarget() ?
        RaceManager::get()->getMaxGoal() : ctf ? hit_capture_limit : -1;
    if (!ctf && ffa && hit_capture_limit != -1)
    {
        int icon_width = irr_driver->getActualScreenSize().Height/19;
        core::rect<s32> indicator_pos(viewport.LowerRightCorner.X - (icon_width+10),
                                    pos.UpperLeftCorner.Y,
                                    viewport.LowerRightCorner.X - 10,
                                    pos.UpperLeftCorner.Y + icon_width);
        core::rect<s32> source_rect(core::position2d<s32>(0,0),
                                                m_champion->getSize());
        draw2DImage(m_champion, indicator_pos, source_rect,
            NULL, NULL, true);

        gui::ScalableFont* font = GUIEngine::getHighresDigitFont();
        font->setBlackBorder(true);
        pos.UpperLeftCorner.X += 30;
        font->draw(StringUtils::toWString(hit_capture_limit).c_str(), pos, color);
        font->setBlackBorder(false);
        font->setScale(1.0f);
        return;
    }

    if (ctf || sw)
    {
        int red_score = ctf ? ctf->getRedScore() : sw->getScore(KART_TEAM_RED);
        int blue_score = ctf ? ctf->getBlueScore() : sw->getScore(KART_TEAM_BLUE);
        gui::ScalableFont* font = GUIEngine::getHighresDigitFont();
        font->setBlackBorder(true);
        font->setScale(1.0f);
        core::dimension2du d;
        if (score_limit != -1)
        {
             d = font->getDimension(
                (StringUtils::toWString(red_score) + L"-"
                + StringUtils::toWString(blue_score) + L"     "
                + StringUtils::toWString(score_limit)).c_str());
            pos.UpperLeftCorner.X -= d.Width / 2;
            int icon_width = irr_driver->getActualScreenSize().Height/19;
            core::rect<s32> indicator_pos(viewport.LowerRightCorner.X - (icon_width+10),
                                        pos.UpperLeftCorner.Y,
                                        viewport.LowerRightCorner.X - 10,
                                        pos.UpperLeftCorner.Y + icon_width);
            core::rect<s32> source_rect(core::position2d<s32>(0,0),
                                                    m_champion->getSize());
            draw2DImage(m_champion, indicator_pos, source_rect,
                NULL, NULL, true);
        }

        core::stringw text = StringUtils::toWString(red_score);
        font->draw(text, pos, video::SColor(255, 255, 0, 0));
        d = font->getDimension(text.c_str());
        pos += core::position2di(d.Width, 0);
        text = L"-";
        font->draw(text, pos, video::SColor(255, 255, 255, 255));
        d = font->getDimension(text.c_str());
        pos += core::position2di(d.Width, 0);
        text = StringUtils::toWString(blue_score);
        font->draw(text, pos, video::SColor(255, 0, 0, 255));
        pos += core::position2di(d.Width, 0);
        if (score_limit != -1)
        {
            text = L"     ";
            text += StringUtils::toWString(score_limit);
            font->draw(text, pos, video::SColor(255, 255, 255, 255));
        }
        font->setBlackBorder(false);
        return;
    }

    if (!world->raceHasLaps()) return;
    int lap = world->getFinishedLapsOfKart(kart->getWorldKartId());
    // Network race has larger lap than getNumLaps near finish line
    // due to waiting for final race result from server
    if (lap + 1> RaceManager::get()->getNumLaps())
        lap--;
    // don't display 'lap 0/..' at the start of a race
    if (lap < 0 ) return;

    // Display lap flag


    int icon_width = irr_driver->getActualScreenSize().Height/19;
    core::rect<s32> indicator_pos(viewport.LowerRightCorner.X - (icon_width+10),
                                  pos.UpperLeftCorner.Y,
                                  viewport.LowerRightCorner.X - 10,
                                  pos.UpperLeftCorner.Y + icon_width);
    core::rect<s32> source_rect(core::position2d<s32>(0,0),
                                               m_lap_flag->getSize());
    draw2DImage(m_lap_flag,indicator_pos,source_rect,
        NULL,NULL,true);

    pos.UpperLeftCorner.X -= icon_width;
    pos.LowerRightCorner.X -= icon_width;

    std::ostringstream out;
    out << lap + 1 << "/" << RaceManager::get()->getNumLaps();

    gui::ScalableFont* font = GUIEngine::getHighresDigitFont();
    font->setBlackBorder(true);
    font->draw(out.str().c_str(), pos, color);
    font->setBlackBorder(false);
    font->setScale(1.0f);
#endif
} // drawLap

