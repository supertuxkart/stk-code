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


#include "music_information.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "string_utils.hpp"
#include "track_manager.hpp"
#include "track.hpp"
#include "translation.hpp"

#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

MusicInformation::MusicInformation(const std::string& filename)
{
    m_title    = "";
    m_composer = "";
    m_numLoops = LOOP_FOREVER;

    if(StringUtils::extension(filename)!="music")
    {
        // Create information just from ogg file
        // -------------------------------------
        m_title        = StringUtils::without_extension(StringUtils::basename(filename));
        m_ogg_filename = filename;
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
    LISP->get      ("title",    m_title       );
    LISP->get      ("composer", m_composer    );
    LISP->get      ("loop",     m_numLoops    );
    LISP->get      ("music",    m_ogg_filename);
    LISP->getVector("tracks",   m_all_tracks);

    // Get the path from the filename and add it to the ogg filename
    std::string path=StringUtils::path(filename);
    m_ogg_filename=path+"/"+m_ogg_filename;

    delete ROOT;

}   // MusicInformation

//-----------------------------------------------------------------------------
void MusicInformation::addMusicToTracks() const
{
    for(int i=0; i<(int)m_all_tracks.size(); i++)
    {
        Track* track=track_manager->getTrack(m_all_tracks[i]);
        if(track) track->addMusic(this);
    }
}   // addMusicToTracks

//-----------------------------------------------------------------------------
