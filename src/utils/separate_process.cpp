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

#include "utils/separate_process.hpp"
#include "io/file_manager.hpp"
#include "utils/command_line.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#ifdef __APPLE__
#  include <mach-o/dyld.h>
#endif

#ifdef WIN32
#  include <windows.h>
#else
#  include <iostream>
#  include <unistd.h>
#  include <signal.h>
#  include <sys/wait.h>
#  include <errno.h>
#endif

#ifdef __FreeBSD__
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#ifdef ANDROID
#include <dlfcn.h>
#include <fstream>

#include "main_android.hpp"
#include "io/assets_android.hpp"
#endif

// ----------------------------------------------------------------------------
std::string SeparateProcess::getCurrentExecutableLocation()
{
#ifdef WIN32
    // We already set the full path of exe name in windows
    return CommandLine::getExecName();
#elif defined (__APPLE__)
    char path[1024];
    unsigned buf_size = 1024;
    if (_NSGetExecutablePath(path, &buf_size) == 0)
    {
        return file_manager->getFileSystem()->getAbsolutePath(path).c_str();
    }
    return "";
#elif defined (__FreeBSD__)
    char path[PATH_MAX];
    size_t len = PATH_MAX;
    const int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
    if (sysctl(&mib[0], 4, &path, &len, NULL, 0) == 0)
    {
        return file_manager->getFileSystem()->getAbsolutePath(path).c_str();
    }
    return "";
#else
    // Assume Linux
    return file_manager->getFileSystem()->getAbsolutePath("/proc/self/exe")
        .c_str();
#endif
}   // getCurrentExecutableLocation

// ----------------------------------------------------------------------------
SeparateProcess::SeparateProcess(const std::string& exe,
                                 const std::string& argument, bool create_pipe,
                                 const std::string& childprocess_name)
{
#ifdef ANDROID
    m_child_handle = NULL;
    m_child_abort_proc = NULL;
#endif

    if (!createChildProcess(exe, argument, create_pipe, childprocess_name))
    {
        Log::error("SeparateProcess", "Failed to run %s %s",
            exe.c_str(), argument.c_str());
        return;
    }
}   // SeparateProcess

// ----------------------------------------------------------------------------
SeparateProcess::~SeparateProcess()
{
    bool dead = false;
#if defined(WIN32)
    core::stringw class_name = "separate_process";
    class_name += StringUtils::toWString(m_child_pid);
    HWND hwnd = FindWindowEx(HWND_MESSAGE, NULL, class_name.c_str(), NULL);
    if (hwnd != NULL)
    {
        PostMessage(hwnd, WM_DESTROY, 0, 0);
        if (WaitForSingleObject(m_child_handle, 5000) != WAIT_TIMEOUT)
        {
            dead = true;
        }
    }

    if (!dead)
    {
        Log::info("SeparateProcess", "Timeout waiting for child process to "
            "self-destroying, killing it anyway");
        if (TerminateProcess(m_child_handle, 0) == 0)
            Log::warn("SeparateProcess", "Failed to kill child process.");
    }
    if (CloseHandle(m_child_handle) == 0)
        Log::warn("SeparateProcess", "Failed to close child process handle.");
        
#elif defined(ANDROID)

    if (m_child_handle != NULL)
    {
        Log::info("SeparateProcess", "Closing child process");
        m_child_abort_proc();
        StkTime::sleep(1000);
        
        if (m_child_thread.joinable())
        {
            Log::info("SeparateProcess", "Wait for closing");
            m_child_thread.join();
        }
        
        dlclose(m_child_handle);
        m_child_handle = NULL;
        m_child_abort_proc = NULL;

        for (char* arg : m_child_args)
        {
            delete[] arg;
        }
    }

#else
    if (m_child_stdin_write != -1 && m_child_stdout_read != -1)
    {
        close(m_child_stdin_write);
        close(m_child_stdout_read);
    }
    
    if (m_child_pid != -1)
    {
        kill(m_child_pid, SIGTERM);
        for (int i = 0; i < 5; i++)
        {
            int status;
            if (waitpid(m_child_pid, &status, WNOHANG) == m_child_pid)
            {
                dead = true;
                break;
            }
            StkTime::sleep(1000);
        }
        if (!dead)
        {
            Log::info("SeparateProcess", "Timeout waiting for child process to "
                "self-destroying, killing it anyway");
            kill(m_child_pid, SIGKILL);
        }
    }
#endif
    Log::info("SeparateProcess", "Destroyed");
}   // ~SeparateProcess

