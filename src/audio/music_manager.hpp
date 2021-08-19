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

#ifndef HEADER_MUSICMANAGER_HPP
#define HEADER_MUSICMANAGER_HPP

#include "audio/music.hpp"
#include "audio/music_information.hpp"
#include "utils/no_copy.hpp"

#include <atomic>
#include <map>
#include <string>
#include <vector>

class Vec3;

/**
  * \brief Central place to request for musics to be loaded, played, stopped, etc...
  * \ingroup audio
  */
class MusicManager : public NoCopy
{
friend class MusicInformation;
private:
    std::atomic<MusicInformation*> m_current_music;

    /** If the sound could not be initialized, e.g. if the player doesn't has
     *  a sound card, we want to avoid anything sound related so we crash the
     *  game. */
    bool              m_initialized;

    /** Stores all music information files (read from the .music files). */
    std::map<std::string, MusicInformation*>
                      m_all_music;
    float             m_master_gain;

    void              loadMusicInformation();
    void              loadMusicFromOneDir(const std::string& dir);

public:
                      MusicManager();
    virtual          ~MusicManager();
    MusicInformation* getMusicInformation(const std::string& filename);
    void              addMusicToTracks();

    void              startMusic();
    void              startMusic(MusicInformation* mi,
                                 bool start_right_now=true);
    void              stopMusic();
    void              pauseMusic();
    void              resumeMusic();
    void              switchToFastMusic();
    void              setMasterMusicVolume(float gain);
    void              resetTemporaryVolume();
    void              setTemporaryVolume(float gain);
    // ------------------------------------------------------------------------
    /** Returns the master volume. */
    float getMasterMusicVolume() const { return m_master_gain; }
    // ------------------------------------------------------------------------
    /** Returns if the music system is initialised. */
    bool initialized() const { return m_initialized; }
    // ------------------------------------------------------------------------
    /** Returns the information object of the current music. */
    MusicInformation* getCurrentMusic() { return m_current_music; }
    // ------------------------------------------------------------------------
    /** Stops and removes the current music. */
    void clearCurrentMusic()
    {
        stopMusic();
        m_current_music = NULL;
    }
};

extern MusicManager* music_manager;

#endif // HEADER_SOUNDMANAGER_HPP

