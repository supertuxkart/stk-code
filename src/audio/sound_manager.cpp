//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Patrick Ammann <pammann@aro.ch>
//  Copyright (C) 2008 Patrick Ammann <pammann@aro.ch>, Joerg Henrichs
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

#include "audio/sound_manager.hpp"

#include <assert.h>
#include <fstream>
#if defined(WIN32) && !defined(__CYGWIN__)
#  define strcasecmp _strcmpi
#  define snprintf _snprintf
#endif

#ifdef __APPLE__
#  include <OpenAL/al.h>
#  include <OpenAL/alc.h>
#else
#  include <AL/al.h>
#  include <AL/alc.h>
#endif

#include "user_config.hpp"
#include "string_utils.hpp"
#include "file_manager.hpp"
#include "translation.hpp"
#include "audio/music_ogg.hpp"
#include "audio/sfx_openal.hpp"

SoundManager* sound_manager= NULL;

SoundManager::SoundManager()
{
    m_current_music= NULL;

    ALCdevice* device = alcOpenDevice ( NULL ); //The default sound device
    if( device == NULL )
    {
        fprintf(stderr, "WARNING: Could not open the default sound device.\n");
        m_initialized = false;
    }
    else
    {

        ALCcontext* context = alcCreateContext( device, NULL );

        if( context == NULL )
        {
            fprintf(stderr, "WARNING: Could not create a sound context.\n");
            m_initialized = false;
        }
        else
        {
            alcMakeContextCurrent( context );
            m_initialized = true;
        }
    }

    alGetError(); //Called here to clear any non-important errors found

    loadMusicInformation();
}  // SoundManager

//-----------------------------------------------------------------------------
SoundManager::~SoundManager()
{
    if(m_initialized)
    {
        ALCcontext* context = alcGetCurrentContext();
        ALCdevice* device = alcGetContextsDevice( context );

        alcMakeContextCurrent( NULL );
        alcDestroyContext( context );

        alcCloseDevice( device );
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
        if(!i->second) 
        {
            fprintf(stderr, "Can't find music file '%s' - ignored.\n",
                    i->first.c_str());
            continue;
        }
        i->second->addMusicToTracks();
    }
}   // addMusicToTracks

//-----------------------------------------------------------------------------
void SoundManager::startMusic(MusicInformation* mi)
{
    m_current_music = mi;
    
    if(!mi || !user_config->doMusic() || !m_initialized) return;
    
    mi->startMusic();
}   // startMusic

//-----------------------------------------------------------------------------
void SoundManager::stopMusic()
{
    if(m_current_music) m_current_music->stopMusic();
}   // stopMusic

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

//----------------------------------------------------------------------------
void SoundManager::positionListener(Vec3 position, Vec3 front)
{
    if(!user_config->doSFX() || !m_initialized) return;

    //forward vector
    listenerVec[0] = front.getX(); 
    listenerVec[1] = front.getY();
    listenerVec[2] = front.getZ(); 
    //up vector
    listenerVec[3] = 0; 
    listenerVec[4] = 0;
    listenerVec[5] = 1;

    alListener3f(AL_POSITION, position.getX(), position.getY(), position.getZ());
    alListenerfv(AL_ORIENTATION, listenerVec);
}