// ----------------------------------------------------------------------------
/** Starts separate as a child process
 *  and sets up communication via pipes.
 *  \return True if the child process creation was successful.
 */
#if defined(WIN32)
bool SeparateProcess::createChildProcess(const std::string& exe,
                                         const std::string& argument,
                                         bool create_pipe,
                                         const std::string& childprocess_name)
{
    int error = 0;
    // Based on: https://msdn.microsoft.com/en-us/library/windows/desktop/ms682499(v=vs.85).aspx
    SECURITY_ATTRIBUTES sec_attr;

    // Set the bInheritHandle flag so pipe handles are inherited.
    sec_attr.nLength = sizeof(SECURITY_ATTRIBUTES);
    sec_attr.bInheritHandle       = TRUE;
    sec_attr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT if needed.
    if (create_pipe)
    {
        if (!CreatePipe(&m_child_stdout_read, &m_child_stdout_write, &sec_attr, 0))
        {
            error = GetLastError();
            Log::error("SeparateProcess", "StdoutRd CreatePipe error code: %d",
                error);
            return false;
        }

        // Ensure the read handle to the pipe for STDOUT is not inherited.
        if (!SetHandleInformation(m_child_stdout_read, HANDLE_FLAG_INHERIT, 0))
        {
            error = GetLastError();
            Log::error("SeparateProcess",
                "Stdout SetHandleInformation error code: %d", error);
            return false;
        }

        // Create a pipe for the child process's STDIN.
        if (!CreatePipe(&m_child_stdin_read, &m_child_stdin_write, &sec_attr, 0))
        {
            error = GetLastError();
            Log::error("SeparateProcess", "Stdin CreatePipe error code: %d",
                error);
            return false;
        }

        // Ensure the write handle to the pipe for STDIN is not inherited.
        if (!SetHandleInformation(m_child_stdin_write, HANDLE_FLAG_INHERIT, 0))
        {
            error = GetLastError();
            Log::error("SeparateProcess",
                "Stdin SetHandleInformation error code: %d", error);
            return false;
        }
    }
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;

    // Set up members of the PROCESS_INFORMATION structure.

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    // Set up members of the STARTUPINFO structure.
    // This structure specifies the STDIN and STDOUT handles for redirection.

    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    if (create_pipe)
    {
        siStartInfo.cb = sizeof(STARTUPINFO);
        siStartInfo.hStdError = m_child_stdout_write;
        siStartInfo.hStdOutput = m_child_stdout_write;
        siStartInfo.hStdInput = m_child_stdin_read;
        siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    }

    // Create the child process.
    std::string cmd = " "; // Required for 1st argument
    cmd += argument + " --parent-process=" +
        StringUtils::toString(GetCurrentProcessId());
    core::stringw exe_w = StringUtils::utf8ToWide(exe);
    core::stringw cmd_w = StringUtils::utf8ToWide(cmd);
    bool success = CreateProcess(exe_w.data(), // application name
        cmd_w.data(),          // command line
        NULL,                  // process security attributes
        NULL,                  // primary thread security attributes
        TRUE,                  // handles are inherited
        CREATE_NO_WINDOW,      // creation flags
        NULL,                  // use parent's environment
        NULL,                  // use parent's current directory
        &siStartInfo,          // STARTUPINFO pointer
        &piProcInfo) != 0;     // receives PROCESS_INFORMATION

    if (!success)
    {
        error = GetLastError();
        Log::error("SeparateProcess", "CreateProcess error code: %d", error);
        return false;
    }

    m_child_handle = piProcInfo.hProcess;
    m_child_pid = piProcInfo.dwProcessId;
    if (m_child_pid == 0)
    {
        Log::warn("SeparateProcess", "Invalid child pid.");
        return false;
    }
    if (CloseHandle(piProcInfo.hThread) == 0)
        Log::warn("SeparateProcess", "Failed to close child thread handle.");

    return true;
}   // createChildProcess - windows version

#elif defined(ANDROID)

