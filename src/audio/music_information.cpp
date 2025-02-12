//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2015 Joerg Henrichs
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

#include "audio/music_information.hpp"

#include <stdexcept>
#include <iostream>

#include "audio/music_dummy.hpp"
#include "audio/music_manager.hpp"
#include "audio/music_ogg.hpp"
#include "config/user_config.hpp"
#include "io/file_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

/** A simple factory to create music information files without raising
 *  an exception on error, instead a NULL will be returned. This avoids
 *  resource freeing problems if the exception is raised, and simplifies
 *  calling code.
 *  \param filename The name of a music file.
 *  \return The MusicInformation object, or NULL in case of an error.
 */
MusicInformation *MusicInformation::create(const std::string &filename)
{
    assert(filename.size() > 0);

    auto root = std::unique_ptr<XMLNode>(file_manager->createXMLTree(filename));
    if (!root) return NULL;
    if(root->getName()!="music")
    {
        Log::error("MusicInformation",
                   "Music file '%s' does not contain music node.\n",
                   filename.c_str());
        return NULL;
    }
    std::string s;
    if(!root->get("title",    &s) ||
       !root->get("file",     &s)    )

    {
        Log::error("MusicInformation",
                    "One of 'title' or 'file' attribute "
                    "is missing in the music XML file '%s'!\n",
                    filename.c_str());
        return NULL;
    }
    MusicInformation *mi = new MusicInformation(root.get(), filename);
    return mi;
}   // create()

// ----------------------------------------------------------------------------
MusicInformation::MusicInformation(const XMLNode *root,
                                   const std::string &filename)
{
    m_title             = "";
    m_mode              = SOUND_NORMAL;
    m_composer          = "";
    //m_numLoops          = LOOP_FOREVER;
    m_normal_filename   = "";
    m_fast_filename     = "";
    m_normal_music      = NULL;
    m_fast_music        = NULL;
    m_normal_loop_start = 0.0f;
    m_fast_loop_start   = 0.0f;
    m_normal_loop_end   = -1.0f;
    m_fast_loop_end     = -1.0f;
    m_enable_fast       = false;
    m_music_waiting     = false;
    m_faster_time       = 1.0f;
    m_max_pitch         = 0.1f;
    m_gain              = 1.0f;


    // Otherwise read config file
    // --------------------------
    std::string title_raw;
    std::string composer_raw;
    root->get("title",           &title_raw          );
    m_title = StringUtils::xmlDecode(title_raw);
    root->get("composer",        &composer_raw       );
    m_composer = StringUtils::xmlDecode(composer_raw);
    root->get("file",            &m_normal_filename  );
    root->get("gain",            &m_gain             );
    root->get("tracks",          &m_all_tracks       );
    root->get("fast",            &m_enable_fast      );
    root->get("fast-filename",   &m_fast_filename    );
    root->get("loop-start",      &m_normal_loop_start);
    root->get("loop-end",        &m_normal_loop_end  );
    root->get("fast-loop-start", &m_fast_loop_start  );
    root->get("fast-loop-end",   &m_fast_loop_end    );

    // Get the path from the filename and add it to the ogg filename
    std::string path  = StringUtils::getPath(filename);
    m_normal_filename = path + "/" + m_normal_filename;

    // Get the path from the filename and add it to the ogg filename
    if (m_fast_filename != "")
    {
        m_fast_filename = path + "/" + m_fast_filename;
    }

    assert(m_normal_filename.size() > 0);

}   // MusicInformation

//-----------------------------------------------------------------------------

MusicInformation::~MusicInformation()
{
    std::lock_guard<std::mutex> lock(m_music_mutex);
    if(m_normal_music) delete m_normal_music;
    if(m_fast_music)   delete m_fast_music;
}   // ~MusicInformation

//-----------------------------------------------------------------------------

void MusicInformation::addMusicToTracks()
{
    for(int i=0; i<(int)m_all_tracks.size(); i++)
    {
        Track* track=track_manager->getTrack(m_all_tracks[i]);
        if(track) track->addMusic(this);
    }
}   // addMusicToTracks

//-----------------------------------------------------------------------------
/** Return false if it should not be started, done in sfx thread.
 */
bool MusicInformation::preStart()
{
    MusicInformation* old_mi = music_manager->m_current_music.load();
    // If this music is already playing, ignore this call.
    if (isPlaying() ||
        (old_mi != NULL && old_mi == this && old_mi->isPlaying()))
    {
        return false;
    }
    // It is possible here that startMusic() will be called without first
    // calling stopMusic(). This would cause a memory leak by overwriting
    // m_current_music without first releasing its resources. Guard against
    // this here by making sure that stopMusic() is called before starting
    // new music.
    if (old_mi)
        old_mi->stopMusic();
    music_manager->m_current_music.store(this);
    return true;
}   // preStart

//-----------------------------------------------------------------------------
/** Starts the music.
 */
