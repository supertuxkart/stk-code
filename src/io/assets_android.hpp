//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#ifndef HEADER_ASSETS_ANDROID_HPP
#define HEADER_ASSETS_ANDROID_HPP

#include <string>

class FileManager;

class AssetsAndroid
{
private:
    FileManager* m_file_manager;
    std::string m_stk_dir;

    void extractData();
    bool extractDir(std::string dir_name);
    void removeData();
    void touchFile(std::string path);
    std::string getPreferredPath(const std::vector<std::string>& paths);

public:
    AssetsAndroid(FileManager* file_manager);
    ~AssetsAndroid() {};

    void init();
};


#endif
