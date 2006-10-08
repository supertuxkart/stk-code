//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Patrick Ammann <pammann@aro.ch>
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

#include <assert.h>

#include "sound_manager.hpp"
#include "config.hpp"

#define USE_PLIB_SOUND !(HAVE_OPENAL && HAVE_MIKMOD)
#if USE_PLIB_SOUND
#include "sound_plib.hpp"
#else
#include <AL/al.h>
#include <AL/alut.h>

#include "music_mikmod.hpp"
#include "sfx_openal.hpp"
#endif

SoundManager* sound_manager= NULL;

SoundManager::SoundManager() {
  m_currentMusic= NULL;

  init();
}

void SoundManager::init() {
  if(config->music || config->sfx) {
#if USE_PLIB_SOUND
    plib_scheduler= new slScheduler;
    plib_scheduler->setSafetyMargin(0.25);
#else
    alutInit(0, NULL);  // init openAL sound system
    alGetError(); // Clear Error Code (so we can catch any new errors)
#endif
  }

  if (config->sfx) {
    // must be in sync with enumSoundSFX
    SFX* sfx= NULL;
    sfx = new SFXImpl("wavs/ugh.wav");       m_SFXs[SOUND_UGH]= sfx;
    sfx = new SFXImpl("wavs/radio/grandprix_winner.wav"); m_SFXs[SOUND_WINNER] = sfx;
    sfx = new SFXImpl("wavs/tintagel/grab_collectable.wav"); m_SFXs[SOUND_GRAB] = sfx;
    sfx = new SFXImpl("wavs/tintagel/crash.wav"); m_SFXs[SOUND_CRASH] = sfx;
    sfx = new SFXImpl("wavs/radio/shot.wav"); m_SFXs[SOUND_SHOT] = sfx;
    sfx = new SFXImpl("wavs/ow.wav"); m_SFXs[SOUND_OW] = sfx;
    sfx = new SFXImpl("wavs/explosion.wav"); m_SFXs[SOUND_EXPLOSION] = sfx;
    sfx = new SFXImpl("wavs/bzzt.wav"); m_SFXs[SOUND_BZZT] = sfx;
    sfx = new SFXImpl("wavs/radio/horn.wav"); m_SFXs[SOUND_BEEP] = sfx;
    sfx = new SFXImpl("wavs/radio/slap.wav"); m_SFXs[SOUND_USE_ANVIL] = sfx;
    sfx = new SFXImpl("wavs/radio/squeaky.wav"); m_SFXs[SOUND_USE_PARACHUTE] = sfx;
    sfx = new SFXImpl("wavs/wee.wav"); m_SFXs[SOUND_WEE] = sfx;

    //FIXME: The following 3 sounds are not used in the game yet.
    sfx = new SFXImpl("wavs/tintagel/deselect_option.wav"); m_SFXs[SOUND_BACK_MENU] = sfx;
    //sfx = new SFXImpl("wavs/tintagel/select_option.wav"); m_SFXs[SOUND_SELECT_MENU] = sfx;
    sfx = new SFXImpl("wavs/tintagel/move_option.wav"); m_SFXs[SOUND_MOVE_MENU] = sfx;

    sfx = new SFXImpl("wavs/tintagel/energy_bar_full.wav"); m_SFXs[SOUND_FULL] = sfx;
    sfx = new SFXImpl("wavs/tintagel/pre_start_race.wav"); m_SFXs[SOUND_PRESTART] = sfx;
    sfx = new SFXImpl("wavs/tintagel/start_race.wav"); m_SFXs[SOUND_START] = sfx;
    sfx = new SFXImpl("wavs/radio/radarping.wav"); m_SFXs[SOUND_MISSILE_LOCK] = sfx;
    sfx = new SFXImpl("wavs/radio/trafficjam.wav"); m_SFXs[SOUND_TRAFFIC_JAM] = sfx;
  }
}

SoundManager::~SoundManager() {
  // SFX cleanup
  for(SFXsType::iterator it= m_SFXs.begin(); it != m_SFXs.end(); it++) {
    SFX* sfx= it->second;
    delete sfx; 
  }
  m_SFXs.empty();

  if (m_currentMusic != NULL) {
    delete m_currentMusic;
    m_currentMusic = NULL;
  }

  if(config->music || config->sfx) {
#if USE_PLIB_SOUND
    delete plib_scheduler;
#else
    alutExit();
#endif
  }
}

void SoundManager::playSfx(unsigned int id) {
  if(config->sfx) {
    SFXsType::iterator it= m_SFXs.find(id);
    if (it == m_SFXs.end()) {
      assert(false);
      return;
    }
    SFX* sfx= it->second;
    sfx->play();
  }
}

void SoundManager::playMusic(const char* filename) {
  if(config->music)
  {
      printf("Music: %s\n", filename);

      if (m_currentMusic != NULL) {
        delete m_currentMusic;
      }

      if (filename == NULL || strlen(filename) == 0) {
        // nothing to play
        return;
      }

      #if USE_PLIB_SOUND
        m_currentMusic= new MusicPlib();
      #else
        m_currentMusic= new MusicMikMod();
      #endif
        assert(m_currentMusic->load(filename));
        m_currentMusic->playMusic();
  }
}

void SoundManager::stopMusic() {
  if (m_currentMusic != NULL) {
    m_currentMusic->stopMusic();
  }
}

void SoundManager::pauseMusic() {
  if (m_currentMusic != NULL) {
    m_currentMusic->pauseMusic();
  }

}

void SoundManager::resumeMusic() {
  if (m_currentMusic != NULL) {
    m_currentMusic->resumeMusic();
  }
}

void SoundManager::update() {
  if (m_currentMusic != NULL) {
    m_currentMusic->update();
  }
}

