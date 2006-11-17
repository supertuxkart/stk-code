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

#ifndef HEADER_MUSICMIKMOD_H
#define HEADER_MUSICMIKMOD_H

#include <string>

#include <mikmod.h>
#include <AL/al.h>

#include "music.hpp"


class MusicMikMod : public Music
{
public:
    MusicMikMod();
    virtual ~MusicMikMod();

    virtual void update();

    virtual bool load(const char* filename);

    virtual bool playMusic();
    virtual bool stopMusic();
    virtual bool pauseMusic();
    virtual bool resumeMusic();

private:
    bool release();
    bool isPlaying();
    bool streamIntoBuffer(ALuint buffer);

    std::string m_fileName;
    MODULE* m_modStream;

    ALuint m_soundBuffers[2];
    ALuint m_soundSource;

    bool m_pausedMusic;
};

#endif // HEADER_MUSICMIKMOD_H

