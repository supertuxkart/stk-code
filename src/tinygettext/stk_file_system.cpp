//  tinygettext - A gettext replacement that works directly on .po files
//  Copyright (C) 2009-2015 Ingo Ruhnke <grumbel@gmx.de>
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
#include <stdlib.h>
#include <string.h>

#include "io/file_manager.hpp"

namespace tinygettext {

StkFileSystem::StkFileSystem()
{
}

std::vector<std::string>
StkFileSystem::open_directory(const std::string& pathname)
{
  std::set<std::string> result;

  file_manager->listFiles(result, pathname);
  std::vector<std::string> files;
  for(std::set<std::string>::iterator i=result.begin(); i!=result.end(); i++)
  {
    files.push_back(*i);
  }
  return files;
}

std::unique_ptr<std::istream>
StkFileSystem::open_file(const std::string& filename)
{
  return std::unique_ptr<std::istream>(new std::ifstream(filename.c_str()));
}

} // namespace tinygettext

/* EOF */
