/*
 * main.c
 */
#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "main.h"
#include "ssd-cache.h"
#include "smr-simulator/smr-simulator.h"
#include "trace2call.h"

int
main(int argc, char **argv)
{
	char           *trace_file_path[] = {"/home/trace/MS-Cambridge/wdev_0.csv.req", "/home/trace/MS-Cambridge/wdev_2.csv.req", "/home/trace/MS-Cambridge/rsrch_1.csv.req", "/home/trace/MS-Cambridge/rsrch_0.csv.req", "/home/trace/MS-Cambridge/src1_2.csv.req", "/home/trace/MS-Cambridge/prn_0.csv.req", "/home/trace/MS-Cambridge/ts_0.csv.req", "/home/trace/MS-Cambridge/mds_0.csv.req", "/home/trace/MS-Cambridge/stg_0.csv.req", "/home/trace/MS-Cambridge/hm_0.csv.req", "/home/trace/MS-Cambridge/web_0.csv.req", "/home/trace/production-MSN-FS-4k.req", "/home/trace/MS-Cambridge/usr_0.csv.req", "/home/trace/production-LiveMap-Backend-4K.req", "/home/test.txt"};

	if (argc == 8) {
		NSSDBuffers = atoi(argv[1]);
		NSSDBufTables = atoi(argv[1]);
		NSSDs = atoi(argv[2]);
		NSSDTables = atoi(argv[2]);
		NSSDLIMIT = atoi(argv[2]);
		PERIODTIMES = atoi(argv[2]);
		BandOrBlock = atoi(argv[3]);
		SSD_BUFFER_SIZE = atoi(argv[4]);
		ZONESZ = atoi(argv[5]);
		if (atoi(argv[6]) == 1)
			EvictStrategy = LRU;
		if (atoi(argv[6]) == 2)
			EvictStrategy = LRUofBand;
		if (atoi(argv[6]) == 3)
			EvictStrategy = Most;
		if (atoi(argv[6]) == 4)
			EvictStrategy = CMR;
		if (atoi(argv[6]) == 5)
			EvictStrategy = SMR;
		if (atoi(argv[6]) == 6)
			EvictStrategy = MaxCold;
	} else {
		printf("parameters are wrong %d\n", argc);
		exit(-1);
	}


	initSSD();
	initSSDBuffer();
	smr_fd = open(smr_device, O_RDWR | O_DIRECT);
	ssd_fd = open(ssd_device, O_RDWR);
	inner_ssd_fd = open(inner_ssd_device, O_RDWR | O_DIRECT);
	trace_to_iocall(trace_file_path[atoi(argv[7])]);
	close(smr_fd);
	close(ssd_fd);
	close(inner_ssd_fd);

	return 0;
}
