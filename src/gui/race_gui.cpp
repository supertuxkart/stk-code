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

#include "gui/race_gui.hpp"

#include "audio/sound_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "gui/font.hpp"
#include "input/input.hpp"
#include "input/input_manager.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include "irrlicht.h"
using namespace irr;

RaceGUI::RaceGUI()
{
    // FIXME: translation problem
    m_pos_string[0] = "?!?";
    m_pos_string[1] = "1st";
    m_pos_string[2] = "2nd";
    m_pos_string[3] = "3rd";
    m_pos_string[4] = "4th";
    m_pos_string[5] = "5th";
    m_pos_string[6] = "6th";
    m_pos_string[7] = "7th";
    m_pos_string[8] = "8th";
    m_pos_string[9] = "9th";
    m_pos_string[10] = "10th";

    gui::IGUIEnvironment *gui_env = irr_driver->getGUI();

    int icon_width=40;
    int icon_player_width=50;
    if(UserConfigParams::m_height<600)
    {
        icon_width        = 27;
        icon_player_width = 35;
    }

#ifndef HAVE_IRRLICHT
    m_speed_back_icon = material_manager->getMaterial("speedback.rgb");
    m_speed_back_icon->getState()->disable(GL_CULL_FACE);
    m_speed_fore_icon = material_manager->getMaterial("speedfore.rgb");
    m_speed_fore_icon->getState()->disable(GL_CULL_FACE);
    
    m_plunger_face = material_manager->getMaterial("plungerface.rgb");
    m_plunger_face->getState()->disable(GL_CULL_FACE);
#endif
    
}   // RaceGUI

//-----------------------------------------------------------------------------
RaceGUI::~RaceGUI()
{
    //FIXME: does all that material stuff need freeing somehow?
}   // ~Racegui

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
#define TRACKVIEW_SIZE 100

void RaceGUI::drawMap ()
{
    // arenas currently don't have a map.
    if(RaceManager::getTrack()->isArena()) return;
    
    glDisable ( GL_TEXTURE_2D ) ;
    assert(RaceManager::getWorld() != NULL);
    int xLeft = 10;
    int yTop   =  10;

    RaceManager::getTrack() -> draw2Dview ( (float)xLeft,   (float)yTop   );

    glBegin ( GL_QUADS ) ;

    for ( unsigned int i = 0 ; i < race_manager->getNumKarts() ; i++ )
    {
        Kart* kart = RaceManager::getKart(i);
        if(kart->isEliminated()) continue;   // don't draw eliminated kart
        glColor3fv ( kart->getColor().toFloat());
	const Vec3& xyz = kart->getXYZ();

        /* If it's a player, draw a bigger sign */
        // TODO
        /*
        if (kart -> isPlayerKart ())
        {
            RaceManager::getTrack() -> glVtx ( xyz.toFloat(), (float)xLeft+3, (float)yTop+3);
            RaceManager::getTrack() -> glVtx ( xyz.toFloat(), (float)xLeft-2, (float)yTop+3);
            RaceManager::getTrack() -> glVtx ( xyz.toFloat(), (float)xLeft-2, (float)yTop-2);
            RaceManager::getTrack() -> glVtx ( xyz.toFloat(), (float)xLeft+3, (float)yTop-2);
        }
        else
        {
            RaceManager::getTrack() -> glVtx ( xyz.toFloat(), (float)xLeft+2, (float)yTop+2);
            RaceManager::getTrack() -> glVtx ( xyz.toFloat(), (float)xLeft-1, (float)yTop+2);
            RaceManager::getTrack() -> glVtx ( xyz.toFloat(), (float)xLeft-1, (float)yTop-1);
            RaceManager::getTrack() -> glVtx ( xyz.toFloat(), (float)xLeft+2, (float)yTop-1);
        }
         */
    }

    glEnd () ;
    glEnable ( GL_TEXTURE_2D ) ;
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
        const core::rect<s32> rect(core::position2d<s32>(0,0), icon->getOriginalSize());
        irr_driver->getVideoDriver()->draw2DImage(icon, pos, rect, 0,
                                                  &video::SColor(255,255,255,255), true);

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
        irr_driver->getVideoDriver()->draw2DImage(t, pos, rect, 0, 
                                                  &video::SColor(255,255,255,255),
                                                  true);
    }   // for i
}   // drawPowerupIcons

