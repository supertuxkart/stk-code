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

#include "states_screens/race_gui.hpp"

#include "irrlicht.h"
using namespace irr;

#include <algorithm>

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
RaceGUI::RaceGUI()
{    
    m_enabled = true;
    
    // Originally m_map_height was 100, and we take 480 as minimum res
    const float scaling = irr_driver->getFrameSize().Height / 480.0f;
    // Marker texture has to be power-of-two for (old) OpenGL compliance
    m_marker_rendered_size  =  2 << ((int) ceil(1.0 + log(32.0 * scaling)));
    m_marker_ai_size        = (int)( 14.0f * scaling);
    m_marker_player_size    = (int)( 16.0f * scaling);
    m_map_width             = (int)(100.0f * scaling);
    m_map_height            = (int)(100.0f * scaling);
    m_map_left              = (int)( 10.0f * scaling);
    m_map_bottom            = (int)( 10.0f * scaling);
    
    // Minimap is also rendered bigger via OpenGL, so find power-of-two again
    const int map_texture   = 2 << ((int) ceil(1.0 + log(128.0 * scaling)));
    m_map_rendered_width    = map_texture;
    m_map_rendered_height   = map_texture;


    // special case : when 3 players play, use available 4th space for such things
    if (race_manager->getNumLocalPlayers() == 3)
    {
        m_map_left = UserConfigParams::m_width - m_map_width;
    }

    m_speed_meter_icon = material_manager->getMaterial("speedback.png");
    m_speed_bar_icon   = material_manager->getMaterial("speedfore.png");
    createMarkerTexture();
    
    // Translate strings only one in constructor to avoid calling
    // gettext in each frame.
    //I18N: Shown at the end of a race
    m_string_lap      = _("Lap");
    m_string_rank     = _("Rank");
    
    //I18N: When some GlobalPlayerIcons are hidden, write "Top 10" to show it
    m_string_top      = _("Top %i");
    
    m_dist_show_overlap=2;
    m_icons_inertia=2;
    
    // Determine maximum length of the rank/lap text, in order to
    // align those texts properly on the right side of the viewport.
    gui::ScalableFont* font = GUIEngine::getFont(); 
    m_rank_lap_width = font->getDimension(m_string_lap.c_str()).Width;
    
    m_timer_width = font->getDimension(L"99:99:99").Width;

    font = (race_manager->getNumLocalPlayers() > 2 ? GUIEngine::getSmallFont() : GUIEngine::getFont());
    
    int w;
    if (race_manager->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER ||
        race_manager->getMinorMode()==RaceManager::MINOR_MODE_3_STRIKES     ||
        race_manager->getNumLaps() > 9)
        w = font->getDimension(L"99/99").Width;
    else
        w = font->getDimension(L"9/9").Width;
    
    // In some split screen configuration the energy bar might be next 
    // to the lap display - so make the lap X/Y display large enough to
    // leave space for the energy bar (16 pixels) and 10 pixels of space
    // to the right (see drawEnergyMeter for details).
    w += 16 + 10;
    if(m_rank_lap_width < w) m_rank_lap_width = w;
    w = font->getDimension(m_string_rank.c_str()).Width;
    if(m_rank_lap_width < w) m_rank_lap_width = w;
    
}   // RaceGUI

//-----------------------------------------------------------------------------
RaceGUI::~RaceGUI()
{
}   // ~Racegui

//-----------------------------------------------------------------------------
/** Render all global parts of the race gui, i.e. things that are only 
 *  displayed once even in splitscreen.
 *  \param dt Timestep sized.
 */
void RaceGUI::renderGlobal(float dt)
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

    // minimap has no mipmaps so disable material2D
    //irr_driver->getVideoDriver()->enableMaterial2D(false);
    drawGlobalMiniMap();
    //irr_driver->getVideoDriver()->enableMaterial2D();
    
    KartIconDisplayInfo* info = world->getKartsDisplayInfo();
    
    drawGlobalPlayerIcons(info);
}   // renderGlobal

