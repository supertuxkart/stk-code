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

#include "tinygettext/language.hpp"

#include <assert.h>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace tinygettext {

struct LanguageSpec {
  /** Language code: "de", "en", ... */
  const char* language;

  /** Country code: "BR", "DE", ..., can be 0 */
  const char* country;

  /** Modifier/Varint: "Latn", "ije", "latin"..., can be 0 */
  const char* modifier;

  /** Language name: "German", "English", "French", ... */
  const char* name;
};

/** Language Definitions */
//*{
static const LanguageSpec languages[] = {
  { "aa", 0,    0, "Afar"                        },
  { "af", 0,    0, "Afrikaans"                   },
  { "af", "ZA", 0, "Afrikaans (South Africa)"    },
  { "am", 0,    0, "Amharic"                     },
  { "ar", 0,    0, "Arabic"                      },
  { "ar", "AR", 0, "Arabic (Argentina)"          },
  { "ar", "OM", 0, "Arabic (Oman)"               },
  { "ar", "SA", 0, "Arabic (Saudi Arabia)"       },
  { "ar", "SY", 0, "Arabic (Syrian Arab Republic)" },
  { "ar", "TN", 0, "Arabic (Tunisia)"            },
  { "as", 0,    0, "Assamese"                    },
  { "ast",0,    0, "Asturian"                    },
  { "ay", 0,    0, "Aymara"                      },
  { "az", 0,    0, "Azerbaijani"                 },
  { "az", "IR", 0, "Azerbaijani (Iran)"          },
  { "be", 0,    0, "Belarusian"                  },
  { "be", 0, "latin", "Belarusian"               },
  { "bg", 0,    0, "Bulgarian"                   },
  { "bg", "BG", 0, "Bulgarian (Bulgaria)"        },
  { "bn", 0,    0, "Bengali"                     },
  { "bn", "BD", 0, "Bengali (Bangladesh)"        },
  { "bn", "IN", 0, "Bengali (India)"             },
  { "bo", 0,    0, "Tibetan"                     },
  { "br", 0,    0, "Breton"                      },
  { "bs", 0,    0, "Bosnian"                     },
  { "bs", "BA", 0, "Bosnian (Bosnia/Herzegovina)"},
  { "bs", "BS", 0, "Bosnian (Bahamas)"           },
  { "ca", "ES", "valencia", "Catalan (valencia)" },
  { "ca", "ES", 0, "Catalan (Spain)"             },
  { "ca", 0,    "valencia", "Catalan (valencia)" },
  { "ca", 0,    0, "Catalan"                     },
  { "co", 0,    0, "Corsican"                    },
  { "cs", 0,    0, "Czech"                       },
  { "cs", "CZ", 0, "Czech (Czech Republic)"      },
  { "cy", 0,    0, "Welsh"                       },
  { "cy", "GB", 0, "Welsh (Great Britain)"       },
  { "cz", 0,    0, "Unknown language"            },
  { "da", 0,    0, "Danish"                      },
  { "da", "DK", 0, "Danish (Denmark)"            },
  { "de", 0,    0, "German"                      },
  { "de", "AT", 0, "German (Austria)"            },
  { "de", "CH", 0, "German (Switzerland)"        },
  { "de", "DE", 0, "German (Germany)"            },
  { "dk", 0,    0, "Unknown language"            },
  { "dz", 0,    0, "Dzongkha"                    },
  { "el", 0,    0, "Greek"                       },
  { "el", "GR", 0, "Greek (Greece)"              },
  { "en", 0,    0, "English"                     },
  { "en", "AU", 0, "English (Australia)"         },
  { "en", "CA", 0, "English (Canada)"            },
  { "en", "GB", 0, "English (Great Britain)"     },
  { "en", "US", 0, "English (United States)"     },
  { "en", "ZA", 0, "English (South Africa)"      },
  { "en", 0, "boldquot", "English"               },
  { "en", 0, "quot", "English"                   },
  { "en", "US", "piglatin", "English"            },
  { "eo", 0,    0, "Esperanto"                   },
  { "es", 0,    0, "Spanish"                     },
  { "es", "AR", 0, "Spanish (Argentina)"         },
  { "es", "CL", 0, "Spanish (Chile)"             },
  { "es", "CO", 0, "Spanish (Colombia)"          },
  { "es", "CR", 0, "Spanish (Costa Rica)"        },
  { "es", "DO", 0, "Spanish (Dominican Republic)"},
  { "es", "EC", 0, "Spanish (Ecuador)"           },
  { "es", "ES", 0, "Spanish (Spain)"             },
  { "es", "GT", 0, "Spanish (Guatemala)"         },
  { "es", "HN", 0, "Spanish (Honduras)"          },
  { "es", "LA", 0, "Spanish (Laos)"              },
  { "es", "MX", 0, "Spanish (Mexico)"            },
  { "es", "NI", 0, "Spanish (Nicaragua)"         },
  { "es", "PA", 0, "Spanish (Panama)"            },
  { "es", "PE", 0, "Spanish (Peru)"              },
  { "es", "PR", 0, "Spanish (Puerto Rico)"       },
  { "es", "SV", 0, "Spanish (El Salvador)"       },
  { "es", "UY", 0, "Spanish (Uruguay)"           },
  { "es", "VE", 0, "Spanish (Venezuela)"         },
  { "et", 0,    0, "Estonian"                    },
  { "et", "EE", 0, "Estonian (Estonia)"          },
  { "et", "ET", 0, "Estonian (Ethiopia)"         },
  { "eu", 0,    0, "Basque"                      },
  { "eu", "ES", 0, "Basque (Spain)"              },
  { "fa", 0,    0, "Persian"                     },
  { "fa", "AF", 0, "Persian (Afghanistan)"       },
  { "fa", "IR", 0, "Persian (Iran)"              },
  { "fi", 0,    0, "Finnish"                     },
  { "fi", "FI", 0, "Finnish (Finland)"           },
  { "fil", 0,   0, "Filipino"                    },
  { "fo", 0,    0, "Faroese"                     },
  { "fo", "FO", 0, "Faeroese (Faroe Islands)"    },
  { "fr", 0,    0, "French"                      },
  { "fr", "CA", 0, "French (Canada)"             },
  { "fr", "CH", 0, "French (Switzerland)"        },
  { "fr", "FR", 0, "French (France)"             },
  { "fr", "LU", 0, "French (Luxembourg)"         },
  { "fy", 0,    0, "Frisian"                     },
  { "ga", 0,    0, "Irish"                       },
  { "gd", 0,    0, "Gaelic Scots"                },
  { "gl", 0,    0, "Galician"                    },
  { "gl", "ES", 0, "Galician (Spain)"            },
  { "gn", 0,    0, "Guarani"                     },
  { "gu", 0,    0, "Gujarati"                    },
  { "gv", 0,    0, "Manx"                        },
  { "ha", 0,    0, "Hausa"                       },
  { "he", 0,    0, "Hebrew"                      },
  { "he", "IL", 0, "Hebrew (Israel)"             },
  { "hi", 0,    0, "Hindi"                       },
  { "hr", 0,    0, "Croatian"                    },
  { "hr", "HR", 0, "Croatian (Croatia)"          },
  { "hu", 0,    0, "Hungarian"                   },
  { "hu", "HU", 0, "Hungarian (Hungary)"         },
  { "hy", 0,    0, "Armenian"                    },
  { "ia", 0,    0, "Interlingua"                 },
  { "id", 0,    0, "Indonesian"                  },
  { "id", "ID", 0, "Indonesian (Indonesia)"      },
  { "is", 0,    0, "Icelandic"                   },
  { "is", "IS", 0, "Icelandic (Iceland)"         },
  { "it", 0,    0, "Italian"                     },
  { "it", "CH", 0, "Italian (Switzerland)"       },
  { "it", "IT", 0, "Italian (Italy)"             },
  { "iu", 0,    0, "Inuktitut"                   },
  { "ja", 0,    0, "Japanese"                    },
  { "ja", "JP", 0, "Japanese (Japan)"            },
  { "jbo",0,    0, "Lojban"                      },
  { "ka", 0,    0, "Georgian"                    },
  { "kk", 0,    0, "Kazakh"                      },
  { "kl", 0,    0, "Kalaallisut"                 },
  { "km", 0,    0, "Khmer"                       },
  { "km", "KH", 0, "Khmer (Cambodia)"            },
  { "kn", 0,    0, "Kannada"                     },
  { "ko", 0,    0, "Korean"                      },
  { "ko", "KR", 0, "Korean (Korea)"              },
  { "krl",0,    0, "Karelian"                    },
  { "ku", 0,    0, "Kurdish"                     },
  { "kw", 0,    0, "Cornish"                     },
  { "ky", 0,    0, "Kirghiz"                     },
  { "la", 0,    0, "Latin"                       },
  { "lo", 0,    0, "Lao"                         },
  { "lt", 0,    0, "Lithuanian"                  },
  { "lt", "LT", 0, "Lithuanian (Lithuania)"      },
  { "lv", 0,    0, "Latvian"                     },
  { "lv", "LV", 0, "Latvian (Latvia)"            },
  { "mg", 0,    0, "Malagasy"                    },
  { "mi", 0,    0, "Maori"                       },
  { "mk", 0,    0, "Macedonian"                  },
  { "mk", "MK", 0, "Macedonian (Macedonia)"      },
  { "ml", 0,    0, "Malayalam"                   },
  { "mn", 0,    0, "Mongolian"                   },
  { "mn", "MN", 0, "Mongolian (Mongolia)"        },
  { "mr", 0,    0, "Marathi"                     },
  { "ms", 0,    0, "Malay"                       },
  { "ms", "MY", 0, "Malay (Malaysia)"            },
  { "mt", 0,    0, "Maltese"                     },
  { "my", 0,    0, "Burmese"                     },
  { "my", "MM", 0, "Burmese (Myanmar)"           },
  { "nb", 0,    0, "Norwegian Bokmal"            },
  { "nb", "NO", 0, "Norwegian Bokmål (Norway)"   },
  { "ne", 0,    0, "Nepali"                      },
  { "nl", 0,    0, "Dutch"                       },
  { "nl", "BE", 0, "Dutch (Belgium)"             },
  { "nl", "NL", 0, "Dutch (Netherlands)"         },
  { "nn", 0,    0, "Norwegian Nynorsk"           },
  { "nn", "NO", 0, "Norwegian Nynorsk (Norway)"  },
  // DEPRECATED
  //{ "no", 0,    0, "Norwegian"                   },
  //{ "no", "NO", 0, "Norwegian (Norway)"          },
  //{ "no", "NY", 0, "Norwegian (NY)"              },
  { "nr", 0,    0, "Ndebele, South"              },
  { "oc", 0,    0, "Occitan post 1500"           },
  { "om", 0,    0, "Oromo"                       },
  { "or", 0,    0, "Oriya"                       },
  { "os", 0,    0, "Ossetian"                    },
  { "pa", 0,    0, "Punjabi"                     },
  { "pl", 0,    0, "Polish"                      },
  { "pl", "PL", 0, "Polish (Poland)"             },
  { "pms",0,    0, "Piedmontese"                 },
  { "ps", 0,    0, "Pashto"                      },
  { "pt", 0,    0, "Portuguese"                  },
  { "pt", "BR", 0, "Portuguese (Brazil)"         },
  { "pt", "PT", 0, "Portuguese (Portugal)"       },
  { "qu", 0,    0, "Quechua"                     },
  { "rm", 0,    0, "Rhaeto-Romance"              },
  { "ro", 0,    0, "Romanian"                    },
  { "ro", "RO", 0, "Romanian (Romania)"          },
  { "ru", 0,    0, "Russian"                     },
  { "rue", 0,   0, "Rusyn"                       },
  { "ru", "RU", 0, "Russian (Russia"             },
  { "rw", 0,    0, "Kinyarwanda"                 },
  { "sa", 0,    0, "Sanskrit"                    },
  { "sc", 0,    0, "Sardinian"                   },
  { "sco",0,    0, "Scots"                       },
  { "sd", 0,    0, "Sindhi"                      },
  { "se", 0,    0, "Sami"                        },
  { "se", "NO", 0, "Sami (Norway)"               },
  { "si", 0,    0, "Sinhalese"                   },
  { "sk", 0,    0, "Slovak"                      },
  { "sk", "SK", 0, "Slovak (Slovakia)"           },
  { "sl", 0,    0, "Slovenian"                   },
  { "sl", "SI", 0, "Slovenian (Slovenia)"        },
  { "sl", "SL", 0, "Slovenian (Sierra Leone)"    },
  { "sm", 0,    0, "Samoan"                      },
  { "so", 0,    0, "Somali"                      },
  { "sp", 0,    0, "Unknown language"            },
  { "sq", 0,    0, "Albanian"                    },
  { "sq", "AL", 0, "Albanian (Albania)"          },
  { "sr", 0,    0, "Serbian"                     },
  { "sr", "YU", 0, "Serbian (Yugoslavia)"        },
  { "sr", 0,"ije", "Serbian"                     },
  { "sr", 0, "latin", "Serbian"                  },
  { "sr", 0, "Latn",  "Serbian"                  },
  { "ss", 0,    0, "Swati"                       },
  { "st", 0,    0, "Sotho"                       },
  { "sv", 0,    0, "Swedish"                     },
  { "sv", "SE", 0, "Swedish (Sweden)"            },
  { "sv", "SV", 0, "Swedish (El Salvador)"       },
  { "sw", 0,    0, "Swahili"                     },
  { "szl", 0,   0, "Silesian"                    },
  { "ta", 0,    0, "Tamil"                       },
  { "te", 0,    0, "Telugu"                      },
  { "tg", 0,    0, "Tajik"                       },
  { "th", 0,    0, "Thai"                        },
  { "th", "TH", 0, "Thai (Thailand)"             },
  { "ti", 0,    0, "Tigrinya"                    },
  { "tk", 0,    0, "Turkmen"                     },
  { "tl", 0,    0, "Tagalog"                     },
  { "to", 0,    0, "Tonga"                       },
  { "tr", 0,    0, "Turkish"                     },
  { "tr", "TR", 0, "Turkish (Turkey)"            },
  { "ts", 0,    0, "Tsonga"                      },
  { "tt", 0,    0, "Tatar"                       },
  { "ug", 0,    0, "Uighur"                      },
  { "uk", 0,    0, "Ukrainian"                   },
  { "uk", "UA", 0, "Ukrainian (Ukraine)"         },
  { "ur", 0,    0, "Urdu"                        },
  { "ur", "PK", 0, "Urdu (Pakistan)"             },
  { "uz", 0,    0, "Uzbek"                       },
  { "uz", 0, "cyrillic", "Uzbek"                 },
  { "vi", 0,    0, "Vietnamese"                  },
  { "vi", "VN", 0, "Vietnamese (Vietnam)"        },
  { "wa", 0,    0, "Walloon"                     },
  { "wo", 0,    0, "Wolof"                       },
  { "xh", 0,    0, "Xhosa"                       },
  { "yi", 0,    0, "Yiddish"                     },
  { "yo", 0,    0, "Yoruba"                      },
  { "zh", 0,    0, "Chinese"                     },
  { "zh", "CN", 0, "Chinese (simplified)"        },
  { "zh", "HK", 0, "Chinese (Hong Kong)"         },
  { "zh", "TW", 0, "Chinese (traditional)"       },
  { "zu", 0,    0, "Zulu"                        },
  { NULL, 0,    0, NULL                          }
};
//*}

