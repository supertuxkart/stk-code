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

#ifndef HEADER_TINYGETTEXT_DICTIONARY_HPP
#define HEADER_TINYGETTEXT_DICTIONARY_HPP

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "plural_forms.hpp"

namespace tinygettext {

/** A simple dictionary class that mimics gettext() behaviour. Each
    Dictionary only works for a single language, for managing multiple
    languages and .po files at once use the DictionaryManager. */
class Dictionary
{
private:
  typedef std::unordered_map<std::string, std::vector<std::string> > Entries;
  Entries entries;

  typedef std::unordered_map<std::string, Entries> CtxtEntries;
  CtxtEntries ctxt_entries;

  std::string charset;
  PluralForms plural_forms;

  std::string translate(const Entries& dict, const std::string& msgid) const;
  std::string translate_plural(const Entries& dict, const std::string& msgid, const std::string& msgidplural, int num) const;

  bool m_has_fallback;
  Dictionary* m_fallback;

public:
  /** Constructs a dictionary converting to the specified \a charset (default UTF-8) */
  Dictionary(const std::string& charset = "UTF-8");
  ~Dictionary();

  /** Return the charset used for this dictionary */
  std::string get_charset() const;

  void set_plural_forms(const PluralForms&);
  PluralForms get_plural_forms() const;


  /** Translate the string \a msgid. */
  std::string translate(const std::string& msgid) const;

  /** Translate the string \a msgid to its correct plural form, based
      on the number of items given by \a num. \a msgid_plural is \a msgid in
      plural form. */
  std::string translate_plural(const std::string& msgid, const std::string& msgidplural, int num) const;

  /** Translate the string \a msgid that is in context \a msgctx. A
      context is a way to disambiguate msgids that contain the same
      letters, but different meaning. For example "exit" might mean to
      quit doing something or it might refer to a door that leads
      outside (i.e. 'Ausgang' vs 'Beenden' in german) */
  std::string translate_ctxt(const std::string& msgctxt, const std::string& msgid) const;

  std::string translate_ctxt_plural(const std::string& msgctxt, const std::string& msgid, const std::string& msgidplural, int num) const;

  /** Add a translation from \a msgid to \a msgstr to the dictionary,
      where \a msgid is the singular form of the message, msgid_plural the
      plural form and msgstrs a table of translations. The right
      translation will be calculated based on the \a num argument to
      translate(). */
  void add_translation(const std::string& msgid, const std::string& msgid_plural,
                       const std::vector<std::string>& msgstrs);
  void add_translation(const std::string& msgctxt,
                       const std::string& msgid, const std::string& msgid_plural,
                       const std::vector<std::string>& msgstrs);

  /** Add a translation from \a msgid to \a msgstr to the
      dictionary */
  void add_translation(const std::string& msgid, const std::string& msgstr);
  void add_translation(const std::string& msgctxt, const std::string& msgid, const std::string& msgstr);

  /** Iterate over all messages, Func is of type:
      void func(const std::string& msgid, const std::vector<std::string>& msgstrs) */
  template<class Func>
  Func foreach(Func func)
  {
    for(Entries::iterator i = entries.begin(); i != entries.end(); ++i)
    {
      func(i->first, i->second);
    }
    return func;
  }

  void addFallback(Dictionary* fallback)
  {
      m_has_fallback = true;
      m_fallback = fallback;
  }

  /** Iterate over all messages with a context, Func is of type:
      void func(const std::string& ctxt, const std::string& msgid, const std::vector<std::string>& msgstrs) */
  template<class Func>
  Func foreach_ctxt(Func func)
  {
    for(CtxtEntries::iterator i = ctxt_entries.begin(); i != ctxt_entries.end(); ++i)
    {
      for(Entries::iterator j = i->second.begin(); j != i->second.end(); ++j)
      {
        func(i->first, j->first, j->second);
      }
    }
    return func;
  }

  /** Get unique characters (in utf32) used in current dictionary, useful to get for missing characters in font file. */
  std::set<unsigned int> get_all_used_chars();
private:
  Dictionary(const Dictionary&) = delete;
  Dictionary& operator=(const Dictionary&) = delete;
};

} // namespace tinygettext

#endif

/* EOF */
