
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef WIN32
#ifdef __CYGWIN__
#include <unistd.h>
#endif
#include <windows.h>
#ifdef _MSC_VER
#include <io.h>
#include <direct.h>
#endif
#else
#include <unistd.h>
#endif
#include <math.h>
#include <signal.h>

#include <plib/pw.h>
#include <plib/ssg.h>
#include <plib/sl.h>
#include <plib/js.h>
#include <plib/fnt.h>
#include <plib/pu.h>

