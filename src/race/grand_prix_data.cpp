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

#include "race/grand_prix_data.hpp"

#include <iostream>
#include <stdexcept>

#include "io/file_manager.hpp"
#include "tracks/track_manager.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

GrandPrixData::GrandPrixData(const std::string filename) throw(std::logic_error)
{
    m_filename = filename;
    m_id       = StringUtils::getBasename(StringUtils::removeExtension(filename));
    
    XMLNode* root = file_manager->createXMLTree(file_manager->getDataDir()+filename);
    if (!root)
    {
        fprintf(stderr, "/!\\ Error while trying to read grandprix file '%s'\n", filename.c_str());
        throw std::logic_error("File not found");
    }
    
    bool foundName = false;
    
    if (root->getName() == "supertuxkart_grand_prix")
    {
        std::string temp_name;
        if (root->get("name", &temp_name) == 0)
        {
            fprintf(stderr, "/!\\ Error while trying to read grandprix file '%s' : "
                    "missing 'name' attribute\n", filename.c_str());
            delete root;
            throw std::logic_error("File contents are incomplete or corrupt");
        }
        m_name = temp_name.c_str();
        foundName = true;                
    }
    else
    {
        fprintf(stderr, "/!\\ Error while trying to read grandprix file '%s' : "
                "Root node has an unexpected name\n", filename.c_str());
        delete root;
        throw std::logic_error("File contents are incomplete or corrupt");
    }

    
    const int amount = root->getNumNodes();
    for (int i=0; i<amount; i++)
    {
        const XMLNode* node = root->getNode(i);
        
        // read a track entry
        if (node->getName() == "track")
        {
            std::string trackID;
            int numLaps;
            bool reversed = false;
            
            const int idFound      = node->get("id",      &trackID  );
            const int lapFound     = node->get("laps",    &numLaps  );
            // Will stay false if not found
            node->get("reverse", &reversed );

            if (!idFound || !lapFound)
            {
                fprintf(stderr, "/!\\ Error while trying to read grandprix file '%s' : "
                                "<track> tag does not have idi and laps reverse attributes. \n",
                                filename.c_str());
                delete root;
                throw std::logic_error("File contents are incomplete or corrupt");
            }
            
            // Make sure the track really is reversible
            Track* t = track_manager->getTrack(trackID);
            if (t != NULL && reversed)
            {
                reversed = t->reverseAvailable();
            }

            m_tracks.push_back(trackID);
            m_laps.push_back(numLaps);
            m_reversed.push_back(reversed);
            
            assert(m_tracks.size() == m_laps.size()    );
            assert(m_laps.size()   == m_reversed.size());
        }
        else
        {
            std::cerr << "Unknown node in Grand Prix XML file : " << node->getName().c_str() << std::endl;
            delete root;
            throw std::runtime_error("Unknown node in sfx XML file");
        }
    }// nend for
    
    delete root;

    // sanity checks
    if  (!foundName)
    {
        fprintf(stderr, "/!\\ Error while trying to read grandprix file '%s' : "
                "missing 'name' attribute\n", filename.c_str());
        throw std::logic_error("File contents are incomplete or corrupt");
    }

}
// ----------------------------------------------------------------------------
bool GrandPrixData::checkConsistency(bool chatty) const
{
    for(unsigned int i=0; i<m_tracks.size(); i++)
    {
        Track* t = track_manager->getTrack(m_tracks[i]);

        if (t == NULL)
        {
            if (chatty)
            {
                fprintf(stderr, "Grand Prix '%ls': Track '%s' does not exist!\n",
                                m_name.c_str(), m_tracks[i].c_str());
                fprintf(stderr, "This Grand Prix will not be available.\n");
            }
            return false;
        }
        
    }   // for i
    return true;
}
/* EOF */
