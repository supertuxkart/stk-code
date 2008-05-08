//  $Id: music_information.cpp 1610 2008-03-01 03:18:53Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
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


#include <stdexcept>

#include "music_information.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "string_utils.hpp"
#include "track_manager.hpp"
#include "track.hpp"
#include "translation.hpp"
#include "user_config.hpp"
#include "music_ogg.hpp"

#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

MusicInformation::MusicInformation(const std::string& filename)
{
    m_title           = "";
    m_composer        = "";
    m_numLoops        = LOOP_FOREVER;
    m_normal_filename = "";
    m_fast_filename   = "";
    m_normal_music    = NULL;
    m_fast_music      = NULL;
    m_faster_time     = 1.0f;
    m_max_pitch       = 0.1f;

    if(StringUtils::extension(filename)!="music")
    {
        // Create information just from ogg file
        // -------------------------------------
        m_title           = StringUtils::without_extension(StringUtils::basename(filename));
        m_normal_filename = filename;
        return;
    }
   

    // Otherwise read config file
    // --------------------------
    lisp::Parser parser;
    const lisp::Lisp* const ROOT = parser.parse(filename);

    const lisp::Lisp* const LISP = ROOT->getLisp("music-information");
    if(!LISP)
    {
        delete ROOT;
        char msg[MAX_ERROR_MESSAGE_LENGTH];
        snprintf(msg, sizeof(msg), 
                 "Couldn't load music information '%s': no music-information node.",
                 filename.c_str());
        throw std::runtime_error(msg);
    }
    LISP->get      ("title",       m_title          );
    LISP->get      ("composer",    m_composer       );
    LISP->get      ("loop",        m_numLoops       );
    LISP->get      ("music",       m_normal_filename);
    LISP->get      ("fast-music",  m_fast_filename  );
    // m_faster_time is used for twice: either as time to fade in faster music
    // (if available), or the time to increase the pitch (if no faster music 
    // is available). We allow each .music file to use any of the two names. 
    // LISP->get doesn't change the value if the item is not found.
    LISP->get      ("fade-time",   m_faster_time    );
    LISP->get      ("faster-time", m_faster_time    );
    LISP->get      ("max-pitch",   m_max_pitch      );
    LISP->getVector("tracks",      m_all_tracks     );

    // Get the path from the filename and add it to the ogg filename
    std::string path=StringUtils::path(filename);
    m_normal_filename=path+"/"+m_normal_filename;

    // Get the path from the filename and add it to the ogg filename
    if(m_fast_filename!="")
    {
        m_fast_filename=path+"/"+m_fast_filename;
    }

    delete ROOT;

}   // MusicInformation

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
void MusicInformation::startMusic()
{
    m_time_since_faster  = 0.0f;
    m_mode               = SOUND_NORMAL;

    if (m_normal_filename== "") return;

    // First load the 'normal' music
    // -----------------------------
    if(StringUtils::extension(m_normal_filename)!="ogg")
    {
        fprintf(stderr, "WARNING: music file %s format not recognized.\n", 
                m_normal_filename.c_str());
        return;
    }
    m_normal_music = new MusicOggStream();

    if((m_normal_music->load(m_normal_filename)) == false)
    {
        delete m_normal_music;
        m_normal_music=0;
	    fprintf(stderr, "WARNING: Unabled to load music %s, not supported or not found.\n", 
                m_normal_filename.c_str());
        return;
    }
    m_normal_music->playMusic();

    // Then (if available) load the music for the last track
    // -----------------------------------------------------
    if(m_fast_filename=="") 
    {
        m_fast_music = NULL;
        return;   // no fast music
    }

    if(StringUtils::extension(m_fast_filename)!="ogg")
    {
        fprintf(stderr, 
                "WARNING: music file %s format not recognized, fast music is ignored\n", 
                m_fast_filename.c_str());
        return;
    }
    m_fast_music= new MusicOggStream();

    if((m_fast_music->load(m_fast_filename)) == false)
    {
        delete m_fast_music;
        m_fast_music=0;
	    fprintf(stderr, "WARNING: Unabled to load fast music %s, not supported or not found.\n", 
                m_fast_filename.c_str());
        return;
    }
}   // startMusic

//-----------------------------------------------------------------------------
void MusicInformation::update(float dt)
{
    switch(m_mode)
    {
    case SOUND_FADING: {
        m_time_since_faster +=dt;
        if(m_time_since_faster>=m_faster_time)
        {
            m_mode=SOUND_FAST;
            m_normal_music->stopMusic();
            m_fast_music->update();
            return;
        }
        float fraction=m_time_since_faster/m_faster_time;
        m_normal_music->updateFading(1-fraction);
        m_fast_music->updateFading(fraction);
        break;
                       }
    case SOUND_FASTER: {
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
        m_normal_music->update();
        break;
    case SOUND_FAST:
        m_fast_music->update();
        break;
    }   // switch

}   // update

//-----------------------------------------------------------------------------
void MusicInformation::stopMusic()
{
    if (m_normal_music != NULL)  
    {
        m_normal_music->stopMusic();
        m_normal_music = NULL;
    }
    if (m_fast_music   != NULL)
    {
        m_fast_music->stopMusic();
        m_fast_music=NULL;
    }
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
    if (m_normal_music != NULL) m_normal_music->resumeMusic();
    if (m_fast_music   != NULL) m_fast_music->resumeMusic();
}   // resumeMusic

//-----------------------------------------------------------------------------
void MusicInformation::switchToFastMusic()
{    
    m_time_since_faster = 0.0f;
    if(m_fast_music)
    {
        m_mode = SOUND_FADING;
        m_fast_music->playMusic();
    }
    else
    {
        // FIXME: for now this music is too annoying, 
        m_mode = SOUND_FASTER;
    }
}   // switchToFastMusic

//-----------------------------------------------------------------------------
