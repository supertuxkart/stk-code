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

#include "states_screens/race_gui_overworld.hpp"

using namespace irr;

#include <algorithm>

#include "challenges/unlock_manager.hpp"
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
#include "modes/world.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <ISceneCollisionManager.h>
#include <ISceneManager.h>

const int LOCKED = 0;
const int OPEN = 1;
const int COMPLETED = 2;

/** The constructor is called before anything is attached to the scene node.
 *  So rendering to a texture can be done here. But world is not yet fully
 *  created, so only the race manager can be accessed safely.
 */
RaceGUIOverworld::RaceGUIOverworld()
{    
    m_enabled = true;
    m_trophy  = irr_driver->getTexture( file_manager->getTextureFile("cup_gold.png") );
    
    const float scaling = irr_driver->getFrameSize().Height / 420.0f;
    // Marker texture has to be power-of-two for (old) OpenGL compliance
    m_marker_rendered_size  =  2 << ((int) ceil(1.0 + log(32.0 * scaling)));
    m_marker_challenge_size        = (int)( 12.0f * scaling);
    m_marker_player_size    = (int)( 24.0f * scaling);
    m_map_width             = (int)(250.0f * scaling);
    m_map_height            = (int)(250.0f * scaling);
    
    m_map_left   = 20;
    m_map_bottom = UserConfigParams::m_height-10;
    
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
    
    
    // Determine maximum length of the rank/lap text, in order to
    // align those texts properly on the right side of the viewport.
    gui::ScalableFont* font = GUIEngine::getFont(); 
    m_trophy_points_width = font->getDimension(L"100").Width;
    
    const std::vector<const ChallengeData*>& v = unlock_manager->getCurrentSlot()->getLockedChallenges();
    for (unsigned int n=0; n<v.size(); n++)
    {
        m_locked_challenges.insert(v[n]);
    }
    
    m_lock = irr_driver->getTexture( file_manager->getTextureFile("gui_lock.png") );
    m_open_challenge = irr_driver->getTexture( file_manager->getGUIDir() + "challenge.png" );
    
    m_icons[0] = m_lock;
    m_icons[1] = m_open_challenge;
    m_icons[2] = m_trophy;
}   // RaceGUIOverworld

//-----------------------------------------------------------------------------
RaceGUIOverworld::~RaceGUIOverworld()
{
}   // ~RaceGUIOverworld

//-----------------------------------------------------------------------------
/** Render all global parts of the race gui, i.e. things that are only 
 *  displayed once even in splitscreen.
 *  \param dt Timestep sized.
 */
void RaceGUIOverworld::renderGlobal(float dt)
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
    
    drawTrophyPoints();
    
    // minimap has no mipmaps so disable material2D
    //irr_driver->getVideoDriver()->enableMaterial2D(false);
    drawGlobalMiniMap();
    //irr_driver->getVideoDriver()->enableMaterial2D();
    
}   // renderGlobal

//-----------------------------------------------------------------------------
/** Render the details for a single player, i.e. speed, energy, 
 *  collectibles, ...
 *  \param kart Pointer to the kart for which to render the view.
 */
void RaceGUIOverworld::renderPlayerView(const Kart *kart)
{
    if (!m_enabled) return;
    
    const core::recti &viewport    = kart->getCamera()->getViewport();
    
    core::vector2df scaling = kart->getCamera()->getScaling();
    //std::cout << "Applied ratio : " << viewport.getWidth()/800.0f << std::endl;
    
    scaling *= viewport.getWidth()/800.0f; // scale race GUI along screen size
    
    //std::cout << "Scale : " << scaling.X << ", " << scaling.Y << std::endl;
    
    drawAllMessages     (kart, viewport, scaling);
    
    if(!World::getWorld()->isRacePhase()) return;
        
    drawPowerupIcons    (kart, viewport, scaling);
    
    RaceGUIBase::renderPlayerView(kart);
}   // renderPlayerView

//-----------------------------------------------------------------------------
/** Displays the number of challenge trophies
 */
void RaceGUIOverworld::drawTrophyPoints()
{
    const int points = unlock_manager->getCurrentSlot()->getPoints();
    std::string s = StringUtils::toString(points);
    core::stringw sw(s.c_str());
    
    static video::SColor time_color = video::SColor(255, 255, 255, 255);
    
    int dist_from_right = 10 + m_trophy_points_width;
    
    core::rect<s32> pos(UserConfigParams::m_width - dist_from_right, 10, 
                        UserConfigParams::m_width                  , 50);
    
    bool vcenter = false;
    
    gui::ScalableFont* font = GUIEngine::getFont();
    
    vcenter = true;
    
    const int size = UserConfigParams::m_width/20;
    core::rect<s32> dest(pos.UpperLeftCorner.X - size - 5, pos.UpperLeftCorner.Y,
                         pos.UpperLeftCorner.X - 5, pos.UpperLeftCorner.Y + size);
    core::rect<s32> source(core::position2di(0, 0), m_trophy->getSize());
    
    irr_driver->getVideoDriver()->draw2DImage(m_trophy, dest, source, NULL,
                                              NULL, true /* alpha */);
    
    pos.LowerRightCorner.Y = dest.LowerRightCorner.Y;
    pos.UpperLeftCorner.X += 5;
    font->setShadow(video::SColor(255,0,0,0));
    
    
    font->draw(sw.c_str(), pos, time_color, false, vcenter, NULL, true /* ignore RTL */);
    font->disableShadow();
    
}   // drawTrophyPoints

