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

#include <iostream>
#include "log.hpp"

namespace tinygettext {

Log::log_callback_t Log::log_info_callback    = &Log::default_log_callback;
Log::log_callback_t Log::log_warning_callback = &Log::default_log_callback;
Log::log_callback_t Log::log_error_callback   = &Log::default_log_callback;

void
Log::default_log_callback(const std::string& str)
{
  std::cerr << "tinygettext: " << str;
}

void
Log::set_log_info_callback(log_callback_t callback)
{
  log_info_callback = callback;
}

void
Log::set_log_warning_callback(log_callback_t callback)
{
  log_warning_callback = callback;
}

void
Log::set_log_error_callback(log_callback_t callback)
{
  log_error_callback = callback;
}

Log::Log(log_callback_t callback_) :
  callback(callback_),
  out()
{
}

Log::~Log() 
{
  callback(out.str());
}

std::ostream&
Log::get() 
{
  return out; 
}

} // namespace tinygettext

/* EOF */
