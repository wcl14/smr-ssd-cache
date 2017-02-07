#include <stdio.h>
#include <stdlib.h>
#include "ssd-cache.h"
#include "smr-simulator/smr-simulator.h"
#include "clock.h"

/*
 * init strategy_control for clock
 */
void 
initSSDBufferForClock()
{

	ssd_buffer_strategy_control_for_clock = (SSDBufferStrategyControlForClock *) malloc(sizeof(SSDBufferStrategyControlForClock));
	ssd_buffer_strategy_control_for_clock->next_victimssd = 0;

	ssd_buffer_descriptors_for_clock = (SSDBufferDescForClock *) malloc(sizeof(SSDBufferDescForClock) * NSSDBuffers);
	SSDBufferDescForClock *ssd_buf_hdr_for_clock;
	long		i;
	ssd_buf_hdr_for_clock = ssd_buffer_descriptors_for_clock;
	for (i = 0; i < NSSDBuffers; ssd_buf_hdr_for_clock++, i++) {
		ssd_buf_hdr_for_clock->ssd_buf_id = i;
		ssd_buf_hdr_for_clock->usage_count = 0;
	}
	flush_fifo_times = 0;
}

SSDBufferDesc  *
getCLOCKBuffer()
{
	SSDBufferDescForClock *ssd_buf_hdr_for_clock;
	SSDBufferDesc  *ssd_buf_hdr;

	if (ssd_buffer_strategy_control->first_freessd >= 0) {
		ssd_buf_hdr = &ssd_buffer_descriptors[ssd_buffer_strategy_control->first_freessd];
		ssd_buffer_strategy_control->first_freessd = ssd_buf_hdr->next_freessd;
		ssd_buf_hdr->next_freessd = -1;
		ssd_buffer_strategy_control->n_usedssd++;
		return ssd_buf_hdr;
	}
	flush_fifo_times++;
	for (;;) {
		ssd_buf_hdr_for_clock = &ssd_buffer_descriptors_for_clock[ssd_buffer_strategy_control_for_clock->next_victimssd];
		ssd_buf_hdr = &ssd_buffer_descriptors[ssd_buffer_strategy_control_for_clock->next_victimssd];
		ssd_buffer_strategy_control_for_clock->next_victimssd++;
		if (ssd_buffer_strategy_control_for_clock->next_victimssd >= NSSDBuffers) {
			ssd_buffer_strategy_control_for_clock->next_victimssd = 0;
		}
		if (ssd_buf_hdr_for_clock->usage_count > 0) {
			ssd_buf_hdr_for_clock->usage_count--;
		} else {
			unsigned char	old_flag = ssd_buf_hdr->ssd_buf_flag;
			SSDBufferTag	old_tag = ssd_buf_hdr->ssd_buf_tag;
			if (DEBUG)
				printf("[INFO] SSDBufferAlloc(): old_flag&SSD_BUF_DIRTY=%d\n", old_flag & SSD_BUF_DIRTY);
			if ((old_flag & SSD_BUF_DIRTY) != 0) {
				flushSSDBuffer(ssd_buf_hdr);
			}
			if ((old_flag & SSD_BUF_VALID) != 0) {
				unsigned long	old_hash = ssdbuftableHashcode(&old_tag);
				ssdbuftableDelete(&old_tag, old_hash);
			}
			return ssd_buf_hdr;
		}
	}

	return NULL;
}


void           *
hitInCLOCKBuffer(SSDBufferDesc * ssd_buf_hdr)
{
	SSDBufferDescForClock *ssd_buf_hdr_for_clock;
	ssd_buf_hdr_for_clock = &ssd_buffer_descriptors_for_clock[ssd_buf_hdr->ssd_buf_id];
	ssd_buf_hdr_for_clock->usage_count++;

	return NULL;
}
