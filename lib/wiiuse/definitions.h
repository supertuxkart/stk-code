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

/**
 *	@file
 *	@brief General definitions.
 */

#ifndef DEFINITIONS_H_INCLUDED
#define DEFINITIONS_H_INCLUDED

/* this is wiiuse - used to distinguish from third party programs using wiiuse.h */
#include <stdio.h>
#include "definitions_os.h"
/** @addtogroup internal_general */
/** @{ */

#define WIIMOTE_PI			3.14159265f

/* #define WITH_WIIUSE_DEBUG */

extern FILE* logtarget[];

#define OUTF_ERROR logtarget[0]
#define OUTF_WARNING logtarget[1]
#define OUTF_INFO logtarget[2]
#define OUTF_DEBUG logtarget[3]

/* Error output macros */
#define WIIUSE_ERROR(fmt, ...)		do { if (OUTF_ERROR) fprintf(OUTF_ERROR, "[ERROR] " fmt "\n", ##__VA_ARGS__); } while(0)

/* Warning output macros */
#define WIIUSE_WARNING(fmt, ...)	do { if (OUTF_WARNING) fprintf(OUTF_WARNING, "[WARNING] " fmt "\n",	##__VA_ARGS__); } while(0)

/* Information output macros */
#define WIIUSE_INFO(fmt, ...)		do { if (OUTF_INFO) fprintf(OUTF_INFO, "[INFO] " fmt "\n", ##__VA_ARGS__); } while(0)

#ifdef WITH_WIIUSE_DEBUG
	#ifdef WIIUSE_WIN32
		#define WIIUSE_DEBUG(fmt, ...)		do {																					\
												if (OUTF_DEBUG) {																	\
													char* file = __FILE__;															\
													int i = strlen(file) - 1;														\
													for (; i && (file[i] != '\\'); --i);											\
													fprintf(OUTF_DEBUG, "[DEBUG] %s:%i: " fmt "\n", file+i+1, __LINE__, ##__VA_ARGS__);	\
												}																					\
											} while (0)
	#else
		#define WIIUSE_DEBUG(fmt, ...)	do { if (OUTF_DEBUG) fprintf(OUTF_DEBUG, "[DEBUG] " __FILE__ ":%i: " fmt "\n", __LINE__, ##__VA_ARGS__); } while (0)
	#endif
#else
	#define WIIUSE_DEBUG(fmt, ...)
#endif

/* Convert between radians and degrees */
#define RAD_TO_DEGREE(r)	((r * 180.0f) / WIIMOTE_PI)
#define DEGREE_TO_RAD(d)	(d * (WIIMOTE_PI / 180.0f))

#define absf(x)						((x >= 0) ? (x) : (x * -1.0f))
#define diff_f(x, y)				((x >= y) ? (absf(x - y)) : (absf(y - x)))

#define WCONST


/** @} */

#endif /* DEFINITIONS_H_INCLUDED */
