//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015  SuperTuxKart-Team
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

#include "utils/time.hpp"

#include "graphics/irr_driver.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"

#include <ctime>
#include <IrrlichtDevice.h>

irr::ITimer *StkTime::m_timer = NULL;
std::chrono::steady_clock::time_point
   StkTime::m_mono_start = std::chrono::steady_clock::now();

/** Init function for the timer. It grabs a copy of the timer of the
 *  current irrlicht device (which is the NULL device). This way the
 *  irrlicht time routine can be used even if no device exists. This
 *  situation can happen when the window resolution is changed - if the
 *  sfx manager (in a separate thread) would access the timer while the
 *  device does not exist, stk crashes.
 */
void StkTime::init()
{
    assert(!m_timer);
    m_timer = irr_driver->getDevice()->getTimer();
    m_timer->grab();
}   // init

// ----------------------------------------------------------------------------
/** Get the time in string for game server logging prefix (thread-safe)*/
std::string StkTime::getLogTime()
{
    time_t time_now = 0;
    time(&time_now);
    std::tm timeptr = {};
#ifdef WIN32
    localtime_s(&timeptr, &time_now);
#else
    localtime_r(&time_now, &timeptr);
#endif
    std::string result;
    result.resize(64);
    strftime(&result[0], 64, "%a %b %d %H:%M:%S %Y", &timeptr);
    size_t len = strlen(result.c_str());
    result.resize(len);
    return result;
}   // getLogTime

// ----------------------------------------------------------------------------

/** Converts the time in this object to a human readable string. */
std::string StkTime::toString(const TimeType &tt)
{
    const struct tm *t = gmtime(&tt);

    //I18N: Format for dates (%d = day, %m = month, %Y = year). See http://www.cplusplus.com/reference/ctime/strftime/ for more info about date formats.
    core::stringw w_date_format = translations->w_gettext(N_("%d/%m/%Y"));
    core::stringc c_date_format(w_date_format.c_str());
    std::string date_format(c_date_format.c_str());
    if (date_format.find("%d", 0) == std::string::npos || // substring not found
        date_format.find("%m", 0) == std::string::npos ||
        date_format.find("%Y", 0) == std::string::npos)
    {
        Log::warn("Time", "Incorrect date format in translation, using default format.");
        date_format = "%d/%m/%Y";
    }

    char s[64];
    strftime(s, 64, date_format.c_str(), t);
    return s;
}   // toString

// ----------------------------------------------------------------------------
/** Returns a time based on an arbitrary 'epoch' (e.g. could be start
 *  time of the application, 1.1.1970, ...).
 *  The value is a double precision floating point value in seconds.
 */
double StkTime::getRealTime(long startAt)
{
    assert(m_timer);
    return m_timer->getRealTime()/1000.0;
}   // getTimeSinceEpoch

// ----------------------------------------------------------------------------
/** Returns the current date.
 *  \param day Day (1 - 31).
 *  \param month (1-12).
 *  \param year (4 digits).
 */
void StkTime::getDate(int *day, int *month, int *year)
{
    std::time_t t = std::time(0);   // get time now
    std::tm * now = std::localtime(&t);
 
    if(day)   *day   = now->tm_mday;
    if(month) *month = now->tm_mon + 1;
    if(year)  *year  = now->tm_year + 1900;
}   // getDate

// ----------------------------------------------------------------------------
StkTime::ScopeProfiler::ScopeProfiler(const char* name)
{
    Log::info("ScopeProfiler", "%s {\n", name);
    m_time = getMonoTimeMs();
    m_name = name;
}   // StkTime::ScopeProfiler::ScopeProfiler

// ----------------------------------------------------------------------------
StkTime::ScopeProfiler::~ScopeProfiler()
{
    uint64_t difference = getMonoTimeMs() - m_time;
    Log::info("ScopeProfiler", "} // took %d ms (%s)\n",
        (int)difference, m_name.c_str());
}   // StkTime::ScopeProfiler::ScopeProfiler
