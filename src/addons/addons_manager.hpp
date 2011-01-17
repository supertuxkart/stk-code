//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Lucas Baudin, Joerg Henrichs
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

#ifdef ADDONS_MANAGER

#ifndef HEADER_ADDONS_MANAGER_HPP
#define HEADER_ADDONS_MANAGER_HPP

#include <string>
#include <map>
#include <vector>

#include "addons/addon.hpp"
#include "io/xml_node.hpp"
#include "utils/synchronised.hpp"

class AddonsManager
{
private:
    /** The list of all addons - installed or uninstalled. The list is
     *  combined from the addons_installed.xml file first, then information
     *  from the downloaded list of items is merged/added to that. */
    Synchronised<std::vector<Addon> >  m_addons_list;
    /** Full filename of the addons_installed.xml file. */
    std::string                        m_file_installed;
    std::string                        m_type;
    int                                m_download_state;

    /** List of loaded icons. */
    std::vector<std::string> m_icon_list;

    /** Queue of icons to download. This queue is used by the
     *  GUI to increase priority of icons that are needed now. */
    Synchronised<std::vector<std::string> > m_icon_queue;

    /** Which state the addons manager is:
    *  INIT:  Waiting to download the list of addons.
    *  READY: List is downloaded, manager is ready.
    *  ERROR: Error downloading the list, no addons available. */
    enum  STATE_TYPE {STATE_INIT, STATE_READY, STATE_ERROR};
    // Synchronise the state between threads (e.g. GUI and update thread)
    Synchronised<STATE_TYPE> m_state;

    void         saveInstalled(const std::string &type="");
    void         loadInstalledAddons();
    static void *downloadIcons(void *obj);

public:
         AddonsManager();
    void initOnline(const XMLNode *xml);
    bool onlineReady();
    /** Marks addon as not being available. */
    void setErrorState() { m_state.set(STATE_ERROR); }

    /** Returns the list of addons (installed and uninstalled). */
    unsigned int getNumAddons() const 
    {
        return m_addons_list.getData().size(); 
    }

    /** Returns the i-th addons. */
    const Addon& getAddon(unsigned int i) { return m_addons_list.getData()[i];}
    const Addon* getAddon(const std::string &id) const;
    int   getAddonIndex(const std::string &id) const;

    bool install(const Addon &addon);

    /** Uninstall the selected addon. This method will remove all the 
    *  directory of the addon.*/
    bool uninstall(const Addon &addon);

    /** Get the install state (if it is the download, unzip...)*/
    const std::string& getDownloadStateAsStr() const;

};

extern AddonsManager *addons_manager;
#endif
#endif
