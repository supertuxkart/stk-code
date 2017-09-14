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
#include "items/attachment.hpp"
#include "items/attachment_manager.hpp"
#include "items/powerup_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "karts/controller/spare_tire_ai.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/follow_the_leader.hpp"
#include "modes/linear_world.hpp"
#include "modes/world.hpp"
#include "modes/soccer_world.hpp"
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
    core::dimension2du area = font->getDimension(L"99:99:99");
    m_timer_width = area.Width;
    m_font_height = area.Height;

    if (race_manager->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER ||
        race_manager->getMinorMode()==RaceManager::MINOR_MODE_3_STRIKES     ||
        race_manager->getNumLaps() > 9)
        m_lap_width = font->getDimension(L"99/99").Width;
    else
        m_lap_width = font->getDimension(L"9/9").Width;

    // Originally m_map_height was 100, and we take 480 as minimum res
    float scaling = irr_driver->getFrameSize().Height / 480.0f;
    const float map_size = 100.0f;
    const float top_margin = 3.5f * m_font_height;
    
    if (UserConfigParams::m_multitouch_enabled && 
        UserConfigParams::m_multitouch_mode != 0 &&
        race_manager->getNumLocalPlayers() == 1)
    {
        m_multitouch_gui = new RaceGUIMultitouch(this);
    }
    
    // Check if we have enough space for minimap when touch steering is enabled
    if (m_multitouch_gui != NULL)
    {
        const float map_bottom = (float)(m_multitouch_gui->getMinimapBottom());
        
        if ((map_size + 20.0f) * scaling > map_bottom - top_margin)
        {
            scaling = (map_bottom - top_margin) / (map_size + 20.0f);
        }
    }
    
    // Marker texture has to be power-of-two for (old) OpenGL compliance
    //m_marker_rendered_size  =  2 << ((int) ceil(1.0 + log(32.0 * scaling)));
    m_minimap_ai_size       = (int)( 14.0f * scaling);
    m_minimap_player_size   = (int)( 16.0f * scaling);
    m_map_width             = (int)(map_size * scaling);
    m_map_height            = (int)(map_size * scaling);
    m_map_left              = (int)( 10.0f * scaling);
    m_map_bottom            = (int)( 10.0f * scaling);

    // Minimap is also rendered bigger via OpenGL, so find power-of-two again
    const int map_texture   = 2 << ((int) ceil(1.0 + log(128.0 * scaling)));
    m_map_rendered_width    = map_texture;
    m_map_rendered_height   = map_texture;


    // special case : when 3 players play, use available 4th space for such things
    if (race_manager->getNumLocalPlayers() == 3)
    {
        m_map_left = irr_driver->getActualScreenSize().Width - m_map_width;
    }
    else if (m_multitouch_gui != NULL)
    {
        m_map_left = (int)((irr_driver->getActualScreenSize().Width - 
                                                        m_map_width) * 0.95f);
        m_map_bottom = (int)(irr_driver->getActualScreenSize().Height - 
                                                    top_margin - m_map_height);
    }

    m_is_tutorial = (race_manager->getTrackName() == "tutorial");

    // Load speedmeter texture before rendering the first frame
    m_speed_meter_icon = material_manager->getMaterial("speedback.png");
    m_speed_meter_icon->getTexture();
    m_speed_bar_icon   = material_manager->getMaterial("speedfore.png");
    m_speed_bar_icon->getTexture();
    //createMarkerTexture();
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
    int n = race_manager->getNumberOfKarts();

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
    for(unsigned int i=0; i<race_manager->getNumberOfKarts(); i++)
    {
        m_animation_states[i] = AS_NONE;
        m_last_ranks[i]       = i+1;
    }
}  // reset

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
    if (race_manager->getNumLocalPlayers() == 3 &&
        !GUIEngine::ModalDialog::isADialogActive())
    {
        static video::SColor black = video::SColor(255,0,0,0);
        GL32_draw2DRectangle(black,
                              core::rect<s32>(irr_driver->getActualScreenSize().Width/2,
                                              irr_driver->getActualScreenSize().Height/2,
                                              irr_driver->getActualScreenSize().Width,
                                              irr_driver->getActualScreenSize().Height));
    }

    World *world = World::getWorld();
    assert(world != NULL);
    if(world->getPhase() >= WorldStatus::READY_PHASE &&
       world->getPhase() <= WorldStatus::GO_PHASE      )
    {
        drawGlobalReadySetGo();
    }
    if(world->getPhase() == World::GOAL_PHASE)
            drawGlobalGoal();

    // Timer etc. are not displayed unless the game is actually started.
    if(!world->isRacePhase()) return;
    if (!m_enabled) return;


    if (!m_is_tutorial)
    {
        //stop displaying timer as soon as race is over
        if (world->getPhase()<WorldStatus::DELAY_FINISH_PHASE)
           drawGlobalTimer();

        if(world->getPhase() == WorldStatus::GO_PHASE ||
           world->getPhase() == WorldStatus::MUSIC_PHASE)
        {
            drawGlobalMusicDescription();
        }
    }

    drawGlobalMiniMap();

    if (!m_is_tutorial)               drawGlobalPlayerIcons(m_map_height);
    if(Track::getCurrentTrack()->isSoccer()) drawScores();
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

    scaling *= viewport.getWidth()/800.0f; // scale race GUI along screen size
    drawAllMessages(kart, viewport, scaling);

    if(!World::getWorld()->isRacePhase()) return;

    if (m_multitouch_gui == NULL)
    {
        drawPowerupIcons(kart, viewport, scaling);
        drawSpeedEnergyRank(kart, viewport, scaling, dt);
    }

    if (!m_is_tutorial)
        drawLap(kart, viewport, scaling);
}   // renderPlayerView