//-----------------------------------------------------------------------------
/** Render the details for a single player, i.e. speed, energy, 
 *  collectibles, ...
 *  \param kart Pointer to the kart for which to render the view.
 */
void RaceGUI::renderPlayerView(const Kart *kart)
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
        
        //static const video::SColor white = video::SColor(255, 255, 255, 255);
        
        irr_driver->getVideoDriver()->draw2DImage(t, dest, source, 
                                                  NULL /* clip */, 
                                                  NULL /* color */, 
                                                  true /* alpha */);
    }

    
    drawAllMessages     (kart, viewport, scaling);
    
    if(!World::getWorld()->isRacePhase()) return;

    RaceGUI::KartIconDisplayInfo* info = World::getWorld()->getKartsDisplayInfo();

    drawPowerupIcons    (kart, viewport, scaling);
    drawSpeedAndEnergy  (kart, viewport, scaling);
    drawRankLap         (info, kart, viewport);

    RaceGUIBase::renderPlayerView(kart);
}   // renderPlayerView

//-----------------------------------------------------------------------------
/** Displays the racing time on the screen.s
 */
void RaceGUI::drawGlobalTimer()
{
    assert(World::getWorld() != NULL);
    
    if(!World::getWorld()->shouldDrawTimer()) return;
    std::string s = StringUtils::timeToString(World::getWorld()->getTime());
    core::stringw sw(s.c_str());

    static video::SColor time_color = video::SColor(255, 255, 255, 255);
    core::rect<s32> pos(UserConfigParams::m_width - m_timer_width - 10, 10, 
                        UserConfigParams::m_width,                      50);
    
    // special case : when 3 players play, use available 4th space for such things
    if (race_manager->getNumLocalPlayers() == 3)
    {
        pos += core::vector2d<s32>(0, UserConfigParams::m_height/2);
    }
    
    gui::ScalableFont* font = GUIEngine::getFont();
    font->draw(sw.c_str(), pos, time_color, false, false, NULL, true /* ignore RTL */);
}   // drawGlobalTimer

//-----------------------------------------------------------------------------
/** Draws the mini map and the position of all karts on it.
 */
void RaceGUI::drawGlobalMiniMap()
{
    World *world = World::getWorld();
    // arenas currently don't have a map.
    if(world->getTrack()->isArena()) return;

    const video::ITexture *mini_map=world->getTrack()->getMiniMap();
    
    int upper_y = UserConfigParams::m_height-m_map_bottom-m_map_height;
    int lower_y = UserConfigParams::m_height-m_map_bottom;
    
    core::rect<s32> dest(m_map_left,               upper_y, 
                         m_map_left + m_map_width, lower_y);
    core::rect<s32> source(core::position2di(0, 0), mini_map->getOriginalSize());
    irr_driver->getVideoDriver()->draw2DImage(mini_map, dest, source, 0, 0, true);

    for(unsigned int i=0; i<world->getNumKarts(); i++)
    {
        const Kart *kart = world->getKart(i);
        if(kart->isEliminated()) continue;   // don't draw eliminated kart
        const Vec3& xyz = kart->getXYZ();
        Vec3 draw_at;
        world->getTrack()->mapPoint2MiniMap(xyz, &draw_at);
        // int marker_height = m_marker->getOriginalSize().Height;
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
        irr_driver->getVideoDriver()->draw2DImage(m_marker, position, source, 
                                                  NULL, NULL, true);
    }   // for i<getNumKarts
}   // drawGlobalMiniMap

