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
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "io/file_manager.hpp"
#include "input/input.hpp"
#include "input/input_manager.hpp"
#include "karts/kart_properties_manager.hpp"
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
#ifdef IRR_SVN
    irr_driver->beginRenderToTexture(core::dimension2du(m_marker_rendered_size * npower2, 
                                     m_marker_rendered_size), 
                                     "RaceGUI::markers");
#else
    irr_driver->beginRenderToTexture(core::dimension2di(m_marker_rendered_size * npower2, 
                                     m_marker_rendered_size), 
                                     "RaceGUI::markers");
#endif
    for(unsigned int i=0; i<race_manager->getNumKarts(); i++)
    {
        const std::string& kart_ident = race_manager->getKartIdent(i);
        const KartProperties *kp = kart_properties_manager->getKart(kart_ident);
        core::vector2df center((float)((m_marker_rendered_size>>1)+i*m_marker_rendered_size), 
                               (float)(m_marker_rendered_size>>1)                   );
        int count = kp->getShape();
        core::array<core::vector2df> vertices;
        createRegularPolygon(count, (float)radius, center,&vertices);

        video::SColor color = kp->getColor();
        core::array<video::SColor> colors;
        colors.push_back(color);
#ifdef IRRLICHT_HAS_SUPERTUXKART_POLYGON
        irr_driver->getVideoDriver()->draw2DPolygon(vertices, &colors);
#endif
    }
    m_marker = irr_driver->endRenderToTexture();
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
                                   core::array<core::vector2df> *v)
{
    float f = 2*M_PI/(float)n;
    for (unsigned int i=0; i<n; i++)
    {
        float p = i*f;
        core::vector2df X = center + core::vector2df(sin(p)*radius, -cos(p)*radius);
        v->push_back(X);
    }

}   // createRegularPolygon

//-----------------------------------------------------------------------------
/** Called before rendering, so no direct output to the screen can be done
 *  here.
 *  \param dt Time step size.
 */
void RaceGUI::update(float dt)
{
    cleanupMessages(dt);
}   // update

//-----------------------------------------------------------------------------
/** Render the race gui. Direct access to the screen is possible here, since 
 *  this is called during irrlicht rendering.
 */
void RaceGUI::render()
{
    drawStatusText();
}   // render

//-----------------------------------------------------------------------------
/** Displays the racing time on the screen.s
 */
void RaceGUI::drawTimer ()
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
}   // drawTimer

//-----------------------------------------------------------------------------
/** Draws the mini map and the position of all karts on it.
 */
void RaceGUI::drawMiniMap()
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
        int marker_height = m_marker->getOriginalSize().Height;
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
}   // drawMap

//-----------------------------------------------------------------------------
// Draw players icons and their times (if defined in the current mode).
void RaceGUI::drawPlayerIcons (const KartIconDisplayInfo* info)
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

        if(info[i].time.length()>0)
        {
            static video::SColor color = video::SColor(255, (int)(255*info[i].r),
                                                       (int)(255*info[i].g), 
                                                       (int)(255*info[i].b)      );
            core::rect<s32> pos(x+ICON_PLAYER_WIDTH, y+5, x+ICON_PLAYER_WIDTH, y+5);
            core::stringw s=info[i].time.c_str();
            font->draw(s.c_str(), pos, color);
        }
        
        if(info[i].special_title.length() >0)
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
    
}   // drawPlayerIcons

