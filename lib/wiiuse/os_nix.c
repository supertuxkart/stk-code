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
 *	@brief Handles device I/O for *nix.
 */

#include "io.h"
#include "events.h"
#include "os.h"

#ifdef WIIUSE_BLUEZ


#include <bluetooth/bluetooth.h>        /* for ba2str, str2ba */
#include <bluetooth/hci.h>              /* for inquiry_info */
#include <bluetooth/hci_lib.h>          /* for hci_get_route, hci_inquiry, etc */
#include <bluetooth/l2cap.h>            /* for sockaddr_l2 */

#include <stdio.h>                      /* for perror */
#include <string.h>                     /* for memset */
#include <sys/socket.h>                 /* for connect, socket */
#include <sys/time.h>                   /* for struct timeval */
#include <unistd.h>                     /* for close, write */
#include <errno.h>
#include <stdbool.h>

static int wiiuse_os_connect_single(struct wiimote_t* wm, char* address);

int wiiuse_os_find(struct wiimote_t** wm, int max_wiimotes, int timeout) {
	int device_id;
	int device_sock;
	inquiry_info scan_info_arr[128];
	inquiry_info* scan_info = scan_info_arr;
	int found_devices;
	int found_wiimotes;
	int i = 0;

	/* reset all wiimote bluetooth device addresses */
	for (found_wiimotes = 0; found_wiimotes < max_wiimotes; ++found_wiimotes) {
		/* bacpy(&(wm[found_wiimotes]->bdaddr), BDADDR_ANY); */
		memset(&(wm[found_wiimotes]->bdaddr), 0, sizeof(bdaddr_t));
	}
	found_wiimotes = 0;

	/* get the id of the first bluetooth device. */
	device_id = hci_get_route(NULL);
	if (device_id < 0) {
		if (errno == ENODEV) {
			WIIUSE_ERROR("Could not detect a Bluetooth adapter!");
		} else {
			perror("hci_get_route");
		}
		return 0;
	}

	/* create a socket to the device */
	device_sock = hci_open_dev(device_id);
	if (device_sock < 0) {
		perror("hci_open_dev");
		return 0;
	}

	memset(&scan_info_arr, 0, sizeof(scan_info_arr));

	/* scan for bluetooth devices for 'timeout' seconds */
	found_devices = hci_inquiry(device_id, timeout, 128, NULL, &scan_info, IREQ_CACHE_FLUSH);
	if (found_devices < 0) {
		perror("hci_inquiry");
		return 0;
	}

	WIIUSE_INFO("Found %i bluetooth device(s).", found_devices);

	/* display discovered devices */
	for (i = 0; (i < found_devices) && (found_wiimotes < max_wiimotes); ++i) {
		bool is_wiimote_regular =   (scan_info[i].dev_class[0] == WM_DEV_CLASS_0) &&
									(scan_info[i].dev_class[1] == WM_DEV_CLASS_1) &&
									(scan_info[i].dev_class[2] == WM_DEV_CLASS_2);
		
		bool is_wiimote_plus =      (scan_info[i].dev_class[0] == WM_PLUS_DEV_CLASS_0) &&
									(scan_info[i].dev_class[1] == WM_PLUS_DEV_CLASS_1) &&
									(scan_info[i].dev_class[2] == WM_PLUS_DEV_CLASS_2);
		if (is_wiimote_regular || is_wiimote_plus) {
			/* found a device */
			ba2str(&scan_info[i].bdaddr, wm[found_wiimotes]->bdaddr_str);
			
			const char* str_type;
			if(is_wiimote_regular)
			{
				wm[found_wiimotes]->type = WIIUSE_WIIMOTE_REGULAR;
				str_type = " (regular wiimote)";
			}
			else if(is_wiimote_plus)
			{
				wm[found_wiimotes]->type = WIIUSE_WIIMOTE_MOTION_PLUS_INSIDE;
				str_type = " (motion plus inside)";
			}

			WIIUSE_INFO("Found wiimote (type: %s) (%s) [id %i].", str_type, wm[found_wiimotes]->bdaddr_str, wm[found_wiimotes]->unid);

			wm[found_wiimotes]->bdaddr = scan_info[i].bdaddr;
			WIIMOTE_ENABLE_STATE(wm[found_wiimotes], WIIMOTE_STATE_DEV_FOUND);
			++found_wiimotes;
		}
	}

	close(device_sock);
	return found_wiimotes;
}