//-----------------------------------------------------------------------------
// Draw players icons and their times (if defined in the current mode).
void RaceGUI::drawGlobalPlayerIcons(const KartIconDisplayInfo* info)
{
    int x_base = 10;
    int y_base = 20;
    int ICON_WIDTH=(int)(40*(UserConfigParams::m_width/800.0f));
    int ICON_PLAYER_WIDTH=(int)(50*(UserConfigParams::m_width/800.0f));
    if(UserConfigParams::m_height<600)
    {
        ICON_WIDTH        = 27;
        ICON_PLAYER_WIDTH = 35;
    }

    // Special case : when 3 players play, use 4th window to display such stuff
    if (race_manager->getNumLocalPlayers() == 3)
    {
        x_base = UserConfigParams::m_width/2 + x_base;
        y_base = UserConfigParams::m_height/2 + y_base;
    }

    WorldWithRank *world    = (WorldWithRank*)(World::getWorld());
    //initialize m_previous_icons_position
    if(m_previous_icons_position.size()==0)
    {
        for(unsigned int i=0; i<race_manager->getNumberOfKarts(); i++)
        {
            const Kart *kart = world->getKart(i);
            int position = kart->getPosition();
            core::vector2d<s32> pos(x_base,y_base+(position-1)*(ICON_PLAYER_WIDTH+2));
            m_previous_icons_position.push_back(pos);
        }
    }
    
    int x;
    int y;
    float previous_distance=0.0;//no need to be far ahead, first kart won't try to overlap
    
    
    
    int previous_x=x_base;
    int previous_y=y_base-ICON_PLAYER_WIDTH-2;
    
    gui::ScalableFont* font = GUIEngine::getFont();
    const unsigned int kart_amount = world->getNumKarts();
    
    //where is the limit to hide last icons
    int y_icons_limit=UserConfigParams::m_height-m_map_height-ICON_PLAYER_WIDTH;
    if (race_manager->getNumLocalPlayers() == 3)
        y_icons_limit=UserConfigParams::m_height-ICON_WIDTH;
    
    
    for(int position = 1; position <= (int)kart_amount ; position++)
    {
        Kart *kart = world->getKartAtPosition(position);
        
        if (kart->getPosition() == -1)//if position is not set
        {
            //we use karts ordered by id only
            //(needed for beginning of MINOR_MODE_3_STRIKES)
            kart= world->getKart(position-1);
        }
        
        if(kart->isEliminated()) continue;
        unsigned int kart_id = kart->getWorldKartId();

        //x,y is the target position
        int lap = info[kart->getWorldKartId()].lap;

        // In battle mode there is no distance along track etc.
        if(race_manager->getMinorMode()==RaceManager::MINOR_MODE_3_STRIKES)
        {
            x = x_base;
            y = previous_y+ICON_PLAYER_WIDTH+2;
        }
        else
        {
            LinearWorld *linear_world      = (LinearWorld*)(World::getWorld());
            
            float distance = linear_world->getDistanceDownTrackForKart(kart_id)
                           + linear_world->getTrack()->getTrackLength()*lap;
            if ((position>1) && (previous_distance-distance<m_dist_show_overlap) && (!kart->hasFinishedRace()))
            {
                //linear translation : form (0,ICON_PLAYER_WIDTH+2) to 
                // (previous_x-x_base+(ICON_PLAYER_WIDTH+2)/2,0)
                x=(int)(x_base+(1-(previous_distance-distance)
                    /m_dist_show_overlap)
                    *(previous_x-x_base+(ICON_PLAYER_WIDTH+2)/2));
                y=(int)(previous_y+(previous_distance-distance)
                    /m_dist_show_overlap*(ICON_PLAYER_WIDTH+2));
            }
            else
            {
                x=x_base;
                y=previous_y+ICON_PLAYER_WIDTH+2;
            }
            previous_distance=distance;
        }   // not three-strike-battle
        
        
        previous_x=x;//save coord of the previous kart in list
        previous_y=y;

        //soft movement using previous position:
        x=(int)((x+m_previous_icons_position[kart_id].X*m_icons_inertia)
            /(m_icons_inertia+1));
        y=(int)((y+m_previous_icons_position[kart_id].Y*m_icons_inertia)
            /(m_icons_inertia+1));

        //save position for next time
        m_previous_icons_position[kart_id].X=x;
        m_previous_icons_position[kart_id].Y=y;

        if (y>y_icons_limit)
        {
            //there are too many icons, write "Top 9", to express that
            //there is not everybody shown
            core::recti pos_top;
            pos_top.UpperLeftCorner.Y  = y_base-22;
            pos_top.UpperLeftCorner.X  = x_base;

            static video::SColor color = video::SColor(255, 255, 255, 255);
            pos_top.LowerRightCorner   = pos_top.UpperLeftCorner;

            font->draw(StringUtils::insertValues( m_string_top, position-1 ), pos_top, color);
            
            break;
        }

        if (info[kart_id].m_text.size() > 0)
        {
            video::SColor color = video::SColor(255,
                (int)(255*info[kart_id].r),
                (int)(255*info[kart_id].g), 
                (int)(255*info[kart_id].b)   );
            core::rect<s32> pos(x+ICON_PLAYER_WIDTH, y+5, 
                x+ICON_PLAYER_WIDTH, y+5);
            core::stringw s=info[kart_id].m_text.c_str();

            font->draw(s.c_str(), pos, color, false, false, NULL, true /* ignore RTL */);
        }

        if (info[kart_id].special_title.size() > 0)
        {
            static video::SColor color = video::SColor(255, 255, 0, 0);
            core::rect<s32> pos(x+ICON_PLAYER_WIDTH, y+5, 
                x+ICON_PLAYER_WIDTH, y+5);
            core::stringw s(info[kart_id].special_title.c_str());
            font->draw(s.c_str(), pos, color, false, false, NULL, true /* ignore RTL */);
        }

        // draw icon
        video::ITexture *icon = 
            kart->getKartProperties()->getIconMaterial()->getTexture();
        int w =
            kart->getController()->isPlayerController() ? ICON_PLAYER_WIDTH
            : ICON_WIDTH;
        const core::rect<s32> pos(x, y, x+w, y+w);
        
        //to bring to light the player's icon: add a background
        if (kart->getController()->isPlayerController())
        {
            video::SColor colors[4];
            for (unsigned int i=0;i<4;i++)
            {
                colors[i]=kart->getKartProperties()->getColor();
                colors[i].setAlpha(
                    100+(int)(100*cos(M_PI/2*i+World::getWorld()->getTime()*2)));
            }
            const core::rect<s32> rect(core::position2d<s32>(0,0),
                m_icons_frame->getTexture()->getOriginalSize());
            irr_driver->getVideoDriver()->draw2DImage(
                m_icons_frame->getTexture(), pos, rect,NULL, colors, true);
        }
        
        // Fixes crash bug, why are certain icons not showing up?
        if ((icon != NULL) && (!kart->playingEmergencyAnimation()))
        {
            const core::rect<s32> rect(core::position2d<s32>(0,0),
                icon->getOriginalSize());
            irr_driver->getVideoDriver()->draw2DImage(icon, pos, rect, 
                NULL, NULL, true);
        }

        //draw status info

        if ((icon != NULL) && (kart->playingRescueAnimation()))
        {
            //icon fades to the left
            float t_anim=100*sin(0.5f*M_PI*kart->getAnimationTimer());
            const core::rect<s32> rect1(core::position2d<s32>(0,0), 
                icon->getOriginalSize());
            const core::rect<s32> pos1((int)(x-t_anim), y, 
                (int)(x+w-t_anim), y+w);
            irr_driver->getVideoDriver()->draw2DImage(icon, pos1, rect1, 
                NULL, NULL, true);
        }

        if ((icon != NULL) && (kart->playingExplosionAnimation()))
        {
            //exploses into 4 parts
            float t_anim=50.0f*sin(0.5f*M_PI*kart->getAnimationTimer());
            u16 icon_size_x=icon->getOriginalSize().Width;
            u16 icon_size_y=icon->getOriginalSize().Height;

            const core::rect<s32> rect1(0, 0, icon_size_x/2,icon_size_y/2);
            const core::rect<s32> pos1((int)(x-t_anim), (int)(y-t_anim), 
                (int)(x+w/2-t_anim),
                (int)(y+w/2-t_anim));
            irr_driver->getVideoDriver()->draw2DImage(icon, pos1, rect1, 
                NULL, NULL, true);

            const core::rect<s32> rect2(icon_size_x/2,0,
                icon_size_x,icon_size_y/2);
            const core::rect<s32> pos2((int)(x+w/2+t_anim),
                (int)(y-t_anim), 
                (int)(x+w+t_anim),
                (int)(y+w/2-t_anim));
            irr_driver->getVideoDriver()->draw2DImage(icon, pos2, rect2,
                NULL, NULL, true);

            const core::rect<s32> rect3(0, icon_size_y/2, icon_size_x/2,icon_size_y);
            const core::rect<s32> pos3((int)(x-t_anim), (int)(y+w/2+t_anim), 
                (int)(x+w/2-t_anim), (int)(y+w+t_anim));
            irr_driver->getVideoDriver()->draw2DImage(icon, pos3, rect3, NULL, NULL, true);

            const core::rect<s32> rect4(icon_size_x/2,icon_size_y/2,icon_size_x,icon_size_y);
            const core::rect<s32> pos4((int)(x+w/2+t_anim), (int)(y+w/2+t_anim), 
                (int)(x+w+t_anim), (int)(y+w+t_anim));
            irr_driver->getVideoDriver()->draw2DImage(icon, pos4, rect4, NULL, NULL, true);
        }

        //Plunger
        if (kart->hasViewBlockedByPlunger())
        {
            video::ITexture *icon_plunger = 
                powerup_manager->getIcon(PowerupManager::POWERUP_PLUNGER)->getTexture();
            if (icon_plunger != NULL)
            {
                const core::rect<s32> rect(core::position2d<s32>(0,0), 
                    icon_plunger->getOriginalSize());
                const core::rect<s32> pos1(x+10, y-10, x+w+10, y+w-10);
                irr_driver->getVideoDriver()->draw2DImage(icon_plunger, pos1, 
                    rect, NULL, NULL, 
                    true);
            }
        }
        //attachment
        if (kart->getAttachment()->getType() != ATTACH_NOTHING)
        {
            video::ITexture *icon_attachment = 
                attachment_manager->getIcon(kart->getAttachment()->getType())
                                 ->getTexture();
            if (icon_attachment != NULL)
            {
                const core::rect<s32> rect(core::position2d<s32>(0,0), 
                                           icon_attachment->getOriginalSize());
                const core::rect<s32> pos1(x-20, y-10, x+w-20, y+w-10);
                irr_driver->getVideoDriver()->draw2DImage(icon_attachment, 
                                                          pos1, rect, NULL, 
                                                          NULL, true);
            }
        }

    } //next position
}   // drawGlobalPlayerIcons