namespace {

std::string
resolve_language_alias(const std::string& name)
{
  typedef std::unordered_map<std::string, std::string> Aliases;
  static Aliases language_aliases;
  if (language_aliases.empty())
  {
    // FIXME: Many of those are not useful for us, since we leave
    // encoding to the app, not to the language, we could/should
    // also match against all language names, not just aliases from
    // locale.alias

    // Aliases taken from /etc/locale.alias
    language_aliases["bokmal"]           = "nb_NO.ISO-8859-1";
    language_aliases["bokmål"]           = "nb_NO.ISO-8859-1";
    language_aliases["catalan"]          = "ca_ES.ISO-8859-1";
    language_aliases["croatian"]         = "hr_HR.ISO-8859-2";
    language_aliases["czech"]            = "cs_CZ.ISO-8859-2";
    language_aliases["danish"]           = "da_DK.ISO-8859-1";
    language_aliases["dansk"]            = "da_DK.ISO-8859-1";
    language_aliases["deutsch"]          = "de_DE.ISO-8859-1";
    language_aliases["dutch"]            = "nl_NL.ISO-8859-1";
    language_aliases["eesti"]            = "et_EE.ISO-8859-1";
    language_aliases["estonian"]         = "et_EE.ISO-8859-1";
    language_aliases["finnish"]          = "fi_FI.ISO-8859-1";
    language_aliases["français"]         = "fr_FR.ISO-8859-1";
    language_aliases["french"]           = "fr_FR.ISO-8859-1";
    language_aliases["galego"]           = "gl_ES.ISO-8859-1";
    language_aliases["galician"]         = "gl_ES.ISO-8859-1";
    language_aliases["german"]           = "de_DE.ISO-8859-1";
    language_aliases["greek"]            = "el_GR.ISO-8859-7";
    language_aliases["hebrew"]           = "he_IL.ISO-8859-8";
    language_aliases["hrvatski"]         = "hr_HR.ISO-8859-2";
    language_aliases["hungarian"]        = "hu_HU.ISO-8859-2";
    language_aliases["icelandic"]        = "is_IS.ISO-8859-1";
    language_aliases["italian"]          = "it_IT.ISO-8859-1";
    language_aliases["japanese"]         = "ja_JP.eucJP";
    language_aliases["japanese.euc"]     = "ja_JP.eucJP";
    language_aliases["ja_JP"]            = "ja_JP.eucJP";
    language_aliases["ja_JP.ujis"]       = "ja_JP.eucJP";
    language_aliases["japanese.sjis"]    = "ja_JP.SJIS";
    language_aliases["korean"]           = "ko_KR.eucKR";
    language_aliases["korean.euc"]       = "ko_KR.eucKR";
    language_aliases["ko_KR"]            = "ko_KR.eucKR";
    language_aliases["lithuanian"]       = "lt_LT.ISO-8859-13";
    language_aliases["no_NO"]            = "nb_NO.ISO-8859-1";
    language_aliases["no_NO.ISO-8859-1"] = "nb_NO.ISO-8859-1";
    language_aliases["norwegian"]        = "nb_NO.ISO-8859-1";
    language_aliases["nynorsk"]          = "nn_NO.ISO-8859-1";
    language_aliases["polish"]           = "pl_PL.ISO-8859-2";
    language_aliases["portuguese"]       = "pt_PT.ISO-8859-1";
    language_aliases["romanian"]         = "ro_RO.ISO-8859-2";
    language_aliases["russian"]          = "ru_RU.ISO-8859-5";
    language_aliases["slovak"]           = "sk_SK.ISO-8859-2";
    language_aliases["slovene"]          = "sl_SI.ISO-8859-2";
    language_aliases["slovenian"]        = "sl_SI.ISO-8859-2";
    language_aliases["spanish"]          = "es_ES.ISO-8859-1";
    language_aliases["swedish"]          = "sv_SE.ISO-8859-1";
    language_aliases["thai"]             = "th_TH.TIS-620";
    language_aliases["turkish"]          = "tr_TR.ISO-8859-9";
  }

  std::string name_lowercase;
  name_lowercase.resize(name.size());
  for(std::string::size_type i = 0; i < name.size(); ++i)
    name_lowercase[i] = static_cast<char>(tolower(name[i]));

  Aliases::iterator i = language_aliases.find(name_lowercase);
  if (i != language_aliases.end())
  {
    return i->second;
  }
  else
  {
    return name;
  }
}

} // namespace

