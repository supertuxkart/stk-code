//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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

#include "utils/log.hpp"

#include "config/user_config.hpp"

#include <cstdio>
#include <stdio.h>

#ifdef ANDROID
#  include <android/log.h>
#endif

#ifdef WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

Log::LogLevel Log::m_min_log_level = Log::LL_VERBOSE;
bool          Log::m_no_colors     = false;
FILE*         Log::m_file_stdout   = NULL;

// ----------------------------------------------------------------------------
/** Selects background/foreground colors for the message depending on
 *  log level. It is only called if messages are not redirected to a file.
 *  \param level The level for which to set the color.
 */
void Log::setTerminalColor(LogLevel level)
{
    if(m_no_colors) return;

    // Thanks to funto for the colouring code!
#ifdef WIN32
    enum TermColor
    {
        TERM_BLACK,    TERM_BLUE,         TERM_GREEN,      TERM_CYAN,
        TERM_RED,      TERM_MAGENTA,      TERM_BROWN,      TERM_LIGHTGRAY,
        TERM_DARKGRAY, TERM_LIGHTBLUE,    TERM_LIGHTGREEN, TERM_LIGHTCYAN,
        TERM_LIGHTRED, TERM_LIGHTMAGENTA, TERM_YELLOW,     TERM_WHITE
    };
    char color;
    switch(level)
    {
    default:
    case LL_VERBOSE: color = TERM_BLACK     << 4 | TERM_LIGHTGRAY; break;
    case LL_DEBUG:   color = TERM_BLACK     << 4 | TERM_LIGHTGRAY; break;
    case LL_INFO:    color = TERM_BLACK     << 4 | TERM_LIGHTGRAY; break;
    case LL_WARN :   color = TERM_LIGHTGRAY << 4 | TERM_LIGHTRED;  break;
    case LL_ERROR:   color = TERM_LIGHTGRAY << 4 | TERM_RED;       break;
    case LL_FATAL:   color = TERM_RED       << 4 | TERM_WHITE;     break;
    }   // switch
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),color);
#else
    enum TermAttr
    {
        TERM_RESET = 0, // "normal" mode
        TERM_BRIGHT = 1,// more luminosity for the foreground
        TERM_DIM = 2,   // less luminosity for the foreground
    };

    enum TermColor
    {
        TERM_BLACK=0, TERM_RED,     TERM_GREEN, TERM_YELLOW,
        TERM_BLUE,    TERM_MAGENTA, TERM_CYAN,  TERM_WHITE
    };
    int attr = TERM_BRIGHT, front_color=-1;
    switch(level)
    {
    case LL_VERBOSE: attr = TERM_DIM;   front_color=TERM_WHITE; break;
    case LL_DEBUG:   attr = TERM_DIM;   front_color=TERM_WHITE; break;
    case LL_INFO:    attr = TERM_RESET; break;
    case LL_WARN:    attr = TERM_DIM;    front_color=TERM_RED; break;
    case LL_ERROR:   attr = TERM_BRIGHT; front_color=TERM_RED; break;
    case LL_FATAL:   attr = TERM_BRIGHT; front_color=TERM_RED; break;

    }
    if(attr==TERM_RESET)
        printf("%c[%dm", 0x1B,attr);  // not necessary, but
    else
    {
        printf("%c[%d;%dm", 0x1B,attr, front_color+30);
    }
#endif
}   // setTerminalColor

// ----------------------------------------------------------------------------
/** Resets the terminal color to the default, and adds a new line (if a new
 *  line is added as part of the message, a potential change of background
 *  color will also affect the next row).
 */
void Log::resetTerminalColor()
{
    if(m_no_colors)
    {
        printf("\n");
        return;
    }


#ifdef WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                            /*TERM_BLACK*/0 << 4 | /*TERM_LIGHTGRAY*/7);
    printf("\n");
#else
    printf("%c[0;;m\n", 0x1B);
#endif
}   // resetTerminalColor

// ----------------------------------------------------------------------------
/** This actually prints the log message. If log messages are not redirected
 *  to a file, it tries to select a terminal colour.
 *  \param level Log level of the message to print.
 *  \param format A printf-like format string.
 *  \param va_list The values to be printed for the format.
 */
