/*
 *   Pvlib - Connection interface
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

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <ctime>

#include "pvlib.h"
#include "Smadata2plus.h"

static void print_usage() {
	printf("Usage: pvlib MAC PASSWORD\n");
	printf("Example: pvlib \"00:11:22:33:44:55\" \"0000\"\n");
}

int main(int argc, char **argv) {
	pvlib_plant *plant;

	pvlib_ac ac;
	pvlib_dc dc;
	pvlib_stats stats;
	pvlib_status status;
	pvlib_inverter_info inverter_info;

	int inv_num;
	uint32_t inv_handle;

	int found;
	int i;

	int con_num;
	uint32_t con_handles[10];
	int prot_num;
	uint32_t prot_handles[10];

	uint32_t con;
	uint32_t prot;

//    smadata2plus_t *sma;

	if (argc < 3) {
		print_usage();
		return -1;
	}

	//Initialize pvlib
	pvlib_init(stderr);

	con_num = pvlib_connections(con_handles, 10);

	found = 0;
	for (i = 0; i < con_num; i++) {
		if (strcmp(pvlib_connection_name(con_handles[i]), "rfcomm") == 0) {
			found = 1;
			break;
		}
	}

	if (!found) {
		fprintf(stderr, "connection rfcomm not available!\n");
		return EXIT_FAILURE;
	}

	con = con_handles[i];

	prot_num = pvlib_protocols(prot_handles, 10);

	found = 0;
	for (i = 0; i < prot_num; i++) {
		if (strcmp(pvlib_protocol_name(prot_handles[i]), "smadata2plus") == 0) {
			found = 1;
			break;
		}
	}

	if (!found) {
		fprintf(stderr, "protocol smadata2plus not available!\n");
		return EXIT_FAILURE;
	}

	prot = prot_handles[i];

	plant = pvlib_open(con, prot, NULL, NULL);
	if (plant == NULL) {
		fprintf(stderr, "Failed opening plant!\n");
		return EXIT_FAILURE;
	}

	if (pvlib_connect(plant, argv[1], argv[2], NULL, NULL) < 0) {
		fprintf(stderr, "Failed connection with plant!\n");
		return EXIT_FAILURE;
	}

	inv_num = pvlib_num_string_inverter(plant);

	if (inv_num <= 0) {
		fprintf(stderr, "no inverters found!\n");
		return EXIT_FAILURE;
	}

	if (inv_num > 1) {
		fprintf(stderr, "more than %d inverter, but only 1 is supported!\n",
				inv_num);
		return EXIT_FAILURE;
	}
	/*
	 if (pvlib_device_handles(plant, &inv_handle, 1) < 0) {
	 fprintf(stderr, "Failed getting inverter handle!\n");
	 return EXIT_FAILURE;
	 }
	 */
	if (pvlib_device_handles(plant, &inv_handle, 1) != 1) {
		fprintf(stderr, "Error getting inverter handle\n");
		return -1;
	}

	for (i = 0; i < 1; i++) {
		if (pvlib_get_ac_values(plant, inv_handle, &ac) < 0) {
			fprintf(stderr, "get live values failed!\n");
			return -1;
		}

		if (pvlib_get_dc_values(plant, inv_handle, &dc) < 0) {
			fprintf(stderr, "get live values failed!\n");
			return -1;
		}

		if (pvlib_get_stats(plant, inv_handle, &stats) < 0) {
			fprintf(stderr, "get stats failed!\n");
			return -1;
		}

		if (pvlib_get_status(plant, inv_handle, &status) < 0) {
			fprintf(stderr, "get stats failed!\n");
			return -1;
		}

		if (pvlib_get_inverter_info(plant, inv_handle, &inverter_info) < 0) {
			fprintf(stderr, "get stats failed!\n");
			return -1;
		}
		printf("Manufacture: %s\n", inverter_info.manufacture);
		printf("Type: %s\n", inverter_info.type);
		printf("Name: %s\n", inverter_info.name);
		printf("Firmware: %s\n", inverter_info.firmware_version);

		printf("status: %d %d\n", status.status, status.number);

		sleep(1);
	}

	time_t to = time(0);
	time_t from = to - 24 * 60 * 60 * 7;

	int days = 0;
	pvlib_day_yield *dayYield;
	if ((days = pvlib_get_day_yield(plant, inv_handle, from, to, &dayYield)) < 0) {
		fprintf(stderr, "get day yield failed\n");
		return -1;

	}

	for (int i = 0; i < days; ++i) {
		printf("%s: %d\n", ctime(&dayYield[i].date), (int32_t)dayYield[i].dayYield);
	}

	pvlib_event *events;
	int eventNum;
	from = 0;
	if ((eventNum = pvlib_get_events(plant, inv_handle, from, to, &events)) < 0) {
		fprintf(stderr, "get day events failed\n");
		return -1;

	}

	for (int i = 0; i < eventNum; ++i) {
		printf("%s: %s (%d)\n", ctime(&events[i].time), events[i].message, events[i].value);
	}

//	time_t to = time(0);
//	time_t from = to - 24 * 60 * 60 * 7;
//
//	Smadata2plus *sma = (Smadata2plus*) pvlib_protocol_handle(plant);
//
//	std::vector<Smadata2plus::EventData> eventData;
//	if (sma->readEventData(inv_handle, from, to, Smadata2plus::USER, eventData)
//			< 0) {
//		fprintf(stderr, "Error reading event data!");
//		return -1;
//	}
//
//	printf("Got %d events\n", eventData.size());
//	for (const Smadata2plus::EventData& event : eventData) {
//		time_t et = static_cast<time_t>(event.time);
//		printf("Time: %s Tag: %d Code: %d Flags: %d\n", ctime(&et), event.tag,
//				event.eventCode, event.eventFlags);
//	}
//
//	std::vector<Smadata2plus::TotalDayData> dayData;
//	if (sma->readTotalDayData(inv_handle, 0, to, dayData) < 0) {
//		fprintf(stderr, "Error reading day data!");
//		return -1;
//	}
//
//	printf("Got %d days\n", dayData.size());
//	for (const Smadata2plus::TotalDayData& day : dayData) {
//		time_t et = static_cast<time_t>(day.time);
//		printf("Time: %s Total: %d\n", ctime(&et), (int32_t)day.totalYield);
//	}

//    if (sma == NULL) {
//        fprintf(stderr, "Could not get native connection handle!");
//        return -1;
//    }
//
//
//    for (;;) {
//        uint16_t channel;
//        uint32_t from_index;
//        uint32_t to_index;
//        char buf[20];
//
//        printf("quit y, n?");
//        fgets(buf, 20, stdin);
//
//        if (buf[0] == 'y') break;
//
//        printf("channel: ");
//        fgets(buf, 20, stdin);
//        channel = strtol(buf, NULL, 16);
//
//        printf("from index: ");
//        fgets(buf, 20, stdin);
//        from_index = strtol(buf, NULL, 16);
//
//        printf("to index: ");
//        fgets(buf, 20, stdin);
//        to_index = strtol(buf, NULL, 16);
//
//        smadata2plus_read_channel(sma, channel, from_index, to_index);
//    }

	pvlib_close(plant);
	pvlib_shutdown();

	return 0;
}
