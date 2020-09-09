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

#include <ctype.h>
#include <assert.h>
#include <sstream>
#include <errno.h>
#include <stdexcept>
#include <string.h>
#include <stdlib.h>

#include "tinygettext/iconv.hpp"
#include "tinygettext/log_stream.hpp"

namespace tinygettext {

#ifndef tinygettext_ICONV_CONST
#  define tinygettext_ICONV_CONST
#endif

IConv::IConv()
  : to_charset(),
    from_charset(),
    cd(0)
{}

IConv::IConv(const std::string& from_charset_, const std::string& to_charset_)
  : to_charset(),
    from_charset(),
    cd(0)
{
  set_charsets(from_charset_, to_charset_);
}

IConv::~IConv()
{
  if (cd)
    tinygettext_iconv_close(cd);
}

void
IConv::set_charsets(const std::string& from_charset_, const std::string& to_charset_)
{
  if (cd)
    tinygettext_iconv_close(cd);

  from_charset = from_charset_;
  to_charset   = to_charset_;

  for(std::string::iterator i = to_charset.begin(); i != to_charset.end(); ++i)
    *i = static_cast<char>(toupper(*i));

  for(std::string::iterator i = from_charset.begin(); i != from_charset.end(); ++i)
    *i = static_cast<char>(toupper(*i));

  if (to_charset == from_charset)
  {
    cd = 0;
  }
  else
  {
    cd = tinygettext_iconv_open(to_charset.c_str(), from_charset.c_str());
    if (cd == reinterpret_cast<tinygettext_iconv_t>(-1))
    {
      if(errno == EINVAL)
      {
        std::ostringstream str;
        str << "IConv construction failed: conversion from '" << from_charset
            << "' to '" << to_charset << "' not available";
        throw std::runtime_error(str.str());
      }
      else
      {
        std::ostringstream str;
        str << "IConv: construction failed: " << strerror(errno);
        throw std::runtime_error(str.str());
      }
    }
  }
}

/// Convert a string from encoding to another.
std::string
IConv::convert(const std::string& text)
{
  if (!cd)
  {
    return text;
  }
  else
  {
    size_t inbytesleft  = text.size();
    size_t outbytesleft = 4*inbytesleft; // Worst case scenario: ASCII -> UTF-32?

    // We try to avoid to much copying around, so we write directly into
    // a std::string
    tinygettext_ICONV_CONST char* inbuf = const_cast<char*>(&text[0]);
    std::string result(outbytesleft, 'X');
    char* outbuf = &result[0];

    // Try to convert the text.
    size_t ret = tinygettext_iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    if (ret == static_cast<size_t>(-1))
    {
      if (errno == EILSEQ || errno == EINVAL)
      { // invalid multibyte sequence
        tinygettext_iconv(cd, NULL, NULL, NULL, NULL); // reset state

        // FIXME: Could try to skip the invalid byte and continue
        log_error << "error: tinygettext:iconv: invalid multibyte sequence in:  \"" << text << "\"" << std::endl;
      }
      else if (errno == E2BIG)
      { // output buffer to small
        assert(false && "tinygettext/iconv.cpp: E2BIG: This should never be reached");
      }
      else if (errno == EBADF)
      {
        assert(false && "tinygettext/iconv.cpp: EBADF: This should never be reached");
      }
      else
      {
        assert(false && "tinygettext/iconv.cpp: <unknown>: This should never be reached");
      }
    }

    result.resize(4*text.size() - outbytesleft);

    return result;
  }
}

} // namespace tinygettext

/* EOF */
