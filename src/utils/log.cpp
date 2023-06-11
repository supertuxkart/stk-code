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
#include "network/network_config.hpp"
#include "utils/file_utils.hpp"
#include "utils/tls.hpp"

#include <cstdio>
#include <ctime>
#include <stdio.h>

#ifdef ANDROID
#  include <android/log.h>
#endif

#ifdef IOS_STK
#include "../../../lib/irrlicht/source/Irrlicht/CIrrDeviceiOS.h"
#endif

#ifdef WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

Log::LogLevel Log::m_min_log_level = Log::LL_VERBOSE;
bool          Log::m_no_colors     = false;
FILE*         Log::m_file_stdout   = NULL;
size_t        Log::m_buffer_size = 1;
bool          Log::m_console_log = true;
Synchronised<std::vector<struct Log::LineInfo> > Log::m_line_buffer;
thread_local  char g_prefix[11] = {};

// ----------------------------------------------------------------------------
void Log::setPrefix(const char* prefix)
{
    size_t len = strlen(prefix);
    if (len > 10)
        len = 10;
    if (len != 0)
        memcpy(g_prefix, prefix, len);
    g_prefix[len] = 0;
}   // setPrefix

// ----------------------------------------------------------------------------
/** Selects background/foreground colors for the message depending on
 *  log level. It is only called if messages are not redirected to a file.
 *  \param level The level for which to set the color.
 */
void Log::setTerminalColor(LogLevel level)
{
    if (!m_console_log) return;

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
    if (!m_console_log) return;

    if(m_no_colors) return;

#ifdef WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
                            /*TERM_BLACK*/0 << 4 | /*TERM_LIGHTGRAY*/7);
#else
    printf("%c[0;;m", 0x1B);
#endif
}   // resetTerminalColor

// ----------------------------------------------------------------------------
/** This actually creates a log message. If the messages are to be buffered,
 *  it will be appended to the output buffer. If the buffer is full, it will
 *  be flushed. If the message is not to be buffered, it will be immediately
 *  written using writeLine().

 *  \param level Log level of the message to print.
 *  \param format A printf-like format string.
 *  \param va_list The values to be printed for the format.
 */
void Log::printMessage(int level, const char *component, const char *format,
                       VALIST args)
{
    assert(level >= 0 && level <= LL_FATAL);

    if (level < m_min_log_level) return;

    static const char *names[] = { "debug", "verbose  ", "info   ",
                                  "warn   ", "error  ", "fatal  " };
    const int MAX_LENGTH = 4096;
    char line[MAX_LENGTH + 1];
    int index = 0;
    int remaining = MAX_LENGTH;

    if (strlen(g_prefix) != 0)
    {
        index += snprintf(line+index, remaining, "%s ", g_prefix);
        remaining = MAX_LENGTH - index > 0 ? MAX_LENGTH - index : 0;
    }

#ifdef MOBILE_STK
    // Mobile STK already has timestamp logging in console
    std::string server_prefix = "Server";
#else
    std::string server_prefix = StkTime::getLogTime();
#endif
    if (NetworkConfig::get()->isNetworking() &&
        NetworkConfig::get()->isServer())
    {
        index += snprintf (line + index, remaining,
            "%s [%s] %s: ", server_prefix.c_str(),
            names[level], component);
    }
    else
    {
        index += snprintf (line + index, remaining,
            "[%s] %s: ", names[level], component);
    }
    remaining = MAX_LENGTH - index > 0 ? MAX_LENGTH - index : 0;
    index += vsnprintf(line + index, remaining, format, args);
    remaining = MAX_LENGTH - index > 0 ? MAX_LENGTH - index : 0;
    va_end(args);

    index = index > MAX_LENGTH - 1 ? MAX_LENGTH - 1 : index;
    sprintf(line + index, "\n");

    // If the data is not buffered, immediately print it:
    if (m_buffer_size <= 1)
    {
        writeLine(line, level);
        return;
    }

    // Now the data needs to be buffered. Add an entry to the buffer,
    // and if necessary flush the buffers.
    struct LineInfo li;
    li.m_level = level;
    li.m_line  = std::string(line);
    m_line_buffer.lock();
    m_line_buffer.getData().push_back(li);
    if (m_line_buffer.getData().size() < m_buffer_size)
    {
        // Buffer not yet full, don't flush data.
        m_line_buffer.unlock();
        return;
    }
    m_line_buffer.unlock();
    // Because of the unlock above it can happen that another thread adds
    // another line and calls flushBuffers() first before this thread can
    // call it, but that doesn't really matter, when this thread will finally
    // call flushBuffer() it will then just print nothing if another thread
    // had been actively flushing it in between.
    flushBuffers();
}   // printMessage

// ----------------------------------------------------------------------------
/** Writes the specified line to the various output devices, e.g. terminal,
 *  log file etc. If log messages are not redirected to a file, it tries to
 *  select a terminal colour.
 *  \param line The line to write.
 *  \param level Message level. Only used to select terminal colour.
 */
void Log::writeLine(const char *line, int level)
{

    // If we don't have a console file, write to stdout and hope for the best
    if (m_buffer_size <= 1 || !m_file_stdout)
    {
        setTerminalColor((LogLevel)level);
        if (m_console_log)
        {
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
            case LL_WARN:    alp = ANDROID_LOG_WARN;    break;
            case LL_ERROR:   alp = ANDROID_LOG_ERROR;   break;
            case LL_FATAL:   alp = ANDROID_LOG_FATAL;   break;
            default:         alp = ANDROID_LOG_FATAL;
            }
            __android_log_print(alp, "SuperTuxKart", "%s", line);
#elif defined(IOS_STK)
            CIrrDeviceiOS::debugPrint(line);
#else
            printf("%s", line);
            fflush(stdout);
#endif
        }
        resetTerminalColor();  // this prints a \n
    }

#if defined(_MSC_FULL_VER) && defined(_DEBUG)
    // We don't use utf8ToWide for performance, and debug message
    // is mostly english anyway
    if (m_buffer_size <= 1) OutputDebugStringA(line);
#endif

    if (m_file_stdout) fprintf(m_file_stdout, "%s", line);

#ifdef WIN32
    if (level >= LL_FATAL)
    {
        MessageBoxA(NULL, line, "SuperTuxKart - Fatal error", MB_OK);
    }
#endif
}   // _fluhBuffers

// ----------------------------------------------------------------------------
void Log::toggleConsoleLog(bool val)
{
    m_console_log = val;
}   // toggleConsoleLog

// ----------------------------------------------------------------------------
/** Flushes all stored log messages to the various output devices (thread safe).
 */
void Log::flushBuffers()
{
    m_line_buffer.lock();
    for (unsigned int i = 0; i < m_line_buffer.getData().size(); i++)
    {
        const LineInfo &li = m_line_buffer.getData()[i];
        writeLine(li.m_line.c_str(), li.m_level);
    }
    m_line_buffer.getData().clear();
    m_line_buffer.unlock();
}   // flushBuffers

// ----------------------------------------------------------------------------
/** This function opens the files that will contain the output.
 *  \param logout : name of the file that will contain stdout output
 *  \param logerr : name of the file that will contain stderr output
 */
void Log::openOutputFiles(const std::string &logout)
{
    m_file_stdout = FileUtils::fopenU8Path(logout, "w");
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

