/*
 *	wiiuse
 *
 *	Written By:
 *		Michael Laforest	< para >
 *		Email: < thepara (--AT--) g m a i l [--DOT--] com >
 *
 *	Copyright 2006-2007
 *
 *	This file is part of wiiuse.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *	$Header$
 *
 */


#include "../os.h"

#ifdef __MACH__
	#include <mach/clock.h>
	#include <mach/mach.h>
#endif

unsigned long wiiuse_os_ticks() {
	struct timespec ts;
  	clock_serv_t cclock;
  	mach_timespec_t mts;
  	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  	clock_get_time(cclock, &mts);
  	mach_port_deallocate(mach_task_self(), cclock);
  	ts.tv_sec = mts.tv_sec;
  	ts.tv_nsec = mts.tv_nsec;
  	unsigned long ms = 1000 * ts.tv_sec + ts.tv_nsec / 1e6;
  	return ms;
}
