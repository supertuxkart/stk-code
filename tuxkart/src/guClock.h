
#ifndef _GUCLOCK_H_
#define _GUCLOCK_H_ 1

class guClock
{
  double start ;
  double now   ;
  double delta ;
  double last_time ;
  double max_delta ;

  double getRawTime () ;

public:

  guClock () { reset () ; }

  void reset ()
  {
    start     = getRawTime () ;
    now       = start ;
    max_delta = 0.2 ; 
    delta     = 1.0 / 30.0 ;  /* Faked so stoopid programs won't div0 */
    last_time = now - delta ;
  }

  void   setMaxDelta  ( double maxDelta ) { max_delta = maxDelta ; }
  double getMaxDelta  () { return max_delta ; }
  void   update       () ;
  double getAbsTime   () { return now   ; }
  double getDeltaTime () { return delta ; }
  double getFrameRate () { return 1.0 / delta ; }
} ;


#endif

