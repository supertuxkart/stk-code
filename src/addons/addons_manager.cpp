//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Lucas Baudin
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
/**
  \page addons Addons
  */
#ifdef ADDONS_MANAGER

#include "addons/addons_manager.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string.h>
#include <vector>

#include "irrXML.h"

#include "addons/network_http.hpp"
#include "addons/zip.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "states_screens/kart_selection.hpp"
#include "tracks/track_manager.hpp"

using namespace irr; /* irrXML which is used to read (not write) xml file,
is located in the namespace irr::io.*/
using namespace io;
AddonsManager* addons_manager = 0;
// ----------------------------------------------------------------------------

AddonsManager::AddonsManager()
{
    m_index = -1;
    int download_state = 0;
    m_download_state = download_state;
    pthread_mutex_init(&m_str_mutex, NULL);

    // FIXME: It is _very_ dirty to save the list as a locale file since we have a
    //        function to load it directly in a string.
    const bool success = download("list");
    if (!success)
    {
        fprintf(stderr, "Downloading 'list' failed\n");
    }
    
    std::string xml_file = file_manager->getConfigDir() + "/" + "list";
    std::cout << "[Addons] Using file '" << xml_file << "'\n";
    IrrXMLReader* xml = createIrrXMLReader(xml_file.c_str());

    // strings for storing the data we want to get out of the file
    std::string attribute_name;

    // parse the file until end reached

    while(xml && xml->read())
    {
        /*only if it is a node*/
        if(xml->getNodeType() == EXN_ELEMENT)
        {
            if (!strcmp("kart", xml->getNodeName()) || !strcmp("track", xml->getNodeName()))
            {
                addons_prop addons;
                //the unsigned is to remove the compiler warnings, maybe it is a bad idea ?
                for(unsigned int i = 0; i < xml->getAttributeCount(); i++)
                {
                    attribute_name = xml->getAttributeName(i);
                    if(attribute_name == "name")
                    {
                        addons.name = xml->getAttributeValue("name");
                    }
                    if(attribute_name == "version")
                    {
                        addons.version = atoi(xml->getAttributeValue("version"));
                    }
                    if(attribute_name == "file")
                    {
                        addons.file = xml->getAttributeValue("file");
                    }
                    if(attribute_name == "description")
                    {
                        addons.description = xml->getAttributeValue("description");
                    }
                    if(attribute_name == "icon")
                    {
                        addons.icon = xml->getAttributeValue("icon");
                    }
                    if(attribute_name == "id")
                    {
                        addons.id = xml->getAttributeValue("id");
                    }
                }
                addons.type = xml->getNodeName();
                addons.installed = false;
                m_addons_list.push_back(addons);
            }
        }
    }
    delete xml;

    m_file_installed = file_manager->getConfigDir() 
                     + "/" + "addons_installed.xml";
    getInstalledAddons();
}
// ----------------------------------------------------------------------------
void AddonsManager::resetIndex()
{
    m_index = -1;
}
// ----------------------------------------------------------------------------
void AddonsManager::getInstalledAddons()
{
    std::string attribute_name;
    int old_index = m_index;

    /* checking for installed addons */

    std::cout << "[Addons] Loading an xml file for installed addons: ";
    std::cout << m_file_installed << std::endl;
    IrrXMLReader* xml = createIrrXMLReader(m_file_installed.c_str());

    // parse the file until end reached

    while(xml && xml->read())
    {
        std::string name;
        std::string id;
        int version = 0;
        switch(xml->getNodeType())
        {
        case EXN_ELEMENT:
            {
                if (!strcmp("kart", xml->getNodeName()) || !strcmp("track", xml->getNodeName()))
                {
                    std::cout << xml->getAttributeCount() << std::endl;
                    //the unsigned is to remove the compiler warnings, maybe it is a bad idea ?
                    for(unsigned int i = 0; i < xml->getAttributeCount(); i++)
                    {
                        attribute_name = xml->getAttributeName(i);
                        if(attribute_name == "id")
                        {
                            id = xml->getAttributeValue("id");
                        }
                        if(attribute_name == "name")
                        {
                            name = xml->getAttributeValue("name");
                        }
                        if(attribute_name == "version")
                        {
                            version = xml->getAttributeValueAsInt("version");
                        }
                    }
                    if(selectId(id))
                    {
                        m_addons_list[m_index].installed = true;
                        m_addons_list[m_index].installed_version = version;
                        std::cout << "[Addons] An addon is already installed: " << id << std::endl;
                    }
                    else
                    {
                        addons_prop addons;
                        addons.type = xml->getNodeName();
                        addons.name = name;
                        addons.installed_version = version;
                        addons.version = version;
                        addons.installed = true;
                        m_addons_list.push_back(addons);
                    }
                }
            }
            break;
        default : break;
        }
    }
    delete xml;
    m_index = old_index;
}


