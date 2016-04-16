#include <stdio.h>
#include <stdlib.h>
#include "ssd-cache.h"
#include "smr-simulator.h"
#include "strategy.h"

SSDBufferDesc *getCLOCKBuffer()
{
	
	SSDBufferDesc *ssd_buf_hdr;

	if (ssd_buffer_strategy_control->first_freessd >=0 ) {
		ssd_buf_hdr = &ssd_buffer_descriptors[ssd_buffer_strategy_control->first_freessd];
		ssd_buffer_strategy_control->first_freessd = ssd_buf_hdr->next_freessd;
		ssd_buf_hdr->next_freessd = -1;
		return ssd_buf_hdr;
	}

	for (;;) {
		ssd_buf_hdr = &ssd_buffer_descriptors[ssd_buffer_strategy_control->next_victimssd];
		ssd_buffer_strategy_control->next_victimssd++;
		if (ssd_buffer_strategy_control->next_victimssd >= NSSDBuffers) {
			ssd_buffer_strategy_control->next_victimssd = 0;
		}
		if (ssd_buf_hdr->usage_count > 0) {
			ssd_buf_hdr->usage_count--;
		}
		else
			return ssd_buf_hdr;
	}
	
	return NULL;
}

SSDBufferDesc *getLRUOfBandBuffer()
{
	return NULL;
}

