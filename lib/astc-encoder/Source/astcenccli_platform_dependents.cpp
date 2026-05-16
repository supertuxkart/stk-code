// SPDX-License-Identifier: Apache-2.0
// ----------------------------------------------------------------------------
// Copyright 2011-2026 Arm Limited
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy
// of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.
// ----------------------------------------------------------------------------

/**
 * @brief Platform-specific function implementations.
 *
 * This module contains functions with strongly OS-dependent implementations:
 *
 *  * CPU count queries
 *  * Threading
 *  * Time
 *
 * In addition to the basic thread abstraction (which is native pthreads on
 * all platforms, except Windows where it is an emulation of pthreads), a
 * utility function to create N threads and wait for them to complete a batch
 * task has also been provided.
 */

#include "astcenccli_internal.h"

/* ============================================================================
   Platform code for Windows using the Win32 APIs.
============================================================================ */
#if defined(_WIN32) && !defined(__CYGWIN__)

#define WIN32_LEAN_AND_MEAN

#if !defined(NOMINMAX)
	#define NOMINMAX
#endif

#include <windows.h>
#include <Processthreadsapi.h>
#include <algorithm>
#include <cstring>

/** @brief Alias pthread_t to one of the internal Windows types. */
typedef HANDLE pthread_t;

/** @brief Alias pthread_attr_t to one of the internal Windows types. */
typedef int pthread_attr_t;

/**
 * @brief Proxy Windows @c CreateThread underneath a pthreads-like wrapper.
 */
static int pthread_create(
	pthread_t* thread,
	const pthread_attr_t* attribs,
	void* (*threadfunc)(void*),
	void* thread_arg
) {
	static_cast<void>(attribs);
	LPTHREAD_START_ROUTINE func = reinterpret_cast<LPTHREAD_START_ROUTINE>(threadfunc);
	*thread = CreateThread(nullptr, 0, func, thread_arg, 0, nullptr);

	// Ensure we return 0 on success, non-zero on error
	if (*thread == NULL)
	{
		return 1;
	}

	return 0;
}

/**
 * @brief Manually set CPU group and thread affinity.
 *
 * This is needed on Windows 10 or older to allow benefit from large core count
 * systems with more than 64 logical CPUs. The assignment is skipped on systems
 * with a single processor group, as it is not necessary.
 */
static void set_group_affinity(
	pthread_t thread,
	int thread_index
) {
	// Skip thread assignment for hardware with a single CPU group
	int group_count = GetActiveProcessorGroupCount();
	if (group_count == 1)
	{
		return;
	}

	// Ensure we have a valid assign if user creates more threads than cores
	int assign_index = thread_index % get_cpu_count();
	int assign_group { 0 };
	int assign_group_cpu_count { 0 };

	// Determine which core group and core in the group to use for this thread
	int group_cpu_count_sum { 0 };
	for (int group = 0; group < group_count; group++)
	{
		int group_cpu_count = static_cast<int>(GetMaximumProcessorCount(group));
		group_cpu_count_sum += group_cpu_count;

		if (assign_index < group_cpu_count_sum)
		{
			assign_group = group;
			assign_group_cpu_count = group_cpu_count;
			break;
		}
	}

	// Set the affinity to the assigned group, and all supported cores
	GROUP_AFFINITY affinity {};
	affinity.Mask = (1 << assign_group_cpu_count) - 1;
	affinity.Group = assign_group;
	SetThreadGroupAffinity(thread, &affinity, nullptr);
}

/**
 * @brief Proxy Windows @c WaitForSingleObject underneath a pthreads-like wrapper.
 */
static int pthread_join(
	pthread_t thread,
	void** value
) {
	static_cast<void>(value);
	WaitForSingleObject(thread, INFINITE);
	return 0;
}

/* See header for documentation */
int get_cpu_count()
{
	DWORD cpu_count = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
	return static_cast<int>(cpu_count);
}

/* See header for documentation */
double get_time()
{
	FILETIME tv;
	GetSystemTimePreciseAsFileTime(&tv);
	unsigned long long ticks = tv.dwHighDateTime;
	ticks = (ticks << 32) | tv.dwLowDateTime;
	return static_cast<double>(ticks) / 1.0e7;
}