// ----------------------------------------------------------------------------
bool AddonsManager::next()
{
    if(m_index + 1 < (int)m_addons_list.size())
    {
        m_index ++;
        return true;
    }
    m_index = -1;
    return false;
}   // next

// ----------------------------------------------------------------------------
bool AddonsManager::nextType(std::string type)
{
    while(next())
    {
        if(m_addons_list[m_index].type == type)
            return true;
    }
    while(next())
    {
        if(m_addons_list[m_index].type == type)
            return false;
    }
    return false;
}   // nextType

// ----------------------------------------------------------------------------
bool AddonsManager::previous()
{
    if(m_index - 1 > 0)
    {
        m_index --;
        return true;
    }
    m_index = m_addons_list.size() - 1;
    return false;
}   // previous

// ----------------------------------------------------------------------------
bool AddonsManager::previousType(std::string type)
{
    while(previous())
    {
        if(m_addons_list[m_index].type == type)
            return true;
    }
    while(previous())
    {
        if(m_addons_list[m_index].type == type)
            return false;
    }
    return false;
}   // previousType

// ----------------------------------------------------------------------------
bool AddonsManager::select(std::string name)
{
    //the unsigned is to remove the compiler warnings, maybe it is a bad idea ?
    for(unsigned int i = 0; i < m_addons_list.size(); i++)
        {
            if(m_addons_list[i].name == name)
            {
                m_index = i;
                return true;
            }
        }
    return false;
}   // select

// ----------------------------------------------------------------------------
bool AddonsManager::selectId(std::string id)
{
    //the unsigned is to remove the compiler warnings, maybe it is a bad idea ?
    for(unsigned int i = 0; i < m_addons_list.size(); i++)
        {
            if(m_addons_list[i].id == id)
            {
                m_index = i;
                return true;
            }
        }
    return false;
}   // selectId

// ----------------------------------------------------------------------------
/* FIXME : remove this function */
addons_prop AddonsManager::getAddons()
{
    return m_addons_list[m_index];
}   // getAddons

// ----------------------------------------------------------------------------
std::string AddonsManager::getVersionAsStr() const
{
    std::ostringstream os;
    os << m_addons_list[m_index].version;
    return os.str();
}   // getVersionAsStr

// ----------------------------------------------------------------------------
std::string AddonsManager::getIdAsStr() const
{
    std::ostringstream os;
    os << m_addons_list[m_index].id;
    return os.str();
}   // getIdAsStr

// ----------------------------------------------------------------------------
int AddonsManager::getInstalledVersion() const
{
    if(m_addons_list[m_index].installed)
        return m_addons_list[m_index].installed_version;
    return 0;
}   // getInstalledVersion

// ----------------------------------------------------------------------------
std::string AddonsManager::getInstalledVersionAsStr() const
{
    if(m_addons_list[m_index].installed)
    {
        std::ostringstream os;
        os << m_addons_list[m_index].installed_version;
        return os.str();
    }
    return "";
}   // getInstalledVersionAsStr