//-----------------------------------------------------------------------------
/** Shows the current soccer result.
 */
void RaceGUI::drawScores()
{
#ifndef SERVER_ONLY
    SoccerWorld* sw = dynamic_cast<SoccerWorld*>(World::getWorld());
    int offset_y = 5;
    int offset_x = 5;
    gui::ScalableFont* font = GUIEngine::getTitleFont();
    static video::SColor color = video::SColor(255,255,255,255);

    //Draw two teams score
    irr::video::ITexture *red_team = irr_driver->getTexture(FileManager::GUI,
                                                            "soccer_ball_red.png");
    irr::video::ITexture *blue_team = irr_driver->getTexture(FileManager::GUI,
                                                            "soccer_ball_blue.png");
    irr::video::ITexture *team_icon = red_team;

    for(unsigned int i=0; i<2; i++)
    {
        core::recti position(offset_x, offset_y,
            offset_x + 2*m_minimap_player_size, offset_y + 2*m_minimap_player_size);

        core::stringw score = StringUtils::toWString(sw->getScore((SoccerTeam)i));
        int string_height =
            GUIEngine::getFont()->getDimension(score.c_str()).Height;
        core::recti pos(position.UpperLeftCorner.X + 5,
                        position.LowerRightCorner.Y + offset_y,
                        position.LowerRightCorner.X,
                        position.LowerRightCorner.Y + string_height);

        font->draw(score.c_str(),pos,color);

        if (i == 1)
        {
            team_icon = blue_team;
        }
        core::rect<s32> indicator_pos(offset_x, offset_y,
                                     offset_x + (int)(m_minimap_player_size*2),
                                     offset_y + (int)(m_minimap_player_size*2));
        core::rect<s32> source_rect(core::position2d<s32>(0,0),
                                                   team_icon->getSize());
        draw2DImage(team_icon,indicator_pos,source_rect,
            NULL,NULL,true);
        offset_x += position.LowerRightCorner.X + 30;
    }
#endif
}   // drawScores

//-----------------------------------------------------------------------------
/** Displays the racing time on the screen.s
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
    if (!race_manager->hasTimeTarget() || race_manager
        ->getMinorMode()==RaceManager::MINOR_MODE_SOCCER)
    {
        sw = core::stringw (
            StringUtils::timeToString(elapsed_time).c_str() );
    }
    else
    {
        float time_target = race_manager->getTimeTarget();
        if (elapsed_time < time_target)
        {
            sw = core::stringw (
              StringUtils::timeToString(time_target - elapsed_time).c_str());
        }
        else
        {
            sw = _("Challenge Failed");
            int string_width =
                GUIEngine::getFont()->getDimension(_("Challenge Failed")).Width;
            dist_from_right = 10 + string_width;
            time_color = video::SColor(255,255,0,0);
            use_digit_font = false;
        }
    }

    core::rect<s32> pos(irr_driver->getActualScreenSize().Width - dist_from_right, 30,
                        irr_driver->getActualScreenSize().Width                  , 50);

    // special case : when 3 players play, use available 4th space for such things
    if (race_manager->getNumLocalPlayers() == 3)
    {
        pos += core::vector2d<s32>(0, irr_driver->getActualScreenSize().Height/2);
    }

    gui::ScalableFont* font = (use_digit_font ? GUIEngine::getHighresDigitFont() : GUIEngine::getFont());
    if (use_digit_font)
        font->setShadow(video::SColor(255, 128, 0, 0));
    font->setScale(1.0f);
    font->draw(sw.c_str(), pos, time_color, false, false, NULL,
               true /* ignore RTL */);

}   // drawGlobalTimer

