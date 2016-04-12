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

#ifndef HEADER_TINYGETTEXT_PO_PARSER_HPP
#define HEADER_TINYGETTEXT_PO_PARSER_HPP

#include <iosfwd>
#include <string>

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

  //IConv conv;

  POParser(const std::string& filename, std::istream& in_, Dictionary& dict_, bool use_fuzzy = true);
  ~POParser();

  void parse_header(const std::string& header);
  void parse();
  void next_line();
  std::string get_string(unsigned int skip);
  void get_string_line(std::ostringstream& str,unsigned int skip);
  bool is_empty_line();
  bool prefix(const char* );
#ifdef WIN32
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
