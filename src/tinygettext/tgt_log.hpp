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

#ifndef HEADER_TINYGETTEXT_LOG_HPP
#define HEADER_TINYGETTEXT_LOG_HPP

#include <sstream>

namespace tinygettext {

class Log
{
public:
  typedef void (*log_callback_t)(const std::string&);

  static log_callback_t log_info_callback;
  static log_callback_t log_warning_callback;
  static log_callback_t log_error_callback;


  static void default_log_callback(const std::string& str);

  static void set_log_info_callback(log_callback_t callback);
  static void set_log_warning_callback(log_callback_t callback);
  static void set_log_error_callback(log_callback_t callback);

private:
  log_callback_t callback;
  std::ostringstream out;

public:
  Log(log_callback_t callback);
  ~Log();
  
  std::ostream& get();
};

} // namespace tinygettext

#endif

/* EOF */
