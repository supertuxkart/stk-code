//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2013-2015 Lionel Fuentes
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

#include "crash_reporting.hpp"

#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#include <string.h>

#if defined(WIN32) && !defined(DEBUG) && !defined(__MINGW32__)
    // --------------------- Windows version -----------------
    #include <Windows.h>
    #include <DbgHelp.h>
    #include <stdlib.h>
    #include <signal.h>
    #include <new.h>

    typedef BOOL  (__stdcall *tSymCleanup)(
        _In_ HANDLE hProcess
        );
    typedef PVOID (__stdcall *tSymFunctionTableAccess64)(
        _In_ HANDLE hProcess,
        _In_ DWORD64 AddrBase
    );
    typedef BOOL  (__stdcall *tSymGetLineFromAddr64)(
        _In_ HANDLE hProcess,
        _In_ DWORD64 qwAddr,
        _Out_ PDWORD pdwDisplacement,
        _Out_ PIMAGEHLP_LINE64 Line64
    );
    typedef DWORD64 (__stdcall *tSymGetModuleBase64)(
        _In_ HANDLE hProcess,
        _In_ DWORD64 qwAddr
    );
    typedef BOOL  (__stdcall *tSymGetSymFromAddr64)(
        _In_ HANDLE hProcess,
        _In_ DWORD64 qwAddr,
        _Out_opt_ PDWORD64 pdwDisplacement,
        _Inout_ PIMAGEHLP_SYMBOL64  Symbol
    );
    typedef BOOL  (__stdcall *tSymInitialize)(
        _In_ HANDLE hProcess,
        _In_opt_ PCSTR UserSearchPath,
        _In_ BOOL fInvadeProcess
    );
    typedef DWORD (__stdcall *tSymSetOptions)(
        _In_ DWORD   SymOptions
    );
    typedef BOOL  (__stdcall *tStackWalk64)(
        _In_ DWORD MachineType,
        _In_ HANDLE hProcess,
        _In_ HANDLE hThread,
        _Inout_ LPSTACKFRAME64 StackFrame,
        _Inout_ PVOID ContextRecord,
        _In_opt_ PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
        _In_opt_ PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
        _In_opt_ PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
        _In_opt_ PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress
    );
    typedef DWORD (__stdcall *tUnDecorateSymbolName)(
        _In_ PCSTR name,
        _Out_ PSTR outputString,
        _In_ DWORD maxStringLength,
        _In_ DWORD flags
    );
    typedef BOOL (__stdcall *tSymFromAddr) (
        _In_ HANDLE hProcess,
        _In_ DWORD64 Address,
        _Out_opt_ PDWORD64 Displacement,
        _Inout_ PSYMBOL_INFO Symbol
        );


    namespace CrashReporting
    {
        void getCallStackWithContext(std::string& callstack, PCONTEXT pContext);

        // --------------------------------------------------------------------
        void winCrashHandler(PCONTEXT pContext=NULL)
        {
            std::string callstack;
            if(pContext)
                getCallStackWithContext(callstack, pContext);
            else
                getCallStack(callstack);

            std::string msg =   "SuperTuxKart crashed!\n"
                                "Please hit Ctrl+C to copy to clipboard and signal the problem\n"
                                "to the developers on our forum: http://forum.freegamedev.net/viewforum.php?f=16\n"
                                "\n"
                                "Call stack:\n";
            msg += callstack;
            Log::error("StackTrace", "%s", msg.c_str());
            MessageBoxA(NULL, msg.c_str(), "SuperTuxKart crashed :/", MB_OK);
        }   // winCrashHandler

        // --------------------------------------------------------------------
        LONG WINAPI sehHandler(_In_ struct _EXCEPTION_POINTERS *ExceptionInfo)
        {
            winCrashHandler(ExceptionInfo->ContextRecord);
            return EXCEPTION_EXECUTE_HANDLER;
        }   // sehHandler

        // --------------------------------------------------------------------
        void pureCallHandler()
        {
            winCrashHandler();
        }   // pureCallHandler

        // --------------------------------------------------------------------
        int newHandler( size_t )
        {
            winCrashHandler();
            return 0;
        }   // newHandler

        // --------------------------------------------------------------------
        void invalidParameterHandler(const wchar_t *, const wchar_t *,
                                     const wchar_t *, unsigned int, uintptr_t)
        {
            winCrashHandler();
        }   // invalidParameterHandler

        // --------------------------------------------------------------------
        void signalHandler(int code)
        {
            winCrashHandler();
        }   // signalHandler
        // --------------------------------------------------------------------

        void installHandlers()
        {
            // ----- Per-process handlers -----
            SetUnhandledExceptionFilter(sehHandler);    // Top-level SEH handler
            _set_purecall_handler(pureCallHandler);     // Pure virtual function calls handler

            // Catch new operator memory allocation exceptions
            _set_new_mode(1); // Force malloc() to call new handler too
            _set_new_handler(newHandler);

            _set_invalid_parameter_handler(invalidParameterHandler);     // Catch invalid parameter exceptions.
            //_set_security_error_handler(securityHandler);              // Catch buffer overrun exceptions

            signal(SIGABRT, signalHandler);
            signal(SIGINT,  signalHandler);
            signal(SIGTERM, signalHandler);

            // ----- Per-thread handlers -----
            // TODO
        }   // installHandlers

        // --------------------------------------------------------------------
        void getCallStackWithContext(std::string& callstack, PCONTEXT pContext)
        {
            HINSTANCE hDbgHelpDll = LoadLibraryA("DbgHelp.dll");
            if (!hDbgHelpDll)
            {
                Log::warn("CrashReporting", "Failed to load DLL dbghelp.dll");
                callstack = "Crash reporting failed to load DLL dbghelp.dll";
                return;
            }

            // Retrieve the DLL functions
#define GET_FUNC_PTR(FuncName)  \
    t##FuncName _##FuncName = (t##FuncName)GetProcAddress(hDbgHelpDll, #FuncName); \
    if(!_##FuncName)                                                               \
     {                                                                             \
         Log::warn("CrashReporting", "Failed to import symbol " #FuncName          \
                   " from hDbgHelpDll");                                           \
            FreeLibrary(hDbgHelpDll);                                              \
            return;                                                                \
    } 

                GET_FUNC_PTR(SymCleanup)
                GET_FUNC_PTR(SymFunctionTableAccess64)
                GET_FUNC_PTR(SymGetLineFromAddr64)
                GET_FUNC_PTR(SymGetModuleBase64)
                GET_FUNC_PTR(SymGetSymFromAddr64)
                GET_FUNC_PTR(SymInitialize)
                GET_FUNC_PTR(SymSetOptions)
                GET_FUNC_PTR(UnDecorateSymbolName)
                GET_FUNC_PTR(SymFromAddr);
                GET_FUNC_PTR(StackWalk64);
#undef GET_FUNC_PTR

            const HANDLE  hProcess  = GetCurrentProcess();
            const HANDLE  hThread   = GetCurrentThread();

            // Since the stack trace can also be used for leak checks, don't
            // initialise this all the time.
            static bool first_time = true;

            // Initialize the symbol hander for the process
            if (first_time)
            {
                // Get the file path of the executable
                char filepath[512];
                GetModuleFileNameA(NULL, filepath, sizeof(filepath));
                if (!filepath)
                {
                    Log::warn("CrashReporting", "GetModuleFileNameA failed");
                    FreeLibrary(hDbgHelpDll);
                    return;
                }
                // Only keep the directory
                std::string s(filepath);
                std::string path = StringUtils::getPath(s);

                // Finally initialize the symbol handler.
                BOOL bOk = _SymInitialize(hProcess,
                                          path.empty() ? NULL : path.c_str(),
                                          TRUE);
                if (!bOk)
                {
                    Log::warn("CrashReporting", "SymInitialize() failed");
                    FreeLibrary(hDbgHelpDll);
                    return;
                }

                _SymSetOptions(SYMOPT_LOAD_LINES);
                first_time = false;
            }   // if first_time

            // Get the stack trace
            {
                // Initialize the STACKFRAME structure so that it
                // corresponds to the current function call
                STACKFRAME64    stackframe;
                memset(&stackframe, 0, sizeof(stackframe));
                stackframe.AddrPC.Mode      = AddrModeFlat;
                stackframe.AddrStack.Mode   = AddrModeFlat;
                stackframe.AddrFrame.Mode   = AddrModeFlat;
#ifdef _WIN64
                stackframe.AddrPC.Offset    = pContext->Rip;
                stackframe.AddrStack.Offset = pContext->Rsp;
                stackframe.AddrFrame.Offset = pContext->Rsp;
                const DWORD machine_type    = IMAGE_FILE_MACHINE_AMD64;
#else
                stackframe.AddrPC.Offset    = pContext->Eip;
                stackframe.AddrStack.Offset = pContext->Esp;
                stackframe.AddrFrame.Offset = pContext->Ebp;
                const DWORD machine_type = IMAGE_FILE_MACHINE_I386;
#endif


                // Walk the stack
                const int   max_nb_calls = 32;
                for(int i=0 ; i < max_nb_calls ; i++)
                {
                    const BOOL stackframe_ok =
                         _StackWalk64(machine_type, hProcess, hThread,
                                      &stackframe, pContext, NULL,
                                      _SymFunctionTableAccess64,
                                      _SymGetModuleBase64, NULL       );
                    if (!stackframe_ok) break;

                    // Decode the symbol and add it to the call stack
                    DWORD64 sym_displacement;
                    char buffer[ sizeof(SYMBOL_INFO) + 
                                 MAX_SYM_NAME * sizeof(TCHAR) ];
                    PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
                    symbol->MaxNameLen = MAX_SYM_NAME;
                    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
                    if (!_SymFromAddr(hProcess, stackframe.AddrPC.Offset,
                        &sym_displacement, symbol))
                    {
                        callstack += "\n <no symbol available>";
                        continue;
                    }
                    IMAGEHLP_LINE64 line64;
                    DWORD   dwDisplacement = (DWORD)sym_displacement;
                    if (_SymGetLineFromAddr64(hProcess,
                        stackframe.AddrPC.Offset,
                        &dwDisplacement, &line64))
                    {
                        std::string s(line64.FileName);
                        callstack += "\n " + StringUtils::getBasename(s)
                                  + ":" + symbol->Name + ":"
                                  + StringUtils::toString(line64.LineNumber);
                    }   // if SymGetLineFromAddr64
                    else
                    {
                        callstack += std::string("\n ") + symbol->Name;
                    }
                }   // for i < max_calls
            }   // get the stack trace

            FreeLibrary(hDbgHelpDll);
        }   //   // getCallStackWithContext

        // --------------------------------------------------------------------
        void getCallStack(std::string& callstack)
        {
            CONTEXT context;
            memset(&context, 0, sizeof(CONTEXT));
            context.ContextFlags = CONTEXT_FULL;
            RtlCaptureContext(&context);
            getCallStackWithContext(callstack, &context);
        }   // getCallStack

    }   // end namespace CrashReporting

// ============================================================================

#elif ENABLE_LIBBFD
    // --------------------- Unix version -----------------------
    /* Derived from addr2line.c from binutils

    addr2line.c -- convert addresses to line number and function name
    Copyright (C) 1997-2015 Free Software Foundation, Inc.
    Contributed by Ulrich Lauther <Ulrich.Lauther@mchp.siemens.de>

    This file is part of GNU Binutils.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, 51 Franklin Street - Fifth Floor, Boston,
    MA 02110-1301, USA.  */

    #define PACKAGE 1
    #define PACKAGE_VERSION 1
    #include <signal.h>
    #include <execinfo.h>
    #include <bfd.h>

    #include "string_utils.hpp"

    namespace CrashReporting
    {
        // BFD of current running STK binary, only can be useful
        // if compiled with debug symbols
        static bfd *m_stk_bfd = NULL;

        // Symbol table
        static asymbol **m_syms = NULL;

        // Address in BFD to find file names or function name
        static bfd_vma m_adress = 0;

        static const char *m_file_name = NULL;
        static const char *m_function_name = NULL;
        static unsigned int m_line = 0;
        static int m_found = 0;

        // Look for an address in a section.  This is called via
        // bfd_map_over_sections.
        void findAddressInSection(bfd* input, asection *section,
                                  void *data ATTRIBUTE_UNUSED)
        {
            bfd_vma vma = 0;
            bfd_size_type size = 0;

            if (m_found)
                return;

            if ((bfd_get_section_flags(m_stk_bfd, section) & SEC_ALLOC) == 0)
                return;

            vma = bfd_get_section_vma(m_stk_bfd, section);
            if (m_adress < vma)
                return;

            size = bfd_section_size(m_stk_bfd, section);
            if (m_adress >= vma + size)
                return;

            m_found = bfd_find_nearest_line(m_stk_bfd, section, m_syms,
                m_adress - vma, &m_file_name, &m_function_name, &m_line);
        }

        void signalHandler(int signal_no)
        {
            if (m_stk_bfd == NULL)
            {
                Log::warn("CrashReporting", "Failed loading or missing BFD of "
                          "STK binary, no backtrace available when reporting");
                exit(0);
            }

            Log::error("CrashReporting", "STK has crashed! Backtrace info:");
            std::string stack;
            getCallStack(stack);

            std::vector<std::string> each = StringUtils::split(stack, '\n');
            for (unsigned int i = 3; i < each.size(); i++)
            {
                // Skip 3 stacks which are crash_reporting doing
                Log::error("CrashReporting", "%s", each[i].c_str());
            }
            exit(0);
        }

        void loadSTKBFD()
        {
            const char* path = realpath("/proc/self/exe", NULL);
            m_stk_bfd = bfd_openr(path, NULL);
            free((void*)path);

            if (m_stk_bfd == NULL)
            {
                return;
            }

            if (bfd_check_format(m_stk_bfd, bfd_archive))
            {
                m_stk_bfd = NULL;
                return;
            }

            if (!bfd_check_format(m_stk_bfd, bfd_object))
            {
                m_stk_bfd = NULL;
                return;
            }

            // Read in the symbol table.
            unsigned int size = 0;
            long symcount = 0;

            if ((bfd_get_file_flags(m_stk_bfd) & HAS_SYMS) == 0)
            {
                m_stk_bfd = NULL;
                return;
            }

            symcount = bfd_read_minisymbols(m_stk_bfd, false, (void**)&m_syms,
                &size);
            if (symcount == 0)
            {
                symcount = bfd_read_minisymbols(m_stk_bfd, true/* dynamic*/,
                    (void**)&m_syms, &size);
            }

            if (symcount < 0)
            {
                m_stk_bfd = NULL;
                m_syms = NULL;
                return;
            }
        }

        void installHandlers()
        {
            loadSTKBFD();
            struct sigaction sa = {0};
            sa.sa_handler = &signalHandler;
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = SA_RESTART;
            sigaction(SIGSEGV, &sa, NULL);
            sigaction(SIGUSR1, &sa, NULL);
        }

        void getCallStack(std::string& callstack)
        {
            if (m_stk_bfd == NULL) return;

            void *trace[16];
            int i, trace_size = 0;
            trace_size = backtrace(trace, 16);
            for (i = 0; i < trace_size; i++)
            {
                m_adress = (bfd_vma)(trace[i]);
                m_found = 0;
                m_file_name = NULL;
                m_function_name = NULL;
                m_line = 0;

                bfd_map_over_sections(m_stk_bfd, findAddressInSection, NULL);
                if (m_found && m_file_name != NULL)
                {
                    callstack = callstack + m_file_name + ":" +
                        StringUtils::toString(m_line) + "\n";
                }
                else if (m_function_name != NULL)
                {
                    callstack = callstack + m_function_name + "\n";
                }
                else
                    callstack = callstack + "No symbol found" + "\n";
            }
        }
    }   // end namespace CrashReporting

#else

    namespace CrashReporting
    {
        void installHandlers() {}
        void getCallStack(std::string& callstack) {}
    }   // end namespace CrashReporting

#endif
