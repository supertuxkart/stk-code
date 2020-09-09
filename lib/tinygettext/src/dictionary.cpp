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

#include <assert.h>

#include "tinygettext/log_stream.hpp"
#include "tinygettext/dictionary.hpp"

namespace tinygettext {

namespace {

std::ostream& operator<<(std::ostream& o, const std::vector<std::string>& v)
{
  for (std::vector<std::string>::const_iterator it = v.begin(); it != v.end(); ++it)
  {
    if (it != v.begin())
      o << ", ";
    o << "'" << *it << "'";
  }
  return o;
}

} // namespace

Dictionary::Dictionary(const std::string& charset_) :
  entries(),
  ctxt_entries(),
  charset(charset_),
  plural_forms(),
  m_has_fallback(false),
  m_fallback()
{
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
Dictionary::translate_plural(const std::string& msgid, const std::string& msgid_plural, int num) const
{
  return translate_plural(entries, msgid, msgid_plural, num);
}

std::string
Dictionary::translate_plural(const Entries& dict, const std::string& msgid, const std::string& msgid_plural, int count) const
{
  Entries::const_iterator it = dict.find(msgid);
  if (it != dict.end())
  {
    unsigned int n = plural_forms.get_plural(count);
    const std::vector<std::string>& msgstrs = it->second;
    if (n >= msgstrs.size())
    {
      log_error << "Plural translation not available (and not set to empty): '" << msgid << "'" << std::endl;
      log_error << "Missing plural form: " << n << std::endl;
      return msgid;
    }

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
    log_info << "Couldn't translate: " << msgid << std::endl;
    log_info << "Candidates: " << std::endl;
    for (it = dict.begin(); it != dict.end(); ++it)
      log_info << "'" << it->first << "'" << std::endl;

    if (count == 1) // default to english rules
      return msgid;
    else
      return msgid_plural;
  }
}

std::string
Dictionary::translate(const std::string& msgid) const
{
  return translate(entries, msgid);
}

std::string
Dictionary::translate(const Entries& dict, const std::string& msgid) const
{
  Entries::const_iterator i = dict.find(msgid);
  if (i != dict.end() && !i->second.empty())
  {
    return i->second[0];
  }
  else
  {
    log_info << "Couldn't translate: " << msgid << std::endl;

    if (m_has_fallback) return m_fallback->translate(msgid);
    else return msgid;
  }
}

std::string
Dictionary::translate_ctxt(const std::string& msgctxt, const std::string& msgid) const
{
  CtxtEntries::const_iterator i = ctxt_entries.find(msgctxt);
  if (i != ctxt_entries.end())
  {
    return translate(i->second, msgid);
  }
  else
  {
    log_info << "Couldn't translate: " << msgid << std::endl;
    return msgid;
  }
}

std::string
Dictionary::translate_ctxt_plural(const std::string& msgctxt,
                                  const std::string& msgid, const std::string& msgidplural, int num) const
{
  CtxtEntries::const_iterator i = ctxt_entries.find(msgctxt);
  if (i != ctxt_entries.end())
  {
    return translate_plural(i->second, msgid, msgidplural, num);
  }
  else
  {
    log_info << "Couldn't translate: " << msgid << std::endl;
    if (num != 1) // default to english
      return msgidplural;
    else
      return msgid;
  }
}

void
Dictionary::add_translation(const std::string& msgid, const std::string& msgid_plural,
                            const std::vector<std::string>& msgstrs)
{
  std::vector<std::string>& vec = entries[msgid];
  if (vec.empty())
  {
    vec = msgstrs;
  }
  else if (vec != msgstrs)
  {
    log_warning << "collision in add_translation: '"
                << msgid << "', '" << msgid_plural
                << "' -> [" << vec << "] vs [" << msgstrs << "]" << std::endl;
    vec = msgstrs;
  }
}

void
Dictionary::add_translation(const std::string& msgid, const std::string& msgstr)
{
  std::vector<std::string>& vec = entries[msgid];
  if (vec.empty())
  {
    vec.push_back(msgstr);
  }
  else if (vec[0] != msgstr)
  {
    log_warning << "collision in add_translation: '"
                << msgid << "' -> '" << msgstr << "' vs '" << vec[0] << "'" << std::endl;
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
  else if (vec != msgstrs)
  {
    log_warning << "collision in add_translation: '"
                << msgctxt << "', '" << msgid << "', '" << msgid_plural
                << "' -> [" << vec << "] vs [" << msgstrs << "]" << std::endl;
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
  else if (vec[0] != msgstr)
  {
    log_warning << "collision in add_translation: '"
                << msgctxt << "', '" << msgid
                << "' -> '" << vec[0] << "' vs '" << msgstr << "'" << std::endl;
    vec[0] = msgstr;
  }
}

} // namespace tinygettext

/* EOF */
