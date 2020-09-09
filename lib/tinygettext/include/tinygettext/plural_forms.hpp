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

#ifndef HEADER_TINYGETTEXT_PLURAL_FORMS_HPP
#define HEADER_TINYGETTEXT_PLURAL_FORMS_HPP

#include <string>

namespace tinygettext {

typedef unsigned int (*PluralFunc)(int n);

class PluralForms
{
private:
  unsigned int nplural;
  PluralFunc   plural;

public:
  static PluralForms from_string(const std::string& str);

  PluralForms()
    : nplural(0),
      plural(0)
  {}

  PluralForms(unsigned int nplural_, PluralFunc plural_)
    : nplural(nplural_),
      plural(plural_)
  {}

  unsigned int get_nplural() const { return nplural; }
  unsigned int get_plural(int n) const { if (plural) return plural(n); else return 0; }

  bool operator==(const PluralForms& other) const { return nplural == other.nplural && plural == other.plural; }
  bool operator!=(const PluralForms& other) const { return !(*this == other); }

  explicit operator bool() const {
    return plural != NULL;
  }
};

} // namespace tinygettext

#endif

/* EOF */
