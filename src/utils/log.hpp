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


#ifndef HEADER_LOG_HPP
#define HEADER_LOG_HPP

#include "utils/synchronised.hpp"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>


#if defined(__GLIBC__)
#  define VALIST __gnuc_va_list
#else
#  define VALIST va_list
#endif

#if defined(_WIN32) && defined(_MSC_VER) && _MSC_VER < 1800
#  define va_copy(dest, src) dest = src
#endif

class Log
{
public:
    /** The various log levels used in STK. */
    enum LogLevel { LL_DEBUG,
                    LL_VERBOSE,
                    LL_INFO,
                    LL_WARN,
                    LL_ERROR,
                    LL_FATAL
    };

private:

    /** Which message level to print. */
    static LogLevel m_min_log_level;

    /** If set this will disable coloring of log messages. */
    static bool     m_no_colors;

    /** If false that logging will only be saved to a file. */
    static bool     m_console_log;

    /** The file where stdout output will be written */
    static FILE* m_file_stdout;

    /** An optional buffer for lines to be output. */
    struct LineInfo
    {
        std::string m_line;
        int m_level;
    };
    static Synchronised<std::vector<struct LineInfo> > m_line_buffer;

    /** <0 if no buffered logging is to be used, otherwise this is
     ** the maximum number of lines the buffer should hold. */
    static size_t m_buffer_size;

    static void setTerminalColor(LogLevel level);
    static void resetTerminalColor();
    static void writeLine(const char *line, int level);

    static void printMessage(int level, const char *component,
                             const char *format, VALIST va_list);

public:

    // ------------------------------------------------------------------------
    /** A simple macro to define the various log functions.
     *  Note that an assert is added so that a debugger is triggered
     *  when debugging. */
#define LOG(NAME, LEVEL)                                             \
    static void NAME(const char *component, const char *format, ...) \
    {                                                                \
        if(LEVEL < m_min_log_level) return;                          \
        va_list args;                                                \
        va_start(args, format);                                      \
        printMessage(LEVEL, component, format, args);                \
        va_end(args);                                                \
                                                                     \
        if (LEVEL == LL_FATAL)                                       \
        {                                                            \
            assert(false);                                           \
            exit(1);                                                 \
        }                                                            \
    }
    LOG(verbose, LL_VERBOSE);
    LOG(debug,   LL_DEBUG);
    LOG(info,    LL_INFO);
    LOG(warn,    LL_WARN);
    LOG(error,   LL_ERROR);
    LOG(fatal,   LL_FATAL);

    static void openOutputFiles(const std::string &logout);

    static void closeOutputFiles();
    static void flushBuffers();
    static void toggleConsoleLog(bool val);

    // ------------------------------------------------------------------------
    /** Sets the number of lines to buffer. Setting the buffer size to a 
     *  a value <=1 means no buffering, lines will be immediately printed. */
    static void setBufferSize(size_t n) { m_buffer_size = n;  }
    // ------------------------------------------------------------------------
    /** Defines the minimum log level to be displayed. */
    static void setLogLevel(int n)
    {
        if(n<0 || n>LL_FATAL)
        {
            warn("Log", "Log level %d not in range [%d-%d] - ignored.\n",
                 n, LL_VERBOSE, LL_FATAL);
            return;
        }
        m_min_log_level = (LogLevel)n;
    }    // setLogLevel

    // ------------------------------------------------------------------------
    /** Returns the log level. This is useful if some work is necessary to
     *  preprate output strings, which might not be used at all (example:
     *  replacing the cleartext password in an http request). */
    static LogLevel getLogLevel() { return m_min_log_level;  }
    // ------------------------------------------------------------------------
    /** Disable coloring of log messages. */
    static void disableColor()
    {
        m_no_colors = true;
    }   // disableColor
    // ------------------------------------------------------------------------
    /** Sets a prefix to be printed before each line. To disable the prefix,
     *  set it to "", max length of prefix is 10, if larger than that the
     *  remaining characters are ignored. */
    static void setPrefix(const char* prefix);
};   // Log
#endif
