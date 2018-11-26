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

#include "wiiuse_internal.h"
#include "events.h"

#include "classic.h"       /* for classic_ctrl_disconnected, etc */
#include "dynamics.h"      /* for calculate_gforce, etc */
#include "guitar_hero_3.h" /* for guitar_hero_3_disconnected, etc */
#include "io.h"            /* for wiiuse_read_data_sync, etc */
#include "ir.h"            /* for calculate_basic_ir, etc */
#include "motion_plus.h"   /* for motion_plus_disconnected, etc */
#include "nunchuk.h"       /* for nunchuk_disconnected, etc */
#include "wiiboard.h"      /* for wii_board_disconnected, etc */

#include "os.h" /* for wiiuse_os_poll */

#include <stdio.h>  /* for printf, perror */
#include <stdlib.h> /* for free, malloc */
#include <string.h> /* for memcpy, memset */

static void event_data_read(struct wiimote_t *wm, byte *msg);
static void event_data_write(struct wiimote_t *wm, byte *msg);
static void event_status(struct wiimote_t *wm, byte *msg);
static void handle_expansion(struct wiimote_t *wm, byte *msg);

static void save_state(struct wiimote_t *wm);
static int state_changed(struct wiimote_t *wm);

/**
 *	@brief Poll the wiimotes for any events.
 *
 *	@param wm		An array of pointers to wiimote_t structures.
 *	@param wiimotes	The number of wiimote_t structures in the \a wm array.
 *
 *	@return Returns number of wiimotes that an event has occurred on.
 *
 *	It is necessary to poll the wiimote devices for events
 *	that occur.  If an event occurs on a particular wiimote,
 *	the event variable will be set.
 */
int wiiuse_poll(struct wiimote_t **wm, int wiimotes) { return wiiuse_os_poll(wm, wiimotes); }

int wiiuse_update(struct wiimote_t **wiimotes, int nwiimotes, wiiuse_update_cb callback)
{
    int evnt = 0;
    if (wiiuse_poll(wiimotes, nwiimotes))
    {
        static struct wiimote_callback_data_t s;
        int i = 0;
        for (; i < nwiimotes; ++i)
        {
            switch (wiimotes[i]->event)
            {
            case WIIUSE_NONE:
                break;
            default:
                /* this could be:  WIIUSE_EVENT, WIIUSE_STATUS, WIIUSE_CONNECT, etc.. */
                s.uid              = wiimotes[i]->unid;
                s.leds             = wiimotes[i]->leds;
                s.battery_level    = wiimotes[i]->battery_level;
                s.accel            = wiimotes[i]->accel;
                s.orient           = wiimotes[i]->orient;
                s.gforce           = wiimotes[i]->gforce;
                s.ir               = wiimotes[i]->ir;
                s.buttons          = wiimotes[i]->btns;
                s.buttons_held     = wiimotes[i]->btns_held;
                s.buttons_released = wiimotes[i]->btns_released;
                s.event            = wiimotes[i]->event;
                s.state            = wiimotes[i]->state;
                s.expansion        = wiimotes[i]->exp;
                callback(&s);
                evnt++;
                break;
            }
        }
    }
    return evnt;
}

/**
 *	@brief Called on a cycle where no significant change occurs.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 */
void idle_cycle(struct wiimote_t *wm)
{
    /*
     *	Smooth the angles.
     *
     *	This is done to make sure that on every cycle the orientation
     *	angles are smoothed.  Normally when an event occurs the angles
     *	are updated and smoothed, but if no packet comes in then the
     *	angles remain the same.  This means the angle wiiuse reports
     *	is still an old value.  Smoothing needs to be applied in this
     *	case in order for the angle it reports to converge to the true
     *	angle of the device.
     */
    if (WIIUSE_USING_ACC(wm) && WIIMOTE_IS_FLAG_SET(wm, WIIUSE_SMOOTHING))
    {
        apply_smoothing(&wm->accel_calib, &wm->orient, SMOOTH_ROLL);
        apply_smoothing(&wm->accel_calib, &wm->orient, SMOOTH_PITCH);
    }

    /* clear out any old read requests */
    clear_dirty_reads(wm);
}

/**
 *	@brief Clear out all old 'dirty' read requests.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 */
void clear_dirty_reads(struct wiimote_t *wm)
{
    struct read_req_t *req = wm->read_req;

    while (req && req->dirty)
    {
        WIIUSE_DEBUG("Cleared old read request for address: %x", req->addr);

        wm->read_req = req->next;
        free(req);
        req = wm->read_req;
    }
}

/**
 *	@brief Handle accel data in a wiimote message.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *	@param msg		The message specified in the event packet.
 */