//-----------------------------------------------------------------------------
/* Energy meter that gets filled with coins */

// Meter fluid color (0 - 255)
#define METER_TOP_COLOR    240, 0, 0, 255
#define METER_BOTTOM_COLOR    240, 200, 0, 160
// Meter border color (0.0 - 1.0)
#define METER_BORDER_BLACK 0.0, 0.0, 0.0
#define METER_BORDER_WHITE 1.0, 1.0, 1.0
#define METER_TARGET_RED   1.0, 0.0, 0.0

//-----------------------------------------------------------------------------
void RaceGUI::drawEnergyMeter ( Kart *player_kart, int offset_x, int offset_y,
                                float ratio_x, float ratio_y             )
{
    float state = (float)(player_kart->getEnergy()) /
                  MAX_ITEMS_COLLECTED;
    int x = (int)((UserConfigParams::m_width-24) * ratio_x) + offset_x;
    int y = (int)(250 * ratio_y) + offset_y;
    int w = (int)(16 * ratio_x);
    int h = (int)(UserConfigParams::m_height/4 * ratio_y);
    int wl = (int)(ratio_x);
    if(wl < 1)
        wl = 1;
    const int GRADS = (int)(MAX_ITEMS_COLLECTED/5);  // each graduation equals 5 items
    int gh = (int)(h/GRADS);  //graduation height
    float coin_target = (float)race_manager->getCoinTarget();
    int th = (int)(h*(coin_target/MAX_ITEMS_COLLECTED));

    glDisable(GL_TEXTURE_2D);
    // Draw a Meter border
    x-=1;
    y-=1;
    // left side
    glBegin ( GL_QUADS ) ;
    glColor3f ( METER_BORDER_BLACK ) ;
    glVertex2i ( x-wl, y-wl ) ;
    glVertex2i ( x,    y-wl ) ;
    glVertex2i ( x,    y + h+1) ;
    glVertex2i ( x-wl, y + h+1) ;
    glEnd () ;

    // right side
    glBegin ( GL_QUADS ) ;
    glColor3f ( METER_BORDER_BLACK ) ;
    glVertex2i ( x+w,    y-wl ) ;
    glVertex2i ( x+w+wl, y-wl ) ;
    glVertex2i ( x+w+wl, y + h+1) ;
    glVertex2i ( x+w,    y + h+1) ;
    glEnd () ;

    // down side
    glBegin ( GL_QUADS ) ;
    glColor3f ( METER_BORDER_BLACK ) ;
    glVertex2i ( x,   y-wl ) ;
    glVertex2i ( x+w, y-wl ) ;
    glVertex2i ( x+w, y ) ;
    glVertex2i ( x,   y ) ;
    glEnd () ;

    // up side
    glBegin ( GL_QUADS ) ;
    glColor3f ( METER_BORDER_BLACK ) ;
    glVertex2i ( x,   y+h ) ;
    glVertex2i ( x+w, y+h ) ;
    glVertex2i ( x+w, y+h+wl ) ;
    glVertex2i ( x,   y+h+wl ) ;
    glEnd () ;

    x+=1;
    y+=1;

    // left side
    glBegin ( GL_QUADS ) ;
    glColor3f ( METER_BORDER_WHITE ) ;
    glVertex2i ( x-wl, y-wl ) ;
    glVertex2i ( x,    y-wl ) ;
    glVertex2i ( x,    y + h+1) ;
    glVertex2i ( x-wl, y + h+1) ;
    glEnd () ;

    // right side
    glBegin ( GL_QUADS ) ;
    glColor3f ( METER_BORDER_WHITE ) ;
    glVertex2i ( x+w,    y-wl ) ;
    glVertex2i ( x+w+wl, y-wl ) ;
    glVertex2i ( x+w+wl, y + h+1) ;
    glVertex2i ( x+w,    y + h+1) ;
    glEnd () ;

    // down side
    glBegin ( GL_QUADS ) ;
    glColor3f ( METER_BORDER_WHITE ) ;
    glVertex2i ( x,   y-wl ) ;
    glVertex2i ( x+w, y-wl ) ;
    glVertex2i ( x+w, y ) ;
    glVertex2i ( x,   y ) ;
    glEnd () ;

    //Graduations
    int gh_incr = gh;
    for (int i=0; i<GRADS-1; i++)
    {
        glBegin( GL_QUADS );
        glColor3f(METER_BORDER_WHITE);
        glVertex2i(x,   y+gh);
        glVertex2i(x+w, y+gh);
        glVertex2i(x+w, y+gh+wl);
        glVertex2i(x,   y+gh+wl);
        glEnd();
        gh+=gh_incr;
    }
    
    //Target line
    if (coin_target > 0)
    {
        glBegin ( GL_QUADS );
        glColor3f(METER_TARGET_RED);
        glVertex2i(x,   y+th);
        glVertex2i(x+w, y+th);
        glVertex2i(x+w, y+th+wl);
        glVertex2i(x,   y+th+wl);
        glEnd();
    }
    
    // up side
    glBegin ( GL_QUADS ) ;
    glColor3f ( METER_BORDER_WHITE ) ;
    glVertex2i ( x,   y+h ) ;
    glVertex2i ( x+w, y+h ) ;
    glVertex2i ( x+w, y+h+wl ) ;
    glVertex2i ( x,   y+h+wl ) ;
    glEnd () ;

    // Draw the Meter fluid
    glBegin ( GL_QUADS ) ;
    glColor4ub ( METER_BOTTOM_COLOR ) ;
    glVertex2i ( x,   y ) ;
    glVertex2i ( x+w, y ) ;

    glColor4ub ( METER_TOP_COLOR ) ;
    glVertex2i ( x+w, y + (int)(state * h));
    glVertex2i ( x,   y + (int)(state * h) ) ;
    glEnd () ;
    glEnable(GL_TEXTURE_2D);
}   // drawEnergyMeter