void MusicInformation::startMusic()
{
    m_time_since_faster  = 0.0f;
    m_mode               = SOUND_NORMAL;

    std::unique_lock<std::mutex> lock(m_music_mutex);
    if (m_normal_music)
    {
        delete m_normal_music;
        m_normal_music = NULL;
    }
    
    if (m_fast_music)
    {
        delete m_fast_music;
        m_fast_music = NULL;
    }
    
    // First load the 'normal' music
    // -----------------------------
    if (m_normal_filename.empty())
        return;
    
    if (StringUtils::getExtension(m_normal_filename) != "ogg")
    {
        Log::warn("MusicInformation", "Music file %s is not found or file "
                  "format is not recognized.\n", m_normal_filename.c_str());
        return;
    }

#ifdef ENABLE_SOUND
    if (UserConfigParams::m_enable_sound)
    {
        m_normal_music = new MusicOggStream(m_normal_loop_start, m_normal_loop_end);
    }
    else
#endif
    {
        m_normal_music = new MusicDummy();
    }
    lock.unlock();

    if (m_normal_music->load(m_normal_filename) == false)
    {
        lock.lock();
        delete m_normal_music;
        m_normal_music = NULL;
        Log::warn("MusicInformation", "Unable to load music %s, "
                  "not supported or not found.",
                  m_normal_filename.c_str());
        return;
    }
    m_normal_music->setVolume(m_gain);
    m_normal_music->playMusic();

    // Then (if available) load the music for the last track
    // -----------------------------------------------------
    if (m_fast_filename.empty())
        return;
    
    if (StringUtils::getExtension(m_fast_filename) != "ogg")
    {
        Log::warn("MusicInformation",
                  "Music file %s format not recognized, fast music is ignored",
                  m_fast_filename.c_str());
        return;
    }

    lock.lock();
#ifdef ENABLE_SOUND
    if (UserConfigParams::m_enable_sound)
    {
        m_fast_music = new MusicOggStream(m_fast_loop_start, m_fast_loop_end);
    }
    else
#endif
    {
        m_fast_music = new MusicDummy();
    }
    lock.unlock();

    if (m_fast_music->load(m_fast_filename) == false)
    {
        lock.lock();
        delete m_fast_music;
        m_fast_music = NULL;
        Log::warn("MusicInformation", "Unabled to load fast music %s, not "
                  "supported or not found.\n", m_fast_filename.c_str());
        return;
    }
    m_fast_music->setVolume(m_gain);
}   // startMusic

//-----------------------------------------------------------------------------
void MusicInformation::update(float dt)
{
    switch(m_mode)
    {
    case SOUND_FADING: {
        if ( m_normal_music == NULL || m_fast_music == NULL ) break;

        m_time_since_faster +=dt;
        if(m_time_since_faster>=m_faster_time)
        {
            m_mode=SOUND_FAST;
            m_normal_music->stopMusic();
            m_fast_music->update();
            return;
        }
        float fraction=m_time_since_faster/m_faster_time;
        m_normal_music->setVolume(1-fraction);
        m_fast_music->setVolume(fraction);
        m_normal_music->update();
        m_fast_music->update();
        break;
                       }
    case SOUND_FASTER: {
        if ( m_normal_music == NULL ) break;

        m_time_since_faster +=dt;
        if(m_time_since_faster>=m_faster_time)
        {
            // Once the pitch is adjusted, just switch back to normal
            // mode. We can't switch to fast music mode, since this would
            // play m_fast_music, which isn't available.
            m_mode=SOUND_NORMAL;
            return;
        }
        float fraction=m_time_since_faster/m_faster_time;
        m_normal_music->updateFaster(fraction, m_max_pitch);

        break;
                       }
    case SOUND_NORMAL:
        if ( m_normal_music ) 
            m_normal_music->update();
        break;
    case SOUND_FAST:
        if ( m_fast_music ) 
            m_fast_music->update();
        break;
    }   // switch

}   // update

//-----------------------------------------------------------------------------
void MusicInformation::stopMusic()
{
    std::lock_guard<std::mutex> lock(m_music_mutex);
    if (m_normal_music != NULL)
    {
        m_normal_music->stopMusic();
        delete m_normal_music;
        m_normal_music = NULL;
    }
    if (m_fast_music   != NULL)
    {
        m_fast_music->stopMusic();
        delete m_fast_music;
        m_fast_music=NULL;
    }
    if(m_music_waiting)
        m_music_waiting = false;
}   // stopMusic

//-----------------------------------------------------------------------------
void MusicInformation::pauseMusic()
{
    if (m_normal_music != NULL) m_normal_music->pauseMusic();
    if (m_fast_music   != NULL) m_fast_music->pauseMusic();
}   // pauseMusic
//-----------------------------------------------------------------------------
void MusicInformation::resumeMusic()
{
    if(m_music_waiting)
    {
        startMusic();
        m_music_waiting = false;
        return;
    }
    if (m_normal_music != NULL) m_normal_music->resumeMusic();
    if (m_fast_music   != NULL) m_fast_music->resumeMusic();
}   // resumeMusic

//-----------------------------------------------------------------------------
void MusicInformation::setDefaultVolume()
{
    if (m_normal_music && m_normal_music->isPlaying())
        m_normal_music->setVolume(m_gain);
    if (m_fast_music && m_fast_music->isPlaying())
        m_fast_music->setVolume(m_gain);
} // setVolume

//-----------------------------------------------------------------------------
/** Overwrites the current volume with a temporary value (used e.g. to fade
 *  from normal music to last lap music).
 */
void MusicInformation::setTemporaryVolume(float volume)
{
    if (m_normal_music != NULL) m_normal_music->setVolume(volume);
    if (m_fast_music   != NULL) m_fast_music->setVolume(volume);
}

//-----------------------------------------------------------------------------
void MusicInformation::switchToFastMusic()
{
    if(!m_enable_fast) return;
    m_time_since_faster = 0.0f;
    if(m_fast_music)
    {
        m_mode = SOUND_FADING;
        m_fast_music->playMusic();
        m_fast_music->setVolume(0);
    }
    else
    {
        // FIXME: for now this music is too annoying,
        m_mode = SOUND_FASTER;
    }
}   // switchToFastMusic

//-----------------------------------------------------------------------------

bool MusicInformation::isPlaying() const
{
    std::lock_guard<std::mutex> lock(m_music_mutex);
    return (m_normal_music != NULL && m_normal_music->isPlaying())  ||
           (m_fast_music   != NULL && m_fast_music->isPlaying());
}

//-----------------------------------------------------------------------------