static void handle_wm_accel(struct wiimote_t *wm, byte *msg)
{
    wm->accel.x = msg[2];
    wm->accel.y = msg[3];
    wm->accel.z = msg[4];

    /* calculate the remote orientation */
    calculate_orientation(&wm->accel_calib, &wm->accel, &wm->orient,
                          WIIMOTE_IS_FLAG_SET(wm, WIIUSE_SMOOTHING));

    /* calculate the gforces on each axis */
    calculate_gforce(&wm->accel_calib, &wm->accel, &wm->gforce);
}

/**
 *	@brief Analyze the event that occurred on a wiimote.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *	@param event	The event that occurred.
 *	@param msg		The message specified in the event packet.
 *
 *	Pass the event to the registered event callback.
 */
void propagate_event(struct wiimote_t *wm, byte event, byte *msg)
{
    save_state(wm);

    switch (event)
    {
    case WM_RPT_BTN:
    {
        /* button */
        wiiuse_pressed_buttons(wm, msg);
        break;
    }
    case WM_RPT_BTN_ACC:
    {
        /* button - motion */
        wiiuse_pressed_buttons(wm, msg);

        handle_wm_accel(wm, msg);

        break;
    }
    case WM_RPT_READ:
    {
        /* data read */
        event_data_read(wm, msg);

        /* yeah buttons may be pressed, but this wasn't an "event" */
        return;
    }
    case WM_RPT_CTRL_STATUS:
    {
        /* controller status */
        event_status(wm, msg);

        /* don't execute the event callback */
        return;
    }
    case WM_RPT_BTN_EXP_8:
    case WM_RPT_BTN_EXP:
    {
        /* button - expansion */
        wiiuse_pressed_buttons(wm, msg);
        handle_expansion(wm, msg + 2);

        break;
    }
    case WM_RPT_BTN_ACC_EXP:
    {
        /* button - motion - expansion */
        wiiuse_pressed_buttons(wm, msg);

        handle_wm_accel(wm, msg);

        handle_expansion(wm, msg + 5);

        break;
    }
    case WM_RPT_BTN_ACC_IR:
    {
        /* button - motion - ir */
        wiiuse_pressed_buttons(wm, msg);

        handle_wm_accel(wm, msg);

        /* ir */
        calculate_extended_ir(wm, msg + 5);

        break;
    }
    case WM_RPT_BTN_IR_EXP:
    {
        /* button - ir - expansion */
        wiiuse_pressed_buttons(wm, msg);
        handle_expansion(wm, msg + 12);

        /* ir */
        calculate_basic_ir(wm, msg + 2);

        break;
    }
    case WM_RPT_BTN_ACC_IR_EXP:
    {
        /* button - motion - ir - expansion */
        wiiuse_pressed_buttons(wm, msg);

        handle_wm_accel(wm, msg);

        handle_expansion(wm, msg + 15);

        /* ir */
        calculate_basic_ir(wm, msg + 5);

        break;
    }

    /*
     * FIXME: this gets triggered only when the Wiimote sends 0x22
     * Acknowledge output report, return function result. This is unfortunately sent only
     * rarely, typically when there is an error (e.g. reading from an invalid address) and
     * *must not* be relied on to call the write callbacks. The report can also appear unsolicited
     * during synchronous handshake, where it would produce spurious error messages. That's why
     * it is disabled.
     */
    case WM_RPT_WRITE:
    {
        /* event_data_write(wm, msg); */
        break;
    }
    default:
    {
        WIIUSE_WARNING("Unknown event, can not handle it [Code 0x%x].", event);
        return;
    }
    }

    /* was there an event? */
    if (state_changed(wm))
    {
        wm->event = WIIUSE_EVENT;
    }
}

/**
 *	@brief Find what buttons are pressed.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *	@param msg		The message specified in the event packet.
 */
void wiiuse_pressed_buttons(struct wiimote_t *wm, byte *msg)
{
    int16_t now;

    /* convert from big endian */
    now = from_big_endian_uint16_t(msg) & WIIMOTE_BUTTON_ALL;

    /* pressed now & were pressed, then held */
    wm->btns_held = (now & wm->btns);

    /* were pressed or were held & not pressed now, then released */
    wm->btns_released = ((wm->btns | wm->btns_held) & ~now);

    /* buttons pressed now */
    wm->btns = now;
}

/**
 *	@brief Received a data packet from a read request.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *	@param msg		The message specified in the event packet.
 *
 *	Data from the wiimote comes in packets.  If the requested
 *	data segment size is bigger than one packet can hold then
 *	several packets will be received.  These packets are first
 *	reassembled into one, then the registered callback function
 *	that handles data reads is invoked.
 */
