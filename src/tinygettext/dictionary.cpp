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

#include <assert.h>
#include "dictionary.hpp"

#include "utils/log.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

namespace tinygettext {

Dictionary::Dictionary(const std::string& charset_) :
  entries(),
  ctxt_entries(),
  charset(charset_),
  plural_forms()
{
    m_has_fallback = false;
}

Dictionary::~Dictionary()
{
}

std::string
Dictionary::get_charset() const
{
  return charset;
}

void
Dictionary::set_plural_forms(const PluralForms& plural_forms_)
{
  plural_forms = plural_forms_;
}

PluralForms
Dictionary::get_plural_forms() const
{
  return plural_forms;
}

std::string
Dictionary::translate_plural(const std::string& msgid, const std::string& msgid_plural, int num)
{
  return translate_plural(entries, msgid, msgid_plural, num);
}

std::string
Dictionary::translate_plural(const Entries& dict, const std::string& msgid, const std::string& msgid_plural, int count)
{
  Entries::const_iterator i = dict.find(msgid);

  if (i != dict.end())
  {
    const std::vector<std::string>& msgstrs = i->second;
    unsigned int n = 0;
    n = plural_forms.get_plural(count);
    assert(/*n >= 0 &&*/ n < msgstrs.size());

    if (!msgstrs[n].empty())
      return msgstrs[n];
    else
      if (count == 1) // default to english rules
        return msgid;
      else
        return msgid_plural;
  }
  else
  {
    //log_info << "Couldn't translate: " << msgid << std::endl;
    //log_info << "Candidates: " << std::endl;
    //for (i = dict.begin(); i != dict.end(); ++i)
    //  log_info << "'" << i->first << "'" << std::endl;

    if (count == 1) // default to english rules
      return msgid;
    else
      return msgid_plural;
  }
}

std::string
Dictionary::translate(const std::string& msgid)
{
  return translate(entries, msgid);
}

std::string
Dictionary::translate(const Entries& dict, const std::string& msgid)
{
  Entries::const_iterator i = dict.find(msgid);
  if (i != dict.end() && !i->second.empty())
  {
    return i->second[0];
  }
  else
  {
    //log_info << "Couldn't translate: " << msgid << std::endl;

    if (m_has_fallback) return m_fallback->translate(msgid);
    else return msgid;
  }
}

std::string
Dictionary::translate_ctxt(const std::string& msgctxt, const std::string& msgid)
{
  CtxtEntries::iterator i = ctxt_entries.find(msgctxt);
  if (i != ctxt_entries.end())
  {
    return translate(i->second, msgid);
  }
  else
  {
    //log_info << "Couldn't translate: " << msgid << std::endl;
    return msgid;
  }
}

std::string
Dictionary::translate_ctxt_plural(const std::string& msgctxt,
                                  const std::string& msgid, const std::string& msgidplural, int num)
{
  CtxtEntries::iterator i = ctxt_entries.find(msgctxt);
  if (i != ctxt_entries.end())
  {
    return translate_plural(i->second, msgid, msgidplural, num);
  }
  else
  {
    //log_info << "Couldn't translate: " << msgid << std::endl;
    if (num != 1) // default to english
      return msgidplural;
    else
      return msgid;
  }
}

void
Dictionary::add_translation(const std::string& msgid, const std::string& ,
                            const std::vector<std::string>& msgstrs)
{
  // Do we need msgid2 for anything? its after all supplied to the
  // translate call, so we just throw it away here
  entries[msgid] = msgstrs;
}

void
Dictionary::add_translation(const std::string& msgid, const std::string& msgstr)
{
  std::vector<std::string>& vec = entries[msgid];
  if (vec.empty())
  {
    vec.push_back(msgstr);
  }
  else
  {
    Log::warn("tinygettext",
              "Collision in add translation: '%s' -> '%s' vs '%s'.",
              msgid.c_str(), msgstr.c_str(), vec[0].c_str());
    vec[0] = msgstr;
  }
}

void
Dictionary::add_translation(const std::string& msgctxt,
                            const std::string& msgid, const std::string& msgid_plural,
                            const std::vector<std::string>& msgstrs)
{
  std::vector<std::string>& vec = ctxt_entries[msgctxt][msgid];
  if (vec.empty())
  {
    vec = msgstrs;
  }
  else
  {
      Log::warn("tinygettext",
          "collision in add_translation(\"%s\", \"%s\", \"%s\")",
          msgctxt.c_str(), msgid.c_str(), msgid_plural.c_str());
    vec = msgstrs;
  }
}

void
Dictionary::add_translation(const std::string& msgctxt, const std::string& msgid, const std::string& msgstr)
{
  std::vector<std::string>& vec = ctxt_entries[msgctxt][msgid];
  if (vec.empty())
  {
    vec.push_back(msgstr);
  }
  else
  {
    Log::warn("tinygettext", "collision in add_translation(\"%s\", \"%s\")",
              msgctxt.c_str(), msgid.c_str());
    vec[0] = msgstr;
  }
}

std::set<wchar_t> Dictionary::get_all_used_chars()
{
    std::set<wchar_t> UsedChars;
    for (Entries::const_iterator i = entries.begin(); i != entries.end(); ++i)
    {
        const std::vector<std::string>& msgstrs = i->second;
        for (unsigned int k = 0; k < msgstrs.size(); k++)
        {
            irr::core::stringw ws = translations->fribidize((StringUtils::utf8ToWide(msgstrs[k])).c_str());
                for (unsigned int l = 0; l < ws.size(); ++l)
                    UsedChars.insert(ws[l]);
        }
    }

    for (CtxtEntries::const_iterator i = ctxt_entries.begin(); i != ctxt_entries.end(); ++i)
    {
        for (Entries::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
        {
            const std::vector<std::string>& msgstrs = j->second;
            for (unsigned int k = 0; k < msgstrs.size(); k++)
            {
                irr::core::stringw ws = translations->fribidize((StringUtils::utf8ToWide(msgstrs[k])).c_str());
                for (unsigned int l = 0; l < ws.size(); ++l)
                    UsedChars.insert(ws[l]);
            }
        }
    }
    return UsedChars;
}

} // namespace tinygettext

/* EOF */
