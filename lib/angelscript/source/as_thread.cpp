/*
   AngelCode Scripting Library
   Copyright (c) 2003-2014 Andreas Jonsson

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
// as_thread.cpp
//
// Functions for multi threading support
//

#include "as_config.h"
#include "as_thread.h"
#include "as_atomic.h"

BEGIN_AS_NAMESPACE

//=======================================================================

// Singleton
static asCThreadManager *threadManager = 0;

//======================================================================

// Global API functions
extern "C"
{

AS_API int asThreadCleanup()
{
	return asCThreadManager::CleanupLocalData();
}

AS_API asIThreadManager *asGetThreadManager()
{
	return threadManager;
}

AS_API int asPrepareMultithread(asIThreadManager *externalThreadMgr)
{
	return asCThreadManager::Prepare(externalThreadMgr);
}

AS_API void asUnprepareMultithread()
{
	asCThreadManager::Unprepare();
}

AS_API void asAcquireExclusiveLock()
{
	if( threadManager )
	{
		ACQUIREEXCLUSIVE(threadManager->appRWLock);
	}
}

AS_API void asReleaseExclusiveLock()
{
	if( threadManager )
	{
		RELEASEEXCLUSIVE(threadManager->appRWLock);
	}
}

AS_API void asAcquireSharedLock()
{
	if( threadManager )
	{
		ACQUIRESHARED(threadManager->appRWLock);
	}
}

AS_API void asReleaseSharedLock()
{
	if( threadManager )
	{
		RELEASESHARED(threadManager->appRWLock);
	}
}

}

//======================================================================

#if !defined(AS_NO_THREADS) && defined(_MSC_VER) && defined(AS_WINDOWS_THREADS) && (WINAPI_FAMILY & WINAPI_FAMILY_PHONE_APP)
__declspec(thread) asCThreadLocalData *asCThreadManager::tld = 0;
#endif

asCThreadManager::asCThreadManager()
{
	// We're already in the critical section when this function is called

#ifdef AS_NO_THREADS
	tld = 0;
#else
	// Allocate the thread local storage
	#if defined AS_POSIX_THREADS
		pthread_key_t pKey;
		pthread_key_create(&pKey, 0);
		tlsKey = (asDWORD)pKey;
	#elif defined AS_WINDOWS_THREADS
		#if defined(_MSC_VER) && (WINAPI_FAMILY & WINAPI_FAMILY_PHONE_APP)
			tld = 0;
		#else
			tlsKey = (asDWORD)TlsAlloc();
		#endif
	#endif
#endif
	refCount = 1;
}

int asCThreadManager::Prepare(asIThreadManager *externalThreadMgr)
{
	// Don't allow an external thread manager if there
	// is already a thread manager defined
	if( externalThreadMgr && threadManager )
		return asINVALID_ARG;

	// The critical section cannot be declared globally, as there is no
	// guarantee for the order in which global variables are initialized
	// or uninitialized.

	// For this reason it's not possible to prevent two threads from calling
	// AddRef at the same time, so there is a chance for a race condition here.

	// To avoid the race condition when the thread manager is first created,
	// the application must make sure to call the global asPrepareForMultiThread()
	// in the main thread before any other thread creates a script engine.
	if( threadManager == 0 && externalThreadMgr == 0 )
		threadManager = asNEW(asCThreadManager);
	else
	{
		// If an application uses different dlls each dll will get it's own memory
		// space for global variables. If multiple dlls then uses AngelScript's
		// global thread support functions it is then best to share the thread
		// manager to make sure all dlls use the same critical section.
		if( externalThreadMgr )
			threadManager = reinterpret_cast<asCThreadManager*>(externalThreadMgr);

		ENTERCRITICALSECTION(threadManager->criticalSection);
		threadManager->refCount++;
		LEAVECRITICALSECTION(threadManager->criticalSection);
	}

	// Success
	return 0;
}

void asCThreadManager::Unprepare()
{
	asASSERT(threadManager);

	if( threadManager == 0 )
		return;

	// It's necessary to protect this section so no
	// other thread attempts to call AddRef or Release
	// while clean up is in progress.
	ENTERCRITICALSECTION(threadManager->criticalSection);
	if( --threadManager->refCount == 0 )
	{
		// Make sure the local data is destroyed, at least for the current thread
		CleanupLocalData();

		// As the critical section will be destroyed together
		// with the thread manager we must first clear the global
		// variable in case a new thread manager needs to be created;
		asCThreadManager *mgr = threadManager;
		threadManager = 0;

		// Leave the critical section before it is destroyed
		LEAVECRITICALSECTION(mgr->criticalSection);

		asDELETE(mgr,asCThreadManager);
	}
	else
		LEAVECRITICALSECTION(threadManager->criticalSection);
}

asCThreadManager::~asCThreadManager()
{
#ifndef AS_NO_THREADS
	// Deallocate the thread local storage
	#if defined AS_POSIX_THREADS
		pthread_key_delete((pthread_key_t)tlsKey);
	#elif defined AS_WINDOWS_THREADS
		#if defined(_MSC_VER) && (WINAPI_FAMILY & WINAPI_FAMILY_PHONE_APP)
			tld = 0;
		#else
			TlsFree((DWORD)tlsKey);
		#endif
	#endif
#else
	if( tld )
	{
		asDELETE(tld,asCThreadLocalData);
	}
	tld = 0;
#endif
}

int asCThreadManager::CleanupLocalData()
{
	if( threadManager == 0 )
		return 0;

#ifndef AS_NO_THREADS
#if defined AS_POSIX_THREADS
	asCThreadLocalData *tld = (asCThreadLocalData*)pthread_getspecific((pthread_key_t)threadManager->tlsKey);
#elif defined AS_WINDOWS_THREADS
	#if !defined(_MSC_VER) || !(WINAPI_FAMILY & WINAPI_FAMILY_PHONE_APP)
		asCThreadLocalData *tld = (asCThreadLocalData*)TlsGetValue((DWORD)threadManager->tlsKey);
	#endif
#endif

	if( tld == 0 )
		return 0;

	if( tld->activeContexts.GetLength() == 0 )
	{
		asDELETE(tld,asCThreadLocalData);
		#if defined AS_POSIX_THREADS
			pthread_setspecific((pthread_key_t)threadManager->tlsKey, 0);
		#elif defined AS_WINDOWS_THREADS
			#if defined(_MSC_VER) && (WINAPI_FAMILY & WINAPI_FAMILY_PHONE_APP)
				tld = 0;
			#else
				TlsSetValue((DWORD)threadManager->tlsKey, 0);
			#endif
		#endif
		return 0;
	}
	else
		return asCONTEXT_ACTIVE;

#else
	if( threadManager->tld )
	{
		if( threadManager->tld->activeContexts.GetLength() == 0 )
		{
			asDELETE(threadManager->tld,asCThreadLocalData);
			threadManager->tld = 0;
		}
		else
			return asCONTEXT_ACTIVE;
	}
	return 0;
#endif
}

asCThreadLocalData *asCThreadManager::GetLocalData()
{
	if( threadManager == 0 )
		return 0;

#ifndef AS_NO_THREADS
#if defined AS_POSIX_THREADS
	asCThreadLocalData *tld = (asCThreadLocalData*)pthread_getspecific((pthread_key_t)threadManager->tlsKey);
	if( tld == 0 )
	{
		tld = asNEW(asCThreadLocalData)();
		pthread_setspecific((pthread_key_t)threadManager->tlsKey, tld);
	}
#elif defined AS_WINDOWS_THREADS
	#if defined(_MSC_VER) && (WINAPI_FAMILY & WINAPI_FAMILY_PHONE_APP)
		if( tld == 0 )
			tld = asNEW(asCThreadLocalData)();
	#else
		asCThreadLocalData *tld = (asCThreadLocalData*)TlsGetValue((DWORD)threadManager->tlsKey);
		if( tld == 0 )
		{
 			tld = asNEW(asCThreadLocalData)();
			TlsSetValue((DWORD)threadManager->tlsKey, tld);
 		}
	#endif
#endif

	return tld;
#else
	if( threadManager->tld == 0 )
		threadManager->tld = asNEW(asCThreadLocalData)();

	return threadManager->tld;
#endif
}

//=========================================================================

asCThreadLocalData::asCThreadLocalData()
{
}

asCThreadLocalData::~asCThreadLocalData()
{
}

//=========================================================================

#ifndef AS_NO_THREADS
asCThreadCriticalSection::asCThreadCriticalSection()
{
#if defined AS_POSIX_THREADS
	pthread_mutex_init(&cs, 0);
#elif defined AS_WINDOWS_THREADS
#if defined(_MSC_VER) && (WINAPI_FAMILY & WINAPI_FAMILY_PHONE_APP)
	// Only the Ex version is available on Windows Store
	InitializeCriticalSectionEx(&cs, 4000, 0);
#else
	// Only the non-Ex version is available on WinXP and older
	// MinGW also only defines this version
	InitializeCriticalSection(&cs);
#endif
#endif
}

asCThreadCriticalSection::~asCThreadCriticalSection()
{
#if defined AS_POSIX_THREADS
	pthread_mutex_destroy(&cs);
#elif defined AS_WINDOWS_THREADS
	DeleteCriticalSection(&cs);
#endif
}

void asCThreadCriticalSection::Enter()
{
#if defined AS_POSIX_THREADS
	pthread_mutex_lock(&cs);
#elif defined AS_WINDOWS_THREADS
	EnterCriticalSection(&cs);
#endif
}

void asCThreadCriticalSection::Leave()
{
#if defined AS_POSIX_THREADS
	pthread_mutex_unlock(&cs);
#elif defined AS_WINDOWS_THREADS
	LeaveCriticalSection(&cs);
#endif
}

bool asCThreadCriticalSection::TryEnter()
{
#if defined AS_POSIX_THREADS
	return !pthread_mutex_trylock(&cs);
#elif defined AS_WINDOWS_THREADS
	return TryEnterCriticalSection(&cs) ? true : false;
#else
	return true;
#endif
}

asCThreadReadWriteLock::asCThreadReadWriteLock()
{
#if defined AS_POSIX_THREADS
	int r = pthread_rwlock_init(&lock, 0);
	asASSERT( r == 0 );
	UNUSED_VAR(r);
#elif defined AS_WINDOWS_THREADS
#if defined(_MSC_VER) && (WINAPI_FAMILY & WINAPI_FAMILY_PHONE_APP)
	// Only the Ex versions are available on Windows Store

	// Create a semaphore to allow up to maxReaders simultaneous readers
	readLocks = CreateSemaphoreExW(NULL, maxReaders, maxReaders, 0, 0, 0);
	// Create a critical section to synchronize writers
	InitializeCriticalSectionEx(&writeLock, 4000, 0);
#else
	readLocks = CreateSemaphoreW(NULL, maxReaders, maxReaders, 0);
	InitializeCriticalSection(&writeLock);
#endif
#endif
}

asCThreadReadWriteLock::~asCThreadReadWriteLock()
{
#if defined AS_POSIX_THREADS
	pthread_rwlock_destroy(&lock);
#elif defined AS_WINDOWS_THREADS
	DeleteCriticalSection(&writeLock);
	CloseHandle(readLocks);
#endif
}

void asCThreadReadWriteLock::AcquireExclusive()
{
#if defined AS_POSIX_THREADS
	pthread_rwlock_wrlock(&lock);
#elif defined AS_WINDOWS_THREADS
	// Synchronize writers, so only one tries to lock out the readers
	EnterCriticalSection(&writeLock);

	// Lock all reader out from the semaphore. Do this one by one,
	// so the lock doesn't have to wait until there are no readers at all.
	// If we try to lock all at once it is quite possible the writer will
	// never succeed.
	for( asUINT n = 0; n < maxReaders; n++ )
		WaitForSingleObjectEx(readLocks, INFINITE, FALSE);

	// Allow another writer to lock. It will only be able to
	// lock the readers when this writer releases them anyway.
	LeaveCriticalSection(&writeLock);
#endif
}

void asCThreadReadWriteLock::ReleaseExclusive()
{
#if defined AS_POSIX_THREADS
	pthread_rwlock_unlock(&lock);
#elif defined AS_WINDOWS_THREADS
	// Release all readers at once
	ReleaseSemaphore(readLocks, maxReaders, 0);
#endif
}

void asCThreadReadWriteLock::AcquireShared()
{
#if defined AS_POSIX_THREADS
	pthread_rwlock_rdlock(&lock);
#elif defined AS_WINDOWS_THREADS
	// Lock a reader slot
	WaitForSingleObjectEx(readLocks, INFINITE, FALSE);
#endif
}

void asCThreadReadWriteLock::ReleaseShared()
{
#if defined AS_POSIX_THREADS
	pthread_rwlock_unlock(&lock);
#elif defined AS_WINDOWS_THREADS
	// Release the reader slot
	ReleaseSemaphore(readLocks, 1, 0);
#endif
}

#endif

//========================================================================

END_AS_NAMESPACE

