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

#include "audio/music_manager.hpp"
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
    m_map_right_side_x = 0;
    
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
    m_minimap_on_left       = true;
    
    // Minimap is also rendered bigger via OpenGL, so find power-of-two again
    const int map_texture   = 2 << ((int) ceil(1.0 + log(128.0 * scaling)));
    m_map_rendered_width    = map_texture;
    m_map_rendered_height   = map_texture;

    m_max_font_height       = GUIEngine::getFontHeight() + 10;
    m_small_font_max_height = GUIEngine::getSmallFontHeight() + 5;

    // special case : when 3 players play, use available 4th space for such things
    if (race_manager->getNumLocalPlayers() == 3)
    {
        m_map_left = UserConfigParams::m_width - m_map_width;
        m_minimap_on_left = false;
    }

    m_speed_meter_icon = material_manager->getMaterial("speedback.png");
    m_speed_bar_icon   = material_manager->getMaterial("speedfore.png");
    m_plunger_face     = material_manager->getMaterial("plungerface.png");
    m_music_icon       = material_manager->getMaterial("notes.png");
    createMarkerTexture();

    // Translate strings only one in constructor to avoid calling
    // gettext in each frame.
    //I18N: Shown at the end of a race
    m_string_finished = _("Finished");
    m_string_lap      = _("Lap");
    m_string_rank     = _("Rank");
    
    //I18N: When some GlobalPlayerIcons are hidden, write "Top 10" to show it
    m_string_top      = _("Top %i");
    //I18N: as in "ready, set, go", shown at the beginning of the race
    m_string_ready    = _("Ready!");
    m_string_set      = _("Set!");
    m_string_go       = _("Go!");
    
    m_dist_show_overlap=2;
    m_icons_inertia=2;
    
    //read icon frame picture
    m_icons_frame=material_manager->getMaterial("icons-frame.png");
     
}   // RaceGUI

//-----------------------------------------------------------------------------
RaceGUI::~RaceGUI()
{
    irr_driver->removeTexture(m_marker);
}   // ~Racegui

//-----------------------------------------------------------------------------
/** Creates a texture with the markers for all karts in the current race
 *  on it. This assumes that nothing is attached to the scene node at
 *  this stage.
 */
void RaceGUI::createMarkerTexture()
{
    unsigned int num_karts = race_manager->getNumberOfKarts();
    unsigned int npower2   = 1;
    // Textures must be power of 2, so 
    while(npower2<num_karts) npower2*=2;

    int radius     = (m_marker_rendered_size>>1)-1;
    IrrDriver::RTTProvider rttProvider(core::dimension2du(m_marker_rendered_size
                                                          *npower2,
                                                          m_marker_rendered_size),
                                     "RaceGUI::markers");
    scene::ICameraSceneNode *camera = irr_driver->addCameraSceneNode();
    core::matrix4 projection;
    projection.buildProjectionMatrixOrthoLH((float)(m_marker_rendered_size*npower2), 
                                            (float)(m_marker_rendered_size), 
                                            -1.0f, 1.0f);
    camera->setProjectionMatrix(projection, true);
    core::vector3df center( (float)(m_marker_rendered_size*npower2>>1),
                            (float)(m_marker_rendered_size>>1), 0.0f);
    camera->setPosition(center);
    camera->setUpVector(core::vector3df(0,1,0));
    camera->setTarget(center + core::vector3df(0,0,4));
    // The call to render sets the projection matrix etc. So we have to call 
    // this now before doing the direct OpenGL calls.
    // FIXME: perhaps we should use three calls to irr_driver: begin(),
    // render(), end() - so we could do the rendering by calling to
    // draw2DPolygon() between render() and end(), avoiding the
    // call to camera->render()
    camera->render();
    // We have to reset the material here, since otherwise the last
    // set material (i.e from the kart selection screen) will be used
    // when rednering to the texture.
    video::SMaterial m;
    m.setTexture(0, NULL);
    irr_driver->getVideoDriver()->setMaterial(m);
    for(unsigned int i=0; i<num_karts; i++)
    {
        const std::string& kart_ident = race_manager->getKartIdent(i);
        assert(kart_ident.size() > 0);
        
        const KartProperties *kp=kart_properties_manager->getKart(kart_ident);
        assert(kp != NULL);
        
        core::vector2df center((float)((m_marker_rendered_size>>1)
                                +i*m_marker_rendered_size), 
                               (float)(m_marker_rendered_size>>1)  );
        int count = kp->getShape();
        video::ITexture *t = kp->getMinimapIcon();
        if(t)
        {
            video::ITexture *t = kp->getIconMaterial()->getTexture();
            core::recti dest_rect(i*m_marker_rendered_size, 
                                  0,
                                  (i+1)*m_marker_rendered_size,
                                  m_marker_rendered_size);
            core::recti source_rect(core::vector2di(0,0), t->getSize());
            irr_driver->getVideoDriver()->draw2DImage(t, dest_rect, 
                                                      source_rect,
                                                      /*clipRect*/0,
                                                      /*color*/   0,
                                                      /*useAlpha*/true);
        }
        else   // no special minimap icon defined
        {
            video::S3DVertex *vertices = new video::S3DVertex[count+1];
            unsigned short int *index  = new unsigned short int[count+1];
            video::SColor color        = kp->getColor();
            createRegularPolygon(count, (float)radius, center, color, 
                vertices, index);
            irr_driver->getVideoDriver()->draw2DVertexPrimitiveList(vertices, 
                                            count, index, count-2,
                                            video::EVT_STANDARD, 
                                            scene::EPT_TRIANGLE_FAN);
            delete [] vertices;
            delete [] index;
        }   // if special minimap icon defined
    }

    m_marker = rttProvider.renderToTexture(-1, /*is_2d_render*/true);
    irr_driver->removeCameraSceneNode(camera);
}   // createMarkerTexture

