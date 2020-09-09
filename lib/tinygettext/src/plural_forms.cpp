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

#include "tinygettext/plural_forms.hpp"
#include "plural_forms_generated.hpp"


namespace tinygettext {

namespace {

/**
 *  Plural functions are used to select a string that matches a given
 *  count. \a n is the count and the return value is the string index
 *  used in the .po file, for example:
 *
 *   msgstr[0] = "You got %d error";
 *   msgstr[1] = "You got %d errors";
 *          ^-- return value of plural function
 */
unsigned int plural(int )     { return 0; }

} // namespace

PluralForms
PluralForms::from_string(const std::string& str)
{
  // Remove spaces from string before lookup
  std::string space_less_str;
  for(std::string::size_type i = 0; i < str.size(); ++i)
    if (!isspace(str[i]))
      space_less_str += str[i];

  std::unordered_map<std::string, PluralForms>::const_iterator it= g_plural_forms.find(space_less_str);
  if (it != g_plural_forms.end())
  {
    return it->second;
  }
  else
  {
    return PluralForms();
  }
}

} // namespace tinygettext

/* EOF */