//-----------------------------------------------------------------------------
/** Draws the mini map and the position of all karts on it.
 */
void RaceGUIOverworld::drawGlobalMiniMap()
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
    
    Track* t = world->getTrack();
    
    Vec3 kart_xyz;
    
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
            kart_xyz= kart->getXYZ();
            Vec3 draw_at;
            t->mapPoint2MiniMap(kart_xyz, &draw_at);
            
            core::rect<s32> source(i    *m_marker_rendered_size,
                                   0, 
                                   (i+1)*m_marker_rendered_size, 
                                   m_marker_rendered_size);
            int marker_half_size = (kart->getController()->isPlayerController() 
                                    ? m_marker_player_size 
                                    : m_marker_challenge_size                        )>>1;
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
                
                irr_driver->getVideoDriver()->draw2DImage(m_icons_frame->getTexture(), position,
                                                          rect, NULL, colors, true);
            }   // if isPlayerController
            
            irr_driver->getVideoDriver()->draw2DImage(m_marker, position, source, 
                                                      NULL, NULL, true);
        }   // for i<getNumKarts
    }   // for only_draw_player_kart
    
    
    const std::vector<OverworldChallenge>& challenges = t->getChallengeList();
    
    for (unsigned int n=0; n<challenges.size(); n++)
    {
        Vec3 draw_at;
        t->mapPoint2MiniMap(challenges[n].m_position, &draw_at);
        
        //const ChallengeData* c = unlock_manager->getChallenge(challenges[n].m_challenge_id);
       // bool locked = (m_locked_challenges.find(c) != m_locked_challenges.end());
        int state = (challenges[n].m_force_field.m_is_locked ? LOCKED : OPEN);
        
        const Challenge* c = unlock_manager->getCurrentSlot()->getChallenge(challenges[n].m_challenge_id);
        if (c->isSolved()) state = COMPLETED;
            
        const core::rect<s32> source(core::position2d<s32>(0,0),
                                     m_icons[state]->getOriginalSize());
        
        core::rect<s32> dest(m_map_left+(int)(draw_at.getX()-m_marker_challenge_size/2), 
                             lower_y   -(int)(draw_at.getY()+m_marker_challenge_size/2),
                             m_map_left+(int)(draw_at.getX()+m_marker_challenge_size/2), 
                             lower_y   -(int)(draw_at.getY()-m_marker_challenge_size/2));
        irr_driver->getVideoDriver()->draw2DImage(m_icons[state],
                                                  dest, source, NULL, NULL, true);
    }
    
    
    // ---- Draw nearby challenge if any
    for (unsigned int n=0; n<challenges.size(); n++)
    {
        if (challenges[n].m_force_field.m_is_locked) continue;

        if ((kart_xyz - Vec3(challenges[n].m_position)).length2_2d() < CHALLENGE_DISTANCE_SQUARED)
        {
            core::rect<s32> pos(15,
                                10, 
                                15 + UserConfigParams::m_width/2,
                                10 + GUIEngine::getTitleFontHeight());
            
            const ChallengeData* challenge = unlock_manager->getChallenge(challenges[n].m_challenge_id);
            
            if (challenge == NULL)
            {
                fprintf(stderr, "[RaceGUIOverworld] ERROR: Cannot find challenge <%s>\n",
                        challenges[n].m_challenge_id.c_str());
                break;
            }
            
            Track* track = track_manager->getTrack(challenge->getTrackId());
            if (track == NULL)
            {
                fprintf(stderr, "[RaceGUIOverworld] ERROR: Cannot find track <%s>, "
                        "referenced from challenge <%s>\n",
                        challenge->getTrackId().c_str(),
                        challenges[n].m_challenge_id.c_str());
                break;
            }
            
            gui::ScalableFont* font = GUIEngine::getTitleFont();
            font->draw(track->getName(), pos, video::SColor(255,255,255,255),
                       false, true /* vcenter */, NULL);
            
            
            pos.UpperLeftCorner.Y += GUIEngine::getTitleFontHeight();
            pos.LowerRightCorner.Y = UserConfigParams::m_height;
            GUIEngine::getFont()->draw(challenge->getChallengeDescription().c_str(),
                                       pos, video::SColor(255,255,255,255),
                                       false, false /* vcenter */, NULL);
            
            core::rect<s32> pos2(0,
                                 UserConfigParams::m_height - GUIEngine::getFontHeight()*2,
                                 UserConfigParams::m_width,
                                 UserConfigParams::m_height);
            GUIEngine::getFont()->draw(_("Press fire to start the challenge"), pos2,
                                       video::SColor(255,255,150,60),
                                       true, true /* vcenter */, NULL);
        }
    }
    
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
void RaceGUIOverworld::drawEnergyMeter(int x, int y, const Kart *kart,              
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

