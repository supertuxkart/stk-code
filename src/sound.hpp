//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
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

#ifndef HEADER_SOUND_H
#define HEADER_SOUND_H

#include <string>

#include <plib/sl.h>

enum allSoundsType {SOUND_UGH,  SOUND_WINNER, SOUND_CRASH, SOUND_GRAB,
		    SOUND_SHOT, SOUND_OW,     SOUND_WEE,   SOUND_EXPLOSION,
		    SOUND_BZZT, SOUND_BEEP,   SOUND_BACK_MENU, 
		    SOUND_SELECT_MENU, SOUND_MOVE_MENU, SOUND_FULL, 
		    SOUND_PRESTART, SOUND_START, SOUND_MISSILE_LOCK,
		    SOUND_TRAFFIC_JAM, NUM_SOUNDS};


using std::string;

class Sound
{
public:
	Sound() { s = NULL ; }
   string fname ;
   slSample *s ;
} ;

class SoundSystem
{
  char current_track [ 256 ] ;
  slScheduler *sched ;

  Sound sfx [NUM_SOUNDS] ;

public:
  SoundSystem () ;

  void update () ;
  void playSfx ( int sound ) ;

  void setSafetyMargin ( float t = 0.25 )
  {
    sched -> setSafetyMargin ( t ) ;
  }

  void play_track  ( const char *fname );
  void play_track  ( const std::string& fname ) {play_track(fname.c_str());}
  void stop_music  ()                           {sched -> stopMusic();     }
  void pause_music ()                           { sched -> pauseMusic();   }
  void resume_music()                           { sched -> resumeMusic();  }
} ;

extern SoundSystem *sound ;

#endif

/* EOF */
