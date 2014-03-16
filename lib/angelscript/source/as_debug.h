/*
   AngelCode Scripting Library
   Copyright (c) 2003-2012 Andreas Jonsson

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
// as_debug.h
//

#ifndef AS_DEBUG_H
#define AS_DEBUG_H

#include "as_config.h"

#ifndef AS_WII
// The Wii SDK doesn't have these, we'll survive without AS_DEBUG

#ifndef _WIN32_WCE
// Neither does WinCE


#if defined(__GNUC__) || defined( AS_MARMALADE )

#ifdef __ghs__ 
// WIIU defines __GNUC__ but types are not defined here in 'conventional' way 
#include <types.h>
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
typedef float float32_t;
typedef double float64_t;
#else
// Define mkdir for GNUC
#include <sys/stat.h>
#include <sys/types.h>
#define _mkdir(dirname) mkdir(dirname, S_IRWXU)
#endif
#else
#include <direct.h>
#endif


#if defined(_MSC_VER) && defined(AS_PROFILE)
// Currently only do profiling with MSVC++

#include <mmsystem.h>
#include "as_string.h"
#include "as_map.h"
#include "as_string_util.h"

BEGIN_AS_NAMESPACE

struct TimeCount
{
	double time;
	int    count;
	double max;
	double min;
};

class CProfiler
{
public:
	CProfiler()
	{
		// We need to know how often the clock is updated
		__int64 tps;
		if( !QueryPerformanceFrequency((LARGE_INTEGER *)&tps) )
			usePerformance = false;
		else
		{
			usePerformance = true;
			ticksPerSecond = double(tps);
		}

		timeOffset = GetTime();
	}

	~CProfiler()
	{
		WriteSummary();
	}

	double GetTime()
	{
		if( usePerformance )
		{
			__int64 ticks;
			QueryPerformanceCounter((LARGE_INTEGER *)&ticks);

			return double(ticks)/ticksPerSecond - timeOffset;
		}
		
		return double(timeGetTime())/1000.0 - timeOffset;
	}

	double Begin(const char *name)
	{
		double time = GetTime();

		// Add the scope to the key
		if( key.GetLength() )
			key += "|";
		key += name;

		// Compensate for the time spent writing to the file
		timeOffset += GetTime() - time;

		return time;
	}

	void End(const char *name, double beginTime)
	{
		double time = GetTime();

		double elapsed = time - beginTime;

		// Update the profile info for this scope
		asSMapNode<asCString, TimeCount> *cursor;
		if( map.MoveTo(&cursor, key) )
		{
			cursor->value.time += elapsed;
			cursor->value.count++;
			if( cursor->value.max < elapsed ) 
				cursor->value.max = elapsed;
			if( cursor->value.min > elapsed ) 
				cursor->value.min = elapsed;
		}
		else
		{
			TimeCount tc = {elapsed, 1, elapsed, elapsed};
			map.Insert(key, tc);
		}

		// Remove the inner most scope from the key
		int n = key.FindLast("|");
		if( n > 0 )
			key.SetLength(n);
		else
			key.SetLength(0);

		// Compensate for the time spent writing to the file
		timeOffset += GetTime() - time;
	}
	
protected:
	void WriteSummary()
	{
		// Write the analyzed info into a file for inspection
		_mkdir("AS_DEBUG");
		FILE *fp;
		#if _MSC_VER >= 1500 && !defined(AS_MARMALADE)
			fopen_s(&fp, "AS_DEBUG/profiling_summary.txt", "wt");
		#else
			fp = fopen("AS_DEBUG/profiling_summary.txt", "wt");
		#endif
		if( fp == 0 )
			return;

		fprintf(fp, "%-60s %10s %15s %15s %15s %15s\n\n", "Scope", "Count", "Tot time", "Avg time", "Max time", "Min time");

		asSMapNode<asCString, TimeCount> *cursor;
		map.MoveLast(&cursor);
		while( cursor )
		{
			asCString key = cursor->key;
			int count;
			int n = key.FindLast("|", &count);
			if( count )
			{
				key = asCString("                                               ", count) + key.SubString(n+1);
			}

			fprintf(fp, "%-60s %10d %15.6f %15.6f %15.6f %15.6f\n", key.AddressOf(), cursor->value.count, cursor->value.time, cursor->value.time / cursor->value.count, cursor->value.max, cursor->value.min);

			map.MovePrev(&cursor, cursor);
		}

		fclose(fp);
	}

	double  timeOffset;
	double  ticksPerSecond;
	bool    usePerformance;

	asCString                    key;
	asCMap<asCString, TimeCount> map;
};

extern CProfiler g_profiler;

class CProfilerScope
{
public:
	CProfilerScope(const char *name)
	{
		this->name = name;
		beginTime = g_profiler.Begin(name);
	}

	~CProfilerScope()
	{
		g_profiler.End(name, beginTime);
	}

protected:
	const char *name;
	double      beginTime;
};

#define TimeIt(x) CProfilerScope profilescope(x)

END_AS_NAMESPACE

#else // _MSC_VER && AS_PROFILE

// Define it so nothing is done
#define TimeIt(x) 

#endif // !(_MSC_VER && AS_PROFILE)








#endif // _WIN32_WCE
#endif // AS_WII

#endif


