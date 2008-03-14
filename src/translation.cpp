#include "translation.hpp"

initTranslations::initTranslations() { 
#ifdef HAS_GETTEXT
    // LC_ALL does not work, sscanf will then not always be able
    // to scan for example: s=-1.1,-2.3,-3.3 correctly, which is
    // used in driveline files.
    setlocale (LC_CTYPE,    "");
//    setlocale (LC_MESSAGES, "");
    bindtextdomain (PACKAGE, "./data/po");
    //bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
#endif
}

