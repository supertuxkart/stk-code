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
//  You should have received a copy of the GNU Getneral Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <iostream>

#include "sound.hpp"
#include "loader.hpp"
#include "config.hpp"

SoundSystem *sound ;

void SoundSystem::play_track ( const char *fname )
{
  if ( fname == NULL )
  {
      std::cerr << "WARNING: tried to play a NULL file\n";
      return;
  }

  if(config->music)
  {
      std::string PATH = loader->getPath(fname);

      if ( strcmp ( PATH.c_str(), current_track ) != 0  )
      {
        strcpy ( current_track, PATH.c_str() ) ;

        sched -> stopMusic () ;
        sched -> loopMusic ( current_track ) ;
      }
  }
}

void SoundSystem::playSfx ( int sfx_num )
{
    if(config->sfx)
        sched -> playSample ( sfx[sfx_num].s, 1, SL_SAMPLE_MUTE, 2, NULL ) ;
}


SoundSystem::SoundSystem ()
{
  sched = new slScheduler ;

  sfx[SOUND_UGH         ].fname = "wavs/ugh.wav";
  sfx[SOUND_WINNER      ].fname = "wavs/radio/grandprix_winner.wav";
  sfx[SOUND_CRASH       ].fname = "wavs/tintagel/crash.wav";
  sfx[SOUND_GRAB        ].fname = "wavs/tintagel/grab_collectable.wav";
  sfx[SOUND_SHOT        ].fname = "wavs/radio/shot.wav";
  sfx[SOUND_OW          ].fname = "wavs/ow.wav";
  sfx[SOUND_WEE         ].fname = "wavs/wee.wav";
  sfx[SOUND_EXPLOSION   ].fname = "wavs/explosion.wav";
  sfx[SOUND_BZZT        ].fname = "wavs/bzzt.wav";
  sfx[SOUND_BEEP        ].fname = "wavs/radio/horn.wav";

  //FIXME: The following 3 sounds are not used in the game yet.
  sfx[SOUND_BACK_MENU   ].fname = "wavs/tintagel/deselect_option.wav";
  sfx[SOUND_SELECT_MENU ].fname = "wavs/tintagel/select_option.wav";
  sfx[SOUND_MOVE_MENU   ].fname = "wavs/tintagel/move_option.wav";

  sfx[SOUND_FULL        ].fname = "wavs/tintagel/energy_bar_full.wav";
  sfx[SOUND_PRESTART    ].fname = "wavs/tintagel/pre_start_race.wav";
  sfx[SOUND_START       ].fname = "wavs/tintagel/start_race.wav";
  sfx[SOUND_MISSILE_LOCK].fname = "wavs/radio/radarping.wav";

  setSafetyMargin () ;

  for (int i = 0; i != NUM_SOUNDS; i++)
  {
    std::string path = loader->getPath(sfx[i].fname.c_str());
      sfx[i].s  = new slSample ( path.c_str(), sched ) ;
  }

  play_track ( "" ) ;
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