Language
Language::from_spec(const std::string& language, const std::string& country, const std::string& modifier)
{
  typedef std::unordered_map<std::string, std::vector<const LanguageSpec*> > LanguageSpecMap;
  static LanguageSpecMap language_map;

  if (language_map.empty())
  { // Init language_map
    for(int i = 0; languages[i].language != NULL; ++i)
      language_map[languages[i].language].push_back(&languages[i]);
  }

  LanguageSpecMap::iterator i = language_map.find(language);
  if (i != language_map.end())
  {
    std::vector<const LanguageSpec*>& lst = i->second;

    LanguageSpec tmpspec;
    tmpspec.language = language.c_str();
    tmpspec.country  = country.c_str();
    tmpspec.modifier = modifier.c_str();
    Language tmplang(&tmpspec);

    const LanguageSpec* best_match = 0;
    int best_match_score = 0;
    for(std::vector<const LanguageSpec*>::iterator j = lst.begin(); j != lst.end(); ++j)
    { // Search for the language that best matches the given spec, value country more then modifier
      int score = Language::match(Language(*j), tmplang);

      if (score > best_match_score)
      {
        best_match = *j;
        best_match_score = score;
      }
    }
    assert(best_match);
    return Language(best_match);
  }
  else
  {
    return Language();
  }
}