//-----------------------------------------------------------------------------
/** Energy meter that gets filled with nitro. This function is called from
 *  drawSpeedAndEnergy, which defines the correct position of the energy
 *  meter.
 *  \param x X position of the meter.
 *  \param y Y position of the meter.
 *  \param kart Kart to display the data for.
 *  \param scaling Scaling applied (in case of split screen)
 */
void RaceGUI::drawEnergyMeter(int x, int y, const Kart *kart,              
                              const core::recti &viewport, 
                              const core::vector2df &scaling)
{
    float state = (float)(kart->getEnergy()) / MAX_NITRO;
    if (state < 0.0f) state = 0.0f;
    else if (state > 1.0f) state = 1.0f;
    
    int h = (int)(viewport.getHeight()/3);
    int w = h/4; // gauge image is so 1:4
    
    y -= h;
    
    x    -= w;
    
    // Background
    irr_driver->getVideoDriver()->draw2DImage(m_gauge_empty, core::rect<s32>(x, y, x+w, y+h) /* dest rect */,
                                              core::rect<s32>(0, 0, 64, 256) /* source rect */,
                                              NULL /* clip rect */, NULL /* colors */,
                                              true /* alpha */);
    
    // Target
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
    if (state > 0.0f)
    {
        const int EMPTY_TOP_PIXELS = 4;
        const int EMPTY_BOTTOM_PIXELS = 3;
        int y1 = y + (int)(EMPTY_TOP_PIXELS 
                           + (h - EMPTY_TOP_PIXELS - EMPTY_BOTTOM_PIXELS)
                              *(1.0f - state)                             );
        if (state >= 1.0f) y1 = y;
        
        core::rect<s32> clip(x, y1, x + w, y + h);
        irr_driver->getVideoDriver()->draw2DImage(m_gauge_full, core::rect<s32>(x, y, x+w, y+h) /* dest rect */,
                                                  core::rect<s32>(0, 0, 64, 256) /* source rect */,
                                                  &clip, NULL /* colors */, true /* alpha */);
    }
    
    
}   // drawEnergyMeter

