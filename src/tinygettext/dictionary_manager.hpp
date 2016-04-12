//  tinygettext - A gettext replacement that works directly on .po files
//  Copyright (C) 2006-2015 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_TINYGETTEXT_DICTIONARY_MANAGER_HPP
#define HEADER_TINYGETTEXT_DICTIONARY_MANAGER_HPP

#include <map>
#include <set>
#include <string>
#include <vector>
#include <memory>

#include "dictionary.hpp"
#include "language.hpp"

namespace tinygettext {

class FileSystem;

/** Manager class for dictionaries, you give it a bunch of directories
    with .po files and it will then automatically load the right file
    on demand depending on which language was set. */
class DictionaryManager
{
private:
  typedef std::map<Language, Dictionary*> Dictionaries;
  Dictionaries dictionaries;

  typedef std::vector<std::string> SearchPath;
  SearchPath search_path;

  std::string charset;
  bool        use_fuzzy;

  Language    current_language;
  Dictionary* current_dict;

  Dictionary  empty_dict;

  std::unique_ptr<FileSystem> filesystem;

  void clear_cache();

#ifdef DEBUG
    unsigned int m_magic_number;
#endif

public:
  DictionaryManager(const std::string& charset_ = "UTF-8");
  ~DictionaryManager();

  /** Return the currently active dictionary, if none is set, an empty
      dictionary is returned. */
  Dictionary& get_dictionary();

  /** Get dictionary for language */
  Dictionary& get_dictionary(const Language& language);

  /** Set a language based on a four? letter country code */
  void set_language(const Language& language);

  /** returns the (normalized) country code of the currently used language */
  Language get_language() const;

  void set_use_fuzzy(bool t);
  bool get_use_fuzzy() const;

  /** Set a charset that will be set on the returned dictionaries */
  void set_charset(const std::string& charset);

  /** Add a directory to the search path for dictionaries, earlier
      added directories have higher priority then later added ones */
  void add_directory(const std::string& pathname);

  /** Return a set of the available languages in their country code */
  std::set<Language> get_languages();

  void set_filesystem(std::unique_ptr<FileSystem> filesystem);
  std::string convertFilename2Language(const std::string &s_in) const;


private:
  DictionaryManager (const DictionaryManager&);
  DictionaryManager& operator= (const DictionaryManager&);
};

} // namespace tinygettext

#endif

/* EOF */
