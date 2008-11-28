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

#include "race_gui.hpp"

#include "input.hpp"
#include "sdldrv.hpp"

#include "user_config.hpp"
#include "history.hpp"
#include "track.hpp"
#include "material_manager.hpp"
#include "menu_manager.hpp"
#include "audio/sound_manager.hpp"
#include "race_manager.hpp"

#undef USE_WIDGET_MANAGER
#ifdef USE_WIDGET_MANAGER
#include "widget_manager.hpp"
#endif

#include "translation.hpp"
#include "font.hpp"

#ifdef USE_WIDGET_MANAGER
//MAX_TOP_POS is the maximum number of racers to be shown in the bar to the
//left where the positions are drawn.
static const int MAX_TOP_POS = 10;

static const int MAX_HUMANS = 4;

enum WidgetTokens
{
    WTOK_FPS,
    WTOK_EMPTY1,
    WTOK_CLOCK,

    WTOK_EMPTY2,

    WTOK_FIRST_TOP_IMG,
    WTOK_LAST_TOP_IMG = WTOK_FIRST_TOP_IMG + MAX_TOP_POS,

    WTOK_FIRST_TOP_TEXT,
    WTOK_LAST_TOP_TEXT = WTOK_FIRST_TOP_TEXT + MAX_TOP_POS,

    WTOK_FIRST_MESSAGE,
    WTOK_LAST_MESSAGE = WTOK_FIRST_MESSAGE + MAX_HUMANS,

    WTOK_FIRST_POWERBAR,
    WTOK_LAST_POWERBAR = WTOK_FIRST_POWERBAR + MAX_HUMANS,

    WTOK_FIRST_POSITION,
    WTOK_LAST_POSITION = WTOK_FIRST_POSITION + MAX_HUMANS,

    WTOK_MAP,

    WTOK_FIRST_LAP,
    WTOK_LAST_LAP = WTOK_FIRST_LAP + MAX_HUMANS,

    WTOK_FIRST_WHEEL,
    WTOK_LAST_WHEEL = WTOK_FIRST_WHEEL + MAX_HUMANS,

    WTOK_FIRST_SPEED,
    WTOK_LAST_SPEED = WTOK_FIRST_SPEED + MAX_HUMANS
};
#endif

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

    //FIXME: Temporary, we need a better icon here
    m_steering_wheel_icon = material_manager->getMaterial("wheel.rgb");
    m_steering_wheel_icon->getState()->disable(GL_CULL_FACE);
    m_speed_back_icon = material_manager->getMaterial("speedback.rgb");
    m_speed_back_icon->getState()->disable(GL_CULL_FACE);
    m_speed_fore_icon = material_manager->getMaterial("speedfore.rgb");
    m_speed_fore_icon->getState()->disable(GL_CULL_FACE);

    m_fps_counter = 0;
    m_fps_string[0]=0;
    m_fps_timer.reset();
    m_fps_timer.update();
    m_fps_timer.setMaxDelta(1000);

#ifdef USE_WIDGET_MANAGER
    const bool HIDE_TEXT = false;
    widget_manager->setInitialTextState(HIDE_TEXT, "", WGT_FNT_LRG,
        WGT_FONT_RACE, WGT_WHITE );

    widget_manager->addWgt( WTOK_FPS, 30, 10 );
    widget_manager->addWgt( WTOK_EMPTY1, 40, 10 );
    widget_manager->addWgt( WTOK_CLOCK, 30, 10 );
    widget_manager->breakLine();

    widget_manager->layout( WGT_AREA_TOP );
#endif
}   // RaceGUI

//-----------------------------------------------------------------------------
RaceGUI::~RaceGUI()
{
#ifdef USE_WIDGET_MANAGER
    widget_manager->reset();
#endif

    //FIXME: does all that material stuff need freeing somehow?
}   // ~Racegui

