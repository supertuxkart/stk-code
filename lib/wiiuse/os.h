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
 *	@brief Handles device I/O.
 */

#ifndef PLATFORM_H_INCLUDED
#define PLATFORM_H_INCLUDED

#include "wiiuse_internal.h"

#ifdef __cplusplus
extern "C" {
#endif


	/** @defgroup internal_io Internal: Platform-specific Device I/O */
	/** @{ */
	void wiiuse_init_platform_fields(struct wiimote_t* wm);
	void wiiuse_cleanup_platform_fields(struct wiimote_t* wm);

	int wiiuse_os_find(struct wiimote_t** wm, int max_wiimotes, int timeout);

	int wiiuse_os_connect(struct wiimote_t** wm, int wiimotes);
	void wiiuse_os_disconnect(struct wiimote_t* wm);

	int wiiuse_os_poll(struct wiimote_t** wm, int wiimotes);
	/* buf[0] will be the report type, buf+1 the rest of the report */
	int wiiuse_os_read(struct wiimote_t* wm, byte* buf, int len);
	int wiiuse_os_write(struct wiimote_t* wm, byte report_type, byte* buf, int len);
	/** @} */

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_H_INCLUDED */