//-----------------------------------------------------------------------------
/** Draws the mini map and the position of all karts on it.
 */
void RaceGUI::drawGlobalMiniMap()
{
#ifndef SERVER_ONLY
    // draw a map when arena has a navigation mesh.
    Track *track = Track::getCurrentTrack();
    if ( (track->isArena() || track->isSoccer()) && !(track->hasNavMesh()) )
        return;

    int upper_y = irr_driver->getActualScreenSize().Height - m_map_bottom - m_map_height;
    int lower_y = irr_driver->getActualScreenSize().Height - m_map_bottom;

    core::rect<s32> dest(m_map_left, upper_y,
                         m_map_left + m_map_width, lower_y);

    track->drawMiniMap(dest);

    World *world = World::getWorld();
    for(unsigned int i=0; i<world->getNumKarts(); i++)
    {
        const AbstractKart *kart = world->getKart(i);
        const SpareTireAI* sta =
            dynamic_cast<const SpareTireAI*>(kart->getController());
        // don't draw eliminated kart
        if(kart->isEliminated() && !(sta && sta->isMoving())) continue;
        const Vec3& xyz = kart->getXYZ();
        Vec3 draw_at;
        track->mapPoint2MiniMap(xyz, &draw_at);
        
        video::ITexture* icon = sta ?
            irr_driver->getTexture(FileManager::GUI, "heart.png") :
            kart->getKartProperties()->getMinimapIcon();

        // int marker_height = m_marker->getSize().Height;
        core::rect<s32> source(core::position2di(0, 0), icon->getSize());
        int marker_half_size = (kart->getController()->isLocalPlayerController()
                                ? m_minimap_player_size
                                : m_minimap_ai_size                        )>>1;
        core::rect<s32> position(m_map_left+(int)(draw_at.getX()-marker_half_size),
                                 lower_y   -(int)(draw_at.getY()+marker_half_size),
                                 m_map_left+(int)(draw_at.getX()+marker_half_size),
                                 lower_y   -(int)(draw_at.getY()-marker_half_size));
        draw2DImage(icon, position, source, NULL, NULL, true);
    }   // for i<getNumKarts

    SoccerWorld *sw = dynamic_cast<SoccerWorld*>(World::getWorld());
    if (sw)
    {
        Vec3 draw_at;
        track->mapPoint2MiniMap(sw->getBallPosition(), &draw_at);
        
        video::ITexture* icon =
            irr_driver->getTexture(FileManager::GUI, "soccer_ball_normal.png");

        core::rect<s32> source(core::position2di(0, 0), icon->getSize());
        core::rect<s32> position(m_map_left+(int)(draw_at.getX()-(m_minimap_player_size/2.5f)),
                                 lower_y   -(int)(draw_at.getY()+(m_minimap_player_size/2.5f)),
                                 m_map_left+(int)(draw_at.getX()+(m_minimap_player_size/2.5f)),
                                 lower_y   -(int)(draw_at.getY()-(m_minimap_player_size/2.5f)));
        draw2DImage(icon, position, source, NULL, NULL, true);
    }
#endif
}   // drawGlobalMiniMap

