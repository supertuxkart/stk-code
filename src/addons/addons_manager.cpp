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

#include "addons/addons_manager.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string.h>
#include <vector>

#include "addons/network_http.hpp"
#include "addons/request.hpp"
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
AddonsManager::AddonsManager() : m_addons_list(std::vector<Addon>() ),
                                 m_state(STATE_INIT)
{
    m_file_installed = file_manager->getAddonsFile("addons_installed.xml");
}   // AddonsManager

// ----------------------------------------------------------------------------
/** This initialises the online portion of the addons manager. It uses the
 *  downloaded list of available addons. This is called by network_http before
 *  it goes into command-receiving mode, so we can't use any asynchronous calls
 *  here (though this is being called from a separate thread , so the
 *  main GUI is not blocked anyway). This function will update the state 
 *  variable
 */
void AddonsManager::initOnline(const XMLNode *xml)
{
    m_addons_list.lock();
    loadInstalledAddons();
    m_addons_list.unlock();

    for(unsigned int i=0; i<xml->getNumNodes(); i++)
    {
        const XMLNode *node = xml->getNode(i);
        const std::string &name = node->getName();
        // Ignore news/redirect, which is handled by network_http
        if(name=="include" || name=="message") continue;
        if(node->getName()=="track" || node->getName()=="kart")
        {
            Addon addon(*node);
            int index = getAddonIndex(addon.getId());

            int stk_version=0;
            node->get("format", &stk_version);
            int   testing=-1;
            node->get("testing", &testing);

            bool wrong_version=false;

            if(addon.getType()=="kart")
                wrong_version = stk_version <stk_config->m_min_kart_version ||
                                stk_version >stk_config->m_max_kart_version   ;
            else
                wrong_version = stk_version <stk_config->m_min_track_version ||
                                stk_version >stk_config->m_max_track_version   ;
            // Check which version to use: only for this stk version,
            // and not addons that are marked as hidden (testing=0)
            if(wrong_version|| testing==0)
            {
                // If the version is too old (e.g. after an update of stk)
                // remove a cached icon.
                std::string full_path = 
                    file_manager->getAddonsFile("icons/"
                                                +addon.getIconBasename());
                if(file_manager->fileExists(full_path))
                {
                    if(UserConfigParams::logAddons())
                        printf("[addons] Removing cached icon '%s'.\n", 
                               addon.getIconBasename().c_str());
                    file_manager->removeFile(full_path);
                }
                continue;
            }

            m_addons_list.lock();
            if(index>=0)
                m_addons_list.getData()[index].copyInstallData(addon);
            else
            {
                m_addons_list.getData().push_back(addon);
                index = m_addons_list.getData().size()-1;
            }
            m_addons_list.unlock();
        }
        else
        {
            fprintf(stderr, 
                    "[addons] Found invalid node '%s' while downloading addons.\n",
                    node->getName().c_str());
            fprintf(stderr, "[addons] Ignored.\n");
        }
    }   // for i<xml->getNumNodes
    delete xml;

    m_state.set(STATE_READY);

    downloadIcons();
}   // initOnline

// ----------------------------------------------------------------------------
/** Download all necessary icons (i.e. icons that are either missing or have 
 *  been updated since they were downloaded).
 */
void *AddonsManager::downloadIcons()
{
    for(unsigned int i=0; i<m_addons_list.getData().size(); i++)
    {
        Addon &addon            = m_addons_list.getData()[i];
        const std::string &icon = addon.getIconBasename();
        const std::string &icon_full
                                = file_manager->getAddonsFile("icons/"+icon); 
        if(addon.iconNeedsUpdate() ||
            !file_manager->fileExists(icon_full))
        {
            const std::string &url  = addon.getIconURL();
            const std::string &icon = addon.getIconBasename();
            std::string save        = "icons/"+icon;
            Request *r = network_http->downloadFileAsynchron(url, save, 
                                                 /*priority*/1,
                                               /*manage_mem*/false);
            r->setAddonIconNotification(&addon);            
        }
        else
            m_addons_list.getData()[i].setIconReady();
    }   // for i<m_addons_list.size()

    return NULL;
}   // downloadIcons

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
/** Loads the installed addons from .../addons/addons_installed.xml.
 */
