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

#define SOUND_UGH		0
#define SOUND_WINNER    1
#define SOUND_CRASH		2
#define SOUND_GRAB		3
#define SOUND_SHOT		4
#define SOUND_OW		5
#define SOUND_WEE		6
#define SOUND_EXPLOSION         7
#define SOUND_BZZT              8
#define SOUND_BEEP              9
#define SOUND_BACK_MENU         10
#define SOUND_SELECT_MENU       11
#define SOUND_MOVE_MENU         12
#define SOUND_FULL              13
#define SOUND_PRESTART          14
#define SOUND_START             15

#define NUM_SOUNDS 16


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

  void play_track ( const char *fname );

  void stop_music () { sched -> stopMusic(); }
  void pause_music () { sched -> pauseMusic(); }
  void resume_music () { sched -> resumeMusic(); }
} ;

extern SoundSystem *sound ;

#endif

/* EOF */
