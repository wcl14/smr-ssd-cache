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

	if (argc == 10) {
		NSSDBuffersWrite = atoi(argv[1]);
		NSSDBuffersRead = atoi(argv[2]);
        NSSDBuffers = NSSDBuffersWrite;
		NSSDBufTables = NSSDBuffersRead + NSSDBuffersWrite;
		NSSDs = atoi(argv[3]);
		NSSDTables = atoi(argv[3]);
		NSSDLIMIT = atoi(argv[3]);
		BandOrBlock = atoi(argv[4]);
		SSD_BUFFER_SIZE = atoi(argv[5]);
		ZONESZ = atoi(argv[6]);
		PERIODTIMES = atoi(argv[7]);
		if (atoi(argv[8]) == 1)
			EvictStrategy = LRU;
		if (atoi(argv[8]) == 2)
			EvictStrategy = LRUofBand;
		if (atoi(argv[8]) == 3)
			EvictStrategy = Most;
		if (atoi(argv[8]) == 4)
			EvictStrategy = CMR;
		if (atoi(argv[8]) == 5)
			EvictStrategy = SMR;
		if (atoi(argv[8]) == 6)
			EvictStrategy = MaxCold;
		if (atoi(argv[8]) == 7)
			EvictStrategy = MaxAll;
		if (atoi(argv[8]) == 8)
			EvictStrategy = AvgBandHot;
		if (atoi(argv[8]) == 9)
			EvictStrategy = HotDivSizeSplitRW;
		if (atoi(argv[8]) == 10)
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
	trace_to_iocall(trace_file_path[atoi(argv[9])]);
    close(smr_fd);
    close(ssd_fd);
    close(inner_ssd_fd);
    
	return 0;
}
