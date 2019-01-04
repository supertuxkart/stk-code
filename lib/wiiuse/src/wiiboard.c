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
 *	@brief Wii Fit Balance Board device.
 */

#include "wiiboard.h"
#include "io.h"

#include <stdio.h>  /* for printf */
#include <string.h> /* for memset */

/**
 *	@brief Handle the handshake data from the wiiboard.
 *
 *	@param wb		A pointer to a wii_board_t structure.
 *	@param data		The data read in from the device.
 *	@param len		The length of the data block, in bytes.
 *
 *	@return	Returns 1 if handshake was successful, 0 if not.
 */

int wii_board_handshake(struct wiimote_t *wm, struct wii_board_t *wb, byte *data, uint16_t len)
{
    byte *bufptr;

    /*
     * read calibration
     */
    wiiuse_read_data_sync(wm, 0, WM_EXP_MEM_CALIBR, EXP_HANDSHAKE_LEN, data);

/* decode data */
#ifdef WITH_WIIUSE_DEBUG
    {
        int i;
        printf("DECRYPTED DATA WIIBOARD\n");
        for (i = 0; i < len; ++i)
        {
            if (i % 16 == 0)
            {
                if (i != 0)
                {
                    printf("\n");
                }

                printf("%X: ", 0x4a40000 + 32 + i);
            }
            printf("%02X ", data[i]);
        }
        printf("\n");
    }
#endif
    memset(wb, 0, sizeof(struct wii_board_t));

    bufptr     = data + 4;
    wb->ctr[0] = unbuffer_big_endian_uint16_t(&bufptr);
    wb->cbr[0] = unbuffer_big_endian_uint16_t(&bufptr);
    wb->ctl[0] = unbuffer_big_endian_uint16_t(&bufptr);
    wb->cbl[0] = unbuffer_big_endian_uint16_t(&bufptr);

    wb->ctr[1] = unbuffer_big_endian_uint16_t(&bufptr);
    wb->cbr[1] = unbuffer_big_endian_uint16_t(&bufptr);
    wb->ctl[1] = unbuffer_big_endian_uint16_t(&bufptr);
    wb->cbl[1] = unbuffer_big_endian_uint16_t(&bufptr);

    wb->ctr[2] = unbuffer_big_endian_uint16_t(&bufptr);
    wb->cbr[2] = unbuffer_big_endian_uint16_t(&bufptr);
    wb->ctl[2] = unbuffer_big_endian_uint16_t(&bufptr);
    wb->cbl[2] = unbuffer_big_endian_uint16_t(&bufptr);

    wb->use_alternate_report = 0;

    /* handshake done */
    wm->event    = WIIUSE_WII_BOARD_CTRL_INSERTED;
    wm->exp.type = EXP_WII_BOARD;

#ifdef WIIUSE_WIN32
    wm->timeout = WIIMOTE_DEFAULT_TIMEOUT;
#endif

    return 1;
}

/**
 *	@brief The wii board disconnected.
 *
 *	@param cc		A pointer to a wii_board_t structure.
 */
void wii_board_disconnected(struct wii_board_t *wb) { memset(wb, 0, sizeof(struct wii_board_t)); }

static float do_interpolate(uint16_t raw, uint16_t cal[3])
{
#define WIIBOARD_MIDDLE_CALIB 17.0f
    if (raw < cal[0])
    {
        return 0.0f;
    } else if (raw < cal[1])
    {
        return ((float)(raw - cal[0]) * WIIBOARD_MIDDLE_CALIB) / (float)(cal[1] - cal[0]);
    } else if (raw < cal[2])
    {
        return ((float)(raw - cal[1]) * WIIBOARD_MIDDLE_CALIB) / (float)(cal[2] - cal[1])
               + WIIBOARD_MIDDLE_CALIB;
    } else
    {
        return WIIBOARD_MIDDLE_CALIB * 2.0f;
    }
}

/**
 *	@brief Handle wii board event.
 *
 *	@param wb		A pointer to a wii_board_t structure.
 *	@param msg		The message specified in the event packet.
 */
void wii_board_event(struct wii_board_t *wb, byte *msg)
{
    byte *bufPtr = msg;

    wb->rtr = unbuffer_big_endian_uint16_t(&bufPtr);
    wb->rbr = unbuffer_big_endian_uint16_t(&bufPtr);
    wb->rtl = unbuffer_big_endian_uint16_t(&bufPtr);
    wb->rbl = unbuffer_big_endian_uint16_t(&bufPtr);

    /*
            Interpolate values
            Calculations borrowed from wiili.org - No names to mention sadly :(
       http://www.wiili.org/index.php/Wii_Balance_Board_PC_Drivers page however!
    */
    wb->tr = do_interpolate(wb->rtr, wb->ctr);
    wb->tl = do_interpolate(wb->rtl, wb->ctl);
    wb->br = do_interpolate(wb->rbr, wb->cbr);
    wb->bl = do_interpolate(wb->rbl, wb->cbl);
}

/**
        @todo not implemented!
*/
void wiiuse_set_wii_board_calib(struct wiimote_t *wm) {}
