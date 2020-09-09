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

#include "tinygettext/dictionary_manager.hpp"

#include <memory>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <algorithm>

#include "tinygettext/log_stream.hpp"
#include "tinygettext/po_parser.hpp"
#include "tinygettext/unix_file_system.hpp"

namespace tinygettext {

static bool has_suffix(const std::string& lhs, const std::string& rhs)
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
  filesystem(new UnixFileSystem)
{
}

DictionaryManager::~DictionaryManager()
{
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

      for (std::vector<std::string>::iterator filename = files.begin(); filename != files.end(); ++filename)
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
          std::unique_ptr<std::istream> in = filesystem->open_file(pofile);
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

    if (!language.get_country().empty())
    {
        // printf("Adding language fallback %s\n", language.get_language().c_str());
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
DictionaryManager::add_directory(const std::string& pathname, bool precedence /* = false */)
{
  clear_cache(); // adding directories invalidates cache
  if (precedence)
    search_path.push_front(pathname);
  else
    search_path.push_back(pathname);
}

void
DictionaryManager::remove_directory(const std::string& pathname)
{
  SearchPath::iterator it = std::find(search_path.begin(), search_path.end(), pathname);
  if (it != search_path.end())
  {
    clear_cache(); // removing directories invalidates cache
    search_path.erase(it);
  }
}

void
DictionaryManager::set_filesystem(std::unique_ptr<FileSystem> filesystem_)
{
  filesystem = std::move(filesystem_);
}
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
            s[i] = static_cast<char>(::toupper(s[i]));
        }
        else
            underscore_found = s[i]=='_';
    }
    return s;
}   // convertFilename2Language


} // namespace tinygettext


/* EOF */
