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

#include <string.h>
#include <map>
#include <vector>
#include <iostream>
#include "addons/addons.hpp"
#include "irrXML.h"
#include "addons/network.hpp"
#include "addons/zip.hpp"

#include <fstream>
#include <sstream>
#include "io/file_manager.hpp"

#include "karts/kart_properties_manager.hpp"
#include "states_screens/kart_selection.hpp"

using namespace irr; /* irrXML which is used to read (not write) xml file,
is located in the namespace irr::io.*/
using namespace io;
Addons* addons_manager = 0;
// ----------------------------------------------------------------------------

Addons::Addons()
{
    this->index = -1;
    std::cout << "Loading an xml file for addons: ";
    int download_state = 0;
    m_download_state = download_state;
    pthread_mutex_init(&m_str_mutex, NULL);

    download("list");
    std::string xml_file = file_manager->getConfigDir() + "/" + "list";
    std::cout << xml_file << std::endl;
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
                this->m_addons_list.push_back(addons);
            }
        }
    }
    delete xml;

    this->file_installed = file_manager->getConfigDir() + "/" + "addons_installed.xml";
    this->GetInstalledAddons();
}
void Addons::resetIndex()
{
    this->index = -1;
}
// ----------------------------------------------------------------------------
void Addons::GetInstalledAddons()
{
    std::string attribute_name;
    int old_index = this->index;

    /* checking for installed addons */

    std::cout << "Loading an xml file for installed addons: ";
    std::cout << this->file_installed << std::endl;
    IrrXMLReader* xml = createIrrXMLReader(this->file_installed.c_str());

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
                    if(this->SelectId(id))
                    {
                        this->m_addons_list[this->index].installed = true;
                        this->m_addons_list[this->index].installed_version = version;
                        std::cout << "An addons is already installed: " + id << std::endl;
                    }
                    else
                    {
                        addons_prop addons;
                        addons.type = xml->getNodeName();
                        addons.name = name;
                        addons.installed_version = version;
                        addons.installed = true;
                        this->m_addons_list.push_back(addons);
                    }
                }
            }
            break;
        default : break;
        }
    }
    delete xml;
    this->index = old_index;
}


// ----------------------------------------------------------------------------
bool Addons::Next()
{
    if(this->index + 1 < (int)this->m_addons_list.size())
    {
        this->index ++;
        return true;
    }
    this->index = -1;
    return false;
}
// ----------------------------------------------------------------------------
bool Addons::NextType(std::string type)
{
    while(this->Next())
    {
        if(this->m_addons_list[this->index].type == type)
            return true;
    }
    while(this->Next())
    {
        if(this->m_addons_list[this->index].type == type)
            return false;
    }
    return false;
}
// ----------------------------------------------------------------------------
bool Addons::Previous()
{
    if(this->index - 1 > 0)
    {
        this->index --;
        return true;
    }
    this->index = this->m_addons_list.size() - 1;
    return false;
}
// ----------------------------------------------------------------------------
bool Addons::PreviousType(std::string type)
{
    while(this->Previous())
    {
        if(this->m_addons_list[this->index].type == type)
            return true;
    }
    while(this->Previous())
    {
        if(this->m_addons_list[this->index].type == type)
            return false;
    }
    return false;
}
// ----------------------------------------------------------------------------
bool Addons::Select(std::string name)
{
    //the unsigned is to remove the compiler warnings, maybe it is a bad idea ?
    for(unsigned int i = 0; i < this->m_addons_list.size(); i++)
        {
            if(this->m_addons_list[i].name == name)
            {
                this->index = i;
                return true;
            }
        }
    return false;
}
// ----------------------------------------------------------------------------
bool Addons::SelectId(std::string id)
{
    //the unsigned is to remove the compiler warnings, maybe it is a bad idea ?
    for(unsigned int i = 0; i < this->m_addons_list.size(); i++)
        {
            if(this->m_addons_list[i].id == id)
            {
                this->index = i;
                return true;
            }
        }
    return false;
}


