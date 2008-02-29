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
#include <fstream>

#include "sound_manager.hpp"
#include "user_config.hpp"
#include "string_utils.hpp"
#include "gui/font.hpp"
#include "loader.hpp"

#define USE_PLIB_SOUND !(HAVE_OPENAL && HAVE_OGGVORBIS)
#if USE_PLIB_SOUND
#  include "sound_plib.hpp"
#else    //We use OpenAL
#  ifdef __APPLE__
#    include <OpenAL/al.h>
#  else
#    include <AL/al.h>
#  endif
#  include <AL/alut.h>

#  if HAVE_OGGVORBIS
#    include "music_ogg.hpp"
#  endif
#  include "sfx_openal.hpp"
#endif // USE_PLIB_SOUND

#include "translation.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define strcasecmp _strcmpi
#endif
SoundManager* sound_manager= NULL;

SoundManager::SoundManager()
{
    m_current_music = NULL;


#if USE_PLIB_SOUND
    plib_scheduler = new slScheduler;

    if(plib_scheduler->notWorking())
    {
        fprintf(stderr, _("WARNING: Could not initialize the PLIB based sound.\n"));

        plib_scheduler->setSafetyMargin(0.25);
        m_initialized = false;
    }
    else
        m_initialized = true;
#else
    if(alutInit(0, NULL) == AL_TRUE)  // init OpenAL sound system
        m_initialized = true;
    else
    {
        fprintf(stderr, _("WARNING: Could not initialize the ALUT based sound.\n"));
        m_initialized = false;
    }

    alGetError(); //Called here to clear any non-important errors found
#endif

    if (m_initialized)
    {
        // must be in sync with enumSoundSFX
        SFX* sfx= NULL;
        sfx = new SFXImpl("ugh.wav");       m_sfxs[SOUND_UGH]= sfx;
        sfx = new SFXImpl("radio/grandprix_winner.wav"); m_sfxs[SOUND_WINNER] = sfx;
        sfx = new SFXImpl("tintagel/grab_collectable.wav"); m_sfxs[SOUND_GRAB] = sfx;
        sfx = new SFXImpl("tintagel/crash.wav"); m_sfxs[SOUND_CRASH] = sfx;
        sfx = new SFXImpl("radio/shot.wav"); m_sfxs[SOUND_SHOT] = sfx;
        sfx = new SFXImpl("explosion.wav"); m_sfxs[SOUND_EXPLOSION] = sfx;
        sfx = new SFXImpl("bzzt.wav"); m_sfxs[SOUND_BZZT] = sfx;
        sfx = new SFXImpl("radio/horn.wav"); m_sfxs[SOUND_BEEP] = sfx;
        sfx = new SFXImpl("radio/slap.wav"); m_sfxs[SOUND_USE_ANVIL] = sfx;
        sfx = new SFXImpl("radio/squeaky.wav"); m_sfxs[SOUND_USE_PARACHUTE] = sfx;
        sfx = new SFXImpl("wee.wav"); m_sfxs[SOUND_WEE] = sfx;

        sfx = new SFXImpl("tintagel/deselect_option.wav"); m_sfxs[SOUND_BACK_MENU] = sfx;
        sfx = new SFXImpl("tintagel/select_option.wav"); m_sfxs[SOUND_SELECT_MENU] = sfx;
        sfx = new SFXImpl("tintagel/move_option.wav"); m_sfxs[SOUND_MOVE_MENU] = sfx;
        sfx = new SFXImpl("tintagel/energy_bar_full.wav"); m_sfxs[SOUND_FULL] = sfx;
        sfx = new SFXImpl("tintagel/pre_start_race.wav"); m_sfxs[SOUND_PRESTART] = sfx;
        sfx = new SFXImpl("tintagel/start_race.wav"); m_sfxs[SOUND_START] = sfx;
        sfx = new SFXImpl("radio/radarping.wav"); m_sfxs[SOUND_MISSILE_LOCK] = sfx;
    }
}  // SoundManager

//-----------------------------------------------------------------------------
SoundManager::~SoundManager()
{
    // SFX cleanup
    for(SFXsType::iterator it= m_sfxs.begin(); it != m_sfxs.end(); it++)
    {
        SFX* sfx= it->second;
        delete sfx;
    }
    m_sfxs.empty();

    if (m_current_music != NULL)
    {
        delete m_current_music;
        m_current_music = NULL;
    }

    if(m_initialized)
    {
#if USE_PLIB_SOUND
        delete plib_scheduler;
#else
        alutExit();
#endif

    }
}   // ~SoundManager

//-----------------------------------------------------------------------------
void SoundManager::playSfx(unsigned int id)
{
    if(user_config->doSFX() && m_initialized)
    {
        SFXsType::iterator it= m_sfxs.find(id);
        if (it == m_sfxs.end())
        {
            assert(false);
            return;
        }
        SFX* sfx= it->second;
        sfx->play();
    }
}   // playSfx

//-----------------------------------------------------------------------------
void SoundManager::playMusic(const std::string& filename)
{
    m_description.clear();
    m_current_music_file = filename;
    if(!user_config->doMusic() || !m_initialized) return;

    if (m_current_music != NULL)
    {
        delete m_current_music;
        m_current_music = NULL;
    }

    if (filename == "" || strlen(filename.c_str()) == 0)
    {
        // nothing to play
        return;
    }

#if USE_PLIB_SOUND
    if (!strcasecmp(".mod", filename.c_str()+filename.size()-4))
        m_current_music= new MusicPlib();
#endif
#if HAVE_OGGVORBIS
    if (!strcasecmp(".ogg", filename.c_str()+filename.size()-4))
        m_current_music= new MusicOggStream();
#endif
    if(m_current_music == NULL)	// no support for file
    {
      fprintf(stderr, "WARNING: music file %s format not recognized.\n", filename.c_str());
        return;
    }

    if((m_current_music->load(filename)) == false)
    {
        delete m_current_music;
        m_current_music=0;
	fprintf(stderr, "WARNING: Unabled to load music %s, not supported or not found.\n", 
                filename.c_str());
        return;
    }

    m_current_music->playMusic();

    // Read up to two lines from the corresponding .readme file: first one 
    // the title, second the composer. This is then displayed by the race gui
    std::string name_readme;
    try
    {
        name_readme = loader->getMusicFile(
                            StringUtils::without_extension(filename)+".readme");
    }
    catch(std::exception)
    {
        // Silently ignore any missing .readme files, m_description was cleared above
        return;
    }
    std::ifstream f(name_readme.c_str());
    if(f)
    { 
        std::string s;
        std::getline(f,s); if(!f.eof()) m_description.push_back(s);
        std::getline(f,s); if(!f.eof()) m_description.push_back(s);
        f.close();
    }
}   // playMusic

//-----------------------------------------------------------------------------
void SoundManager::stopMusic()
{
    if (m_current_music != NULL)
    {
        m_current_music->stopMusic();
    }
    m_description.clear();
}   // stopMusic

//-----------------------------------------------------------------------------
void SoundManager::pauseMusic()
{
    if (m_current_music != NULL)
    {
        m_current_music->pauseMusic();
    }

}   // pauseMusic

//-----------------------------------------------------------------------------
void SoundManager::resumeMusic()
{
    if (m_current_music != NULL)
    {
        m_current_music->resumeMusic();
    }
}   // resumeMusic

//-----------------------------------------------------------------------------
void SoundManager::update()
{
    if (m_current_music != NULL)
    {	
    	m_current_music->update();
    }
}   // update

