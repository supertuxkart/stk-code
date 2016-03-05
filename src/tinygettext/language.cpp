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

#include "language.hpp"

#include "utils/string_utils.hpp"

#include <map>
#include <assert.h>
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

  /** Language name in the specified language */
  const char* name_localized;
};

/** Language Definitions */
//*{
static const LanguageSpec languages[] = {
  { "aa", 0,    0, "Afar"                        , "ʿAfár af"                 },
  { "af", 0,    0, "Afrikaans"                   , "Afrikaans"                },
  { "af", "ZA", 0, "Afrikaans (South Africa)"    , 0                          },
  { "am", 0,    0, "Amharic"                     , "ኣማርኛ"                    },
  { "ar", 0,    0, "Arabic"                      , "العربية"                  },
  { "ar", "AR", 0, "Arabic (Argentina)"          , 0                          },
  { "ar", "OM", 0, "Arabic (Oman)"               , 0                          },
  { "ar", "SA", 0, "Arabic (Saudi Arabia)"       , 0                          },
  { "ar", "SY", 0, "Arabic (Syrian Arab Republic)", 0                         },
  { "ar", "TN", 0, "Arabic (Tunisia)"            , 0                          },
  { "as", 0,    0, "Assamese"                    , "অসমীয়া"                    },
  { "ast",0,    0, "Asturian"                    , "Asturianu"                },
  { "ay", 0,    0, "Aymara"                      , "aymar aru"                },
  { "az", 0,    0, "Azerbaijani"                 , "Azərbaycanca"             },
  { "az", "IR", 0, "Azerbaijani (Iran)"          , 0                          },
  { "be", 0,    0, "Belarusian"                  , "Беларуская мова"          },
  { "be", 0, "latin", "Belarusian"               , "Беларуская мова"          },
  { "bg", 0,    0, "Bulgarian"                   , "български"                },
  { "bg", "BG", 0, "Bulgarian (Bulgaria)"        , 0                          },
  { "bn", 0,    0, "Bengali"                     , "বাংলা"                     },
  { "bn", "BD", 0, "Bengali (Bangladesh)"        , 0                          },
  { "bn", "IN", 0, "Bengali (India)"             , 0                          },
  { "bo", 0,    0, "Tibetan"                     , "བོད་སྐད་"                     },
  { "br", 0,    0, "Breton"                      , "brezhoneg"                },
  { "bs", 0,    0, "Bosnian"                     , "Bosanski"                 },
  { "bs", "BA", 0, "Bosnian (Bosnia/Herzegovina)", 0                          },
  { "bs", "BS", 0, "Bosnian (Bahamas)"           , 0                          },
  { "ca", "ES", "valencia", "Catalan (valencia)" , 0                          },
  { "ca", "ES", 0, "Catalan (Spain)"             , 0                          },
  { "ca", 0,    "valencia", "Catalan (valencia)" , 0                          },
  { "ca", 0,    0, "Catalan"                     , 0                          },
  { "cmn", 0,    0, "Mandarin"                   , 0                          },
  { "co", 0,    0, "Corsican"                    , "corsu"                    },
  { "cs", 0,    0, "Czech"                       , "Čeština"                  },
  { "cs", "CZ", 0, "Czech (Czech Republic)"      , "Čeština (Česká Republika)"},
  { "cy", 0,    0, "Welsh"                       , "Welsh"                    },
  { "cy", "GB", 0, "Welsh (Great Britain)"       , "Welsh (Great Britain)"    },
  { "cz", 0,    0, "Unknown language"            , "Unknown language"         },
  { "da", 0,    0, "Danish"                      , "Dansk"                    },
  { "da", "DK", 0, "Danish (Denmark)"            , "Dansk (Danmark)"          },
  { "de", 0,    0, "German"                      , "Deutsch"                  },
  { "de", "AT", 0, "German (Austria)"            , "Deutsch (Österreich)"     },
  { "de", "CH", 0, "German (Switzerland)"        , "Deutsch (Schweiz)"        },
  { "de", "DE", 0, "German (Germany)"            , "Deutsch (Deutschland)"    },
  { "dk", 0,    0, "Unknown language"            , "Unknown language"         },
  { "dz", 0,    0, "Dzongkha"                    , "རྫོང་ཁ"                      },
  { "el", 0,    0, "Greek"                       , "ελληνικά"                 },
  { "el", "GR", 0, "Greek (Greece)"              , 0                          },
  { "en", 0,    0, "English"                     , "English"                  },
  { "en", "AU", 0, "English (Australia)"         , "English (Australia)"      },
  { "en", "CA", 0, "English (Canada)"            , "English (Canada)"         },
  { "en", "GB", 0, "English (Great Britain)"     , "English (Great Britain)"  },
  { "en", "US", 0, "English (United States)"     , "English (United States)"  },
  { "en", "ZA", 0, "English (South Africa)"      , "English (South Africa)"   },
  { "en", 0, "boldquot", "English"               , "English"                  },
  { "en", 0, "quot", "English"                   , "English"                  },
  { "en", "US", "piglatin", "English"            , "English"                  },
  { "eo", 0,    0, "Esperanto"                   , "Esperanto"                },
  { "es", 0,    0, "Spanish"                     , "Español"                  },
  { "es", "AR", 0, "Spanish (Argentina)"         , 0                          },
  { "es", "CL", 0, "Spanish (Chile)"             , 0                          },
  { "es", "CO", 0, "Spanish (Colombia)"          , 0                          },
  { "es", "CR", 0, "Spanish (Costa Rica)"        , 0                          },
  { "es", "DO", 0, "Spanish (Dominican Republic)", 0                          },
  { "es", "EC", 0, "Spanish (Ecuador)"           , 0                          },
  { "es", "ES", 0, "Spanish (Spain)"             , 0                          },
  { "es", "GT", 0, "Spanish (Guatemala)"         , 0                          },
  { "es", "HN", 0, "Spanish (Honduras)"          , 0                          },
  { "es", "LA", 0, "Spanish (Laos)"              , 0                          },
  { "es", "MX", 0, "Spanish (Mexico)"            , 0                          },
  { "es", "NI", 0, "Spanish (Nicaragua)"         , 0                          },
  { "es", "PA", 0, "Spanish (Panama)"            , 0                          },
  { "es", "PE", 0, "Spanish (Peru)"              , 0                          },
  { "es", "PR", 0, "Spanish (Puerto Rico)"       , 0                          },
  { "es", "SV", 0, "Spanish (El Salvador)"       , 0                          },
  { "es", "UY", 0, "Spanish (Uruguay)"           , 0                          },
  { "es", "VE", 0, "Spanish (Venezuela)"         , 0                          },
  { "et", 0,    0, "Estonian"                    , "eesti keel"               },
  { "et", "EE", 0, "Estonian (Estonia)"          , 0                          },
  { "et", "ET", 0, "Estonian (Ethiopia)"         , 0                          },
  { "eu", 0,    0, "Basque"                      , "euskara"                  },
  { "eu", "ES", 0, "Basque (Spain)"              , 0                          },
  { "fa", 0,    0, "Persian"                     , "فارسى"                    },
  { "fa", "AF", 0, "Persian (Afghanistan)"       , 0                          },
  { "fa", "IR", 0, "Persian (Iran)"              , 0                          },
  { "fi", 0,    0, "Finnish"                     , "suomi"                    },
  { "fi", "FI", 0, "Finnish (Finland)"           , 0                          },
  { "fo", 0,    0, "Faroese"                     , "Føroyskt"                 },
  { "fo", "FO", 0, "Faeroese (Faroe Islands)"    , 0                          },
  { "fr", 0,    0, "French"                      , "Français"                 },
  { "fr", "CA", 0, "French (Canada)"             , "Français (Canada)"        },
  { "fr", "CH", 0, "French (Switzerland)"        , "Français (Suisse)"        },
  { "fr", "FR", 0, "French (France)"             , "Français (France)"        },
  { "fr", "LU", 0, "French (Luxembourg)"         , "Français (Luxembourg)"    },
  { "fy", 0,    0, "Frisian"                     , "Frysk"                    },
  { "ga", 0,    0, "Irish"                       , "Gaeilge"                  },
  { "gd", 0,    0, "Gaelic Scots"                , "Gàidhlig"                 },
  { "gl", 0,    0, "Galician"                    , "Galego"                   },
  { "gl", "ES", 0, "Galician (Spain)"            , 0                          },
  { "gn", 0,    0, "Guarani"                     , "Avañe'ẽ"                  },
  { "gu", 0,    0, "Gujarati"                    , "ગુજરાતી"                    },
  { "gv", 0,    0, "Manx"                        , "Gaelg"                    },
  { "ha", 0,    0, "Hausa"                       , "حَوْسَ"                   },
  { "he", 0,    0, "Hebrew"                      , "עברית"                     },
  { "he", "IL", 0, "Hebrew (Israel)"             , 0                          },
  { "hi", 0,    0, "Hindi"                       , "हिन्दी"                      },
  { "hr", 0,    0, "Croatian"                    , "Hrvatski"                 },
  { "hr", "HR", 0, "Croatian (Croatia)"          , 0                          },
  { "hu", 0,    0, "Hungarian"                   , "magyar"                   },
  { "hu", "HU", 0, "Hungarian (Hungary)"         , 0                          },
  { "hy", 0,    0, "Armenian"                    , "Հայերեն"                   },
  { "ia", 0,    0, "Interlingua"                 , "Interlingua"              },
  { "id", 0,    0, "Indonesian"                  , "Bahasa Indonesia"         },
  { "id", "ID", 0, "Indonesian (Indonesia)"      , 0                          },
  { "is", 0,    0, "Icelandic"                   , "Íslenska"                 },
  { "is", "IS", 0, "Icelandic (Iceland)"         , 0                          },
  { "it", 0,    0, "Italian"                     , "Italiano"                 },
  { "it", "CH", 0, "Italian (Switzerland)"       , 0                          },
  { "it", "IT", 0, "Italian (Italy)"             , 0                          },
  { "iu", 0,    0, "Inuktitut"                   , "ᐃᓄᒃᑎᑐᑦ/inuktitut"         },
  { "ja", 0,    0, "Japanese"                    , "日本語"                    },
  { "ja", "JP", 0, "Japanese (Japan)"            , 0                          },
  { "ka", 0,    0, "Georgian"                    , "ქართული"                  },
  { "kk", 0,    0, "Kazakh"                      , "Қазақша"                  },
  { "kl", 0,    0, "Kalaallisut"                 , "Kalaallisut"              },
  { "km", 0,    0, "Khmer"                       , "ភាសាខ្មែរ"                   },
  { "km", "KH", 0, "Khmer (Cambodia)"            , 0                          },
  { "kn", 0,    0, "Kannada"                     , "ಕನ್ನಡ"                      },
  { "ko", 0,    0, "Korean"                      , "한국어"                     },
  { "ko", "KR", 0, "Korean (Korea)"              , 0                          },
  { "ku", 0,    0, "Kurdish"                     , "Kurdî"                    },
  { "kw", 0,    0, "Cornish"                     , "Kernowek"                 },
  { "ky", 0,    0, "Kirghiz"                     , 0                          },
  { "la", 0,    0, "Latin"                       , "Latina"                   },
  { "lo", 0,    0, "Lao"                         , "ລາວ"                       },
  { "lt", 0,    0, "Lithuanian"                  , "Lietuvių"                 },
  { "lt", "LT", 0, "Lithuanian (Lithuania)"      , 0                          },
  { "lv", 0,    0, "Latvian"                     , "Latviešu"                 },
  { "lv", "LV", 0, "Latvian (Latvia)"            , 0                          },
  { "jbo", 0,    0, "Lojban"                     , "La .lojban."              },
  { "mg", 0,    0, "Malagasy"                    , "Malagasy"                 },
  { "mi", 0,    0, "Maori"                       , "Māori"                    },
  { "mk", 0,    0, "Macedonian"                  , "Македонски"               },
  { "mk", "MK", 0, "Macedonian (Macedonia)"      , 0                          },
  { "ml", 0,    0, "Malayalam"                   , "മലയാളം"                   },
  { "mn", 0,    0, "Mongolian"                   , "Монгол"                   },
  { "mr", 0,    0, "Marathi"                     , "मराठी"                      },
  { "ms", 0,    0, "Malay"                       , "Bahasa Melayu"            },
  { "ms", "MY", 0, "Malay (Malaysia)"            , 0                          },
  { "mt", 0,    0, "Maltese"                     , "Malti"                    },
  { "my", 0,    0, "Burmese"                     , "မြန်မာဘာသာ"                },
  { "my", "MM", 0, "Burmese (Myanmar)"           , 0                          },
  { "nb", 0,    0, "Norwegian Bokmal"            , 0                          },
  { "nb", "NO", 0, "Norwegian Bokmål (Norway)"   , 0                          },
  { "ne", 0,    0, "Nepali"                      , 0                          },
  { "nl", 0,    0, "Dutch"                       , "Nederlands"               },
  { "nl", "BE", 0, "Dutch (Belgium)"             , 0                          },
  { "nl", "NL", 0, "Dutch (Netherlands)"         , 0                          },
  { "nn", 0,    0, "Norwegian Nynorsk"           , "Norsk nynorsk"            },
  { "nn", "NO", 0, "Norwegian Nynorsk (Norway)"  , 0                          },
  { "no", 0,    0, "Norwegian"                   , "Norsk bokmål"             },
  { "no", "NO", 0, "Norwegian (Norway)"          , 0                          },
  { "no", "NY", 0, "Norwegian (NY)"              , 0                          },
  { "nr", 0,    0, "Ndebele, South"              , 0                          },
  { "oc", 0,    0, "Occitan post 1500"           , "Occitan"                  },
  { "om", 0,    0, "Oromo"                       , "Oromoo"                   },
  { "or", 0,    0, "Oriya"                       , "ଓଡ଼ିଆ"                     },
  { "pa", 0,    0, "Punjabi"                     , "ਪੰਜਾਬੀ"                     },
  { "pl", 0,    0, "Polish"                      , "Polski"                   },
  { "pl", "PL", 0, "Polish (Poland)"             , 0                          },
  { "ps", 0,    0, "Pashto"                      , "پښتو"                     },
  { "pt", 0,    0, "Portuguese"                  , "Português"                },
  { "pt", "BR", 0, "Portuguese (Brazil)"         , 0                          },
  { "pt", "PT", 0, "Portuguese (Portugal)"       , 0                          },
  { "qu", 0,    0, "Quechua"                     , "Runa Simi"                },
  { "rm", 0,    0, "Rhaeto-Romance"              , "Rumantsch"                },
  { "ro", 0,    0, "Romanian"                    , "Română"                   },
  { "ro", "RO", 0, "Romanian (Romania)"          , 0                          },
  { "ru", 0,    0, "Russian"                     ,"Русский"                   },
  { "ru", "RU", 0, "Russian (Russia"             , 0                          },
  { "rw", 0,    0, "Kinyarwanda"                 , "Kinyarwanda"              },
  { "sa", 0,    0, "Sanskrit"                    , 0                          },
  { "sd", 0,    0, "Sindhi"                      , 0                          },
  { "se", 0,    0, "Sami"                        , "Sámegiella"               },
  { "se", "NO", 0, "Sami (Norway)"               , 0                          },
  { "si", 0,    0, "Sinhalese"                   , 0                          },
  { "sk", 0,    0, "Slovak"                      , "Slovenčina"               },
  { "sk", "SK", 0, "Slovak (Slovakia)"           , 0                          },
  { "sl", 0,    0, "Slovenian"                   , "Slovenščina"              },
  { "sl", "SI", 0, "Slovenian (Slovenia)"        , 0                          },
  { "sl", "SL", 0, "Slovenian (Sierra Leone)"    , 0                          },
  { "sm", 0,    0, "Samoan"                      , 0                          },
  { "so", 0,    0, "Somali"                      , 0                          },
  { "sp", 0,    0, "Unknown language"            , 0                          },
  { "sq", 0,    0, "Albanian"                    , "Shqip"                    },
  { "sq", "AL", 0, "Albanian (Albania)"          , 0                          },
  { "sr", 0,    0, "Serbian"                     , "Српски / srpski"          },
  { "sr", "YU", 0, "Serbian (Yugoslavia)"        , 0                          },
  { "sr", 0,"ije", "Serbian"                     , 0                          },
  { "sr", 0, "latin", "Serbian"                  , 0                          },
  { "sr", 0, "Latn",  "Serbian"                  , 0                          },
  { "ss", 0,    0, "Swati"                       , 0                          },
  { "st", 0,    0, "Sotho"                       , 0                          },
  { "sv", 0,    0, "Swedish"                     , "Svenska"                  },
  { "sv", "SE", 0, "Swedish (Sweden)"            , 0                          },
  { "sv", "SV", 0, "Swedish (El Salvador)"       , 0                          },
  { "sw", 0,    0, "Swahili"                     , 0                          },
  { "ta", 0,    0, "Tamil"                       , 0                          },
  { "te", 0,    0, "Telugu"                      , 0                          },
  { "tg", 0,    0, "Tajik"                       , 0                          },
  { "th", 0,    0, "Thai"                        , "ไทย"                      },
  { "th", "TH", 0, "Thai (Thailand)"             , 0                          },
  { "ti", 0,    0, "Tigrinya"                    , 0                          },
  { "tk", 0,    0, "Turkmen"                     , 0                          },
  { "tl", 0,    0, "Tagalog"                     , 0                          },
  { "to", 0,    0, "Tonga"                       , 0                          },
  { "tr", 0,    0, "Turkish"                     , "Türkçe"                   },
  { "tr", "TR", 0, "Turkish (Turkey)"            , 0                          },
  { "ts", 0,    0, "Tsonga"                      , 0                          },
  { "tt", 0,    0, "Tatar"                       , 0                          },
  { "ug", 0,    0, "Uighur"                      , 0                          },
  { "uk", 0,    0, "Ukrainian"                   , "Українська"               },
  { "uk", "UA", 0, "Ukrainian (Ukraine)"         , 0                          },
  { "ur", 0,    0, "Urdu"                        , 0                          },
  { "ur", "PK", 0, "Urdu (Pakistan)"             , 0                          },
  { "uz", 0,    0, "Uzbek"                       , 0                          },
  { "uz", 0, "cyrillic", "Uzbek"                 , 0                          },
  { "vi", 0,    0, "Vietnamese"                  , "Tiếng Việt"               },
  { "vi", "VN", 0, "Vietnamese (Vietnam)"        , 0                          },
  { "wa", 0,    0, "Walloon"                     , 0                          },
  { "wo", 0,    0, "Wolof"                       , 0                          },
  { "xh", 0,    0, "Xhosa"                       , 0                          },
  { "yi", 0,    0, "Yiddish"                     , "ייִדיש"                      },
  { "yo", 0,    0, "Yoruba"                      , 0                          },
  { "zh", 0,    0, "Chinese"                     , "中文"                      },
  { "zh", "CN", 0, "Chinese (simplified)"        , "中文（简体）"               },
  { "zh", "HK", 0, "Chinese (Hong Kong)"         , "中文（香港）"               },
  { "zh", "TW", 0, "Chinese (traditional)"       , "中文（繁體）"               },
  { "zu", 0,    0, "Zulu"                        , 0                          },
  { NULL, 0,    0, NULL                          , 0                          }
};
//*}

std::string
resolve_language_alias(const std::string& name)
{
  typedef std::map<std::string, std::string> Aliases;
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

Language
Language::from_spec(const std::string& language, const std::string& country, const std::string& modifier)
{
  static std::map<std::string, std::vector<const LanguageSpec*> > language_map;

  if (language_map.empty())
  { // Init language_map
    for(int i = 0; languages[i].language != NULL; ++i)
      language_map[languages[i].language].push_back(&languages[i]);
  }

  std::map<std::string, std::vector<const LanguageSpec*> >::iterator i = language_map.find(language);
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
    country = env.substr(ln+1, (std::min(dt, at) == std::string::npos) 
                            ? std::string::npos : std::min(dt, at) - (ln+1));
    country = StringUtils::toUpperCase(country);
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
Language::get_localized_name() const
{
  if(language_spec && language_spec->name_localized)
    return language_spec->name_localized;
  else
    return this->get_name();
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
Language::operator==(const Language& rhs)
{
  return language_spec == rhs.language_spec;
}

bool
Language::operator!=(const Language& rhs)
{
  return language_spec != rhs.language_spec;
}

} // namespace tinygettext

/* EOF */
