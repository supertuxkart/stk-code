// tinygettext - A gettext replacement that works directly on .po files
// Copyright (c) 2009 Ingo Ruhnke <grumbel@gmail.com>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgement in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include <iostream>
#include "tinygettext/log.hpp"

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