// ----------------------------------------------------------------------------
void AddonsManager::install()
{
    //download of the addons file
    
    pthread_mutex_lock(&m_str_mutex);
    m_str_state = "Downloading...";
    pthread_mutex_unlock(&m_str_mutex);

    std::string file = "file/" + m_addons_list[m_index].file;
    bool success = download(file,
                            m_addons_list[m_index].name, &m_download_state);
    
    if (!success)
    {
        // TODO: show a message in the interface
        fprintf(stderr, "[Addons] Failed to download '%s'\n", file.c_str());
        return;
    }
    
    file_manager->checkAndCreateDirForAddons(m_addons_list[m_index].name,
                                             m_addons_list[m_index].type + "s/");

    //extract the zip in the addons folder called like the addons name    
    std::string dest_file =file_manager->getAddonsDir() + "/" + "data" + "/" +
                m_addons_list[m_index].type + "s/" +
                m_addons_list[m_index].name + "/" ;
    std::string from = file_manager->getConfigDir() + "/" + m_addons_list[m_index].name;
    std::string to = dest_file;
    
    pthread_mutex_lock(&m_str_mutex);
    m_str_state = "Unzip the addons...";
    pthread_mutex_unlock(&m_str_mutex);

    success = extract_zip(from, to);
    if (!success)
    {
        // TODO: show a message in the interface
        std::cerr << "[Addons] Failed to unzip '" << from << "' to '" << to << "'\n";
        return;
    }

    m_addons_list[m_index].installed = true;
    m_addons_list[m_index].installed_version = m_addons_list[m_index].version;
    
    pthread_mutex_lock(&m_str_mutex);
    m_str_state = "Reloading kart list...";
    pthread_mutex_unlock(&m_str_mutex);
    saveInstalled();
}   // install

// ----------------------------------------------------------------------------
void AddonsManager::saveInstalled()
{
    //Put the addons in the xml file
    //Manually because the irrlicht xml writer doesn't seem finished, FIXME ?
    std::ofstream xml_installed(m_file_installed.c_str());

    //write the header of the xml file
    xml_installed << "<?xml version=\"1.0\"?>" << std::endl;
    xml_installed << "<addons  xmlns='http://stkaddons.tuxfamily.org/'>"
                    << std::endl;

    for(unsigned int i = 0; i < m_addons_list.size(); i++)
    {
        if(m_addons_list[i].installed)
        {
            std::ostringstream os;
            os << m_addons_list[i].installed_version;

            //transform the version (int) in string
            xml_installed << "<"+ m_addons_list[i].type +" name=\"" +
                        m_addons_list[i].name + "\" id=\"" +
                        m_addons_list[i].id + "\"";
            xml_installed << " version=\"" + os.str() + "\" />" << std::endl;
        }
    }
    xml_installed << "</addons>" << std::endl;
    xml_installed.close();
    kart_properties_manager->reLoadAllKarts();
	track_manager->loadTrackList();
}   // saveInstalled

// ----------------------------------------------------------------------------
void AddonsManager::uninstall()
{
    std::cout << "[Addons] Uninstalling <" << m_addons_list[m_index].name << ">\n";

    m_addons_list[m_index].installed = false;
    //write the xml file with the informations about installed karts
    std::string dest_file = file_manager->getAddonsDir() + "/" + "data" + "/" +
                m_addons_list[m_index].type + "s/" +
                m_addons_list[m_index].name + "/";

    //remove the addons directory
    file_manager->removeDirectory(dest_file.c_str());
    saveInstalled();

}   // uninstall

// ----------------------------------------------------------------------------
int AddonsManager::getDownloadState()
{
    pthread_mutex_lock(&download_mutex);
    int value = m_download_state;
    pthread_mutex_unlock(&download_mutex);
    return value;
}

// ----------------------------------------------------------------------------

std::string AddonsManager::getDownloadStateAsStr() const
{
    pthread_mutex_lock(&m_str_mutex);
    std::string value = m_str_state;
    pthread_mutex_unlock(&m_str_mutex);
    return value;
}   // getDownloadStateAsStr

// ----------------------------------------------------------------------------

bool AddonsManager::needUpdate() const
{
    return getInstalledVersion() < getVersion();
}   // needUpdate
#endif