//-----------------------------------------------------------------------------
void RaceGUI::drawSpeed(Kart* kart, int offset_x, int offset_y,
                        float ratio_x, float ratio_y           )
{

    float minRatio = std::min(ratio_x, ratio_y);
#define SPEEDWIDTH 128
    int width  = (int)(SPEEDWIDTH*minRatio);
    int height = (int)(SPEEDWIDTH*minRatio);
    offset_x += (int)((UserConfigParams::m_width-10)*ratio_x) - width;
    offset_y += (int)(10*ratio_y);

#ifdef HAVE_IRRLICHT
#else
    glMatrixMode(GL_MODELVIEW);
    m_speed_back_icon->getState()->force();
    // If the colour isn't set, the speedometer is blended with the last
    // used colour.
    glColor4f(1,1,1,1);
    glBegin ( GL_QUADS ) ;
    glTexCoord2f(0, 0);glVertex2i(offset_x      , offset_y       );
    glTexCoord2f(1, 0);glVertex2i(offset_x+width, offset_y       );
    glTexCoord2f(1, 1);glVertex2i(offset_x+width, offset_y+height);
    glTexCoord2f(0, 1);glVertex2i(offset_x      , offset_y+height);
    glEnd () ;

    //convention taken from btRaycastVehicle::updateVehicle
    const float speed =  kart->getSpeed();

    if ( !kart->isOnGround() )
        font_race->PrintShadow("!", (int)(60*minRatio), 
                               offset_x-(int)(30*minRatio), 
                               offset_y-(int)(10*minRatio));
    /* Show speed */
    if ( speed < 0 )
        font_race->PrintShadow(_("REV"), (int)(40*minRatio), 
                               offset_x+(int)(40*minRatio), 
                               offset_y+(int)(10*minRatio));
    else
    {
        float speedRatio = speed/KILOMETERS_PER_HOUR/110.0f;
        // The following does not work with wheelie or Zipper
        //float speedRatio = kart->getVelocity()->xyz[1]/(kart->getMaxSpeed();

        if ( speedRatio > 1 )
            speedRatio = 1;
        
        m_speed_fore_icon->getState()->force();
        glBegin ( GL_POLYGON ) ;
        glTexCoord2f(1, 0);glVertex2i(offset_x+width, offset_y);
        glTexCoord2f(0, 0);glVertex2i(offset_x, offset_y);
        if (speedRatio < 0.5)
        {
            glTexCoord2f(0, speedRatio*2);glVertex2i(offset_x, (int)(offset_y+width*speedRatio*2));
        }
        else
        {
            glTexCoord2f(0, 1);glVertex2i(offset_x, offset_y+width);
            glTexCoord2f((speedRatio-0.5f)*2, 1);glVertex2i((int)(offset_x+height*(speedRatio-0.5f)*2), offset_y+height);
        }

        glEnd () ;
    }   // speed<0
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
    y = (int)(ratio_y*(UserConfigParams::m_height -164)+offset_y);
    // The message are displayed in reverse order, so that a multi-line
    // message (addMessage("1", ...); addMessage("2",...) is displayed
    // in the right order: "1" on top of "2"
    for(AllMessageType::const_iterator i=m_messages.begin();i!=m_messages.end(); ++i)
    {
        TimedMessage const &msg = *i;

        // Display only messages for all karts, or messages for this kart
        if( msg.m_kart && msg.m_kart!=player_kart) continue;

        //FIXME: instead of the next line, in msg there should be a GLfloat that acts as the colors.
        GLfloat const COLORS[] = {msg.m_red/255.0f, msg.m_green/255.0f, msg.m_blue/255.0f, 255.0f};
        font_race->Print( msg.m_message.c_str(), msg.m_font_size, 
                          Font::CENTER_OF_SCREEN, y,
                          COLORS,
                          ratio_x, ratio_y,
                          offset_x, offset_x+(int)(UserConfigParams::m_width*ratio_x));
        // Add 20% of font size as space between the lines
        y-=msg.m_font_size*12/10;
        
        
    }   // for i in all messages
}   // drawAllMessages

