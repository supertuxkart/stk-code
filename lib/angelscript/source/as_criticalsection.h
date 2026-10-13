/*
   AngelCode Scripting Library
   Copyright (c) 2003-2017 Andreas Jonsson

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any
   purpose, including commercial applications, and to alter it and
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
      distribution.

   The original version of this library can be located at:
   http://www.angelcode.com/angelscript/

   Andreas Jonsson
   andreas@angelcode.com
*/



//
// as_criticalsection.h
//
// Classes for multi threading support
//

#ifndef AS_CRITICALSECTION_H
#define AS_CRITICALSECTION_H

#include "as_config.h"

BEGIN_AS_NAMESPACE

#ifdef AS_NO_THREADS

#define DECLARECRITICALSECTION(x)
#define ENTERCRITICALSECTION(x)
#define LEAVECRITICALSECTION(x)

inline bool tryEnter() { return true; }
#define TRYENTERCRITICALSECTION(x) tryEnter()

#define DECLAREREADWRITELOCK(x)
#define ACQUIREEXCLUSIVE(x)
#define RELEASEEXCLUSIVE(x)
#define ACQUIRESHARED(x)
#define RELEASESHARED(x)

#else

#define DECLARECRITICALSECTION(x)  asCThreadCriticalSection x;
#define ENTERCRITICALSECTION(x)    x.Enter()
#define LEAVECRITICALSECTION(x)    x.Leave()
#define TRYENTERCRITICALSECTION(x) x.TryEnter()

#define DECLAREREADWRITELOCK(x)    asCThreadReadWriteLock x;
#define ACQUIREEXCLUSIVE(x)        x.AcquireExclusive()
#define RELEASEEXCLUSIVE(x)        x.ReleaseExclusive()
#define ACQUIRESHARED(x)           x.AcquireShared()
#define RELEASESHARED(x)           x.ReleaseShared()

#ifdef AS_POSIX_THREADS

END_AS_NAMESPACE
#include <pthread.h>
BEGIN_AS_NAMESPACE

class asCThreadCriticalSection
{
public:
	asCThreadCriticalSection();
	~asCThreadCriticalSection();

	void Enter();
	void Leave();
	bool TryEnter();

protected:
	pthread_mutex_t cs;
};

class asCThreadReadWriteLock
{
public:
	asCThreadReadWriteLock();
	~asCThreadReadWriteLock();

	void AcquireExclusive();
	void ReleaseExclusive();
	bool TryAcquireExclusive();

	void AcquireShared();
	void ReleaseShared();
	bool TryAcquireShared();

protected:
	pthread_rwlock_t lock;
};

#elif defined(AS_WINDOWS_THREADS)

END_AS_NAMESPACE
#ifdef AS_XBOX360
#include <xtl.h>
#else
#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINNT
  #define _WIN32_WINNT 0x0600 // We need this to get the declaration for Windows Phone compatible Ex functions
#endif
#include <windows.h>
#endif
BEGIN_AS_NAMESPACE

// Undefine macros that cause problems in our code
#undef GetObject
#undef RegisterClass

class asCThreadCriticalSection
{
public:
	asCThreadCriticalSection();
	~asCThreadCriticalSection();

	void Enter();
	void Leave();
	bool TryEnter();

protected:
	CRITICAL_SECTION cs;
};

class asCThreadReadWriteLock
{
public:
	asCThreadReadWriteLock();
	~asCThreadReadWriteLock();

	void AcquireExclusive();
	void ReleaseExclusive();

	void AcquireShared();
	void ReleaseShared();

protected:
	// The Slim Read Write Lock object, SRWLOCK, is more efficient
	// but it is only available from Windows Vista so we cannot use it and
	// maintain compatibility with olders versions of Windows.

	// Critical sections and semaphores are available on Windows XP and onwards.
	// Windows XP is oldest version we support with multithreading.
	
	// The implementation is based on the following article, that shows
	// how to implement a fair read/write lock that doesn't risk starving
	// the writers:

	// http://doc.qt.nokia.com/qq/qq11-mutex.html

	// TODO: Allow use of SRWLOCK through configuration in as_config.h

	CRITICAL_SECTION    writeLock;
	HANDLE              readLocks;
};

// This constant really should be a member of asCThreadReadWriteLock,
// but it gives a compiler error on MSVC6 so I'm leaving it outside
static const asUINT maxReaders = 10;

#endif

#endif

END_AS_NAMESPACE

#endif

