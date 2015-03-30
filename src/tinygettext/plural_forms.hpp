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

  bool operator==(const PluralForms& other) { return nplural == other.nplural && plural == other.plural; }
  bool operator!=(const PluralForms& other) { return !(*this == other); }

  operator bool() const {
    return plural!=NULL;
  }
};

} // namespace tinygettext

#endif

/* EOF */
