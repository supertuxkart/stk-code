//  tinygettext - A gettext replacement that works directly on .po files
//  Copyright (C) 2006-2013 Ingo Ruhnke <grumbel@gmx.de>
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

#include "dictionary_manager.hpp"

#include <memory>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <algorithm>

#include "log_stream.hpp"
#include "po_parser.hpp"
#include "stk_file_system.hpp"

namespace tinygettext {

static bool has_suffix(const std::string& lhs, const std::string rhs)
{
  if (lhs.length() < rhs.length())
    return false;
  else
    return lhs.compare(lhs.length() - rhs.length(), rhs.length(), rhs) == 0;
}

DictionaryManager::DictionaryManager(const std::string& charset_) :
  dictionaries(),
  search_path(),
  charset(charset_),
  use_fuzzy(true),
  current_language(),
  current_dict(0),
  empty_dict(),
  filesystem(new StkFileSystem)
{
#ifdef DEBUG
    m_magic_number = 0xD1C70471;
#endif
}

DictionaryManager::~DictionaryManager()
{
#ifdef DEBUG
    assert(m_magic_number == 0xD1C70471);
    m_magic_number = 0xDEADBEEF;
#endif
  for(Dictionaries::iterator i = dictionaries.begin(); i != dictionaries.end(); ++i)
  {
    delete i->second;
  }
}

void
DictionaryManager::clear_cache()
{
  for(Dictionaries::iterator i = dictionaries.begin(); i != dictionaries.end(); ++i)
  {
    delete i->second;
  }
  dictionaries.clear();

  current_dict = 0;
}

Dictionary&
DictionaryManager::get_dictionary()
{
  if (current_dict)
  {
    return *current_dict;
  }
  else
  {
    if (current_language)
    {
      current_dict = &get_dictionary(current_language);
      return *current_dict;
    }
    else
    {
      return empty_dict;
    }
  }
}

Dictionary&
DictionaryManager::get_dictionary(const Language& language)
{
  //log_debug << "Dictionary for language \"" << spec << "\" requested" << std::endl;
  //log_debug << "...normalized as \"" << lang << "\"" << std::endl;
  assert(language);

  Dictionaries::iterator i = dictionaries.find(language);
  if (i != dictionaries.end())
  {
    return *i->second;
  }
  else // Dictionary for languages lang isn't loaded, so we load it
  {
    //log_debug << "get_dictionary: " << lang << std::endl;
    Dictionary* dict = new Dictionary(charset);

    dictionaries[language] = dict;

    for (SearchPath::reverse_iterator p = search_path.rbegin(); p != search_path.rend(); ++p)
    {
      std::vector<std::string> files = filesystem->open_directory(*p);

      std::string best_filename;
      int best_score = 0;

      for(std::vector<std::string>::iterator filename = files.begin(); filename != files.end(); filename++)
      {
        // check if filename matches requested language
        if (has_suffix(*filename, ".po"))
        { // ignore anything that isn't a .po file

            Language po_language = Language::from_env(convertFilename2Language(*filename));

          if (!po_language)
          {
            log_warning << *filename << ": warning: ignoring, unknown language" << std::endl;
          }
          else
          {
            int score = Language::match(language, po_language);

            if (score > best_score)
            {
              best_score = score;
              best_filename = *filename;
            }
          }
        }
      }

      if (!best_filename.empty())
      {
        std::string pofile = *p + "/" + best_filename;
        try
        {
          std::auto_ptr<std::istream> in = filesystem->open_file(pofile);
          if (!in.get())
          {
            log_error << "error: failure opening: " << pofile << std::endl;
          }
          else
          {
            POParser::parse(pofile, *in, *dict);
          }
        }
        catch(std::exception& e)
        {
          log_error << "error: failure parsing: " << pofile << std::endl;
          log_error << e.what() << "" << std::endl;
        }
      }
    }

    if (language.get_country().size() > 0)
    {
        printf("Adding language fallback %s\n", language.get_language().c_str());
        dict->addFallback( &get_dictionary(Language::from_spec(language.get_language())) );
    }
    return *dict;
  }
}

std::set<Language>
DictionaryManager::get_languages()
{
  std::set<Language> languages;

  for (SearchPath::iterator p = search_path.begin(); p != search_path.end(); ++p)
  {
    std::vector<std::string> files = filesystem->open_directory(*p);

    for(std::vector<std::string>::iterator file = files.begin(); file != files.end(); ++file)
    {
      if (has_suffix(*file, ".po"))
      {
        languages.insert(Language::from_env(file->substr(0, file->size()-3)));
      }
    }
  }
  return languages;
}

void
DictionaryManager::set_language(const Language& language)
{
  if (current_language != language)
  {
    current_language = language;
    current_dict     = 0;
  }
}

Language
DictionaryManager::get_language() const
{
  return current_language;
}

void
DictionaryManager::set_charset(const std::string& charset_)
{
  clear_cache(); // changing charset invalidates cache
  charset = charset_;
}

void
DictionaryManager::set_use_fuzzy(bool t)
{
  clear_cache();
  use_fuzzy = t;
}

bool
DictionaryManager::get_use_fuzzy() const
{
  return use_fuzzy;
}

void
DictionaryManager::add_directory(const std::string& pathname)
{
  clear_cache(); // adding directories invalidates cache
  search_path.push_back(pathname);
}

/*void
DictionaryManager::set_filesystem(std::auto_ptr<FileSystem> filesystem_)
{
  filesystem = filesystem_;
}*/
// ----------------------------------------------------------------------------
/** This function converts a .po filename (e.g. zh_TW.po) into a language
 *  specification (zh_TW). On case insensitive file systems (think windows)
 *  the filename and therefore the country specification is lower case
 *  (zh_tw). It Converts the lower case characters of the country back to
 *  upper case, otherwise tinygettext does not identify the country
 *  correctly.
 */
std::string DictionaryManager::convertFilename2Language(const std::string &s_in) const
{
    std::string s;
    if(s_in.substr(s_in.size()-3, 3)==".po")
        s = s_in.substr(0, s_in.size()-3);
    else
        s = s_in;

    bool underscore_found = false;
    for(unsigned int i=0; i<s.size(); i++)
    {
        if(underscore_found)
        {
            // If we get a non-alphanumerical character/
            // we are done (en_GB.UTF-8) - only convert
            // the 'gb' part ... if we ever get this kind
            // of filename.
            if(!::isalpha(s[i]))
                break;
            s[i]=::toupper(s[i]);
        }
        else
            underscore_found = s[i]=='_';
    }
    return s;
}   // convertFilename2Language


} // namespace tinygettext


/* EOF */
