//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2015 Joerg Henrichs
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
#include "io/file_manager.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#include <algorithm>
#include <set>


GrandPrixManager *grand_prix_manager = NULL;

const char* GrandPrixManager::SUFFIX = ".grandprix";

// ----------------------------------------------------------------------------
GrandPrixManager::GrandPrixManager()
{
    loadFiles();
}   // GrandPrixManager

// ----------------------------------------------------------------------------
GrandPrixManager::~GrandPrixManager()
{
}   // ~GrandPrixManager

// ----------------------------------------------------------------------------
void GrandPrixManager::loadFiles()
{
    // Add the directories to a set to avoid duplicates
    std::set<std::string> dirs;
    std::string dir;

    //Standard GPs
    loadDir(file_manager->getAsset(FileManager::GRANDPRIX, ""), GrandPrixData::GP_STANDARD);

    //User defined GPs
    dir = file_manager->getGPDir();
    if (!dir.empty() && dir[dir.size() - 1] == '/' && dirs.count(dir) == 0)
        loadDir(dir, GrandPrixData::GP_USER_DEFINED);

    //Add-on GPs
    dir = UserConfigParams::m_additional_gp_directory;
    if (!dir.empty() && dir[dir.size() - 1] == '/' && dirs.count(dir) == 0)
        loadDir(dir, GrandPrixData::GP_ADDONS);
}   // loadFiles

// ----------------------------------------------------------------------------
void GrandPrixManager::loadDir(const std::string& dir, enum GrandPrixData::GPGroupType group)
{
    Log::info("GrandPrixManager",
              "Loading Grand Prix files from %s", dir.c_str());
    assert(!dir.empty() && dir[dir.size() - 1] == '/');

    // Find out which grand prix are available and load them
    std::set<std::string> result;
    file_manager->listFiles(result, dir);
    for(std::set<std::string>::iterator i = result.begin(); i != result.end(); i++)
    {
        if (StringUtils::hasSuffix(*i, SUFFIX))
            load(dir + *i, group);
    }
}   // loadDir

// ----------------------------------------------------------------------------
void GrandPrixManager::load(const std::string& filename, enum GrandPrixData::GPGroupType group)
{
    GrandPrixData* gp = NULL;
    try
    {
        gp = new GrandPrixData(filename, group);
        m_gp_data.push_back(gp);
        Log::debug("GrandPrixManager",
                   "Grand Prix '%s' loaded from %s",
                   gp->getId().c_str(), filename.c_str());
    }
    catch (std::runtime_error& e)
    {
        if (gp != NULL)
            delete gp;
        Log::error("GrandPrixManager",
                   "Ignoring Grand Prix %s (%s)\n", filename.c_str(), e.what());
    }
}   // load

// ----------------------------------------------------------------------------
void GrandPrixManager::reload()
{
    m_gp_data.clearAndDeleteAll();

    loadFiles();
}   // reload

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
            if (m_gp_data[i].getId() == s.str())
            {
                unique = false;
                break;
            }
        }
    }

    return s.str();
}   // generateId

// ----------------------------------------------------------------------------
bool GrandPrixManager::existsName(const irr::core::stringw& name) const
{
    for (unsigned int i = 0; i < m_gp_data.size(); i++)
        if (m_gp_data[i].getName() == name)
            return true;

    return false;
}   // existsName

// ----------------------------------------------------------------------------
const GrandPrixData* GrandPrixManager::getGrandPrix(const std::string& s) const
{
    for (unsigned int i = 0; i<m_gp_data.size(); i++)
    {
        if (m_gp_data[i].getId() == s)
            return m_gp_data.get(i);
    }   // for i in m_gp_data

    return NULL;
}   // getGrandPrix

// ----------------------------------------------------------------------------
GrandPrixData* GrandPrixManager::editGrandPrix(const std::string& s)
{
    for(unsigned int i=0; i<m_gp_data.size(); i++)
    {
        if (m_gp_data[i].getId() == s)
            return m_gp_data.get(i);
    }   // for i in m_gp_data

    return NULL;
}   // editGrandPrix

// ----------------------------------------------------------------------------
void GrandPrixManager::checkConsistency()
{
    for (int i = (int)m_gp_data.size() - 1; i >= 0; i--)
    {
        if (!m_gp_data[i].checkConsistency())
        {
            // delete this GP, since a track is missing
            m_gp_data.erase(i);
        }
    }
}   // checkConsistency

// ----------------------------------------------------------------------------
GrandPrixData* GrandPrixManager::createNewGP(const irr::core::stringw& newName)
{
    if (existsName(newName))
        return NULL;

    std::string newID = generateId();

    GrandPrixData* gp = new GrandPrixData;
    gp->setId(newID);
    gp->setName(newName);
    gp->setFilename(file_manager->getGPDir() + newID + SUFFIX);
    gp->setEditable(true);
    gp->setGroup(GrandPrixData::GP_USER_DEFINED);
    gp->writeToFile();
    m_gp_data.push_back(gp);

    return gp;
}   // createNewGP

// ----------------------------------------------------------------------------
GrandPrixData* GrandPrixManager::copy(const std::string& id,
                                      const irr::core::stringw& newName)
{
    if (existsName(newName))
        return NULL;

    std::string newID = generateId();

    GrandPrixData* gp = new GrandPrixData(*getGrandPrix(id));
    gp->setId(newID);
    gp->setName(newName);
    gp->setFilename(file_manager->getGPDir() + newID + SUFFIX);
    gp->setEditable(true);
    gp->writeToFile();
    m_gp_data.push_back(gp);

    return gp;
}   // copy

// ----------------------------------------------------------------------------
void GrandPrixManager::remove(const std::string& id)
{
    const GrandPrixData* gp = getGrandPrix(id);
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
}   // remove
