//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Patrick Ammann <pammann@aro.ch>
//  Copyright (C) 2008-2015 Patrick Ammann <pammann@aro.ch>, Joerg Henrichs
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

#include "audio/music_manager.hpp"

#include <assert.h>
#include <fstream>

#ifdef ENABLE_SOUND
#  include <AL/al.h>
#  include <AL/alc.h>
#endif

#include "audio/music_ogg.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_openal.hpp"
#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "utils/stk_process.hpp"
#include "utils/string_utils.hpp"

MusicManager* music_manager= NULL;


MusicManager::MusicManager()
{
    m_current_music= NULL;
    m_initialized = false;
    setMasterMusicVolume(UserConfigParams::m_music_volume);

    //FIXME: I'm not sure that this code goes here
#ifdef ENABLE_SOUND
    if (UserConfigParams::m_enable_sound)
    {
#if defined(__APPLE__) && !defined(NDEBUG)
        // HACK: On OSX, when OpenAL is initialized, breaking in a debugger 
        // causes my iTunes music to stop too, which is highly annoying ;) so in
        // debug mode, require a restart to enable sound
        if (UserConfigParams::m_sfx || UserConfigParams::m_music)
        {
#endif
            ALCdevice* device = alcOpenDevice(NULL); //The default sound device
            
            if (device == NULL)
            {
                Log::warn("MusicManager", "Could not open the default sound "
                                          "device.");
                m_initialized = false;
            }
            else
            {
                ALCcontext* context = alcCreateContext(device, NULL);
        
                if (context == NULL)
                {
                    Log::warn("MusicManager", "Could not create a sound "
                                              "context.");
                    m_initialized = false;
                }
                else
                {
                    alcMakeContextCurrent(context);
                    m_initialized = true;
                }
            }
#if defined(__APPLE__) && !defined(NDEBUG)
        }
#endif
        alGetError(); //Called here to clear any non-important errors found
    }
#endif

    loadMusicInformation();
}  // MusicManager

//-----------------------------------------------------------------------------
MusicManager::~MusicManager()
{
    for(std::map<std::string,MusicInformation*>::iterator
        i=m_all_music.begin(); i!=m_all_music.end(); i++)
    {
        delete i->second;
        i->second = NULL;
    }

#ifdef ENABLE_SOUND
    if(m_initialized)
    {
        ALCcontext* context = alcGetCurrentContext();
        ALCdevice* device = alcGetContextsDevice( context );

        alcMakeContextCurrent( NULL );
        alcDestroyContext( context );

        alcCloseDevice( device );
    }
#endif
}   // ~MusicManager

//-----------------------------------------------------------------------------
void MusicManager::loadMusicInformation()
{
    // Load music files from data/music, and dirs defined in
    // SUPERTUXKART_MUSIC_PATH
    std::vector<std::string> allMusicDirs=file_manager->getMusicDirs();
    for(std::vector<std::string>::iterator dir=allMusicDirs.begin();
                                           dir!=allMusicDirs.end(); dir++)
    {
        loadMusicFromOneDir(*dir);
    }   // for dir
}   // loadMusicInformation

 //----------------------------------------------------------------------------
void MusicManager::loadMusicFromOneDir(const std::string& dir)
{
    std::set<std::string> files;
    file_manager->listFiles(files, dir, /*is_full_path*/ true);
    for(std::set<std::string>::iterator i  = files.begin();
                                        i != files.end(); ++i)
    {
        if(StringUtils::getExtension(*i)!="music") continue;
        MusicInformation *mi =  MusicInformation::create(*i);
        if(mi)
            m_all_music[StringUtils::getBasename(*i)] = mi;
    }   // for i

} // loadMusicFromOneDir

//-----------------------------------------------------------------------------
void MusicManager::addMusicToTracks()
{
    for(std::map<std::string,MusicInformation*>::iterator
        i=m_all_music.begin(); i!=m_all_music.end(); i++)
    {
        if(!i->second)
        {
            Log::warn("MusicManager", "Can't find music file '%s' - ignored.",
                      i->first.c_str());
            continue;
        }
        i->second->addMusicToTracks();
    }
}   // addMusicToTracks

//-----------------------------------------------------------------------------
/** Special shortcut vor overworld (which skips other phases where the music
 *  would normally be started.
 */
void MusicManager::startMusic()
{
    if (m_current_music.load() != NULL && UserConfigParams::m_music)
        SFXManager::get()->queue(SFXManager::SFX_MUSIC_START, m_current_music);
}   // startMusic

