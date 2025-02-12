//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Patrick Ammann <pammann@aro.ch>
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

#ifndef HEADER_MUSIC_HPP
#define HEADER_MUSIC_HPP

#include <string>

#include "utils/no_copy.hpp"
/**
  * \brief Abstract interface for classes that can handle music playback
  * \ingroup audio
  */
class Music : public NoCopy
{
public:
    virtual bool load        (const std::string& filename) = 0;
    virtual bool playMusic   ()                            = 0;
    virtual bool stopMusic   ()                            = 0;
    virtual bool pauseMusic  ()                            = 0;
    virtual bool resumeMusic ()                            = 0;
    virtual void setVolume   (float volume)                = 0;
    virtual void updateFaster(float percent, float pitch)  = 0;
    virtual void update      ()                            = 0;
    virtual bool isPlaying   ()                            = 0;

    virtual     ~Music       () {};
};

#endif // HEADER_MUSIC_HPP

