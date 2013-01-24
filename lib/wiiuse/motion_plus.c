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

#include "motion_plus.h"

#include "io.h"                         /* for wiiuse_read */
#include "events.h"                     /* for disable_expansion */
#include "ir.h"                         /* for wiiuse_set_ir_mode */
#include "nunchuk.h"                    /* for nunchuk_pressed_buttons */
#include "dynamics.h"                   /* for calc_joystick_state, etc */

#include <string.h>                     /* for memset */
#include <math.h>                       /* for fabs */

static void wiiuse_calibrate_motion_plus(struct motion_plus_t *mp);
static void calculate_gyro_rates(struct motion_plus_t* mp);


void wiiuse_probe_motion_plus(struct wiimote_t *wm) {
	byte buf[MAX_PAYLOAD];
	unsigned id;

	wiiuse_read_data_sync(wm, 0, WM_EXP_MOTION_PLUS_IDENT, 6, buf);

	/* check error code */
	if (buf[4] & 0x0f) {
		WIIUSE_DEBUG("No Motion+ available, stopping probe.");
		WIIMOTE_DISABLE_STATE(wm, WIIMOTE_STATE_MPLUS_PRESENT);
		return;
	}

	/* decode the id */
	id = from_big_endian_uint32_t(buf + 2);

	if (id != EXP_ID_CODE_INACTIVE_MOTION_PLUS &&
	        id != EXP_ID_CODE_NLA_MOTION_PLUS &&
	        id != EXP_ID_CODE_NLA_MOTION_PLUS_NUNCHUK &&
	        id != EXP_ID_CODE_NLA_MOTION_PLUS_CLASSIC) {
		/* we have read something weird */
		WIIUSE_DEBUG("Motion+ ID doesn't match, probably not connected.");
		WIIMOTE_DISABLE_STATE(wm, WIIMOTE_STATE_MPLUS_PRESENT);
		return;
	}

	WIIUSE_DEBUG("Detected inactive Motion+!");
	WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_MPLUS_PRESENT);

	/* init M+ */
	buf[0] = 0x55;
	wiiuse_write_data(wm, WM_EXP_MOTION_PLUS_INIT, buf, 1);

	/* Init whatever is hanging on the pass-through port */
	buf[0] = 0x55;
	wiiuse_write_data(wm, WM_EXP_MEM_ENABLE1, buf, 1);

	buf[0] = 0x00;
	wiiuse_write_data(wm, WM_EXP_MEM_ENABLE2, buf, 1);

	/* Init gyroscope data */
	wm->exp.mp.cal_gyro.roll = 0;
	wm->exp.mp.cal_gyro.pitch = 0;
	wm->exp.mp.cal_gyro.yaw = 0;
	wm->exp.mp.orient.roll = 0.0;
	wm->exp.mp.orient.pitch = 0.0;
	wm->exp.mp.orient.yaw = 0.0;
	wm->exp.mp.raw_gyro_threshold = 10;

	wm->exp.mp.nc = &(wm->exp.nunchuk);
	wm->exp.mp.classic = &(wm->exp.classic);
	wm->exp.nunchuk.flags = &wm->flags;

	wm->exp.mp.ext = 0;

	wiiuse_set_ir_mode(wm);
	wiiuse_set_report_type(wm);
}

