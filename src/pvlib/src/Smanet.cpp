/*
 *   Pvlib - Smanet implementation
 *
 *   Copyright (C) 2011
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <Smanet.h>

#include "log.h"

static const uint32_t ACCM = 0x000E0000;
static const uint8_t HDLC_ESC  = 0x7d;
static const uint8_t HDLC_SYNC = 0x7e;

static const uint16_t PPPINITFCS16 = 0xffff;
static const uint16_t PPPGOODFCS16 = 0xf0b8;

static const int FRAME_SIZE = 512 + 16;

static const uint16_t fcstab[256] = { 0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536,
        0x74bf, 0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7, 0x1081, 0x0108,
        0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e, 0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed,
        0xcb64, 0xf9ff, 0xe876, 0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
        0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5, 0x3183, 0x200a, 0x1291,
        0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c, 0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66,
        0xd8fd, 0xc974, 0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb, 0xce4c,
        0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3, 0x5285, 0x430c, 0x7197, 0x601e,
        0x14a1, 0x0528, 0x37b3, 0x263a, 0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb,
        0xaa72, 0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9, 0xef4e, 0xfec7,
        0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1, 0x7387, 0x620e, 0x5095, 0x411c, 0x35a3,
        0x242a, 0x16b1, 0x0738, 0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
        0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7, 0x0840, 0x19c9, 0x2b52,
        0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff, 0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324,
        0xf1bf, 0xe036, 0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e, 0xa50a,
        0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5, 0x2942, 0x38cb, 0x0a50, 0x1bd9,
        0x6f66, 0x7eef, 0x4c74, 0x5dfd, 0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd,
        0xc134, 0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c, 0xc60c, 0xd785,
        0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3, 0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60,
        0x1de9, 0x2f72, 0x3efb, 0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
        0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a, 0xe70e, 0xf687, 0xc41c,
        0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1, 0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb,
        0x0e70, 0x1ff9, 0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330, 0x7bc7,
        0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78 };

static uint16_t fcsCalc(uint16_t fcs, const uint8_t* buf, int len)
{
	while (len--) {
		fcs = (uint16_t)((fcs >> 8) ^ fcstab[(fcs ^ *buf++) & 0xff]);
	}
	return fcs;
}


//int Smanet::write(uint8_t *data, int len, const char *to)
//{
//	if (smanet->smabluetooth) {
//		smabluetooth_packet_t packet;
//
//		packet.data = data;
//		packet.len = len;
//		packet.cmd = 0x01;
//		memcpy(packet.mac_dst, to, 6);
//
//		if (smabluetooth_write(smanet->sma, &packet) < 0) {
//			LOG_ERROR("smabluetooth_write failed");
//			return -1;
//		}
//	} else {
//		if (connection_write(smanet->con, data, len) < 0) {
//			LOG_ERROR("connection_write failed");
//			return -1;
//		}
//	}
//
//	return 0;
//}
//
//static int con_read(smanet_t *smanet, uint8_t *data, int len, uint8_t *from)
//{
//	if (smanet->smabluetooth) {
//		smabluetooth_packet_t packet;
//
//		packet.data = data;
//		packet.len = len;
//
//		if (smabluetooth_read(smanet->sma, &packet) < 0) {
//			return -1;
//		}
//		if (from != NULL) {
//			memcpy(from, packet.mac_src, 6);
//		}
//
//		return packet.len;
//	} else {
//		return connection_read(smanet->con, data, len, TIMEOUT);
//	}
//}

static int addHdlc(const uint8_t *in, uint8_t *out, uint8_t len)
{
	uint16_t pos = 0;
	uint8_t i;

	for (i = 0; i < len; i++) {
		if ((in[i] < 0x20) && (ACCM & (0x00000001 << in[i]))) {
			out[pos++] = HDLC_ESC;
			out[pos++] = in[i] ^ 0x20;
		} else {
			switch (in[i]) {
			case HDLC_ESC:
			case HDLC_SYNC:
				out[pos++] = HDLC_ESC;
				out[pos++] = in[i] ^ 0x20;
				break;
			default:
				out[pos++] = in[i];
				break;
			}
		}
	}

	return pos;
}

static int validateFrame(uint8_t *frame, uint16_t len)
{
	if (len > FRAME_SIZE) {
		return -1;
	}

	if (fcsCalc(PPPINITFCS16, frame, len) != PPPGOODFCS16) {
		return -1;
	}

	return 0;
}

//int Smanet::readFrame(uint8_t *data, int len, char *from)
//{
//	uint8_t buf[FRAME_SIZE];
//	uint16_t i = 0;
//	uint16_t pos = 0;
//	uint8_t sync = 0;
//
//	if (len <= 0) return 0;
//
//
//	for (i = 0; (i < FRAME_SIZE) && !sync; i++) {
//		// add last byte to new buffer, because it could be a HDLC_ESC
//		if (((pos + 1 == size) && (read_buf[pos] != HDLC_SYNC)) || pos >= size) {
//			if (pos >= size) {
//				size = con->read(read_buf, BUF_SIZE, from);
//				if (size < 0) return -1;
//			} else {
//				read_buf[0] = read_buf[size - 1];
//				size = con->read(read_buf + 1, BUF_SIZE - 1, from);
//				if (size < 0) return -1;
//				size++;
//			}
//			pos = 0;
//		}
//
//		switch (read_buf[pos]) {
//		case HDLC_ESC:
//			pos++;
//			buf[i] = read_buf[pos++] ^ 0x20;
//			break;
//		case HDLC_SYNC:
//			if (i == 0) {
//				//remove all HDLC_SYNC bytes, because emtpy frames are allowed.
//				while ((pos < size) && (read_buf[pos] == HDLC_SYNC))
//					pos++;
//				i--;
//			} else {
//				i--;
//				size = size - pos;
//				memmove(read_buf, read_buf + pos, size);
//				sync = 1; //success
//			}
//			break;
//		default:
//			buf[i] = read_buf[pos++];
//			break;
//		}
//	}
//
//	if (!sync) {
//		LOG_ERROR("Failed: frame to big!");
//		return -1;
//	}
//
//	if (validateFrame(buf, i) < 0) {
//		LOG_ERROR("Invalid frame!");
//		return -1;
//	}
//
//	if (i < 4) return -1;
//
//	i -= 6; // header (4 bytes) + FCS (2 bytes)
//	len = (i < len) ? i : len;
//
//	memcpy(data, &buf[4], len);
//	return len;
//}

//smanet_t *smanet_init(uint16_t protocol, connection_t *con, smabluetooth_t *sma)
//{
//	smanet_t *smanet;
//
//	assert((con == NULL && sma != NULL) || (con != NULL && sma == NULL));
//
//	smanet = malloc(sizeof(*smanet));
//	if (con != NULL) {
//		smanet->smabluetooth = 0;
//		smanet->con = con;
//	} else {
//		smanet->smabluetooth = 1;
//		smanet->sma = sma;
//	}
//
//	smanet->protocol = protocol;
//	smanet->size = 0;
//
//	return smanet;
//}

//void smanet_close(smanet_t *smanet)
//{
//	free(smanet);
//}

Smanet::Smanet(uint16_t protocol, ReadWrite *con) :
		protocol(protocol),
		con(con),
		size(0) {

}

int Smanet::read(uint8_t *data, int len, std::string &from)
{
	uint8_t buf[FRAME_SIZE];
	uint16_t i = 0;
	uint16_t pos = 0;
	uint8_t sync = 0;

	if (len <= 0) return 0;


	for (i = 0; (i < FRAME_SIZE) && !sync; i++) {
		// add last byte to new buffer, because it could be a HDLC_ESC
		if (((pos + 1 == size) && (read_buf[pos] != HDLC_SYNC)) || pos >= size) {
			if (pos >= size) {
				size = con->read(read_buf, BUF_SIZE, from);
				if (size < 0) return -1;
			} else {
				read_buf[0] = read_buf[size - 1];
				size = con->read(read_buf + 1, BUF_SIZE - 1, from);
				if (size < 0) return -1;
				size++;
			}
			pos = 0;
		}

		switch (read_buf[pos]) {
		case HDLC_ESC:
			pos++;
			buf[i] = read_buf[pos++] ^ 0x20;
			break;
		case HDLC_SYNC:
			if (i == 0) {
				//remove all HDLC_SYNC bytes, because emtpy frames are allowed.
				while ((pos < size) && (read_buf[pos] == HDLC_SYNC))
					pos++;
				i--;
			} else {
				i--;
				size = size - pos;
				memmove(read_buf, read_buf + pos, size);
				sync = 1; //success
			}
			break;
		default:
			buf[i] = read_buf[pos++];
			break;
		}
	}

	if (!sync) {
		LOG_ERROR("Failed: frame to big!");
		return -1;
	}

	if (validateFrame(buf, i) < 0) {
		LOG_ERROR("Invalid frame!");
		return -1;
	}

	if (i < 4) return -1;

	i -= 6; // header (4 bytes) + FCS (2 bytes)
	len = (i < len) ? i : len;

	memcpy(data, &buf[4], len);
	return len;
}

int Smanet::write(const uint8_t *data, int len, const std::string &to)
{
	uint8_t buf[FRAME_SIZE];
	uint16_t fcs;
	uint16_t pos = 0;

	assert(len < 256);

	buf[pos++] = 0x7e;
	buf[pos++] = 0xff;
	buf[pos++] = 0x03;
	buf[pos++] = protocol & 0xff;
	buf[pos++] = (protocol >> 8) & 0xff;

	fcs = fcsCalc(PPPINITFCS16, &buf[1], 4);
	fcs = fcsCalc(fcs, data, len);
	fcs ^= 0xffff; /* complement */

	//FIXME: length check!!!
	pos += addHdlc(data, &buf[5], len);

	buf[pos++] = fcs & 0x00ff;
	buf[pos++] = (fcs >> 8) & 0x00ff;
	buf[pos++] = 0x7e;

	return con->write(buf, pos, to);
}
