
#include <stdio.h>
#include <stdlib.h>
#if defined(WIN32)
#include <windows.h>
 #ifdef __CYGWIN__
  typedef long long _int64;
  #define LARGEINTEGER _int64
//  #include <largeint.h>
 #endif
#include <time.h>
#else
#include <sys/time.h>
#endif

#include "guClock.h"


double guClock::getRawTime ()
{
#if defined(WIN32)

  static double res ;
  static int perf_timer = -1 ;

  /* Use Performance Timer if it's available, mmtimer if not.  */

  if ( perf_timer == -1 )
  {
    __int64 frequency ;

    perf_timer = QueryPerformanceFrequency ( (LARGE_INTEGER *) & frequency ) ;

    if ( perf_timer )
    {
      res = 1.0 / (double) frequency ;
      perf_timer = 1 ;
    }
  }

  if ( perf_timer )
  {
    __int64 t ;
 
    QueryPerformanceCounter ( (LARGE_INTEGER *) &t ) ;
 
    return res * (double) t ;
  }
 
  return (double) timeGetTime() * 0.001 ;

#else
  timeval tv ;

  gettimeofday ( & tv, NULL ) ;

  return (double) tv.tv_sec + (double) tv.tv_usec / 1000000.0 ;
#endif
}


void guClock::update ()
{
  now = getRawTime() - start ;

  delta = now - last_time ;

  /*
    KLUDGE: If the frame rate drops below ~5Hz, then
            control will be very difficult.  It's
            actually easier to give up and slow
            down the action. max_delta defaults to
            200ms for that reason.

    KLUDGE: If update is called very rapidly, then
            delta can be zero which causes some
            programs to div0. So we'll clamp to
            a millionth of a second.
  */

  if ( delta >  max_delta ) delta = max_delta ;
  if ( delta <= 0.0 ) delta = 0.0000001 ;

  last_time = now ;
}



