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

#include "audio/sound_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/camera.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "io/file_manager.hpp"
#include "input/input.hpp"
#include "input/input_manager.hpp"
#include "karts/kart_properties_manager.hpp"
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
    m_marker_rendered_size =  32;
    m_marker_ai_size       =  14;
    m_marker_player_size   =  16;
    m_map_rendered_width   = 128;
    m_map_rendered_height  = 128;
    m_map_width            = 100;
    m_map_height           = 100;
    m_map_left             =  10;
    m_map_bottom           =  10;

    m_speed_meter_icon = material_manager->getMaterial("speedback.png");
    m_speed_bar_icon   = material_manager->getMaterial("speedfore.png");    
    m_plunger_face     = material_manager->getMaterial("plungerface.png");
    m_music_icon       = material_manager->getMaterial("notes.png");
    createMarkerTexture();
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
    unsigned int n=race_manager->getNumKarts();
    unsigned int npower2 = 1;
    // Textures must be power of 2, so 
    while(npower2<n) npower2*=2;

    int radius     = (m_marker_rendered_size>>1)-1;
#if IRRLICHT_VERSION_MAJOR > 1 || IRRLICHT_VERSION_MINOR >= 6
    IrrDriver::RTTProvider rttProvider(core::dimension2du(m_marker_rendered_size * npower2, 
                                                          m_marker_rendered_size), 
                                     "RaceGUI::markers");
#else
    IrrDriver::RTTProvider rttProvider(core::dimension2di(m_marker_rendered_size * npower2, 
                                                          m_marker_rendered_size), 
                                     "RaceGUI::markers");