bool SeparateProcess::createChildProcess(const std::string& exe,
                                         const std::string& argument,
                                         bool create_pipe,
                                         const std::string& childprocess_name)
{
    if (create_pipe)
    {
        Log::error("SeparateProcess", "Error: create_pipe is not supported.");
        return false;
    }
    
    if (m_child_handle != NULL)
    {
        Log::error("SeparateProcess", "Error: Child process is already started");
        return false;
    }
    
    std::string data_path = AssetsAndroid::getDataPath();
    std::string main_path;
    
    if (!data_path.empty())
    {
        main_path = data_path + "/lib/libmain.so";
    }

    if (main_path.empty() || access(main_path.c_str(), R_OK) != 0)
    {
        std::string lib_path = AssetsAndroid::getLibPath();
        
        if (!lib_path.empty())
        {
            main_path = lib_path + "/libmain.so";
        }
        
        Log::info("SeparateProcess", "Trying to use fallback lib path: %s", 
                  main_path.c_str());
    }
        
    if (main_path.empty() || access(main_path.c_str(), R_OK) != 0)
    {
        Log::error("SeparateProcess", "Error: Cannot read libmain.so");
        return false;
    }
    
    Log::info("SeparateProcess", "Data dir found in: %s", data_path.c_str());

    std::string child_path = data_path + "/files/lib" +
        childprocess_name + ".so";

    if (access(child_path.c_str(), R_OK) != 0)
    {
        Log::info("SeparateProcess", "Creating libchildprocess.so");

        std::ifstream src(main_path, std::ios::binary);
        
        if (!src.good())
        {
            Log::error("SeparateProcess", "Error: Cannot open libmain.so");
            return false;
        }
        
        std::ofstream dst(child_path, std::ios::binary);
        
        if (!dst.good())
        {
            Log::error("SeparateProcess", "Error: Cannot copy libmain.so");
            return false;
        }
    
        dst << src.rdbuf();
    }
    
    if (access(child_path.c_str(), R_OK) != 0)
    {
        Log::error("SeparateProcess", "Error: Cannot read libchildprocess.so");
        return false;
    }
    
    m_child_handle = dlopen(child_path.c_str(), RTLD_NOW);
    
    if (m_child_handle == NULL)
    {
        Log::error("SeparateProcess", "Error: Cannot dlopen libchildprocess.so");
        return false;
    }

    typedef void (*main_proc_t)(int, char**);
    main_proc_t main_proc = (main_proc_t)dlsym(m_child_handle, "main");
    
    if (main_proc == NULL)
    {
        Log::error("SeparateProcess", "Error: Cannot get handle to main()");
        dlclose(m_child_handle);
        m_child_handle = NULL;
        return false;
    }
    
    m_child_abort_proc = (void(*)())dlsym(m_child_handle, "main_abort");
    
    if (m_child_abort_proc == NULL)
    {
        Log::error("SeparateProcess", "Error: Cannot get handle to main_abort()");
        dlclose(m_child_handle);
        m_child_handle = NULL;
        return false;
    }

    typedef void (*set_activity_proc_t)(ANativeActivity*);
    set_activity_proc_t set_activity_proc = 
        (set_activity_proc_t)dlsym(m_child_handle, "set_global_android_activity");
    
    if (set_activity_proc == NULL)
    {
        Log::error("SeparateProcess", "Error: Cannot get handle to "
                   "set_global_android_activity()");
        dlclose(m_child_handle);
        m_child_handle = NULL;
        return false;
    }

    set_activity_proc(global_android_activity);
    
    const std::string exe_file = StringUtils::getBasename(exe);
    auto rest_argv = StringUtils::split(argument, ' ');

    char* arg = new char[exe_file.size() + 1]();
    memcpy(arg, exe_file.c_str(), exe_file.size());
    m_child_args.push_back(arg);
    
    for (unsigned i = 0; i < rest_argv.size(); i++)
    {
        char* arg = new char[rest_argv[i].size() + 1]();
        memcpy(arg, rest_argv[i].c_str(), rest_argv[i].size());
        m_child_args.push_back(arg);
    }
    
    Log::info("SeparateProcess", "Starting main()");
    m_child_thread = std::thread(main_proc, m_child_args.size(), &m_child_args[0]);
    
    return true;
}

#else    // linux and osx