/**
 *	@see wiiuse_connect()
 *	@see wiiuse_os_connect_single()
 */
int wiiuse_os_connect(struct wiimote_t** wm, int wiimotes) {
	int connected = 0;
	int i = 0;

	for (; i < wiimotes; ++i) {
		if (!WIIMOTE_IS_SET(wm[i], WIIMOTE_STATE_DEV_FOUND))
			/* if the device address is not set, skip it */
		{
			continue;
		}

		if (wiiuse_os_connect_single(wm[i], NULL)) {
			++connected;
		}
	}

	return connected;
}


/**
 *	@brief Connect to a wiimote with a known address.
 *
 *	@param wm		Pointer to a wiimote_t structure.
 *	@param address	The address of the device to connect to.
 *					If NULL, use the address in the struct set by wiiuse_os_find().
 *
 *	@return 1 on success, 0 on failure
 */
static int wiiuse_os_connect_single(struct wiimote_t* wm, char* address) {
	struct sockaddr_l2 addr;
	memset(&addr, 0, sizeof(addr));

	if (!wm || WIIMOTE_IS_CONNECTED(wm)) {
		return 0;
	}

	addr.l2_family = AF_BLUETOOTH;
	bdaddr_t *bdaddr = &wm->bdaddr;
	if (address)
		/* use provided address */
	{
		str2ba(address, &addr.l2_bdaddr);
	} else {
		/** @todo this line doesn't make sense
		bacmp(bdaddr, BDADDR_ANY);*/
		/* use address of device discovered */
		addr.l2_bdaddr = *bdaddr;

	}

	/*
	 *	OUTPUT CHANNEL
	 */
	wm->out_sock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (wm->out_sock == -1) {
		return 0;
	}

	addr.l2_psm = htobs(WM_OUTPUT_CHANNEL);

	/* connect to wiimote */
	if (connect(wm->out_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("connect() output sock");
		return 0;
	}

	/*
	 *	INPUT CHANNEL
	 */
	wm->in_sock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (wm->in_sock == -1) {
		close(wm->out_sock);
		wm->out_sock = -1;
		return 0;
	}

	addr.l2_psm = htobs(WM_INPUT_CHANNEL);

	/* connect to wiimote */
	if (connect(wm->in_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("connect() interrupt sock");
		close(wm->out_sock);
		wm->out_sock = -1;
		return 0;
	}

	WIIUSE_INFO("Connected to wiimote [id %i].", wm->unid);

	/* do the handshake */
	WIIMOTE_ENABLE_STATE(wm, WIIMOTE_STATE_CONNECTED);
	wiiuse_handshake(wm, NULL, 0);

	wiiuse_set_report_type(wm);

	return 1;
}

void wiiuse_os_disconnect(struct wiimote_t* wm) {
	if (!wm || WIIMOTE_IS_CONNECTED(wm)) {
		return;
	}

	close(wm->out_sock);
	close(wm->in_sock);

	wm->out_sock = -1;
	wm->in_sock = -1;
	wm->event = WIIUSE_NONE;

	WIIMOTE_DISABLE_STATE(wm, WIIMOTE_STATE_CONNECTED);
	WIIMOTE_DISABLE_STATE(wm, WIIMOTE_STATE_HANDSHAKE);
}


int wiiuse_os_poll(struct wiimote_t** wm, int wiimotes) {
	int evnt;
	struct timeval tv;
	fd_set fds;
	int r;
	int i;
	byte read_buffer[MAX_PAYLOAD];
	int highest_fd = -1;

	evnt = 0;
	if (!wm) {
		return 0;
	}

	/* block select() for 1/2000th of a second */
	tv.tv_sec = 0;
	tv.tv_usec = 500;

	FD_ZERO(&fds);

	for (i = 0; i < wiimotes; ++i) {
		/* only poll it if it is connected */
		if (WIIMOTE_IS_SET(wm[i], WIIMOTE_STATE_CONNECTED)) {
			FD_SET(wm[i]->in_sock, &fds);

			/* find the highest fd of the connected wiimotes */
			if (wm[i]->in_sock > highest_fd) {
				highest_fd = wm[i]->in_sock;
			}
		}

		wm[i]->event = WIIUSE_NONE;
	}

	if (highest_fd == -1)
		/* nothing to poll */
	{
		return 0;
	}

	if (select(highest_fd + 1, &fds, NULL, NULL, &tv) == -1) {
		WIIUSE_ERROR("Unable to select() the wiimote interrupt socket(s).");
		perror("Error Details");
		return 0;
	}

	/* check each socket for an event */
	for (i = 0; i < wiimotes; ++i) {
		/* if this wiimote is not connected, skip it */
		if (!WIIMOTE_IS_CONNECTED(wm[i])) {
			continue;
		}

		if (FD_ISSET(wm[i]->in_sock, &fds)) {
			/* clear out the event buffer */
			memset(read_buffer, 0, sizeof(read_buffer));

			/* clear out any old read data */
			clear_dirty_reads(wm[i]);

			/* read the pending message into the buffer */
			r = wiiuse_os_read(wm[i], read_buffer, sizeof(read_buffer));
			if (r > 0) {
				/* propagate the event */
				propagate_event(wm[i], read_buffer[0], read_buffer + 1);
				evnt += (wm[i]->event != WIIUSE_NONE);
			}
		} else {
			/* send out any waiting writes */
			wiiuse_send_next_pending_write_request(wm[i]);
			idle_cycle(wm[i]);
		}
	}

	return evnt;
}

int wiiuse_os_read(struct wiimote_t* wm, byte* buf, int len) {
	int rc;
	int i;

	rc = read(wm->in_sock, buf, len);

	if (rc == -1) {
		/* error reading data */
		WIIUSE_ERROR("Receiving wiimote data (id %i).", wm->unid);
		perror("Error Details");

		if (errno == ENOTCONN) {
			/* this can happen if the bluetooth dongle is disconnected */
			WIIUSE_ERROR("Bluetooth appears to be disconnected. Wiimote unid %i will be disconnected.", wm->unid);
			wiiuse_os_disconnect(wm);
			wiiuse_disconnected(wm);
		}
	} else if (rc == 0) {
		/* remote disconnect */
		wiiuse_disconnected(wm);
	} else {
		/* read successful */
		/* on *nix we ignore the first byte */
		memmove(buf, buf + 1, len - 1);

		/* log the received data */
#ifdef WITH_WIIUSE_DEBUG
		{
			int i;
			printf("[DEBUG] (id %i) RECV: (%.2x) ", wm->unid, buf[0]);
			for (i = 1; i < rc; i++) {
				printf("%.2x ", buf[i]);
			}
			printf("\n");
		}
#endif
	}

	return rc;
}

int wiiuse_os_write(struct wiimote_t* wm, byte report_type, byte* buf, int len) {
	int rc;
	byte write_buffer[MAX_PAYLOAD];

	write_buffer[0] = WM_SET_REPORT | WM_BT_OUTPUT;
	write_buffer[1] = report_type;
	memcpy(write_buffer + 2, buf, len);
	rc = write(wm->out_sock, write_buffer, len + 2);

	if (rc < 0) {
		wiiuse_disconnected(wm);
	}

	return rc;
}

void wiiuse_init_platform_fields(struct wiimote_t* wm) {
	memset(&(wm->bdaddr), 0, sizeof(bdaddr_t)); /* = *BDADDR_ANY;*/
	wm->out_sock = -1;
	wm->in_sock = -1;
}

void wiiuse_cleanup_platform_fields(struct wiimote_t* wm) {
	wm->out_sock = -1;
	wm->in_sock = -1;
}



#endif /* ifdef WIIUSE_BLUEZ */
