#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "ssd-cache.h"
#include "smr-simulator/smr-simulator.h"
#include "strategy/clock.h"
#include "strategy/lru.h"
#include "strategy/lruofband.h"
#include "strategy/scan.h"
int BandOrBlock = 0;
/*Block = 0, Band=1*/

void trace_to_iocall(char* trace_file_path) {
	FILE* trace;
	if((trace = fopen(trace_file_path, "rt")) == NULL) {
//		printf("[ERROR] trace_to_iocall():--------Fail to open the trace file!");
		exit(1);
	}
	double time, time_begin, time_now;
    struct timeval tv_begin, tv_now;
    struct timezone tz_begin, tz_now;
	long time_diff;
	char action;
	char write_or_read[100];
	off_t offset;
    size_t size;
	char* ssd_buffer;
	bool is_first_call = 1;
	int i;
	float size_float;

    gettimeofday(&tv_begin, &tz_begin);
    time_begin = tv_begin.tv_sec + tv_begin.tv_usec/1000000.0;
    while(!feof(trace)) {
		fscanf(trace, "%lf %c %s %lu %f", &time, &action, write_or_read, &offset, &size_float);
        //printf("original size : %f\n",size_float);
	gettimeofday(&tv_now, &tz_now);
        if (DEBUG)
          printf("[INFO] trace_to_iocall():--------now time = %lf\n", time_now-time_begin);
        time_now = tv_now.tv_sec + tv_now.tv_usec/1000000.0;
		if (!is_first_call) {
			time_diff = (time - (time_now - time_begin)) * 1000000;
		//	if (time_diff > 0)
                //usleep(time_diff);
		} else {
			is_first_call = 0;
		}
        size = size_float*1024;
//	printf("size: %lu\n",size);
	unsigned long offset_end = offset+size;
	if(offset % 4096 != 0)
		offset = offset/4096*4096;
	if(offset_end % 4096 != 0)
		size = offset_end / 4096 * 4096 - offset + 4096;
	else 
		size = offset_end - offset;
//	printf("offset : %lu    size %lu\n",offset,size);
	ssd_buffer = (char *)malloc(sizeof(char)*BLCKSZ);
    	for (i=0; i<BLCKSZ; i++)
      		ssd_buffer[i] = '1';
	while (size > 0 ) {
        	if(strstr(write_or_read, "W")) {
               	 if (DEBUG)
       				printf("[INFO] trace_to_iocall():--------wirte offset=%lu\n", offset);
        		if(BandOrBlock == 0 )
				write_block(offset, ssd_buffer);
			else
				write_band(offset,ssd_buffer);
     		 } else if(strstr(write_or_read, "R")) {
        /*       	if (DEBUG)
       			printf("[INFO] trace_to_iocall():--------read offset=%lu\n", offset);
        		if(BandOrBlock == 0 )
				read_block(offset, ssd_buffer);
       			else
				read_band(offset,ssd_buffer);
	 */	 }
      	offset += BLCKSZ;
     	size -= BLCKSZ;
    	}
	}
    gettimeofday(&tv_now, &tz_now);
    time_now = tv_now.tv_sec + tv_now.tv_usec/1000000.0;
    printf("total run time (s) = %lf\n", time_now - time_begin);
	printf("hit num:%lu   flush_ssd_blocks:%lu flush_fifo_times:%lu flush_fifo_blocks:%lu  flusd_bands:%lu\n ",hit_num,flush_ssd_blocks,flush_fifo_times,flush_fifo_blocks,flush_bands);
	fclose(trace);
	
}