//-----------------------------------------------------------------------------
/** Creates the 2D vertices for a regular polygon. Adopted from Irrlicht.
 *  \param n Number of vertices to use.
 *  \param radius Radius of the polygon.
 *  \param center The center point of the polygon.
 *  \param v Pointer to the array of vertices.
 */
void RaceGUI::createRegularPolygon(unsigned int n, float radius, 
                                   const core::vector2df &center,
                                   const video::SColor &color,
                                   video::S3DVertex *v, 
                                   unsigned short int *index)
{
    float f = 2*M_PI/(float)n;
    for (unsigned int i=0; i<n; i++)
    {
        float p = i*f;
        core::vector2df X = center + core::vector2df( sin(p)*radius, 
                                                     -cos(p)*radius);
        v[i].Pos.X = X.X;
        v[i].Pos.Y = X.Y;
        v[i].Color = color;
        index[i]   = i;
    }
}   // createRegularPolygon


//-----------------------------------------------------------------------------
/** Render all global parts of the race gui, i.e. things that are only 
 *  displayed once even in splitscreen.
 *  \param dt Timestep sized.
 */
void RaceGUI::renderGlobal(float dt)
{
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

    drawGlobalTimer();
    if(world->getPhase() == WorldStatus::GO_PHASE ||
       world->getPhase() == WorldStatus::MUSIC_PHASE)
    {
        drawGlobalMusicDescription();
    }

    drawGlobalMiniMap();
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
    drawEnergyMeter     (kart, viewport, scaling);
    drawSpeed           (kart, viewport, scaling);
    drawLap             (info, kart, viewport);
    drawRank            (info, kart, viewport);

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
    core::rect<s32> pos(UserConfigParams::m_width-120, 10, 
                        UserConfigParams::m_width,     50);
    
    // special case : when 3 players play, use available 4th space for such things
    if (race_manager->getNumLocalPlayers() == 3)
    {
        pos += core::vector2d<s32>(0, UserConfigParams::m_height/2);
    }
    
    gui::ScalableFont* font = GUIEngine::getFont();
    font->draw(sw.c_str(), pos, time_color);
}   // DRAWGLOBALTimer

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
    m_map_right_side_x = dest.LowerRightCorner.X;

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
    float y_icons_limit=UserConfigParams::m_height-m_map_height-ICON_PLAYER_WIDTH;
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
            font->draw(s.c_str(), pos, color);
        }

        if (info[kart_id].special_title.size() > 0)
        {
            static video::SColor color = video::SColor(255, 255, 0, 0);
            core::rect<s32> pos(x+ICON_PLAYER_WIDTH, y+5, 
                x+ICON_PLAYER_WIDTH, y+5);
            core::stringw s(info[kart_id].special_title.c_str());
            font->draw(s.c_str(), pos, color);
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
void RaceGUI::drawPowerupIcons(const Kart* kart, 
                               const core::recti &viewport, 
                               const core::vector2df &scaling)
{
    // If player doesn't have anything, do nothing.
    const Powerup* powerup = kart->getPowerup();
    if(powerup->getType() == PowerupManager::POWERUP_NOTHING) return;
    int n  = kart->getNumPowerup() ;
    if(n<1) return;    // shouldn't happen, but just in case
    if(n>5) n=5;       // Display at most 5 items

    int nSize=(int)(64.0f*std::min(scaling.X, scaling.Y));
        
    int itemSpacing = (int)(std::min(scaling.X, scaling.Y)*30);
    
    int x1 = viewport.UpperLeftCorner.X  + viewport.getWidth()/2 
           - (n * itemSpacing)/2;
    int y1 = viewport.UpperLeftCorner.Y  + (int)(20 * scaling.Y);

    assert(powerup != NULL);
    assert(powerup->getIcon() != NULL);
    video::ITexture *t=powerup->getIcon()->getTexture();
    assert(t != NULL);
    core::rect<s32> rect(core::position2di(0, 0), t->getOriginalSize());
    
    for ( int i = 0 ; i < n ; i++ )
    {
        int x2=(int)(x1+i*itemSpacing);
        core::rect<s32> pos(x2, y1, x2+nSize, y1+nSize);
        irr_driver->getVideoDriver()->draw2DImage(t, pos, rect, NULL, 
                                                  NULL, true);
    }   // for i
}   // drawPowerupIcons

//-----------------------------------------------------------------------------
/* Energy meter that gets filled with coins */
void RaceGUI::drawEnergyMeter (const Kart *kart,              
                               const core::recti &viewport, 
                               const core::vector2df &scaling)
{
    float state = (float)(kart->getEnergy()) / MAX_ITEMS_COLLECTED;
    //int y = (int)(250 * scaling.Y) + viewport.UpperLeftCorner.Y;
    int w = (int)(16 * scaling.X);
    int h = (int)(UserConfigParams::m_height/4 * scaling.Y);
    
    int x = viewport.LowerRightCorner.X - w - 5;
    int y = viewport.LowerRightCorner.Y -  (int)(250 * scaling.Y);

    float coin_target = (float)race_manager->getCoinTarget();
    int th = (int)(h*(coin_target/MAX_ITEMS_COLLECTED));
    
    video::SColor black_border(255, 0, 0, 0);
    video::SColor white_border(255, 255, 255, 255);
    video::IVideoDriver *video = irr_driver->getVideoDriver();
#define LINE(x0,y0,x1,y1, color) video->draw2DLine(core::position2di(x0,y0), \
                                                   core::position2di(x1,y1), color)

    // Left side:
    LINE(x-1,   y+1,   x-1,   y-h-1, black_border);
    LINE(x,     y,     x,     y-h-2, white_border);
 
    // Right side:
    LINE(x+w,   y+1,   x+w,   y-h-1, black_border);
    LINE(x+w+1, y,     x+w+1, y-h-2, white_border);
 
    // Bottom
    LINE(x,     y+1,   x+w,   y+1,   black_border);
    LINE(x+1,   y,     x+w+1, y,     white_border);
 
    // Top
    LINE(x,     y-h,   x+w,   y-h,   black_border);
    LINE(x,     y-h-1, x+w,   y-h-1, white_border);

    const int GRADS = (int)(MAX_ITEMS_COLLECTED/5);  // each graduation equals 5 items
    int gh = (int)(h/GRADS);  //graduation height

    // 'Meter marks;
    int gh_incr = gh;
    for (int i=0; i<GRADS-1; i++)
    {
        LINE(x+1, y-1-gh, x+1+w, y-1-gh, white_border);
        gh+=gh_incr;
    }

    //Target line
    if (coin_target > 0)
    {
        LINE(x+1, y-1-th, x+1+w, y-1-th, video::SColor(255, 255, 0, 0));
    }
#undef LINE

    // The actual energy meter
    core::rect<s32> energy(x+1, y-1-(int)(state*h), x+1+w, y-1);
    video::SColor bottom(255, 240, 0,   0);
    video::SColor top   (160, 240, 200, 0);
    video->draw2DRectangle(energy, top, top, bottom, bottom);
}   // drawEnergyMeter

