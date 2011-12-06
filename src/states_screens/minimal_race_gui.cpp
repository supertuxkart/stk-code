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

#include "states_screens/minimal_race_gui.hpp"

using namespace irr;

#include "config/user_config.hpp"
#include "graphics/camera.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/scalable_font.hpp"
#include "io/file_manager.hpp"
#include "input/input.hpp"
#include "input/input_manager.hpp"
#include "items/attachment.hpp"
#include "items/attachment_manager.hpp"
#include "items/powerup_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/follow_the_leader.hpp"
#include "modes/linear_world.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

/** The constructor is called before anything is attached to the scene node.
 *  So rendering to a texture can be done here. But world is not yet fully
 *  created, so only the race manager can be accessed safely.
 */
MinimalRaceGUI::MinimalRaceGUI()
{    
    m_enabled = true;
    
    // Ignore item messages. 
    ignoreUnimportantMessages();

    // Originally m_map_height was 100, and we take 480 as minimum res
    const float scaling = irr_driver->getFrameSize().Height / 480.0f;
    // Marker texture has to be power-of-two for (old) OpenGL compliance
    m_marker_rendered_size  =  2 << ((int) ceil(1.0 + log(32.0 * scaling)));
    m_marker_ai_size        = (int)( 24.0f * scaling);
    m_marker_player_size    = (int)( 34.0f * scaling);
    m_map_width             = (int)(200.0f * scaling);
    m_map_height            = (int)(200.0f * scaling);

    // The location of the minimap varies with number of 
    // splitscreen players:
    switch(race_manager->getNumLocalPlayers())
    {
    case 0 : // In case of profile mode
    case 1 : // Lower left corner
             m_map_left   = 10;
             m_map_bottom = UserConfigParams::m_height-10;
             break;
    case 2:  // Middle of left side
             m_map_left   = 10;
             m_map_bottom = UserConfigParams::m_height/2 + m_map_height/2;
             break;
    case 3:  // Lower right quarter (which is not used by a player)
             m_map_left   = UserConfigParams::m_width/2 + 10;
             m_map_bottom = UserConfigParams::m_height-10;
             break;
    case 4:  // Middle of the screen.
             m_map_left   = UserConfigParams::m_width/2-m_map_width/2;
             m_map_bottom = UserConfigParams::m_height/2 + m_map_height/2;
             break;
    }
    
    // Minimap is also rendered bigger via OpenGL, so find power-of-two again
    const int map_texture   = 2 << ((int) ceil(1.0 + log(128.0 * scaling)));
    m_map_rendered_width    = map_texture;
    m_map_rendered_height   = map_texture;

    createMarkerTexture();
    
    m_gauge_full      = irr_driver->getTexture( file_manager->getGUIDir() + "gauge_full.png" );
    m_gauge_empty     = irr_driver->getTexture( file_manager->getGUIDir() + "gauge_empty.png" );
    m_gauge_goal      = irr_driver->getTexture( file_manager->getGUIDir() + "gauge_goal.png" );

    // Translate strings only one in constructor to avoid calling
    // gettext in each frame.
    //I18N: Shown at the end of a race
    m_string_lap      = _("Lap");
    m_string_rank     = _("Rank");
     
    // Scaled fonts don't look good atm.
    m_font_scale      = 1.0f; //race_manager->getNumLocalPlayers()==1 ? 1.2f : 1.0f;

    //read icon frame picture

    // Determine maximum length of the rank/lap text, in order to
    // align those texts properly on the right side of the viewport.
    gui::ScalableFont* font = GUIEngine::getFont(); 
    float old_scale = font->getScale();
    font->setScale(m_font_scale);
    m_lap_width             = font->getDimension(m_string_lap.c_str()).Width;
    m_timer_width           = font->getDimension(L"99:99:99").Width;
    if(race_manager->getNumberOfKarts()>9)
        m_rank_width        = font->getDimension(L"99/99").Width;
    else
        m_rank_width        = font->getDimension(L"9/9").Width;

    int w;
    if (race_manager->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER ||
        race_manager->getMinorMode()==RaceManager::MINOR_MODE_3_STRIKES     ||
        race_manager->getNumLaps() > 9)
        w = font->getDimension(L" 99/99").Width;
    else
        w = font->getDimension(L" 9/9").Width;
    m_lap_width += w;
    font->setScale(old_scale);
        
}   // MinimalRaceGUI

//-----------------------------------------------------------------------------
MinimalRaceGUI::~MinimalRaceGUI()
{
}   // ~MinimalRaceGUI

//-----------------------------------------------------------------------------
/** Render all global parts of the race gui, i.e. things that are only 
 *  displayed once even in splitscreen.
 *  \param dt Timestep sized.
 */
