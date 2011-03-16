//  tinygettext - A gettext replacement that works directly on .po files
//  Copyright (C) 2009 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#include "stk_file_system.hpp"

#include <sys/types.h>
#include <fstream>
#include <dirent.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

namespace tinygettext {

StkFileSystem::StkFileSystem()
{
}

std::vector<std::string>
StkFileSystem::open_directory(const std::string& pathname)
{
  DIR* dir = opendir(pathname.c_str());
  if (!dir)
  {
    // FIXME: error handling
    return std::vector<std::string>();
  }
  else
  {
    std::vector<std::string> files;

    struct dirent* dp;
    while((dp = readdir(dir)) != 0)
    {
      files.push_back(dp->d_name);
    }
    closedir(dir);

    return files;
  }
}
  
std::auto_ptr<std::istream>
StkFileSystem::open_file(const std::string& filename)
{
  return std::auto_ptr<std::istream>(new std::ifstream(filename.c_str()));
}

} // namespace tinygettext

/* EOF */
