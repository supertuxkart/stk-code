// tinygettext - A gettext replacement that works directly on .po files
// Copyright (c) 2006 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_TINYGETTEXT_ICONV_HPP
#define HEADER_TINYGETTEXT_ICONV_HPP

#include <string>

#ifdef HAVE_SDL
#  include "SDL.h"

#  define tinygettext_ICONV_CONST const
#  define tinygettext_iconv_t     SDL_iconv_t
#  define tinygettext_iconv       SDL_iconv
#  define tinygettext_iconv_open  SDL_iconv_open
#  define tinygettext_iconv_close SDL_iconv_close
#else
#  include <iconv.h>

#  ifdef HAVE_ICONV_CONST
#    define tinygettext_ICONV_CONST ICONV_CONST
#  else
#    define tinygettext_ICONV_CONST
#  endif

#  define tinygettext_iconv_t     iconv_t
#  define tinygettext_iconv       iconv
#  define tinygettext_iconv_open  iconv_open
#  define tinygettext_iconv_close iconv_close
#endif

namespace tinygettext {

class IConv
{
private:
  std::string to_charset;
  std::string from_charset;
  tinygettext_iconv_t cd;

public:
  IConv();
  IConv(const std::string& fromcode, const std::string& tocode);
  ~IConv();

  void set_charsets(const std::string& fromcode, const std::string& tocode);
  std::string convert(const std::string& text);

private:
  IConv (const IConv&);
  IConv& operator= (const IConv&);
};

} // namespace tinygettext

#endif

/* EOF */