//-----------------------------------------------------------------------------
/** Schedules the indicated music to be played next.
 *  \param mi Music information of the music to be played.
 *  \param start_right_now 
 */
void MusicManager::startMusic(MusicInformation* mi, bool start_right_now)
{
    if (STKProcess::getType() != PT_MAIN || !m_initialized)
        return;

    if (!UserConfigParams::m_music)
    {
        // Save it so it can be turned on later in options menu
        m_current_music = mi;
        return;
    }

    if (!mi)
    {
        // NULL music will stop current music
        clearCurrentMusic();
        return;
    }

    SFXManager::get()->queue(start_right_now ? SFXManager::SFX_MUSIC_START
                                             : SFXManager::SFX_MUSIC_WAITING,
                             mi);
}   // startMusic

//-----------------------------------------------------------------------------
/** Queues a stop current music event for the audio thread.
 */
void MusicManager::stopMusic()
{
    if (m_current_music.load() != NULL)
        SFXManager::get()->queue(SFXManager::SFX_MUSIC_STOP, m_current_music);
}   // stopMusic

//-----------------------------------------------------------------------------
/** Insert a command into the sfx queue to pause the current music.
 */
void MusicManager::pauseMusic()
{
    if (m_current_music.load() != NULL)
        SFXManager::get()->queue(SFXManager::SFX_MUSIC_PAUSE, m_current_music);
}   // pauseMusic

//-----------------------------------------------------------------------------
/** Inserts a resume current music event into the queue.
 */
void MusicManager::resumeMusic()
{
    if (m_current_music.load() != NULL)
        SFXManager::get()->queue(SFXManager::SFX_MUSIC_RESUME, m_current_music);
}   // resumeMusic

//-----------------------------------------------------------------------------
/** Switches to fast (last lap ) music (if defined for the current music).
 */
void MusicManager::switchToFastMusic()
{
    if (m_current_music.load() != NULL)
        SFXManager::get()->queue(SFXManager::SFX_MUSIC_SWITCH_FAST,
                                m_current_music);
}   // switchToFastMusic

//-----------------------------------------------------------------------------
/** Queues a command to temporarily change the volume. This is used to make
 *  the music a bit quieter while the 'last lap fanfare' is being played.
 *  \param gain The temporary volume value.
 */
void MusicManager::setTemporaryVolume(float gain)
{
    if (m_current_music.load() != NULL)
        SFXManager::get()->queue(SFXManager::SFX_MUSIC_SET_TMP_VOLUME, 
                                 m_current_music, gain);
}   // setTemporaryVolume

//-----------------------------------------------------------------------------
/** Queues a command for the sfx manager to reset a temporary volume change.
 */
void MusicManager::resetTemporaryVolume()
{
    if (m_current_music.load() != NULL)
        SFXManager::get()->queue(SFXManager::SFX_MUSIC_DEFAULT_VOLUME,
                                 m_current_music);
}   // resetTemporaryVolume

//-----------------------------------------------------------------------------
/** Sets the master music volume.
 *  \param gain The volume.
 */
void MusicManager::setMasterMusicVolume(float gain)
{
    if(gain > 1.0)
        gain = 1.0f;
    if(gain < 0.0f)
        gain = 0.0f;

    m_master_gain = gain;
    if (m_current_music.load() != NULL)
    {
        // Sets the music volume to m_master_gain
        SFXManager::get()->queue(SFXManager::SFX_MUSIC_DEFAULT_VOLUME,
                                 m_current_music);
    }

    UserConfigParams::m_music_volume = m_master_gain;
}   // setMasterMusicVolume

//-----------------------------------------------------------------------------
/** @throw runtime_error if the music file could not be found/opened
*/
MusicInformation* MusicManager::getMusicInformation(const std::string& filename)
{
    if(filename=="")
    {
        return NULL;
    }
    const std::string basename = StringUtils::getBasename(filename);
    std::map<std::string, MusicInformation*>::iterator p;
    p = m_all_music.find(basename);
    if(p==m_all_music.end())
    {
        // Note that this might raise an exception
        MusicInformation *mi = MusicInformation::create(filename);
        if(mi)
        {
            SFXManager::get()->queue(SFXManager::SFX_MUSIC_DEFAULT_VOLUME, mi);
            m_all_music[basename] = mi;
        }
        return mi;
    }
    return p->second;
}   // getMusicInformation

//----------------------------------------------------------------------------
