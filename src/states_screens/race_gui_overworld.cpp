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

#include "states_screens/race_gui_overworld.hpp"

#include "challenges/challenge_status.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
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
#include "input/input.hpp"
#include "input/input_manager.hpp"
#include "items/attachment.hpp"
#include "items/attachment_manager.hpp"
#include "items/powerup_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/world.hpp"
#include "race/grand_prix_manager.hpp"
#include "race/race_manager.hpp"
#include "states_screens/race_gui_multitouch.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <ISceneCollisionManager.h>
#include <ISceneManager.h>
using namespace irr;

#include <algorithm>

const int LOCKED = 0;
const int OPEN = 1;
const int COMPLETED_EASY = 2;
const int COMPLETED_MEDIUM = 3;
const int COMPLETED_HARD = 4;

/** The constructor is called before anything is attached to the scene node.
 *  So rendering to a texture can be done here. But world is not yet fully
 *  created, so only the race manager can be accessed safely.
 */
RaceGUIOverworld::RaceGUIOverworld()
{
    m_enabled = true;

    if (UserConfigParams::m_artist_debug_mode && UserConfigParams::m_hide_gui)
        m_enabled = false;

    m_is_first_render_call = true;
    m_close_to_a_challenge = false;
    m_current_challenge = NULL;
    m_trophy1 = irr_driver->getTexture(FileManager::GUI, "cup_bronze.png");
    m_trophy2 = irr_driver->getTexture(FileManager::GUI, "cup_silver.png");
    m_trophy3 = irr_driver->getTexture(FileManager::GUI, "cup_gold.png"  );

    float scaling = irr_driver->getFrameSize().Height / 420.0f;
    const float map_size = 250.0f;
    
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
        
        if ((map_size + 20.0f) * scaling > map_bottom)
        {
            scaling = map_bottom / (map_size + 20.0f);
        }
    }

    // Marker texture has to be power-of-two for (old) OpenGL compliance
    //m_marker_rendered_size  =  2 << ((int) ceil(1.0 + log(32.0 * scaling)));
    m_minimap_challenge_size = (int)( 12.0f * scaling);
    m_minimap_player_size    = (int)( 24.0f * scaling);
    m_map_width              = (int)(map_size * scaling);
    m_map_height             = (int)(map_size * scaling);

    m_map_left   = 20;
    m_map_bottom = irr_driver->getActualScreenSize().Height-10;

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
                                                        m_map_width) * 0.9f);
        m_map_bottom = m_map_height + int(10 * scaling);
    }

    m_speed_meter_icon = material_manager->getMaterial("speedback.png");
    m_speed_bar_icon   = material_manager->getMaterial("speedfore.png");
    //createMarkerTexture();

    // Translate strings only one in constructor to avoid calling
    // gettext in each frame.
    //I18N: Shown at the end of a race
    m_string_lap      = _("Lap");
    m_string_rank     = _("Rank");

    m_active_challenge = NULL;

    // Determine maximum length of the rank/lap text, in order to
    // align those texts properly on the right side of the viewport.
    gui::ScalableFont* font = GUIEngine::getFont();
    m_trophy_points_width = font->getDimension(L"1000").Width;

    m_lock           = irr_driver->getTexture(FileManager::GUI,"gui_lock.png");
    m_open_challenge = irr_driver->getTexture(FileManager::GUI,"challenge.png");

    m_icons[0] = m_lock;
    m_icons[1] = m_open_challenge;
    m_icons[2] = m_trophy1;
    m_icons[3] = m_trophy2;
    m_icons[4] = m_trophy3;
}   // RaceGUIOverworld

//-----------------------------------------------------------------------------
RaceGUIOverworld::~RaceGUIOverworld()
{
    delete m_multitouch_gui;
}   // ~RaceGUIOverworld

//-----------------------------------------------------------------------------
/** Render all global parts of the race gui, i.e. things that are only
 *  displayed once even in splitscreen.
 *  \param dt Timestep sized.
 */
