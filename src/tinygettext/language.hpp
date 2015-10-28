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

#ifndef HEADER_TINYGETTEXT_LANGUAGE_HPP
#define HEADER_TINYGETTEXT_LANGUAGE_HPP

#include <string>

namespace tinygettext {

struct LanguageSpec;

/** Lightweight wrapper around LanguageSpec */
class Language
{
private:
  const LanguageSpec* language_spec;

  Language(const LanguageSpec* language_spec);

public:
  /** Create a language from language and country code:
      Example: Languge("de", "DE"); */
  static Language from_spec(const std::string& language,
                            const std::string& country = std::string(),
                            const std::string& modifier = std::string());

  /** Create a language from language and country code:
      Example: Languge("deutsch");
      Example: Languge("de_DE"); */
  static Language from_name(const std::string& str);

  /** Create a language from an environment variable style string (e.g de_DE.UTF-8@modifier) */
  static Language from_env(const std::string& env);

  /** Compares two Languages, returns 0 on missmatch and a score
      between 1 and 9 on match, the higher the score the better the
      match */
  static int match(const Language& lhs, const Language& rhs);

  /** Create an undefined Language object */
  Language();

  operator bool() const { return language_spec!=NULL; }

  /** Returns the language code (i.e. de, en, fr) */
  std::string get_language() const;

  /** Returns the country code (i.e. DE, AT, US) */
  std::string get_country()  const;

  /** Returns the modifier of the language (i.e. latn or Latn for
      Serbian with non-cyrilic characters) */
  std::string get_modifier()  const;

  /** Returns the human readable name of the Language */
  std::string get_name() const;

  /** Returns the Language as string in the form of an environment
      variable: {language}_{country}@{modifier} */
  std::string str() const;

  bool operator==(const Language& rhs);
  bool operator!=(const Language& rhs);

  friend bool operator<(const Language& lhs, const Language& rhs);
};

inline bool operator<(const Language& lhs, const Language& rhs) {
  return lhs.language_spec < rhs.language_spec;
}

} // namespace tinygettext

#endif

/* EOF */
