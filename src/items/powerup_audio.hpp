//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2024 Alayan
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

#ifndef HEADER_POWERUPAUDIO_HPP
#define HEADER_POWERUPAUDIO_HPP

#include "audio/sfx_base.hpp"

class PowerupAudio : public NoCopy
{
private:
    /** Store the nitro hack sounds separately:
     * - This avoids them being interrupted too often.
     * - This limits the number of sound sources in use
     * - This simplifies creation and deletion management */
    SFXBase *m_sudo_good;
    SFXBase *m_sudo_bad;

    PowerupAudio();
public:
    static PowerupAudio* getInstance();

	 ~PowerupAudio();

    void playSudoGoodSFX() { m_sudo_good->play(); }
    void playSudoBadSFX()  { m_sudo_bad->play();  }
};

#endif