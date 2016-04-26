#include <stdio.h>
#include <stdlib.h>
#include "ssd-cache.h"
#include "smr-simulator.h"
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

void trace_to_iocall(char* trace_file_path) {
	FILE* trace;
	if((trace = fopen(trace_file_path, "rt")) == NULL) {
		printf("Fail to open the trace file!");
		exit(1);
	}
	double time_prv, time_cur;
	int time_diff;
	char action;
	char write_or_read[100];
	off_t offset;
    size_t size;
	char* ssd_buffer;
	bool is_first_call = 1;
	int i;
    while(!feof(trace)) {
		fscanf(trace, "%lf %c %s %lu %lu", &time_cur, &action, write_or_read, &offset, &size);
		if (!is_first_call) {
			time_diff = (time_cur - time_prv) * 1000000;
			usleep(time_diff);
		} else {
			is_first_call = 0;
		}
        size = size*1024;
		if(strstr(write_or_read, "W")) {
            ssd_buffer = (char *)malloc(sizeof(char)*BLCKSZ);
            for (i=0; i<BLCKSZ; i++)

                ssd_buffer[i] = '1';
            while (size > 0 ) {
                printf("wirte offset=%lu\n", offset);
			    write_block(offset, ssd_buffer);
                offset += BLCKSZ;
                size -= BLCKSZ;
            }
		} else if(strstr(write_or_read, "R")) {
            printf("read offset=%lu\n", offset);
			read_block(offset, ssd_buffer); 
		}
		time_prv = time_cur;
	}
	fclose(trace);
	
}
