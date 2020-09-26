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

#ifndef HEADER_TINYGETTEXT_DICTIONARY_MANAGER_HPP
#define HEADER_TINYGETTEXT_DICTIONARY_MANAGER_HPP

#include <deque>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

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
  typedef std::unordered_map<Language, Dictionary*, Language_hash> Dictionaries;
  Dictionaries dictionaries;

  typedef std::deque<std::string> SearchPath;
  SearchPath search_path;

  std::string charset;
  bool        use_fuzzy;

  Language    current_language;
  Dictionary* current_dict;

  Dictionary  empty_dict;

  std::unique_ptr<FileSystem> filesystem;

  void clear_cache();

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
      added directories have higher priority then later added ones.
      Set @p precedence to true to invert this for a single addition. */
  void add_directory(const std::string& pathname, bool precedence = false);

  /** Remove a directory from the search path */
  void remove_directory(const std::string& pathname);

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
