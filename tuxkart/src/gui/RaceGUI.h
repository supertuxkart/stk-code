
//  $Id: RaceGUI.h,v 1.12 2004/09/02 13:48:42 rmcruz Exp $
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_RACEGUI_H
#define HEADER_RACEGUI_H

#include "BaseGUI.h"
#include "material.h"
//#include <plib/fnt.h>
#include <SDL_ttf.h>
#include <map>

#define MAX_STRING          30
#define MAX_STRING_LENGTH  256

#define SCREEN_CENTERED_TEXT -1

class RaceGUI: public BaseGUI
{
public:
	RaceGUI();
	~RaceGUI();
	
	void update(float dt);
	void select() {}
	void keybd(const SDL_keysym& key);
	void point(int x, int y) { (void)x; (void)y; }
	void stick(int x, int y) { (void)x; (void)y; }
	
private:	
	void drawFPS();
	
	int fps_id;
	
	Material *herringbones_gst ;
	Material *herring_gst ;
	Material *spark_gst ;
	Material *missile_gst ;
	Material *flamemissile_gst ;
	Material *magnet_gst ;
	Material *zipper_gst ;
	
	double time_left ;

  /* Display informat on screen */
	void drawStatusText (const RaceSetup& raceSetup);
	void drawEnergyMeter ( float state, int offset_x, int offset_y, float ratio_x, float ratio_y );
	void drawCollectableIcons ( int player_nb, int offset_x, int offset_y, float ratio_x, float ratio_y );
	void drawEmergencyText ( int player_nb, int offset_x, int offset_y, float ratio_x, float ratio_y );
	void drawPlayerIcons ();
	void drawGameOverText ();
	void drawMap ();
	void drawScore (const RaceSetup& raceSetup, int player_nb, int offset_x, int offset_y, float ratio_x, float ratio_y);
	void drawTimer ();

  /* Text drawing */
	void drawText ( char* text, int sz, int x, int y, int red, int green, int blue );
  // This functions allows one to draw text by specifying both width and height
  // and so be able to do a stretching
	void drawStretchedText ( char *text, int x, int y, int w, int h, int red, int green, int blue );

	void drawDropShadowText ( char *str, int sz, int x, int y );
	void drawInverseDropShadowText ( char *str, int sz, int x, int y );


	char *pos_string [10];
	
	//debugging arrays and functions, never actually get used for anything at the moment
	void stToggle ();
	void stPrintf ( char *fmt, ... );
	bool stats_enabled ;
	float tt[6] ;
	char debug_strings [ MAX_STRING ][ MAX_STRING_LENGTH ] ;
	int  next_string ;

	typedef std::map <int, TTF_Font*> FontsCache;
	FontsCache fonts_cache;
};

#endif