#endif
    scene::ICameraSceneNode *camera = irr_driver->addCameraSceneNode();
    core::matrix4 projection;
    projection.buildProjectionMatrixOrthoLH((float)(m_marker_rendered_size*npower2), 
                                            (float)(m_marker_rendered_size), -1.0f, 1.0f);
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
    for(unsigned int i=0; i<race_manager->getNumKarts(); i++)
    {
        const std::string& kart_ident = race_manager->getKartIdent(i);
        const KartProperties *kp = kart_properties_manager->getKart(kart_ident);
        core::vector2df center((float)((m_marker_rendered_size>>1)+i*m_marker_rendered_size), 
                               (float)(m_marker_rendered_size>>1)                   );
        int count = kp->getShape();
        //core::array<core::vector2df> vertices;
        video::S3DVertex *vertices = new video::S3DVertex[count+1];
        unsigned short int *index                 = new unsigned short int[count+1];
        video::SColor color        = kp->getColor();
        createRegularPolygon(count, (float)radius, center, color, 
                             vertices, index);

        irr_driver->getVideoDriver()->draw2DVertexPrimitiveList(vertices, count,
            index, count-2, video::EVT_STANDARD, scene::EPT_TRIANGLE_FAN);
        delete vertices;
        delete index;
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
                                   video::S3DVertex *v, unsigned short int *index)
{
    float f = 2*M_PI/(float)n;
    for (unsigned int i=0; i<n; i++)
    {
        float p = i*f;
        core::vector2df X = center + core::vector2df(sin(p)*radius, -cos(p)*radius);
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

    assert(RaceManager::getWorld() != NULL);
    if(RaceManager::getWorld()->getPhase() >= READY_PHASE &&
       RaceManager::getWorld()->getPhase() <= GO_PHASE      )
    {
        drawGlobalReadySetGo();
    }

    // The penalty message needs to be displayed for up to one second
    // after the start of the race, otherwise it disappears if 
    // "Go" is displayed and the race starts
    if(RaceManager::getWorld()->isStartPhase() || 
       RaceManager::getWorld()->getTime()<1.0f)
    {
        displayPenaltyMessages();
    }
    // Timer etc. are not displayed unless the game is actually started.
    if(!RaceManager::getWorld()->isRacePhase()) return;

    drawGlobalTimer();
    if(RaceManager::getWorld()->getPhase() == GO_PHASE ||
        RaceManager::getWorld()->getPhase() == MUSIC_PHASE)
    {
        drawGlobalMusicDescription();
    }

    drawGlobalMiniMap();
    RaceGUI::KartIconDisplayInfo* info = RaceManager::getWorld()->getKartsDisplayInfo();
    drawGlobalPlayerIcons(info);
}   // renderGlobal

//-----------------------------------------------------------------------------
/** Render the details for a single player, i.e. speed, energy, 
 *  collectibles, ...
 *  \param player_id  Player id.
 *  \param viewport Viewport to use (already set in the camera).
 *  \param scaling Scaling to use.
 */
void RaceGUI::renderPlayerView(unsigned int player_id, 
                               const core::recti &viewport, 
                               const core::vector2df &scaling)
{
    if(!RaceManager::getWorld()->isRacePhase()) return;

    RaceGUI::KartIconDisplayInfo* info = RaceManager::getWorld()->getKartsDisplayInfo();

    Kart* player_kart = RaceManager::getWorld()->getLocalPlayerKart(player_id);
    drawPowerupIcons    (player_kart, viewport, scaling);
    drawEnergyMeter     (player_kart, viewport, scaling);
    drawSpeed           (player_kart, viewport, scaling);
    drawLap             (info, player_kart, viewport, scaling);
    drawAllMessages     (player_kart, viewport, scaling);
    if(player_kart->hasViewBlockedByPlunger())
    {
        int offset_x = viewport.UpperLeftCorner.X;
        int offset_y = viewport.UpperLeftCorner.Y;
        float split_screen_ratio_x = scaling.X;
        float split_screen_ratio_y = scaling.Y;

        const int screen_width = viewport.LowerRightCorner.X-viewport.UpperLeftCorner.X;
        const int plunger_size = viewport.UpperLeftCorner.Y-viewport.LowerRightCorner.Y;
        int plunger_x = viewport.UpperLeftCorner.X + screen_width/2 - plunger_size/2;

        video::ITexture *t=m_plunger_face->getTexture();
        core::rect<s32> dest(plunger_x,              offset_y, 
                             plunger_x+plunger_size, offset_y+plunger_size);
        const core::rect<s32> source(core::position2d<s32>(0,0), t->getOriginalSize());

        static const video::SColor white = video::SColor(255, 255, 255, 255);

        irr_driver->getVideoDriver()->draw2DImage(t, dest, source, 0, 
                                                  &white, true);
    }
}   // renderPlayerView

//-----------------------------------------------------------------------------
/** Displays the racing time on the screen.s
 */
void RaceGUI::drawGlobalTimer()
{
    assert(RaceManager::getWorld() != NULL);
    
    if(!RaceManager::getWorld()->shouldDrawTimer()) return;
    std::string s = StringUtils::timeToString(RaceManager::getWorld()->getTime());
    core::stringw sw(s.c_str());

    static video::SColor time_color = video::SColor(255, 255, 255, 255);
    core::rect<s32> pos(UserConfigParams::m_width-120, 10, 
                        UserConfigParams::m_width,     50);
    gui::IGUIFont* font = irr_driver->getRaceFont();
    font->draw(sw.c_str(), pos, time_color);
}   // DRAWGLOBALTimer

//-----------------------------------------------------------------------------
/** Draws the mini map and the position of all karts on it.
 */
void RaceGUI::drawGlobalMiniMap()
{
    // arenas currently don't have a map.
    if(RaceManager::getTrack()->isArena()) return;

    const video::ITexture *mini_map=RaceManager::getTrack()->getMiniMap();
    
    int upper_y = UserConfigParams::m_height-m_map_bottom-m_map_height;
    int lower_y = UserConfigParams::m_height-m_map_bottom;
    core::rect<s32> dest(m_map_left,               upper_y, 
                         m_map_left + m_map_width, lower_y);
    core::rect<s32> source(core::position2di(0, 0), mini_map->getOriginalSize());
    irr_driver->getVideoDriver()->draw2DImage(mini_map, dest, source, 0, 0, true);
    
    for(unsigned int i=0; i<race_manager->getNumKarts(); i++)
    {
        const Kart *kart = RaceManager::getKart(i);
        if(kart->isEliminated()) continue;   // don't draw eliminated kart
    	const Vec3& xyz = kart->getXYZ();
        Vec3 draw_at;
        RaceManager::getTrack()->mapPoint2MiniMap(xyz, &draw_at);
        // int marker_height = m_marker->getOriginalSize().Height;
        core::rect<s32> source(i    *m_marker_rendered_size, 0, 
                               (i+1)*m_marker_rendered_size, m_marker_rendered_size);
        int marker_half_size =  (kart->isPlayerKart() ? m_marker_player_size 
                                                      : m_marker_ai_size      )>>1;
        core::rect<s32> position(m_map_left+(int)(draw_at.getX()-marker_half_size), 
                                 lower_y   -(int)(draw_at.getY()+marker_half_size),
                                 m_map_left+(int)(draw_at.getX()+marker_half_size), 
                                 lower_y   -(int)(draw_at.getY()-marker_half_size));
        irr_driver->getVideoDriver()->draw2DImage(m_marker, position, source, NULL, NULL, true);
    }   // for i<getNumKarts
}   // drawGlobalMiniMap

//-----------------------------------------------------------------------------
// Draw players icons and their times (if defined in the current mode).
void RaceGUI::drawGlobalPlayerIcons(const KartIconDisplayInfo* info)
{
    assert(RaceManager::getWorld() != NULL);

    int x = 5;
    int y;
    int ICON_WIDTH=40;
    int ICON_PLAYER_WIDTH=50;
    if(UserConfigParams::m_height<600)
    {
        ICON_WIDTH        = 27;
        ICON_PLAYER_WIDTH = 35;
    }
    
    gui::IGUIFont* font = irr_driver->getRaceFont();
    const unsigned int kart_amount = race_manager->getNumKarts();
    for(unsigned int i = 0; i < kart_amount ; i++)
    {
        Kart* kart   = RaceManager::getKart(i);
        if(kart->isEliminated()) continue;
        const int position = kart->getPosition();

        y = 20 + ( (position == -1 ? i : position-1)*(ICON_PLAYER_WIDTH+2));

        if (info[i].time.size() > 0)
        {
            static video::SColor color = video::SColor(255, (int)(255*info[i].r),
                                                       (int)(255*info[i].g), 
                                                       (int)(255*info[i].b)      );
            core::rect<s32> pos(x+ICON_PLAYER_WIDTH, y+5, x+ICON_PLAYER_WIDTH, y+5);
            core::stringw s=info[i].time.c_str();
            font->draw(s.c_str(), pos, color);
        }
        
        if (info[i].special_title.size() > 0)
        {
            static video::SColor color = video::SColor(255, 255, 0, 0);
            core::rect<s32> pos(x+ICON_PLAYER_WIDTH, y+5, x+ICON_PLAYER_WIDTH, y+5);
            core::stringw s(info[i].special_title.c_str());
            font->draw(s.c_str(), pos, color);
        }

        // draw icon
        video::ITexture *icon = kart->getKartProperties()->getIconMaterial()->getTexture();
        int w = kart->isPlayerKart() ? ICON_PLAYER_WIDTH : ICON_WIDTH;
        const core::rect<s32> pos(x, y, x+w, y+w);

        // Fixes crash bug, why are certain icons not showing up?
        if (icon != NULL)
        {
            const core::rect<s32> rect(core::position2d<s32>(0,0), icon->getOriginalSize());
            irr_driver->getVideoDriver()->draw2DImage(icon, pos, rect, NULL, NULL, true);
        }

    } // next kart
    
}   // drawGlobalPlayerIcons

//-----------------------------------------------------------------------------
void RaceGUI::drawPowerupIcons(Kart* player_kart, 
                               const core::recti &viewport, 
                               const core::vector2df &scaling)
{
    // If player doesn't have anything, do nothing.
    Powerup* powerup=player_kart->getPowerup();
    if(powerup->getType() == POWERUP_NOTHING) return;
    int n  = player_kart->getNumPowerup() ;
    if(n<1) return;    // shouldn't happen, but just in case
    if(n>5) n=5;       // Display at most 5 items

    int nSize=(int)(64.0f*std::min(scaling.X, scaling.Y));
    int x1 = (int)((UserConfigParams::m_width/2-32) * scaling.X) 
           + viewport.UpperLeftCorner.X;
    int y1 = UserConfigParams::m_height - viewport.LowerRightCorner.Y 
           + (int)(20 * scaling.Y)+nSize;

    video::ITexture *t=powerup->getIcon()->getTexture();
    core::rect<s32> rect(core::position2di(0, 0), t->getOriginalSize());

    for ( int i = 0 ; i < n ; i++ )
    {
        core::rect<s32> pos(x1+i*30, y1-nSize, x1+i*30+nSize, y1);
        irr_driver->getVideoDriver()->draw2DImage(t, pos, rect, NULL, 
                                                  NULL, true);
    }   // for i
}   // drawPowerupIcons

//-----------------------------------------------------------------------------
/* Energy meter that gets filled with coins */
void RaceGUI::drawEnergyMeter (Kart *player_kart,              
                               const core::recti &viewport, 
                               const core::vector2df &scaling)
{
    float state = (float)(player_kart->getEnergy()) / MAX_ITEMS_COLLECTED;
    int x = (int)((UserConfigParams::m_width-24) * scaling.X) + viewport.UpperLeftCorner.X;
    //int y = (int)(250 * scaling.Y) + viewport.UpperLeftCorner.Y;
    int y = UserConfigParams::m_height - (int)(250 * scaling.Y) - viewport.UpperLeftCorner.Y;
    int w = (int)(16 * scaling.X);
    int h = (int)(UserConfigParams::m_height/4 * scaling.Y);
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
void RaceGUI::drawSpeed(Kart* kart, const core::recti &viewport, 
                        const core::vector2df &scaling)
{

    float minRatio         = std::min(scaling.X, scaling.Y);
    const int SPEEDWIDTH   = 128;
    int meter_width        = (int)(SPEEDWIDTH*minRatio);
    int meter_height       = (int)(SPEEDWIDTH*minRatio);
    core::vector2di offset = viewport.UpperLeftCorner;
    offset.X              += (int)((UserConfigParams::m_width-10)*scaling.X) - meter_width;
    offset.Y              += (int)(10*scaling.Y);

    // First draw the meter (i.e. the background which contains the numbers etc.
    // -------------------------------------------------------------------------
    video::IVideoDriver *video = irr_driver->getVideoDriver();
    const core::rect<s32> meter_pos(offset.X,
                                    UserConfigParams::m_height-offset.Y-meter_height, 
                                    offset.X+meter_width, 
                                    UserConfigParams::m_height-offset.Y);
    video::ITexture *meter_texture = m_speed_meter_icon->getTexture();
    const core::rect<s32> meter_texture_coords(core::position2d<s32>(0,0), 
                                               meter_texture->getOriginalSize());
    video->draw2DImage(meter_texture, meter_pos, meter_texture_coords, NULL, NULL, true);

    // Indicate when the kart is off ground
    // ------------------------------------
    if ( !kart->isOnGround() )
    {
        static video::SColor color = video::SColor(255, 255, 255, 255);
        core::rect<s32> pos(offset.X-(int)(30*minRatio), 
                            UserConfigParams::m_height-(offset.Y-(int)(10*minRatio)),
                            offset.X-(int)(30*minRatio), 
                            UserConfigParams::m_height-(offset.Y-(int)(10*minRatio)) );
        irr_driver->getRaceFont()->draw(core::stringw("!").c_str(), pos, color);
    }

    const float speed =  kart->getSpeed();
    if(speed <=0) return;  // Nothing to do if speed is negative.

    // Draw the actual speed bar (if the speed is >0)
    // ----------------------------------------------
    float speed_ratio = speed/KILOMETERS_PER_HOUR/110.0f;
    if(speed_ratio>1) speed_ratio = 1;

    video::ITexture   *bar_texture = m_speed_bar_icon->getTexture();
#if IRRLICHT_VERSION_MAJOR > 1 || IRRLICHT_VERSION_MINOR >= 6
    core::dimension2du bar_size    = bar_texture->getOriginalSize();
#else
    core::dimension2di bar_size    = bar_texture->getOriginalSize();
#endif
    video::S3DVertex vertices[4];

    vertices[0].TCoords = core::vector2df(1.0f, 1.0f);
    vertices[0].Pos = core::vector3df((float)meter_pos.LowerRightCorner.X,
                                      (float)meter_pos.LowerRightCorner.Y,
                                      0);
    vertices[1].TCoords = core::vector2df(0, 1.0f);
    vertices[1].Pos = core::vector3df((float)meter_pos.UpperLeftCorner.X, 
                                      (float)meter_pos.LowerRightCorner.Y, 0);
    unsigned int count;
    if(speed_ratio<=0.5f)
    {
        count = 3;
        vertices[2].TCoords = core::vector2df(0, 1-2*speed_ratio);
        vertices[2].Pos = core::vector3df((float)meter_pos.UpperLeftCorner.X, 
                                          (float)meter_pos.LowerRightCorner.Y-speed_ratio*2*meter_height,
                                          0);
    }
    else
    {
        count = 4;
        vertices[2].TCoords = core::vector2df(0,0);
        vertices[2].Pos = core::vector3df((float)offset.X, 
            (float)(UserConfigParams::m_height-offset.Y-meter_height),
            0);
        vertices[3].TCoords = core::vector2df(2*speed_ratio-1.0f, 0);
        vertices[3].Pos = core::vector3df(offset.X+2*(speed_ratio-0.5f)*meter_width, 
            (float)(UserConfigParams::m_height-offset.Y-meter_height),
            0);
    }
    short int index[4];
    for(unsigned int i=0; i<count; i++)
    {
        index[i]=i;
        vertices[i].Color = video::SColor(255, 255, 255, 255);
    }
    video::SMaterial m;
    m.setTexture(0, m_speed_bar_icon->getTexture());
    irr_driver->getVideoDriver()->setMaterial(m);
#define DOES_NOT_WORK_ATM
#ifdef DOES_NOT_WORK_ATM
    irr_driver->getVideoDriver()->draw2DVertexPrimitiveList(vertices, count,
        index, count-2, video::EVT_STANDARD, scene::EPT_TRIANGLE_FAN);
#endif
} // drawSpeed

//-----------------------------------------------------------------------------
void RaceGUI::drawLap(const KartIconDisplayInfo* info, Kart* kart, 
                      const core::recti &viewport, 
                      const core::vector2df &scaling)
{
    // Don't display laps in follow the leader mode
    if(!RaceManager::getWorld()->raceHasLaps()) return;
    
    const int lap = info[kart->getWorldKartId()].lap;
    
    if(lap<0) return;  // don't display 'lap 0/...'
    float minRatio = std::min(scaling.X, scaling.Y);
    core::recti pos;
    pos.UpperLeftCorner.X  = viewport.UpperLeftCorner.X 
                           + (int)(0.15f*UserConfigParams::m_width*scaling.X);
    pos.UpperLeftCorner.Y  = UserConfigParams::m_height - viewport.LowerRightCorner.Y;
    //pos.UpperLeftCorner.Y += (int)(UserConfigParams::m_height*5/6*minRatio);
    pos.UpperLeftCorner.Y += (int)(40*minRatio);
    pos.LowerRightCorner   = pos.UpperLeftCorner;
    gui::IGUIFont* font    = irr_driver->getRaceFont();
    if(kart->hasFinishedRace())
    {
        static video::SColor color = video::SColor(255, 255, 255, 255);
        //I18N: Shown at the end of a race
        core::stringw s=_("Finished");
        font->draw(s.c_str(), pos, color);
    }
    else
    {
        static video::SColor color = video::SColor(255, 255, 255, 255);
        core::stringw s = _("Lap");
        font->draw(core::stringw(_("Lap")).c_str(), pos, color);
    
        char str[256];
        sprintf(str, "%d/%d", lap+1, race_manager->getNumLaps());
        pos.UpperLeftCorner.Y  += (int)(40*scaling.Y);
        pos.LowerRightCorner.Y += (int)(40*scaling.Y);
        font->draw(core::stringw(str).c_str(), pos, color);
    }
} // drawLap

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
void RaceGUI::drawAllMessages(Kart* player_kart,
                              const core::recti &viewport, 
                              const core::vector2df &scaling)
{
    int y;
    // First line of text somewhat under the top of the screen. For now
    // start just under the timer display
    y = (int)(scaling.Y*164+viewport.UpperLeftCorner.Y);
    // The message are displayed in reverse order, so that a multi-line
    // message (addMessage("1", ...); addMessage("2",...) is displayed
    // in the right order: "1" on top of "2"
    for(AllMessageType::const_iterator i=m_messages.begin();i!=m_messages.end(); ++i)
    {
        TimedMessage const &msg = *i;

        // Display only messages for all karts, or messages for this kart
        if( msg.m_kart && msg.m_kart!=player_kart) continue;

        core::rect<s32> pos(UserConfigParams::m_width>>1, y,
                            UserConfigParams::m_width>>1, y);
        irr_driver->getRaceFont()->draw(core::stringw(msg.m_message.c_str()).c_str(),
                                        pos, msg.m_color, true, true);
        y+=40;        
    }   // for i in all messages
}   // drawAllMessages

//-----------------------------------------------------------------------------
/** Adds a message to the message queue. The message is displayed for a
 *  certain amount of time (unless time<0, then the message is displayed
 *  once).
 **/
void RaceGUI::addMessage(const core::stringw &msg, const Kart *kart, float time, 
                         int font_size, const video::SColor &color)
{
    m_messages.push_back(TimedMessage(msg, kart, time, font_size, color));
}   // addMessage

//-----------------------------------------------------------------------------
// Displays the description given for the music currently being played -
// usually the title and composer.
void RaceGUI::drawGlobalMusicDescription()
{
    if (!UserConfigParams::m_music) return; // show no music description when it's off
    
    // 3.0 is the duration of ready/set (TODO: don't hardcode)
    float timeProgression = (float)(RaceManager::getWorld()->m_auxiliary_timer - 2.0f) /
    (float)(stk_config->m_music_credit_time - 2.0f);
    
    const int x_pulse = (int)(sin(RaceManager::getWorld()->m_auxiliary_timer*9.0f)*10.0f);
    const int y_pulse = (int)(cos(RaceManager::getWorld()->m_auxiliary_timer*9.0f)*10.0f);
    
    float resize = 1.0f;
    if (timeProgression < 0.1)
    {
        resize = timeProgression/0.1f;
    }
    else if (timeProgression > 0.9)
    {
        resize = 1.0f - (timeProgression - 0.9f)/0.1f;
    }
    
    
    const MusicInformation* mi=sound_manager->getCurrentMusic();
    if(!mi) return;
    
    const float resize3 = resize*resize*resize;
    
    const int y = UserConfigParams::m_height - 80;
    const int text_y = (int)(UserConfigParams::m_height - 80*(resize3) + 40*(1-resize));
    
    static const video::SColor white = video::SColor(255, 255, 255, 255);
    gui::IGUIFont*       font = irr_driver->getRaceFont();
    if(mi->getComposer()!="")
    {
        core::rect<s32> pos_by((UserConfigParams::m_width>>1) + 32, text_y+40,
                              (UserConfigParams::m_width>>1) + 32, text_y+40);
        std::string s="by "+mi->getComposer();
        font->draw(core::stringw(s.c_str()).c_str(), pos_by, white, true, true);
    }

    std::string s="\""+mi->getTitle()+"\"";

    // Draw text
    core::stringw thetext(s.c_str());

#if IRRLICHT_VERSION_MAJOR > 1 || IRRLICHT_VERSION_MINOR >= 6
    core::dimension2d< u32 > textSize = font->getDimension(thetext.c_str());
#else
    core::dimension2d< s32 > textSize = font->getDimension(thetext.c_str());
#endif
    const int textWidth = textSize.Width;
    
    core::rect<s32> pos((UserConfigParams::m_width >> 1) + 32, text_y,
                        (UserConfigParams::m_width >> 1) + 32, text_y);
    
    font->draw(thetext.c_str(), pos, white, true, true);
    
    

    int iconSizeX = (int)(64*resize + x_pulse*resize*resize);
    int iconSizeY = (int)(64*resize + y_pulse*resize*resize);

    // Draw music icon
    video::ITexture *t = m_music_icon->getTexture();
    const int noteX = (UserConfigParams::m_width >> 1) - textWidth/2 - 32;
    const int noteY = y;
    core::rect<s32> dest(noteX-iconSizeX/2+20,    noteY-iconSizeY/2+32, 
                         noteX+iconSizeX/2+20,    noteY+iconSizeY/2+32);
    const core::rect<s32> source(core::position2d<s32>(0,0), t->getOriginalSize());
    
    irr_driver->getVideoDriver()->draw2DImage(t, dest, source, NULL, NULL, true);

}   // drawGlobalMusicDescription

// ----------------------------------------------------------------------------
/** Draws the ready-set-go message on the screen.
 */
void RaceGUI::drawGlobalReadySetGo()
{
    assert(RaceManager::getWorld() != NULL);
    switch (RaceManager::getWorld()->getPhase())
    {
    case READY_PHASE:
        {
            static video::SColor color = video::SColor(255, 230, 168, 158);
            core::rect<s32> pos(UserConfigParams::m_width>>1, UserConfigParams::m_height>>1,
                                UserConfigParams::m_width>>1, UserConfigParams::m_height>>1);
            gui::IGUIFont* font = irr_driver->getRaceFont();
            //I18N: as in "ready, set, go", shown at the beginning of the race
            core::stringw s=_("Ready!");
            font->draw(s.c_str(), pos, color, true, true);
        }
        break;
    case SET_PHASE:
        {
            static video::SColor color = video::SColor(255, 230, 230, 158);
            core::rect<s32> pos(UserConfigParams::m_width>>1, UserConfigParams::m_height>>1,
                                UserConfigParams::m_width>>1, UserConfigParams::m_height>>1);
            gui::IGUIFont* font = irr_driver->getRaceFont();
            //I18N: as in "ready, set, go", shown at the beginning of the race
            core::stringw s=_("Set!");
            font->draw(s.c_str(), pos, color, true, true);
        }
        break;
    case GO_PHASE:
        {
            static video::SColor color = video::SColor(255, 100, 209, 100);
            core::rect<s32> pos(UserConfigParams::m_width>>1, UserConfigParams::m_height>>1,
                                UserConfigParams::m_width>>1, UserConfigParams::m_height>>1);
            gui::IGUIFont* font = irr_driver->getRaceFont();
            //I18N: as in "ready, set, go", shown at the beginning of the race
            core::stringw s=_("Go!");
            font->draw(s.c_str(), pos, color, true, true);
        }
        break;
    default: 
         break;
    }   // switch
}   // drawGlobalReadySetGo

// ----------------------------------------------------------------------------
void RaceGUI::displayPenaltyMessages()
{
    // The penalty message needs to be displayed for up to one second
    // after the start of the race, otherwise it disappears if 
    // "Go" is displayed and the race starts
    const unsigned int num_players = race_manager->getNumLocalPlayers();
    if(RaceManager::getWorld()->isStartPhase() || RaceManager::getWorld()->getTime()<1.0f)
    {
        for(unsigned int i=0; i<num_players; i++)
        {
            if(RaceManager::getWorld()->getLocalPlayerKart(i)->earlyStartPenalty())
            {
                static video::SColor color = video::SColor(255, 179, 6, 6);
                // FIXME: the position should take split screen into account!
                core::rect<s32> pos(UserConfigParams::m_width>>1, (UserConfigParams::m_height>>1)-40,
                                    UserConfigParams::m_width>>1, (UserConfigParams::m_height>>1)-40);
                gui::IGUIFont* font = irr_driver->getRaceFont();
                core::stringw s=_("Penalty time!!");
                font->draw(s.c_str(), pos, color, true, true);
            }   // if penalty
        }  // for i < getNum_players
    }  // if not RACE_PHASE
}   // displayPenaltyMessage