void wiiuse_motion_plus_handshake(struct wiimote_t *wm, byte *data, unsigned short len) {
	uint32_t val;
	if (data == NULL) {
		wiiuse_read_data_cb(wm, wiiuse_motion_plus_handshake, wm->motion_plus_id, WM_EXP_ID, 6);
	} else {
		WIIMOTE_DISABLE_STATE(wm, WIIMOTE_STATE_EXP_FAILED);
		WIIMOTE_DISABLE_STATE(wm, WIIMOTE_STATE_EXP_HANDSHAKE);
		WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_EXP); /* tell wiimote to include exp. data in reports */

		val = from_big_endian_uint32_t(data + 2);

		if (val == EXP_ID_CODE_MOTION_PLUS ||
		        val == EXP_ID_CODE_MOTION_PLUS_NUNCHUK ||
		        val == EXP_ID_CODE_MOTION_PLUS_CLASSIC) {
			/* handshake done */
			wm->event = WIIUSE_MOTION_PLUS_ACTIVATED;

			switch (val) {
				case EXP_ID_CODE_MOTION_PLUS:
					wm->exp.type = EXP_MOTION_PLUS;
					break;

				case EXP_ID_CODE_MOTION_PLUS_NUNCHUK:
					wm->exp.type = EXP_MOTION_PLUS_NUNCHUK;
					break;

				case EXP_ID_CODE_MOTION_PLUS_CLASSIC:
					wm->exp.type = EXP_MOTION_PLUS_CLASSIC;
					break;

				default:
					/* huh? */
					WIIUSE_WARNING("Unknown ID returned in Motion+ handshake %d\n", val);
					wm->exp.type = EXP_MOTION_PLUS;
					break;
			}

			WIIUSE_DEBUG("Motion plus connected");

			/* Init gyroscopes */
			wm->exp.mp.cal_gyro.roll = 0;
			wm->exp.mp.cal_gyro.pitch = 0;
			wm->exp.mp.cal_gyro.yaw = 0;
			wm->exp.mp.orient.roll = 0.0;
			wm->exp.mp.orient.pitch = 0.0;
			wm->exp.mp.orient.yaw = 0.0;
			wm->exp.mp.raw_gyro_threshold = 10;

			wm->exp.mp.nc = &(wm->exp.nunchuk);
			wm->exp.mp.classic = &(wm->exp.classic);
			wm->exp.nunchuk.flags = &wm->flags;

			wm->exp.mp.ext = 0;

			wiiuse_set_ir_mode(wm);
			wiiuse_set_report_type(wm);
		}
	}
}

static void wiiuse_set_motion_plus_clear2(struct wiimote_t *wm, byte *data, unsigned short len) {
	WIIMOTE_DISABLE_STATE(wm, WIIMOTE_STATE_EXP_FAILED);
	WIIMOTE_DISABLE_STATE(wm, WIIMOTE_STATE_EXP_HANDSHAKE);
	wiiuse_set_ir_mode(wm);
	wiiuse_status(wm);
}

static void wiiuse_set_motion_plus_clear1(struct wiimote_t *wm, byte *data, unsigned short len) {
	byte val = 0x00;
	wiiuse_write_data_cb(wm, WM_EXP_MEM_ENABLE1, &val, 1, wiiuse_set_motion_plus_clear2);
}

/**
 *      @brief Enable/disable Motion+ expansion
 *
 *      @param wm        Pointer to the wiimote with Motion+
 *      @param status    0 - off, 1 - on, standalone, 2 - nunchuk pass-through
 *
 */
void wiiuse_set_motion_plus(struct wiimote_t *wm, int status) {
	byte val;

	if (!WIIMOTE_IS_SET(wm, WIIMOTE_STATE_MPLUS_PRESENT) ||
	        WIIMOTE_IS_SET(wm, WIIMOTE_STATE_EXP_HANDSHAKE)) {
		return;
	}

	if (status) {
		WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_EXP_HANDSHAKE);
		val = (status == 1) ? 0x04 : 0x05;
		wiiuse_write_data_cb(wm, WM_EXP_MOTION_PLUS_ENABLE, &val, 1, wiiuse_motion_plus_handshake);
	} else {
		disable_expansion(wm);
		val = 0x55;
		wiiuse_write_data_cb(wm, WM_EXP_MEM_ENABLE1, &val, 1, wiiuse_set_motion_plus_clear1);
	}
}

void motion_plus_disconnected(struct motion_plus_t* mp) {
	WIIUSE_DEBUG("Motion plus disconnected");
	memset(mp, 0, sizeof(struct motion_plus_t));
}