static void event_data_read(struct wiimote_t *wm, byte *msg)
{
    /* we must always assume the packet received is from the most recent request */
    byte err;
    byte len;
    uint16_t offset;
    struct read_req_t *req = wm->read_req;

    wiiuse_pressed_buttons(wm, msg);

    /* find the next non-dirty request */
    while (req && req->dirty)
    {
        req = req->next;
    }

    /* if we don't have a request out then we didn't ask for this packet */
    if (!req)
    {
        WIIUSE_WARNING("Received data packet when no request was made.");
        return;
    }

    err = msg[2] & 0x0F;

    if (err == 0x08)
    {
        WIIUSE_WARNING("Unable to read data - address does not exist.");
    } else if (err == 0x07)
    {
        WIIUSE_WARNING("Unable to read data - address is for write-only registers.");
    } else if (err)
    {
        WIIUSE_WARNING("Unable to read data - unknown error code %x.", err);
    }

    if (err)
    {
        /* this request errored out, so skip it and go to the next one */

        /* delete this request */
        wm->read_req = req->next;
        free(req);

        /* if another request exists send it to the wiimote */
        if (wm->read_req)
        {
            wiiuse_send_next_pending_read_request(wm);
        }

        return;
    }

    len       = ((msg[2] & 0xF0) >> 4) + 1;
    offset    = from_big_endian_uint16_t(msg + 3);
    req->addr = (req->addr & 0xFFFF);

    req->wait -= len;
    if (req->wait >= req->size)
    /* this should never happen */
    {
        req->wait = 0;
    }

    WIIUSE_DEBUG("Received read packet:");
    WIIUSE_DEBUG("    Packet read offset:   %i bytes", offset);
    WIIUSE_DEBUG("    Request read offset:  %i bytes", req->addr);
    WIIUSE_DEBUG("    Read offset into buf: %i bytes", offset - req->addr);
    WIIUSE_DEBUG("    Read data size:       %i bytes", len);
    WIIUSE_DEBUG("    Still need:           %i bytes", req->wait);

    /* reconstruct this part of the data */
    memcpy((req->buf + offset - req->addr), (msg + 5), len);

#ifdef WITH_WIIUSE_DEBUG
    {
        int i = 0;
        printf("Read: ");
        for (; i < req->size - req->wait; ++i)
        {
            printf("%x ", req->buf[i]);
        }
        printf("\n");
    }
#endif

    /* if all data has been received, execute the read event callback or generate event */
    if (!req->wait)
    {
        if (req->cb)
        {
            /* this was a callback, so invoke it now */
            req->cb(wm, req->buf, req->size);

            /* delete this request */
            wm->read_req = req->next;
            free(req);
        } else
        {
            /*
             *	This should generate an event.
             *	We need to leave the event in the array so the client
             *	can access it still.  We'll flag is as being 'dirty'
             *	and give the client one cycle to use it.  Next event
             *	we will remove it from the list.
             */
            wm->event  = WIIUSE_READ_DATA;
            req->dirty = 1;
        }

        /* if another request exists send it to the wiimote */
        if (wm->read_req)
        {
            wiiuse_send_next_pending_read_request(wm);
        }
    }
}

static void event_data_write(struct wiimote_t *wm, byte *msg)
{

    struct data_req_t *req = wm->data_req;

    wiiuse_pressed_buttons(wm, msg);

    /* if we don't have a request out then we didn't ask for this packet */
    if (!req)
    {
        WIIUSE_WARNING("Transmitting data packet when no request was made.");
        return;
    }
    if (!(req->state == REQ_SENT))
    {
        WIIUSE_WARNING("Transmission is not necessary");
        /* delete this request */
        wm->data_req = req->next;
        free(req);
        return;
    }

    req->state = REQ_DONE;

    if (req->cb)
    {
        /* this was a callback, so invoke it now */
        req->cb(wm, NULL, 0);
        /* delete this request */
        wm->data_req = req->next;
        free(req);
    } else
    {
        /*
         *  This should generate an event.
         *  We need to leave the event in the array so the client
         *  can access it still.  We'll flag is as being 'REQ_DONE'
         *  and give the client one cycle to use it.  Next event
         *  we will remove it from the list.
         */
        wm->event = WIIUSE_WRITE_DATA;
    }
    /* if another request exists send it to the wiimote */
    if (wm->data_req)
    {
        wiiuse_send_next_pending_write_request(wm);
    }
}

/**
 *	@brief Read the controller status.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *	@param msg		The message specified in the event packet.
 *
 *	Read the controller status and execute the registered status callback.
 */
