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
