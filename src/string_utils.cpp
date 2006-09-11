//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>,
//                     Ingo Ruhnke <grumbel@gmx.de>
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

#include <algorithm>
#include "string_utils.hpp"

namespace StringUtils {

bool has_suffix(const std::string& lhs, const std::string rhs)
{
  if (lhs.length() < rhs.length())
    return false;
  else
    // While this is basically correct, it fails with older 
    // g++ versions (at least 2.95.3), which have a wrong template. To 
    // avoid this issue, a more C-traditional way is used.
    return strcmp(lhs.c_str()+(lhs.length()-rhs.length()), rhs.c_str())==0;
}

std::string basename(const std::string& filename)
{
  for(int i = int(filename.size()) - 1; i >= 0; --i)
    {
      if (filename[i] == '/') {
	return filename.substr(i+1);
      }
    }
  return filename;
}

std::string without_extension(const std::string& filename)
{
  for(int i = int(filename.size()) - 1; i >= 0; --i)
    {
      if (filename[i] == '.') {
	return filename.substr(0, i);
      }
    }
  return filename;  
}

std::string extension(const std::string& filename)
{
  for(int i = int(filename.size()) - 1; i >= 0; --i)
    {
      if (filename[i] == '.') {
	return filename.substr(i+1);
      }
    }
  return filename;  
}

std::string upcase (const std::string& str)
{
  std::string name = str;
  std::transform(name.begin(), name.end(), name.begin(), ::toupper);
  return name;
}

std::string downcase (const std::string& str)
{
  std::string name = str;
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  return name;
}

} // namespace StringUtils

/* EOF */