//-----------------------------------------------------------------------------
void
RaceGUI::handle(GameAction ga, int value)
{
	static int isWireframe = false;
	
	// The next lines find out the player and kartaction that belongs
	// to a certain gameaction value (GameAction -> Player number, Kartaction).
	// Since the numbers are fixed this can be done through computation
	// (instead of using e.g. a separate data structure).
	// Note that the kartaction enum value and their representatives in
	// gameaction enum have the same order (Otherwise the stuff below would
	// not work ...)!
	if (ga >= GA_FIRST_KARTACTION && ga <= GA_LAST_KARTACTION)
	{
		// 'Pulls down' the gameaction value to make them multiples of the
		// kartaction values.
		int ka = ga - GA_FIRST_KARTACTION;
		
		int playerNo = ka / KC_COUNT;
		ka = ka % KC_COUNT;
		
		RaceManager::getWorld()->getLocalPlayerKart(playerNo)->action((KartAction) ka, value);
		
		return;
	}
	
	if (value)
		return;
	
	switch (ga)
	{
		case GA_DEBUG_ADD_BOWLING:
			if (race_manager->getNumPlayers() ==1 )
			{
				Kart* kart = RaceManager::getWorld()->getLocalPlayerKart(0);
				kart->setPowerup(POWERUP_BOWLING, 10000);
			}
			break;
		case GA_DEBUG_ADD_MISSILE:
			if (race_manager->getNumPlayers() ==1 )
			{
				Kart* kart = RaceManager::getPlayerKart(0);
				kart->setPowerup(POWERUP_PLUNGER, 10000);
			}
			break;
		case GA_DEBUG_ADD_HOMING:
			if (race_manager->getNumPlayers() ==1 )
			{
				Kart* kart = RaceManager::getPlayerKart(0);
				kart->setPowerup(POWERUP_CAKE, 10000);
			}
			break;
		case GA_DEBUG_TOGGLE_FPS:
			user_config->m_display_fps = !user_config->m_display_fps;
			if(user_config->m_display_fps)
			{
				m_fps_timer.reset();
				m_fps_timer.setMaxDelta(1000);
				m_fps_counter=0;
#ifdef USE_WIDGET_MANAGER
                widget_manager->showWgtText( WTOK_FPS );
			}
            else
            {
                widget_manager->hideWgtText( WTOK_FPS );
#endif
            }
			break;
		case GA_DEBUG_TOGGLE_WIREFRAME:
			glPolygonMode(GL_FRONT_AND_BACK, isWireframe ? GL_FILL : GL_LINE);
			isWireframe = ! isWireframe;
			break;
#ifndef WIN32
		// For now disable F9 toggling fullscreen, since windows requires
		// to reload all textures, display lists etc. Fullscreen can
		// be toggled from the main menu (options->display).
		case GA_TOGGLE_FULLSCREEN:
			inputDriver->toggleFullscreen(false);   // 0: do not reset textures
			// Fall through to put the game into pause mode.
#endif
		case GA_LEAVE_RACE:
			RaceManager::getWorld()->pause();
			menu_manager->pushMenu(MENUID_RACEMENU);
		break;
		case GA_DEBUG_HISTORY:
			history->Save();
			break;
		default:
			break;
	} // switch

}

//-----------------------------------------------------------------------------
void RaceGUI::update(float dt)
{
    drawStatusText(dt);
    cleanupMessages(dt);

    BaseGUI::update( dt );
}   // update

//-----------------------------------------------------------------------------
void RaceGUI::drawFPS ()
{
    if (++m_fps_counter>=50)
    {
        m_fps_timer.update();
        sprintf(m_fps_string, "%d",
                (int)(m_fps_counter/m_fps_timer.getDeltaTime()));
        m_fps_counter = 0;
        m_fps_timer.setMaxDelta(1000);
    }
#ifdef USE_WIDGET_MANAGER
    widget_manager->setWgtText( WTOK_FPS, m_fps_string );
#else
    font_race->PrintShadow(m_fps_string,48, 0, user_config->m_height-50);
#endif
}   // drawFPS

//-----------------------------------------------------------------------------
void RaceGUI::drawTimer ()
{
    assert(RaceManager::getWorld() != NULL);
    
    if(!RaceManager::getWorld()->shouldDrawTimer()) return;
    char str[256];
    
    TimeToString(RaceManager::getWorld()->getTime(), str);
#ifdef USE_WIDGET_MANAGER
    widget_manager->showWgtText( WTOK_CLOCK );
    widget_manager->setWgtText( WTOK_CLOCK, str );
#else
    font_race->PrintShadow(str, 60, user_config->m_width-260,
        user_config->m_height-64);
#endif
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
    }

    glEnd () ;
    glEnable ( GL_TEXTURE_2D ) ;
}   // drawMap