void motion_plus_event(struct motion_plus_t* mp, int exp_type, byte* msg) {
	/*
	 * Pass-through modes interleave data from the gyro
	 * with the expansion data. This extracts the tag
	 * determining which is which
	 */
	int isMPFrame = (1 << 1) & msg[5];
	mp->ext = msg[4] & 0x1; /* extension attached to pass-through port? */

	if (mp->ext == 0 || isMPFrame) { /* reading gyro frame */
		/* Check if the gyroscope is in fast or slow mode (0 if rotating fast, 1 if slow or still) */
		mp->acc_mode = ((msg[4] & 0x2) << 1) | ((msg[3] & 0x1) << 1) | ((msg[3] & 0x2) >> 1);

		mp->raw_gyro.roll = ((msg[4] & 0xFC) << 6) | msg[1];
		mp->raw_gyro.pitch = ((msg[5] & 0xFC) << 6) | msg[2];
		mp->raw_gyro.yaw = ((msg[3] & 0xFC) << 6) | msg[0];

		/* First calibration */
		if ((mp->raw_gyro.roll > 5000) &&
		        (mp->raw_gyro.pitch > 5000) &&
		        (mp->raw_gyro.yaw > 5000) &&
		        (mp->raw_gyro.roll < 0x3fff) &&
		        (mp->raw_gyro.pitch < 0x3fff) &&
		        (mp->raw_gyro.yaw < 0x3fff) &&
		        !(mp->cal_gyro.roll) &&
		        !(mp->cal_gyro.pitch) &&
		        !(mp->cal_gyro.yaw)) {
			wiiuse_calibrate_motion_plus(mp);
		}

		/* Calculate angular rates in deg/sec and performs some simple filtering */
		calculate_gyro_rates(mp);
	}

	else {
		/* expansion frame */
		if (exp_type == EXP_MOTION_PLUS_NUNCHUK) {
			/* ok, this is nunchuck, re-encode it as regular nunchuck packet */

			/* get button states */
			nunchuk_pressed_buttons(mp->nc, (msg[5] >> 2));

			/* calculate joystick state */
			calc_joystick_state(&(mp->nc->js), msg[0], msg[1]);

			/* calculate orientation */
			mp->nc->accel.x = msg[2];
			mp->nc->accel.y = msg[3];
			mp->nc->accel.z = (msg[4] & 0xFE) | ((msg[5] >> 5) & 0x04);

			calculate_orientation(&(mp->nc->accel_calib),
			                      &(mp->nc->accel),
			                      &(mp->nc->orient),
			                      NUNCHUK_IS_FLAG_SET(mp->nc, WIIUSE_SMOOTHING));

			calculate_gforce(&(mp->nc->accel_calib),
			                 &(mp->nc->accel),
			                 &(mp->nc->gforce));

		}

		else if (exp_type == EXP_MOTION_PLUS_CLASSIC) {
			WIIUSE_ERROR("Classic controller pass-through is not implemented!\n");
		}

		else {
			WIIUSE_ERROR("Unsupported mode passed to motion_plus_event() !\n");
		}
	}
}

/**
 *    @brief Calibrate the Motion Plus gyroscopes.
 *
 *    @param mp        Pointer to a motion_plus_t structure.
 *
 *  This should be called only after receiving the first values
 *    from the Motion Plus.
 */
void wiiuse_calibrate_motion_plus(struct motion_plus_t *mp) {
	mp->cal_gyro.roll = mp->raw_gyro.roll;
	mp->cal_gyro.pitch = mp->raw_gyro.pitch;
	mp->cal_gyro.yaw = mp->raw_gyro.yaw;
	mp->orient.roll = 0.0;
	mp->orient.pitch = 0.0;
	mp->orient.yaw = 0.0;
}

static void calculate_gyro_rates(struct motion_plus_t* mp) {
	short int tmp_r, tmp_p, tmp_y;
	float tmp_roll, tmp_pitch, tmp_yaw;

	/* We consider calibration data */
	tmp_r = mp->raw_gyro.roll - mp->cal_gyro.roll;
	tmp_p = mp->raw_gyro.pitch - mp->cal_gyro.pitch;
	tmp_y = mp->raw_gyro.yaw - mp->cal_gyro.yaw;

	/* We convert to degree/sec according to fast/slow mode */
	if (mp->acc_mode & 0x04) {
		tmp_roll = (float)tmp_r / 20.0f;
	} else {
		tmp_roll = (float)tmp_r / 4.0f;
	}

	if (mp->acc_mode & 0x02) {
		tmp_pitch = (float)tmp_p / 20.0f;
	} else {
		tmp_pitch = (float)tmp_p / 4.0f;
	}

	if (mp->acc_mode & 0x01) {
		tmp_yaw = (float)tmp_y / 20.0f;
	} else {
		tmp_yaw = (float)tmp_y / 4.0f;
	}

	/* Simple filtering */
	if (fabs(tmp_roll) < 0.5f) {
		tmp_roll = 0.0f;
	}
	if (fabs(tmp_pitch) < 0.5f) {
		tmp_pitch = 0.0f;
	}
	if (fabs(tmp_yaw) < 0.5f) {
		tmp_yaw = 0.0f;
	}

	mp->angle_rate_gyro.roll = tmp_roll;
	mp->angle_rate_gyro.pitch = tmp_pitch;
	mp->angle_rate_gyro.yaw = tmp_yaw;
}
