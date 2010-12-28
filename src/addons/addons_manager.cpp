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

#include "addons/network_http.hpp"
#include "addons/zip.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "karts/kart_properties_manager.hpp"
#include "states_screens/kart_selection.hpp"
#include "tracks/track_manager.hpp"

AddonsManager* addons_manager = 0;

// ----------------------------------------------------------------------------
/** Initialises the non-online component of the addons manager (i.e. handling
 *  the list of already installed addons). The online component is initialised
 *  later from a separate thread in network_http (once network_http is setup).
 */
AddonsManager::AddonsManager() : m_state(STATE_INIT)
{
    m_index = -1;

    m_file_installed = file_manager->getConfigDir() 
                     + "/" + "addons_installed.xml";
    getInstalledAddons();
}   // AddonsManager

// ----------------------------------------------------------------------------
/** This initialises the online portion of the addons manager. It downloads
 *  the list of available addons. This is called by network_http before it
 *  goes into command-receiving mode, so we can't use any asynchronous calls
 *  here (though this is being called from a separate thread anyway, so the
 *  main GUI is not blocked). This function will update the state variable
 *  
 */
void AddonsManager::initOnline()
{
    if(UserConfigParams::m_verbosity>=3)
        printf("[addons] Init online addons manager\n");
    int download_state = 0;
    m_download_state = download_state;

    // FIXME: It is _very_ dirty to save the list as a locale file 
    //   since we have a function to load it directly in a string.
    if(UserConfigParams::m_verbosity>=3)
        printf("[addons] Addons manager downloading list\n");
    if(!network_http->downloadFileSynchron("list"))
    {
        m_state.set(STATE_ERROR);
        return;
    }
    if(UserConfigParams::m_verbosity>=3)
        printf("[addons] Addons manager list downloaded\n");

    std::string xml_file = file_manager->getAddonsFile("list");

    const XMLNode *xml = new XMLNode(xml_file);
    for(unsigned int i=0; i<xml->getNumNodes(); i++)
    {
        const XMLNode *node = xml->getNode(i);
        if(node->getName()=="track")
        {
            AddonsProp addons(*node);
            m_addons_list.push_back(addons);
        }
        else if(node->getName()=="kart")
        {
            AddonsProp addons(*node);
            m_addons_list.push_back(addons);
        }
        else
        {
            fprintf(stderr, 
                    "Found invalid node '%s' while downloading addons.\n",
                    node->getName().c_str());
            fprintf(stderr, "Ignored.\n");
        }
    }   // for i<xml->getNumNodes
    delete xml;

    m_state.set(STATE_READY);
}   // initOnline

// ----------------------------------------------------------------------------
/** Returns true if the list of online addons has been downloaded. This is 
 *  used to grey out the 'addons' entry till a network connections could be
 *  established.
 */
