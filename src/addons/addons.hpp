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

#include <string.h>
#include <map>
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
        int                         index;
        std::string                 file_installed;
        void                        SaveInstalled();
        void                        GetInstalledAddons();
        std::string                 type;
        int                         m_download_state;
        pthread_mutex_t             m_str_mutex;
        std::string m_str_state;
    public:
        Addons();

        /** Select the next addons in the addons list. */
        bool Next();
        /** Select the next addons in the addons list. */
        bool Previous();

        /** Get all the selected addon parameters. */
        addons_prop GetAddons();

        /** Select an addon with it name. */
        bool Select(std::string);

        /** Select an addon with it id. */
        bool SelectId(std::string);

        /** Get the name of the selected addon. */
        std::string GetName(){ return this->m_addons_list[this->index].name; };

        /** Get the version of the selected addon. */
        int GetVersion(){ return this->m_addons_list[this->index].version; };

        /** Get the path of the addon icon. */
        std::string GetIcon() { return this->m_addons_list[this->index].icon; };

        /** Get the version of the selected addon as a string. */
        std::string GetVersionAsStr();

        /** Get the installed version of the selected addon. */
        int GetInstalledVersion();
        std::string GetInstalledVersionAsStr();

        /** Get the installed version of the selected addon. */
        std::string GetIdAsStr();

        /** Get the description of the selected addons. */
        std::string GetDescription(){ return this->m_addons_list[this->index].description; };

        std::string GetType(){ return this->m_addons_list[this->index].type; };
        /** Install or upgrade the selected addon. */
        void Install();

        /** Uninstall the selected addon. This method will remove all the directory of the addon.*/
        void UnInstall();

        void resetIndex();

        /** Get the state of the addon: if it is installed or not.*/
        std::string IsInstalled();

        /** Get the state of the addon: if it is installed or not.*/
        bool IsInstalledAsBool(){ return this->m_addons_list[this->index].installed; };

        bool NextType(std::string type);
        bool PreviousType(std::string type);
        int getDownloadState();

        /** Get the install state (if it is the download, unzip...)*/
        std::string getDownloadStateAsStr();

};
extern Addons * addons_manager;
#endif
#endif
