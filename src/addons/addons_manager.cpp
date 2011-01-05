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
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "karts/kart_properties_manager.hpp"
#include "states_screens/kart_selection.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"

AddonsManager* addons_manager = 0;

// ----------------------------------------------------------------------------
/** Initialises the non-online component of the addons manager (i.e. handling
 *  the list of already installed addons). The online component is initialised
 *  later from a separate thread in network_http (once network_http is setup).
 */
AddonsManager::AddonsManager() : m_state(STATE_INIT)
{
    m_file_installed = file_manager->getAddonsFile("addons_installed.xml");
    loadInstalledAddons();
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
        if(node->getName()=="track" || node->getName()=="kart")
        {
            Addon addon(*node);
            int index = getAddonIndex(addon.getId());
            if(index>=0)
                m_addons_list[index].copyInstallData(addon);
            else
                m_addons_list.push_back(addon);
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
/** Loads the installed addons from .../addons/addons_installed.xml. This is
 *  called before network_http is constructed (so no need to protect 
 *  m_addons_list with a mutex).
 */
void AddonsManager::loadInstalledAddons()
{
    /* checking for installed addons */
    std::cout << "[Addons] Loading an xml file for installed addons: ";
    std::cout << m_file_installed << std::endl;
    const XMLNode *xml = file_manager->createXMLTree(m_file_installed);
    if(!xml)
        return;

    for(unsigned int i=0; i<xml->getNumNodes(); i++)
    {
        const XMLNode *node=xml->getNode(i);
        if(node->getName()=="kart"   ||
            node->getName()=="track"    )
        {
            Addon addon(*node, /*installed*/ true);
            m_addons_list.push_back(addon);
        }
    }   // for i <= xml->getNumNodes()

    delete xml;
}   // loadInstalledAddons

// ----------------------------------------------------------------------------
/** Returns an addon with a given id. Raises an assertion if the id is not 
 *  found!
 *  \param id The id to search for.
 */
const Addon* AddonsManager::getAddon(const std::string &id) const
{
    int i = getAddonIndex(id);
    return (i<0) ? NULL : &(m_addons_list[i]);
}   // getAddon

// ----------------------------------------------------------------------------
int AddonsManager::getAddonIndex(const std::string &id) const
{
    for(unsigned int i = 0; i < m_addons_list.size(); i++)
    {
        if(m_addons_list[i].getId()== id)
        {
            return i;
        }
    }
    return -1;
}   // getAddonIndex

// ----------------------------------------------------------------------------
void AddonsManager::install(const Addon &addon)
{
    bool success=true;
    const std::string &id = addon.getId();
    file_manager->checkAndCreateDirForAddons(id, addon.getType()+ "s/");

    //extract the zip in the addons folder called like the addons name    
    std::string dest_file = file_manager->getAddonsDir() + "/"
                          + addon.getType()+ "s/" + id + "/" ;
    std::string base_name = StringUtils::getBasename(addon.getZipFileName());
    std::string from      = file_manager->getAddonsFile(base_name);
    std::string to        = dest_file;
    
    m_str_state = "Unzip the addons...";

    success = extract_zip(from, to);
    if (!success)
    {
        // TODO: show a message in the interface
        std::cerr << "[Addons] Failed to unzip '" << from << "' to '" 
                  << to << "'\n";
        return;
    }

    int index = getAddonIndex(addon.getId());
    assert(index>=0 && index < (int)m_addons_list.size());
    m_addons_list[index].setInstalled(true);
    
    m_str_state = "Reloading kart list...";
    if(addon.getType()=="kart")
    {
        // We have to remove the mesh of the kart since otherwise it remains
        // cashed, and will therefore be found again when reloading the karts.
        // This is important on one hand since we reload all karts (this
        // function is easily available) and existing karts will not reload
        // their meshes.
        const KartProperties *prop = kart_properties_manager->getKart(addon.getId());
        const KartModel &model = prop->getMasterKartModel();
        irr_driver->removeMesh(model.getModel());
    }
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
            m_addons_list[i].writeXML(&xml_installed);
        }
    }
    xml_installed << "</addons>" << std::endl;
    xml_installed.close();
    kart_properties_manager->reLoadAllKarts();
	track_manager->loadTrackList();
}   // saveInstalled

// ----------------------------------------------------------------------------
void AddonsManager::uninstall(const Addon &addon)
{
    std::cout << "[Addons] Uninstalling <" 
              << addon.getName() << ">\n";

    // addon is a const reference, and to avoid removing the const, we
    // find the proper index again to modify the installed state
    int index = getAddonIndex(addon.getId());
    assert(index>=0 && index < (int)m_addons_list.size());
    m_addons_list[index].setInstalled(false);

    //write the xml file with the informations about installed karts
    std::string name=addon.getType()+"s/"+addon.getId();
    std::string dest_file = file_manager->getAddonsFile(name);

    //remove the addons directory
    file_manager->removeDirectory(dest_file);
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

#endif