//-----------------------------------------------------------------------------
// Draw players position on the race
void RaceGUI::drawPlayerIcons (const KartIconDisplayInfo* info)
{
    assert(RaceManager::getWorld() != NULL);

    int x = 5;
    int y;
#define ICON_WIDTH 40
#define ICON_PLAYER_WIDHT 50

    //glEnable(GL_TEXTURE_2D);
    Material *last_players_gst = 0;

    int bFirst                 = 1;
    glDisable(GL_CULL_FACE);
    
    const unsigned int kart_amount = race_manager->getNumKarts();
    for(unsigned int i = 0; i < kart_amount ; i++)
    {
        Kart* kart   = RaceManager::getKart(i);
        if(kart->isEliminated()) continue;
        const int position = kart->getPosition();

        y = user_config->m_height*3/4-20 - ( (position == -1 ? i : position-1)*(ICON_PLAYER_WIDHT+2));

        GLfloat COLOR[] = {info[i].r, info[i].g, info[i].b, 1.0f};
        font_race->PrintShadow(info[i].time.c_str(), 30, ICON_PLAYER_WIDHT+x, y+5, COLOR);
        
        if(info[i].special_title.length() >0)
        {
            GLfloat const RED[] = { 1.0f, 0, 0, 1.0f};
            font_race->PrintShadow(info[i].special_title.c_str(), 30, ICON_PLAYER_WIDHT+x, y+5, RED );
        }
        
        glEnable(GL_CULL_FACE);

        bFirst = 0;
        // draw icon
        Material* players_gst = kart->getKartProperties()->getIconMaterial();
        // Hmm - if the same icon is displayed more than once in a row,
        // plib does only do the first setTexture, therefore nothing is
        // displayed for the remaining icons. So we have to call force() if
        // the same icon is displayed more than once in a row.
        if(last_players_gst==players_gst)
        {
            players_gst->getState()->force();
        }
        //The material of the icons should not have a non-zero alpha_ref value,
        //because if so the next call can make the text look aliased.
        players_gst -> apply ();
        last_players_gst = players_gst;
        glBegin ( GL_QUADS ) ;
        glColor4f    ( 1, 1, 1, 1 ) ;
        if (kart -> isPlayerKart ())
        {
            glTexCoord2f ( 0, 0 ) ; glVertex2i ( x                  , y                   ) ;
            glTexCoord2f ( 1, 0 ) ; glVertex2i ( x+ICON_PLAYER_WIDHT, y                   ) ;
            glTexCoord2f ( 1, 1 ) ; glVertex2i ( x+ICON_PLAYER_WIDHT, y+ICON_PLAYER_WIDHT ) ;
            glTexCoord2f ( 0, 1 ) ; glVertex2i ( x                  , y+ICON_PLAYER_WIDHT ) ;
        }
        else
        {
            glTexCoord2f ( 0, 0 ) ; glVertex2i ( x           , y            ) ;
            glTexCoord2f ( 1, 0 ) ; glVertex2i ( x+ICON_WIDTH, y            ) ;
            glTexCoord2f ( 1, 1 ) ; glVertex2i ( x+ICON_WIDTH, y+ICON_WIDTH ) ;
            glTexCoord2f ( 0, 1 ) ; glVertex2i ( x           , y+ICON_WIDTH ) ;
        }
        glEnd () ;

        // draw position (1st, 2nd...)
        glDisable(GL_CULL_FACE);
        char str[256];

        if(position != -1)
        {
            sprintf(str, "%d", position);
            font_race->PrintShadow(str, 33, x-7, y-4);
            
            // FIXME: translation
            if (kart->getPosition() == 1)
                font_race->PrintShadow("st", 13, x-7+17, y-4+17);
            else if (kart->getPosition() == 2)
                font_race->PrintShadow("nd", 13, x-7+17, y-4+17);
            else if (kart->getPosition() == 3)
                font_race->PrintShadow("rd", 13, x-7+17, y-4+17);
            else
                font_race->PrintShadow("th", 13, x-7+17, y-4+17);
        }
    } // next kart
    
    glEnable(GL_CULL_FACE);
}   // drawPlayerIcons

