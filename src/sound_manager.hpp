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

#ifndef HEADER_SOUNDMANAGER_H
#define HEADER_SOUNDMANAGER_H

#include <map>
#include <vector>
#include <string>

#include "music.hpp"
#include "music_information.hpp"
#include "sfx.hpp"

enum enumSFX {SOUND_UGH,  SOUND_WINNER, SOUND_CRASH, SOUND_GRAB,
              SOUND_SHOT, SOUND_WEE,   SOUND_EXPLOSION,
              SOUND_BZZT, SOUND_BEEP,
              SOUND_BACK_MENU, SOUND_USE_ANVIL, SOUND_USE_PARACHUTE,
              SOUND_SELECT_MENU, SOUND_MOVE_MENU, SOUND_FULL,
              SOUND_PRESTART, SOUND_START, SOUND_MISSILE_LOCK,
              NUM_SOUNDS};

class SoundManager
{
private:

    typedef std::vector<SFX*> SFXsType;

    SFXsType                m_sfxs;
    Music                  *m_normal_music,
                           *m_fast_music;
    MusicInformation const *m_music_information;
       
    bool                    m_initialized; //If the sound could not be initialized, e.g.
                                           //if the player doesn't has a sound card, we want
                                           //to avoid anything sound related so we crash the game.
    std::map<std::string,   const MusicInformation*> 
                            m_allMusic;
    void                    loadMusicInformation();
    enum {SOUND_NORMAL, SOUND_FADING, 
         SOUND_FAST}        m_mode; 
    float                   m_time_since_fade;

public:
    SoundManager();
    virtual ~SoundManager();

    void                    update(float dt);
    void                    playSfx(unsigned int id);
    void                    playMusic(const MusicInformation* mi);
    void                    stopMusic();
    void                    pauseMusic();
    void                    resumeMusic();
    void                    switchToFastMusic();
    const MusicInformation* getCurrentMusic() {return m_music_information; }    
    const MusicInformation* getMusicInformation(const std::string& filename);
    void                    loadMusicFromOneDir(const std::string& dir);
    void                    addMusicToTracks() const;

};

extern SoundManager* sound_manager;

#endif // HEADER_SOUNDMANAGER_H