Language
Language::from_name(const std::string& spec_str)
{
  return from_env(resolve_language_alias(spec_str));
}

Language
Language::from_env(const std::string& env)
{
  // Split LANGUAGE_COUNTRY.CODESET@MODIFIER into parts
  std::string::size_type ln = env.find('_');
  std::string::size_type dt = env.find('.');
  std::string::size_type at = env.find('@');

  std::string language;
  std::string country;
  std::string codeset;
  std::string modifier;

  //std::cout << ln << " " << dt << " " << at << std::endl;

  language = env.substr(0, std::min(std::min(ln, dt), at));

  if (ln != std::string::npos && ln+1 < env.size()) // _
  {
    country = env.substr(ln+1, (std::min(dt, at) == std::string::npos) ? std::string::npos : std::min(dt, at) - (ln+1));
  }

  if (dt != std::string::npos && dt+1 < env.size()) // .
  {
    codeset = env.substr(dt+1, (at == std::string::npos) ? std::string::npos : (at - (dt+1)));
  }

  if (at != std::string::npos && at+1 < env.size()) // @
  {
    modifier = env.substr(at+1);
  }

  return from_spec(language, country, modifier);
}

Language::Language(const LanguageSpec* language_spec_)
  : language_spec(language_spec_)
{
}

Language::Language()
  : language_spec(0)
{
}