//-----------------------------------------------------------------------------
/** Adds a message to the message queue. The message is displayed for a
 *  certain amount of time (unless time<0, then the message is displayed
 *  once).
 **/
void RaceGUI::addMessage(const std::string &msg, const Kart *kart, float time, 
                         int font_size, int red, int green, int blue)
{
    m_messages.push_back(TimedMessage(msg, kart, time, font_size, red, green, blue));
}   // addMessage

//-----------------------------------------------------------------------------
// Displays the description given for the music currently being played -
// usually the title and composer.
void RaceGUI::drawMusicDescription()
{
    const MusicInformation* mi=sound_manager->getCurrentMusic();
    if(!mi) return;
    int y=0;
    if(mi->getComposer()!="")
    {
        std::string s="by "+mi->getComposer();
        font_race->Print( s.c_str(), 25, 
                          Font::CENTER_OF_SCREEN, y );
        y+=20;
    }
    std::string s="\""+mi->getTitle()+"\"";
    font_race->Print( s.c_str(), 25, 
                      Font::CENTER_OF_SCREEN, y );
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
        //            drawEnergyMeter     (player_kart, offset_x, offset_y,
        //                                 split_screen_ratio_x, split_screen_ratio_y );
        //            drawSpeed           (player_kart, offset_x, offset_y,
        //                                 split_screen_ratio_x, split_screen_ratio_y );
                    drawLap             (info, player_kart, offset_x, offset_y,
                                         split_screen_ratio_x, split_screen_ratio_y );
        //            drawAllMessages     (player_kart, offset_x, offset_y,
        //                                 split_screen_ratio_x, split_screen_ratio_y );

        if(player_kart->hasViewBlockedByPlunger())
        {
            const int screen_width = (num_players > 2) ? UserConfigParams::m_width/2 : UserConfigParams::m_width;
            const int plunger_size = (num_players > 1) ? UserConfigParams::m_height/2 : UserConfigParams::m_height;
            int plunger_x = offset_x + screen_width/2 - plunger_size/2;

            if (num_players == 3 && pla > 1)
                plunger_x = offset_x + UserConfigParams::m_width/2 - plunger_size/2;

#ifndef HAVE_IRRLICHT
            m_plunger_face->getState()->force();
#endif
            glBegin ( GL_QUADS ) ;
            glTexCoord2f(1, 0); glVertex2i(plunger_x+plunger_size,    offset_y);
            glTexCoord2f(0, 0); glVertex2i(plunger_x,                 offset_y);
            glTexCoord2f(0, 1); glVertex2i(plunger_x,                 offset_y+plunger_size);
            glTexCoord2f(1, 1); glVertex2i(plunger_x+plunger_size,    offset_y+plunger_size);
            glEnd () ;                
        }
    }   // next player

    drawTimer();

    if(RaceManager::getWorld()->getPhase() == GO_PHASE ||
        RaceManager::getWorld()->getPhase() == MUSIC_PHASE)
    {
        drawMusicDescription();
    }

    //drawMap();
    drawPlayerIcons(info);

}   // drawStatusText