static void event_status(struct wiimote_t *wm, byte *msg)
{
    int led[4]             = {0, 0, 0, 0};
    int attachment         = 0;
    int ir                 = 0;
    int exp_changed        = 0;
    struct data_req_t *req = wm->data_req;

    /* initial handshake is not finished yet, ignore this */
    if (WIIMOTE_IS_SET(wm, WIIMOTE_STATE_HANDSHAKE) || !msg)
    {
        return;
    }

    /*
     *	An event occurred.
     *	This event can be overwritten by a more specific
     *	event type during a handshake or expansion removal.
     */
    wm->event = WIIUSE_STATUS;

    wiiuse_pressed_buttons(wm, msg);

    /* find what LEDs are lit */
    if (msg[2] & WM_CTRL_STATUS_BYTE1_LED_1)
    {
        led[0] = 1;
    }
    if (msg[2] & WM_CTRL_STATUS_BYTE1_LED_2)
    {
        led[1] = 1;
    }
    if (msg[2] & WM_CTRL_STATUS_BYTE1_LED_3)
    {
        led[2] = 1;
    }
    if (msg[2] & WM_CTRL_STATUS_BYTE1_LED_4)
    {
        led[3] = 1;
    }

    /* probe for Motion+ */
    if (!WIIMOTE_IS_SET(wm, WIIMOTE_STATE_MPLUS_PRESENT))
    {
        wiiuse_probe_motion_plus(wm);
    }

    /* is an attachment connected to the expansion port? */
    if ((msg[2] & WM_CTRL_STATUS_BYTE1_ATTACHMENT) == WM_CTRL_STATUS_BYTE1_ATTACHMENT)
    {
        WIIUSE_DEBUG("Attachment detected!");
        attachment = 1;
    }

    /* is the speaker enabled? */
    if ((msg[2] & WM_CTRL_STATUS_BYTE1_SPEAKER_ENABLED) == WM_CTRL_STATUS_BYTE1_SPEAKER_ENABLED)
    {
        WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_SPEAKER);
    }

    /* is IR sensing enabled? */
    if ((msg[2] & WM_CTRL_STATUS_BYTE1_IR_ENABLED) == WM_CTRL_STATUS_BYTE1_IR_ENABLED)
    {
        ir = 1;
    }

    /* find the battery level and normalize between 0 and 1 */
    wm->battery_level = (msg[5] / (float)WM_MAX_BATTERY_CODE);

    /* expansion port */
    if (attachment && !WIIMOTE_IS_SET(wm, WIIMOTE_STATE_EXP)
        && !WIIMOTE_IS_SET(wm, WIIMOTE_STATE_EXP_HANDSHAKE))
    {
        /* send the initialization code for the attachment */
        handshake_expansion(wm, NULL, 0);
        exp_changed = 1;
    } else if (!attachment && WIIMOTE_IS_SET(wm, WIIMOTE_STATE_EXP))
    {
        /* attachment removed */
        disable_expansion(wm);
        exp_changed = 1;
    }

#ifdef WIIUSE_WIN32
    if (!attachment)
    {
        WIIUSE_DEBUG("Setting timeout to normal %i ms.", wm->normal_timeout);
        wm->timeout = wm->normal_timeout;
    }
#endif

    /*
     *	From now on the remote will only send status packets.
     *	We need to send a WIIMOTE_CMD_REPORT_TYPE packet to
     *	reenable other incoming reports.
     */
    if (exp_changed)
    {
        if (WIIMOTE_IS_SET(wm, WIIMOTE_STATE_IR))
        {
            /*
             *  Since the expansion status changed IR needs to
             *  be reset for the new IR report mode.
             */
            WIIMOTE_DISABLE_STATE(wm, WIIMOTE_STATE_IR);
            wiiuse_set_ir(wm, 1);
        }
    } else
    {
        wiiuse_set_report_type(wm);
        return;
    }

    /* handling new Tx for changed exp */
    if (!req)
    {
        return;
    }
    if (!(req->state == REQ_SENT))
    {
        return;
    }
    wm->data_req = req->next;
    req->state   = REQ_DONE;
    /* if(req->cb!=NULL) req->cb(wm,msg,6); */
    free(req);
}

/**
 *	@brief Handle data from the expansion.
 *
 *	@param wm		A pointer to a wiimote_t structure.
 *	@param msg		The message specified in the event packet for the expansion.
 */
static void handle_expansion(struct wiimote_t *wm, byte *msg)
{
    switch (wm->exp.type)
    {
    case EXP_NUNCHUK:
        nunchuk_event(&wm->exp.nunchuk, msg);
        break;
    case EXP_CLASSIC:
        classic_ctrl_event(&wm->exp.classic, msg);
        break;
    case EXP_GUITAR_HERO_3:
        guitar_hero_3_event(&wm->exp.gh3, msg);
        break;
    case EXP_WII_BOARD:
        wii_board_event(&wm->exp.wb, msg);
        break;
    case EXP_MOTION_PLUS:
    case EXP_MOTION_PLUS_CLASSIC:
    case EXP_MOTION_PLUS_NUNCHUK:
        motion_plus_event(&wm->exp.mp, wm->exp.type, msg);
        break;
    default:
        break;
    }
}

