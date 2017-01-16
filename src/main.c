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

int main()
{
    char trace_file_path[]="../trace/mds_0.csv.req";

	initSSD();
    initSSDBuffer();
    smr_fd = open(smr_device, O_RDWR|O_DIRECT);
    ssd_fd = open(ssd_device, O_RDWR);
    inner_ssd_fd = open(inner_ssd_device, O_RDWR|O_DIRECT);
    trace_to_iocall(trace_file_path);
    close(smr_fd);
    close(ssd_fd);
    close(inner_ssd_fd);
    
	return 0;
}
