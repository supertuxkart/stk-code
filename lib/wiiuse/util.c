/*
 *	wiiuse
 *
 *  Copyright 2011 Iowa State University Virtual Reality Applications Center
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

/**
 *	@file
 *	@brief Provides platform-specific utility functions.
 */


#include "wiiuse_internal.h"

#ifdef WIIUSE_WIN32

#include <windows.h>

void wiiuse_millisleep(int durationMilliseconds) {
	Sleep(durationMilliseconds);
}

#else /* not win32 - assuming posix */

#include <unistd.h>                     /* for usleep */

void wiiuse_millisleep(int durationMilliseconds) {
	usleep(durationMilliseconds * 1000);
}

#endif /* ifdef WIIUSE_WIN32 */