void Log::printMessage(int level, const char *component, const char *format,
                       VALIST args)
{
    assert(level>=0 && level <=LL_FATAL);

    if(level<m_min_log_level) return;

#ifdef ANDROID
    android_LogPriority alp;
    switch (level)
    {
    // STK is using the levels slightly different from android
    // (debug lowest, verbose above it; while android reverses
    // this order. So to get the same behaviour (e.g. filter
    // out debug message, but still get verbose, we swap
    // the order here.
    case LL_VERBOSE: alp = ANDROID_LOG_DEBUG;   break;
    case LL_DEBUG:   alp = ANDROID_LOG_VERBOSE; break;
    case LL_INFO:    alp = ANDROID_LOG_INFO;    break;
    case LL_WARN :   alp = ANDROID_LOG_WARN;    break;
    case LL_ERROR:   alp = ANDROID_LOG_ERROR;   break;
    case LL_FATAL:   alp = ANDROID_LOG_FATAL;   break;
    default:         alp = ANDROID_LOG_FATAL;
    }
#endif

    static const char *names[] = {"debug", "verbose  ", "info   ",
                                  "warn   ", "error  ", "fatal  "};

    // Using a va_list twice produces undefined results, ie crash.
    // So make a copy if we're going to use it twice.
    VALIST copy;
    if (m_file_stdout)
    {
        va_copy(copy, args);
    }
#if defined(_MSC_FULL_VER) && defined(_DEBUG)
    VALIST copy2;
    va_copy(copy2, args);
#endif

    // If we don't have a console file, write to stdout and hope for the best
    if(!m_file_stdout || level >= LL_WARN ||
        UserConfigParams::m_log_errors_to_console) // log to console & file
    {
        VALIST out;
        va_copy(out, args);

        setTerminalColor((LogLevel)level);
        #ifdef ANDROID
        __android_log_vprint(alp, "SuperTuxKart", format, out);
        #else
        printf("[%s] %s: ", names[level], component);
        vprintf(format, out);
        #endif
        resetTerminalColor();  // this prints a \n

        va_end(out);
    }

#if defined(_MSC_FULL_VER) && defined(_DEBUG)
    static char szBuff[2048];
    vsnprintf(szBuff, sizeof(szBuff), format, copy2);

    OutputDebugString("[");
    OutputDebugString(names[level]);
    OutputDebugString("] ");
    OutputDebugString(component);
    OutputDebugString(": ");
    OutputDebugString(szBuff);
    OutputDebugString("\r\n");
#endif


    if(m_file_stdout)
    {
        fprintf (m_file_stdout, "[%s] %s: ", names[level], component);
        vfprintf(m_file_stdout, format, copy);
        fprintf (m_file_stdout, "\n");
        va_end(copy);
    }

#ifdef WIN32
    if (level >= LL_FATAL)
    {
        std::string message;

        char tmp[2048];
        sprintf(tmp, "[%s] %s: ", names[level], component);
        message += tmp;

        VALIST out;
        va_copy(out, args);
        vsprintf(tmp, format, out);
        message += tmp;
        va_end(out);

        MessageBoxA(NULL, message.c_str(), "SuperTuxKart - Fatal error", MB_OK);
    }
#endif
}   // printMessage


// ----------------------------------------------------------------------------
/** This function opens the files that will contain the output.
 *  \param logout : name of the file that will contain stdout output
 *  \param logerr : name of the file that will contain stderr output
 */
void Log::openOutputFiles(const std::string &logout)
{
    m_file_stdout = fopen(logout.c_str(), "w");
    if (!m_file_stdout)
    {
        Log::error("main", "Can not open log file '%s'. Writing to "
                           "stdout instead.", logout.c_str());
    }
    else
    {
        // Disable buffering so that messages are seen asap
        setvbuf(m_file_stdout, NULL, _IONBF, 0);
    }
} // closeOutputFiles

// ----------------------------------------------------------------------------
/** Function to close output files */
void Log::closeOutputFiles()
{
    fclose(m_file_stdout);
} // closeOutputFiles