//-----------------------------------------------------------------------------
void RaceGUI::drawPowerupIcons(Kart* player_kart, int offset_x,
                               int offset_y, float ratio_x,
                               float ratio_y                    )
{
    // If player doesn't have anything, do nothing.
    Powerup* powerup=player_kart->getPowerup();
    if(powerup->getType() == POWERUP_NOTHING) return;
    int n  = player_kart->getNumPowerup() ;
    if(n<1) return;    // shouldn't happen, but just in case
    if(n>5) n=5;       // Display at most 5 items

    // Originally the hardcoded sizes were 320-32 and 400
    int x1 = (int)((UserConfigParams::m_width/2-32) * ratio_x) + offset_x;
    int y1 = (int)(20 * ratio_y) + offset_y;

    int nSize=(int)(64.0f*std::min(ratio_x, ratio_y));

    video::ITexture *t=powerup->getIcon()->getTexture();
    core::rect<s32> rect(core::position2di(0, 0), t->getOriginalSize());

    for ( int i = 0 ; i < n ; i++ )
    {
        core::rect<s32> pos(x1+i*30, y1, x1+i*30+nSize, y1+nSize);
        irr_driver->getVideoDriver()->draw2DImage(t, pos, rect, NULL, 
                                                  NULL, true);
    }   // for i
}   // drawPowerupIcons

