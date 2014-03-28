//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013  SuperTuxKart-Team
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

#include <ctime>

/** Returns a time based on an arbitrary 'epoch' (e.g. could be start
 *  time of the application, 1.1.1970, ...).
 *  The value is a double precision floating point value in seconds.
 */
double StkTime::getRealTime(long startAt)
{
    return irr_driver->getRealTime()/1000.0;
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
