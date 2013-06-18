#ifndef TIME_HPP_INCLUDED
#define TIME_HPP_INCLUDED

#include <time.h>

namespace Time
{
double getSeconds()
{
    time_t timer;
    time(&timer);
    struct tm y2k;
    y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
    y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;
    return difftime(timer,mktime(&y2k)); // get the seconds elapsed since january 2000
}
}

#endif // TIME_HPP_INCLUDED
