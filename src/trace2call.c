#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "ssd-cache.h"
#include "smr-simulator/smr-simulator.h"

void trace_to_iocall(char* trace_file_path) {
  FILE* trace;
  if((trace = fopen(trace_file_path, "rt")) == NULL) {
    printf("[ERROR] trace_to_iocall():--------Fail to open the trace file!");
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

  gettimeofday(&tv_begin, &tz_begin);
  time_begin = tv_begin.tv_sec + tv_begin.tv_usec/1000000.0;
  while(!feof(trace)) {
    fscanf(trace, "%lf %c %s %lu %lu", &time, &action, write_or_read, &offset, &size);
    gettimeofday(&tv_now, &tz_now);
    //        if (DEBUG)
    printf("[INFO] trace_to_iocall():--------now time = %lf\n", time_now-time_begin);
    time_now = tv_now.tv_sec + tv_now.tv_usec/1000000.0;
    if (!is_first_call) {
      time_diff = (time - (time_now - time_begin)) * 1000000;
      if (time_diff > 0)
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
        //                if (DEBUG)
        printf("[INFO] trace_to_iocall():--------wirte offset=%lu\n", offset);
        write_block(offset, ssd_buffer);
        offset += BLCKSZ;
        size -= BLCKSZ;
      }
    } else if(strstr(write_or_read, "R")) {
      //            if (DEBUG)
      printf("[INFO] trace_to_iocall():--------read offset=%lu\n", offset);
      read_block(offset, ssd_buffer); 
    }
  }

  gettimeofday(&tv_now, &tz_now);
  time_now = tv_now.tv_sec + tv_now.tv_usec/1000000.0;
  printf("total run time (s) = %lf\n", time_now - time_begin);

  fclose(trace);
}
