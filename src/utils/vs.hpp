//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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


#ifndef HEADER_VS_HPP
#define HEADER_VS_HPP
/** Visual studio workarounds in one place
 *  Note that Visual Studio 2013 does have the maths functions defined,
 *  so we define the work arounds only for compiler versions before 18.00
 */

#if defined(WIN32) && defined(_MSC_VER) && _MSC_VER < 1800
#  include <math.h>

#  define roundf(x) (floorf(x + 0.5f))
#  define round(x)  (floorf(x + 0.5))
#endif

#ifdef __MINGW32__
	#include <cmath>
	using std::isnan;
#endif

#if defined(WIN32) && defined(DEBUG)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

#if (defined(__linux__) && defined(__GLIBC__) && defined(__GLIBC_MINOR__)) || defined(__NetBSD__) || defined(__APPLE__) || defined(__sun)
#  include <pthread.h>
#endif

#if defined(__FreeBSD__) || defined(__DragonFly__) || defined(__OpenBSD__)
#  include <pthread.h>
#  include <pthread_np.h>
#endif

#if defined(__HAIKU__)
#  include <kernel/scheduler.h>
#endif

namespace VS
{
#if defined(_MSC_VER) && defined(DEBUG)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>

    /** This function sets the name of this thread in the VS debugger.
     *  \param name Name of the thread.
     */
    static void setThreadName(const char *name)
    {
        const DWORD MS_VC_EXCEPTION=0x406D1388;
#pragma pack(push,8)
        typedef struct tagTHREADNAME_INFO
        {
            DWORD dwType; // Must be 0x1000.
            LPCSTR szName; // Pointer to name (in user addr space).
            DWORD dwThreadID; // Thread ID (-1=caller thread).
            DWORD dwFlags; // Reserved for future use, must be zero.
        } THREADNAME_INFO;
#pragma pack(pop)

        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = name;
        info.dwThreadID = -1;
        info.dwFlags = 0;

        __try
        {
            RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR),
                            (ULONG_PTR*)&info );
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
        }

    }   // setThreadName
#else
    static void setThreadName(const char* name)
    {
#if defined(__linux__) && defined(__GLIBC__) && defined(__GLIBC_MINOR__)
#if __GLIBC__ > 2 || __GLIBC_MINOR__ > 11 || defined(__sun)
        pthread_setname_np(pthread_self(), name);
#endif
#elif defined(__FreeBSD__) || defined(__DragonFly__)
        pthread_set_name_np(pthread_self(), name);
#elif defined(__NetBSD__)
        pthread_setname_np(pthread_self(), "%s", const_cast<char *>(name));
#elif defined(__OpenBSD__)
        pthread_set_name_np(pthread_self(), const_cast<char *>(name));
#elif defined(__APPLE__)
        pthread_setname_np(name);
#elif defined(__HAIKU__)
        rename_thread(find_thread(nullptr), name);
#endif
    }   // setThreadName
#endif

}   // namespace VS

#endif   // HEADER_VS_HPP
