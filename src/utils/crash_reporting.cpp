//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2013 Lionel Fuentes
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
#include "log.hpp"
#include <string.h>

#if defined(WIN32) && !defined(DEBUG)
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
    
    namespace CrashReporting
    {
        void getCallStackWithContext(std::string& callstack, PCONTEXT pContext);

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
            MessageBoxA(NULL, msg.c_str(), "SuperTuxKart crashed :/", MB_OK);
        }

        LONG WINAPI sehHandler(_In_ struct _EXCEPTION_POINTERS *ExceptionInfo)
        {
            winCrashHandler(ExceptionInfo->ContextRecord);
            return EXCEPTION_EXECUTE_HANDLER;
        }

        void pureCallHandler()
        {
            winCrashHandler();
        }

        int newHandler( size_t )
        {
            winCrashHandler();
            return 0;
        }

        void invalidParameterHandler(const wchar_t *, const wchar_t *, const wchar_t *, unsigned int, uintptr_t)
        {
            winCrashHandler();
        }

        void signalHandler(int code)
        {
            winCrashHandler();
        }

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
        }

        void getCallStackWithContext(std::string& callstack, PCONTEXT pContext)
        {
            HINSTANCE hImageHlpDll = LoadLibraryA("imagehlp.dll");
            if(!hImageHlpDll)
            {
                Log::warn("CrashReporting", "Failed to load DLL imagehlp.dll");
                callstack = "Crash reporting failed to load DLL imagehlp.dll";
                return;
            }

            // Retrieve the DLL functions
#define GET_FUNC_PTR(FuncName)  \
    t##FuncName _##FuncName = (t##FuncName)GetProcAddress(hImageHlpDll, #FuncName); \
    if(!_##FuncName) {    \
    Log::warn("CrashReporting", "Failed to import symbol " #FuncName " from imagehlp.dll"); \
            FreeLibrary(hImageHlpDll);  \
            return; \
    }
            
            GET_FUNC_PTR(SymCleanup                 )
            GET_FUNC_PTR(SymFunctionTableAccess64   )
            GET_FUNC_PTR(SymGetLineFromAddr64       )
            GET_FUNC_PTR(SymGetModuleBase64         )
            GET_FUNC_PTR(SymGetSymFromAddr64        )
            GET_FUNC_PTR(SymInitialize              )
            GET_FUNC_PTR(SymSetOptions              )
            GET_FUNC_PTR(StackWalk64                )
            GET_FUNC_PTR(UnDecorateSymbolName       )

#undef GET_FUNC_PTR

            const HANDLE  hProcess  = GetCurrentProcess();
            const HANDLE  hThread   = GetCurrentThread();

            // Initialize the symbol hander for the process
            {
                // Get the file path of the executable
                char    filepath[512];
                GetModuleFileNameA(NULL, filepath, sizeof(filepath));
                if(!filepath)
                {
                    Log::warn("CrashReporting", "GetModuleFileNameA failed");
                    FreeLibrary(hImageHlpDll);
                    return;
                }

                // Only keep the directory
                char* last_separator = strchr(filepath, '/');
                if(!last_separator) last_separator = strchr(filepath, '\\');
                if(last_separator)
                    last_separator[0] = '\0';

                // Since the stack trace can also be used for leak checks, don't
                // initialise this all the time.
                static bool first_time = true;

                if (first_time)
                {
                    // Finally initialize the symbol handler.
                    BOOL    bOk = _SymInitialize(hProcess, filepath ? filepath : NULL, TRUE);
                    if (!bOk)
                    {
                        Log::warn("CrashReporting", "SymInitialize() failed");
                        FreeLibrary(hImageHlpDll);
                        return;
                    }

                    _SymSetOptions(SYMOPT_LOAD_LINES);
                    first_time = false;
                }
            }

            // Get the stack trace
            {
                // Initialize the IMAGEHLP_SYMBOL64 structure
                const size_t    MaxNameLength = 256;
                IMAGEHLP_SYMBOL64*  sym = (IMAGEHLP_SYMBOL64*)alloca(sizeof(IMAGEHLP_SYMBOL64) + MaxNameLength);
                sym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
                sym->MaxNameLength = MaxNameLength;

                // Initialize the STACKFRAME structure so that it
                // corresponds to the current function call
                STACKFRAME64    stackframe;
                memset(&stackframe, 0, sizeof(stackframe));
                stackframe.AddrPC.Offset    = pContext->Eip;
                stackframe.AddrPC.Mode      = AddrModeFlat;
                stackframe.AddrStack.Offset = pContext->Esp;
                stackframe.AddrStack.Mode   = AddrModeFlat;
                stackframe.AddrFrame.Offset = pContext->Ebp;
                stackframe.AddrFrame.Mode   = AddrModeFlat;

                const DWORD machine_type    = IMAGE_FILE_MACHINE_I386;

                // Walk the stack
                const int   max_nb_calls = 32;
                for(int i=0 ; i < max_nb_calls ; i++)
                {
                    const BOOL stackframe_ok = _StackWalk64(   machine_type,
                                                               hProcess,
                                                               hThread,
                                                               &stackframe,
                                                               pContext,
                                                               NULL,
                                                               _SymFunctionTableAccess64,
                                                               _SymGetModuleBase64,
                                                               NULL);
                    if(stackframe_ok)
                    {
                        // Decode the symbol and add it to the call stack
                        DWORD64 sym_displacement;
                        if(_SymGetSymFromAddr64(    hProcess,
                                                    stackframe.AddrPC.Offset,
                                                    &sym_displacement,
                                                    sym))
                        {
                            IMAGEHLP_LINE64 line64;
                            DWORD   dwDisplacement = (DWORD)sym_displacement;
                            if(_SymGetLineFromAddr64(hProcess, stackframe.AddrPC.Offset, &dwDisplacement, &line64))
                            {
                                callstack += "\n ";

                                // Directory + filename -> filename only
                                const char* filename = line64.FileName;
                                const char* ptr = line64.FileName;
                                while(*ptr)
                                {
                                    if(*ptr == '\\' || *ptr == '/')
                                        filename = ptr+1;
                                    ptr++;
                                }
                                callstack += filename;
                                callstack += ":";
                                callstack += sym->Name;

                                char str[128];
                                _itoa(line64.LineNumber, str, 10);
                                callstack += ":";
                                callstack += str;
                            }
                            else
                            {
                                callstack += "\n ";
                                callstack += sym->Name;
                            }
                        }
                        else
                            callstack += "\n <no symbol available>";
                    }
                    else
                        break;  // done
                }
            }

            FreeLibrary(hImageHlpDll);
        }

        
        void getCallStack(std::string& callstack)
        {
            // Get the current CONTEXT
            // NB: this code is ONLY VALID FOR X86 (32 bit)!
            CONTEXT ctx;
            memset(&ctx, '\0', sizeof(ctx));
            ctx.ContextFlags = CONTEXT_FULL;
            __asm    call x
            __asm x: pop eax    // get eip (can't directly use mov)
            __asm    mov ctx.Eip, eax
            __asm    mov ctx.Ebp, ebp
            __asm    mov ctx.Esp, esp

            getCallStackWithContext(callstack, &ctx);
        }
    }   // end namespace CrashReporting

#else
    // --------------------- Unix version -----------------------
    namespace CrashReporting
    {
        void installHandlers()
        {
            // TODO!
        }

        void getCallStack(std::string& callstack)
        {
            // TODO!
        }
    }   // end namespace CrashReporting

#endif