int
Language::match(const Language& lhs, const Language& rhs)
{
  if (lhs.get_language() != rhs.get_language())
  {
    return 0;
  }
  else
  {
    static int match_tbl[3][3] = {
      // modifier match, wildchard, miss
      { 9, 8, 5 }, // country match
      { 7, 6, 3 }, // country wildcard
      { 4, 2, 1 }, // country miss
    };

    int c;
    if (lhs.get_country() == rhs.get_country())
      c = 0;
    else if (lhs.get_country().empty() || rhs.get_country().empty())
      c = 1;
    else
      c = 2;

    int m;
    if (lhs.get_modifier() == rhs.get_modifier())
      m = 0;
    else if (lhs.get_modifier().empty() || rhs.get_modifier().empty())
      m = 1;
    else
      m = 2;

    return match_tbl[c][m];
  }
}

std::string
Language::get_language() const
{
  if (language_spec)
    return language_spec->language;
  else
    return "";
}

std::string
Language::get_country()  const
{
  if (language_spec && language_spec->country)
    return language_spec->country;
  else
    return "";
}

std::string
Language::get_modifier() const
{
  if (language_spec && language_spec->modifier)
    return language_spec->modifier;
  else
    return "";
}

std::string
Language::get_name()  const
{
  if (language_spec)
    return language_spec->name;
  else
    return "";
}

std::string
Language::str() const
{
  if (language_spec)
  {
    std::string var;
    var += language_spec->language;
    if (language_spec->country)
    {
      var += "_";
      var += language_spec->country;
    }

    if (language_spec->modifier)
    {
      var += "@";
      var += language_spec->modifier;
    }
    return var;
  }
  else
  {
    return "";
  }
}

bool
Language::operator==(const Language& rhs) const
{
  return language_spec == rhs.language_spec;
}

bool
Language::operator!=(const Language& rhs) const
{
  return language_spec != rhs.language_spec;
}

} // namespace tinygettext

/* EOF */