//-----------------------------------------------------------------------------
void RaceGUI::drawPowerupIcons ( Kart* player_kart, int offset_x,
                                     int offset_y, float ratio_x,
                                     float ratio_y                    )
{
    // If player doesn't have anything, do nothing.
    Powerup* powerup=player_kart->getPowerup();
    if(powerup->getType() == POWERUP_NOTHING) return;

    // Originally the hardcoded sizes were 320-32 and 400
    int x1 = (int)((user_config->m_width/2-32) * ratio_x) + offset_x ;
    int y1 = (int)(user_config->m_height*5/6 * ratio_y)      + offset_y;

    int nSize=(int)(64.0f*std::min(ratio_x, ratio_y));
    powerup->getIcon()->apply();

    int n  = player_kart->getNumPowerup() ;

    if ( n > 5 ) n = 5 ;
    if ( n < 1 ) n = 1 ;

    glBegin(GL_QUADS) ;
    glColor4f(1, 1, 1, 1 );

    for ( int i = 0 ; i < n ; i++ )
    {
        glTexCoord2f(0, 0); glVertex2i( i*30 + x1      , y1      );
        glTexCoord2f(1, 0); glVertex2i( i*30 + x1+nSize, y1      );
        glTexCoord2f(1, 1); glVertex2i( i*30 + x1+nSize, y1+nSize);
        glTexCoord2f(0, 1); glVertex2i( i*30 + x1      , y1+nSize);
    }   // for i
    glEnd () ;

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
    int x = (int)((user_config->m_width-24) * ratio_x) + offset_x;
    int y = (int)(250 * ratio_y) + offset_y;
    int w = (int)(16 * ratio_x);
    int h = (int)(user_config->m_height/4 * ratio_y);
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
void RaceGUI::drawSteering(Kart* kart, int offset_x, int offset_y,
                           float ratio_x, float ratio_y           )
{

    float minRatio = std::min(ratio_x, ratio_y);
#define WHEELWIDTH 64
    int width  = (int)(WHEELWIDTH*minRatio);
    int height = (int)(WHEELWIDTH*minRatio);
    offset_x += (int)((user_config->m_width-160)*ratio_x) - width;
    offset_y += (int)(6*ratio_y);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    // for now we display the maximum steering as a 45 degree angle.
    // One the steering angle for all karts are fixed, this should be
    // changed, so that the user gets feedback about how much steering
    // is currently done, since it will vary from kart to kart.
    float displayedAngle = 45.0f * kart->getSteerPercent();

    int tw = width/2; int th = height/2;
    glTranslatef( (float)(offset_x+tw), (float)(offset_y+th), 0.0f);
    glRotatef(displayedAngle, 0.0f, 0.0f, 1.0f);
    glTranslatef((float)(-offset_x-tw), (float)(-offset_y-th), 0.0f);

    m_steering_wheel_icon->getState()->force();
    glBegin ( GL_QUADS ) ;
    glColor4f    ( 1, 1, 1, 1 ) ;
    glTexCoord2f(0, 0);glVertex2i(offset_x      , offset_y       );
    glTexCoord2f(1, 0);glVertex2i(offset_x+width, offset_y       );
    glTexCoord2f(1, 1);glVertex2i(offset_x+width, offset_y+height);
    glTexCoord2f(0, 1);glVertex2i(offset_x      , offset_y+height);
    glEnd () ;

    glPopMatrix();
} // drawSteering

//-----------------------------------------------------------------------------
void RaceGUI::drawPosition(Kart* kart, int offset_x, int offset_y,
                           float ratio_x, float ratio_y           )
{
    // arenas don't have a position (rank)
    if(RaceManager::getTrack()->isArena()) return;
    
    if(kart->getPosition() == -1) return;
    
    char str[256];
    offset_x += (int)((user_config->m_width-110)*ratio_x);
    offset_y += (int)(140*ratio_y);

    sprintf(str, "%d", kart->getPosition());
    font_race->PrintShadow(str, (int)(100*ratio_y), offset_x, offset_y);

    offset_x += (int)(50*ratio_x);
    offset_y += (int)(50*ratio_y);

    // FIXME: translation
    if (kart->getPosition() == 1)
        font_race->PrintShadow("st", (int)(40*ratio_y), offset_x, offset_y);
    else if (kart->getPosition() == 2)
        font_race->PrintShadow("nd", (int)(40*ratio_y), offset_x, offset_y);
    else if (kart->getPosition() == 3)
        font_race->PrintShadow("rd", (int)(40*ratio_y), offset_x, offset_y);
    else
        font_race->PrintShadow("th", (int)(40*ratio_y), offset_x, offset_y);
} // drawPosition

//-----------------------------------------------------------------------------
void RaceGUI::drawSpeed(Kart* kart, int offset_x, int offset_y,
                        float ratio_x, float ratio_y           )
{

    float minRatio = std::min(ratio_x, ratio_y);
#define SPEEDWIDTH 128
    int width  = (int)(SPEEDWIDTH*minRatio);
    int height = (int)(SPEEDWIDTH*minRatio);
    offset_x += (int)((user_config->m_width-10)*ratio_x) - width;
    offset_y += (int)(10*ratio_y);

    glMatrixMode(GL_MODELVIEW);
    m_speed_back_icon->getState()->force();
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
        if (stk_config->m_game_style==STKConfig::GS_WHEELIE && 
            speed >= kart->getMaxSpeed()*kart->getWheelieMaxSpeedRatio() )
        {
            font_race->PrintShadow("l", (int)(60*minRatio), 
                                   offset_x+(int)(70*minRatio), offset_y);
            font_race->PrintShadow("^", (int)(60*minRatio), 
                                   offset_x+(int)(65*minRatio), 
                                   offset_y+(int)(7*minRatio));
        }

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

} // drawSpeed

//-----------------------------------------------------------------------------
void RaceGUI::drawLap(const KartIconDisplayInfo* info, Kart* kart, int offset_x,
                      int offset_y, float ratio_x, float ratio_y           )
{
    // Don't display laps in follow the leader mode
    if(!RaceManager::getWorld()->raceHasLaps()) return;
    
    const int lap = info[kart->getWorldKartId()].lap;
    
    if(lap<0) return;  // don't display 'lap 0/...', or do nothing if laps are disabled (-1)
    float maxRatio = std::max(ratio_x, ratio_y);
    char str[256];
    offset_x += (int)(120*ratio_x);
    offset_y += (int)(70*maxRatio);

    if(kart->hasFinishedRace())
    {
        sprintf(str, _("Finished"));
        font_race->PrintShadow(str, (int)(48*maxRatio), offset_x, offset_y);
    }
    else
    {
        font_race->PrintShadow( _("Lap"), (int)(48*maxRatio), offset_x, offset_y);

        offset_y -= (int)(50*ratio_y);

        sprintf(str, "%d/%d", lap < 0 ? 0 : lap+1, 
                race_manager->getNumLaps());
        font_race->PrintShadow(str, (int)(48*maxRatio), offset_x, offset_y);
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
    y = (int)(ratio_y*(user_config->m_height -164)+offset_y);
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
                          offset_x, offset_x+(int)(user_config->m_width*ratio_x));
        // Add 20% of font size as space between the lines
        y-=msg.m_font_size*12/10;
        
        
    }   // for i in all messages
}   // drawAllMessages

