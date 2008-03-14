//  $Id: music_information.hpp 1610 2008-03-01 03:18:53Z hikerstk $
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

#ifndef HEADER_MUSIC_INFORMATION
#define HEADER_MUSIC_INFORMATION

#include <string>
#include <vector>

class MusicInformation
{
private:
    std::string              m_composer;
    std::string              m_title;
    std::string              m_ogg_filename;
    std::vector<std::string> m_all_tracks;      
    int                      m_numLoops;
    static const int         LOOP_FOREVER=-1;
public:
                       MusicInformation(const std::string& filename);
    const std::string& getComposer     () const {return m_composer;     }
    const std::string& getTitle        () const {return m_title;        }
    const std::string& getFilename     () const {return m_ogg_filename; }
    int                getNumLoops     () const {return m_numLoops;     }
    void               addMusicToTracks() const;
};   // MusicInformation
#endif