//-----------------------------------------------------------------------------
void RaceGUI::drawSpeedAndEnergy(const Kart* kart, const core::recti &viewport,
                                 const core::vector2df &scaling)
{

    float minRatio         = std::min(scaling.X, scaling.Y);
    const int SPEEDWIDTH   = 128;
    int meter_width        = (int)(SPEEDWIDTH*minRatio);
    int meter_height       = (int)(SPEEDWIDTH*minRatio);

    drawEnergyMeter(viewport.LowerRightCorner.X, 
                    (int)(viewport.LowerRightCorner.Y - meter_height*0.75f), 
                    kart, viewport, scaling);
    
    // First draw the meter (i.e. the background which contains the numbers etc.
    // -------------------------------------------------------------------------
    
    core::vector2df offset;
    offset.X = (float)(viewport.LowerRightCorner.X-meter_width) - 15.0f*scaling.X;
    offset.Y = viewport.LowerRightCorner.Y-10*scaling.Y;
    
    video::IVideoDriver *video = irr_driver->getVideoDriver();
    const core::rect<s32> meter_pos((int)offset.X,
                                    (int)(offset.Y-meter_height),
                                    (int)(offset.X+meter_width),
                                    (int)offset.Y);
    video::ITexture *meter_texture = m_speed_meter_icon->getTexture();
    const core::rect<s32> meter_texture_coords(core::position2d<s32>(0,0), 
                                               meter_texture->getOriginalSize());
    video->draw2DImage(meter_texture, meter_pos, meter_texture_coords, NULL, NULL, true);

    const float speed =  kart->getSpeed();
    if(speed <=0) return;  // Nothing to do if speed is negative.

    // Draw the actual speed bar (if the speed is >0)
    // ----------------------------------------------
    float speed_ratio = speed/KILOMETERS_PER_HOUR/110.0f;
    if(speed_ratio>1) speed_ratio = 1;

    video::ITexture   *bar_texture = m_speed_bar_icon->getTexture();
    core::dimension2du bar_size    = bar_texture->getOriginalSize();
    video::S3DVertex vertices[5];
    unsigned int count;

    // There are three different polygons used, depending on 
    // the speed ratio. Consider the speed-display texture:
    //
    //   C----x----D       (position of v,w,x,y vary depending on  
    //   |         |        speed)
    //   w         y
    //   |         |
    //   B----A----E
    // For speed ratio <= r1 the triangle ABw is used, with w between B and C.
    // For speed ratio <= r2 the quad ABCx is used, with x between C and D.
    // For speed ratio >  r2 the poly ABCDy is used, with y between D and E.

    vertices[0].TCoords = core::vector2df(0.5f, 1.0f);
    vertices[0].Pos     = core::vector3df(offset.X+meter_width/2, offset.Y, 0);
    vertices[1].TCoords = core::vector2df(0, 1.0f);
    vertices[1].Pos     = core::vector3df(offset.X, offset.Y, 0);
    // The speed ratios at which different triangles must be used.
    // These values should be adjusted in case that the speed display
    // is not linear enough. Mostly the speed values are below 0.7, it
    // needs some zipper to get closer to 1.
    const float r1 = 0.4f;
    const float r2 = 0.8f;
    if(speed_ratio<=r1)
    {
        count   = 3;
        float f = speed_ratio/r1;
        vertices[2].TCoords = core::vector2df(0, 1.0f-f);
        vertices[2].Pos = core::vector3df(offset.X, offset.Y-f*meter_height,0);
    }
    else if(speed_ratio<=r2)
    {
        count   = 4;
        float f = (speed_ratio - r1)/(r2-r1);
        vertices[2].TCoords = core::vector2df(0,0);
        vertices[2].Pos = core::vector3df(offset.X, offset.Y-meter_height, 0);
        vertices[3].TCoords = core::vector2df(f, 0);
        vertices[3].Pos     = core::vector3df(offset.X+f*meter_width,
                                              offset.Y-meter_height,
                                              0);
    }
    else
    {
        count   = 5;
        float f = (speed_ratio - r2)/(1-r2);
        vertices[2].TCoords = core::vector2df(0,0);
        vertices[2].Pos = core::vector3df(offset.X, offset.Y-meter_height, 0);
        vertices[3].TCoords = core::vector2df(1, 0);
        vertices[3].Pos     = core::vector3df(offset.X+meter_width,
                                              offset.Y-meter_height,
                                              0);
        vertices[4].TCoords = core::vector2df(1.0f, f);
        vertices[4].Pos     = core::vector3df(offset.X+meter_width,
                                              offset.Y - (1-f)*meter_height,
                                              0);
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
    irr_driver->getVideoDriver()->draw2DVertexPrimitiveList(vertices, count,
        index, count-2, video::EVT_STANDARD, scene::EPT_TRIANGLE_FAN);
} // drawSpeed

//-----------------------------------------------------------------------------
/** Displays the rank and the lap of the kart.
 *  \param info Info object c
*/
void RaceGUI::drawRankLap(const KartIconDisplayInfo* info, const Kart* kart, 
                          const core::recti &viewport)
{
    // Don't display laps or ranks if the kart has already finished the race.
    if (kart->hasFinishedRace()) return;

    core::recti pos;
    pos.UpperLeftCorner.Y   = viewport.UpperLeftCorner.Y;
    // If the time display in the top right is in this viewport,
    // move the lap/rank display down a little bit so that it is
    // displayed under the time.
    if(viewport.UpperLeftCorner.Y==0 && 
        viewport.LowerRightCorner.X==UserConfigParams::m_width &&
        race_manager->getNumPlayers()!=3)
        pos.UpperLeftCorner.Y   += 40;
    pos.LowerRightCorner.Y  = viewport.LowerRightCorner.Y;
    pos.UpperLeftCorner.X   = viewport.LowerRightCorner.X 
                            - m_rank_lap_width - 10;
    pos.LowerRightCorner.X  = viewport.LowerRightCorner.X;

    gui::ScalableFont* font = (race_manager->getNumLocalPlayers() > 2 ? GUIEngine::getSmallFont() : GUIEngine::getFont());
    int font_height         = (int)(font->getDimension(L"X").Height);
    static video::SColor color = video::SColor(255, 255, 255, 255);
    WorldWithRank *world    = (WorldWithRank*)(World::getWorld());

    if (world->displayRank())
    {
        const int rank = kart->getPosition();
            
        font->draw(m_string_rank.c_str(), pos, color);
        pos.UpperLeftCorner.Y  += font_height;
        pos.LowerRightCorner.Y += font_height;

        char str[256];
        const unsigned int kart_amount = world->getCurrentNumKarts();
        sprintf(str, "%d/%d", rank, kart_amount);
        font->draw(core::stringw(str).c_str(), pos, color);
        pos.UpperLeftCorner.Y  += font_height;
        pos.LowerRightCorner.Y += font_height;
    }
    
    // Don't display laps in follow the leader mode
    if(world->raceHasLaps())
    {
        const int lap = info[kart->getWorldKartId()].lap;
    
        // don't display 'lap 0/...'
        if(lap>=0)
        {
            font->draw(m_string_lap.c_str(), pos, color);
            char str[256];
            sprintf(str, "%d/%d", lap+1, race_manager->getNumLaps());
            pos.UpperLeftCorner.Y  += font_height;
            pos.LowerRightCorner.Y += font_height;
            font->draw(core::stringw(str).c_str(), pos, color);
            pos.UpperLeftCorner.Y  += font_height;
            pos.LowerRightCorner.Y += font_height;
        }
    }

} // drawRankLap
