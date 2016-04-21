#include <stdio.h>
#include <stdlib.h>
#include "ssd-cache.h"
#include "smr-simulator.h"
#include <string.h>

void trace_to_iocall(char* trace_file_path) {
	FILE* trace;
	if((trace = fopen(trace_file_path, rt)) == NULL) {
		printf("Fail to open the trace file!");
		exit(1);
	}
	double time;
	char action;
	char* write_or_read;
	long blk_address;
	int offset;
	char* ssd_buffer;
	while(feof(trace)) {
		fscanf("%lf %c %s %ld %d", time, action, write_or_read, blk_address, offset);
		if(strstr(write_or_read, "W")) {
			write_block(blk_address + offset, ssd_buffer);
		} else if(strstr(write_or_read, "R")) {
			read_block(blk_address + offset, ssd_buffer); 
		}
	}
	fclose(trace);
	
}
