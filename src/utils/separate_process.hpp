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
#include <thread>
#include <vector>

class SeparateProcess
{
private:
#if defined(WIN32)
    // Various handles for the window pipes
    HANDLE m_child_stdin_read;
    HANDLE m_child_stdin_write;
    HANDLE m_child_stdout_read;
    HANDLE m_child_stdout_write;
    HANDLE m_child_handle;
    DWORD m_child_pid;
#elif defined(ANDROID)
    void* m_child_handle;
    void (*m_child_abort_proc)();
    std::thread m_child_thread;
    std::vector<char*> m_child_args;
#else
    int m_child_stdin_write = -1;
    int m_child_stdout_read = -1;
    int m_child_pid = -1;
#endif

    // ------------------------------------------------------------------------
    bool createChildProcess(const std::string& exe,
                            const std::string& argument, bool create_pipe,
                            const std::string& childprocess_name);
    // ------------------------------------------------------------------------
    std::string getLine();

public:
    // ------------------------------------------------------------------------
    static std::string getCurrentExecutableLocation();
    // ------------------------------------------------------------------------
     SeparateProcess(const std::string& exe, const std::string& argument,
                     bool create_pipe = false,
                     const std::string& childprocess_name = "childprocess");
    // ------------------------------------------------------------------------
    ~SeparateProcess();
    // ------------------------------------------------------------------------
    std::string sendCommand(const std::string &command);
    // ------------------------------------------------------------------------
    std::string decodeString(const std::string &s);

};   // class SeparateProcess

#endif // HEADER_SEPERATE_PROCESS_HPP
