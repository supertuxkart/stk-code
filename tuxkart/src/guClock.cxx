
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#include <time.h>
#else
#include <sys/time.h>
#endif

#include "guClock.h"

double guClock::getRawTime ()
{
#if defined( WIN32 ) && !defined( __CYGWIN32__ )
  _int64 u, v ;
  QueryPerformanceCounter   ((LARGE_INTEGER*) &u ) ;
  QueryPerformanceFrequency ((LARGE_INTEGER*) &v ) ;
  return (double)u / (double)v ;
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
    KLUDGE: If the frame rate drops below 5Hz, then
            control will be very difficult.  It's
            actually easier to give up and slow
            down the action.

    KLUDGE: If update is called very rapidly, then
            delta can be zero which causes some
            programs to div0. So we'll clamp to
            a millionth of a second.
  */

  if ( delta >  0.2 ) delta = 0.2 ;
  if ( delta <= 0.0 ) delta = 0.0000001 ;

  last_time = now ;
}



