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

#ifndef HEADER_TINYGETTEXT_LOG_STREAM_HPP
#define HEADER_TINYGETTEXT_LOG_STREAM_HPP

#include "log.hpp"

namespace tinygettext {

// FIXME: very bad to have such things in the API
#define log_error   if (!Log::log_error_callback);   else (Log(Log::log_error_callback)).get()
#define log_warning if (!Log::log_warning_callback); else (Log(Log::log_warning_callback)).get()
#define log_info    if (!Log::log_info_callback);    else (Log(Log::log_warning_callback)).get()

} // namespace tinygettext

#endif

/* EOF */