void RaceGUIOverworld::renderGlobal(float dt)
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

    // Timer etc. are not displayed unless the game is actually started.
    if(!world->isRacePhase()) return;
    if (!m_enabled) return;

    if (m_multitouch_gui == NULL)
    {
        drawTrophyPoints();
    }

    // minimap has no mipmaps so disable material2D
    //irr_driver->getVideoDriver()->enableMaterial2D(false);
    drawGlobalMiniMap();
    //irr_driver->getVideoDriver()->enableMaterial2D();

    m_is_first_render_call = false;
#endif
}   // renderGlobal

//-----------------------------------------------------------------------------
/** Render the details for a single player, i.e. speed, energy,
 *  collectibles, ...
 *  \param kart Pointer to the kart for which to render the view.
 */
void RaceGUIOverworld::renderPlayerView(const Camera *camera, float dt)
{
    if (!m_enabled) return;
    
    RaceGUIBase::renderPlayerView(camera, dt);
    
    const AbstractKart *kart = camera->getKart();
    if(!kart) return;
    
    const core::recti &viewport = camera->getViewport();
    core::vector2df scaling     = camera->getScaling();
    //Log::info("RaceGUIOverworld", "Applied ratio: %f", viewport.getWidth()/800.0f);

    scaling *= viewport.getWidth()/800.0f; // scale race GUI along screen size

    //Log::info("RaceGUIOverworld", "Scale: %f, %f", scaling.X, scaling.Y);

    drawAllMessages     (kart, viewport, scaling);

    if(!World::getWorld()->isRacePhase()) return;

    if (m_multitouch_gui == NULL)
    {
        drawPowerupIcons(kart, viewport, scaling);
    }
}   // renderPlayerView

//-----------------------------------------------------------------------------
/** Displays the number of challenge trophies
 */
void RaceGUIOverworld::drawTrophyPoints()
{
#ifndef SERVER_ONLY
    PlayerProfile *player = PlayerManager::getCurrentPlayer();
    const int points = player->getPoints();
    std::string s = StringUtils::toString(points);
    core::stringw sw(s.c_str());

    static video::SColor time_color = video::SColor(255, 255, 255, 255);

    int dist_from_right = 10 + m_trophy_points_width;

    core::rect<s32> pos(irr_driver->getActualScreenSize().Width - dist_from_right, 10,
                        irr_driver->getActualScreenSize().Width                  , 50);

    gui::ScalableFont* font = GUIEngine::getFont();

    bool vcenter = true;

    const int size = irr_driver->getActualScreenSize().Width/20;
    core::rect<s32> dest(size, pos.UpperLeftCorner.Y,
                         size*2, pos.UpperLeftCorner.Y + size);
    core::rect<s32> source(core::position2di(0, 0), m_trophy3->getSize());

    font->setShadow(video::SColor(255,0,0,0));

    if (!m_close_to_a_challenge)
    {
        draw2DImage(m_trophy1, dest, source, NULL,
                                                  NULL, true /* alpha */);
    }

    dest += core::position2di((int)(size*1.5f), 0);
    std::string easyTrophies = StringUtils::toString(player->getNumEasyTrophies());
    core::stringw easyTrophiesW(easyTrophies.c_str());
    if (!m_close_to_a_challenge)
    {
        font->draw(easyTrophiesW.c_str(), dest, time_color, false, vcenter, NULL, true /* ignore RTL */);
    }

    dest += core::position2di(size*2, 0);
    if (!m_close_to_a_challenge)
    {
        draw2DImage(m_trophy2, dest, source, NULL,
                                                  NULL, true /* alpha */);
    }

    dest += core::position2di((int)(size*1.5f), 0);
    std::string mediumTrophies = StringUtils::toString(player->getNumMediumTrophies());
    core::stringw mediumTrophiesW(mediumTrophies.c_str());
    if (!m_close_to_a_challenge)
    {
        font->draw(mediumTrophiesW.c_str(), dest, time_color, false, vcenter, NULL, true /* ignore RTL */);
    }

    dest += core::position2di(size*2, 0);
    if (!m_close_to_a_challenge)
    {
        draw2DImage(m_trophy3, dest, source, NULL,
                                                  NULL, true /* alpha */);
    }
    dest += core::position2di((int)(size*1.5f), 0);
    std::string hardTrophies = StringUtils::toString(player->getNumHardTrophies());
    core::stringw hardTrophiesW(hardTrophies.c_str());
    if (!m_close_to_a_challenge)
    {
        font->draw(hardTrophiesW.c_str(), dest, time_color, false, vcenter, NULL, true /* ignore RTL */);
    }

    dest = core::rect<s32>(pos.UpperLeftCorner.X - size - 5, pos.UpperLeftCorner.Y,
                           pos.UpperLeftCorner.X - 5, pos.UpperLeftCorner.Y + size);

    draw2DImage(m_open_challenge, dest, source, NULL,
                                              NULL, true /* alpha */);

    pos.LowerRightCorner.Y = dest.LowerRightCorner.Y;
    pos.UpperLeftCorner.X += 5;


    font->draw(sw.c_str(), pos, time_color, false, vcenter, NULL, true /* ignore RTL */);
    font->disableShadow();
#endif
}   // drawTrophyPoints

