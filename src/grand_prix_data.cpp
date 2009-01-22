//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Ingo Ruhnke <grumbel@gmx.de>
//  Copyright (C) 2006      Joerg Henrichs, Ingo Ruhnke <grumbel@gmx.de>
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

#include <iostream>
#include <stdexcept>
#include "file_manager.hpp"
#include "grand_prix_data.hpp"
#include "lisp/parser.hpp"
#include "lisp/lisp.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"

GrandPrixData::GrandPrixData(const std::string filename)
{
    m_filename = filename;
    m_id       = StringUtils::basename(StringUtils::without_extension(filename));
    const lisp::Lisp* lisp = 0;
    try
    {
        lisp::Parser parser;
        lisp = parser.parse(file_manager->getConfigFile(m_filename));

        lisp = lisp->getLisp("supertuxkart-grand-prix");
        if(!lisp)
        {
            throw std::runtime_error("No supertuxkart-grand-prix node");
        }

        lisp->get      ("name",        m_name        );
        lisp->get      ("description", m_description );
        lisp->get      ("item",        m_item_style);
        lisp->getVector("tracks",      m_tracks      );
        lisp->getVector("laps",        m_laps        );
    }
    catch(std::exception& err)
    {
        fprintf(stderr, "Error while reading grandprix file '%s'\n", filename.c_str());
        fprintf(stderr, err.what());
        fprintf(stderr, "\n");
    }

    delete lisp;
}
// ----------------------------------------------------------------------------
bool GrandPrixData::checkConsistency()
{
    for(unsigned int i=0; i<m_tracks.size(); i++)
    {
        try
        {
            track_manager->getTrack(m_tracks[i]);
        }
        catch(std::exception& e)
        {
            (void)e;
            fprintf(stderr, "Grand Prix '%s': Track '%s' does not exist!\n",
                m_name.c_str(), m_tracks[i].c_str());
            fprintf(stderr, "This Grand Prix will not be available.\n");
            return false;
        }
        
    }   // for i
    return true;
}
/* EOF */
