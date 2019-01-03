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
 *	@brief Handles wiimote events.
 *
 *	The file includes functions that handle the events
 *	that are sent from the wiimote to us.
 */

#ifndef EVENTS_H_INCLUDED
#define EVENTS_H_INCLUDED

#include "wiiuse.h"

/** @defgroup internal_events Internal: Event Utilities */
/** @{ */
void wiiuse_pressed_buttons(struct wiimote_t *wm, byte *msg);

void handshake_expansion(struct wiimote_t *wm, byte *data, uint16_t len);
void disable_expansion(struct wiimote_t *wm);

void propagate_event(struct wiimote_t *wm, byte event, byte *msg);
void idle_cycle(struct wiimote_t *wm);

void clear_dirty_reads(struct wiimote_t *wm);
/** @} */

#endif /* EVENTS_H_INCLUDED */
