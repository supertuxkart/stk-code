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

#include "config/user_config.hpp"
<<<<<<< HEAD
#include "race/grand_prix.hpp"
=======
>>>>>>> b0169d28961d0e1d5c19cc2325cf09d634ba255b
#include "io/file_manager.hpp"
#include "race/grand_prix_data.hpp"
#include "utils/string_utils.hpp"

#include <algorithm>
#include <set>
#include <sstream>

GrandPrixManager *grand_prix_manager = NULL;

const char* GrandPrixManager::SUFFIX = ".grandprix";

// ----------------------------------------------------------------------------
void GrandPrixManager::loadFiles()
{
    std::set<std::string> dirs;

    // Add all the directories to a set to avoid duplicates
    dirs.insert(file_manager->getAsset(FileManager::GRANDPRIX, ""));
    dirs.insert(file_manager->getGPDir());
    dirs.insert(UserConfigParams::m_additional_gp_directory);

    for (std::set<std::string>::const_iterator it  = dirs.begin();
                                               it != dirs.end  (); ++it)
    {
        std::string dir = *it;
        if (!dir.empty() && dir[dir.size() - 1] == '/')
            loadDir(dir);
    }
}

// ----------------------------------------------------------------------------
void GrandPrixManager::loadDir(const std::string& dir)
{
    Log::info("GrandPrixManager",
              "Loading Grand Prix files from %s", dir.c_str());
    assert(!dir.empty() && dir[dir.size() - 1] == '/');

    // Findout which grand prix are available and load them
    std::set<std::string> result;
    file_manager->listFiles(result, dir);
    for(std::set<std::string>::iterator i  = result.begin();
                                        i != result.end(); i++)
        if (StringUtils::hasSuffix(*i, SUFFIX))
            load(dir + *i);
}

// ----------------------------------------------------------------------------
void GrandPrixManager::load(const std::string& filename)
{
    try
    {
<<<<<<< HEAD
        GrandPrix* gp = new GrandPrix(filename);
=======
        GrandPrixData* gp = new GrandPrixData(filename);
>>>>>>> b0169d28961d0e1d5c19cc2325cf09d634ba255b
        m_gp_data.push_back(gp);
        Log::debug("GrandPrixManager",
                   "Grand Prix '%s' loaded from %s",
                   gp->getId().c_str(), filename.c_str());
    }
    catch (std::runtime_error& e)
    {
        Log::error("GrandPrixManager",
                   "Ignoring grand prix %s (%s)\n", filename.c_str(), e.what());
    }
}   // load

// ----------------------------------------------------------------------------
void GrandPrixManager::reload()
{
    for(unsigned int i=0; i<m_gp_data.size(); i++)
        delete m_gp_data[i];
    m_gp_data.clear();

    loadFiles();
}

// ----------------------------------------------------------------------------
std::string GrandPrixManager::generateId()
{
    std::stringstream s;

    bool unique = false;
    while(!unique)
    {
        s.clear();
        s << "usr_gp_" << ((rand() % 90000000) + 10000000);

        // Check if the id already exists
        unique = true;
        for (unsigned int i = 0; i < m_gp_data.size(); i++)
        {
            if (m_gp_data[i]->getId() == s.str())
            {
                unique = false;
                break;
            }
        }
    }

    return s.str();
}

// ----------------------------------------------------------------------------
bool GrandPrixManager::existsName(const irr::core::stringw& name) const
{
    for (unsigned int i = 0; i < m_gp_data.size(); i++)
        if (m_gp_data[i]->getName() == name)
            return true;

    return false;
}

// ----------------------------------------------------------------------------
GrandPrixManager::GrandPrixManager()
{
    loadFiles();
}

// ----------------------------------------------------------------------------
GrandPrixManager::~GrandPrixManager()
{
    for(unsigned int i=0; i<m_gp_data.size(); i++)
        delete m_gp_data[i];
<<<<<<< HEAD
}

// ----------------------------------------------------------------------------
GrandPrix* GrandPrixManager::getGrandPrix(const std::string& s) const
=======
    }
}

// ----------------------------------------------------------------------------
GrandPrixData* GrandPrixManager::getGrandPrix(const std::string& s) const
>>>>>>> b0169d28961d0e1d5c19cc2325cf09d634ba255b
{
    return editGrandPrix(s);
}

// ----------------------------------------------------------------------------
GrandPrix* GrandPrixManager::editGrandPrix(const std::string& s) const
{
    for(unsigned int i=0; i<m_gp_data.size(); i++)
    {
        if(m_gp_data[i]->getId() == s)
            return m_gp_data[i];
    }   // for i in m_gp_data

    return NULL;
}

// ----------------------------------------------------------------------------
void GrandPrixManager::checkConsistency()
{
    for(unsigned int i=0; i<m_gp_data.size(); i++)
    {
        if(!m_gp_data[i]->checkConsistency())
        {
            // delete this GP, since a track is missing
            delete *(m_gp_data.erase(m_gp_data.begin()+i));
            i--;
        }
    }
}   // checkConsistency

// ----------------------------------------------------------------------------
<<<<<<< HEAD
GrandPrix* GrandPrixManager::createNewGP(const irr::core::stringw& newName)
=======
GrandPrixData* GrandPrixManager::createNewGP(const irr::core::stringw& newName)
>>>>>>> b0169d28961d0e1d5c19cc2325cf09d634ba255b
{
    if (existsName(newName))
        return NULL;

    std::string newID = generateId();

    GrandPrix* gp = new GrandPrix;
    gp->m_id = newID;
    gp->m_name = newName;
    gp->m_filename = file_manager->getGPDir() + newID + SUFFIX;
    gp->m_editable = true;
    gp->writeToFile();
    m_gp_data.push_back(gp);

    return gp;
}

// ----------------------------------------------------------------------------
<<<<<<< HEAD
GrandPrix* GrandPrixManager::copy(const std::string& id,
=======
GrandPrixData* GrandPrixManager::copy(const std::string& id,
>>>>>>> b0169d28961d0e1d5c19cc2325cf09d634ba255b
                                      const irr::core::stringw& newName)
{
    if (existsName(newName))
        return NULL;

    std::string newID = generateId();

    GrandPrix* gp = new GrandPrix(*getGrandPrix(id));
    gp->m_id = newID;
    gp->m_name = newName;
    gp->m_filename = file_manager->getGPDir() + newID + SUFFIX;
    gp->m_editable = true;
    gp->writeToFile();
    m_gp_data.push_back(gp);

    return gp;
}

// ----------------------------------------------------------------------------
void GrandPrixManager::remove(const std::string& id)
{
    const GrandPrix* gp = getGrandPrix(id);
    assert(gp != NULL);

    if (gp->isEditable())
    {
        file_manager->removeFile(gp->getFilename());
        reload();
    }
    else
    {
        Log::warn("GrandPrixManager",
                  "Grand Prix '%s' can not be removed", gp->getId().c_str());
    }
}