// ----------------------------------------------------------------------------
/* FIXME : remove this function */
addons_prop Addons::GetAddons()
{
    return this->m_addons_list[this->index];
}
// ----------------------------------------------------------------------------
std::string Addons::IsInstalled()
{
    if(this->m_addons_list[this->index].installed)
    {
        return "yes";
    }
    return "no";
}
// ----------------------------------------------------------------------------
std::string Addons::GetVersionAsStr()
{
    //maybe it is dirty, FIXME ?
    std::ostringstream os;
    os << this->m_addons_list[this->index].version;
    return os.str();
}
// ----------------------------------------------------------------------------
std::string Addons::GetIdAsStr()
{
    std::ostringstream os;
    os << this->m_addons_list[this->index].id;
    return os.str();
}
// ----------------------------------------------------------------------------
int Addons::GetInstalledVersion()
{
    if(this->m_addons_list[this->index].installed)
        return this->m_addons_list[this->index].installed_version;
    return 0;
}
// ----------------------------------------------------------------------------
std::string Addons::GetInstalledVersionAsStr()
{
    if(this->m_addons_list[this->index].installed)
    {
        std::ostringstream os;
        os << this->m_addons_list[this->index].installed_version;
        return os.str();
    }
    return "";
}
// ----------------------------------------------------------------------------
void Addons::Install()
{
    //download of the addons file
    
    pthread_mutex_lock(&m_str_mutex);
    m_str_state = "Downloading...";
    pthread_mutex_unlock(&m_str_mutex);

    download(std::string("file/" + this->m_addons_list[this->index].file),
            this->m_addons_list[this->index].name, &m_download_state);
    file_manager->checkAndCreateDirForAddons(this->m_addons_list[this->index].name,
        this->m_addons_list[this->index].type + "s/");

    //extract the zip in the addons folder called like the addons name    
    std::string dest_file =file_manager->getAddonsDir() + "/" + "data" + "/" +
                this->m_addons_list[this->index].type + "s/" +
                this->m_addons_list[this->index].name + "/" ;
    std::string from = file_manager->getConfigDir() + "/" + this->m_addons_list[this->index].name;
    std::string to = dest_file;
    
    pthread_mutex_lock(&m_str_mutex);
    m_str_state = "Unzip the addons...";
    pthread_mutex_unlock(&m_str_mutex);

    const bool success = extract_zip(from, to);
    if (!success)
    {
        // TODO: show a message in the interface
        std::cerr << "Failed to unzip " << from << " to " << to << std::endl;
        return;
    }

    this->m_addons_list[this->index].installed = true;
    this->m_addons_list[this->index].installed_version = this->m_addons_list[this->index].version;
    
    pthread_mutex_lock(&m_str_mutex);
    m_str_state = "Reloaing karts list...";
    pthread_mutex_unlock(&m_str_mutex);
    this->SaveInstalled();
}
// ----------------------------------------------------------------------------
void Addons::SaveInstalled()
{
    //Put the addons in the xml file
    //Manually because the irrlicht xml writer doesn't seem finished, FIXME ?
    std::ofstream xml_installed(this->file_installed.c_str());

    //write the header of the xml file
    xml_installed << "<?xml version=\"1.0\"?>" << std::endl;
    xml_installed << "<addons  xmlns='http://stkaddons.tuxfamily.org/'>"
                    << std::endl;

    for(unsigned int i = 0; i < this->m_addons_list.size(); i++)
    {
        if(this->m_addons_list[i].installed)
        {
            std::ostringstream os;
            os << this->m_addons_list[i].installed_version;

            //transform the version (int) in string
            xml_installed << "<"+ this->m_addons_list[i].type +" name=\"" +
                        this->m_addons_list[i].name + "\" id=\"" +
                        this->m_addons_list[i].id + "\"";
            xml_installed << " version=\"" + os.str() + "\" />" << std::endl;
        }
    }
    xml_installed << "</addons>" << std::endl;
    xml_installed.close();
    kart_properties_manager->reLoadAllKarts();
}
// ----------------------------------------------------------------------------
void Addons::UnInstall()
{
    std::cout << "Uninstall: " << this->m_addons_list[this->index].name << std::endl;

    this->m_addons_list[this->index].installed = false;
    //write the xml file with the informations about installed karts
    std::string dest_file = file_manager->getAddonsDir() + "/" + "data" + "/" +
                this->m_addons_list[this->index].type + "s/" +
                this->m_addons_list[this->index].name + "/";

    //remove the addons directory
    file_manager->removeDirectory(dest_file.c_str());
    this->SaveInstalled();

}
// ----------------------------------------------------------------------------
int Addons::getDownloadState()
{
    pthread_mutex_lock(&download_mutex);
    int value = m_download_state;
    pthread_mutex_unlock(&download_mutex);
    return value;
}
std::string Addons::getDownloadStateAsStr()
{
    pthread_mutex_lock(&m_str_mutex);
    std::string value = m_str_state;
    pthread_mutex_unlock(&m_str_mutex);
    return value;
}


// ----------------------------------------------------------------------------
bool Addons::NeedUpdate()
{
    return GetInstalledVersion() < GetVersion();
}
#endif
