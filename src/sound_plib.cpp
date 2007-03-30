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

//The next #if *needs* double parenthesis
#if !((HAVE_OPENAL && (HAVE_MIKMOD || HAVE_OGGVORBIS)))

#include <string>

#include "sound_plib.hpp"
#include "sound_manager.hpp"
#include "loader.hpp"

slScheduler* plib_scheduler= NULL;

SFXImpl::SFXImpl(const char* filename)
{
    std::string path = loader->getPath(filename);
    m_sample= new slSample(path.c_str(), plib_scheduler);
}

//-----------------------------------------------------------------------------
SFXImpl::~SFXImpl()
{
    delete m_sample;
}

//-----------------------------------------------------------------------------
void SFXImpl::play()
{
    plib_scheduler->playSample(m_sample, 1, SL_SAMPLE_MUTE, 2, NULL);
}

//=============================================================================
void MusicPlib::update()
{
    // Comment this next line out if the sound causes big glitches
    // on your IRIX machine!
    plib_scheduler->update();
}

//-----------------------------------------------------------------------------
bool MusicPlib::load(const char* filename)
{
    m_filename= loader->getPath(filename);
    return true;
}

//-----------------------------------------------------------------------------
bool MusicPlib::playMusic()
{
    plib_scheduler->stopMusic();
    plib_scheduler->loopMusic(m_filename.c_str());
    return true;
}

//-----------------------------------------------------------------------------
bool MusicPlib::stopMusic()
{
    plib_scheduler->stopMusic();
    return true;
}

//-----------------------------------------------------------------------------
bool MusicPlib::pauseMusic()
{
    plib_scheduler->stopMusic();
    return true;
}

//-----------------------------------------------------------------------------
bool MusicPlib::resumeMusic()
{
    plib_scheduler->loopMusic(m_filename.c_str());
    return true;
}

#endif //!(HAVE_OPENAL && (HAVE_MIKMOD || HAVE_OGGVORBIS))

