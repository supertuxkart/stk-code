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

#include "io/xml_node.hpp"
#include "utils/synchronised.hpp"

class AddonsManager
{
    class AddonsProp
    {
    public:
        std::string m_name;
        int         m_version;
        int         m_installed_version;
        std::string m_description;
        std::string m_icon;
        std::string m_file;
        std::string m_id;
        bool        m_installed;
        std::string m_type;
        AddonsProp() {};
        AddonsProp(const XMLNode &xml)
        {
            m_installed         = false;
            m_installed_version = 0;
            m_type        = xml.getName();
            m_name        = ""; xml.get("name",        &m_name       );
            m_version     = 0 ; xml.get("version",     &m_version    );
            m_file        = ""; xml.get("file",        &m_file       );
            m_description = ""; xml.get("description", &m_description);
            m_icon        = ""; xml.get("icon",        &m_icon       );
            m_id          = ""; xml.get("id",          &m_id         );
        };   // AddonsProp(const XML&)
    };

    private:
        std::vector<AddonsProp>     m_addons_list;
        int                         m_index;
        std::string                 m_file_installed;
        void                        saveInstalled();
        void                        getInstalledAddons();
        std::string                 m_type;
        int                         m_download_state;
        std::string                 m_str_state;

        /** Which state the addons manager is:
         *  INIT:  Waiting to download the list of addons.
         *  READY: List is downloaded, manager is ready.
         *  ERROR: Error downloading the list, no addons available. */
        enum  STATE_TYPE {STATE_INIT, STATE_READY, STATE_ERROR};
        // Synchronise the state between threads (e.g. GUI and update thread)
        Synchronised<STATE_TYPE> m_state;

    public:
             AddonsManager();

        void initOnline();
        bool onlineReady();
        /** Select the next addons in the addons list. */
        bool next();
        /** Select the next addons in the addons list. */
        bool previous();

        /** Get all the selected addon parameters. */
        const AddonsProp &getAddons() const;

        /** Select an addon with it name. */
        bool select(std::string);

        /** Select an addon with it id. */
        bool selectId(std::string);

        /** Get the name of the selected addon. */
        const std::string &getName() const 
                                { return m_addons_list[m_index].m_name; };

        /** Get the version of the selected addon. */
        int getVersion() const { return m_addons_list[m_index].m_version; };

        /** Get the path of the addon icon. */
        const std::string &getIcon() const
                               { return m_addons_list[m_index].m_icon; };

        /** Get the version of the selected addon as a string. */
        std::string getVersionAsStr() const;

        /** Get the installed version of the selected addon. */
        int getInstalledVersion() const;
        std::string getInstalledVersionAsStr() const;

        /** Return a simple bool to know if the addon needs to be updated */
        bool needUpdate() const;

        /** Get the installed version of the selected addon. */
        std::string getIdAsStr() const;

        /** Get the description of the selected addons. */
        const std::string &getDescription() const 
                                { return m_addons_list[m_index].m_description; };

        const std::string &getType() const 
                                { return m_addons_list[m_index].m_type; };
        /** Install or upgrade the selected addon. */
        void install();

        /** Uninstall the selected addon. This method will remove all the 
         *  directory of the addon.*/
        void uninstall();

        void resetIndex();

        /** Get the state of the addon: if it is installed or not.*/
        bool isInstalled() const 
                            { return m_addons_list[m_index].m_installed; };

        bool nextType(std::string type);
        bool previousType(std::string type);
        int  getDownloadState();

        /** Get the install state (if it is the download, unzip...)*/
        const std::string& getDownloadStateAsStr() const;

};
extern AddonsManager *addons_manager;
#endif
#endif
