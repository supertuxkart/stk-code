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

#ifdef ADDONS_MANAGER

#ifndef HEADER_ADDONS_HPP
#define HEADER_ADDONS_HPP

#include <string>
#include <map>
#include <pthread.h>
#include <vector>

struct addons_prop
{
    std::string name;
    int version;
    int installed_version;
    std::string description;
    std::string icon;
    std::string file;
    std::string id;
    bool installed;
    std::string type;
};
class Addons
{
    private:
        std::vector<addons_prop>    m_addons_list;
        int                         m_index;
        std::string                 m_file_installed;
        void                        saveInstalled();
        void                        getInstalledAddons();
        std::string                 m_type;
        int                         m_download_state;
        mutable pthread_mutex_t     m_str_mutex;
        std::string m_str_state;
    public:
        Addons();

        /** Select the next addons in the addons list. */
        bool next();
        /** Select the next addons in the addons list. */
        bool previous();

        /** Get all the selected addon parameters. */
        addons_prop getAddons();

        /** Select an addon with it name. */
        bool select(std::string);

        /** Select an addon with it id. */
        bool selectId(std::string);

        /** Get the name of the selected addon. */
        const std::string &getName() const 
                                { return m_addons_list[m_index].name; };

        /** Get the version of the selected addon. */
        int getVersion() const { return m_addons_list[m_index].version; };

        /** Get the path of the addon icon. */
        const std::string &getIcon() const
                               { return m_addons_list[m_index].icon; };

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
                                { return m_addons_list[m_index].description; };

        const std::string &getType() const 
                                { return m_addons_list[m_index].type; };
        /** Install or upgrade the selected addon. */
        void install();

        /** Uninstall the selected addon. This method will remove all the 
         *  directory of the addon.*/
        void uninstall();

        void resetIndex();

        /** Get the state of the addon: if it is installed or not.*/
        bool isInstalledAsBool() const 
                            { return m_addons_list[m_index].installed; };

        bool nextType(std::string type);
        bool previousType(std::string type);
        int  getDownloadState();

        /** Get the install state (if it is the download, unzip...)*/
        std::string getDownloadStateAsStr() const;

};
extern Addons *addons_manager;
#endif
#endif