//-----------------------------------------------------------------------------
void RaceGUI::drawSpeed(const Kart* kart, const core::recti &viewport, 
                        const core::vector2df &scaling)
{

    float minRatio         = std::min(scaling.X, scaling.Y);
    const int SPEEDWIDTH   = 128;
    int meter_width        = (int)(SPEEDWIDTH*minRatio);
    int meter_height       = (int)(SPEEDWIDTH*minRatio);
    core::vector2df offset;
    offset.X = (float)(viewport.LowerRightCorner.X-meter_width) - 10.0f*scaling.X;
    offset.Y = viewport.LowerRightCorner.Y-10*scaling.Y;
    
    // First draw the meter (i.e. the background which contains the numbers etc.
    // -------------------------------------------------------------------------
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
    if(speed_ratio>1 || !kart->isOnGround()) speed_ratio = 1;

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
void RaceGUI::drawLap(const KartIconDisplayInfo* info, const Kart* kart, 
                      const core::recti &viewport)
{
    // Don't display laps in follow the leader mode
    if(!World::getWorld()->raceHasLaps()) return;
    
    const int lap = info[kart->getWorldKartId()].lap;
    
    if(lap<0) return;  // don't display 'lap 0/...'

    core::recti pos;
    pos.UpperLeftCorner.Y  = viewport.LowerRightCorner.Y;

    // place lap count somewhere on the left of the screen
    if (m_minimap_on_left)
    {
        // check if mini-map is within Y coords of this player.
        // if the mini-map is not even in the viewport of this player, don't 
        // bother placing the lap text at the right of the minimap.
        if (UserConfigParams::m_height - m_map_bottom - m_map_height 
            > viewport.LowerRightCorner.Y)
        {
            pos.UpperLeftCorner.X  = viewport.UpperLeftCorner.X 
                                   + (int)(0.1f*UserConfigParams::m_width);
        }
        else
        {
            // place lap text at the right of the mini-map
            const int calculated_x = viewport.UpperLeftCorner.X 
                                   + (int)(0.05f*UserConfigParams::m_width);
            // don't overlap minimap
            pos.UpperLeftCorner.X  = std::max(calculated_x, 
                                              m_map_right_side_x + 15);
        }
    }
    else
    {
        // mini-map is on the right, and lap text on right,
        // so no overlap possible
        pos.UpperLeftCorner.X  = viewport.UpperLeftCorner.X 
                               + (int)(0.05f*UserConfigParams::m_width);
    }
    
    gui::ScalableFont* font = GUIEngine::getFont(); 
    int font_height         = (int)(font->getDimension(L"X").Height);
    
    if (kart->hasFinishedRace())
    {
        static video::SColor color = video::SColor(255, 255, 255, 255);
        pos.UpperLeftCorner.Y -= 2*font_height;
        pos.LowerRightCorner   = pos.UpperLeftCorner;
        font->draw(m_string_finished.c_str(), pos, color);
    }
    else
    {
        static video::SColor color = video::SColor(255, 255, 255, 255);
        pos.UpperLeftCorner.Y -= 3*font_height;
        pos.LowerRightCorner   = pos.UpperLeftCorner;
        font->draw(m_string_lap.c_str(), pos, color);
    
        char str[256];
        sprintf(str, "%d/%d", lap+1, race_manager->getNumLaps());
        pos.UpperLeftCorner.Y  += font_height;
        pos.LowerRightCorner.Y += font_height;
        font->draw(core::stringw(str).c_str(), pos, color);
    }
} // drawLap

//-----------------------------------------------------------------------------
void RaceGUI::drawRank(const KartIconDisplayInfo* info, const Kart* kart, 
                      const core::recti &viewport)
{
    WorldWithRank *world    = (WorldWithRank*)(World::getWorld());
    const int rank = kart->getPosition();
    const unsigned int kart_amount = world->getNumKarts();
    
    core::recti pos;
    pos.UpperLeftCorner.Y  = viewport.LowerRightCorner.Y;

    // place rank string somewhere on the left of the screen
    if (m_minimap_on_left)
    {
        // check if mini-map is within Y coords of this player.
        // if the mini-map is not even in the viewport of this player, don't 
        // bother placing the lap text at the right of the minimap.
        if (UserConfigParams::m_height - m_map_bottom - m_map_height 
            > viewport.LowerRightCorner.Y)
        {
            pos.UpperLeftCorner.X  = viewport.UpperLeftCorner.X 
                                   + (int)(0.1f*UserConfigParams::m_width);
        }
        else
        {
            // place lap text at the right of the mini-map
            const int calculated_x = viewport.UpperLeftCorner.X 
                                   + (int)(0.05f*UserConfigParams::m_width);
            // don't overlap minimap
            pos.UpperLeftCorner.X  = std::max(calculated_x, 
                                              m_map_right_side_x + 15);
        }
    }
    else
    {
        // mini-map is on the right, and lap text on right,
        // so no overlap possible
        pos.UpperLeftCorner.X  = viewport.UpperLeftCorner.X 
                               + (int)(0.05f*UserConfigParams::m_width);
    }
    
    gui::ScalableFont* font = GUIEngine::getFont(); 
    int font_height         = (int)(font->getDimension(L"X").Height);
    
    if (kart->hasFinishedRace())
    {
        static video::SColor color = video::SColor(255, 255, 255, 255);
        pos.UpperLeftCorner.Y -= (2+2)*font_height;
        pos.LowerRightCorner   = pos.UpperLeftCorner;
        font->draw(m_string_finished.c_str(), pos, color);
    }
    else
    {
        static video::SColor color = video::SColor(255, 255, 255, 255);
        pos.UpperLeftCorner.Y -= (3+2)*font_height;
        pos.LowerRightCorner   = pos.UpperLeftCorner;
        font->draw(m_string_rank.c_str(), pos, color);
    
        char str[256];
        sprintf(str, "%d/%d", rank, kart_amount);
        pos.UpperLeftCorner.Y  += font_height;
        pos.LowerRightCorner.Y += font_height;
        font->draw(core::stringw(str).c_str(), pos, color);
    }
} // drawRank

//-----------------------------------------------------------------------------
/** Removes messages which have been displayed long enough. This function
 *  must be called after drawAllMessages, otherwise messages which are only
 *  displayed once will not be drawn!
 **/

void RaceGUI::cleanupMessages(const float dt)
{
    AllMessageType::iterator p =m_messages.begin(); 
    while(p!=m_messages.end())
    {
        if((*p).done(dt))
        {
            p = m_messages.erase(p);
        }
        else
        {
            ++p;
        }
    }
}   // cleanupMessages

//-----------------------------------------------------------------------------
/** Displays all messages in the message queue
 **/
void RaceGUI::drawAllMessages(const Kart* kart,
                              const core::recti &viewport, 
                              const core::vector2df &scaling)
{    
    int y = viewport.LowerRightCorner.Y - m_small_font_max_height - 10;
          
    const int x = (viewport.LowerRightCorner.X - viewport.UpperLeftCorner.X)/2;
    const int w = (viewport.LowerRightCorner.X + viewport.UpperLeftCorner.X)/2;
    
    // draw less important first, at the very top of the screen
    for (AllMessageType::const_iterator i = m_messages.begin(); 
         i != m_messages.end(); ++i)
    {
        TimedMessage const &msg = *i;
        if (!msg.m_important)
        {
            // Display only messages for all karts, or messages for this kart
            if (msg.m_kart && msg.m_kart!=kart) continue;
            
            core::rect<s32> pos(x - w/2, y, x + w/2, y + m_max_font_height);
            GUIEngine::getSmallFont()->draw(
                core::stringw(msg.m_message.c_str()).c_str(),
                pos, msg.m_color, true /* hcenter */, true /* vcenter */);
            y -= m_small_font_max_height;                    
        }
    }
    
    // First line of text somewhat under the top of the viewport.
    y = (int)(viewport.UpperLeftCorner.Y + 164*scaling.Y);

    // The message are displayed in reverse order, so that a multi-line
    // message (addMessage("1", ...); addMessage("2",...) is displayed
    // in the right order: "1" on top of "2"
    for (AllMessageType::const_iterator i = m_messages.begin(); 
         i != m_messages.end(); ++i)
    {
        TimedMessage const &msg = *i;

         // less important messages were already displayed
        if (!msg.m_important) continue;
        
        // Display only messages for all karts, or messages for this kart
        if (msg.m_kart && msg.m_kart!=kart) continue;

        core::rect<s32> pos(x - w/2, y, x + w/2, y + m_max_font_height);
        GUIEngine::getFont()->draw(core::stringw(msg.m_message.c_str()).c_str(),
                                   pos, msg.m_color, true /* hcenter */, 
                                   true /* vcenter */);
        y += m_max_font_height;        
    }   // for i in all messages
}   // drawAllMessages

//-----------------------------------------------------------------------------
/** Adds a message to the message queue. The message is displayed for a
 *  certain amount of time (unless time<0, then the message is displayed
 *  once).
 **/
void RaceGUI::addMessage(const core::stringw &msg, const Kart *kart, 
                         float time, int font_size, 
                         const video::SColor &color, bool important)
{
    m_messages.push_back(TimedMessage(msg, kart, time, font_size, color,
                                      important));
}   // addMessage

//-----------------------------------------------------------------------------
// Displays the description given for the music currently being played -
// usually the title and composer.
void RaceGUI::drawGlobalMusicDescription()
{
     // show no music description when it's off
    if (!UserConfigParams::m_music) return;
    
    gui::IGUIFont*       font = GUIEngine::getFont();

    float race_time = World::getWorld()->getTime();
    // ---- Manage pulsing effect
    // 3.0 is the duration of ready/set (TODO: don't hardcode)
    float timeProgression = (float)(race_time) /
                            (float)(stk_config->m_music_credit_time - 2.0f);
    
    const int x_pulse = (int)(sin(race_time*9.0f)*10.0f);
    const int y_pulse = (int)(cos(race_time*9.0f)*10.0f);
    
    float resize = 1.0f;
    if (timeProgression < 0.1)
    {
        resize = timeProgression/0.1f;
    }
    else if (timeProgression > 0.9)
    {
        resize = 1.0f - (timeProgression - 0.9f)/0.1f;
    }
    
    const float resize3 = resize*resize*resize;
    
    // Get song name, and calculate its size, allowing us to position stuff
    const MusicInformation* mi = music_manager->getCurrentMusic();
    if (!mi) return;
    
    std::string s="\""+mi->getTitle()+"\"";
    core::stringw thetext(s.c_str());
    
    core::dimension2d< u32 > textSize = font->getDimension(thetext.c_str());
    int textWidth = textSize.Width;
    
    int textWidth2 = 0;
    core::stringw thetext_composer;
    if (mi->getComposer()!="")
    {
        std::string s = "by "+mi->getComposer();
        thetext_composer = s.c_str();
        textWidth2 = font->getDimension(thetext_composer.c_str()).Width;
    }
    const int max_text_size = (int)(UserConfigParams::m_width*2.0f/3.0f);
    if (textWidth  > max_text_size) textWidth  = max_text_size;
    if (textWidth2 > max_text_size) textWidth2 = max_text_size;

    const int ICON_SIZE = 64;
    const int y         = UserConfigParams::m_height - 80;
    // the 20 is an arbitrary space left between the note icon and the text
    const int noteX     = (UserConfigParams::m_width / 2) 
                        - std::max(textWidth, textWidth2)/2 - ICON_SIZE/2 - 20;
    const int noteY     = y;
    // the 20 is an arbitrary space left between the note icon and the text
    const int textXFrom = (UserConfigParams::m_width / 2) 
                        - std::max(textWidth, textWidth2)/2 + 20;
    const int textXTo   = (UserConfigParams::m_width / 2) 
                        + std::max(textWidth, textWidth2)/2 + 20;

    // ---- Draw "by" text
    const int text_y = (int)(UserConfigParams::m_height - 80*(resize3) 
                     + 40*(1-resize));
    
    static const video::SColor white = video::SColor(255, 255, 255, 255);
    if(mi->getComposer()!="")
    {
        core::rect<s32> pos_by(textXFrom, text_y+40,
                               textXTo,   text_y+40);
        std::string s="by "+mi->getComposer();
        font->draw(core::stringw(s.c_str()).c_str(), pos_by, white, 
                   true, true);
    }
    
    // ---- Draw "song name" text
    core::rect<s32> pos(textXFrom, text_y,
                        textXTo,   text_y);
    
    font->draw(thetext.c_str(), pos, white, true /* hcenter */, 
               true /* vcenter */);
 
    // Draw music icon
    int iconSizeX = (int)(ICON_SIZE*resize + x_pulse*resize*resize);
    int iconSizeY = (int)(ICON_SIZE*resize + y_pulse*resize*resize);
    
    video::ITexture *t = m_music_icon->getTexture();
    core::rect<s32> dest(noteX-iconSizeX/2+20,    
                         noteY-iconSizeY/2+ICON_SIZE/2,
                         noteX+iconSizeX/2+20,
                         noteY+iconSizeY/2+ICON_SIZE/2);
    const core::rect<s32> source(core::position2d<s32>(0,0), 
                                 t->getOriginalSize());
    
    irr_driver->getVideoDriver()->draw2DImage(t, dest, source,
                                              NULL, NULL, true);
}   // drawGlobalMusicDescription

// ----------------------------------------------------------------------------
/** Draws the ready-set-go message on the screen.
 */
void RaceGUI::drawGlobalReadySetGo()
{
    switch (World::getWorld()->getPhase())
    {
    case WorldStatus::READY_PHASE:
        {
            static video::SColor color = video::SColor(255, 255, 255, 255);
            core::rect<s32> pos(UserConfigParams::m_width>>1, 
                                UserConfigParams::m_height>>1,
                                UserConfigParams::m_width>>1,
                                UserConfigParams::m_height>>1);
            gui::IGUIFont* font = GUIEngine::getTitleFont();
            font->draw(m_string_ready.c_str(), pos, color, true, true);
        }
        break;
    case WorldStatus::SET_PHASE:
        {
            static video::SColor color = video::SColor(255, 255, 255, 255);
            core::rect<s32> pos(UserConfigParams::m_width>>1, 
                                UserConfigParams::m_height>>1,
                                UserConfigParams::m_width>>1, 
                                UserConfigParams::m_height>>1);
            gui::IGUIFont* font = GUIEngine::getTitleFont();
            //I18N: as in "ready, set, go", shown at the beginning of the race
            font->draw(m_string_set.c_str(), pos, color, true, true);
        }
        break;
    case WorldStatus::GO_PHASE:
        {
            static video::SColor color = video::SColor(255, 255, 255, 255);
            core::rect<s32> pos(UserConfigParams::m_width>>1, 
                                UserConfigParams::m_height>>1,
                                UserConfigParams::m_width>>1, 
                                UserConfigParams::m_height>>1);
            //gui::IGUIFont* font = irr_driver->getRaceFont();
            gui::IGUIFont* font = GUIEngine::getTitleFont();
            //I18N: as in "ready, set, go", shown at the beginning of the race
            font->draw(m_string_go.c_str(), pos, color, true, true);
        }
        break;
    default: 
         break;
    }   // switch
}   // drawGlobalReadySetGo