//-----------------------------------------------------------------------------
/** Energy meter that gets filled with nitro. This function is called from
 *  drawSpeedAndEnergy, which defines the correct position of the energy
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
    const int GAUGEWIDTH   = 78;
    int gauge_width        = (int)(GAUGEWIDTH*min_ratio);
    int gauge_height       = (int)(GAUGEWIDTH*min_ratio);

    float state = (float)(kart->getEnergy())
                / kart->getKartProperties()->getNitroMax();
    if (state < 0.0f) state = 0.0f;
    else if (state > 1.0f) state = 1.0f;

    core::vector2df offset;
    offset.X = (float)(x-gauge_width) - 9.0f*scaling.X;
    offset.Y = (float)y-30.0f*scaling.Y;


    // Background
    draw2DImage(m_gauge_empty, core::rect<s32>((int)offset.X,
                                               (int)offset.Y-gauge_height,
                                               (int)offset.X + gauge_width,
                                               (int)offset.Y) /* dest rect */,
                core::rect<s32>(core::position2d<s32>(0,0),
                                m_gauge_empty->getSize()) /* source rect */,
                NULL /* clip rect */, NULL /* colors */,
                true /* alpha */);

    // Target

    if (race_manager->getCoinTarget() > 0)
    {
        float coin_target = (float)race_manager->getCoinTarget()
                          / kart->getKartProperties()->getNitroMax();

        video::S3DVertex vertices[5];
        unsigned int count=2;

        // There are three different polygons used, depending on
        // the target. Consider the nitro-display texture:
        //
        //   ----E-x--D       (position of v,w,x vary depending on
        //            |        nitro)
        //       A    w
        //            |
        //   -B--v----C
        // For nitro state <= r1 the triangle ABv is used, with v between B and C.
        // For nitro state <= r2 the quad ABCw is used, with w between C and D.
        // For nitro state >  r2 the poly ABCDx is used, with x between D and E.

        vertices[0].TCoords = core::vector2df(0.3f, 0.4f);
        vertices[0].Pos     = core::vector3df(offset.X+0.3f*gauge_width,
                                              offset.Y-(1-0.4f)*gauge_height, 0);
        vertices[1].TCoords = core::vector2df(0, 1.0f);
        vertices[1].Pos     = core::vector3df(offset.X, offset.Y, 0);
        // The targets at which different polygons must be used.

        const float r1 = 0.4f;
        const float r2 = 0.65f;
        if(coin_target<=r1)
        {
            count   = 3;
            float f = coin_target/r1;
            vertices[2].TCoords = core::vector2df(0.08f + (1.0f-0.08f)*f, 1.0f);
            vertices[2].Pos =  core::vector3df(offset.X + (0.08f*gauge_width)
                                                + (1.0f - 0.08f)*f*gauge_width,
                                               offset.Y,0);
                }
        else if(coin_target<=r2)
        {
            count   = 4;
            float f = (coin_target - r1)/(r2-r1);
            vertices[2].TCoords = core::vector2df(1.0f, 1.0f);
            vertices[2].Pos     = core::vector3df(offset.X + gauge_width,
                                                  offset.Y, 0);
            vertices[3].TCoords = core::vector2df(1.0f, (1.0f-f));
            vertices[3].Pos = core::vector3df(offset.X + gauge_width,
                                              offset.Y-f*gauge_height,0);
        }
        else
        {
            count   = 5;
            float f = (coin_target - r2)/(1-r2);
            vertices[2].TCoords = core::vector2df(1.0, 1.0f);
            vertices[2].Pos     = core::vector3df(offset.X + gauge_width,
                                                  offset.Y, 0);
            vertices[3].TCoords = core::vector2df(1.0,0);
            vertices[3].Pos     = core::vector3df(offset.X + gauge_width,
                                                  offset.Y-gauge_height, 0);
            vertices[4].TCoords = core::vector2df(1.0f - f*(1-0.61f), 0);
            vertices[4].Pos     = core::vector3df(offset.X + gauge_width
                                                   - (1.0f-0.61f)*f*gauge_width,
                                                   offset.Y-gauge_height, 0);
        }
        short int index[5]={0};
        for(unsigned int i=0; i<count; i++)
        {
            index[i]=count-i-1;
            vertices[i].Color = video::SColor(255, 255, 255, 255);
        }

        video::SMaterial m;
        m.setTexture(0, m_gauge_goal);
        m.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
        irr_driver->getVideoDriver()->setMaterial(m);
        draw2DVertexPrimitiveList(m_gauge_goal, vertices, count,
        index, count-2, video::EVT_STANDARD, scene::EPT_TRIANGLE_FAN);

    }



    // Filling (current state)

    if(state <=0) return; //Nothing to do

    if (state > 0.0f)
    {
        video::S3DVertex vertices[5];
        unsigned int count=2;

        // There are three different polygons used, depending on
        // the nitro state. Consider the nitro-display texture:
        //
        //   ----E-x--D       (position of v,w,x vary depending on
        //            |        nitro)
        //       A    w
        //            |
        //   -B--v----C
        // For nitro state <= r1 the triangle ABv is used, with v between B and C.
        // For nitro state <= r2 the quad ABCw is used, with w between C and D.
        // For nitro state >  r2 the poly ABCDx is used, with x between D and E.

        vertices[0].TCoords = core::vector2df(0.3f, 0.4f);
        vertices[0].Pos     = core::vector3df(offset.X+0.3f*gauge_width,
                                              offset.Y-(1-0.4f)*gauge_height, 0);
        vertices[1].TCoords = core::vector2df(0, 1.0f);
        vertices[1].Pos     = core::vector3df(offset.X, offset.Y, 0);
        // The states at which different polygons must be used.

        const float r1 = 0.4f;
        const float r2 = 0.65f;
        if(state<=r1)
        {
            count   = 3;
            float f = state/r1;
            vertices[2].TCoords = core::vector2df(0.08f + (1.0f-0.08f)*f, 1.0f);
            vertices[2].Pos = core::vector3df(offset.X + (0.08f*gauge_width)
                                                       + (1.0f - 0.08f)
                                                         *f*gauge_width,
                                              offset.Y,0);
                }
        else if(state<=r2)
        {
            count   = 4;
            float f = (state - r1)/(r2-r1);
            vertices[2].TCoords = core::vector2df(1.0f, 1.0f);
            vertices[2].Pos     = core::vector3df(offset.X + gauge_width,
                                                  offset.Y, 0);
            vertices[3].TCoords = core::vector2df(1.0f, (1.0f-f));
            vertices[3].Pos = core::vector3df(offset.X + gauge_width,
                                              offset.Y-f*gauge_height,0);
        }
        else
        {
            count   = 5;
            float f = (state - r2)/(1-r2);
            vertices[2].TCoords = core::vector2df(1.0, 1.0f);
            vertices[2].Pos     = core::vector3df(offset.X + gauge_width,
                                                  offset.Y, 0);
            vertices[3].TCoords = core::vector2df(1.0,0);
            vertices[3].Pos = core::vector3df(offset.X + gauge_width,
                                              offset.Y-gauge_height, 0);
            vertices[4].TCoords = core::vector2df(1.0f - f*(1-0.61f), 0);
            vertices[4].Pos     =
                core::vector3df(offset.X + gauge_width - (1.0f-0.61f)*f*gauge_width,
                                offset.Y-gauge_height, 0);
        }
        short int index[5]={0};
        for(unsigned int i=0; i<count; i++)
        {
            index[i]=count-i-1;
            vertices[i].Color = video::SColor(255, 255, 255, 255);
        }


        video::SMaterial m;
        if(kart->getControls().getNitro() || kart->isOnMinNitroTime())
            m.setTexture(0, m_gauge_full_bright);
        else
            m.setTexture(0, m_gauge_full);
        m.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
        irr_driver->getVideoDriver()->setMaterial(m);
        draw2DVertexPrimitiveList(m.getTexture(0), vertices, count,
        index, count-2, video::EVT_STANDARD, scene::EPT_TRIANGLE_FAN);

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
    font->setScale(min_ratio * scale);
    font->setShadow(video::SColor(255, 128, 0, 0));
    std::ostringstream oss;
    oss << rank; // the current font has no . :(   << ".";

    core::recti pos;
    pos.LowerRightCorner = core::vector2di(int(offset.X + 0.65f*meter_width),
                                           int(offset.Y - 0.55f*meter_height));
    pos.UpperLeftCorner = core::vector2di(int(offset.X + 0.65f*meter_width),
                                          int(offset.Y - 0.55f*meter_height));

    static video::SColor color = video::SColor(255, 255, 255, 255);
    font->draw(oss.str().c_str(), pos, color, true, true);
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
    float speed_ratio = speed/KILOMETERS_PER_HOUR/110.0f;
    if(speed_ratio>1) speed_ratio = 1;

    video::S3DVertex vertices[5];
    unsigned int count;

    // There are three different polygons used, depending on
    // the speed ratio. Consider the speed-display texture:
    //
    //   D----x----D       (position of v,w,x vary depending on
    //   |                 speed)
    //   w    A
    //   |
    //   C--v-B----E
    // For speed ratio <= r1 the triangle ABv is used, with v between B and C.
    // For speed ratio <= r2 the quad ABCw is used, with w between C and D.
    // For speed ratio >  r2 the poly ABCDx is used, with x between D and E.

    vertices[0].TCoords = core::vector2df(0.7f, 0.5f);
    vertices[0].Pos     = core::vector3df(offset.X+0.7f*meter_width,
                                          offset.Y-0.5f*meter_height, 0);
    vertices[1].TCoords = core::vector2df(0.52f, 1.0f);
    vertices[1].Pos     = core::vector3df(offset.X+0.52f*meter_width, offset.Y, 0);
    // The speed ratios at which different triangles must be used.
    // These values should be adjusted in case that the speed display
    // is not linear enough. Mostly the speed values are below 0.7, it
    // needs some zipper to get closer to 1.
    const float r1 = 0.2f;
    const float r2 = 0.6f;
    if(speed_ratio<=r1)
    {
        count   = 3;
        float f = speed_ratio/r1;
        vertices[2].TCoords = core::vector2df(0.52f*(1-f), 1.0f);
        vertices[2].Pos = core::vector3df(offset.X+ (0.52f*(1.0f-f)*meter_width),
                                          offset.Y,0);
    }
    else if(speed_ratio<=r2)
    {
        count   = 4;
        float f = (speed_ratio - r1)/(r2-r1);
        vertices[2].TCoords = core::vector2df(0, 1.0f);
        vertices[2].Pos     = core::vector3df(offset.X, offset.Y, 0);
        vertices[3].TCoords = core::vector2df(0, (1-f));
        vertices[3].Pos = core::vector3df(offset.X, offset.Y-f*meter_height,0);
    }
    else
    {
        count   = 5;
        float f = (speed_ratio - r2)/(1-r2);
        vertices[2].TCoords = core::vector2df(0, 1.0f);
        vertices[2].Pos     = core::vector3df(offset.X, offset.Y, 0);
        vertices[3].TCoords = core::vector2df(0,0);
        vertices[3].Pos = core::vector3df(offset.X, offset.Y-meter_height, 0);
        vertices[4].TCoords = core::vector2df(f, 0);
        vertices[4].Pos     = core::vector3df(offset.X+f*meter_width,
                                              offset.Y-meter_height, 0);
    }
    short int index[5];
    for(unsigned int i=0; i<count; i++)
    {
        index[i]=i;
        vertices[i].Color = video::SColor(255, 255, 255, 255);
    }
    video::SMaterial m;
    m.setTexture(0, m_speed_bar_icon->getTexture());
    m.MaterialType = video::EMT_TRANSPARENT_ALPHA_CHANNEL;
    irr_driver->getVideoDriver()->setMaterial(m);
    draw2DVertexPrimitiveList(m_speed_bar_icon->getTexture(), vertices, count,
        index, count-2, video::EVT_STANDARD, scene::EPT_TRIANGLE_FAN);
#endif
}   // drawSpeedEnergyRank

