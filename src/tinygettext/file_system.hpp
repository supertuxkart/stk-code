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

#ifndef HEADER_TINYGETTEXT_FILE_SYSTEM_HPP
#define HEADER_TINYGETTEXT_FILE_SYSTEM_HPP

#include <vector>
#include <memory>
#include <iosfwd>
#include <string>

namespace tinygettext {

class FileSystem
{
public:
  virtual ~FileSystem() {}

  virtual std::vector<std::string>    open_directory(const std::string& pathname) =0;
  virtual std::unique_ptr<std::istream> open_file(const std::string& filename)      =0;
};

} // namespace tinygettext

#endif

/* EOF */

