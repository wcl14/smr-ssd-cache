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

void
trace_to_iocall(char *trace_file_path)
{
	FILE           *trace;
	if ((trace = fopen(trace_file_path, "rt")) == NULL) {
		printf("[ERROR] trace_to_iocall():--------Fail to open the trace file!");
		exit(1);
	}
	double		time   , time_begin, time_now;
	struct timeval	tv_begin, tv_now;
	struct timezone	tz_begin, tz_now;
	long		time_diff;
	char		action;
	char		write_or_read[100];
	off_t		offset;
	size_t		size;
	char           *ssd_buffer;
	bool		is_first_call = 1;
	int		i;
	float		size_float;

	gettimeofday(&tv_begin, &tz_begin);
	time_begin = tv_begin.tv_sec + tv_begin.tv_usec / 1000000.0;
	int		returnCode = posix_memalign(&ssd_buffer, 512, sizeof(char) * BLCKSZ);
	if (returnCode < 0) {
		printf("[ERROR] flushSSDBuffer():--------posix memalign\n");
		free(ssd_buffer);
		exit(-1);
	}
	while (!feof(trace)) {
		returnCode = fscanf(trace, "%c %d %lu\n", &action, &i, &offset);
		if (returnCode < 0)
			break;

		gettimeofday(&tv_now, &tz_now);
		if (DEBUG)
			printf("[INFO] trace_to_iocall():--------now time = %lf\n", time_now - time_begin);
		time_now = tv_now.tv_sec + tv_now.tv_usec / 1000000.0;
		if (!is_first_call) {
			time_diff = (time - (time_now - time_begin)) * 1000000;
		} else {
			is_first_call = 0;
		}
		size = 4096;
		offset = offset * BLCKSZ;

		unsigned long	offset_end = offset + size;
		if (offset % 4096 != 0)
			offset = offset / 4096 * 4096;
		if (offset_end % 4096 != 0)
			size = offset_end / 4096 * 4096 - offset + 4096;
		else
			size = offset_end - offset;

		for (i = 0; i < BLCKSZ; i++)
			ssd_buffer[i] = '1';
		while (size > 0) {
			if (action == '1') {
				if (DEBUG)
					printf("[INFO] trace_to_iocall():--------wirte offset=%lu\n", offset);
				if (BandOrBlock == 0)
					write_block(offset, ssd_buffer);
				else
					write_band(offset, ssd_buffer);
			} else if (action == '0') {
				if (DEBUG)
					printf("[INFO] trace_to_iocall():--------read offset=%lu\n", offset);
				//if (BandOrBlock == 0)
				//	read_block(offset, ssd_buffer);
				//else
				//	read_band(offset, ssd_buffer);
			}
			offset += BLCKSZ;
			size -= BLCKSZ;
		}
	}
	gettimeofday(&tv_now, &tz_now);
	time_now = tv_now.tv_sec + tv_now.tv_usec / 1000000.0;
	printf("total run time (s) = %lf  time_read_cmr = %lf time_write_cmr = %lf time_read_ssd = %lf  time_write_ssd = %lf  time_read_fifo = %lf  time_write_fifo = %lf   time_read_smr = %lf   time_write_smr = %lf\n", time_now - time_begin, time_read_cmr, time_write_cmr, time_read_ssd, time_write_ssd, time_read_fifo, time_write_fifo, time_read_smr, time_write_smr);
	printf("read_hit_num:%lu  hit num:%lu   read_ssd_blocks:%lu  flush_ssd_blocks:%lu flush_fifo_times:%lu read_fifo_blocks:%lu   flush_fifo_blocks:%lu  read_smr_blocks:%lu   read_smr_bands:%lu   flush_bands:%lu flush_band_size=%lu\n ", read_hit_num, hit_num, read_ssd_blocks, flush_ssd_blocks, flush_fifo_times, read_fifo_blocks, flush_fifo_blocks, read_smr_blocks, read_smr_bands, flush_bands, flush_band_size);
	free(ssd_buffer);
	fclose(trace);

}
