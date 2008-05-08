//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Patrick Ammann <pammann@aro.ch>
//  Copyright (C) 2008 Patrick Ammann <pammann@aro.ch>, Joerg Henrichs
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
#include <fstream>

#include "sound_manager.hpp"
#include "user_config.hpp"
#include "string_utils.hpp"
#include "gui/font.hpp"
#include "file_manager.hpp"

#ifdef __APPLE__
#  include <OpenAL/al.h>
#else
#  include <AL/al.h>
#endif
#include <AL/alut.h>

#include "music_ogg.hpp"
#include "sfx_openal.hpp"

#include "translation.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define strcasecmp _strcmpi
#endif
SoundManager* sound_manager= NULL;

SoundManager::SoundManager() : m_sfxs(NUM_SOUNDS)
{
    m_current_music= NULL;
    if(alutInit(0, NULL) == AL_TRUE)  // init OpenAL sound system
        m_initialized = true;
    else
    {
        fprintf(stderr, "WARNING: Could not initialize the ALUT based sound.\n");
        m_initialized = false;
    }

    alGetError(); //Called here to clear any non-important errors found

    if (m_initialized)
    {
        // must be in sync with enumSoundSFX
        m_sfxs[SOUND_UGH          ] = new SFXImpl("ugh.wav");
        m_sfxs[SOUND_WINNER       ] = new SFXImpl("radio/grandprix_winner.wav");
        m_sfxs[SOUND_GRAB         ] = new SFXImpl("tintagel/grab_collectable.wav");
        m_sfxs[SOUND_CRASH        ] = new SFXImpl("tintagel/crash.wav");
        m_sfxs[SOUND_SHOT         ] = new SFXImpl("radio/shot.wav");
        m_sfxs[SOUND_EXPLOSION    ] = new SFXImpl("explosion.wav");
        m_sfxs[SOUND_BZZT         ] = new SFXImpl("bzzt.wav");
        m_sfxs[SOUND_BEEP         ] = new SFXImpl("radio/horn.wav");
        m_sfxs[SOUND_USE_ANVIL    ] = new SFXImpl("radio/slap.wav");
        m_sfxs[SOUND_USE_PARACHUTE] = new SFXImpl("radio/squeaky.wav");
        m_sfxs[SOUND_WEE          ] = new SFXImpl("wee.wav"); 
        m_sfxs[SOUND_BACK_MENU    ] = new SFXImpl("tintagel/deselect_option.wav");
        m_sfxs[SOUND_SELECT_MENU  ] = new SFXImpl("tintagel/select_option.wav"); 
        m_sfxs[SOUND_MOVE_MENU    ] = new SFXImpl("tintagel/move_option.wav");
        m_sfxs[SOUND_FULL         ] = new SFXImpl("tintagel/energy_bar_full.wav");
        m_sfxs[SOUND_PRESTART     ] = new SFXImpl("tintagel/pre_start_race.wav");
        m_sfxs[SOUND_START        ] = new SFXImpl("tintagel/start_race.wav");
        m_sfxs[SOUND_MISSILE_LOCK ] = new SFXImpl("radio/radarping.wav");
    }

    loadMusicInformation();
}  // SoundManager

//-----------------------------------------------------------------------------
SoundManager::~SoundManager()
{
    // SFX cleanup
    for(SFXsType::iterator it= m_sfxs.begin(); it != m_sfxs.end(); it++)
    {
        delete *it;
    }
    m_sfxs.empty();

    if(m_initialized)
    {
        alutExit();
    }
}   // ~SoundManager

//-----------------------------------------------------------------------------
void SoundManager::loadMusicInformation()
{
    // Load music files from data/music, and dirs defined in SUPERTUXKART_MUSIC_PATH
    std::vector<std::string> allMusicDirs=file_manager->getMusicDirs();
    for(std::vector<std::string>::iterator dir=allMusicDirs.begin();
                                           dir!=allMusicDirs.end(); dir++)
    {
        loadMusicFromOneDir(*dir);
    }   // for dir
}   // loadMusicInformation

//-----------------------------------------------------------------------------
void SoundManager::loadMusicFromOneDir(const std::string& dir)
{
    std::set<std::string> files;
    file_manager->listFiles(files, dir, /*is_full_path*/ true,
                            /*make_full_path*/ true);
    for(std::set<std::string>::iterator i = files.begin(); i != files.end(); ++i)
    {
        if(StringUtils::extension(*i)!="music") continue;
        m_allMusic[StringUtils::basename(*i)] = new MusicInformation(*i);
    }   // for i
} // loadMusicFromOneDir

//-----------------------------------------------------------------------------
void SoundManager::addMusicToTracks()
{
    for(std::map<std::string,MusicInformation*>::iterator i=m_allMusic.begin();
                                                          i!=m_allMusic.end(); i++)
    {
        i->second->addMusicToTracks();
    }
}   // addMusicToTracks

//-----------------------------------------------------------------------------
MusicInformation* SoundManager::getMusicInformation(const std::string& filename)
{
    if(filename=="")
    {
        return NULL;
    }
    const std::string basename = StringUtils::basename(filename);
    MusicInformation* mi = m_allMusic[basename];
    if(!mi)
    {
        mi = new MusicInformation(filename);
        m_allMusic[basename] = mi;
    }
    return mi;
}   // SoundManager
//-----------------------------------------------------------------------------
void SoundManager::playSfx(unsigned int id)
{
    if(!user_config->doSFX() || !m_initialized) return;

    assert(id>=0 && id<m_sfxs.size() && m_sfxs[id]);
    m_sfxs[id]->play();

}   // playSfx

//-----------------------------------------------------------------------------
void SoundManager::startMusic(MusicInformation* mi)
{
    m_current_music = mi;
    
    if(!user_config->doMusic() || !m_initialized) return;
    
    mi->startMusic();
}   // startMusic

//-----------------------------------------------------------------------------
void SoundManager::stopMusic()
{
    if(m_current_music) m_current_music->stopMusic();
    m_current_music=NULL;
}   // stopMusic

//-----------------------------------------------------------------------------
