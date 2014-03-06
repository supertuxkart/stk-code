//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2013 Joerg Henrichs
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

#include "race/grand_prix_manager.hpp"

#include <set>
#include "io/file_manager.hpp"
#include "utils/string_utils.hpp"
#include "config/user_config.hpp"

GrandPrixManager *grand_prix_manager = NULL;

GrandPrixManager::GrandPrixManager()
{
    // Findout which grand prixs are available and load them
    // Grand Prix in the standart directory
    std::set<std::string> result;
    std::string gp_dir = file_manager->getAsset(FileManager::GRANDPRIX, "");
    file_manager->listFiles(result, gp_dir);
    for(std::set<std::string>::iterator i  = result.begin();
                                        i != result.end()  ; i++)
    {
        if (StringUtils::hasSuffix(*i, ".grandprix"))
        {
            try
            {
                m_gp_data.push_back(new GrandPrixData(*i));
                Log::debug("GrandPrixManager", "Grand Prix %s loaded.",
                                               i->c_str());
            }
            catch (std::logic_error& e)
            {
                Log::error("GrandPrixManager", "Ignoring GP %s ( %s ) \n",
                                                          i->c_str(), e.what());
            }
        }
    }

    // Load additional Grand Prix
    const std::string dir = UserConfigParams::m_additional_gp_directory;
    if(dir != "") {
        Log::info("GrandPrixManager", "Loading additional Grand Prix from "
                                      "%s ...", dir.c_str());
        file_manager->listFiles(result, dir);
        for(std::set<std::string>::iterator i  = result.begin();
                                            i != result.end()  ; i++)
        {
            if (StringUtils::hasSuffix(*i, ".grandprix"))
            {
                try
                {
                    m_gp_data.push_back(new GrandPrixData(dir, *i));
                    Log::debug("GrandPrixManager", "Grand Prix %s loaded from "
                                                   "%s", i->c_str(),
                                                   dir.c_str());
                }
                catch (std::logic_error& e)
                {
                    Log::error("GrandPrixManager", "Ignoring GP %s ( %s ) \n",
                                                   i->c_str(), e.what());
                }
            }
        }   // end for
    }   // end if
}   // GrandPrixManager
// ----------------------------------------------------------------------------
GrandPrixManager::~GrandPrixManager()
{
    for(unsigned int i=0; i<m_gp_data.size(); i++)
    {
        delete m_gp_data[i];
    }   // for i

}   // ~GrandPrixManager
// ----------------------------------------------------------------------------
const GrandPrixData* GrandPrixManager::getGrandPrix(const std::string& s) const
{
    for(unsigned int i=0; i<m_gp_data.size(); i++)
        if(m_gp_data[i]->getId() == s)
            return m_gp_data[i];

    return NULL;
}   // getGrandPrix
// ----------------------------------------------------------------------------
void GrandPrixManager::checkConsistency()
{
    for(unsigned int i=0; i<m_gp_data.size(); i++)
    {
        if(!m_gp_data[i]->checkConsistency())
        {
            // delete this GP, since a track is missing
            m_gp_data.erase(m_gp_data.begin()+i);
            i--;
        }
    }
}   // checkConsistency
// ----------------------------------------------------------------------------