/* See header for documentation */
void set_thread_name(
	const char* name
) {
	// Names are limited to 16 characters
	wchar_t wname [16] { 0 };
	size_t name_len = std::strlen(name);
	size_t clamp_len = std::min<size_t>(name_len, 15);

	// We know we only have basic 7-bit ASCII so just widen
	for (size_t i = 0; i < clamp_len; i++)
	{
		wname[i] = static_cast<wchar_t>(name[i]);
	}

 	SetThreadDescription(GetCurrentThread(), wname);
}

/* ============================================================================
   Platform code for an platform using POSIX APIs.
============================================================================ */
#else

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

/* See header for documentation */
int get_cpu_count()
{
	return static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
}

/* See header for documentation */
double get_time()
{
	timeval tv;
	gettimeofday(&tv, 0);
	return static_cast<double>(tv.tv_sec) + static_cast<double>(tv.tv_usec) * 1.0e-6;
}

/* See header for documentation */
void set_thread_name(
	const char* name
) {
	// No standard mechanism, so be defensive here
#if defined(__linux__)
	pthread_setname_np(pthread_self(), name);
#elif defined(__APPLE__)
	pthread_setname_np(name);
#else
	(void)name;
#endif
}

#endif

/**
 * @brief Worker thread helper payload for launch_threads.
 */
struct launch_desc
{
	/** @brief The native thread handle. */
	pthread_t thread_handle;
	/** @brief The total number of threads in the thread pool. */
	int thread_count;
	/** @brief The thread index in the thread pool. */
	int thread_id;
	/** @brief The user thread function to execute. */
	void (*func)(int, int, void*);
	/** @brief The user thread payload. */
	void* payload;
};

/**
 * @brief Helper function to translate thread entry points.
 *
 * Convert a (void*) thread entry to an (int, void*) thread entry, where the
 * integer contains the thread ID in the thread pool.
 *
 * @param p The thread launch helper payload.
 */
static void* launch_threads_helper(
	void *p
) {
	launch_desc* ltd = reinterpret_cast<launch_desc*>(p);
	ltd->func(ltd->thread_count, ltd->thread_id, ltd->payload);
	return nullptr;
}

/* See header for documentation */
void launch_threads(
	const char* operation,
	int thread_count,
	void (*func)(int, int, void*),
	void *payload
) {
	// Directly execute single threaded workloads on this thread
	if (thread_count <= 1)
	{
		func(1, 0, payload);
		return;
	}

	// Otherwise spawn worker threads
	launch_desc *thread_descs = new launch_desc[thread_count];
	int actual_thread_count { 0 };

	for (int i = 0; i < thread_count; i++)
	{
		thread_descs[actual_thread_count].thread_count = thread_count;
		thread_descs[actual_thread_count].thread_id = actual_thread_count;
		thread_descs[actual_thread_count].payload = payload;
		thread_descs[actual_thread_count].func = func;

		// Handle pthread_create failing by simply using fewer threads
		int error = pthread_create(
			&(thread_descs[actual_thread_count].thread_handle),
			nullptr,
			launch_threads_helper,
			reinterpret_cast<void*>(thread_descs + actual_thread_count));

		// Track how many threads we actually created
		if (!error)
		{
			// Windows needs explicit thread assignment to handle large core count systems
			#if defined(_WIN32) && !defined(__CYGWIN__)
				set_group_affinity(
					thread_descs[actual_thread_count].thread_handle,
					actual_thread_count);
			#endif

			actual_thread_count++;
		}
	}

	// If we did not create thread_count threads then emit a warning
	if (actual_thread_count != thread_count)
	{
		int log_count = actual_thread_count == 0 ? 1 : actual_thread_count;
		const char* log_s = log_count == 1 ? "" : "s";
		printf("WARNING: %s using %d thread%s due to thread creation error\n\n",
		       operation, log_count, log_s);
	}

	// If we managed to spawn any threads wait for them to complete
	if (actual_thread_count != 0)
	{
		for (int i = 0; i < actual_thread_count; i++)
		{
			pthread_join(thread_descs[i].thread_handle, nullptr);
		}
	}
	// Else fall back to using this thread
	else
	{
		func(1, 0, payload);
	}

	delete[] thread_descs;
}
