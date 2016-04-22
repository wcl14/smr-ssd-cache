#include <stdio.h>
#include <stdlib.h>
#include "ssd-cache.h"
#include "smr-simulator.h"
#include <string.h>

void trace_to_iocall(char* trace_file_path) {
	FILE* trace;
	if((trace = fopen(trace_file_path, "rt")) == NULL) {
		printf("Fail to open the trace file!");
		exit(1);
	}
	double time;
	char action;
	char* write_or_read;
	off_t offset;
    size_t size;
	char* ssd_buffer;
	while(feof(trace)) {
		fscanf(trace, "%lf %c %s %lu %lu", &time, &action, write_or_read, &offset, &size);
        size = size*1024;
		if(strstr(write_or_read, "W")) {
            ssd_buffer = (char *)malloc(sizeof(char)*BLCKSZ);
            while (size > 0 ) {
			    write_block(offset, ssd_buffer);
                offset += BLCKSZ;
                size -= BLCKSZ;
            }
		} else if(strstr(write_or_read, "R")) {
			read_block(offset, ssd_buffer); 
		}
	}
	fclose(trace);
	
}
