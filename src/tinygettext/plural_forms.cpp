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

#include "plural_forms.hpp"

#include <map>

namespace tinygettext {

/**
 *  Plural functions are used to select a string that matches a given
 *  count. \a n is the count and the return value is the string index
 *  used in the .po file, for example:
 *
 *   msgstr[0] = "You got %d error";
 *   msgstr[1] = "You got %d errors";
 *          ^-- return value of plural function
 */
// Base on Unicode CLDR (http://mlocati.github.io/cldr-to-gettext-plural-rules/)
unsigned int plural1(int )     { return 0; }

unsigned int plural2_1(int n)  { return (n != 1); }
unsigned int plural2_2(int n)  { return (n > 1); }
unsigned int plural2_is(int n) { return n % 10 != 1 || n % 100 == 11; }
unsigned int plural2_mk(int n) { return n==1 || n%10==1 ? 0 : 1; }

unsigned int plural3_be(int n) { return static_cast<unsigned int>(n % 10 == 1 && n % 100 != 11) ? 0 : ((n % 10 >= 2 && n % 10 <= 4 && (n % 100 < 12 || n % 100 > 14)) ? 1 : 2); }
unsigned int plural3_kw(int n) { return static_cast<unsigned int>(n == 1) ? 0 : ((n == 2) ? 1 : 2); }
unsigned int plural3_lv(int n) { return static_cast<unsigned int>(n % 10 == 0 || (n % 100 >= 11 && n % 100 <= 19)) ? 0 : ((n % 10 == 1 && n % 100 != 11) ? 1 : 2); }
unsigned int plural3_lt(int n) { return static_cast<unsigned int>(n % 10 == 1 && (n % 100 < 11 || n % 100 > 19)) ? 0 : ((n % 10 >= 2 && n % 10 <= 9 && (n % 100 < 11 || n % 100 > 19)) ? 1 : 2); }
unsigned int plural3_pl(int n) { return static_cast<unsigned int>(n == 1) ? 0 : ((n % 10 >= 2 && n % 10 <= 4 && (n % 100 < 12 || n % 100 > 14)) ? 1 : 2); }
unsigned int plural3_ro(int n) { return static_cast<unsigned int>(n == 1) ? 0 : ((n == 0 || (n != 1 && n % 100 >= 1 && n % 100 <= 19)) ? 1 : 2); }
unsigned int plural3_sk(int n) { return static_cast<unsigned int>(n == 1) ? 0 : ((n >= 2 && n <= 4) ? 1 : 2); }

unsigned int plural4_gd(int n) { return static_cast<unsigned int>(n == 1 || n == 11) ? 0 : ((n == 2 || n == 12) ? 1 : (((n >= 3 && n <= 10) || (n >= 13 && n <= 19)) ? 2 : 3)); }
unsigned int plural4_sl(int n) { return static_cast<unsigned int>(n % 100 == 1) ? 0 : ((n % 100 == 2) ? 1 : ((n % 100 == 3 || n % 100 == 4) ? 2 : 3)); }

unsigned int plural5_ga(int n) { return static_cast<unsigned int>(n == 1) ? 0 : ((n == 2) ? 1 : ((n >= 3 && n <= 6) ? 2 : ((n >= 7 && n <= 10) ? 3 : 4))); }

unsigned int plural6_ar(int n) { return static_cast<unsigned int>(n == 0) ? 0 : ((n == 1) ? 1 : ((n == 2) ? 2 : ((n % 100 >= 3 && n % 100 <= 10) ? 3 : ((n % 100 >= 11 && n % 100 <= 99) ? 4 : 5)))); }

typedef std::map<std::string, class PluralForms> tPluralForms;

PluralForms
PluralForms::from_string(const std::string& str)
{
  static tPluralForms plural_forms;

  if (plural_forms.empty())
  {
    // Note that the plural forms here shouldn't contain any spaces
    // Some strings are specific to STK po file
    // TODO in the future, map language code to plural form instead?
    plural_forms["Plural-Forms:nplurals=1;plural=0;"] = PluralForms(1, plural1);

    plural_forms["Plural-Forms:nplurals=2;plural=(n!=1);"] = PluralForms(2, plural2_1);
    plural_forms["Plural-Forms:nplurals=2;plural=n!=1;"] = PluralForms(2, plural2_1);
    plural_forms["Plural-Forms:nplurals=2;plural=(n>1);"] = PluralForms(2, plural2_2);
    plural_forms["Plural-Forms:nplurals=2;plural=(n%10!=1||n%100==11);"] = PluralForms(2, plural2_is);
    plural_forms["Plural-Forms:nplurals=2;plural=n==1||n%10==1?0:1;"] = PluralForms(2, plural2_mk);

    //FIXME wrong plural equation in be/ru translation files! (transifex error?) nplurals of be/ru is 3 indeed.
    plural_forms["Plural-Forms:nplurals=4;plural=(n%10==1&&n%100!=11?0:n%10>=2&&n%10<=4&&(n%100<12||n%100>14)?1:n%10==0||(n%10>=5&&n%10<=9)||(n%100>=11&&n%100<=14)?2:3);"] = PluralForms(3, plural3_be);
    plural_forms["Plural-Forms:nplurals=3;plural=n%10==1&&n%100!=11?0:n%10>=2&&n%10<=4&&(n%100<10||n%100>=20)?1:2;"] = PluralForms(3, plural3_be);
    plural_forms["Plural-Forms:nplurals=3;plural=(n%10==1&&n%100!=11?0:n%10>=2&&n%10<=4&&(n%100<10||n%100>=20)?1:2);"] = PluralForms(3, plural3_be);
    //FIXME wrong again
    plural_forms["Plural-Forms:nplurals=4;plural=(n==1)?0:(n==2)?1:(n==3)?2:3;"] = PluralForms(3, plural3_kw);
    plural_forms["Plural-Forms:nplurals=3;plural=(n%10==1&&n%100!=11?0:n!=0?1:2);"] = PluralForms(3, plural3_lv);
    plural_forms["Plural-Forms:nplurals=3;plural=(n%10==1&&n%100!=11?0:n%10>=2&&(n%100<10||n%100>=20)?1:2);"] = PluralForms(3, plural3_lt);
    plural_forms["Plural-Forms:nplurals=3;plural=(n==1?0:n%10>=2&&n%10<=4&&(n%100<10||n%100>=20)?1:2);"] = PluralForms(3, plural3_pl);
    plural_forms["Plural-Forms:nplurals=4;plural=(n==1?0:(n%10>=2&&n%10<=4)&&(n%100<12||n%100>14)?1:n!=1&&(n%10>=0&&n%10<=1)||(n%10>=5&&n%10<=9)||(n%100>=12&&n%100<=14)?2:3);"] = PluralForms(4, plural3_pl);
    plural_forms["Plural-Forms:nplurals=3;plural=(n==1?0:(((n%100>19)||((n%100==0)&&(n!=0)))?2:1));"] = PluralForms(3, plural3_ro);
    plural_forms["Plural-Forms:nplurals=3;plural=(n==1)?0:(n>=2&&n<=4)?1:2;"] = PluralForms(3, plural3_sk);

    plural_forms["Plural-Forms:nplurals=4;plural=(n%100==1?0:n%100==2?1:n%100==3||n%100==4?2:3);"] = PluralForms(4, plural4_sl);
    plural_forms["Plural-Forms:nplurals=4;plural=(n==1||n==11)?0:(n==2||n==12)?1:(n>2&&n<20)?2:3;"] = PluralForms(4, plural4_gd);

    plural_forms["Plural-Forms:nplurals=5;plural=(n==1?0:n==2?1:n<7?2:n<11?3:4);"] = PluralForms(5, plural5_ga);

    plural_forms["Plural-Forms:nplurals=6;plural=n==0?0:n==1?1:n==2?2:n%100>=3&&n%100<=10?3:n%100>=11&&n%100<=99?4:5;"] = PluralForms(6, plural6_ar);
  }

  // Remove spaces from string before lookup
  std::string space_less_str;
  for(std::string::size_type i = 0; i < str.size(); ++i)
    if (!isspace(str[i]))
      space_less_str += str[i];

  tPluralForms::const_iterator it= plural_forms.find(space_less_str);
  if (it != plural_forms.end())
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