bool AddonsManager::onlineReady()
{
    return m_state.get()==STATE_READY;
}   // onlineReady

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
    io::IrrXMLReader* xml = io::createIrrXMLReader(m_file_installed.c_str());

    // parse the file until end reached

    while(xml && xml->read())
    {
        std::string name;
        std::string id;
        int version = 0;
        switch(xml->getNodeType())
        {
        case io::EXN_ELEMENT:
            {
                if (!strcmp("kart",  xml->getNodeName()) || 
                    !strcmp("track", xml->getNodeName())   )
                {
                    std::cout << xml->getAttributeCount() << std::endl;
                    //the unsigned is to remove the compiler warnings, 
                    // maybe it is a bad idea ?
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
                        m_addons_list[m_index].m_installed = true;
                        m_addons_list[m_index].m_installed_version = version;
                        std::cout << "[Addons] An addon is already installed: " 
                                  << id << std::endl;
                    }
                    else
                    {
                        AddonsProp addons;
                        addons.m_type = xml->getNodeName();
                        addons.m_name = name;
                        addons.m_installed_version = version;
                        addons.m_version = version;
                        addons.m_installed = true;
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
        if(m_addons_list[m_index].m_type == type)
            return true;
    }
    while(next())
    {
        if(m_addons_list[m_index].m_type == type)
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
        if(m_addons_list[m_index].m_type == type)
            return true;
    }
    while(previous())
    {
        if(m_addons_list[m_index].m_type == type)
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
            if(m_addons_list[i].m_name == name)
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
    for(unsigned int i = 0; i < m_addons_list.size(); i++)
    {
        if(m_addons_list[i].m_id == id)
        {
            m_index = i;
            return true;
        }
    }
    return false;
}   // selectId

// ----------------------------------------------------------------------------
/* FIXME : remove this function */
const AddonsManager::AddonsProp& AddonsManager::getAddons() const
{
    return m_addons_list[m_index];
}   // getAddons

// ----------------------------------------------------------------------------
std::string AddonsManager::getVersionAsStr() const
{
    std::ostringstream os;
    os << m_addons_list[m_index].m_version;
    return os.str();
}   // getVersionAsStr

// ----------------------------------------------------------------------------
std::string AddonsManager::getIdAsStr() const
{
    std::ostringstream os;
    os << m_addons_list[m_index].m_id;
    return os.str();
}   // getIdAsStr

// ----------------------------------------------------------------------------
int AddonsManager::getInstalledVersion() const
{
    if(m_addons_list[m_index].m_installed)
        return m_addons_list[m_index].m_installed_version;
    return 0;
}   // getInstalledVersion

// ----------------------------------------------------------------------------
std::string AddonsManager::getInstalledVersionAsStr() const
{
    if(m_addons_list[m_index].m_installed)
    {
        std::ostringstream os;
        os << m_addons_list[m_index].m_installed_version;
        return os.str();
    }
    return "";
}   // getInstalledVersionAsStr

// ----------------------------------------------------------------------------
void AddonsManager::install()
{
    //download of the addons file
    
    m_str_state = "Downloading...";

    std::string file = "file/" + m_addons_list[m_index].m_file;
    network_http->downloadFileAsynchron(file, m_addons_list[m_index].m_name);
    //FIXME , &m_download_state);
    bool success=true;
    if (!success)
    {
        // TODO: show a message in the interface
        fprintf(stderr, "[Addons] Failed to download '%s'\n", file.c_str());
        return;
    }
    
    file_manager->checkAndCreateDirForAddons(m_addons_list[m_index].m_name,
                                             m_addons_list[m_index].m_type + "s/");

    //extract the zip in the addons folder called like the addons name    
    std::string dest_file =file_manager->getAddonsDir() + "/" + "data" + "/" +
                m_addons_list[m_index].m_type + "s/" +
                m_addons_list[m_index].m_name + "/" ;
    std::string from = file_manager->getConfigDir() + "/" 
                     + m_addons_list[m_index].m_name;
    std::string to = dest_file;
    
    m_str_state = "Unzip the addons...";

    success = extract_zip(from, to);
    if (!success)
    {
        // TODO: show a message in the interface
        std::cerr << "[Addons] Failed to unzip '" << from << "' to '" 
                  << to << "'\n";
        return;
    }

    m_addons_list[m_index].m_installed = true;
    m_addons_list[m_index].m_installed_version = m_addons_list[m_index].m_version;
    
    m_str_state = "Reloading kart list...";
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
        if(m_addons_list[i].m_installed)
        {
            std::ostringstream os;
            os << m_addons_list[i].m_installed_version;

            //transform the version (int) in string
            xml_installed << "<"+ m_addons_list[i].m_type +" name=\"" +
                        m_addons_list[i].m_name + "\" id=\"" +
                        m_addons_list[i].m_id + "\"";
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
    std::cout << "[Addons] Uninstalling <" 
              << m_addons_list[m_index].m_name << ">\n";

    m_addons_list[m_index].m_installed = false;
    //write the xml file with the informations about installed karts
    std::string dest_file = file_manager->getAddonsDir() + "/" + "data" + "/" +
                m_addons_list[m_index].m_type + "s/" +
                m_addons_list[m_index].m_name + "/";

    //remove the addons directory
    file_manager->removeDirectory(dest_file.c_str());
    saveInstalled();

}   // uninstall

// ----------------------------------------------------------------------------
int AddonsManager::getDownloadState()
{
    int value = m_download_state;
    return value;
}

// ----------------------------------------------------------------------------

const std::string& AddonsManager::getDownloadStateAsStr() const
{
    return m_str_state;
}   // getDownloadStateAsStr

// ----------------------------------------------------------------------------

bool AddonsManager::needUpdate() const
{
    return getInstalledVersion() < getVersion();
}   // needUpdate
#endif
