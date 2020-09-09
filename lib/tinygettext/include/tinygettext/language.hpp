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

#ifndef HEADER_TINYGETTEXT_LANGUAGE_HPP
#define HEADER_TINYGETTEXT_LANGUAGE_HPP

#include <string>
#include <unordered_map>

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

  explicit operator bool() const { return language_spec != NULL; }

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

  bool operator==(const Language& rhs) const;
  bool operator!=(const Language& rhs) const;

  friend bool operator<(const Language& lhs, const Language& rhs);
  friend struct Language_hash;
};

inline bool operator<(const Language& lhs, const Language& rhs) {
  return lhs.language_spec < rhs.language_spec;
}

struct Language_hash
{
  size_t operator()(const Language& v) const
  {
    return reinterpret_cast<size_t>(v.language_spec);
  }
};

} // namespace tinygettext

#endif

/* EOF */