//-----------------------------------------------------------------------------
/* Energy meter that gets filled with coins */
void RaceGUI::drawEnergyMeter ( Kart *player_kart, int offset_x, int offset_y,
                                float ratio_x, float ratio_y             )
{
    float state = (float)(player_kart->getEnergy()) / MAX_ITEMS_COLLECTED;
    int x = (int)((UserConfigParams::m_width-24) * ratio_x) + offset_x;
    int y = (int)(250 * ratio_y) + offset_y;
    int w = (int)(16 * ratio_x);
    int h = (int)(UserConfigParams::m_height/4 * ratio_y);
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
void RaceGUI::drawSpeed(Kart* kart, int offset_x, int offset_y,
                        float ratio_x, float ratio_y           )
{

    float minRatio       = std::min(ratio_x, ratio_y);
    const int SPEEDWIDTH = 128;
    int meter_width      = (int)(SPEEDWIDTH*minRatio);
    int meter_height     = (int)(SPEEDWIDTH*minRatio);
    offset_x            += (int)((UserConfigParams::m_width-10)*ratio_x) - meter_width;
    offset_y            += (int)(10*ratio_y);

    // First draw the meter (i.e. the background which contains the numbers etc.
    // -------------------------------------------------------------------------
    video::IVideoDriver *video = irr_driver->getVideoDriver();
    const core::rect<s32> meter_pos(offset_x,             UserConfigParams::m_height-offset_y-meter_height, 
                                    offset_x+meter_width, UserConfigParams::m_height-offset_y);
    video::ITexture *meter_texture = m_speed_meter_icon->getTexture();
    const core::rect<s32> meter_texture_coords(core::position2d<s32>(0,0), 
                                               meter_texture->getOriginalSize());
    video->draw2DImage(meter_texture, meter_pos, meter_texture_coords, NULL, NULL, true);

    // Indicate when the kart is off ground
    // ------------------------------------
    if ( !kart->isOnGround() )
    {
        static video::SColor color = video::SColor(255, 255, 255, 255);
        core::rect<s32> pos(offset_x-(int)(30*minRatio), 
                            UserConfigParams::m_height-(offset_y-(int)(10*minRatio)),
                            offset_x-(int)(30*minRatio), 
                            UserConfigParams::m_height-(offset_y-(int)(10*minRatio)) );
        irr_driver->getRaceFont()->draw(core::stringw("!").c_str(), pos, color);
    }

    const float speed =  kart->getSpeed();
    if(speed <=0) return;  // Nothing to do if speed is negative.

    // Draw the actual speed bar (if the speed is >0)
    // ----------------------------------------------
    float speed_ratio = speed/KILOMETERS_PER_HOUR/110.0f;
    if(speed_ratio>1) speed_ratio = 1;

    video::ITexture   *bar_texture = m_speed_bar_icon->getTexture();
#ifdef IRR_SVN
    core::dimension2du bar_size    = bar_texture->getOriginalSize();
#else
    core::dimension2di bar_size    = bar_texture->getOriginalSize();
#endif
    core::array<core::vector2di> tex_coords;        // texture coordinates
    core::array<core::vector2df> bar_vertices;      // screen coordinates

    tex_coords.push_back(core::vector2di(bar_size.Width, bar_size.Height));
    bar_vertices.push_back(core::vector2df((float)meter_pos.LowerRightCorner.X, 
                                           (float)meter_pos.LowerRightCorner.Y));
    tex_coords.push_back(core::vector2di(0,bar_size.Height));
    bar_vertices.push_back(core::vector2df((float)meter_pos.UpperLeftCorner.X, 
                                           (float)meter_pos.LowerRightCorner.Y));

    if(speed_ratio<=0.5f)
    {
        core::vector2di v0(0, bar_size.Height-(int)(2*(speed_ratio)*bar_size.Height));
        tex_coords.push_back(v0);
        bar_vertices.push_back(core::vector2df((float)meter_pos.UpperLeftCorner.X, 
                                               (float)meter_pos.LowerRightCorner.Y-speed_ratio*2*meter_height));
    }
    else
    {
        tex_coords.push_back(core::vector2di(0, 0));
        bar_vertices.push_back(core::vector2df((float)offset_x, 
                                               (float)(UserConfigParams::m_height-offset_y-meter_height)));

        core::vector2di v0((int)(2*(speed_ratio-0.5f)*bar_size.Width), 0);
        tex_coords.push_back(v0);
        bar_vertices.push_back(core::vector2df(offset_x+2*(speed_ratio-0.5f)*meter_width, 
                                               (float)(UserConfigParams::m_height-offset_y-meter_height)));
    }
#ifdef IRRLICHT_HAS_SUPERTUXKART_POLYGON
    irr_driver->getVideoDriver()->draw2DPolygon(bar_vertices, NULL, bar_texture, true, &tex_coords);
#endif
} // drawSpeed

//-----------------------------------------------------------------------------
void RaceGUI::drawLap(const KartIconDisplayInfo* info, Kart* kart, int offset_x,
                      int offset_y, float ratio_x, float ratio_y           )
{
    // Don't display laps in follow the leader mode
    if(!RaceManager::getWorld()->raceHasLaps()) return;
    
    const int lap = info[kart->getWorldKartId()].lap;
    
    if(lap<0) return;  // don't display 'lap 0/...', or do nothing if laps are disabled (-1)
    float minRatio = std::min(ratio_x, ratio_y);
    offset_x += (int)(120*ratio_x);
    offset_y += (int)(UserConfigParams::m_height*5/6*minRatio);

    gui::IGUIFont* font = irr_driver->getRaceFont();
    if(kart->hasFinishedRace())
    {
        static video::SColor color = video::SColor(255, 255, 255, 255);
        core::rect<s32> pos(offset_x, offset_y, offset_x, offset_y);
        core::stringw s=_("Finished");
        font->draw(s.c_str(), pos, color);
    }
    else
    {
        static video::SColor color = video::SColor(255, 255, 255, 255);
        core::rect<s32> pos(offset_x, offset_y, offset_x, offset_y);
        core::stringw s = _("Lap");
        font->draw(core::stringw(_("Lap")).c_str(), pos, color);
    
        char str[256];
        sprintf(str, "%d/%d", lap+1, race_manager->getNumLaps());
        pos.UpperLeftCorner.Y  += (int)(40*ratio_y);
        pos.LowerRightCorner.Y += (int)(40*ratio_y);
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
void RaceGUI::drawAllMessages(Kart* player_kart, int offset_x, int offset_y,
                              float ratio_x,  float ratio_y  )
{
    int y;
    // First line of text somewhat under the top of the screen. For now
    // start just under the timer display
    y = (int)(ratio_y*164+offset_y);
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
void RaceGUI::addMessage(const std::string &msg, const Kart *kart, float time, 
                         int font_size, const video::SColor &color)
{
    m_messages.push_back(TimedMessage(msg, kart, time, font_size, color));
}   // addMessage

//-----------------------------------------------------------------------------
// Displays the description given for the music currently being played -
// usually the title and composer.
void RaceGUI::drawMusicDescription()
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
    
    core::dimension2d< s32 > textSize = font->getDimension(thetext.c_str());
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

}   // drawMusicDescription

//-----------------------------------------------------------------------------
void RaceGUI::drawStatusText()
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

    for(int i = 0; i < 10; ++i)
    {
        if(RaceManager::getWorld()->m_debug_text[i] != "")
        {
            static video::SColor color = video::SColor(255, 100, 209, 100);
            core::rect<s32> pos(20, i*20, 20, (i+1)*20);
            gui::IGUIFont* font = irr_driver->getRaceFont();
            font->draw(RaceManager::getWorld()->m_debug_text[i].c_str(), pos, color);
        }
    }

    float split_screen_ratio_x, split_screen_ratio_y;
    split_screen_ratio_x = split_screen_ratio_y = 1.0;
    if(race_manager->getNumLocalPlayers() >= 2)
        split_screen_ratio_y = 0.5;
    if(race_manager->getNumLocalPlayers() >= 3)
        split_screen_ratio_x = 0.5;

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


    if(!RaceManager::getWorld()->isRacePhase()) return;


    KartIconDisplayInfo* info = RaceManager::getWorld()->getKartsDisplayInfo();

    for(unsigned int pla = 0; pla < num_players; pla++)
    {
        int offset_x = 0;
        int offset_y = 0;

        if(num_players == 2)
        {
            if(pla == 0) offset_y = UserConfigParams::m_height/2;
        }
        else if (num_players == 3)
        {
            if (pla == 0  || pla == 1)
                offset_y = UserConfigParams::m_height/2;
            else
            {
                // Fixes width for player 3
                split_screen_ratio_x = 1.0;
            }

            if (pla == 1)
                offset_x = UserConfigParams::m_width/2;

        }
        else if(num_players == 4)
        {
            if(pla == 0  || pla == 1)
                offset_y = UserConfigParams::m_height/2;

            if((pla == 1) || pla == 3)
                offset_x = UserConfigParams::m_width/2;
        }

        Kart* player_kart = RaceManager::getWorld()->getLocalPlayerKart(pla);
        drawPowerupIcons(player_kart, offset_x, offset_y,
                         split_screen_ratio_x, split_screen_ratio_y );
        drawEnergyMeter     (player_kart, offset_x, offset_y,
                            split_screen_ratio_x, split_screen_ratio_y );
        drawSpeed           (player_kart, offset_x, offset_y,
                             split_screen_ratio_x, split_screen_ratio_y );
        drawLap             (info, player_kart, offset_x, offset_y,
                             split_screen_ratio_x, split_screen_ratio_y );
        drawAllMessages     (player_kart, offset_x, offset_y,
                             split_screen_ratio_x, split_screen_ratio_y );

        if(player_kart->hasViewBlockedByPlunger())
        {
            const int screen_width = (num_players > 2) ? UserConfigParams::m_width/2 : UserConfigParams::m_width;
            const int plunger_size = (num_players > 1) ? UserConfigParams::m_height/2 : UserConfigParams::m_height;
            int plunger_x = offset_x + screen_width/2 - plunger_size/2;

            if (num_players == 3 && pla > 1)
                plunger_x = offset_x + UserConfigParams::m_width/2 - plunger_size/2;

            video::ITexture *t=m_plunger_face->getTexture();
            core::rect<s32> dest(plunger_x,              offset_y, 
                                 plunger_x+plunger_size, offset_y+plunger_size);
            const core::rect<s32> source(core::position2d<s32>(0,0), t->getOriginalSize());

            irr_driver->getVideoDriver()->draw2DImage(t, dest, source);
        }
    }   // next player

    drawTimer();

    if(RaceManager::getWorld()->getPhase() == GO_PHASE ||
        RaceManager::getWorld()->getPhase() == MUSIC_PHASE)
    {
        drawMusicDescription();
    }

    drawMiniMap();
    drawPlayerIcons(info);

}   // drawStatusText
