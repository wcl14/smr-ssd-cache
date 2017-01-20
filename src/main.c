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
	char           *trace_file_path[] = {"../trace/hm_0.csv.req", 
                                         "../trace/production-LiveMap-Backend-4K.req",
                                         "../trace/mds_0.csv.req", 
                                         "../trace/prn_0.csv.req", 
                                         "../trace/rsrch_0.csv.req", 
                                         "../trace/src1_2.csv.req", 
                                         "../trace/stg_0.csv.req", 
                                         "../trace/ts_0.csv.req", 
                                         "../trace/usr_0.csv.req", 
                                         "../trace/wdev_0.csv.req", 
                                         "../trace/web_0.csv.req"};

	if (argc == 9) {
		NSSDBuffers = atoi(argv[1]);
		NSSDBufTables = atoi(argv[1]);
		NSSDs = atoi(argv[2]);
		NSSDTables = atoi(argv[2]);
		NSSDLIMIT = atoi(argv[2]);
		BandOrBlock = atoi(argv[3]);
		SSD_BUFFER_SIZE = atoi(argv[4]);
		ZONESZ = atoi(argv[5]);
		PERIODTIMES = atoi(argv[6]);
		if (atoi(argv[7]) == 1)
			EvictStrategy = LRU;
		if (atoi(argv[7]) == 2)
			EvictStrategy = LRUofBand;
		if (atoi(argv[7]) == 3)
			EvictStrategy = Most;
		if (atoi(argv[7]) == 4)
			EvictStrategy = CMR;
		if (atoi(argv[7]) == 5)
			EvictStrategy = SMR;
		if (atoi(argv[7]) == 6)
			EvictStrategy = MaxCold;
		if (atoi(argv[7]) == 7)
			EvictStrategy = MaxAll;
		if (atoi(argv[7]) == 8)
			EvictStrategy = AvgBandHot;
		if (atoi(argv[7]) == 9)
			EvictStrategy = HotDivSizeWriteOnly;
		if (atoi(argv[7]) == 10)
			EvictStrategy = FourQuadrant;
	} else {
		printf("parameters are wrong %d\n", argc);
		exit(-1);
	}


	initSSD();
    initSSDBuffer();
    //smr_fd = open(smr_device, O_RDWR|O_DIRECT);
    //ssd_fd = open(ssd_device, O_RDWR);
    //inner_ssd_fd = open(inner_ssd_device, O_RDWR|O_DIRECT);
    smr_fd = open(smr_device, O_RDWR|O_SYNC);
    ssd_fd = open(ssd_device, O_RDWR);
    inner_ssd_fd = open(inner_ssd_device, O_RDWR|O_SYNC);
	trace_to_iocall(trace_file_path[atoi(argv[8])]);
    close(smr_fd);
    close(ssd_fd);
    close(inner_ssd_fd);
    
	return 0;
}
