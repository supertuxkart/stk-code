//  $Id: sound.cxx,v 1.5 2004/08/01 18:47:14 jamesgregory Exp $
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

#include "tuxkart.h"
#include "sound.h"
#include "Loader.h"


void SoundSystem::disable_music ()
{
  sched -> stopMusic () ;
  sched -> update    () ;  /* Ugh! Nasty Kludge! */
  sched -> update    () ;  /* Ugh! Nasty Kludge! */

  music_off = TRUE  ;
}

void SoundSystem::pause_music()
{
  sched -> pauseMusic () ;
  //FIXME: I'm just copying disable_music, no idea if the following is neccessary, let alone twice
  sched -> update    () ;  /* Ugh! Nasty Kludge! */
  sched -> update    () ;  /* Ugh! Nasty Kludge! */
}

void SoundSystem::resume_music()
{
  sched -> resumeMusic () ;
  //FIXME: I'm just copying disable_music, no idea if the following is neccessary, let alone twice
  sched -> update    () ;  /* Ugh! Nasty Kludge! */
  sched -> update    () ;  /* Ugh! Nasty Kludge! */
}


void SoundSystem::change_track ( char *fname )
{
  if ( fname == NULL )
    fname = "" ;

  if ( strcmp ( fname, current_track ) != 0  )
  {
    strcpy ( current_track, fname ) ;

    if ( ! music_off )
      enable_music  () ;
  }
}

void SoundSystem::enable_music ()
{
  sched -> stopMusic () ;

  if ( current_track [ 0 ] != '\0' )
    sched -> loopMusic ( current_track ) ;
 
  music_off = FALSE ;
}


void SoundSystem::disable_sfx () { sfx_off = TRUE  ; }
void SoundSystem:: enable_sfx () { sfx_off = FALSE ; }



void SoundSystem::playSfx ( int sfx_num )
{
  if ( ! sfx_off )
    sched -> playSample ( sfx[sfx_num].s, 1, SL_SAMPLE_MUTE, 2, NULL ) ;
}


SoundSystem::SoundSystem ():
music_off(FALSE), sfx_off(FALSE)
{
  sched = new slScheduler ;
  
  sfx[SOUND_UGH].fname = "wavs/ugh.wav";
  sfx[SOUND_BOING].fname = "wavs/boing.wav";
  sfx[SOUND_BONK].fname = "wavs/bonk.wav";
  sfx[SOUND_BURP].fname = "wavs/burp.wav";
  sfx[SOUND_LASER].fname = "wavs/laser.wav";
  sfx[SOUND_OW].fname = "wavs/ow.wav";
  sfx[SOUND_WEE].fname = "wavs/wee.wav";
  sfx[SOUND_EXPLOSION].fname = "wavs/explosion.wav";
  sfx[SOUND_BZZT].fname = "wavs/bzzt.wav";
  sfx[SOUND_BEEP].fname = "wavs/horn.wav";
  sfx[SOUND_SHOOMF].fname = "wavs/shoomf.wav";

  setSafetyMargin () ;
  
  for (int i = 0; i != NUM_SOUNDS; i++)
  {
    std::string path = loader->getPath(sfx[i].fname);
      sfx[i].s  = new slSample ( path.c_str(), sched ) ;
  }

  enable_sfx   () ;
  change_track ( "" ) ;
  enable_music () ;
}


void SoundSystem::update ()
{
  /*
    Comment this next line out if the
    sound causes big glitches on your
    IRIX machine!
  */

  sched -> update () ;
}

/* EOF */
