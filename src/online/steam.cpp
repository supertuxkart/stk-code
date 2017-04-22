//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2017 Joerg Henrichs
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

#include "online/steam.hpp"

#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#ifdef WIN32
#  include <windows.h> 
#endif

Steam::Steam()
{
#ifdef WIN32
    // Based on: https://msdn.microsoft.com/en-us/library/windows/desktop/ms682499(v=vs.85).aspx
    SECURITY_ATTRIBUTES sec_attr;

    // Set the bInheritHandle flag so pipe handles are inherited. 
    sec_attr.nLength = sizeof(SECURITY_ATTRIBUTES);
    sec_attr.bInheritHandle = TRUE;
    sec_attr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT. 

    if (!CreatePipe(&m_child_stdout_read, &m_child_stdout_write, &sec_attr, 0))
    {
        Log::error("Steam", "Error creating StdoutRd CreatePipe");
    }

    // Ensure the read handle to the pipe for STDOUT is not inherited.

    if (!SetHandleInformation(m_child_stdout_read, HANDLE_FLAG_INHERIT, 0))
    {
        Log::error("Steam", "Stdout SetHandleInformation");
    }

    // Create a pipe for the child process's STDIN. 
    if (!CreatePipe(&m_child_stdin_read, &m_child_stdin_write, &sec_attr, 0))
    {
        Log::error("Steam", "Stdin CreatePipe");
    }

    // Ensure the write handle to the pipe for STDIN is not inherited. 

    if (!SetHandleInformation(m_child_stdin_write, HANDLE_FLAG_INHERIT, 0))
    {
        Log::error("Steam", "Stdin SetHandleInformation");
    }

    // Create the child process. 

    createChildProcess();
#endif
}   // Steam
// ----------------------------------------------------------------------------
Steam::~Steam()
{

}   // ~Steam

// ----------------------------------------------------------------------------
int Steam::createChildProcess()
{
#ifdef WIN32
    TCHAR command_line[] = TEXT("ssm.exe 1");
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE;

    // Set up members of the PROCESS_INFORMATION structure. 

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    // Set up members of the STARTUPINFO structure. 
    // This structure specifies the STDIN and STDOUT handles for redirection.

    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = m_child_stdout_write;
    siStartInfo.hStdOutput = m_child_stdout_write;
    siStartInfo.hStdInput = m_child_stdin_read;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Create the child process. 

    bSuccess = CreateProcess(NULL,
        command_line,  // command line 
        NULL,          // process security attributes 
        NULL,          // primary thread security attributes 
        TRUE,          // handles are inherited 
        0,             // creation flags 
        NULL,          // use parent's environment 
        NULL,          // use parent's current directory 
        &siStartInfo,  // STARTUPINFO pointer 
        &piProcInfo);  // receives PROCESS_INFORMATION 

                       // If an error occurs, exit the application. 
    if (!bSuccess)
    {
        Log::error("Steam", "CreateProcess");
    }
    else
    {
        // Close handles to the child process and its primary thread.
        // Some applications might keep these handles to monitor the status
        // of the child process, for example. 

        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);
    }
#endif

    return 0;
}   // createChildProcess

// ----------------------------------------------------------------------------
std::string Steam::getLine()
{
#define BUFSIZE 1024
    CHAR buffer[BUFSIZE];
    DWORD bytes_read;
    // Read from pipe that is the standard output for child process. 
    bool success = ReadFile(m_child_stdout_read, buffer, BUFSIZE-1,
                            &bytes_read, NULL)!=0;
    if (success && bytes_read < BUFSIZE)
    {
        buffer[bytes_read] = 0;
        std::string s = buffer;
        return s;
    }
    return std::string("");
}   // getLine

// ----------------------------------------------------------------------------
std::string Steam::sendCommand(const std::string &command)
{
#ifdef WIN32
    // Write to the pipe that is the standard input for a child process. 
    // Data is written to the pipe's buffers, so it is not necessary to wait
    // until the child process is running before writing data.
    DWORD bytes_written;
    bool success = WriteFile(m_child_stdin_write, (command+"\n").c_str(),
                             command.size(), &bytes_written, NULL) != 0;
    return getLine();
#endif

    return std::string("");
}   // sendCommand

// ----------------------------------------------------------------------------
std::string Steam::decodeString(const std::string &s)
{
    std::vector<std::string> l = StringUtils::split(s, ' ');
    if (l.size() != 2) return "INVALID ANSWER - wrong number of fields";

    int n;
    StringUtils::fromString(l[0], n);
    if (n != l[1].size()) return "INVALID ANSWER - incorrect length";

    return l[1];

}   // decodeString

// ----------------------------------------------------------------------------
/** Returns the steam user name. SSM returns 'N name" where N is
 *  the length of the name.
 */
std::string Steam::getName()
{
    std::string s = sendCommand("name");
    return decodeString(s);
}   // getName

// ----------------------------------------------------------------------------
/** Returns a unique id (string) from steam. SSM returns 'N ID" where N is
 *  the length of the ID.
 */
std::string Steam::getId()
{
    std::string s = sendCommand("id");
    return decodeString(s);
}   // getId

// ----------------------------------------------------------------------------
/** Returns a std::vector with the names of all friends. SSM returns a first
 *  line with the number of friends, then one friend in a line.
 */
std::vector<std::string> Steam::getFriends()
{
    std::string s = sendCommand("friends");
    int num_friends;
    StringUtils::fromString(s, num_friends);
    std::vector<std::string> result;
    for (int i = 0; i < num_friends; i++)
    {
        std::string f = getLine();
        result.push_back(decodeString(f));
    }
    return result;
}
// ----------------------------------------------------------------------------
int Steam::saveAvatarAs(const std::string filename)
{
    //std::string s = sendCommand(std::string("avatar ")+filename);
    std::string s = sendCommand("avatar");
    if(s=="filename")
        s=sendCommand(filename);
    return s == "done";

}   // saveAvatarAs
