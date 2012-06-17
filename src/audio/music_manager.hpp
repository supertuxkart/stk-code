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

#ifndef HEADER_MUSICMANAGER_HPP
#define HEADER_MUSICMANAGER_HPP

#include <map>
#include <vector>
#include <string>

#include "audio/music.hpp"
#include "audio/music_information.hpp"
#include "utils/no_copy.hpp"

class Vec3;

/**
  * \brief Central place to request for musics to be loaded, played, stopped, etc...
  * \ingroup audio
  */
class MusicManager : public NoCopy
{
private:        
    MusicInformation        *m_current_music;

    /** If the sound could not be initialized, e.g. if the player doesn't has
     *  a sound card, we want to avoid anything sound related so we crash the 
     *  game. */
    bool                     m_initialized; 
    std::map<std::string, MusicInformation*> 
                             m_all_music;

    void                     loadMusicInformation();
    float                    m_masterGain;

public:
    MusicManager();
    virtual ~MusicManager();

    void                    startMusic(MusicInformation* mi, bool startRightNow=true);
    void                    stopMusic();
    bool                    initialized() const {return m_initialized;                 }
    void                    update(float dt)    {if(m_current_music)
                                                     m_current_music->update(dt);      }
    void                    pauseMusic()        {if(m_current_music)
                                                     m_current_music->pauseMusic();    }
    void                    resumeMusic()       {if(m_current_music)
                                                     m_current_music->resumeMusic();   }
    void                    switchToFastMusic() {if(m_current_music)
                                                    m_current_music->switchToFastMusic();}
    
    void                    setMasterMusicVolume(float gain);
    float                   getMasterMusicVolume() const { return m_masterGain; }
    
    MusicInformation       *getCurrentMusic() {return m_current_music; }   
    
    /**
      * @throw runtime_error if the music file could not be found/opened
      */ 
    MusicInformation       *getMusicInformation(const std::string& filename);

    void                    loadMusicFromOneDir(const std::string& dir);
    void                    addMusicToTracks();

    void                    clearCurrentMusic() { m_current_music = NULL; }
};

extern MusicManager* music_manager;

#endif // HEADER_SOUNDMANAGER_HPP