//-----------------------------------------------------------------------------
/** Displays the rank and the lap of the kart.
 *  \param info Info object c
*/
void RaceGUI::drawLap(const AbstractKart* kart,
                      const core::recti &viewport,
                      const core::vector2df &scaling)
{
    // Don't display laps or ranks if the kart has already finished the race.
    if (kart->hasFinishedRace()) return;

    World *world = World::getWorld();
    if (!world->raceHasLaps()) return;
    const int lap = world->getKartLaps(kart->getWorldKartId());

    // don't display 'lap 0/..' at the start of a race
    if (lap < 0 ) return;

    core::recti pos;
    pos.UpperLeftCorner.Y   = viewport.UpperLeftCorner.Y + m_font_height;
    // If the time display in the top right is in this viewport,
    // move the lap/rank display down a little bit so that it is
    // displayed under the time.
    if (viewport.UpperLeftCorner.Y==0 &&
        viewport.LowerRightCorner.X==(int)(irr_driver->getActualScreenSize().Width) &&
        race_manager->getNumPlayers()!=3)
        pos.UpperLeftCorner.Y   += m_font_height;
    pos.LowerRightCorner.Y  = viewport.LowerRightCorner.Y+20;
    pos.UpperLeftCorner.X   = viewport.LowerRightCorner.X
                            - m_lap_width - 10;
    pos.LowerRightCorner.X  = viewport.LowerRightCorner.X;

    static video::SColor color = video::SColor(255, 255, 255, 255);
    std::ostringstream out;
    out << lap + 1 << "/" << race_manager->getNumLaps();

    gui::ScalableFont* font = GUIEngine::getHighresDigitFont();
    font->setScale(scaling.Y < 1.0f ? 0.5f: 1.0f);
    font->draw(out.str().c_str(), pos, color);
    font->setScale(1.0f);

} // drawLap
