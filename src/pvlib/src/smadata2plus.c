/*
 *   Pvlib - Smadata2plus implementation
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include "smadata2plus.h"
#include "smabluetooth.h"
#include "smanet.h"
#include "log.h"
#include "byte.h"
#include "pvlib.h"
#include "protocol.h"
#include "uthash.h"
#include "resources.h"
#include "thread.h"

static const uint16_t PROTOCOL = 0x6560;
static const unsigned int HEADER_SIZE = 24;

/* ctrl */
static const int CTRL_MASTER = 1 << 7 | 1 << 5;
static const int  CTRL_NO_BROADCAST = 1 << 6;
static const int CTRL_UNKNOWN = 1 << 3;

/* address */
#define ADDR_BROADCAST 0xffffffff
static const uint8_t MAC_BROADCAST[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

static const int VOLTAGE_DIVISOR = 100;     //to volts
static const int CURRENT_DIVISOR = 1000;    // to ampere
static const int FREQUENCE_DIVISOR = 100;   // to herz
//static const uint32_t smadata2plus_serial = 0x3b225946;
static const uint32_t smadata2plus_serial = 0x3a8b74b6;

static const int NUM_RETRIES = 3;

typedef enum user_type_s {
	PASSWORD_USER,
	PASSWORD_INSTALLER
} user_type_t;

enum {
	TOTAL_POWER    = 0x263f,
	MAX_PHASE1     = 0x411e,
	MAX_PHASE2     = 0x411f,
	MAX_PHASE3     = 0x4120,
	UNKNOWN_1      = 0x4166,
	UNKNWON_2      = 0x417f,
	POWER_PHASE1   = 0x4640,
	POWER_PHASE2   = 0x4641,
	POWER_PHASE3   = 0x4642,
	VOLTAGE_PHASE1 = 0x4648,
	VOLTAGE_PHASE2 = 0x4649,
	VOLTAGE_PHASE3 = 0x464a,
	CURRENT_PHASE1 = 0x4650,
	CURRENT_PHASE2 = 0x4651,
	CURRENT_PHASE3 = 0x4652,
	FREQUENCE      = 0x4657
};

enum {
	DC_POWER = 0x251e, DC_VOLTAGE = 0x451f, DC_CURRENT = 0x4521
};

enum {
	STAT_OPERATION_TIME = 0x462E,
	STAT_FEED_IN_TIME   = 0x462F,
	STAT_TOTAL_YIELD    = 0x2601,
	STAT_DAY_YIELD      = 0x2622,
};

enum {
	DEVICE_NAME  = 0x821E,
	DEVICE_CLASS = 0x821F,
	DEVICE_TYPE  = 0x8220,
	DEVICE_UNKNOWN = 0x8221,
	DEVICE_SWVER = 0x8234,
};

enum {
	DEVICE_STATUS = 0x2148
};

typedef struct devices_s {
	uint32_t serial;
	uint8_t mac[6];
	bool authenticated;
} device_t;

struct tag_hash {
    int num;
    char tag[128];
    char message[256];

    UT_hash_handle hh;
};


struct smadata2plus_s {
	connection_t *con;
	smabluetooth_t *sma;
	smanet_t *smanet;
	uint16_t pkt_cnt; // Packet counter

	device_t *devices;
	int device_num;

	struct tag_hash *tags;

};

typedef struct {
	uint32_t attribute;
	bool selected;
} attribute_t;


typedef struct {
	uint8_t cnt;
	uint32_t idx;
	uint8_t type; // 00, 04 -> type 1 or 2, 08 -> attributes, 0x10 -> string
	uint32_t time;
} record_header_t;

typedef struct {
	uint32_t value1;
	uint32_t value2;
	uint32_t value3;
	uint32_t value4;
	uint32_t unknonwn;
} record_1_t;

typedef struct {
	uint64_t value;
} record_2_t;

typedef struct {
	uint8_t data[32];
} record_3_t;


typedef enum {
	RECORD_1,
	RECORD_2,
	RECORD_3
}record_type_t;

typedef struct record {
	record_header_t header;

	record_type_t record_type;

	union {
		record_1_t r1;
		record_2_t r2;
		record_3_t r3;
	} record;
} record_t;


static void parse_attributes(const uint8_t *data, int data_len, attribute_t *attributes, int *len)
{
	int attribute_idx = 0;

	for (int idx = 0; idx < data_len && attribute_idx < *len; idx += 4, attribute_idx++) {
		uint32_t attribute = byte_parse_u32_little(data + idx) & 0x00ffffff;
		uint8_t selected = data[idx + 3];

		if (attribute == 0xfffffe) {
			break; //end of enums
		}

		attributes[attribute_idx].attribute = attribute;
		attributes[attribute_idx].selected  = selected;
	}

	*len = attribute_idx;
}

static void parse_record_header(const uint8_t* buf, record_header_t *header) {
	header->cnt = buf[0];
	header->idx = byte_parse_u16_little(buf + 1);
	header->type = buf[3];
	header->time = byte_parse_u32_little(buf + 4);
}

static void parse_record_1(const uint8_t *buf, record_1_t *r1)
{
	r1->value1 = byte_parse_u32_little(buf);
	r1->value2 = byte_parse_u32_little(buf + 4);
	r1->value3 = byte_parse_u32_little(buf + 8);
	r1->value4 = byte_parse_u32_little(buf + 12);
	r1->unknonwn = byte_parse_u32_little(buf + 16);
}

static void parse_record_2(const uint8_t *buf, record_2_t *r2)
{
	r2->value = byte_parse_u64_little(buf);
}

static void parse_record_3(const uint8_t *buf, record_3_t *r3)
{
	memcpy(r3->data, buf, 40);
}


static int parse_channel_records(const uint8_t *buf, int len, record_t *records, int *max_records, record_type_t type)
{
	if (buf[0] != 0x1 || buf[1] != 0x02) {
		LOG_ERROR("Unexpected data in record header!");
		return -1; //invalid data
	}

	uint16_t object = byte_parse_u16_little(buf + 2);
	LOG_DEBUG("Object id %02x", object);

	uint32_t unknown1 = byte_parse_u32_little(buf + 4);
	uint32_t unknown2 = byte_parse_u32_little(buf + 8);
	LOG_DEBUG("record data unknonwn1: %d", unknown1);
	LOG_DEBUG("record data unknonwn2: %d", unknown2);

	size_t record_length = 0;
	switch (type) {
		case RECORD_1 : record_length = 28; break;
		case RECORD_2 : record_length = 16; break;
		case RECORD_3 : record_length = 40; break;
		default: assert(0 && "Invalid record!"); break;
	}

	int rec_idx = 0;
	for (int i = 12; i < len && rec_idx < *max_records; i += record_length, rec_idx++) {
		record_t *r = &records[rec_idx];

		parse_record_header(buf + i, &r->header);
		switch (type) {
			case RECORD_1 : parse_record_1(buf + i + 8, &r->record.r1); break;
			case RECORD_2 : parse_record_2(buf + i + 8, &r->record.r2); break;
			case RECORD_3 : parse_record_3(buf + i + 8, &r->record.r3);; break;
			default: assert(0 && "Invalid record!"); break;
		}
	}

	*max_records = rec_idx;

	return 0;
}


static int add_tag(smadata2plus_t *sma, int tag_num, const char* tag, const char* tag_message)
{
    struct tag_hash *s;
    s = malloc(sizeof(*s));
    if (s == NULL) {
        LOG_ERROR("malloc failed!");
        return -1;
    }

    s->num = tag_num;
    strncpy(s->tag, tag, sizeof(s->tag) - 1);
    strncpy(s->message, tag_message, sizeof(s->message) - 1);

    HASH_ADD_INT(sma->tags, num, s);

    return 0;
}

static struct tag_hash *find_tag(smadata2plus_t *sma, int num)
{
    struct tag_hash *s;

    HASH_FIND_INT(sma->tags, &num, s);

    return s;
}

static int read_tags(smadata2plus_t *sma, FILE *file) {
	int ret;

	char buf[256];
	char tag_num[256];
	char tag[256];
	char tag_name[256];

	while (fgets(buf, sizeof(buf), file) != NULL ) {
		if (buf[0] == '#') { //remove comments
			continue;
		}

		if (sscanf(buf, "%[^'=']=%[^';'];%s", tag_num, tag, tag_name) != 3) {
			LOG_ERROR("Invalid line %s (Ignoring it)", buf);
			continue;
		}

		errno = 0;
		long num = strtol(tag_num, NULL, 10);
		if (errno != 0) {
			LOG_ERROR("Error parsing tag number from %s", tag_num);
			return -1;
		}

		if ((ret = add_tag(sma, num, tag, tag_name)) < 0) {
			return ret;
		}
	}

	if (ferror(file)) {
		LOG_ERROR("Error reading tag file: %s", strerror(errno));
		fclose(file);
		return -1;
	}

	fclose(file);
	return 0;
}

static void reset_pkt_cnt(uint16_t *pkt_cnt)
{
	*pkt_cnt = 0x8000;
}

static void pkt_cnt_inc(smadata2plus_t *sma)
{
	if ((sma->pkt_cnt < 0x8000) || (sma->pkt_cnt == 0xffff)) {
		sma->pkt_cnt = 0x8000;
	} else {
		sma->pkt_cnt++;
	}
}

static device_t *get_device(device_t *devices, int device_num, uint32_t serial)
{
	for (int i = 0; i < device_num; i++) {
		if (devices[i].serial == serial) {
			return &devices[i];
		}
	}

	return 0;
}

static int serial_to_mac(uint8_t *mac, device_t *devices, int device_num, uint32_t serial)
{
	for (int i = 0; i < device_num; i++) {
		if (devices[i].serial == serial) {
			memcpy(mac, devices[i].mac, 6);
			return 0;
		}
	}
	return -1;
}

static int connect_bluetooth(smadata2plus_t *sma)
{
	if (smabluetooth_connect(sma->sma) < 0) {
		return -1;
	}

	return 0;
}

static int smadata2plus_write_replay(smadata2plus_t *sma,
		smadata2plus_packet_t *packet, uint16_t transaction_cntr)
{
	uint8_t buf[511 + HEADER_SIZE];
	uint8_t size = HEADER_SIZE;
	uint8_t mac_dst[6];

	assert(packet->len + HEADER_SIZE <= sizeof(buf));
	assert(packet->len % 4 == 0);

	memset(buf, 0x00, HEADER_SIZE);

	buf[0] = (packet->len + HEADER_SIZE) / 4;
	buf[1] = packet->ctrl;

	if (packet->dst == SMADATA2PLUS_BROADCAST) {
		buf[2] = 0xff;
		buf[3] = 0xff;
		memcpy(mac_dst, MAC_BROADCAST, 6);
	} else {
		if (serial_to_mac(mac_dst, sma->devices, sma->device_num, packet->dst)
				< 0) {
			LOG_ERROR("device: %d not in device list!", packet->dst);
			return -1;
		}
		buf[2] = 0x4e;
		buf[3] = 0x00;
	}
	byte_store_u32_little(&buf[4], packet->dst);

	buf[9] = packet->flag;

	buf[10] = 0x78;
	buf[11] = 0x00;
	byte_store_u32_little(&buf[12], smadata2plus_serial);

	if (packet->ctrl == 0xe8)
		buf[17] = 0;
	else
		buf[17] = packet->flag;

	if (packet->start)
		buf[20] = packet->packet_num;

	byte_store_u16_little(&buf[22], transaction_cntr);

	memcpy(&buf[size], packet->data, packet->len);

	LOG_TRACE_HEX("write smadata2plus packet", buf, packet->len + size);

	return smanet_write(sma->smanet, buf, size + packet->len, mac_dst);
}

static int smadata2plus_write(smadata2plus_t *sma, smadata2plus_packet_t *packet)
{
	return smadata2plus_write_replay(sma, packet, sma->pkt_cnt);
}


static int smadata2plus_read(smadata2plus_t *sma, smadata2plus_packet_t *packet)
{
	uint8_t buf[512 + HEADER_SIZE];
	int size = HEADER_SIZE;
	int len;

	assert(packet->len <= 512);

	len = smanet_read(sma->smanet, buf, packet->len + HEADER_SIZE, packet->src_mac);
	if (len < 0) {
		LOG_ERROR("smanet_read failed.");
		return -1;
	}

	LOG_TRACE_HEX("read smadata2plus packet", buf, len);

	packet->ctrl = buf[1];
	packet->dst = byte_parse_u32_little(&buf[4]);
	packet->src = byte_parse_u32_little(&buf[12]);
	packet->flag = buf[9];
	packet->start = (buf[23] == 0x80) ? 1 : 0; //Fix
	packet->transaction_cntr = byte_parse_u16_little(&buf[22]);

	len -= HEADER_SIZE;
	if (len < packet->len) packet->len = len;

	memcpy(packet->data, &buf[size], packet->len);

	return 0;
}

/*
 static int sync_online(smadata2plus_t *sma)
 {
 smadata2plus_packet_t packet;
 uint8_t buf[16];
 int ret;

 packet.ctrl = CTRL_MASTER;
 packet.dst  = ADDR_BROADCAST;
 packet.flag = 0x05;
 packet.data = buf;
 packet.len  = sizeof(buf);
 packet.cnt  = 0;
 packet.start = 1;

 memset(buf, 0x00, sizeof(buf));

 buf[0] = 0x0c;
 buf[2] = 0xfd;
 buf[3] = 0xff;

 new_transaction(sma);
 ret = smadata2plus_write(sma, &packet);
 commit_transaction(sma);

 return ret;
 }
 */

/*
 * Request a channel.
 */
static int request_channel(smadata2plus_t *sma,
                           uint16_t channel,
                           uint32_t from_idx,
                           uint32_t to_idx)
{
	smadata2plus_packet_t packet;
	uint8_t buf[12];
	int ret;

	memset(buf, 0x00, sizeof(buf));

	packet.ctrl = CTRL_MASTER;
	packet.dst = ADDR_BROADCAST;
	packet.flag = 0x00;
	packet.data = buf;
	packet.len = sizeof(buf);
	packet.packet_num = 0;
	packet.start = 1;

	buf[0] = 0x00;
	buf[1] = 0x02;

	byte_store_u16_little(&buf[2], channel);
	byte_store_u32_little(&buf[4], from_idx);
	byte_store_u32_little(&buf[8], to_idx);

	ret = smadata2plus_write(sma, &packet);
	pkt_cnt_inc(sma);

	return ret;
}

static int read_records(smadata2plus_t *sma,
                        uint16_t object,
                        uint32_t from_idx,
                        uint32_t to_idx,
                        record_t *records,
                        int *len,
                        record_type_t type)
{
	int ret = 0;
	smadata2plus_packet_t packet;
	uint8_t data[512];

	if ((ret = request_channel(sma, object, from_idx, to_idx)) < 0) {
		LOG_ERROR("Failed requesting %04X %04X % 04X", object, from_idx, to_idx);
		return ret;
	}

	memset(&packet, 0x00, sizeof(packet));
	packet.data = data;
	packet.len = sizeof(data);

	if ((ret = smadata2plus_read(sma, &packet)) < 0) {
		return ret;
	}

	if ((ret = parse_channel_records(data, packet.len, records, len, type)) < 0) {
		LOG_ERROR("Error parsing record of %04X %04X % 04X", object, from_idx, to_idx);
		return ret;
	}

	return 0;
}

static void add_device(smadata2plus_t *sma, uint32_t serial, uint8_t *mac)
{
	device_t *devices;

	devices = realloc(sma->devices, sizeof(*devices) * (sma->device_num + 1));
	if (devices == NULL) {
		LOG_ERROR("realloc failed with %X bytes: %s", sizeof(*devices) * sma->device_num, strerror(
		        errno));
		exit(EXIT_FAILURE);
	}
	devices[sma->device_num].serial = serial;
	memcpy(devices[sma->device_num].mac, mac, 6);

	sma->devices = devices;
	sma->device_num++;
}

/*
 * This is done ones to start connection.
 * This packet does not get a response from inverters.
 */
static int reset_devices(smadata2plus_t *sma)
{
	return request_channel(sma, 0, 0, 0);
}

/*
 * Find all devices in net and extract serial and mac.
 */
static int discover_devices(smadata2plus_t *sma, int device_num)
{
	smadata2plus_packet_t packet;
	uint8_t buf[52];

	if (request_channel(sma, 0, 0, 0) < 0) {
		return -1;
	}

	for (int i = 0; i < device_num; i++) {
		packet.data = buf;
		packet.len = sizeof(buf);

		if (smadata2plus_read(sma, &packet) < 0) {
			return -1;
		}

		add_device(sma, packet.src, packet.src_mac);
	}

	return 0;
}

/*
 * Send password to all devices in network.
 */
static int send_password(smadata2plus_t *sma, const char *password, user_type_t user)
{
	smadata2plus_packet_t packet;
	uint8_t buf[32];
	int i = 0;
	time_t cur_time;

	packet.ctrl = CTRL_MASTER;
	packet.dst = ADDR_BROADCAST;
	packet.flag = 0x01;
	packet.data = buf;
	packet.len = sizeof(buf);
	packet.packet_num = 0;
	packet.start = true;

	memset(buf, 0x00, sizeof(buf));

	buf[0] = 0x0c;
	buf[1] = 0x04;
	buf[2] = 0xfd;
	buf[3] = 0xff;
	buf[4] = 0x07;

	byte_store_u32_little(&buf[8], 40 * 365 * 24 * 60 * 60);

	cur_time = time(NULL);
	LOG_INFO("Sending password %s at %s.", password, ctime(&cur_time));

	byte_store_u32_little(&buf[12], cur_time);

	memset(&buf[20], 0x88, 12);
	for (i = 0; (i < 12) && (password[i] != '\0'); i++) {
		buf[20 + i] = password[i] ^ 0x88;
	}

	return smadata2plus_write(sma, &packet);
}

/*
 * Seems to be only needed for single inverter (netid 1)
 */
int auth_acknowledge(smadata2plus_t *sma, uint32_t serial)
{
	uint8_t buf[8];
	smadata2plus_packet_t packet;

	memset(buf, 0x00, sizeof(buf));

	packet.ctrl = CTRL_MASTER | CTRL_NO_BROADCAST | CTRL_UNKNOWN;
	packet.dst  = serial;
	packet.flag = 0x01;
	packet.data = buf;
	packet.len  = sizeof(buf);
	packet.packet_num  = 0;
	packet.start = 1;

	buf[0] = 0x0d;
	buf[1] = 0x04;
	buf[2] = 0xfd;
	buf[3] = 0xff;
	buf[4] = 0x01;

	return smadata2plus_write(sma, &packet);
}

/*
 * Send password and interpret response from inverter.
 * Answer packet seems to be different depending on inverter number or netid
 * (On multinverter connection password is missing in answer packet, everything else is exactly the same)
 * On netid 1 (single inverter we need 2 send a second packet!?
 */
static int authenticate(smadata2plus_t *sma, const char *password, user_type_t user)
{
	uint8_t buf[52];
	smadata2plus_packet_t packet;

	packet.data = buf;
	packet.len = 52;

	if (send_password(sma, password, user) < 0) {
		LOG_ERROR("Failed sending password!");
		return -1;
	}

	for (int j = 0; j < sma->device_num; j++) {
		if (smadata2plus_read(sma, &packet) < 0) {
			return -1;
		}

		for (int i = 0; i < (i < 12) && (password[i] != '\0'); i++) {
			device_t *device;

			if ((buf[20 + i] ^ 0x88) != password[i]) {
				LOG_INFO("Plant authentication error, serial: %d", packet.src);
			}

			device = get_device(sma->devices, sma->device_num, packet.src);
			if (device == NULL) {
				LOG_WARNING("Got authentication answer of non registered device: %d", packet.src);
			}
			device->authenticated = true;
		}
	}

	if (sma->device_num == 1) {
		if (auth_acknowledge(sma, sma->devices[0].serial) < 0) {
			return -1;
		}
	}

	pkt_cnt_inc(sma);

	return 0;
}

#if 0
static int send_time(smadata2plus_t *sma)
{
	uint8_t buf[40];
	smadata2plus_packet_t packet;
	time_t cur_time;
	int ret;

	memset(buf, 0x00, sizeof(buf));

	buf[0] = 0x0a;
	buf[1] = 0x02;
	buf[3] = 0xf0;
	buf[5] = 0x6d;
	buf[6] = 0x23;
	buf[9] = 0x6d;
	buf[10] = 0x23;
	buf[13] = 0x6d;
	buf[14] = 0x23;

	cur_time = time(NULL);
	LOG_INFO("send_time: %s", ctime(&cur_time));

	/* FIXME: time ??? */
	byte_store_u32_little(&buf[16], cur_time);
	byte_store_u32_little(&buf[20], cur_time - 10);
	byte_store_u32_little(&buf[24], cur_time);
	byte_store_u32_little(&buf[28], 0x00000e11);
	byte_store_u32_little(&buf[32], 0x007eff03);

	buf[36] = 1;

	packet.ctrl = CTRL_MASTER;
	packet.dst = ADDR_BROADCAST;
	packet.flag = 0x00;
	packet.data = buf;
	packet.len = sizeof(buf);
	packet.packet_num = 0;
	packet.start = 1;

	ret = smadata2plus_write(sma, &packet);

	return ret;
}

#endif

/*
 static int read_archiv_channel(smadata2plus_t *sma, uint32_t address, uint32_t time_from, uint32_t time_to)
 {
 smadata2plus_packet_t packet;
 uint8_t buf[16];
 int ret;

 packet.ctrl = CTRL_MASTER | CTRL_NO_BROADCAST;
 packet.dst  = address;
 packet.flag = 0x00;
 packet.data = buf;
 packet.len  = sizeof(buf);
 packet.cnt   = 0;
 packet.start = 1;

 memset(buf, 0x00, sizeof(buf));

 buf[1] = 0x02;
 buf[3] = 0x70;

 byte_store_u32_little(&buf[4], time_from);
 byte_store_u32_little(&buf[8], time_to);

 new_transaction(sma);
 ret = smadata2plus_write(sma, &packet);
 commit_transaction(sma);

 return ret;
 }
 */
/*
 int smadata2plus_read_channel(smadata2plus_t *sma, uint16_t channel, uint32_t from_idx, uint32_t to_idx)
 {
 smadata2plus_packet_t packet;

 if (read_channel(sma, channel, from_idx, to_idx) < 0) {
 return -1;
 }

 packet.len  = 0;
 packet.data = NULL;

 if (smadata2plus_read(sma, &packet) < 0) {
 LOG_ERROR("Read failed!");
 return -1;
 }

 return 0;
 }
 */

#if 0
static int cmd_A008(smadata2plus_t *sma)
{
	uint8_t buf[8];
	smadata2plus_packet_t packet;
	int ret;

	buf[0] = 0x0e;
	buf[1] = 0x01;
	buf[2] = 0xfd;
	memset(&buf[3], 0xff, 5);

	packet.ctrl = CTRL_MASTER;
	packet.dst = ADDR_BROADCAST;
	packet.flag = 0x03;
	packet.data = buf;
	packet.len = sizeof(buf);
	packet.packet_num = 0;
	packet.start = 1;

	ret = smadata2plus_write(sma, &packet);

	return ret;
}

static int cmd_E808(smadata2plus_t *sma, uint32_t serial)
{
	uint8_t buf[8];
	smadata2plus_packet_t packet;

	memset(buf, 0x00, 8);
	buf[0] = 0x0d;
	buf[1] = 0x04;
	buf[2] = 0xfd;
	buf[3] = 0xff;
	buf[4] = 0x01;

	//   packet.cmd  = CMD_UNKNOWN;
	packet.ctrl = CTRL_MASTER | CTRL_NO_BROADCAST | CTRL_UNKNOWN;
	packet.dst = serial;
	packet.flag = 0x01;
	packet.data = buf;
	packet.len = sizeof(buf);
	packet.packet_num = 0;
	packet.start = 1;

	//new_transaction(sma);
	return smadata2plus_write(sma, &packet);
	// commit_transaction(sma);
}
#endif

#if 0
static int time_setup(smadata2plus_t *sma)
{
	//    smadata2plus_packet_t packet;
	//    uint8_t buf[42];
	//    uint8_t unknown[6];
	//    uint8_t unknown2[5];
	//    uint32_t time1;
	//    uint32_t time2;
	//    uint32_t serial;


	if (send_time(sma) < 0) return -1;
	/*
	 packet.len  = sizeof(buf);
	 packet.data = buf;


	 if (smadata2plus_read(sma, &packet) < 0) {
	 LOG_ERROR("smadata2plus_read failed!");
	 return -1;
	 }

	 if (packet.cmd != CMD_TIME) {
	 LOG_ERROR("Invalid packet cmd!");
	 return -1;
	 }

	 memcpy(unknown, buf, 6);
	 time1 = byte_parse_u32_little(&buf[18]);
	 time2 = byte_parse_u32_little(&buf[22]);
	 memcpy(unknown2, &buf[30], 5);

	 serial = packet.src;

	 LOG_INFO("time answer1: %s", ctime((time_t*)&time1));
	 LOG_INFO("time answer2: %s", ctime((time_t*)&time2));

	 memset(&packet, 0x00, sizeof(packet));
	 memset(buf, 0x00, 10);

	 memcpy(buf, unknown, 6);
	 buf[6] = 0x01;

	 packet.cmd  = CMD_UNKNOWN;
	 packet.ctrl = CTRL_MASTER | CTRL_UNKNOWN | CTRL_NO_BROADCAST;
	 packet.dst  = serial;
	 packet.flag = 0x00;
	 packet.data = buf;
	 packet.len  = 10;
	 packet.cnt   = 0xff;
	 packet.start = 0;

	 if (smadata2plus_write(sma, &packet) < 0) {
	 LOG_ERROR("smadata2plus_write failed!");
	 return -1;
	 }

	 memset(&packet, 0x00, sizeof(packet));
	 memset(buf, 0x00, 40);


	 packet.cmd  = CMD_TIME;
	 packet.ctrl = CTRL_MASTER;
	 packet.dst  = ADDR_BROADCAST;
	 packet.flag = 0x00;
	 packet.data = buf;
	 packet.len  = 40;
	 packet.cnt   = 0;
	 packet.start = 1;

	 buf[0] = 0x0a;
	 buf[1] = 0x02;
	 buf[3] = 0xf0;
	 buf[5] = 0x6d;
	 buf[6] = 0x23;
	 buf[9] = 0x6d;
	 buf[10] = 0x23;
	 buf[13] = 0x6d;
	 buf[14] = 0x23;

	 byte_store_u32_little(&buf[16], time1);
	 byte_store_u32_little(&buf[20], time2);
	 byte_store_u32_little(&buf[24], time1);

	 memcpy(&buf[28], unknown2, 5);
	 buf[33] = 0xfe;
	 buf[34] = 0x7e;
	 buf[36] = 0x01;

	 if (smadata2plus_write(sma, &packet) < 0) {
	 LOG_ERROR("smadata2plus_write failed!");
	 return -1;
	 }
	 */
	return 0;
}

#endif
/*
 static int read_status(smadata2plus_t *sma)
 {
 smadata2plus_packet_t packet;

 packet.len  = 0;
 packet.data = NULL;

 read_channel(sma, 0x5800, 0x821e00, 0x8221ff);
 read_channel(sma, 0x5800, 0xa21e00, 0xa21eff);
 read_channel(sma, 0x5180, 0x214800, 0x2148ff);

 return 0;
 }
 */

static smadata2plus_t *init(connection_t *con)
{
	smadata2plus_t *sma;
	smabluetooth_t *smabluetooth;
	smanet_t *smanet;

	smabluetooth = smabluetooth_init(con);
	if (smabluetooth == NULL) return NULL;

	smanet = smanet_init(PROTOCOL, NULL, smabluetooth);
	if (smanet == NULL) return NULL;

	sma = calloc(1, sizeof(*sma));

	sma->con = con;
	sma->sma = smabluetooth;
	sma->smanet = smanet;
	sma->tags = NULL;

	reset_pkt_cnt(&sma->pkt_cnt);

	return sma;
}

void smadata2plus_close(protocol_t *protocol)
{
	smadata2plus_t *sma = protocol->handle;
	smabluetooth_close(sma->sma);
	smanet_close(sma->smanet);

	free(sma);
	free(protocol);
}
/*
 int test_channel(smadata2plus_t *sma, uint8_t channel, uint8_t type, uint32_t from, uint16_t to)
 {
 smadata2plus_packet_t packet;

 packet.len  = 0;
 packet.data = NULL;


 if (read_channel(sma, type, channel, from, to) < 0) return -1;

 return smadata2plus_read(sma, &packet);
 }
 */

static int sync_time(smadata2plus_t *sma) {
	smadata2plus_packet_t packet;
	uint8_t buf[40];
	int ret;

	packet.ctrl = CTRL_MASTER;
	packet.dst = ADDR_BROADCAST;
	packet.flag = 0x00;
	packet.data = buf;
	packet.len = 40;
	packet.packet_num = 0;
	packet.start = 1;

	byte_store_u32_little(buf, 0xf000020a);
	byte_store_u32_little(buf + 4, 0x00236d00);
	byte_store_u32_little(buf + 8, 0x00236d00);
	byte_store_u32_little(buf + 12, 0x00236d00);
	byte_store_u32_little(buf + 16, 0);
	byte_store_u32_little(buf + 20, 0);
	byte_store_u32_little(buf + 24, 0);
	byte_store_u32_little(buf + 24, 0);
	byte_store_u32_little(buf + 28, 1);
	byte_store_u32_little(buf + 32, 1);

	if ((ret = smadata2plus_write(sma, &packet)) < 0) {
		LOG_ERROR("Error reading inverter date!");
		return ret;
	}

	if ((ret = smadata2plus_read(sma, &packet)) < 0) {
		LOG_ERROR("smadata2plus_read failed!");
		return ret;
	}

	if (packet.len != 40) {
		LOG_ERROR("Invalid packet!");
		return -1;
	}

	time_t last_adjusted = byte_parse_u32_little(buf + 20);
	LOG_INFO("Time last adjusted: %s", ctime(&last_adjusted));

	time_t inverter_time1 = byte_parse_u32_little(buf + 16);
	time_t inverter_time2 = byte_parse_u32_little(buf + 24);
	LOG_INFO("Inverter time 1: %s", ctime(&inverter_time1));
	LOG_INFO("Inverter time 2: %s", ctime(&inverter_time2));

	uint32_t tz_dst = byte_parse_u32_little(buf + 28);
	int tz = tz_dst & 0xfffffe;
	int dst = tz_dst & 0x1;
	uint32_t unknown = byte_parse_u32_little(buf + 32);
	uint16_t transaction_cntr = packet.transaction_cntr;

	memset(&packet, 0x00, sizeof(packet));
	memset(buf, 0x00, sizeof(buf));

	packet.ctrl = CTRL_MASTER | CTRL_UNKNOWN | CTRL_NO_BROADCAST;
	packet.dst = sma->devices[0].serial;
	packet.flag = 0x00;
	packet.data = buf;
	packet.len = 8;
	packet.packet_num = 0;
	packet.start = 0;

	byte_store_u32_little(buf, 0xf000020a);
	byte_store_u32_little(buf + 4, 0x1);

	if ((ret = smadata2plus_write_replay(sma, &packet, transaction_cntr)) < 0) {
		LOG_ERROR("Error writing time ack!");
		return ret;
	}

	time_t cur_time = time(NULL);

	if ((abs(cur_time - inverter_time1)) > 10) {
		memset(&packet, 0x00, sizeof(packet));
		memset(buf, 0x00, sizeof(buf));

		byte_store_u32_little(buf, 0xf000020a);
		byte_store_u32_little(buf + 4, 0x00236d00);
		byte_store_u32_little(buf + 8, 0x00236d00);
		byte_store_u32_little(buf + 12, 0x00236d00);

		byte_store_u32_little(buf + 16, cur_time);
		byte_store_u32_little(buf + 20, cur_time);
		byte_store_u32_little(buf + 24, cur_time);
		byte_store_u32_little(buf + 28, dst | tz);
		byte_store_u32_little(buf + 32, unknown);
		byte_store_u32_little(buf + 36, 1);

		packet.ctrl = CTRL_MASTER;
		packet.dst = ADDR_BROADCAST;
		packet.flag = 0x00;
		packet.data = buf;
		packet.len = sizeof(buf);
		packet.packet_num = 0;
		packet.start = 1;

		if ((ret = smadata2plus_write(sma, &packet)) < 0) {
			LOG_ERROR("Error setting date!");
			return ret;
		}
	}

	return 0;
}

int smadata2plus_connect(protocol_t *prot, const char *password, const void *param)
{
	smadata2plus_t *sma;
	int device_num;
	int ret;
	int cnt = 0;

	sma = (smadata2plus_t*)prot->handle;

	if ((ret = connect_bluetooth(sma)) < 0) {
	    LOG_ERROR("Connecting bluetooth failed!");
	    return ret;
	}

	device_num = smabluetooth_device_num(sma->sma);
	LOG_INFO("%d devices!", device_num);

	do {
		ret = discover_devices(sma, device_num);
		if (cnt > NUM_RETRIES && ret < 0) {
			LOG_ERROR("Device discover failed!");
			return ret;
		} else if (ret < 0){
			LOG_WARNING("Device discover failed! Retrying ...");
			cnt++;
			thread_sleep(1000 * cnt);
		}
	} while (ret < 0);
	cnt = 0;

	do {
		ret = authenticate(sma, password, PASSWORD_USER);
		if (cnt > NUM_RETRIES && ret < 0) {
			LOG_ERROR("Authentication  failed!");
			return ret;
		} else if (ret < 0){
			LOG_WARNING("Authentication failed! Retrying ...");
			cnt++;
			thread_sleep(1000 * cnt);
		}
	} while (ret < 0);
	cnt = 0;

	do {
		ret = sync_time(sma);
		if (cnt > NUM_RETRIES && ret < 0) {
			LOG_ERROR("Sync time failed!");
			return ret;
		} else if (ret < 0){
			LOG_WARNING("Sync time failed! Retrying ...");
			cnt++;
			thread_sleep(1000 * cnt);
		}
	} while (ret < 0);
	cnt = 0;
	LOG_INFO("Synchronized time!");

	return 0;

	/*
	 if (time_setup(sma) < 0) {
	 LOG_ERROR("Time setup failed!");
	 return -1;
	 }
	 */
	/*
	 if (read_status(sma) < 0) {
	 LOG_ERROR("read status failed!");
	 return -1;
	 }

	 packet.len  = 0;
	 packet.data = NULL;

	 if (smadata2plus_read(sma, &packet) < 0) {
	 LOG_ERROR("Read failed!");
	 return -1;
	 }

	 packet.len  = 0;
	 packet.data = NULL;

	 if (smadata2plus_read(sma, &packet) < 0) {
	 LOG_ERROR("Read failed!");
	 return -1;
	 }

	 packet.len  = 0;
	 packet.data = NULL;

	 if (smadata2plus_read(sma, &packet) < 0) {
	 LOG_ERROR("Read failed!");
	 return -1;
	 }

	 read_channel(sma, 0x5800, 0x870000, 0x87ffff);

	 packet.len  = 0;
	 packet.data = NULL;

	 if (smadata2plus_read(sma, &packet) < 0) {
	 LOG_ERROR("Read failed!");
	 return -1;
	 }

	 return 0;
	 */
}

static int get_ac(protocol_t *prot, uint32_t id, pvlib_ac_t *ac)
{
	smadata2plus_t *sma = prot->handle;;
	int ret;
	int cnt = 0;
	record_t records[20];
	int num_recs = 20;

	do {
		ret = read_records(sma, 0x5100, 0x200000, 0x50ffff, records, &num_recs, RECORD_1);
		if (cnt > NUM_RETRIES && ret < 0) {
			LOG_ERROR("Reading dc spot data  failed!");
			return ret;
		} else if (ret < 0){
			LOG_WARNING("Reading dc spot data failed! Retrying ...");
			cnt++;
			thread_sleep(1000 * cnt);
		}
	} while (ret < 0);

	for (int i = 0; i < num_recs; i++) {
		record_t *r = &records[i];

		uint32_t value = r->record.r1.value2;
		LOG_DEBUG("Read idx %d value: %d", r->header.idx, value);

		switch(r->header.idx) {
		case TOTAL_POWER:
			ac->current_power = value;
			break;
		case MAX_PHASE1:
			break;
		case MAX_PHASE2:
			break;
		case MAX_PHASE3:
			break;
		case UNKNOWN_1:
			LOG_DEBUG("UNKNOWN_1, %d", value);
			break;
		case UNKNWON_2:
			LOG_DEBUG("UNKNOWN_2, %d", value);
			break;
		case POWER_PHASE1:
			ac->power[0] = value;
			break;
		case POWER_PHASE2:
			ac->power[1] = value;
			break;
		case POWER_PHASE3:
			ac->power[2] = value;
			break;
		case VOLTAGE_PHASE1:
			ac->voltage[0] = value * 1000 / VOLTAGE_DIVISOR;
			break;
		case VOLTAGE_PHASE2:
			ac->voltage[1] = value * 1000 / VOLTAGE_DIVISOR;
			break;
		case VOLTAGE_PHASE3:
			ac->voltage[2] = value * 1000 / VOLTAGE_DIVISOR;
			break;
		case CURRENT_PHASE1:
			ac->current[0] = value * 1000 / CURRENT_DIVISOR;
			break;
		case CURRENT_PHASE2:
			ac->current[1] = value * 1000 / CURRENT_DIVISOR;
			break;
		case CURRENT_PHASE3:
			ac->current[2] = value * 1000 / CURRENT_DIVISOR;
			break;
		case FREQUENCE:
			ac->frequence=value * 1000 / FREQUENCE_DIVISOR;
			break;
		default:
			break;
		}
	}

	return 0;
}

static int get_dc(protocol_t *prot, uint32_t id, pvlib_dc_t *dc)
{
	smadata2plus_t *sma = prot->handle;;
	int ret;
	int cnt = 0;
	record_t records[9];
	int num_recs = 9;

	do {
		ret = read_records(sma, 0x5380, 0x200000, 0x5000ff, records, &num_recs, RECORD_1);
		if (cnt > NUM_RETRIES && ret < 0) {
			LOG_ERROR("Reading dc spot data  failed!");
			return ret;
		} else if (ret < 0){
			LOG_WARNING("Reading dc spot data failed! Retrying ...");
			cnt++;
			thread_sleep(1000 * cnt);
		}
	} while (ret < 0);

	for (int i = 0; i < num_recs; i++) {
		record_t *r = &records[i];
		uint32_t value = r->record.r1.value2;

		LOG_DEBUG("Read idx %d value: %d", r->header.idx, value);

		int tracker = r->header.cnt;
		if (tracker < 1) {
			LOG_ERROR("Invalid tracker number: %d", tracker);
			continue;
		}

		switch (r->header.idx) {
		case DC_POWER:
			dc->power[tracker - 1] = value;
			break;
		case DC_VOLTAGE:
			dc->voltage[tracker - 1] = value * 1000 / VOLTAGE_DIVISOR;
			break;
		case DC_CURRENT:
			dc->current[tracker - 1] = value * 1000 / CURRENT_DIVISOR;
			break;
		default:
			break;
		}
	}

	return 0;
}

static int get_stats(protocol_t *prot, uint32_t id, pvlib_stats_t *stats)
{
	smadata2plus_t *sma = prot->handle;;
	int ret;
	int cnt = 0;
	record_t records[4];
	int num_recs = 4;

	do {
		ret = read_records(sma, 0x5400, 0x20000, 0x50ffff, records, &num_recs, RECORD_2);
		if (cnt > NUM_RETRIES && ret < 0) {
			LOG_ERROR("Reading stats  failed!");
			return ret;
		} else if (ret < 0){
			LOG_WARNING("Reading stats failed! Retrying ...");
			cnt++;
			thread_sleep(1000 * cnt);
		}
	} while (ret < 0);

	for (int i = 0; i < num_recs; i++) {
		record_t *r = &records[i];

		uint64_t value = r->record.r2.value;

		LOG_DEBUG("Read stats: idx %x value: %"PRIu64, r->header.idx, value);

		switch (r->header.idx) {
		case STAT_TOTAL_YIELD:
			stats->total_yield = (uint32_t)value;
			break;
		case STAT_DAY_YIELD:
			stats->day_yield = (uint32_t)value;
			break;
		case STAT_OPERATION_TIME:
			stats->operation_time = (uint32_t)value;
			break;
		case STAT_FEED_IN_TIME:
			stats->feed_in_time = (uint32_t)value;
			break;
		default:
			break;
		}
	}

	return 0;
}

static int get_status(protocol_t* prot, uint32_t id, pvlib_status_t *status)
{
	smadata2plus_t *sma = prot->handle;
	int ret;
	int cnt = 0;
	record_t records[1];
	int num_recs = 1;

	do {
		ret = read_records(sma, 0x5800, 0x214800, 0x2148ff, records, &num_recs, RECORD_3);
		if (cnt > NUM_RETRIES && ret < 0) {
			LOG_ERROR("Reading inverter info  failed!");
			return ret;
		} else if (ret < 0){
			LOG_WARNING("Reading inverter info failed! Retrying ...");
			cnt++;
			thread_sleep(1000 * cnt);
		}
	} while (ret < 0);

	status->number = 0;
	status->status = PVLIB_STATUS_UNKNOWN;

	for (int i = 0; i < num_recs; i++) {
		record_t *r = &records[i];
		uint8_t *d = r->record.r3.data;

		switch(r->header.idx) {
		case DEVICE_STATUS: {
			attribute_t attributes[8];
			int num_attributes = 8;
			parse_attributes(d, sizeof(r->record.r3.data), attributes, &num_attributes);
			for (int i = 0; i < num_attributes; i++) {
				if (attributes[i].selected) {
					status->number = attributes[i].attribute;
					switch (status->number) {
					case 307: status->status = PVLIB_STATUS_OK; break;
					case 35: status->status = PVLIB_STATUS_ERROR; break;
					case 303: status->status = PVLIB_STATUS_OFF; break;
					case 455: status->status = PVLIB_STATUS_WARNING; break;
					default: status->status = PVLIB_STATUS_UNKNOWN; break;
					}
				}
			}
			break;
		}
		default:
			LOG_ERROR("Unexpected idx: %x", r->header.idx);
			break;
		}

	}

	return 0;
}

//version needs to be 10 at least 10 bytes
static int parse_firmware_version(uint8_t *data, char *version)
{
	//firmware version is stored from byte 16 + to 19

	if (data[18] > 9 || data[19] > 9) {
		LOG_ERROR("Invalid firmware version: 0x%02x%02x%02x%02x", data[16], data[17], data[18], data[19]);
		return -1;
	}

	char release_type[3] = {0};

	switch (data[16]) {
		case 0:release_type[0] = 'N'; break;
		case 1:release_type[0] = 'E'; break;
		case 2:release_type[0] = 'A'; break;
		case 3:release_type[0] = 'B'; break;
		case 4:release_type[0] = 'R'; break;
		case 5:release_type[0] = 'S'; break;
		default: snprintf(release_type, 3, "%02d", data[16]); break;
	}

	sprintf(version, "%d.%02d.%02d.%s", data[19], data[18], data[17], release_type);

	return 0;
}

static int get_inverter_info(protocol_t* prot, uint32_t id, pvlib_inverter_info_t *inverter_info)
{
	smadata2plus_t *sma = prot->handle;;
	int ret;
	int cnt = 0;
	record_t records[10];
	int num_recs = 10;

	do {
		ret = read_records(sma, 0x5800, 0x821e00, 0x8234FF, records, &num_recs, RECORD_3);
		if (cnt > NUM_RETRIES && ret < 0) {
			LOG_ERROR("Reading inverter info  failed!");
			return ret;
		} else if (ret < 0){
			LOG_WARNING("Reading inverter info failed! Retrying ...");
			cnt++;
			thread_sleep(1000 * cnt);
		}
	} while (ret < 0);

	memset(inverter_info, 0, sizeof(*inverter_info));
	strcpy(inverter_info->manufacture, "SMA");

	for (int i = 0; i < num_recs; i++) {
		record_t *r = &records[i];
		uint8_t * d = r->record.r3.data;

		switch (r->header.idx) {
		case DEVICE_NAME:
			if (strncmp((char*)d, "SN: ", 4) != 0) {
				LOG_WARNING("Unexpected device name!");
			}
			strncpy(inverter_info->name, (const char*)d, 32);
			break;
		case DEVICE_CLASS: {
			attribute_t attributes[8];
			int num_attributes = 8;
			parse_attributes(d, sizeof(r->record.r3.data), attributes, &num_attributes);
			for (int i = 0; i < num_attributes; i++) {
				if (attributes[i].selected) {
					LOG_DEBUG("Device class: %d", attributes[i].attribute);
				}
			}
			break;
		}
		case DEVICE_TYPE: {
			attribute_t attributes[8];
			int num_attributes = 8;
			parse_attributes(d, sizeof(r->record.r3.data), attributes, &num_attributes);
			for (int i = 0; i < num_attributes; i++) {
				if (attributes[i].selected) {
					LOG_DEBUG("Device type: %d", attributes[i].attribute);
					snprintf(inverter_info->type, sizeof(inverter_info->type), "%d", attributes[i].attribute);

				}
			}
			break;
		}
		case DEVICE_UNKNOWN:
			break;
		case DEVICE_SWVER:
			if (parse_firmware_version(d, inverter_info->firmware_version) < 0) {
				LOG_WARNING("Invalid firmware format. Ignoring it!");
			}
			break;
		default:
			break;
		}
	}

	return 0;
}

#if 0
int smadata2plus_test(smadata2plus_t *sma)
{
	smadata2plus_packet_t packet;
	uint8_t data[512];
	int pos = 0;
	int i = 0;

	//    sync_online(sma);

	if (read_channel(sma, 0x80, 0x54, 0x2000, 0x50ff) < 0) return -1;

	memset(&packet, 0x00, sizeof(packet));
	packet.data = data;
	packet.len = sizeof(data);

	if (smadata2plus_read(sma, &packet) < 0) {
		return -1;
	}

	pos = 13;
	i = 0;
	while (pos + 11 < packet.len) {
		uint32_t value = byte_parse_u32_little(&data[pos + 7]);
		uint32_t value_time = byte_parse_u32_little(&data[pos + 3]);

		LOG_INFO("value unknwon_%d, time: %s : %d", i++, ctime((time_t *)&value_time), value);
		pos += 16;
	}
	return 0;
}
#endif

static int smadata2plus_device_num(protocol_t *prot)
{
	smadata2plus_t *sma;

	sma = (smadata2plus_t*) prot->handle;
	return sma->device_num;
}

static int smadata2plus_get_devices(protocol_t *prot, uint32_t* ids, int max_num)
{
	smadata2plus_t *sma = (smadata2plus_t*) prot->handle;

	for (int i = 0; i < max_num && i < sma->device_num; i++) {
		ids[i] = sma->devices[i].serial;
	}

	return 0;
}

int smadata2plus_read_channel(smadata2plus_t *sma, uint16_t channel, uint32_t idx1, uint32_t idx2)
{
	int ret;
	smadata2plus_packet_t packet;
	uint8_t data[512];

	if ((ret = request_channel(sma, channel, idx1, idx2)) < 0) {
		return ret;
	}

	memset(&packet, 0x00, sizeof(packet));
	packet.data = data;
	packet.len = sizeof(data);

	if ((ret = smadata2plus_read(sma, &packet)) < 0) {
		return ret;
	}

	printf("\n\n\n");
	for (int i = 0; i < packet.len; i++) {
		printf("%02x ", packet.data[i]);
		if ((i + 1) % 20 == 0) {
			printf("\n");
		}
	}
	printf("\n");
	for (int i = 0; i < packet.len; i++) {
		if (packet.data[i] >= 20)
			printf("%c  ", packet.data[i]);
		else
			printf("   ");
		if ((i + 1) % 20 == 0) {
			printf("\n");
		}
	}
	printf("\n\n\n");

	return 0;
}

static void disconnect(protocol_t *prot) {
	smadata2plus_t *sma = (smadata2plus_t*) prot->handle;
	smabluetooth_disconnect(sma->sma);
}

int smadata2plus_open(protocol_t *prot, connection_t *con, const char* params) {
	smadata2plus_t *sma;
	int ret;

	sma = init(con);

	if (sma == NULL)
		return -1;

	FILE *tag_file = NULL;
	size_t file_length = strlen("en_US_tags.txt");
	char tag_path[strlen(resources_path()) + 1 + file_length + 1];
	if (params != NULL && strlen(params) == 5) {
		;
		strcpy(tag_path, resources_path());
		strcat(tag_path, params);
		strcat(tag_path, "_tags.txt");

		tag_file = fopen(tag_path, "r");
		if (tag_file == NULL) {
			LOG_ERROR("tag file for local %s doesn't exist.", params);
		}
	}

	if (tag_file == NULL) {
		strcpy(tag_path, resources_path());
		strcat(tag_path, "en_US_tags.txt");
		tag_file = fopen(tag_path, "r");
	}

	if (tag_file == NULL) {
		LOG_ERROR("tag file  %s doesn't exist.", tag_path);
		return -1;
	}

	//params only contains filename to tag file
	if ((ret = read_tags(sma, tag_file)) < 0) {
		return ret;
	}

	prot->handle = sma;
	prot->inverter_num = smadata2plus_device_num;
	prot->get_devices = smadata2plus_get_devices;
	prot->connect = smadata2plus_connect;
	prot->disconnect = disconnect;
	prot->get_stats = get_stats;
	prot->get_ac = get_ac;
	prot->get_dc = get_dc;
	prot->get_status = get_status;
	prot->close = smadata2plus_close;
	prot->get_inverter_info = get_inverter_info;

	return 0;
}

const protocol_info_t protocol_info_smadata2plus = {
		"smadata2plus",
		"pvlogdev",
		"",
        smadata2plus_open
};