void AddonsManager::loadInstalledAddons()
{
    /* checking for installed addons */
    std::cout << "[addons] Loading an xml file for installed addons: ";
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
            Addon addon(*node);
            m_addons_list.getData().push_back(addon);
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
    return (i<0) ? NULL : &(m_addons_list.getData()[i]);
}   // getAddon

// ----------------------------------------------------------------------------
int AddonsManager::getAddonIndex(const std::string &id) const
{
    for(unsigned int i = 0; i < m_addons_list.getData().size(); i++)
    {
        if(m_addons_list.getData()[i].getId()== id)
        {
            return i;
        }
    }
    return -1;
}   // getAddonIndex

// ----------------------------------------------------------------------------
/** Installs or updates (i.e. = install on top of an existing installation) an 
 *  addon. It checks for the directories and then unzips the file (which must 
 *  already have been downloaded).
 *  \param addon Addon data for the addon to install.
 *  \return true if installation was successful.
 */
bool AddonsManager::install(const Addon &addon)
{
    bool success=true;
    const std::string &id = addon.getId();
    file_manager->checkAndCreateDirForAddons(id, addon.getTypeDirectory());

    //extract the zip in the addons folder called like the addons name    
    std::string base_name = StringUtils::getBasename(addon.getZipFileName());
    std::string from      = file_manager->getAddonsFile("tmp/"+base_name);
    std::string to        = addon.getDataDir();
    
    success = extract_zip(from, to);
    if (!success)
    {
        // TODO: show a message in the interface
        std::cerr << "[Addons] Failed to unzip '" << from << "' to '" 
                  << to << "'\n";
        return false;
    }

    int index = getAddonIndex(addon.getId());
    assert(index>=0 && index < (int)m_addons_list.getData().size());
    m_addons_list.getData()[index].setInstalled(true);
    
    if(addon.getType()=="kart")
    {
        // We have to remove the mesh of the kart since otherwise it remains
        // cashed (if a kart is updated), and will therefore be found again 
        // when reloading the karts. This is important on one hand since we 
        // reload all karts (this function is easily available) and existing
        // karts will not reload their meshes.
        const KartProperties *prop = 
            kart_properties_manager->getKart(addon.getId());
        // If the model already exist (i.e. it's an update, not a new install)
        // make sure to remove the cached copy of the mesh
        if(prop)
        {
            const KartModel &model = prop->getMasterKartModel();
            irr_driver->removeMeshFromCache(model.getModel());
        }
    }
    saveInstalled(addon.getType());
    return true;
}   // install

// ----------------------------------------------------------------------------
/** Removes all files froma login.
 *  \param addon The addon to be removed.
 *  \return True if uninstallation was successful.
 */
bool AddonsManager::uninstall(const Addon &addon)
{
    std::cout << "[Addons] Uninstalling <" 
              << addon.getName() << ">\n";

    // addon is a const reference, and to avoid removing the const, we
    // find the proper index again to modify the installed state
    int index = getAddonIndex(addon.getId());
    assert(index>=0 && index < (int)m_addons_list.getData().size());
    m_addons_list.getData()[index].setInstalled(false);

    //remove the addons directory
    bool error = !file_manager->removeDirectory(addon.getDataDir());
    saveInstalled(addon.getType());
    return !error;
}   // uninstall

// ----------------------------------------------------------------------------
void AddonsManager::saveInstalled(const std::string &type)
{
    //Put the addons in the xml file
    //Manually because the irrlicht xml writer doesn't seem finished, FIXME ?
    std::ofstream xml_installed(m_file_installed.c_str());

    //write the header of the xml file
    xml_installed << "<?xml version=\"1.0\"?>" << std::endl;
    xml_installed << "<addons  xmlns='http://stkaddons.net/'>"
                    << std::endl;

    for(unsigned int i = 0; i < m_addons_list.getData().size(); i++)
    {
        m_addons_list.getData()[i].writeXML(&xml_installed);
    }
    xml_installed << "</addons>" << std::endl;
    xml_installed.close();
    if(type=="kart")
        kart_properties_manager->reLoadAllKarts();
    else if(type=="track")
        track_manager->loadTrackList();
}   // saveInstalled