bool SeparateProcess::createChildProcess(const std::string& exe,
                                         const std::string& argument,
                                         bool create_pipe,
                                         const std::string& childprocess_name)
{
    const int PIPE_READ=0;
    const int PIPE_WRITE=1;
    int       stdin_pipe[2];
    int       stdout_pipe[2];
    int       child;

    if (create_pipe)
    {
        if (pipe(stdin_pipe) < 0)
        {
            Log::error("SeparateProcess", "Can't allocate pipe for input "
                "redirection.");
            return -1;
        }
        if (pipe(stdout_pipe) < 0)
        {
            close(stdin_pipe[PIPE_READ]);
            close(stdin_pipe[PIPE_WRITE]);
            Log::error("SeparateProcess", "allocating pipe for child output "
                "redirect");
            return false;
        }
    }

    std::string parent_pid = "--parent-process=";
    int current_pid = getpid();
    parent_pid += StringUtils::toString(current_pid);
    child = fork();
    if (child == 0)
    {
        // Child process:
        Log::info("SeparateProcess", "Child process started.");

        if (create_pipe)
        {
            // redirect stdin
            if (dup2(stdin_pipe[PIPE_READ], STDIN_FILENO) == -1)
            {
                Log::error("SeparateProcess", "Redirecting stdin");
                return false;
            }

            // redirect stdout
            if (dup2(stdout_pipe[PIPE_WRITE], STDOUT_FILENO) == -1)
            {
                Log::error("SeparateProcess", "Redirecting stdout");
                return false;
            }

            // all these are for use by parent only
            close(stdin_pipe[PIPE_READ]);
            close(stdin_pipe[PIPE_WRITE]);
            close(stdout_pipe[PIPE_READ]);
            close(stdout_pipe[PIPE_WRITE]);
        }

        // run child process image
        std::vector<char*> argv;
        const std::string exe_file = StringUtils::getBasename(exe);
        auto rest_argv = StringUtils::split(argument, ' ');
        argv.push_back(const_cast<char*>(exe_file.c_str()));
        for (unsigned i = 0; i < rest_argv.size(); i++)
            argv.push_back(const_cast<char*>(rest_argv[i].c_str()));
        argv.push_back(const_cast<char*>(parent_pid.c_str()));
        argv.push_back(NULL);
        execvp(exe.c_str(), argv.data());
        Log::error("SeparateProcess", "Error in execl: errnp %d", errno);

        // if we get here at all, an error occurred, but we are in the child
        // process, so just exit
        perror("SeparateProcess: execl error");
        exit(-1);
    }
    else if (child > 0)
    {
        m_child_pid = child;
        if (create_pipe)
        {
            // parent continues here
            // close unused file descriptors, these are for child only
            close(stdin_pipe[PIPE_READ]);
            close(stdout_pipe[PIPE_WRITE]);
            m_child_stdin_write = stdin_pipe[PIPE_WRITE];
            m_child_stdout_read = stdout_pipe[PIPE_READ];
        }
    }
    else   // child < 0
    {
        if (create_pipe)
        {
            // failed to create child
            close(stdin_pipe[PIPE_READ]);
            close(stdin_pipe[PIPE_WRITE]);
            close(stdout_pipe[PIPE_READ]);
            close(stdout_pipe[PIPE_WRITE]);
        }
        return false;
    }
    return true;
}   // createChildProcess
#endif

// ----------------------------------------------------------------------------
/** Reads a command from the input pipe.
 */
std::string SeparateProcess::getLine()
{
#define BUFSIZE 1024
    char buffer[BUFSIZE];

#if defined(WIN32)
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
#elif defined(ANDROID)
    return "";
#else
    //std::string s;
    //std::getline(std::cin, s);
    //return s;

    int bytes_read = read(m_child_stdout_read, buffer, BUFSIZE-1);
    if(bytes_read>0)
    {
        buffer[bytes_read] = 0;
        std::string s = buffer;
        return s;
    }
#endif
    return std::string("");
}   // getLine

// ----------------------------------------------------------------------------
/** Sends a command to the SeparateProcess via a pipe, and reads the answer.
 *  \return Answer from SeparateProcess.
 */
std::string SeparateProcess::sendCommand(const std::string &command)
{
#if defined(WIN32)
    // Write to the pipe that is the standard input for a child process.
    // Data is written to the pipe's buffers, so it is not necessary to wait
    // until the child process is running before writing data.
    DWORD bytes_written;
    bool success = WriteFile(m_child_stdin_write, command.c_str(),
        unsigned(command.size()) + 1, &bytes_written, NULL) != 0;
#elif defined(ANDROID)

#else
    write(m_child_stdin_write, (command+"\n").c_str(), command.size()+1);
#endif
    return getLine();

    return std::string("");
}   // sendCommand

// ----------------------------------------------------------------------------
/** All answer strings from SeparateProcess are in the form: "length string",
 *  i.e. the length of the string, followed by a space and then the actual
 *  strings. This allows for checking on some potential problems (e.g. if a
 *  pipe should only send part of the answer string - todo: handle this problem
 *  instead of ignoring it.
 */
std::string SeparateProcess::decodeString(const std::string &s)
{
    std::vector<std::string> l = StringUtils::split(s, ' ');
    if (l.size() != 2) return "INVALID ANSWER - wrong number of fields";

    int n;
    StringUtils::fromString(l[0], n);
    if (n != (int)l[1].size()) return "INVALID ANSWER - incorrect length";

    return l[1];

}   // decodeString