/**
 *	@brief Handle the handshake data from the expansion device.
 *
 *	@param wm		A pointer to a wiimote_t structure.
 *	@param data		The data read in from the device.
 *	@param len		The length of the data block, in bytes.
 *
 *	Tries to determine what kind of expansion was attached
 *	and invoke the correct handshake function.
 *
 *	If the data is NULL then this function will try to start
 *	a handshake with the expansion.
 */
void handshake_expansion(struct wiimote_t *wm, byte *data, uint16_t len)
{
    uint32_t id;
    byte val = 0;
    byte buf = 0x00;
    byte *handshake_buf;
    int gotIt = 0;

    int attempt   = 0;
    int init_good = 0;

    /*
     * KLUDGE
     * Sometimes we get the expansion in "half-connected" state
     * with an ID like 0xffffffff and invalid data - in such case retry,
     * hoping that it will sort itself out
     */

    while (attempt < 10 && !init_good)
    {
        /*
         * phase 1 - write 0x55 0x00 to init expansion without encryption
         */

        wm->expansion_state = 1;
#ifdef WIIUSE_WIN32
        /* increase the timeout until the handshake completes */
        WIIUSE_DEBUG("write 0x55 - Setting timeout to expansion %i ms.", wm->exp_timeout);
        wm->timeout = wm->exp_timeout;
#endif
        buf = 0x55;
        wiiuse_write_data(wm, WM_EXP_MEM_ENABLE1, &buf, 1);

#ifdef WIIUSE_WIN32
        /* increase the timeout until the handshake completes */
        WIIUSE_DEBUG("write 0x00 - Setting timeout to expansion %i ms.", wm->exp_timeout);
        wm->timeout = wm->exp_timeout;
#endif
        buf = 0x00;
        wiiuse_write_data(wm, WM_EXP_MEM_ENABLE2, &buf, 1);
        wiiuse_millisleep(
            500); /* delay to let the wiimote time to react, makes the handshake more reliable */

        /*
         * phase 2 - get expansion ID & calibration data
         */
        wm->expansion_state = 2;
        if (WIIMOTE_IS_SET(wm, WIIMOTE_STATE_EXP))
            disable_expansion(wm);

        handshake_buf = (byte *)malloc(EXP_HANDSHAKE_LEN * sizeof(byte));
        /* tell the wiimote to send expansion data */
        WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_EXP);
        wiiuse_read_data_sync(wm, 0, WM_EXP_MEM_CALIBR, EXP_HANDSHAKE_LEN, handshake_buf);

        id = from_big_endian_uint32_t(handshake_buf + 220);

        /*
         * Check whether we have detected expansion, sometimes we get the expansion in "half-connected" state
         * with an ID like 0xffffffff and invalid data - in such case retry
         */

        if (id != 0xffffffff && id != 0x0)
        {
            init_good = 1;
        }

        attempt++;

        wiiuse_millisleep(500);
    }

    /*
     * phase 3 - process the data, init the expansions
     */
    wm->expansion_state = 0;
    switch (id)
    {
    case EXP_ID_CODE_NUNCHUK:
        if (nunchuk_handshake(wm, &wm->exp.nunchuk, handshake_buf, EXP_HANDSHAKE_LEN))
        {
            wm->event = WIIUSE_NUNCHUK_INSERTED;
            gotIt     = 1;
        }
        break;

    case EXP_ID_CODE_CLASSIC_CONTROLLER:
        if (classic_ctrl_handshake(wm, &wm->exp.classic, handshake_buf, EXP_HANDSHAKE_LEN))
        {
            wm->event = WIIUSE_CLASSIC_CTRL_INSERTED;
            gotIt     = 1;
        }
        break;

    case EXP_ID_CODE_GUITAR:
        if (guitar_hero_3_handshake(wm, &wm->exp.gh3, handshake_buf, EXP_HANDSHAKE_LEN))
        {
            wm->event = WIIUSE_GUITAR_HERO_3_CTRL_INSERTED;
            gotIt     = 1;
        }
        break;

    case EXP_ID_CODE_MOTION_PLUS:
    case EXP_ID_CODE_MOTION_PLUS_CLASSIC:
    case EXP_ID_CODE_MOTION_PLUS_NUNCHUK:
        wiiuse_motion_plus_handshake(wm, handshake_buf, EXP_HANDSHAKE_LEN);
        wm->event = WIIUSE_MOTION_PLUS_ACTIVATED;
        gotIt     = 1;
        break;

    case EXP_ID_CODE_WII_BOARD:
        if (wii_board_handshake(wm, &wm->exp.wb, handshake_buf, EXP_HANDSHAKE_LEN))
        {
            wm->event = WIIUSE_WII_BOARD_CTRL_INSERTED;
            gotIt     = 1;
        }
        break;

    default:
        WIIUSE_WARNING("Unknown expansion type. Code: 0x%x", id);
        break;
    }

    free(handshake_buf);

    if (gotIt)
    {
        WIIMOTE_DISABLE_STATE(wm, WIIMOTE_STATE_EXP_HANDSHAKE);
        WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_EXP);
    } else
    {
        WIIUSE_WARNING("Could not handshake with expansion id: 0x%x", id);
    }

    wiiuse_set_ir_mode(wm);
    wiiuse_set_report_type(wm);
}

