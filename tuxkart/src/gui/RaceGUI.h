
//  $Id: RaceGUI.h,v 1.5 2004/08/23 12:04:54 rmcruz Exp $
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
#include <plib/fnt.h>

#define MAX_STRING          30
#define MAX_STRING_LENGTH  256

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
	bool show_fps;	
	
	Material *herringbones_gst ;
	Material *herring_gst ;
	Material *spark_gst ;
	Material *missile_gst ;
	Material *flamemissile_gst ;
	Material *magnet_gst ;
	Material *zipper_gst ;
	
	double time_left ;
	
	void drawStatusText (const RaceSetup& raceSetup);
	void drawEnergyMeter ( float state );
	void drawCollectableIcons ();
	void drawEmergencyText ();
	void drawPlayerIcons ();
	void drawGameRunningText (const RaceSetup& raceSetup);
	void drawGameOverText ();
	void drawMap ();
	void drawScore (const RaceSetup& raceSetup);
	void drawTimer ();
	void drawDropShadowText ( char *str, int sz, int x, int y );
	void drawInverseDropShadowText ( char *str, int sz, int x, int y );
	void drawText ( char *str, int sz, int x, int y );
	
	char *pos_string [10];
	
	//debugging arrays and functions, never actually get used for anything at the moment
	void stToggle ();
	void stPrintf ( char *fmt, ... );
	void drawStatsText () ;
	bool stats_enabled ;
	float tt[6] ;
	char debug_strings [ MAX_STRING ][ MAX_STRING_LENGTH ] ;
	int  next_string ;
	
	//FIXME: these probably want changing to use SDL_ttf
	fntRenderer *text ;
	fntTexFont *oldfont ;
};
#endif

