/*
 *	wiiuse
 *
 *	Written By:
 *		Michal Wiedenbauer	< shagkur >
 *		Dave Murphy			< WinterMute >
 *		Hector Martin		< marcan >
 * 		Radu Andries		<admiral0>
 *
 *	Copyright 2009
 *
 *	This file is part of wiiuse and fWIIne.
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
*	@brief Motion plus extension
*/

#ifndef MOTION_PLUS_H_INCLUDED
#define MOTION_PLUS_H_INCLUDED

#include "wiiuse_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

	/** @defgroup internal_mp Internal: MotionPlus */
	/** @{ */
	void motion_plus_disconnected(struct motion_plus_t* mp);

	void motion_plus_event(struct motion_plus_t* mp, int exp_type, byte* msg);

	void wiiuse_motion_plus_handshake(struct wiimote_t *wm, byte *data, unsigned short len);

	void wiiuse_probe_motion_plus(struct wiimote_t *wm);

	/** @} */

#ifdef __cplusplus
}
#endif

#endif