/**
 *	@brief Disable the expansion device if it was enabled.
 *
 *	@param wm		A pointer to a wiimote_t structure.
 *	@param data		The data read in from the device.
 *	@param len		The length of the data block, in bytes.
 *
 *	If the data is NULL then this function will try to start
 *	a handshake with the expansion.
 */
void disable_expansion(struct wiimote_t *wm)
{
    WIIUSE_DEBUG("Disabling expansion");
    if (!WIIMOTE_IS_SET(wm, WIIMOTE_STATE_EXP))
    {
        return;
    }

    /* tell the associated module the expansion was removed */
    switch (wm->exp.type)
    {
    case EXP_NUNCHUK:
        nunchuk_disconnected(&wm->exp.nunchuk);
        wm->event = WIIUSE_NUNCHUK_REMOVED;
        break;
    case EXP_CLASSIC:
        classic_ctrl_disconnected(&wm->exp.classic);
        wm->event = WIIUSE_CLASSIC_CTRL_REMOVED;
        break;
    case EXP_GUITAR_HERO_3:
        guitar_hero_3_disconnected(&wm->exp.gh3);
        wm->event = WIIUSE_GUITAR_HERO_3_CTRL_REMOVED;
        break;
    case EXP_WII_BOARD:
        wii_board_disconnected(&wm->exp.wb);
        wm->event = WIIUSE_WII_BOARD_CTRL_REMOVED;
        break;
    case EXP_MOTION_PLUS:
    case EXP_MOTION_PLUS_CLASSIC:
    case EXP_MOTION_PLUS_NUNCHUK:
        motion_plus_disconnected(&wm->exp.mp);
        wm->event = WIIUSE_MOTION_PLUS_REMOVED;
        break;
    default:
        break;
    }

    WIIMOTE_DISABLE_STATE(wm, WIIMOTE_STATE_EXP);
    wm->exp.type        = EXP_NONE;
    wm->expansion_state = 0;
}

/**
 *	@brief Save important state data.
 *	@param wm	A pointer to a wiimote_t structure.
 */
static void save_state(struct wiimote_t *wm)
{
    /* wiimote */
    wm->lstate.btns  = wm->btns;
    wm->lstate.accel = wm->accel;

    /* ir */
    if (WIIUSE_USING_IR(wm))
    {
        wm->lstate.ir_ax       = wm->ir.ax;
        wm->lstate.ir_ay       = wm->ir.ay;
        wm->lstate.ir_distance = wm->ir.distance;
    }

    /* expansion */
    switch (wm->exp.type)
    {
    case EXP_NUNCHUK:
        wm->lstate.exp_ljs_ang = wm->exp.nunchuk.js.ang;
        wm->lstate.exp_ljs_mag = wm->exp.nunchuk.js.mag;
        wm->lstate.exp_btns    = wm->exp.nunchuk.btns;
        wm->lstate.exp_accel   = wm->exp.nunchuk.accel;
        break;

    case EXP_CLASSIC:
        wm->lstate.exp_ljs_ang    = wm->exp.classic.ljs.ang;
        wm->lstate.exp_ljs_mag    = wm->exp.classic.ljs.mag;
        wm->lstate.exp_rjs_ang    = wm->exp.classic.rjs.ang;
        wm->lstate.exp_rjs_mag    = wm->exp.classic.rjs.mag;
        wm->lstate.exp_r_shoulder = wm->exp.classic.r_shoulder;
        wm->lstate.exp_l_shoulder = wm->exp.classic.l_shoulder;
        wm->lstate.exp_btns       = wm->exp.classic.btns;
        break;

    case EXP_GUITAR_HERO_3:
        wm->lstate.exp_ljs_ang    = wm->exp.gh3.js.ang;
        wm->lstate.exp_ljs_mag    = wm->exp.gh3.js.mag;
        wm->lstate.exp_r_shoulder = wm->exp.gh3.whammy_bar;
        wm->lstate.exp_btns       = wm->exp.gh3.btns;
        break;

    case EXP_WII_BOARD:
        wm->lstate.exp_wb_rtr = wm->exp.wb.rtr;
        wm->lstate.exp_wb_rtl = wm->exp.wb.rtl;
        wm->lstate.exp_wb_rbr = wm->exp.wb.rbr;
        wm->lstate.exp_wb_rbl = wm->exp.wb.rbl;
        break;

    case EXP_MOTION_PLUS:
    case EXP_MOTION_PLUS_CLASSIC:
    case EXP_MOTION_PLUS_NUNCHUK:
    {
        wm->lstate.drx = wm->exp.mp.raw_gyro.pitch;
        wm->lstate.dry = wm->exp.mp.raw_gyro.roll;
        wm->lstate.drz = wm->exp.mp.raw_gyro.yaw;

        if (wm->exp.type == EXP_MOTION_PLUS_CLASSIC)
        {
            wm->lstate.exp_ljs_ang    = wm->exp.classic.ljs.ang;
            wm->lstate.exp_ljs_mag    = wm->exp.classic.ljs.mag;
            wm->lstate.exp_rjs_ang    = wm->exp.classic.rjs.ang;
            wm->lstate.exp_rjs_mag    = wm->exp.classic.rjs.mag;
            wm->lstate.exp_r_shoulder = wm->exp.classic.r_shoulder;
            wm->lstate.exp_l_shoulder = wm->exp.classic.l_shoulder;
            wm->lstate.exp_btns       = wm->exp.classic.btns;
        } else
        {
            wm->lstate.exp_ljs_ang = wm->exp.nunchuk.js.ang;
            wm->lstate.exp_ljs_mag = wm->exp.nunchuk.js.mag;
            wm->lstate.exp_btns    = wm->exp.nunchuk.btns;
            wm->lstate.exp_accel   = wm->exp.nunchuk.accel;
        }

        break;
    }

    case EXP_NONE:
        break;
    }
}