//-----------------------------------------------------------------------------
/** Adds a message to the message queue. The message is displayed for a
 *  certain amount of time (unless time<0, then the message is displayed
 *  once).
 **/
void RaceGUI::addMessage(const char *msg, Kart *kart, float time, 
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
void RaceGUI::drawStatusText(const float dt)
{
    assert(RaceManager::getWorld() != NULL);

    glMatrixMode   ( GL_MODELVIEW ) ;
    glPushMatrix   () ;
    glLoadIdentity () ;

    glMatrixMode   ( GL_PROJECTION ) ;
    glPushMatrix   () ;
    glLoadIdentity () ;

    glPushAttrib   ( GL_ENABLE_BIT | GL_LIGHTING_BIT ) ;
    glDisable      ( GL_DEPTH_TEST   );
    glDisable      ( GL_LIGHTING     );
    glDisable      ( GL_FOG          );
    glDisable      ( GL_CULL_FACE    );
    glEnable       ( GL_ALPHA_TEST   );
    glAlphaFunc    ( GL_GREATER, 0.1f);
    glEnable       ( GL_BLEND        );

    glOrtho        ( 0, user_config->m_width, 0, user_config->m_height, 0, 100 ) ;
    switch (RaceManager::getWorld()->getPhase())
    {
    case READY_PHASE:
        {
            GLfloat const COLORS[] = { 0.9f, 0.66f, 0.62f, 1.0f };
            //I18N: as in "ready, set, go", shown at the beginning of the race
            font_race->PrintShadow( _("Ready!"), 90,
                                   Font::CENTER_OF_SCREEN,
                                   Font::CENTER_OF_SCREEN,
                                   COLORS );
        }
        break;
    case SET_PHASE:
        {
            GLfloat const COLORS[] = { 0.9f, 0.9f, 0.62f, 1.0f };
            //I18N: as in "ready, set, go", shown at the beginning of the race
            font_race->PrintShadow( _("Set!"), 90,
                                   Font::CENTER_OF_SCREEN,
                                   Font::CENTER_OF_SCREEN,
                                   COLORS );
        }
        break;
    case GO_PHASE:
        {
            GLfloat const COLORS[] = { 0.39f, 0.82f, 0.39f, 1.0f };
            //I18N: as in "ready, set, go", shown at the beginning of the race
            font_race->PrintShadow( _("Go!"), 90, 
                                   Font::CENTER_OF_SCREEN,
                                   Font::CENTER_OF_SCREEN,
                                   COLORS );
        }
        break;
    default: 
         break;
    }   // switch

    for(int i = 0; i < 10; ++i)
    {
        if(RaceManager::getWorld()->m_debug_text[i] != "")
        {
            GLfloat const COLORS[] = { 0.39f, 0.82f, 0.39f, 1.0f };
            font_race->Print( RaceManager::getWorld()->m_debug_text[i].c_str(),
                             20, 20, 200 -i*20, COLORS );
        }
    }
    // The penalty message needs to be displayed for up to one second
    // after the start of the race, otherwise it disappears if 
    // "Go" is displayed and the race starts
    if(RaceManager::getWorld()->isStartPhase() || RaceManager::getWorld()->getTime()<1.0f)
    {
        for(unsigned int i=0; i<race_manager->getNumLocalPlayers(); i++)
        {
            if(RaceManager::getWorld()->getLocalPlayerKart(i)->earlyStartPenalty())
            {
                GLfloat const COLORS[] = { 0.78f, 0.025f, 0.025f, 1.0f };
                font_race->PrintShadow( _("Penalty time!!"), 80,
                                       Font::CENTER_OF_SCREEN, 200,
                                        COLORS );
            }   // if penalty
        }  // for i < getNumPlayers
    }  // if not RACE_PHASE

    float split_screen_ratio_x, split_screen_ratio_y;
    split_screen_ratio_x = split_screen_ratio_y = 1.0;
    if(race_manager->getNumLocalPlayers() >= 2)
        split_screen_ratio_y = 0.5;
    if(race_manager->getNumLocalPlayers() >= 3)
        split_screen_ratio_x = 0.5;

    if ( RaceManager::getWorld()->isRacePhase() )
    {
        KartIconDisplayInfo* info = RaceManager::getWorld()->getKartsDisplayInfo(this);
        
        const int numPlayers = race_manager->getNumLocalPlayers();

        for(int pla = 0; pla < numPlayers; pla++)
        {
            int offset_x, offset_y;
            offset_x = offset_y = 0;

            if(numPlayers == 2)
            {
              if(pla == 0) offset_y = user_config->m_height/2;
            }
            else if (numPlayers == 3)
            {
              if (pla == 0  || pla == 1)
                offset_y = user_config->m_height/2;
              else
              {
                // Fixes width for player 3
                split_screen_ratio_x = 1.0;
              }

              if (pla == 1)
                offset_x = user_config->m_width/2;

            }
            else if(numPlayers == 4)
            {
              if(pla == 0  || pla == 1)
                offset_y = user_config->m_height/2;

              if((pla == 1) || pla == 3)
                offset_x = user_config->m_width/2;
            }

            Kart* player_kart = RaceManager::getWorld()->getLocalPlayerKart(pla);
            drawPowerupIcons(player_kart, offset_x, offset_y,
                                 split_screen_ratio_x, split_screen_ratio_y );
            drawEnergyMeter     (player_kart, offset_x, offset_y,
                                 split_screen_ratio_x, split_screen_ratio_y );
            drawSteering        (player_kart, offset_x, offset_y,
                                 split_screen_ratio_x, split_screen_ratio_y );
            drawPosition        (player_kart, offset_x, offset_y,
                                 split_screen_ratio_x, split_screen_ratio_y );
            drawSpeed           (player_kart, offset_x, offset_y,
                                 split_screen_ratio_x, split_screen_ratio_y );
            drawLap             (info, player_kart, offset_x, offset_y,
                                 split_screen_ratio_x, split_screen_ratio_y );
            drawAllMessages     (player_kart, offset_x, offset_y,
                                 split_screen_ratio_x, split_screen_ratio_y );
        }   // next player
        
        drawTimer();
        
        if(RaceManager::getWorld()->getPhase() == GO_PHASE ||
           RaceManager::getWorld()->getPhase() == MUSIC_PHASE)
        {
            drawMusicDescription();
        }

            
        drawMap();
        if ( user_config->m_display_fps ) drawFPS();
        
        drawPlayerIcons(info);
        
    }   // if RACE_PHASE

    glPopAttrib  () ;
    glPopMatrix  () ;
    glMatrixMode ( GL_MODELVIEW ) ;
    glPopMatrix  () ;
}   // drawStatusText

/* EOF */