//-----------------------------------------------------------------------------
/** Draws the mini map and the position of all karts on it.
 */
void RaceGUIOverworld::drawGlobalMiniMap()
{
#ifndef SERVER_ONLY
    World *world = World::getWorld();
    // arenas currently don't have a map.
    Track* track = Track::getCurrentTrack();
    if(track->isArena() || track->isSoccer()) return;
    const std::vector<OverworldChallenge>& challenges =
                                                     track->getChallengeList();

    // The trophies might be to the left of the minimap on large displays
    // Adjust the left side of the minimap to take this into account.
    // This can't be done in the constructor of this object, since at
    // that time the scene.xml file has not been read (so the challenges
    // are not defined yet).
    if(m_is_first_render_call)
    {
        float left_most = 0;
        float right_most = 0;

        for (unsigned int n=0; n<challenges.size(); n++)
        {
            Vec3 draw_at;
            track->mapPoint2MiniMap(challenges[n].m_position, &draw_at);
            if(draw_at.getX()<left_most) left_most = draw_at.getX();
            if(draw_at.getX()>right_most) right_most = draw_at.getX();
        }

        if (m_multitouch_gui != NULL)
        {
            m_map_left += m_map_width - (int)right_most;
        }
        else
        {
            m_map_left -= (int)left_most;
        }
    }

    int upper_y = m_map_bottom - m_map_height;
    int lower_y = m_map_bottom;

    core::rect<s32> dest(m_map_left, upper_y,
                         m_map_left + m_map_width, lower_y);

    track->drawMiniMap(dest);

    Vec3 kart_xyz;

    // In the first iteration, only draw AI karts, then only draw
    // player karts. This guarantees that player kart icons are always
    // on top of AI kart icons.
    for(unsigned int only_draw_player_kart=0; only_draw_player_kart<=1;
        only_draw_player_kart++)
    {
        for(unsigned int i=0; i<world->getNumKarts(); i++)
        {
            const AbstractKart *kart = world->getKart(i);
            if(kart->isEliminated()) continue;   // don't draw eliminated kart
                                                 // Make sure to only draw AI kart icons first, then
                                                 // only player karts.
            if(kart->getController()->isLocalPlayerController()
               !=(only_draw_player_kart==1)) continue;
            kart_xyz= kart->getXYZ();
            Vec3 draw_at;
            track->mapPoint2MiniMap(kart_xyz, &draw_at);
            
            video::ITexture* icon = kart->getKartProperties()->getMinimapIcon();
            core::rect<s32> source(core::position2di(0, 0), icon->getSize());
            int marker_half_size = (kart->getController()->isLocalPlayerController()
                                    ? m_minimap_player_size
                                    : m_minimap_challenge_size                        )>>1;
            core::rect<s32> position(m_map_left+(int)(draw_at.getX()-marker_half_size),
                                     lower_y   -(int)(draw_at.getY()+marker_half_size),
                                     m_map_left+(int)(draw_at.getX()+marker_half_size),
                                     lower_y   -(int)(draw_at.getY()-marker_half_size));

            // Highlight the player icons with some backgorund image.
            if (kart->getController()->isLocalPlayerController())
            {
                video::SColor colors[4];
                for (unsigned int i=0;i<4;i++)
                {
                    colors[i]=kart->getKartProperties()->getColor();
                }
                const core::rect<s32> rect(core::position2d<s32>(0,0),
                                           m_icons_frame->getSize());

                draw2DImage(m_icons_frame, position, rect, NULL, colors, true);
            }   // if isPlayerController

            draw2DImage(icon, position, source, NULL, NULL, true);
        }   // for i<getNumKarts
    }   // for only_draw_player_kart

    m_current_challenge = NULL;
    for (unsigned int n=0; n<challenges.size(); n++)
    {
        if (challenges[n].m_challenge_id == "tutorial") continue;

        Vec3 draw_at;
        track->mapPoint2MiniMap(challenges[n].m_position, &draw_at);
        
        const ChallengeData* challenge = unlock_manager->getChallengeData(challenges[n].m_challenge_id);
        const unsigned int val = challenge->getNumTrophies();
        bool unlocked = (PlayerManager::getCurrentPlayer()->getPoints() >= val);
        if (challenges[n].m_challenge_id == "fortmagma")
        {
            // For each track, check whether any difficulty has been completed ; fortmagma will not affect our decision (`n == m`) ; tutorial is ignored because it has no completion level
            for (unsigned int m = 0; unlocked && m < challenges.size(); m++)
            {
                if (challenges[m].m_challenge_id == "tutorial") continue;
                    unlocked = unlocked &&
                        (PlayerManager::getCurrentPlayer()
                            ->getChallengeStatus(challenges[m].m_challenge_id)
                            ->isSolvedAtAnyDifficulty() || n == m);
            }
        }

        int state = (unlocked ? OPEN : LOCKED);
        
        if (UserConfigParams::m_everything_unlocked)
            state = OPEN;

        const ChallengeStatus* c = PlayerManager::getCurrentPlayer()
                                  ->getChallengeStatus(challenges[n].m_challenge_id);
        if      (c->isSolved(RaceManager::DIFFICULTY_HARD))   state = COMPLETED_HARD;
        else if (c->isSolved(RaceManager::DIFFICULTY_MEDIUM)) state = COMPLETED_MEDIUM;
        else if (c->isSolved(RaceManager::DIFFICULTY_EASY))   state = COMPLETED_EASY;

        const core::rect<s32> source(core::position2d<s32>(0,0),
                                     m_icons[state]->getSize());

        int marker_size = m_minimap_challenge_size;
        core::position2di mouse = irr_driver->getMouseLocation();
        core::rect<s32> dest(m_map_left+(int)(draw_at.getX()-marker_size/2),
                             lower_y   -(int)(draw_at.getY()+marker_size/2),
                             m_map_left+(int)(draw_at.getX()+marker_size/2),
                             lower_y   -(int)(draw_at.getY()-marker_size/2));
        if (dest.isPointInside(mouse))
        {
            marker_size = (int)(marker_size*1.6f);
            dest = core::rect<s32>(m_map_left+(int)(draw_at.getX()-marker_size/2),
                                   lower_y   -(int)(draw_at.getY()+marker_size/2),
                                   m_map_left+(int)(draw_at.getX()+marker_size/2),
                                   lower_y   -(int)(draw_at.getY()-marker_size/2));
            m_current_challenge = &(challenges[n]);
        }
        draw2DImage(m_icons[state],
                                                  dest, source, NULL, NULL, true);
    }


    // ---- Draw nearby challenge if any
    core::rect<s32> pos(15,
                        10,
                        irr_driver->getActualScreenSize().Width - 200,
                        10 + GUIEngine::getTitleFontHeight());

    m_close_to_a_challenge = false;
    for (unsigned int n=0; n<challenges.size(); n++)
    {
        if (challenges[n].m_challenge_id != "tutorial")
        {
            const ChallengeData* challenge = unlock_manager->getChallengeData(challenges[n].m_challenge_id);
            const unsigned int val = challenge->getNumTrophies();
            bool unlocked = (PlayerManager::getCurrentPlayer()->getPoints() >= val);
            
            if (UserConfigParams::m_everything_unlocked)
                unlocked = true;
                            
            if (!unlocked)
                continue;
        }

        if ((kart_xyz - Vec3(challenges[n].m_position)).length2_2d() < CHALLENGE_DISTANCE_SQUARED &&
            fabsf(kart_xyz[1] - challenges[n].m_position.Y) < CHALLENGE_HEIGHT)
        {
            m_close_to_a_challenge = true;

            if (challenges[n].m_challenge_id == "tutorial")
            {
                gui::ScalableFont* font = GUIEngine::getTitleFont();
                font->draw(_("Tutorial"), pos, video::SColor(255,255,255,255),
                           false, true /* vcenter */, NULL);

                core::rect<s32> pos2(0,
                                     irr_driver->getActualScreenSize().Height - GUIEngine::getFontHeight()*2,
                                     irr_driver->getActualScreenSize().Width,
                                     irr_driver->getActualScreenSize().Height);
                GUIEngine::getOutlineFont()->draw(_("Press fire to start the tutorial"), pos2,
                                           GUIEngine::getSkin()->getColor("font::normal"),
                                           true, true /* vcenter */, NULL);
                continue;
            }

            const ChallengeData* challenge =
                unlock_manager->getChallengeData(challenges[n].m_challenge_id);

            if (challenge == NULL)
            {
                Log::error("RaceGUIOverworld", "Cannot find challenge <%s>.",
                           challenges[n].m_challenge_id.c_str());
                break;
            }

            if (challenge->isGrandPrix())
            {
                const GrandPrixData* gp =
                    grand_prix_manager->getGrandPrix(challenge->getGPId());

                if (gp == NULL)
                {
                    Log::error("RaceGUIOverworld", "Cannot find GP <%s>, "
                               "referenced from challenge <%s>",
                               challenge->getGPId().c_str(),
                               challenges[n].m_challenge_id.c_str());
                    break;
                }

                gui::ScalableFont* font = GUIEngine::getTitleFont();
                font->draw(translations->fribidize(gp->getName()), pos, video::SColor(255,255,255,255),
                           false, true /* vcenter */, NULL);

                core::rect<s32> pos2(pos);
                pos2.UpperLeftCorner.Y += 10 + GUIEngine::getTitleFontHeight();
                pos2.LowerRightCorner.Y += 10 + GUIEngine::getTitleFontHeight();

                //just below GP name
                font->draw(_("Type: Grand Prix"), pos2, video::SColor(255,255,255,255),
                           false, true /* vcenter */, NULL);
            }
            else
            {
                Track* track = track_manager->getTrack(challenge->getTrackId());
                if (track == NULL)
                {
                    Log::error("RaceGUIOverworld", "Cannot find track <%s>, "
                               "referenced from challenge <%s>",
                               challenge->getTrackId().c_str(),
                               challenges[n].m_challenge_id.c_str());
                    break;
                }

                gui::ScalableFont* font = GUIEngine::getTitleFont();
                font->draw(translations->fribidize(track->getName()),
                           pos, video::SColor(255, 255, 255, 255),
                           false, true /* vcenter */, NULL);
            }

            pos.UpperLeftCorner.Y += GUIEngine::getTitleFontHeight();
            pos.LowerRightCorner.Y = irr_driver->getActualScreenSize().Height;

            if (m_active_challenge != challenge)
            {
                m_active_challenge = challenge;
                m_challenge_description = challenge->getChallengeDescription();
            }
            GUIEngine::getFont()->draw(m_challenge_description,
                                       pos, video::SColor(255,255,255,255),
                                       false, false /* vcenter */, NULL);

            core::rect<s32> pos2(0,
                                 irr_driver->getActualScreenSize().Height - GUIEngine::getFontHeight()*2,
                                 irr_driver->getActualScreenSize().Width,
                                 irr_driver->getActualScreenSize().Height);
            GUIEngine::getOutlineFont()->draw(_("Press fire to start the challenge"), pos2,
                                       GUIEngine::getSkin()->getColor("font::normal"),
                                       true, true /* vcenter */, NULL);
        }
    }
    
    if (m_multitouch_gui != NULL)
    {
        m_multitouch_gui->setGuiAction(m_close_to_a_challenge);
    }
#endif   // SERVER_ONLY
}   // drawGlobalMiniMap

//-----------------------------------------------------------------------------