/**
 *	@brief Determine if the current state differs significantly from the previous.
 *	@param wm	A pointer to a wiimote_t structure.
 *	@return	1 if a significant change occurred, 0 if not.
 */
static int state_changed(struct wiimote_t *wm)
{
#define STATE_CHANGED(a, b) \
    if (a != b)             \
    return 1

#define CROSS_THRESH(last, now, thresh)                                                              \
    do                                                                                               \
    {                                                                                                \
        if (WIIMOTE_IS_FLAG_SET(wm, WIIUSE_ORIENT_THRESH))                                           \
        {                                                                                            \
            if ((diff_f(last.roll, now.roll) >= thresh) || (diff_f(last.pitch, now.pitch) >= thresh) \
                || (diff_f(last.yaw, now.yaw) >= thresh))                                            \
            {                                                                                        \
                last = now;                                                                          \
                return 1;                                                                            \
            }                                                                                        \
        } else                                                                                       \
        {                                                                                            \
            if (last.roll != now.roll)                                                               \
                return 1;                                                                            \
            if (last.pitch != now.pitch)                                                             \
                return 1;                                                                            \
            if (last.yaw != now.yaw)                                                                 \
                return 1;                                                                            \
        }                                                                                            \
    } while (0)

#define CROSS_THRESH_XYZ(last, now, thresh)                                            \
    do                                                                                 \
    {                                                                                  \
        if (WIIMOTE_IS_FLAG_SET(wm, WIIUSE_ORIENT_THRESH))                             \
        {                                                                              \
            if ((diff_f(last.x, now.x) >= thresh) || (diff_f(last.y, now.y) >= thresh) \
                || (diff_f(last.z, now.z) >= thresh))                                  \
            {                                                                          \
                last = now;                                                            \
                return 1;                                                              \
            }                                                                          \
        } else                                                                         \
        {                                                                              \
            if (last.x != now.x)                                                       \
                return 1;                                                              \
            if (last.y != now.y)                                                       \
                return 1;                                                              \
            if (last.z != now.z)                                                       \
                return 1;                                                              \
        }                                                                              \
    } while (0)

    /* ir */
    if (WIIUSE_USING_IR(wm))
    {
        STATE_CHANGED(wm->lstate.ir_ax, wm->ir.ax);
        STATE_CHANGED(wm->lstate.ir_ay, wm->ir.ay);
        STATE_CHANGED(wm->lstate.ir_distance, wm->ir.distance);
    }

    /* accelerometer */
    if (WIIUSE_USING_ACC(wm))
    {
        /* raw accelerometer */
        CROSS_THRESH_XYZ(wm->lstate.accel, wm->accel, wm->accel_threshold);

        /* orientation */
        CROSS_THRESH(wm->lstate.orient, wm->orient, wm->orient_threshold);
    }

    /* expansion */
    switch (wm->exp.type)
    {
    case EXP_NUNCHUK:
    {
        STATE_CHANGED(wm->lstate.exp_ljs_ang, wm->exp.nunchuk.js.ang);
        STATE_CHANGED(wm->lstate.exp_ljs_mag, wm->exp.nunchuk.js.mag);
        STATE_CHANGED(wm->lstate.exp_btns, wm->exp.nunchuk.btns);

        CROSS_THRESH(wm->lstate.exp_orient, wm->exp.nunchuk.orient, wm->exp.nunchuk.orient_threshold);
        CROSS_THRESH_XYZ(wm->lstate.exp_accel, wm->exp.nunchuk.accel, wm->exp.nunchuk.accel_threshold);
        break;
    }
    case EXP_CLASSIC:
    {
        STATE_CHANGED(wm->lstate.exp_ljs_ang, wm->exp.classic.ljs.ang);
        STATE_CHANGED(wm->lstate.exp_ljs_mag, wm->exp.classic.ljs.mag);
        STATE_CHANGED(wm->lstate.exp_rjs_ang, wm->exp.classic.rjs.ang);
        STATE_CHANGED(wm->lstate.exp_rjs_mag, wm->exp.classic.rjs.mag);
        STATE_CHANGED(wm->lstate.exp_r_shoulder, wm->exp.classic.r_shoulder);
        STATE_CHANGED(wm->lstate.exp_l_shoulder, wm->exp.classic.l_shoulder);
        STATE_CHANGED(wm->lstate.exp_btns, wm->exp.classic.btns);
        break;
    }
    case EXP_GUITAR_HERO_3:
    {
        STATE_CHANGED(wm->lstate.exp_ljs_ang, wm->exp.gh3.js.ang);
        STATE_CHANGED(wm->lstate.exp_ljs_mag, wm->exp.gh3.js.mag);
        STATE_CHANGED(wm->lstate.exp_r_shoulder, wm->exp.gh3.whammy_bar);
        STATE_CHANGED(wm->lstate.exp_btns, wm->exp.gh3.btns);
        break;
    }
    case EXP_WII_BOARD:
    {
        STATE_CHANGED(wm->lstate.exp_wb_rtr, wm->exp.wb.tr);
        STATE_CHANGED(wm->lstate.exp_wb_rtl, wm->exp.wb.tl);
        STATE_CHANGED(wm->lstate.exp_wb_rbr, wm->exp.wb.br);
        STATE_CHANGED(wm->lstate.exp_wb_rbl, wm->exp.wb.bl);
        break;
    }

    case EXP_MOTION_PLUS:
    case EXP_MOTION_PLUS_CLASSIC:
    case EXP_MOTION_PLUS_NUNCHUK:
    {
        STATE_CHANGED(wm->lstate.drx, wm->exp.mp.raw_gyro.pitch);
        STATE_CHANGED(wm->lstate.dry, wm->exp.mp.raw_gyro.roll);
        STATE_CHANGED(wm->lstate.drz, wm->exp.mp.raw_gyro.yaw);

        if (wm->exp.type == EXP_MOTION_PLUS_CLASSIC)
        {
            STATE_CHANGED(wm->lstate.exp_ljs_ang, wm->exp.classic.ljs.ang);
            STATE_CHANGED(wm->lstate.exp_ljs_mag, wm->exp.classic.ljs.mag);
            STATE_CHANGED(wm->lstate.exp_rjs_ang, wm->exp.classic.rjs.ang);
            STATE_CHANGED(wm->lstate.exp_rjs_mag, wm->exp.classic.rjs.mag);
            STATE_CHANGED(wm->lstate.exp_r_shoulder, wm->exp.classic.r_shoulder);
            STATE_CHANGED(wm->lstate.exp_l_shoulder, wm->exp.classic.l_shoulder);
            STATE_CHANGED(wm->lstate.exp_btns, wm->exp.classic.btns);
        } else
        {
            STATE_CHANGED(wm->lstate.exp_ljs_ang, wm->exp.nunchuk.js.ang);
            STATE_CHANGED(wm->lstate.exp_ljs_mag, wm->exp.nunchuk.js.mag);
            STATE_CHANGED(wm->lstate.exp_btns, wm->exp.nunchuk.btns);

            CROSS_THRESH(wm->lstate.exp_orient, wm->exp.nunchuk.orient, wm->exp.nunchuk.orient_threshold);
            CROSS_THRESH_XYZ(wm->lstate.exp_accel, wm->exp.nunchuk.accel, wm->exp.nunchuk.accel_threshold);
        }

        break;
    }
    case EXP_NONE:
    {
        break;
    }
    }

    STATE_CHANGED(wm->lstate.btns, wm->btns);

    return 0;
}
