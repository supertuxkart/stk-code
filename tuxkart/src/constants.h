
#ifndef _CONSTANTS_H_

#define _CONSTANTS_H_ 1

/*
  All final units are in meters (or meters/sec or meters/sec^2)
  and degrees (or degrees/sec).
*/

/* Handy constants */

#define GRAVITY              (4.0f * 9.80665f)
#define MILE                 1609.344f
#define KILOMETER            1000.000f
#define HOUR                 (60.0f * 60.0f)

/*
  For convenience - here are some multipliers for other units.

  eg  30 * MILES_PER_HOUR  is 30mph expressed in m/sec
*/

#define MILES_PER_HOUR       (MILE/HOUR)
#define KILOMETERS_PER_HOUR  (KILOMETER/HOUR)

/* M$ compilers don't define M_PI... */

#ifndef M_PI
#  define M_PI 3.14159265358979323846  /* As in Linux's math.h */
#endif

#endif