void MinimalRaceGUI::renderGlobal(float dt)
{
    RaceGUIBase::renderGlobal(dt);
    cleanupMessages(dt);
    
    // Special case : when 3 players play, use 4th window to display such 
    // stuff (but we must clear it)
    if (race_manager->getNumLocalPlayers() == 3 && 
        !GUIEngine::ModalDialog::isADialogActive())
    {
        static video::SColor black = video::SColor(255,0,0,0);
        irr_driver->getVideoDriver()
            ->draw2DRectangle(black,
                              core::rect<s32>(UserConfigParams::m_width/2, 
                                              UserConfigParams::m_height/2,
                                              UserConfigParams::m_width, 
                                              UserConfigParams::m_height));
    }
    
    World *world = World::getWorld();
    assert(world != NULL);
    if(world->getPhase() >= WorldStatus::READY_PHASE &&
       world->getPhase() <= WorldStatus::GO_PHASE      )
    {
        drawGlobalReadySetGo();
    }

    // Timer etc. are not displayed unless the game is actually started.
    if(!world->isRacePhase()) return;
    if (!m_enabled) return;

    drawGlobalTimer();
    if(world->getPhase() == WorldStatus::GO_PHASE ||
       world->getPhase() == WorldStatus::MUSIC_PHASE)
    {
        drawGlobalMusicDescription();
    }

    drawGlobalMiniMap();
    
    // in 3 strikes mode we need to see the lives
    if (world->getIdent() == IDENT_STRIKES)
    {
        KartIconDisplayInfo* info = world->getKartsDisplayInfo();
        drawGlobalPlayerIcons(info, m_map_height);
    }
}   // renderGlobal

//-----------------------------------------------------------------------------
/** Render the details for a single player, i.e. speed, energy, 
 *  collectibles, ...
 *  \param kart Pointer to the kart for which to render the view.
 */
void MinimalRaceGUI::renderPlayerView(const Kart *kart)
{
    if (!m_enabled) return;
    
    const core::recti &viewport    = kart->getCamera()->getViewport();
    core::vector2df scaling = kart->getCamera()->getScaling();
    //std::cout << "Applied ratio : " << viewport.getWidth()/800.0f << std::endl;
    
    scaling *= viewport.getWidth()/800.0f; // scale race GUI along screen size
    
    //std::cout << "Scale : " << scaling.X << ", " << scaling.Y << std::endl;

    if (kart->hasViewBlockedByPlunger())
    {
        int offset_y = viewport.UpperLeftCorner.Y;
        
        const int screen_width = viewport.LowerRightCorner.X 
                               - viewport.UpperLeftCorner.X;
        const int plunger_size = viewport.LowerRightCorner.Y 
                               - viewport.UpperLeftCorner.Y;
        int plunger_x = viewport.UpperLeftCorner.X + screen_width/2 
                      - plunger_size/2;
        
        video::ITexture *t=m_plunger_face->getTexture();
        core::rect<s32> dest(plunger_x,              offset_y, 
                             plunger_x+plunger_size, offset_y+plunger_size);
        const core::rect<s32> source(core::position2d<s32>(0,0), 
                                     t->getOriginalSize());
                
        irr_driver->getVideoDriver()->draw2DImage(t, dest, source, 
                                                  NULL /* clip */, 
                                                  NULL /* color */, 
                                                  true /* alpha */);
    }

    
    drawAllMessages     (kart, viewport, scaling);
    if(!World::getWorld()->isRacePhase()) return;

    MinimalRaceGUI::KartIconDisplayInfo* info = World::getWorld()->getKartsDisplayInfo();

    drawPowerupIcons    (kart, viewport, scaling);
    drawEnergyMeter     (kart, viewport, scaling);
    drawRankLap         (info, kart, viewport);

    RaceGUIBase::renderPlayerView(kart);
}   // renderPlayerView

//-----------------------------------------------------------------------------
/** Displays the racing time on the screen.s
 */
void MinimalRaceGUI::drawGlobalTimer()
{
    assert(World::getWorld() != NULL);
    
    if(!World::getWorld()->shouldDrawTimer()) return;
    std::string s = StringUtils::timeToString(World::getWorld()->getTime());
    core::stringw sw(s.c_str());

    static video::SColor time_color = video::SColor(255, 255, 255, 255);
    int x=0,y=0;  // initialise to avoid compiler warning
    switch(race_manager->getNumLocalPlayers())
    {
    case 1: x = 10; y=0; break;
    case 2: x = 10; y=0; break;
    case 3: x = UserConfigParams::m_width   - m_timer_width-10; 
            y = UserConfigParams::m_height/2; break;
    case 4: x = UserConfigParams::m_width/2 - m_timer_width/2; 
            y = 0;       break;
    }   // switch        

    core::rect<s32> pos(x,                         y, 
                        UserConfigParams::m_width, y+50);
    
    
    gui::ScalableFont* font = GUIEngine::getFont();
    float old_scale = font->getScale();
    font->setScale(m_font_scale);
    font->draw(sw.c_str(), pos, time_color, false, false, NULL, true /* ignore RTL */);
    font->setScale(old_scale);
}   // drawGlobalTimer

