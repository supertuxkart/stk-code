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

#include "po_parser.hpp"

#include <iostream>
#include <ctype.h>
#include <string>
#include <istream>
#include <string.h>
#include <sstream>
#include <map>
#include <stdlib.h>

#include "language.hpp"
#include "dictionary.hpp"
#include "plural_forms.hpp"

#include "utils/log.hpp"

namespace tinygettext {

bool POParser::pedantic = true;

void
POParser::parse(const std::string& filename, std::istream& in, Dictionary& dict)
{
  POParser parser(filename, in, dict);
  parser.parse();
}

class POParserError {};

POParser::POParser(const std::string& filename_, std::istream& in_, Dictionary& dict_, bool use_fuzzy_) :
  filename(filename_),
  in(in_),
  dict(dict_),
  use_fuzzy(use_fuzzy_),
  running(false),
  eof(false),
  big5(false),
  line_number(0),
  current_line()//,
  //conv()
{
}

POParser::~POParser()
{
}

void
POParser::warning(const std::string& msg)
{
    Log::warn("tinygettext", "%s line %d %s: \"%s\"",
        filename.c_str(), line_number, msg.c_str(), current_line.c_str());
}

void
POParser::error(const std::string& msg)
{
    Log::error("tinygettext", "%s line %d %s: \"%s\"",
        filename.c_str(), line_number, msg.c_str(), current_line.c_str());

  // Try to recover from an error by searching for start of another entry
  do
    next_line();
  while(!eof && !is_empty_line());

  throw POParserError();
}

void
POParser::next_line()
{
  line_number += 1;
  if (!std::getline(in, current_line))
    eof = true;
}

void
POParser::get_string_line(std::ostringstream& out,unsigned int skip)
{
  if (skip+1 >= static_cast<unsigned int>(current_line.size()))
    error("unexpected end of line");

  if (current_line[skip] != '"')
    error("expected start of string '\"'");

  std::string::size_type i;
  for(i = skip+1; current_line[i] != '\"'; ++i)
  {
    if (big5 && static_cast<unsigned char>(current_line[i]) >= 0x81 && static_cast<unsigned char>(current_line[i]) <= 0xfe)
    {
      out << current_line[i];

      i += 1;

      if (i >= current_line.size())
        error("invalid big5 encoding");

      out << current_line[i];
    }
    else if (i >= current_line.size())
    {
      error("unexpected end of string");
    }
    else if (current_line[i] == '\\')
    {
      i += 1;

      if (i >= current_line.size())
        error("unexpected end of string in handling '\\'");

      switch (current_line[i])
      {
        case 'a':  out << '\a'; break;
        case 'b':  out << '\b'; break;
        case 'v':  out << '\v'; break;
        case 'n':  out << '\n'; break;
        case 't':  out << '\t'; break;
        case 'r':  out << '\r'; break;
        case '"':  out << '"'; break;
        case '\\': out << '\\'; break;
        default:
          std::ostringstream err;
          err << "unhandled escape '\\" << current_line[i] << "'";
          warning(err.str());

          out << current_line[i-1] << current_line[i];
          break;
      }
    }
    else
    {
      out << current_line[i];
    }
  }

  // process trailing garbage in line and warn if there is any
  for(i = i+1; i < current_line.size(); ++i)
    if (!isspace(current_line[i]))
    {
      warning("unexpected garbage after string ignoren");
      break;
    }
}

std::string
POParser::get_string(unsigned int skip)
{
  std::ostringstream out;

  if (skip+1 >= static_cast<unsigned int>(current_line.size()))
    error("unexpected end of line");

  if (current_line[skip] == ' ' && current_line[skip+1] == '"')
  {
    get_string_line(out, skip+1);
  }
  else
  {
    if (pedantic)
      warning("keyword and string must be separated by a single space");

    for(;;)
    {
      if (skip >= static_cast<unsigned int>(current_line.size()))
        error("unexpected end of line");
      else if (current_line[skip] == '\"')
      {
        get_string_line(out, skip);
        break;
      }
      else if (!isspace(current_line[skip]))
      {
        error("string must start with '\"'");
      }
      else
      {
        // skip space
      }

      skip += 1;
    }
  }

next:
  next_line();
  for(std::string::size_type i = 0; i < current_line.size(); ++i)
  {
    if (current_line[i] == '"')
    {
      if (i == 1)
        if (pedantic)
          warning("leading whitespace before string");

      get_string_line(out, (unsigned int) i);
      goto next;
    }
    else if (isspace(current_line[i]))
    {
      // skip
    }
    else
    {
      break;
    }
  }

  return out.str();
}

static bool has_prefix(const std::string& lhs, const std::string &rhs)
{
  if (lhs.length() < rhs.length())
    return false;
  else
    return lhs.compare(0, rhs.length(), rhs) == 0;
}

void
POParser::parse_header(const std::string& header)
{
  std::string from_charset;
  std::string::size_type start = 0;
  for(std::string::size_type i = 0; i < header.length(); ++i)
  {
    if (header[i] == '\n')
    {
      std::string line = header.substr(start, i - start);

      if (has_prefix(line, "Content-Type:"))
      {
        // from_charset = line.substr(len);
        unsigned int len = (unsigned int) strlen("Content-Type: text/plain; charset=");
        if (line.compare(0, len, "Content-Type: text/plain; charset=") == 0)
        {
          from_charset = line.substr(len);

          for(std::string::iterator ch = from_charset.begin(); ch != from_charset.end(); ++ch)
            *ch = static_cast<char>(toupper(*ch));
        }
        else
        {
          warning("malformed Content-Type header");
        }
      }
      else if (has_prefix(line, "Plural-Forms:"))
      {
        PluralForms plural_forms = PluralForms::from_string(line);
        if (!plural_forms)
        {
          warning("unknown Plural-Forms given");
        }
        else
        {
          if (!dict.get_plural_forms())
          {
            dict.set_plural_forms(plural_forms);
          }
          else
          {
            if (dict.get_plural_forms() != plural_forms)
            {
              warning("Plural-Forms missmatch between .po file and dictionary");
            }
          }
        }
      }
      start = i+1;
    }
  }

  if (from_charset.empty() || from_charset == "CHARSET")
  {
    warning("charset not specified for .po, fallback to utf-8");
    from_charset = "UTF-8";
  }
  else if (from_charset == "BIG5")
  {
    big5 = true;
  }

  //conv.set_charsets(from_charset, dict.get_charset());
}

bool
POParser::is_empty_line()
{
  if (current_line.empty())
  {
    return true;
  }
  else if (current_line[0] == '#')
  { // handle comments as empty lines
    if (current_line.size() == 1 || (current_line.size() >= 2 && isspace(current_line[1])))
      return true;
    else
      return false;
  }
  else
  {
    for(std::string::iterator i = current_line.begin(); i != current_line.end(); ++i)
    {
      if (!isspace(*i))
        return false;
    }
  }
  return true;
}

bool
POParser::prefix(const char* prefix_str)
{
  return current_line.compare(0, strlen(prefix_str), prefix_str) == 0;
}

void
POParser::parse()
{
  next_line();

  // skip UTF-8 intro that some text editors produce
  // see http://en.wikipedia.org/wiki/Byte-order_mark
  if (current_line.size() >= 3 &&
      current_line[0] == static_cast<unsigned char>(0xef) &&
      current_line[1] == static_cast<unsigned char>(0xbb) &&
      current_line[2] == static_cast<unsigned char>(0xbf))
  {
    current_line = current_line.substr(3);
  }

  // Parser structure
  while(!eof)
  {
    try
    {
      bool fuzzy =  false;
      bool has_msgctxt = false;
      std::string msgctxt;
      std::string msgid;

      while(prefix("#"))
      {
        if (current_line.size() >= 2 && current_line[1] == ',')
        {
          // FIXME: Rather simplistic hunt for fuzzy flag
          if (current_line.find("fuzzy", 2) != std::string::npos)
            fuzzy = true;
        }

        next_line();
      }

      if (!is_empty_line())
      {
        if (prefix("msgctxt"))
        {
          has_msgctxt = true;
          msgctxt = get_string(7);
        }

        if (prefix("msgid"))
          msgid = get_string(5);
        else
          error("expected 'msgid'");

        if (prefix("msgid_plural"))
        {
          std::string msgid_plural = get_string(12);
          std::vector<std::string> msgstr_num;
          bool saw_nonempty_msgstr = false;

        next:
          if (is_empty_line())
          {
            if (msgstr_num.empty())
              error("expected 'msgstr[N] (0 <= N <= 9)'");
          }
          else if (prefix("msgstr[") &&
                   current_line.size() > 8 &&
                   isdigit(current_line[7]) && current_line[8] == ']')
          {
            unsigned int number = static_cast<unsigned int>(current_line[7] - '0');
            std::string msgstr = get_string(9);

            if(!msgstr.empty())
              saw_nonempty_msgstr = true;

            if (number >= msgstr_num.size())
              msgstr_num.resize(number+1);

            msgstr_num[number] = msgstr; //conv.convert(msgstr);
            goto next;
          }
          else
          {
            error("expected 'msgstr[N]'");
          }

          if (!is_empty_line())
            error("expected 'msgstr[N]' or empty line");

          if (saw_nonempty_msgstr)
          {
            if (use_fuzzy || !fuzzy)
            {
              if (!dict.get_plural_forms())
              {
                warning("msgstr[N] seen, but no Plural-Forms given");
              }
              else
              {
                if (msgstr_num.size() != dict.get_plural_forms().get_nplural())
                {
                  warning("msgstr[N] count doesn't match Plural-Forms.nplural");
                }
              }

              if (has_msgctxt)
                dict.add_translation(msgctxt, msgid, msgid_plural, msgstr_num);
              else
                dict.add_translation(msgid, msgid_plural, msgstr_num);
            }

            if (0)
            {
              std::cout << (fuzzy?"fuzzy":"not-fuzzy") << std::endl;
              std::cout << "msgid \"" << msgid << "\"" << std::endl;
              std::cout << "msgid_plural \"" << msgid_plural << "\"" << std::endl;
              for(std::vector<std::string>::size_type i = 0; i < msgstr_num.size(); ++i)
                std::cout << "msgstr[" << i << "] \"" << msgstr_num[i] /*conv.convert(msgstr_num[i])*/ << "\"" << std::endl;
              std::cout << std::endl;
            }
          }
        }
        else if (prefix("msgstr"))
        {
          std::string msgstr = get_string(6);

          if (msgid.empty())
          {
            parse_header(msgstr);
          }
          else if(!msgstr.empty())
          {
            if (use_fuzzy || !fuzzy)
            {
              if (has_msgctxt)
                dict.add_translation(msgctxt, msgid, msgstr /*conv.convert(msgstr)*/);
              else
                dict.add_translation(msgid, msgstr /*conv.convert(msgstr)*/);
            }

            if (0)
            {
              std::cout << (fuzzy?"fuzzy":"not-fuzzy") << std::endl;
              std::cout << "msgid \"" << msgid << "\"" << std::endl;
              std::cout << "msgstr \"" << msgstr /*conv.convert(msgstr)*/ << "\"" << std::endl;
              std::cout << std::endl;
            }
          }
        }
        else
        {
          error("expected 'msgstr' or 'msgid_plural'");
        }
      }

      if (!is_empty_line())
        error("expected empty line");

      next_line();
    }
    catch(POParserError&)
    {
    }
  }
}

} // namespace tinygettext

/* EOF */
