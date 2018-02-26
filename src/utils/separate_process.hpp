//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 Joerg Henrichs
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

#ifndef HEADER_SEPERATE_PROCESS_HPP
#define HEADER_SEPERATE_PROCESS_HPP

#ifdef WIN32
#  include <windows.h>
#endif

#include <assert.h>
#include <string>

class SeparateProcess
{
private:
    const std::string m_destroy_command;

#ifdef WIN32
    // Various handles for the window pipes
    HANDLE m_child_stdin_read;
    HANDLE m_child_stdin_write;
    HANDLE m_child_stdout_read;
    HANDLE m_child_stdout_write;
#else
    int m_child_stdin_write;
    int m_child_stdout_read;
#endif

    // ------------------------------------------------------------------------
    bool createChildProcess(const std::string& exe,
                            const std::string& argument);
    // ------------------------------------------------------------------------
    std::string getLine();

public:
    // ------------------------------------------------------------------------
    static std::string getCurrentExecutableLocation();
    // ------------------------------------------------------------------------
     SeparateProcess(const std::string& exe, const std::string& argument,
                     const std::string& destroy_command);
    // ------------------------------------------------------------------------
    ~SeparateProcess();
    // ------------------------------------------------------------------------
    std::string sendCommand(const std::string &command);
    // ------------------------------------------------------------------------
    std::string decodeString(const std::string &s);

};   // class SeparateProcess

#endif // HEADER_SEPERATE_PROCESS_HPP