//-----------------------------------------------------------------------------
/** Draws the mini map and the position of all karts on it.
 */
void MinimalRaceGUI::drawGlobalMiniMap()
{
    World *world = World::getWorld();
    // arenas currently don't have a map.
    if(world->getTrack()->isArena()) return;

    const video::ITexture *mini_map = world->getTrack()->getMiniMap();
    
    int upper_y = m_map_bottom - m_map_height;
    int lower_y = m_map_bottom;
    
    if (mini_map != NULL)
    {
        core::rect<s32> dest(m_map_left,               upper_y, 
                             m_map_left + m_map_width, lower_y);
        core::rect<s32> source(core::position2di(0, 0), mini_map->getOriginalSize());
        irr_driver->getVideoDriver()->draw2DImage(mini_map, dest, source, 0, 0, true);
    }
    
    // In the first iteration, only draw AI karts, then only draw
    // player karts. This guarantees that player kart icons are always
    // on top of AI kart icons.
    for(unsigned int only_draw_player_kart=0; only_draw_player_kart<=1; 
        only_draw_player_kart++)
    {
        for(unsigned int i=0; i<world->getNumKarts(); i++)
        {
            const Kart *kart = world->getKart(i);
            if(kart->isEliminated()) continue;   // don't draw eliminated kart
            // Make sure to only draw AI kart icons first, then
            // only player karts.
            if(kart->getController()->isPlayerController() 
                !=(only_draw_player_kart==1)) continue;
            const Vec3& xyz = kart->getXYZ();
            Vec3 draw_at;
            world->getTrack()->mapPoint2MiniMap(xyz, &draw_at);

            core::rect<s32> source(i    *m_marker_rendered_size,
                0, 
                (i+1)*m_marker_rendered_size, 
                m_marker_rendered_size);
            int marker_half_size = (kart->getController()->isPlayerController() 
                ? m_marker_player_size 
                : m_marker_ai_size                        )>>1;
            core::rect<s32> position(m_map_left+(int)(draw_at.getX()-marker_half_size), 
                lower_y   -(int)(draw_at.getY()+marker_half_size),
                m_map_left+(int)(draw_at.getX()+marker_half_size), 
                lower_y   -(int)(draw_at.getY()-marker_half_size));

            // Highlight the player icons with some backgorund image.
            if (kart->getController()->isPlayerController())
            {
                video::SColor colors[4];
                for (unsigned int i=0;i<4;i++)
                {
                    colors[i]=kart->getKartProperties()->getColor();
                }
                const core::rect<s32> rect(core::position2d<s32>(0,0),
                    m_icons_frame->getTexture()->getOriginalSize());

                irr_driver->getVideoDriver()->draw2DImage(
                    m_icons_frame->getTexture(), position, rect,
                    NULL, colors, true);
            }   // if isPlayerController

            irr_driver->getVideoDriver()->draw2DImage(m_marker, position, source, 
                NULL, NULL, true);
        }   // for i<getNumKarts
    }   // for only_draw_player_kart
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
void MinimalRaceGUI::drawEnergyMeter(const Kart *kart,              
                                     const core::recti &viewport, 
                                     const core::vector2df &scaling)
{
    float state = (float)(kart->getEnergy()) / MAX_NITRO;
    if      (state < 0.0f) state = 0.0f;
    else if (state > 1.0f) state = 1.0f;
    
    int h = (int)(viewport.getHeight()/3);
    int w = h/4; // gauge image is so 1:4
    
    // In split screen mode of 3 or 4 players, the players on
    // the left side will have the energy meter on the left side
    int mirrored = race_manager->getNumLocalPlayers()>=3 &&
                   viewport.UpperLeftCorner.X==0;

    int x = mirrored ? 0 : viewport.LowerRightCorner.X - w;
    int y = viewport.UpperLeftCorner.Y + viewport.getHeight()/2- h/2;
    
    // Background
    // ----------
    core::rect<s32> dest(x+mirrored*w, y+mirrored*h, 
                         x+(1-mirrored)*w, y+(1-mirrored)*h);

    irr_driver->getVideoDriver()->draw2DImage(m_gauge_empty, dest,
                                              core::rect<s32>(0, 0, 64, 256) /* source rect */,
                                              NULL /* clip rect */, NULL /* colors */,
                                              true /* alpha */);
    // Target
    // ------
    if (race_manager->getCoinTarget() > 0)
    {
        float coin_target = (float)race_manager->getCoinTarget() / MAX_NITRO;
        
        const int EMPTY_TOP_PIXELS = 4;
        const int EMPTY_BOTTOM_PIXELS = 3;
        int y1 = y + (int)(EMPTY_TOP_PIXELS + 
                             (h - EMPTY_TOP_PIXELS - EMPTY_BOTTOM_PIXELS)
                            *(1.0f - coin_target)                        );
        if (state >= 1.0f) y1 = y;
        
        core::rect<s32> clip(x, y1, x + w, y + h);
        irr_driver->getVideoDriver()->draw2DImage(m_gauge_goal, core::rect<s32>(x, y, x+w, y+h) /* dest rect */,
                                                  core::rect<s32>(0, 0, 64, 256) /* source rect */,
                                                  &clip, NULL /* colors */, true /* alpha */);
    }
    
    // Filling (current state)
    // -----------------------
    if (state > 0.0f)
    {
        const int EMPTY_TOP_PIXELS = 4;
        const int EMPTY_BOTTOM_PIXELS = 3;
        int y1 = y + (int)(EMPTY_TOP_PIXELS 
                           + (h - EMPTY_TOP_PIXELS - EMPTY_BOTTOM_PIXELS)
                              *(1.0f - state)                             );
        if (state >= 1.0f) y1 = y;
        core::rect<s32> dest(x+mirrored*w,
                             mirrored ? y+h : y,
                             x+(1-mirrored)*w,
                             mirrored ? y : y + h);
        core::rect<s32> clip(x, y1, x + w, y + h);
        core::rect<s32> tex_c(0,
                              mirrored ? 256 :   0,
                              64,
                              mirrored ?   0 : 256);
        irr_driver->getVideoDriver()->draw2DImage(m_gauge_full, dest,
                                                  tex_c,
                                                  &clip, NULL /* colors */, true /* alpha */);
    }
    
    
}   // drawEnergyMeter

//-----------------------------------------------------------------------------
/** Displays the rank and the lap of the kart.
 *  \param info Info object c
*/
void MinimalRaceGUI::drawRankLap(const KartIconDisplayInfo* info, 
                                 const Kart* kart,
                                 const core::recti &viewport)
{
    // Don't display laps or ranks if the kart has already finished the race.
    if (kart->hasFinishedRace()) return;

    core::recti pos;

    gui::ScalableFont* font = (race_manager->getNumLocalPlayers() > 2 
                            ? GUIEngine::getSmallFont() 
                            : GUIEngine::getFont());
    float scale = font->getScale();
    font->setScale(m_font_scale);
    // Add a black shadow to make the text better readable on
    // 'white' tracks (e.g. with snow and ice).
    font->setShadow(video::SColor(255, 0, 0, 0));
    static video::SColor color = video::SColor(255, 255, 255, 255);
    WorldWithRank *world    = (WorldWithRank*)(World::getWorld());

    if (world->displayRank())
    {
        pos.UpperLeftCorner.Y   = viewport.UpperLeftCorner.Y;
        pos.LowerRightCorner.Y  = viewport.UpperLeftCorner.Y+50;
        // Split screen 3 or 4 players, left side:
        if(viewport.LowerRightCorner.X < UserConfigParams::m_width)
        {
            pos.UpperLeftCorner.X   = 10;
            pos.LowerRightCorner.X  = viewport.LowerRightCorner.X;
        }
        else
        {
            pos.UpperLeftCorner.X   = viewport.LowerRightCorner.X
                                    - m_rank_width-10;
            pos.LowerRightCorner.X  = viewport.LowerRightCorner.X;
        }

        char str[256];
        sprintf(str, "%d/%d", kart->getPosition(), 
                world->getCurrentNumKarts());
        font->draw(str, pos, color);
    }
    
    // Don't display laps in follow the leader mode
    if(world->raceHasLaps())
    {
        const int lap = info[kart->getWorldKartId()].lap;
    
        // don't display 'lap 0/...'
        if(lap>=0)
        {
            pos.LowerRightCorner.Y  = viewport.LowerRightCorner.Y;
            pos.UpperLeftCorner.Y   = viewport.LowerRightCorner.Y-60;
            pos.LowerRightCorner.X  = viewport.LowerRightCorner.X;
            // Split screen 3 or 4 players, left side:
            if(viewport.LowerRightCorner.X < UserConfigParams::m_width)
            {
                pos.UpperLeftCorner.X = 10;
            }
            else
            {
                pos.UpperLeftCorner.X = (int)(viewport.LowerRightCorner.X
                                              - m_lap_width -10          );
            }

            char str[256];
            sprintf(str, "%d/%d", lap+1, race_manager->getNumLaps());
            core::stringw s = m_string_lap+" "+str;
            font->draw(s.c_str(), pos, color);
        }
    }
    font->setScale(scale);
    font->disableShadow();
} // drawRankLap

//-----------------------------------------------------------------------------
