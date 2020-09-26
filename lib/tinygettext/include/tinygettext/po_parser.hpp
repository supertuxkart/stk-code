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

#ifndef HEADER_TINYGETTEXT_PO_PARSER_HPP
#define HEADER_TINYGETTEXT_PO_PARSER_HPP

#include <iosfwd>

#include "iconv.hpp"

namespace tinygettext {

class Dictionary;

class POParser
{
private:
  std::string filename;
  std::istream& in;
  Dictionary& dict;
  bool  use_fuzzy;

  bool running;
  bool eof;
  bool big5;

  int line_number;
  std::string current_line;

  IConv conv;

  POParser(const std::string& filename, std::istream& in_, Dictionary& dict_, bool use_fuzzy = true);
  ~POParser();

  void parse_header(const std::string& header);
  void parse();
  void next_line();
  std::string get_string(unsigned int skip);
  void get_string_line(std::ostringstream& str, size_t skip);
  bool is_empty_line();
  bool prefix(const char* );
#ifdef _WIN32
  void error(const std::string& msg);
#else
  void error(const std::string& msg) __attribute__((__noreturn__));
#endif
  void warning(const std::string& msg);

public:
  /** @param filename name of the istream, only used in error messages
      @param in stream from which the PO file is read.
      @param dict dictionary to which the strings are written */
  static void parse(const std::string& filename, std::istream& in, Dictionary& dict);
  static bool pedantic;

private:
  POParser (const POParser&);
  POParser& operator= (const POParser&);
};

} // namespace tinygettext

#endif

/* EOF */
